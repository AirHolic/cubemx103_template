/**
 * @file WS8475DLS_driver.h
 * @author AirHolic
 * @brief WS8475DLS LORA 模块驱动程序,使用LORA时调用此程序中的发射数据函数,接收数据函数
 * @note 本驱动程序基于STM32 HAL库,使用串口通信,使用前请确保已经初始化串口
 * @version 0.1
 * @date 2025-03-26
 * @todo 待实际测试,以及补充setmode函数
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __WS8475DLS_DRIVER_H__
#define __WS8475DLS_DRIVER_H__

#include <stdint.h>

/* 硬件抽象层函数指针类型定义 */
typedef uint8_t (*uart_send_buf_fn)(uint8_t *data, uint16_t len);
typedef uint8_t* (*uart_recv_buf_fn)(void);
typedef uint16_t (*uart_recv_buflen_fn)(void);
typedef void (*uart_recv_buf_reset_fn)(void);
typedef uint8_t (*gpio_read_aux_fn)(void);
typedef void (*delay_ms_fn)(uint32_t ms);

/* 模块工作模式枚举 */
typedef enum {
    MODE_TRANSPARENT = 0,
    MODE_WOR_TX = 1,
    MODE_WOR_RX = 2,
    MODE_CONFIG = 3
} ws8475_mode_m;

typedef enum {
    CHECK_BIT_NONE_DEFAULT = 0,  //8N1
    CHECK_BIT_ODD = 1,   //8O1
    CHECK_BIT_EVEN = 2,   //8E1
    CHECK_BIT_NONE = 3    //8N1
} ws8475_check_bit_m;

typedef enum {
    BAUDRATE_1200 = 0,
    BAUDRATE_2400 = 1,
    BAUDRATE_4800 = 2,
    BAUDRATE_9600 = 3,
    BAUDRATE_19200 = 4,
    BAUDRATE_38400 = 5,
    BAUDRATE_57600 = 6,
    BAUDRATE_115200 = 7
}ws8475_baudrate_m;

typedef enum{
    AIR_RATE_2_4K = 0,
    AIR_RATE_4_8K = 4,
    AIR_RATE_9_6K,
    AIR_RATE_19_2K,
    AIR_RATE_38_4K,
    AIR_RATE_62_5K,
}ws8475_air_rate_m;

typedef enum{
    WS8475_DISABLE = 0,
    WS8475_ENABLE = 1
}ws8475_enable_m;

typedef enum{
    WS8475_WOR_WAKEUP_250MS = 0,
    WS8475_WOR_WAKEUP_500MS,
    WS8475_WOR_WAKEUP_750MS,
    WS8475_WOR_WAKEUP_1000MS,
    WS8475_WOR_WAKEUP_1250MS,
    WS8475_WOR_WAKEUP_1500MS,
    WS8475_WOR_WAKEUP_1750MS,
    WS8475_WOR_WAKEUP_2000MS,
}ws8475_wor_wakeup_time_m;

typedef enum{
    WS8475_POWER_22DBM = 0,
    WS8475_POWER_17DBM,
    WS8475_POWER_13DBM,
    WS8475_POWER_10DBM
}ws8475_power_m;

#define WS8475DLS_CHANNEL_410_PLUS(x) (x > 82) ? 82 : x //0~96,但为保证通信质量，建议使用410+频段的0~82通道
#define WS8475DLS_MAX_BUF_LEN 240

#pragma pack(push, 1)
/* 模块配置参数结构体 */
typedef struct {
    //字节1
    uint8_t address_h;
    //字节2
    uint8_t address_l;
    //字节3
    uint8_t check_bit:2;//bit7~6
    uint8_t baudrate:3;//bit5~3
    uint8_t air_rate:3;//bit2~0
    //字节4
    uint8_t channel;
    //字节5
    uint8_t fp_mode_enable:1;//bit7
    uint8_t wor_mode_enable:1;//bit6
    uint8_t wor_wireless_wakeup_time:3;//bit5~3
    uint8_t lbt_enble:1;//bit2
    uint8_t tx_power:2;//bit1~0
    //字节6
    uint8_t rssi_enable:1;
} ws8475_config_t;
#pragma pack(pop)

/* 驱动上下文结构体 */
typedef struct {
    /* 硬件抽象接口 */
    uart_send_buf_fn send_buf;
    uart_recv_buf_fn recv_buf;
    uart_recv_buflen_fn recv_buflen;
    uart_recv_buf_reset_fn recv_buf_reset;
    gpio_read_aux_fn read_aux;
    delay_ms_fn delay;
    
    /* 内部状态 */
    ws8475_config_t config;
} ws8475_handle_t;

/* 初始化函数 */
uint8_t ws8475_init(ws8475_handle_t *handle);

/* 基础功能 */
uint8_t ws8475_set_mode(ws8475_handle_t *handle, ws8475_mode_m mode);
uint8_t ws8475_send_data(ws8475_handle_t *handle, uint8_t *data, uint16_t len);
uint8_t ws8475_receive_data(ws8475_handle_t *handle, uint8_t *buffer, uint16_t *len);

/* 高级配置 */
uint8_t ws8475_write_config(ws8475_handle_t *handle, uint8_t save_to_flash);
uint8_t ws8475_read_config(ws8475_handle_t *handle);
uint8_t ws8475_reboot(ws8475_handle_t *handle);
uint8_t ws8475_factory_reset(ws8475_handle_t *handle);

/* 状态检查 */
uint8_t ws8475_wait_ready(ws8475_handle_t *handle, uint32_t timeout_ms);

#endif /* __WS8475DLS_DRIVER_H__ */
