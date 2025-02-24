#ifndef __NET_AT_FUN_H__
#define __NET_AT_FUN_H__

extern at_device_t net_at_dev;

/* 基础开关 */
typedef enum
{
    NET_ON = 0x00U,
    NET_OFF = 0x01U,
} net_switch_t;

/* 串口波特率 */
typedef enum
{
    NET_UART_BAUDRATE_1200 = 1200,
    NET_UART_BAUDRATE_2400 = 2400,
    NET_UART_BAUDRATE_4800 = 4800,
    NET_UART_BAUDRATE_9600 = 9600,
    NET_UART_BAUDRATE_19200 = 19200,
    NET_UART_BAUDRATE_38400 = 38400,
    NET_UART_BAUDRATE_57600 = 57600,
    NET_UART_BAUDRATE_115200 = 115200,
    NET_UART_BAUDRATE_230400 = 230400,
    NET_UART_BAUDRATE_460800 = 460800,
    NET_UART_BAUDRATE_921600 = 921600,
} net_uart_baudrate_t;//限定范围300-921600,如需其他波特率请自行添加

/* 串口数据位 */
typedef enum
{
    NET_UART_DATABITS_7 = 7,
    NET_UART_DATABITS_8 = 8,
} net_uart_databits_t;

/* 串口停止位 */
typedef enum
{
    NET_UART_STOPBITS_1 = 1,
    NET_UART_STOPBITS_2 = 2,
} net_uart_stopbits_t;

/* 串口校验位 */
//进口芯片通用
//国产芯片8位数据位时支持NONE,EVEN,ODD
//国产芯片7位数据位时支持EVEN,ODD,MARK,SPACE
#define NET_UART_PARITY_NONE "NONE"
#define NET_UART_PARITY_EVEN "EVEN"
#define NET_UART_PARITY_ODD "ODD"
#define NET_UART_PARITY_MARK "MARK"
#define NET_UART_PARITY_SPACE "SPACE"

/* 串口流控 未使用，已弃用 */
#define NET_UART_FLOWCTRL_NONE "NFC"
#define NET_UART_FLOWCTRL_FCR "FCR"

/* 基础命令函数 */
void net_at_fun_init(void);
at_cmd_status_t net_at_mode_entry(void);
at_cmd_status_t net_at_mode_exit(void);
at_cmd_status_t net_at_save(void);
at_cmd_status_t net_at_default(void);
at_cmd_status_t net_at_restore(void);
at_cmd_status_t net_at_restart(void);

/* 查询命令函数 */

at_cmd_status_t net_at_get_echo(net_switch_t *echo);
at_cmd_status_t net_at_get_uart(net_uart_baudrate_t *baudrate, net_uart_databits_t *databits, net_uart_stopbits_t *stopbits, char *parity);
at_cmd_status_t net_at_get_uarttl(uint8_t *pack_len, uint16_t *pack_time);
at_cmd_status_t net_at_get_uartclbuf(net_switch_t *switch_status);
at_cmd_status_t net_at_get_wann(char *ipmode, char *ipaddr, char *mask, char *gateway);
at_cmd_status_t net_at_get_webu(char *username, char *password);
at_cmd_status_t net_at_get_sock(char *work_mode, char *ipaddr, char *prot);
at_cmd_status_t net_at_get_socklk(net_switch_t *switch_status);
at_cmd_status_t net_at_get_sockport(char *server_port, char *client_port);
at_cmd_status_t net_at_get_dhcpen(net_switch_t *switch_t);
at_cmd_status_t net_at_get_dnsmode(char *dns_mode);
at_cmd_status_t net_at_get_dns(char *addr);
at_cmd_status_t net_at_get_webport(char *port);
at_cmd_status_t net_at_get_search(char *port, char *atcmd);
at_cmd_status_t net_at_get_tcpreip(char *ipaddr);
at_cmd_status_t net_at_get_maxsk(uint8_t *maxsk);
at_cmd_status_t net_at_get_ping(char *addr, char *result);
at_cmd_status_t net_at_get_mid(char *mid);
at_cmd_status_t net_at_get_rstim(char *time);
at_cmd_status_t net_at_get_clientrst(net_switch_t *switch_t);
at_cmd_status_t net_at_get_uartset(net_switch_t *switch_t);
at_cmd_status_t net_at_get_strson(char *strson);

/* 设置命令函数 */

at_cmd_status_t net_at_set_echo(net_switch_t echo);
at_cmd_status_t net_at_set_uart(net_uart_baudrate_t baudrate, net_uart_databits_t databits, net_uart_stopbits_t stopbits, char *parity);
at_cmd_status_t net_at_set_uarttl(uint8_t pack_len, uint16_t pack_time);
at_cmd_status_t net_at_set_uartclbuf(net_switch_t switch_status);
at_cmd_status_t net_at_set_wann(char *ipmode, char *ipaddr, char *mask, char *gateway);
at_cmd_status_t net_at_set_webu(char *username, char *password);
at_cmd_status_t net_at_set_sock(char *work_mode, char *ipaddr, char *prot);
at_cmd_status_t net_at_set_sockport(char *server_port, char *client_port);
at_cmd_status_t net_at_set_dhcpen(net_switch_t switch_t);
at_cmd_status_t net_at_set_dnsmode(char *dns_mode);
at_cmd_status_t net_at_set_dns(char *addr);
at_cmd_status_t net_at_set_webport(char *port);
at_cmd_status_t net_at_set_search(char *port, char *atcmd);
at_cmd_status_t net_at_set_maxsk(char *maxsk);
at_cmd_status_t net_at_set_mid(char *mid);
at_cmd_status_t net_at_set_rstim(uint16_t time);
at_cmd_status_t net_at_set_clientrst(net_switch_t switch_t);
at_cmd_status_t net_at_set_uartset(net_switch_t switch_t);
at_cmd_status_t net_at_set_strson(net_switch_t *strson);

#endif /* _NET_AT_FUN_H__ */
