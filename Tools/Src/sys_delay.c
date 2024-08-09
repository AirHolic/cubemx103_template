#include "main.h"
#include "sys_delay.h"

#if (RTOS == 1)

/**
 * @brief  Delay function
 * @param  Delay: Delay in ms
 * @retval None
 * @note   替换为rtos用的延时函数
*/
void sys_delay_ms(uint32_t Delay)
{
	vTaskDelay(Delay);
}

/**
 * @brief  Delay function
 * @param  Delay: Delay in us
 * @retval None
 * @note   基于读取SysTick寄存器的延时函数,存在误差,systick为默认配置即1ms中断时生效,
 * 不支持大于1000us的延时,由于微秒级延时不足以支持RTOS切换任务,因此仅用于时序要求较高的场景
*/
void sys_delay_us(uint32_t udelay)
{
    uint32_t startval, tickn, delays, wait;
	startval = SysTick->VAL;//即不大于72000，当前时间计数值，随时间递减
    tickn = HAL_GetTick();//当前时间，随中断递增
    //sysc = 72000; //SystemCoreClock / (1000U / uwTickFreq);
    delays = udelay * MCU_CLOCK_MHZ;  // sysc / 1000 * udelay;
    if (delays > startval)//如果延时时间计数大于当前时间计数值
    {
        while (HAL_GetTick() == tickn);//等待下一个ms，即重新装载
        wait = MCU_CLOCK_MHZ * 1000 + startval - delays;//新装载值+当前时间计数值-延时时间计数值
        while (wait < SysTick->VAL);//等待剩余装载值
    }
    else
    {   //如果延时时间计数小于当前时间计数值
        wait = startval - delays;//当前时间计数值-延时时间计数值
        while (wait < SysTick->VAL && HAL_GetTick() == tickn);
        //等待延时时间计数值小于当前时间计数值
        //或判断指令执行期间是否进入下一个ms
    }
}

#else //无RTOS

/**
 * @brief  Delay function
 * @param  Delay: Delay in ms
 * @retval None
 * @note   基于HAL库的延时函数，不可用于中断
*/
void sys_delay_ms(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/**
 * @brief  Delay function
 * @param  Delay: Delay in us
 * @retval None
 * @note   基于读取SysTick寄存器的延时函数,存在误差,systick为默认配置即1ms中断时生效,
 * 不支持大于1000us的延时
*/
void sys_delay_us(uint32_t udelay)
{
    uint32_t startval, tickn, delays, wait;
	startval = SysTick->VAL;//即不大于72000，当前时间计数值，随时间递减
    tickn = HAL_GetTick();//当前时间，随中断递增
    //sysc = 72000; //SystemCoreClock / (1000U / uwTickFreq);
    delays = udelay * MCU_CLOCK_MHZ;  // sysc / 1000 * udelay;
    if (delays > startval)//如果延时时间计数大于当前时间计数值
    {
        while (HAL_GetTick() == tickn);//等待下一个ms，即重新装载
        wait = MCU_CLOCK_MHZ * 1000 + startval - delays;//新装载值+当前时间计数值-延时时间计数值
        while (wait < SysTick->VAL);//等待剩余装载值
    }
    else
    {   //如果延时时间计数小于当前时间计数值
        wait = startval - delays;//当前时间计数值-延时时间计数值
        while (wait < SysTick->VAL && HAL_GetTick() == tickn);
        //等待延时时间计数值小于当前时间计数值
        //或判断指令执行期间是否进入下一个ms
    }
}

#endif
