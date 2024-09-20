#include "jump_into_app.h"
#include "sys_delay.h"
#include "usart_printf.h"
#include "main.h"
#include "usart.h"
#include "dma.h"

typedef void (*pFunction)(void);

/**
 * @brief 跳转到应用程序段
 *        执行条件：无
 * @param[in1] : 用户代码起始地址.
 *
 * @retval: 无
 */
void jump_to_app(uint32_t app_addr)
{

    pFunction jump_to_application;
    uint32_t jump_address;

    /* Check if valid stack address (RAM address) then jump to user application */
    // 检查代码第一个地址，是否为合法的MSP，第二个地址为中断向量表地址
    if (((*(__IO uint32_t *)app_addr) & 0x2FFE0000) == 0x20000000)
    {
        printf("JUMP1\n");
        /* 屏蔽所有中断，防止在跳转过程中，中断干扰出现异常 */

        HAL_DeInit();
        HAL_UART_MspDeInit(&huart2);
        HAL_DMA_DeInit(&hdma_usart2_rx);
        HAL_DMA_DeInit(&hdma_usart2_tx);

        HAL_RCC_DeInit();
        __disable_irq();
        for (int i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
        SysTick->CTRL = 0X00; // 禁止SysTick
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        __enable_irq(); // 开启中断
        /* Jump to user application */
        jump_address = *(__IO uint32_t *)(app_addr + 4);
        // printf("JUMP2\n");
        jump_to_application = (pFunction)jump_address;
        // printf("JUMP3\n");
        /* Initialize user application's Stack Pointer */
        __set_PSP(*(__IO uint32_t *)app_addr);
        __set_CONTROL(0);
        __set_MSP(*(__IO uint32_t *)app_addr);
        // printf("JUMP4\n");//调用打印会导致跳转失败
        jump_to_application();
        printf("JUMP5\n");
    }
}

/**
 * @brief 校验和启动程序
 */
void main_check_and_start(void)
{
    if (((FLASH_JUMP_ADDR + 4) & 0xFF000000) == 0x08000000) // Judge if start at 0X08XXXXXX.
    {
        printf("HELLO WORLD!\n");
        sys_delay_ms(2000);
        jump_to_app(FLASH_JUMP_ADDR); // Jump to  APP
    }
}
