#ifndef __BLE_AT_FUN_H__
#define __BLE_AT_FUN_H__

#include "main.h"

extern at_device_t ble_at_dev;

/**
 * @brief  BLE AT 开关
*/
typedef enum
{
    BLE_ON = 0x01U,
    BLE_OFF = 0x00U,
} ble_switch_t;

/**
 * @brief  BLE AT 波特率
 */
typedef enum
{
    BLE_BAUDRATE_1200 = 1200,
    BLE_BAUDRATE_2400 = 2400,
    BLE_BAUDRATE_4800 = 4800,
    BLE_BAUDRATE_9600 = 9600,
    BLE_BAUDRATE_19200 = 19200,
    BLE_BAUDRATE_38400 = 38400,
    BLE_BAUDRATE_57600 = 57600,
    BLE_BAUDRATE_115200 = 115200,
    BLE_BAUDRATE_230400 = 230400,
    BLE_BAUDRATE_460800 = 460800,
    BLE_BAUDRATE_921600 = 921600,
} ble_uart_baudrate_t;

/**
 * @brief  BLE AT 校验位
 */
#define BLE_UART_PARITY_NONE "N"
#define BLE_UART_PARITY_EVEN "E"
#define BLE_UART_PARITY_ODD "O"

/**
 * @brief  BLE AT 模块角色
 */
#define BLE_ROLE_SLAVE "S"
#define BLE_ROLE_SSPMASTER "M"
#define BLE_ROLE_BLEMASTER "BM"


void ble_at_init(void);
at_cmd_status_t ble_at_default(void);
at_cmd_status_t ble_at_reset(void);
at_cmd_status_t ble_at_clear(void);
at_cmd_status_t ble_at_test(void);

at_cmd_status_t ble_at_get_baud(ble_uart_baudrate_t *baudrate, char *parity);
at_cmd_status_t ble_at_get_btmode(ble_switch_t *switch_t);
at_cmd_status_t ble_at_get_role(char *role);
at_cmd_status_t ble_at_get_name(char *name);
at_cmd_status_t ble_at_get_pin(char *pin);
at_cmd_status_t ble_at_get_addr(char *addr);
at_cmd_status_t ble_at_get_ssp(ble_switch_t *switch_t);
at_cmd_status_t ble_at_get_ble(ble_switch_t *switch_t);
at_cmd_status_t ble_at_get_bname(char *name);
at_cmd_status_t ble_at_get_baddr(char *addr);

at_cmd_status_t ble_at_set_baud(ble_uart_baudrate_t baudrate, char *parity);
at_cmd_status_t ble_at_set_btmode(ble_switch_t switch_t);
at_cmd_status_t ble_at_set_role(char *role);
at_cmd_status_t ble_at_set_name(char *name);
at_cmd_status_t ble_at_set_pin(char *pin);
at_cmd_status_t ble_at_set_addr(char *addr);
at_cmd_status_t ble_at_set_ssp(ble_switch_t *switch_t);
at_cmd_status_t ble_at_set_ble(ble_switch_t *switch_t);
at_cmd_status_t ble_at_set_bname(char *name);
at_cmd_status_t ble_at_set_baddr(char *addr);

#endif /* _BLE_AT_FUN_H__ */
