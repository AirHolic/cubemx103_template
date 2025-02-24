#include "main.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include "net_at_fun.h"

#define NET_ACK_OK "+OK"

/* at_cmd_tools初始化 */

at_device_t net_at_dev;

/**
 * @brief  有人网at设备初始化
*/
void net_at_fun_init(void)
{
    net_at_dev.at_id = 0x01;
    net_at_dev.at_cmd_pprintf = NET_printf;
    net_at_dev.at_ack_restart = NET_rx_reset;
    net_at_dev.at_delay_ms = sys_delay_ms;
    net_at_dev.at_cmd_ack = NET_rx_get_buf;
}

/* 基础命令 */

/**
 * @brief  有人网at设备进入AT模式
 * @note   +++
 */
at_cmd_status_t net_at_mode_entry(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    net_at_dev.at_ack_restart();
    net_at_dev.at_delay_ms(100);
    ret = at_sp_cmd_send(&net_at_dev, "+++", "a", AT_WAIT_ACK_TIMEOUT);
    ret += at_sp_cmd_send(&net_at_dev, "a", NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  有人网at设备退出AT模式，进入透传模式
 * @note   AT+ENTM
 */
at_cmd_status_t net_at_mode_exit(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    
    ret = at_cmd_send(&net_at_dev, "AT+ENTM", NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  有人网at设备将当前配置参数作为为用户默认出厂配置
 * @note   AT+CFGTF
 */
at_cmd_status_t net_at_save(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    ret = at_cmd_send(&net_at_dev, "AT+CFGTF", NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  有人网at设备恢复为用户默认出厂配置
 * @note   AT+RELD
*/
at_cmd_status_t net_at_default(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    ret = at_cmd_send(&net_at_dev, "AT+RELD", NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  有人网at设备从出厂参数区恢复参数
 * @note   AT+CLEAR
 */
at_cmd_status_t net_at_restore(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    ret = at_cmd_send(&net_at_dev, "AT+CLEAR", NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  重启有人网at设备
 * @note   AT+Z
 */
at_cmd_status_t net_at_restart(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "Z", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/* 查询命令 */

/**
 * @brief  获取有人网at设备的回显状态
 * @param  echo: 回显状态
 * @note   AT+E
 */
at_cmd_status_t net_at_get_echo(net_switch_t *echo)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _echo[4];

    if(echo == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "E", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_echo, net_at_dev.at_cmd_ack() + param_index, param_len);
    _echo[param_len] = '\0';

    if(strcmp((char *)_echo, "ON") == 0)
    {
        *echo = NET_ON;
    }
    else if(strcmp((char *)_echo, "OFF") == 0)
    {
        *echo = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的串口参数
 * @param  baudrate: 波特率
 * @param  databit: 数据位
 * @param  stopbit: 停止位
 * @param  paritybit: 校验位
 * @param  flowctrl: 流控
 * @note   AT+UART
*/
at_cmd_status_t net_at_get_uart(net_uart_baudrate_t *baudrate, net_uart_databits_t *databit, net_uart_stopbits_t *stopbit, char *paritybit)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _baudrate[7];
    uint8_t _databit[2];
    uint8_t _stopbit[2];
    uint8_t _paritybit[6];

    if(baudrate == NULL || databit == NULL || stopbit == NULL || paritybit == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UART", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_baudrate, net_at_dev.at_cmd_ack() + param_index, param_len);
    _baudrate[param_len] = '\0';
    switch(atoi((char *)_baudrate))
    {
        case 1200:
            *baudrate = NET_UART_BAUDRATE_1200;
            break;
        case 2400:
            *baudrate = NET_UART_BAUDRATE_2400;
            break;
        case 4800:
            *baudrate = NET_UART_BAUDRATE_4800;
            break;
        case 9600:
            *baudrate = NET_UART_BAUDRATE_9600;
            break;
        case 19200:
            *baudrate = NET_UART_BAUDRATE_19200;
            break;
        case 38400:
            *baudrate = NET_UART_BAUDRATE_38400;
            break;
        case 57600:
            *baudrate = NET_UART_BAUDRATE_57600;
            break;
        case 115200:
            *baudrate = NET_UART_BAUDRATE_115200;
            break;
        case 230400:
            *baudrate = NET_UART_BAUDRATE_230400;
            break;
        case 460800:
            *baudrate = NET_UART_BAUDRATE_460800;
            break;
        case 921600:
            *baudrate = NET_UART_BAUDRATE_921600;
            break;
        default:
            return AT_CMD_ERROR;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_databit, net_at_dev.at_cmd_ack() + param_index, param_len);
    _databit[param_len] = '\0';
    switch(atoi((char *)_databit))
    {
        case 7:
            *databit = NET_UART_DATABITS_7;
            break;
        case 8:
            *databit = NET_UART_DATABITS_8;
            break;
        default:
            return AT_CMD_ERROR;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 3, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_stopbit, net_at_dev.at_cmd_ack() + param_index, param_len);
    _stopbit[param_len] = '\0';
    switch(atoi((char *)_stopbit))
    {
        case 1:
            *stopbit = NET_UART_STOPBITS_1;
            break;
        case 2:
            *stopbit = NET_UART_STOPBITS_2;
            break;
        default:
            return AT_CMD_ERROR;
    }
    
    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 4, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_paritybit, net_at_dev.at_cmd_ack() + param_index, param_len);
    _paritybit[param_len] = '\0';
    strcpy(paritybit, (char *)_paritybit);

    return ret;
}

/**
 * @brief  获取有人网at设备的透传包长度和超时时间
 * @param  pack_len: 透传包长度,范围1-1024
 * @param  pack_time: 透传超时时间,范围0-255
 * @note   AT+UARTTL
 */
at_cmd_status_t net_at_get_uarttl(uint8_t *pack_len, uint16_t *pack_time)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[15];
    uint8_t _pack_len[4];
    uint8_t _pack_time[5];

    if(pack_len == NULL || pack_time == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTTL", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_pack_time, net_at_dev.at_cmd_ack() + param_index, param_len);
    _pack_time[param_len] = '\0';
    *pack_time = atoi((char *)_pack_time);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_pack_len, net_at_dev.at_cmd_ack() + param_index, param_len);
    _pack_len[param_len] = '\0';
    *pack_len = atoi((char *)_pack_len);

    return ret;
}

/**
 * @brief  获取有人网at设备连接前是否清空串口缓存
 * @param  switch_status: 缓存清空状态
 * @note   AT+UARTCLBUF
 */
at_cmd_status_t net_at_get_uartclbuf(net_switch_t *switch_status)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[15];
    uint8_t _switch[4];

    if(switch_status == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTCLBUF", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_switch, net_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    if(strcmp((char *)_switch, "ON") == 0)
    {
        *switch_status = NET_ON;
    }
    else if(strcmp((char *)_switch, "OFF") == 0)
    {
        *switch_status = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的WANN参数
 * @param  ipmode: IP模式, STATIC / DHCP
 * @param  ipaddr: IP地址
 * @param  mask: 子网掩码
 * @param  gateway: 网关
 * @note   AT+WANN
*/
at_cmd_status_t net_at_get_wann(char *ipmode, char *ipaddr, char *mask, char *gateway)
{
    if(ipmode == NULL || ipaddr == NULL || mask == NULL || gateway == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _ipmode[6];
    uint8_t _ipaddr[16];
    uint8_t _mask[16];
    uint8_t _gateway[16];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WANN", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_ipmode, net_at_dev.at_cmd_ack() + param_index, param_len);
    _ipmode[param_len] = '\0';
    strcpy(ipmode, (char *)_ipmode);
    
    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_ipaddr, net_at_dev.at_cmd_ack() + param_index, param_len);
    _ipaddr[param_len] = '\0';
    strcpy(ipaddr, (char *)_ipaddr);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 3, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_mask, net_at_dev.at_cmd_ack() + param_index, param_len);
    _mask[param_len] = '\0';
    strcpy(mask, (char *)_mask);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 4, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_gateway, net_at_dev.at_cmd_ack() + param_index, param_len);
    _gateway[param_len] = '\0';
    strcpy(gateway, (char *)_gateway);

    return ret;
}

/**
 * @brief 获取有人网at设备的WEBU参数
 * @param  username: 用户名，1-16个字符,不为空
 * @param  password: 密码，1-16个字符,不为空
 * @note   AT+WEBU
 */
at_cmd_status_t net_at_get_webu(char *username, char *password)
{
    if(username == NULL || password == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _username[16];
    uint8_t _password[16];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WEBU", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_username, net_at_dev.at_cmd_ack() + param_index, param_len);
    _username[param_len] = '\0';
    strcpy(username, (char *)_username);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_password, net_at_dev.at_cmd_ack() + param_index, param_len);
    _password[param_len] = '\0';
    strcpy(password, (char *)_password);

    return ret;
}

/**
 * @brief  获取有人网at设备的SOCK参数
 * @param  work_mode:工作模式:TCPS对应TCPServer
 *                           TCPC对应TCPClient
 *                           UDPS对应UDPServer
 *                           UDPC对应UDPClient
 * @param  ipaddr: IP地址,client模式下为服务器地址,server模式下为本地地址
 * @param  prot: 协议端口,十进制,1-65535,0表示随机端口
 * @note   AT+SOCK
 */
at_cmd_status_t net_at_get_sock(char *work_mode, char *ipaddr, char *prot)
{
    if(work_mode == NULL || ipaddr == NULL || prot == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _work_mode[5];
    uint8_t _ipaddr[16];
    uint8_t _prot[6];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SOCK", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_work_mode, net_at_dev.at_cmd_ack() + param_index, param_len);
    _work_mode[param_len] = '\0';
    strcpy(work_mode, (char *)_work_mode);
    
    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_ipaddr, net_at_dev.at_cmd_ack() + param_index, param_len);
    _ipaddr[param_len] = '\0';
    strcpy(ipaddr, (char *)_ipaddr);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 3, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_prot, net_at_dev.at_cmd_ack() + param_index, param_len);
    _prot[param_len] = '\0';
    strcpy(prot, (char *)_prot);

    return ret;
}

/**
 * @brief  获取有人网at设备的socket连接状态
 * @param  switch_status: 连接锁状态,ON / OFF
 * @note   AT+SOCKLK
 */
at_cmd_status_t net_at_get_socklk(net_switch_t *switch_status)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char __switch[12];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SOCKLK", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(__switch, net_at_dev.at_cmd_ack() + param_index, param_len);
    __switch[param_len] = '\0';
    if(strcmp(__switch, "connect") == 0)
    {
        *switch_status = NET_ON;
    }
    else if(strcmp(__switch, "disconnect") == 0)
    {
        *switch_status = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的socket本地端口和远程端口
 * @param  server_port: 作为client时需要连接的服务器端口,范围1-65535
 * @param  client_port: 作为client时本地端口，范围1-65535
 * @note   AT+SOCKPORT
*/
at_cmd_status_t net_at_get_sockport(char *server_port, char *client_port)
{
    if(server_port == NULL || client_port == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _server_port[6];
    uint8_t _client_port[6];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SOCKPORT", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_server_port, net_at_dev.at_cmd_ack() + param_index, param_len);
    _server_port[param_len] = '\0';
    strcpy(server_port, (char *)_server_port);
    
    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_client_port, net_at_dev.at_cmd_ack() + param_index, param_len);
    _client_port[param_len] = '\0';
    strcpy(client_port, (char *)_client_port);

    return ret;
}

/**
 * @brief  获取有人网at设备的DHCP使能状态
 * @param  switch_t: DHCP使能状态,ON / OFF
 * @note   AT+DHCPEN
 */
at_cmd_status_t net_at_get_dhcpen(net_switch_t *switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char __switch[4];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DHCPEN", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(__switch, net_at_dev.at_cmd_ack() + param_index, param_len);
    __switch[param_len] = '\0';
    if(strcmp(__switch, "ON") == 0)
    {
        *switch_t = NET_ON;
    }
    else if(strcmp(__switch, "OFF") == 0)
    {
        *switch_t = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief 获取有人网at设备的DNS获取方式
 * @param  dns_mode: DNS获取方式,AUTO / MANUAL
 * @note   AT+DNSMODE
*/
at_cmd_status_t net_at_get_dnsmode(char *dns_mode)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _dns_mode[6];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DNSMODE", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_dns_mode, net_at_dev.at_cmd_ack() + param_index, param_len);
    _dns_mode[param_len] = '\0';
    strcpy(dns_mode, (char *)_dns_mode);

    return ret;
}

/**
 * @brief  获取有人网at设备的DNS服务器地址
 * @param  addr: DNS服务器地址
 * @note   AT+DNS
 */
at_cmd_status_t net_at_get_dns(char *addr)
{
    if(addr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _addr[16];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DNS", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_addr, net_at_dev.at_cmd_ack() + param_index, param_len);
    _addr[param_len] = '\0';
    strcpy(addr, (char *)_addr);

    return ret;
}

/**
 * @brief  获取有人网at设备的内置WEB端口
 * @param  port: WEB端口,范围1-65535,默认80
 * @note   AT+WEBPORT
 */
at_cmd_status_t net_at_get_webport(char *port)
{
    if(port == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _port[6];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WEBPORT", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_port, net_at_dev.at_cmd_ack() + param_index, param_len);
    _port[param_len] = '\0';
    strcpy(port, (char *)_port);

    return ret;
}

/**
 * @brief  获取有人网at设备的at搜索指令和端口
 * @param  port: 端口，范围0-65535
 * @param  atcmd: 搜索指令，1-32个字符
 * @note   AT+SEARCH
*/
at_cmd_status_t net_at_get_search(char *port, char *atcmd)
{
    if(port == NULL || atcmd == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _port[6];
    uint8_t _atcmd[32];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SEARCH", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_port, net_at_dev.at_cmd_ack() + param_index, param_len);
    _port[param_len] = '\0';
    strcpy(port, (char *)_port);

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_atcmd, net_at_dev.at_cmd_ack() + param_index, param_len);
    _atcmd[param_len] = '\0';
    strcpy(atcmd, (char *)_atcmd);

    return ret;
}

/**
 * @brief  查询目标ip地址
 * @param  ipaddr: 目标ip地址
 * @note   AT+TCPIP
 */
at_cmd_status_t net_at_get_tcpreip(char *ipaddr)
{
    if(ipaddr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _ipaddr[16];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "TCPREIP", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_ipaddr, net_at_dev.at_cmd_ack() + param_index, param_len);
    _ipaddr[param_len] = '\0';
    strcpy(ipaddr, (char *)_ipaddr);

    return ret;
}

/**
 * @brief  查询端口最大连接数
 * @param  maxsk: 最大连接数,1-16,默认8
 * @note   AT+MAXSK
*/
at_cmd_status_t net_at_get_maxsk(uint8_t *maxsk)
{
    if(maxsk == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _maxsk[4];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "MAXSK", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_maxsk, net_at_dev.at_cmd_ack() + param_index, param_len);
    _maxsk[param_len] = '\0';
    *maxsk = atoi((char *)_maxsk);

    return ret;
}

/**
 * @brief  查询有人网at设备的ping状态
 * @param  addr: 目标ip地址,最大64个字符
 * @param  result: ping结果
 * @note   AT+PING
*/
at_cmd_status_t net_at_get_ping(char *addr, char *result)
{
    if(addr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[75];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _result[8];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "PING", addr), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_result, net_at_dev.at_cmd_ack() + param_index, param_len);
    _result[param_len] = '\0';
    strcpy(result, (char *)_result);

    return ret;
}

/**
 * @brief  查询有人网at设备的名称
 * @param  name: 设备名称,1-14个字符
 * @note   AT+NAME
*/
at_cmd_status_t net_at_get_mid(char *mid)
{
    if (mid == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _mid[16];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "MID", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_mid, net_at_dev.at_cmd_ack() + param_index, param_len);
    _mid[param_len] = '\0';
    strcpy(mid, (char *)_mid);

    return ret;
}

/**
 * @brief  查询有人网at设备的无数据重启时间
 * @param  time: 无数据重启时间,范围60-65535，0表示关闭
 * @note   AT+RSTIM
*/
at_cmd_status_t net_at_get_rstim(char *time)
{
    if(time == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _time[5];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "RSTIM", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_time, net_at_dev.at_cmd_ack() + param_index, param_len);
    _time[param_len] = '\0';
    strcpy(time, (char *)_time);

    return ret;
}

/**
 * @brief  查询有人网at设备的客户端自动重启使能状态,即无法连接tcp服务器时是否重启
 * @param  switch_t: 重启使能状态,ON / OFF
 * @note   AT+CLIENTRST
*/
at_cmd_status_t net_at_get_clientrst(net_switch_t *switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _switch[4];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "CLIENTRST", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_switch, net_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    if(strcmp((char *)_switch, "ON") == 0)
    {
        *switch_t = NET_ON;
    }
    else if(strcmp((char *)_switch, "OFF") == 0)
    {
        *switch_t = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  查询有人网at设备的串口参数配置使能状态
 * @param  switch_t: 串口设置状态,ON / OFF
 * @note   AT+UARTSET
*/
at_cmd_status_t net_at_get_uartset(net_switch_t *switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _switch[4];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTSET", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_switch, net_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    if(strcmp((char *)_switch, "ON") == 0)
    {
        *switch_t = NET_ON;
    }
    else if(strcmp((char *)_switch, "OFF") == 0)
    {
        *switch_t = NET_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  查询有人网at设备的重启原因
 * @param  reason: 重启原因 HARD / SOFT
 * @note   AT+RST
*/
at_cmd_status_t net_at_get_strson(char *strson)
{
    if(strson == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    uint8_t _strson[4];

    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "STRSON", NULL), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(net_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_strson, net_at_dev.at_cmd_ack() + param_index, param_len);
    _strson[param_len] = '\0';
    strcpy(strson, (char *)_strson);

    return ret;
}

/* 设置命令 */

/**
 * @brief  设置有人网at设备的回显状态
 * @param  echo: 回显状态
 * @note   AT+E=ON / AT+E=OFF
 */
at_cmd_status_t net_at_set_echo(net_switch_t echo)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];

    switch(echo)
    {
        case NET_ON:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "E", "ON"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        case NET_OFF:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "E", "OFF"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        default:
            ret = AT_CMD_PARMINVAL;
            break;
    }

    return ret;
}

/**
 * @brief  设置有人网at设备的串口参数
 * @param  baudrate: 波特率
 * @param  databit: 数据位
 * @param  stopbit: 停止位
 * @param  paritybit: 校验位
 * @note   AT+UART1=波特率,数据位,停止位,校验位,流控
 */
at_cmd_status_t net_at_set_uart(net_uart_baudrate_t baudrate, net_uart_databits_t databit, net_uart_stopbits_t stopbit, char *paritybit)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[40];
    char cmd_para[20];

    sprintf(cmd_para, "%d,%d,%d,%s", baudrate, databit, stopbit, paritybit);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UART", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    
    return ret;
}

/**
 * @brief  设置有人网at设备的透传包长度和超时时间
 * @param  pack_len: 透传包长度,范围1-1024
 * @param  pack_time: 透传超时时间,范围0-255
 * @note   AT+UARTTL=透传超时时间,透传包长度
 */
at_cmd_status_t net_at_set_uarttl(uint8_t pack_len, uint16_t pack_time)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];
    char cmd_para[10];

    sprintf(cmd_para, "%d,%d", pack_time, pack_len);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTTL", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备连接前是否清空串口缓存
 * @param  switch_status: 缓存清空状态
 * @note   AT+UARTCLBUF=ON / AT+UARTCLBUF=OFF
 */
at_cmd_status_t net_at_set_uartclbuf(net_switch_t switch_status)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];

    switch(switch_status)
    {
        case NET_ON:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTCLBUF", "ON"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        case NET_OFF:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTCLBUF", "OFF"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        default:
            ret = AT_CMD_PARMINVAL;
            break;
    }

    return ret;
}

/**
 * @brief  设置有人网at设备的WANN参数
 * @param  ipmode: IP模式, STATIC / DHCP
 * @param  ipaddr: IP地址，静态模式下不为空
 * @param  mask: 子网掩码，静态模式下不为空
 * @param  gateway: 网关，静态模式下不为空
 * @note   AT+WANN=IP模式,<IP地址,子网掩码,网关>
 */
at_cmd_status_t net_at_set_wann(char *ipmode, char *ipaddr, char *mask, char *gateway)
{
    if(ipmode == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[55];
    char cmd_para[50];
    if(strstr(ipmode, "STATIC") == NULL && strstr(ipmode, "DHCP") == NULL)
    {
        ipmode = "DHCP";
    }
    
    if(strstr(ipmode, "STATIC") != NULL)
    {
        sprintf(cmd_para, "%s,%s,%s,%s", ipmode, ipaddr, mask, gateway);
        ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WANN", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        return ret;
    }
    else if(strstr(ipmode, "DHCP") != NULL)
    {
        sprintf(cmd_para, "%s", ipmode);
        ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WANN", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        return ret;
    }
    else
    {
        return AT_CMD_PARMINVAL;
    }
}

/**
 * @brief  设置有人网at设备的WEBU参数
 * @param  username: 用户名,1-5个字符,不为空
 * @param  password: 密码，1-5个字符,不为空
 * @note   AT+WEBU=用户名,密码
 */
at_cmd_status_t net_at_set_webu(char *username, char *password)
{
    if(username == NULL || password == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[45];
    char cmd_para[35];

    sprintf(cmd_para, "%s,%s", username, password);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WEBU", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的SOCK参数
 * @param work_mode:  TCPS对应TCPServer;
 *                    TCPC对应TCPClient;
 *                    UDPS对应UDPServer;
 *                    UDPC对应UDPClient.
 * 
 * @param  ipaddr: IP地址,client模式下为服务器地址,server模式下为本地地址
 * @param  prot: 协议端口,十进制,1-65535,0表示随机端口
 * @note   AT+SOCK=工作模式,IP地址,协议端口
 */
at_cmd_status_t net_at_set_sock(char *work_mode, char *ipaddr, char *prot)
{
    if(work_mode == NULL || ipaddr == NULL || prot == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    if(strstr(work_mode, "TCPS") == NULL && strstr(work_mode, "TCPC") == NULL && strstr(work_mode, "UDPS") == NULL && strstr(work_mode, "UDPC") == NULL)
    {
        work_mode = "TCPC";
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[45];
    char cmd_para[35];

    sprintf(cmd_para, "%s,%s,%s", work_mode, ipaddr, prot);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SOCK", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的socket本地端口和远程端口
 * @param  server_port: 作为client时需要连接的服务器端口,范围1-65535
 * @param  client_port: 作为client时本地端口，范围1-65535
 * @note   AT+SOCKPORT=服务器端口,本地端口
 */
at_cmd_status_t net_at_set_sockport(char *server_port, char *client_port)
{
    if(server_port == NULL || client_port == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[30];
    char cmd_para[15];

    sprintf(cmd_para, "%s,%s", server_port, client_port);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SOCKPORTAN", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的DHCP使能状态
 * @param  switch_t: DHCP使能状态,ON / OFF
 * @note   AT+DHCPEN=DHCP使能状态
 */
at_cmd_status_t net_at_set_dhcpen(net_switch_t switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[14];
    char cmd_para[4];

    switch(switch_t)
    {
        case NET_ON:
            sprintf(cmd_para, "ON");
            break;
        case NET_OFF:
            sprintf(cmd_para, "OFF");
            break;
        default:
            return AT_CMD_PARMINVAL;
    }
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DHCPEN", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的DNS获取方式
 * @param  dns_mode: DNS获取方式,AUTO / MANUAL
 * @note   AT+DNSMODE=DNS获取方式
 */
at_cmd_status_t net_at_set_dnsmode(char *dns_mode)
{
    if(dns_mode == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];
    char cmd_para[6];

    sprintf(cmd_para, "%s", dns_mode);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DNSMODE", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的DNS服务器地址
 * @param  addr: DNS服务器地址
 * @note   AT+DNS=DNS服务器地址
 */
at_cmd_status_t net_at_set_dns(char *addr)
{
    if(addr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[30];
    char cmd_para[20];

    sprintf(cmd_para, "%s", addr);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "DNS", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的内置WEB端口
 * @param  port: WEB端口,范围1-65535,默认80
 * @note   AT+WEBPORT=WEB端口
 */
at_cmd_status_t net_at_set_webport(char *port)
{
    if(port == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[30];
    char cmd_para[6];

    sprintf(cmd_para, "%s", port);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "WEBPORT", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的at搜索指令和端口
 * @param  port: 端口，范围0-65535
 * @param  atcmd: 搜索指令，1-32个字符
 * @note   AT+SEARCH=端口,搜索指令
*/
at_cmd_status_t net_at_set_search(char *port, char *atcmd)
{
    if(port == NULL || atcmd == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[50];
    char cmd_para[40];

    sprintf(cmd_para, "%s,%s", port, atcmd);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "SEARCH", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的端口最大连接数
 * @param  maxsk: 最大连接数,1-16,默认8
 * @note   AT+MAXSK=最大连接数
 */
at_cmd_status_t net_at_set_maxsk(char *maxsk)
{
    if(maxsk == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[14];
    char cmd_para[2];

    sprintf(cmd_para, "%s", maxsk);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "MAXSK", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的名称
 * @param  name: 设备名称,1-14个字符
 * @note   AT+NAME=设备名称
 */
at_cmd_status_t net_at_set_mid(char *mid)
{
    if(mid == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[30];
    char cmd_para[20];

    sprintf(cmd_para, "%s", mid);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "MID", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的无数据重启时间
 * @param  time: 无数据重启时间,范围60-65535，0表示关闭
 * @note   AT+RSTIM=无数据重启时间
 */
at_cmd_status_t net_at_set_rstim(uint16_t time)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];
    char cmd_para[6];

    sprintf(cmd_para, "%d", time);
    ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "RSTIM", cmd_para), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的客户端自动重启使能状态,即无法连接tcp服务器时是否重启
 * @param  switch_t: 重启使能状态,ON / OFF
 * @note   AT+CLIENTRST=ON / AT+CLIENTRST=OFF
 */
at_cmd_status_t net_at_set_clientrst(net_switch_t switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];

    switch(switch_t)
    {
        case NET_ON:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "CLIENTRST", "ON"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        case NET_OFF:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "CLIENTRST", "OFF"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        default:
            ret = AT_CMD_PARMINVAL;
            break;
    }

    return ret;
}

/**
 * @brief  设置有人网at设备的串口参数配置使能状态
 * @param  switch_t: 串口设置状态,ON / OFF
 * @note   AT+UARTSET=ON / AT+UARTSET=OFF
 */
at_cmd_status_t net_at_set_uartset(net_switch_t switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];

    switch(switch_t)
    {
        case NET_ON:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTSET", "ON"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        case NET_OFF:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "UARTSET", "OFF"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        default:
            ret = AT_CMD_PARMINVAL;
            break;
    }

    return ret;
}

at_cmd_status_t net_at_set_strson(net_switch_t *switch_t)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[20];

    switch(*switch_t)
    {
        case NET_ON:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "STRSON", "ON"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        case NET_OFF:
            ret = at_cmd_send(&net_at_dev, at_cmd_pack(cmd, "STRSON", "OFF"), NET_ACK_OK, AT_WAIT_ACK_TIMEOUT);
            break;
        default:
            ret = AT_CMD_PARMINVAL;
            break;
    }

    return ret;
}
