#ifndef __USART_PRINTF_H__
#define __USART_PRINTF_H__

#include <stdio.h>

#define PRINTF_UART USART1
#define PRINTF_FPUTC_MODE 0//是否启用FPUTC(不使用蓝牙修改配置)


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

#if PRINTF_FPUTC_MODE == 0

#define PRINTF_RX_BUFFER_SIZE 256
#define PRINTF_TX_BUFFER_SIZE 256 

static uint8_t printf_rx_buffer[PRINTF_RX_BUFFER_SIZE];
static uint8_t printf_tx_buffer[PRINTF_TX_BUFFER_SIZE];

extern uart_rx_frame printf_rx_frame;
extern uart_tx_frame printf_tx_frame;

// typedef enum debug_out_type
// {
//   debug_out_reset = 0,
//   debug_out_set = 1,
// };

void printf_irq(void);
uint8_t config_pirntf(char *fmt, ...);
uint8_t config_hex_printf(uint8_t *data, uint16_t len);
uint8_t *config_get_rx_buffer(void);
uint16_t config_get_rx_len(void);
void config_rx_reset(void);
void debug_out_set(uint8_t status);

#endif

void uart_irq(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_usart_rx,uart_rx_frame *rx_frame, uint16_t rx_len);
void uart_enable_dma_it(UART_HandleTypeDef *huart, uart_rx_frame *rx_frame, uart_tx_frame *tx_frame, uint16_t rx_len);
void uart_rx_reset(uart_rx_frame *rx_frame);
uint8_t *uart_rx_get_buf(uart_rx_frame *rx_frame);
uint16_t uart_rx_get_len(uart_rx_frame rx_frame);
void wait_uart_tx_finish_flag(uart_tx_frame *tx_frame);
uint8_t uart_transmit_dma(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame);
uint8_t uart_hex_printf(UART_HandleTypeDef *huart, uart_tx_frame *tx_frame, uint8_t *buf, uint16_t len, uint16_t tx_len);

#endif /* __USART_PRINTF_H__ */
