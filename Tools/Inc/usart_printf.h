#ifndef __USART_PRINTF_H__
#define __USART_PRINTF_H__

#include <stdio.h>

#define PRINTF_UART USART1

typedef struct
{
  struct
  {
    uint16_t len : 15;                                /* 帧接收长度，sta[14:0] */
    uint16_t finsh : 1;                               /* 帧接收完成标志，sta[15] */
  } sta;                                              /* 帧状态信息 */
  uint8_t *buf; /* 帧接收缓冲 */
} uart_rx_frame;                                /* ATK-IDE01 UART接收帧缓冲信息结构体 */

typedef struct
{
  struct
  {
    uint16_t len : 15;                                /* 帧发送长度，sta[14:0] */
    uint16_t finsh : 1;                               /* 帧发送完成标志，sta[15] */
  } sta;                                              /* 帧状态信息 */
  uint8_t *uart_tx_buf; /* ATK-IDE01 UART发送缓冲 */
} uart_tx_frame;                                /* ATK-IDE01 UART发送帧缓冲信息结构体 */

void uart_irq(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_usart_rx,uart_rx_frame *rx_frame, uint16_t rx_len);
void uart_enable_dma_it(UART_HandleTypeDef *huart, uart_rx_frame *rx_frame, uart_tx_frame *tx_frame, uint16_t rx_len);
void uart_rx_reset(uart_rx_frame *rx_frame);
uint8_t *uart_rx_get_buf(uart_rx_frame *rx_frame);
uint16_t uart_rx_get_len(uart_rx_frame rx_frame);
void wait_uart_tx_finish_flag(uart_tx_frame *tx_frame);
uint8_t uart_transmit_dma(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame);
uint8_t uart_hex_printf(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame, uint8_t *buf, uint16_t len, uint16_t tx_len);

#endif /* __USART_PRINTF_H__ */
