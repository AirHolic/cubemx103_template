#ifndef __LORA_AT_FUN_H__
#define __LORA_AT_FUN_H__

extern at_device_t lora_at_dev;

/* 使能枚举 */
typedef enum
{
    LORA_DISABLE             = 0x00,
    LORA_ENABLE,
} lora_enable_t;

/* 发射功率枚举 */
typedef enum
{
    LORA_TPOWER_9DBM         = 0,   /* 9dBm */
    LORA_TPOWER_11DBM        = 1,   /* 11dBm */
    LORA_TPOWER_14DBM        = 2,   /* 14dBm */
    LORA_TPOWER_17DBM        = 3,   /* 17dBm */
    LORA_TPOWER_20DBM        = 4,   /* 20dBm（默认） */
    LORA_TPOWER_22DBM        = 5,   /* 22dBm */
} lora_tpower_t;

/* 工作模式枚举 */
typedef enum
{
    LORA_WORKMODE_NORMAL     = 0,    /* 一般模式（默认） */
    LORA_WORKMODE_WAKEUP     = 1,    /* 唤醒模式 */
    LORA_WORKMODE_LOWPOWER   = 2,    /* 省电模式 */
    LORA_WORKMODE_SIGNAL     = 3,    /* 信号强度模式 */
    LORA_WORKMODE_SLEEP      = 4,    /* 休眠模式 */
    LORA_WORKMODE_RELAY      = 5,    /* 中继模式 */
} lora_workmode_t;

/* 发射模式枚举 */
typedef enum
{
    LORA_TMODE_TT            = 0,    /* 透明传输（默认） */
    LORA_TMODE_DT            = 1,    /* 定向传输 */
} lora_tmode_t;

/* 空中速率枚举 */
typedef enum
{
    LORA_WLRATE_1K2          = 1,    /* 1.2Kbps */
    LORA_WLRATE_2K4          = 2,    /* 2.4Kbps */
    LORA_WLRATE_4K8          = 3,    /* 4.8Kbps */
    LORA_WLRATE_9K6          = 4,    /* 9.6Kbps */
    LORA_WLRATE_19K2         = 5,    /* 19.2Kbps（默认） */
    LORA_WLRATE_38K4         = 6,    /* 38.4Kbps */
    LORA_WLRATE_62K5         = 7,    /* 62.5Kbps */
} lora_wlrate_t;

/* 休眠时间枚举 */
typedef enum
{
    LORA_WLTIME_1S           = 0,    /* 1秒（默认） */
    LORA_WLTIME_2S           = 1,    /* 2秒 */
} lora_wltime_t;

/* 数据包大小枚举 */
typedef enum
{
    LORA_PACKSIZE_32         = 0,    /* 32字节 */
    LORA_PACKSIZE_64         = 1,    /* 64字节 */
    LORA_PACKSIZE_128        = 2,    /* 128字节 */
    LORA_PACKSIZE_240        = 3,    /* 240字节 */
} lora_packsize_t;

/* 串口通信波特率枚举 */
typedef enum
{
    LORA_UARTRATE_1200BPS    = 0,    /* 1200bps */
    LORA_UARTRATE_2400BPS    = 1,    /* 2400bps */
    LORA_UARTRATE_4800BPS    = 2,    /* 4800bps */
    LORA_UARTRATE_9600BPS    = 3,    /* 9600bps */
    LORA_UARTRATE_19200BPS   = 4,    /* 19200bps */
    LORA_UARTRATE_38400BPS   = 5,    /* 38400bps */
    LORA_UARTRATE_57600BPS   = 6,    /* 57600bps */
    LORA_UARTRATE_115200BPS  = 7,    /* 115200bps（默认） */
} lora_uartrate_t;

/* 串口通讯校验位枚举 */
typedef enum
{
    LORA_UARTPARI_NONE       = 0,    /* 无校验（默认） */
    LORA_UARTPARI_EVEN       = 1,    /* 偶校验 */
    LORA_UARTPARI_ODD        = 2,    /* 奇校验 */
} lora_uartpari_t;

/* 错误代码 */
#define LORA_EOK             0       /* 没有错误 */
#define LORA_ERROR           1       /* 通用错误 */
#define LORA_ETIMEOUT        2       /* 超时错误 */
#define LORA_EINVAL          3       /* 参数错误 */
#define LORA_EBUSY           4       /* 忙错误 */

/* 操作函数 */
void lora_at_fun_init(void);                                                                  /* LORA初始化 */
void lora_enter_config(void);                                                                /* LORA模块进入配置模式 */
void lora_exit_config(void);                                                                 /* LORA模块进退出置模式 */
uint8_t lora_free(void);                                                                     /* 判断LORA模块是否空闲 */
uint8_t lora_at_test(void);                                                                  /* LORA模块AT指令测试 */
uint8_t lora_echo_config(lora_enable_t enable);                                       /* LORA模块指令回显配置 */
uint8_t lora_sw_reset(void);                                                                 /* LORA模块软件复位 */
uint8_t lora_flash_config(lora_enable_t enable);                                      /* LORA模块参数保存配置 */
uint8_t lora_default(void);                                                                  /* LORA模块恢复出厂配置 */
uint8_t lora_addr_config(uint16_t addr);                                                     /* LORA模块设备地址配置 */
uint8_t lora_tpower_config(lora_tpower_t tpower);                                     /* LORA模块发射功率配置 */
uint8_t lora_workmode_config(lora_workmode_t workmode);                               /* LORA模块工作模式配置 */
uint8_t lora_tmode_config(lora_tmode_t tmode);                                        /* LORA模块发送模式配置 */
uint8_t lora_netid_config(uint8_t netid);                                               /* LORA模块网络ID配置 */
uint8_t lora_wlrate_channel_config(lora_wlrate_t wlrate, uint8_t channel);            /* LORA模块空中速率和信道配置 */
uint8_t lora_wltime_config(lora_wltime_t wltime);                                     /* LORA模块休眠时间配置 */
uint8_t lora_packsize_config(lora_packsize_t packsize);                               /* LORA模块数据包大小配置 */
uint8_t lora_datakey_config(uint32_t datakey);                                         /* LORA模块数据加密密钥配置 */
uint8_t lora_uaconfig(lora_uartrate_t baudrate, lora_uartpari_t parity);    				/* LORA模块串口配置 */
uint8_t lora_lbt_config(lora_enable_t enable);                                         /* LORA模块信道检测配置 */

#endif /* _LORA_AT_FUN_H__ */
