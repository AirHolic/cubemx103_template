/* 本文件旨在实现重定向printf至PRINTF_UART，PRINTF_UART的接收部分需于中断和回调部分自行完成。 */
#include "main.h"
#include "usart.h"
#include "usart_printf.h"
#include "stdarg.h"
#include "string.h"

/* 如果使用os,则包括下面的头文件即可 */
#if SYS_SUPPORT_OS
#include "includes.h" /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)           /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t"); /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");   /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((PRINTF_UART->SR & 0X40) == 0)
        ; /* 等待上一个字符发送完成 */

    PRINTF_UART->DR = (uint8_t)ch; /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/

/**********************************usart外设printf相关函数**************************************/
/* 空闲中断和dma接收与dma发送 */

/**
 * @brief 通用串口DMA与空闲中断处理函数
 * @param huart 串口句柄
 * @param hdma_usart_rx 串口DMA接收句柄
 * @param rx_frame 串口接收帧缓冲信息结构体
 * @retval None
 */
void uart_irq(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_usart_rx, uart_rx_frame *rx_frame)
{
    uint16_t tmp = 0;
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET) /* UART接收过载错误中断 */
    {
        __HAL_UART_CLEAR_OREFLAG(huart); /* 清除接收过载错误中断标志 */
        (void)huart->Instance->SR;       /* 先读SR寄存器，再读DR寄存器 */
        (void)huart->Instance->DR;
    }

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) /* UART总线空闲中断 */
    {
        rx_frame->sta.finsh = 1;                                                                  /* 标记帧接收完成 */
        __HAL_UART_CLEAR_IDLEFLAG(huart);                                                         /* 清除UART总线空闲中断 */
        HAL_UART_DMAStop(huart);                                                                  /* 停止DMA传输 */
        tmp = __HAL_DMA_GET_COUNTER(hdma_usart_rx);                                               /* 清除DMA接收中断标志 */
        rx_frame->sta.len = ((UART_RX_BUFFER_SIZE - tmp) <= 0) ? 0 : (UART_RX_BUFFER_SIZE - tmp); /* 计算接收到的数据长度 */
        HAL_UART_Receive_DMA(huart, rx_frame->buf, UART_RX_BUFFER_SIZE);                          /* 重新开始DMA传输 */
    }
}

/**
 * @brief 串口等待发送完成标志
 * @param tx_frame 串口发送帧缓冲信息结构体
 * @retval None
 */
void wait_uart_tx_finish_flag(uart_tx_frame *tx_frame)
{
    while (tx_frame->sta.finsh == 0)
        sys_delay_us(1);
    tx_frame->sta.finsh = 0;
}

/**
 * @brief 串口发送DMA
 * @param huart 串口句柄
 * @param tx_frame 串口发送帧缓冲信息结构体
 * @retval HAL_StatusTypeDef
 */
uint8_t uart_transmit_dma(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame)
{
    return HAL_UART_Transmit_DMA(huart, tx_frame->uart_tx_buf, tx_frame->sta.len);
}

/** 串口printf参考函数
uint8_t uart_printf(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame, char *fmt, ...)
{
    va_list ap;
    wait_uart_tx_finish_flag(tx_frame);

    va_start(ap, fmt);
    vsnprintf((char *)tx_frame->uart_tx_buf, fmt, ap);
    va_end(ap);

    tx_frame->sta.len = strlen((const char *)tx_frame->uart_tx_buf);
    return uart_transmit_dma(huart, tx_frame);
}
*/

/**
 * @brief  串口发送十六进制数据
 * @param  huart: 串口句柄
 * @param  tx_frame: 串口发送帧缓冲信息结构体
 * @param  buf: 待发送的数据
 * @param  len: 待发送的数据长度
 * @retval HAL_StatusTypeDef
*/
uint8_t uart_hex_printf(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame, uint8_t *buf, uint16_t len)
{
    if(len > UART_TX_BUFFER_SIZE)
    {
        return 1;
    }
    wait_uart_tx_finish_flag(tx_frame);
    memcpy(tx_frame->uart_tx_buf, buf, len);
    tx_frame->sta.len = len;
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", tx_frame->uart_tx_buf[i]);
    }
    printf("\r\n");
    return uart_transmit_dma(huart, tx_frame);
}

/**
 * @brief  重置接收缓冲区
 * @param  rx_frame: 接收帧缓冲区指针
 */
void uart_rx_reset(uart_rx_frame *rx_frame)
{
    rx_frame->sta.len = 0;
    rx_frame->sta.finsh = 0;
}

/**
 * @brief  获取接收到的数据
 * @retval 数据指针
 */
uint8_t *uart_rx_get_buf(uart_rx_frame *rx_frame)
{
    if (rx_frame->sta.finsh == 1)
    {
        rx_frame->buf[rx_frame->sta.len] = '\0';
        return rx_frame->buf;
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief  获取接收到的数据长度
 * @retval 数据长度
 */
uint16_t uart_rx_get_len(uart_rx_frame rx_frame)
{
    if (rx_frame.sta.finsh == 1)
    {
        return rx_frame.sta.len;
    }
    else
    {
        return 0;
    }
}

/**********************************************END*********************************************/
