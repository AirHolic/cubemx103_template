#ifndef __LORA_AT_FUN_H__
#define __LORA_AT_FUN_H__

#include "main.h"

extern at_device_t lora_at_dev;

/* 基础开关 */
typedef enum
{
    LORA_ON = 0x00U,
    LORA_OFF = 0x01U,
} lora_switch_t;

/* 工作模式 */
typedef enum
{
    LORA_MODE_TRANS = 0x00U,
    LORA_MODE_FP = 0x01U,
} lora_wmode_t;

/* 串口波特率 */
typedef enum
{
    LORA_UART_BAUDRATE_1200 = 1200,
    LORA_UART_BAUDRATE_2400 = 2400,
    LORA_UART_BAUDRATE_4800 = 4800,
    LORA_UART_BAUDRATE_9600 = 9600,
    LORA_UART_BAUDRATE_19200 = 19200,
    LORA_UART_BAUDRATE_38400 = 38400,
    LORA_UART_BAUDRATE_57600 = 57600,
    LORA_UART_BAUDRATE_115200 = 115200,
} lora_uart_baudrate_t;//限定范围1200-115200

/* 串口数据位 */
typedef enum
{
    LORA_UART_DATABITS_8 = 8,
} lora_uart_databits_t;

/* 串口停止位 */
typedef enum
{
    LORA_UART_STOPBITS_1 = 1,
    LORA_UART_STOPBITS_2 = 2,
} lora_uart_stopbits_t;

/* 串口校验位 */
#define LORA_UART_PARITY_NONE "NONE"
#define LORA_UART_PARITY_EVEN "EVEN"
#define LORA_UART_PARITY_ODD "ODD"

/* 串口流控 */
#define LORA_UART_FLOWCTRL_NONE "NFC"
#define LORA_UART_FLOWCTRL_485 "485"

/* 休眠模式 */
#define LORA_PMODE_RUN "RUN"//运行模式
#define LORA_PMODE_WU "WU"//唤醒模式
#define LORA_PMODE_LR "LR"//低功耗接收模式
#define LORA_PMODE_LSR "LSR"//低功耗发送接收模式

/* 唤醒间隔 */
typedef enum
{
    LORA_WU_INTERVAL_500MS = 500,
    LORA_WU_INTERVAL_1000MS = 1000,
    LORA_WU_INTERVAL_1500MS = 1500,
    LORA_WU_INTERVAL_2000MS = 2000,
    LORA_WU_INTERVAL_2500MS = 2500,
    LORA_WU_INTERVAL_3000MS = 3000,
    LORA_WU_INTERVAL_3500MS = 3500,
    LORA_WU_INTERVAL_4000MS = 4000,
} lora_wtm_t;

/* 空中速率 */
typedef enum
{
    LORA_SPD_268BPS = 1,
    LORA_SPD_488BPS = 2,
    LORA_SPD_537BPS = 3,
    LORA_SPD_878BPS = 4,
    LORA_SPD_977BPS = 5,
    LORA_SPD_1758BPS = 6,
    LORA_SPD_3125BPS = 7,
    LORA_SPD_6250BPS = 8,
    LORA_SPD_10937BPS = 9,
    LORA_SPD_21875BPS = 10,
} lora_spd_t;

/* at_cmd_tools初始化 */
void lora_at_fun_init(void);
at_cmd_status_t lora_at_mode_entry(void);
at_cmd_status_t lora_at_mode_exit(void);
at_cmd_status_t lora_at_save(void);
at_cmd_status_t lora_at_default(void);
at_cmd_status_t lora_at_restart(void);
/* 查询命令 */
at_cmd_status_t lora_at_get_echo(lora_switch_t *echo);
at_cmd_status_t lora_at_get_wmode(lora_wmode_t *wmode);
at_cmd_status_t lora_at_get_uart(lora_uart_baudrate_t *baudrate, lora_uart_databits_t *databits, lora_uart_stopbits_t *stopbits, char *parity, char *flowctrl);
at_cmd_status_t lora_at_get_pmode(char *pmode);
at_cmd_status_t lora_at_get_wtm(lora_wtm_t *wtm);
at_cmd_status_t lora_at_get_spd(lora_spd_t *spd);
at_cmd_status_t lora_at_get_addr(char *addr);
at_cmd_status_t lora_at_get_pwr(char *pwr);
at_cmd_status_t lora_at_get_rto(char *rto);
at_cmd_status_t lora_at_get_pflag(lora_switch_t *pflag);
at_cmd_status_t lora_at_get_sendok(lora_switch_t *sflag);
at_cmd_status_t lora_at_get_idle_time(uint8_t *idletime);
at_cmd_status_t lora_at_get_ch(char *ch);
at_cmd_status_t lora_at_get_fec(lora_switch_t *fec);
/* 设置命令 */
at_cmd_status_t lora_at_set_wmode(lora_wmode_t wmode);
at_cmd_status_t lora_at_set_uart(lora_uart_baudrate_t baudrate, lora_uart_databits_t databits, lora_uart_stopbits_t stopbits, char *parity, char *flowctrl);
at_cmd_status_t lora_at_set_pmode(char *pmode);
at_cmd_status_t lora_at_set_idle_time(uint8_t idletime);
at_cmd_status_t lora_at_set_wtm(lora_wtm_t wtm);
at_cmd_status_t lora_at_set_spd(lora_spd_t spd);
at_cmd_status_t lora_at_set_addr(uint16_t addr);
at_cmd_status_t lora_at_set_ch(char *ch);
at_cmd_status_t lora_at_set_pwr(char *pwr);
at_cmd_status_t lora_at_set_rto(char *rto);
at_cmd_status_t lora_at_open_sqt(void);
at_cmd_status_t lora_at_set_sqt_time(char *rssi_interval);
at_cmd_status_t lora_at_set_key(char *key);
at_cmd_status_t lora_at_set_pflag(lora_switch_t sta);
at_cmd_status_t lora_at_set_sendok(lora_switch_t sta);
at_cmd_status_t lora_at_set_fec(lora_switch_t sta);

#endif /* _LORA_AT_FUN_H__ */
