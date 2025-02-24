#include "main.h"
#include "stdlib.h"
#include "usart.h"
#include "string.h"
#include "lora_at_fun.h"

#define LORA_ACK_OK "OK"

/* at_cmd_tools初始化 */

at_device_t lora_at_dev;

/**
 * @brief     LoRa AT设备初始化
*/
void lora_at_fun_init(void)
{
    lora_at_dev.at_id = 0x02;
    lora_at_dev.at_cmd_pprintf = LORA_printf;
    lora_at_dev.at_ack_restart = LORA_rx_reset;
    lora_at_dev.at_delay_ms = sys_delay_ms;
    lora_at_dev.at_cmd_ack = LORA_rx_get_buf;
}

/* 基础命令 */

/**
 * @brief  有人网at设备进入AT模式
 * @note   +++
 */
at_cmd_status_t lora_at_mode_entry(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    lora_at_dev.at_ack_restart();
    lora_at_dev.at_delay_ms(100);
    ret = at_sp_cmd_send(&lora_at_dev, "+++", "a", AT_WAIT_ACK_TIMEOUT);
    ret = at_sp_cmd_send(&lora_at_dev, "a", "+OK", AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  有人网at设备退出AT模式，进入透传模式
 * @note   AT+ENTM
 */
at_cmd_status_t lora_at_mode_exit(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    
    ret = at_cmd_send(&lora_at_dev, "AT+ENTM", LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  有人网at设备将当前配置参数作为为用户默认出厂配置
 * @note   AT+CFGTF
 */
at_cmd_status_t lora_at_save(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    ret = at_cmd_send(&lora_at_dev, "AT+CFGTF", LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  有人网at设备恢复为用户默认出厂配置
 * @note   AT+RELD
*/
at_cmd_status_t lora_at_default(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    ret = at_cmd_send(&lora_at_dev, "AT+RELD", LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    return ret;
}

/**
 * @brief  重启有人网at设备
 * @note   AT+Z
 */
at_cmd_status_t lora_at_restart(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[5];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "Z", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/* 查询命令 */

/**
 * @brief  获取有人网at设备的回显状态
 * @param  echo: 回显状态
 * @note   AT+E
 */
at_cmd_status_t lora_at_get_echo(lora_switch_t *echo)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[5];
    uint8_t _echo[4];

    if(echo == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "E", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_echo, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _echo[param_len] = '\0';

    if(strcmp((char *)_echo, "ON") == 0)
    {
        *echo = LORA_ON;
    }
    else if(strcmp((char *)_echo, "OFF") == 0)
    {
        *echo = LORA_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的工作模式
 * @param  wmode: 工作模式 TRANS:透传模式 FP:定点模式
 * @note   AT+WMODE
 */
at_cmd_status_t lora_at_get_wmode(lora_wmode_t *wmode)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _wmode[4];

    if(wmode == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "WMODE", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_wmode, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _wmode[param_len] = '\0';

    if(strcmp((char *)_wmode, "TRANS") == 0)
    {
        *wmode = LORA_MODE_TRANS;
    }
    else if(strcmp((char *)_wmode, "FP") == 0)
    {
        *wmode = LORA_MODE_FP;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的串口配置
 * @param  baudrate: 波特率 databits: 数据位 stopbits: 停止位 parity: 校验位 flow_ctrl: 流控
 * @note   AT+UART
 */
at_cmd_status_t lora_at_get_uart(lora_uart_baudrate_t *baudrate, lora_uart_databits_t *databits, lora_uart_stopbits_t *stopbits, char *parity, char *flow_ctrl)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _baudrate[10];
    uint8_t _databits[4];
    uint8_t _stopbits[4];
    uint8_t _parity[10];
    uint8_t _flow_ctrl[10];

    if(baudrate == NULL || databits == NULL || stopbits == NULL || parity == NULL || flow_ctrl == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "UART", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_baudrate, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _baudrate[param_len] = '\0';
    switch(atoi((char *)_baudrate))
    {
        case 1200:
            *baudrate = LORA_UART_BAUDRATE_1200;
            break;
        case 2400:
            *baudrate = LORA_UART_BAUDRATE_2400;
            break;
        case 4800:
            *baudrate = LORA_UART_BAUDRATE_4800;
            break;
        case 9600:
            *baudrate = LORA_UART_BAUDRATE_9600;
            break;
        case 19200:
            *baudrate = LORA_UART_BAUDRATE_19200;
            break;
        case 38400:
            *baudrate = LORA_UART_BAUDRATE_38400;
            break;
        case 57600:
            *baudrate = LORA_UART_BAUDRATE_57600;
            break;
        case 115200:
            *baudrate = LORA_UART_BAUDRATE_115200;
            break;
        default:
            return AT_CMD_ERROR;
    }

    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_databits, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _databits[param_len] = '\0';
    switch(atoi((char *)_databits))
    {
        case 8:
            *databits = LORA_UART_DATABITS_8;
            break;
        default:
            return AT_CMD_ERROR;
    }

    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 3, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_stopbits, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _stopbits[param_len] = '\0';
    switch(atoi((char *)_stopbits))
    {
        case 1:
            *stopbits = LORA_UART_STOPBITS_1;
            break;
        case 2:
            *stopbits = LORA_UART_STOPBITS_2;
            break;
        default:
            return AT_CMD_ERROR;
    }

    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 4, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_parity, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _parity[param_len] = '\0';
    strcpy(parity, (char *)_parity);

    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 5, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_flow_ctrl, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _flow_ctrl[param_len] = '\0';
    strcpy(flow_ctrl, (char *)_flow_ctrl);

    return ret;
}

/**
 * @brief  获取有人网at设备的休眠模式
 * @param  pmode 
 * @note    AT+PMODE
*/
at_cmd_status_t lora_at_get_pmode(char *pmode)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _pmode[10];

    if(pmode == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PMODE", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_pmode, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _pmode[param_len] = '\0';
    strcpy(pmode, (char *)_pmode);

    return ret;
}

/**
 * @brief  获取有人网at设备的唤醒间隔
 * @param  wtm: 唤醒间隔
 * @note   AT+WTM
 */
at_cmd_status_t lora_at_get_wtm(lora_wtm_t *wtm)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];
    uint8_t _wtm[4];

    if(wtm == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "WTM", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_wtm, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _wtm[param_len] = '\0';
    switch(atoi((char *)_wtm))
    {
        case 500:
            *wtm = LORA_WU_INTERVAL_500MS;
            break;
        case 1000:
            *wtm = LORA_WU_INTERVAL_1000MS;
            break;
        case 1500:
            *wtm = LORA_WU_INTERVAL_1500MS;
            break;
        case 2000:
            *wtm = LORA_WU_INTERVAL_2000MS;
            break;
        case 2500:
            *wtm = LORA_WU_INTERVAL_2500MS;
            break;
        case 3000:
            *wtm = LORA_WU_INTERVAL_3000MS;
            break;
        case 3500:
            *wtm = LORA_WU_INTERVAL_3500MS;
            break;
        case 4000:
            *wtm = LORA_WU_INTERVAL_4000MS;
            break;
        default:
            return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的空中速率
 * @param  spd: 空中速率
 * @note   AT+SPD
 */
at_cmd_status_t lora_at_get_spd(lora_spd_t *spd)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];
    uint8_t _spd[4];

    if(spd == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "SPD", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_spd, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _spd[param_len] = '\0';
    switch(atoi((char *)_spd))
    {
        case 1:
            *spd = LORA_SPD_268BPS;
            break;
        case 2:
            *spd = LORA_SPD_488BPS;
            break;
        case 3:
            *spd = LORA_SPD_537BPS;
            break;
        case 4:
            *spd = LORA_SPD_878BPS;
            break;
        case 5:
            *spd = LORA_SPD_977BPS;
            break;
        case 6:
            *spd = LORA_SPD_1758BPS;
            break;
        case 7:
            *spd = LORA_SPD_3125BPS;
            break;
        case 8:
            *spd = LORA_SPD_6250BPS;
            break;
        case 9:
            *spd = LORA_SPD_10937BPS;
            break;
        case 10:
            *spd = LORA_SPD_21875BPS;
            break;
        default:
            return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的目标地址
 * @param  addr: 目标地址,0~65535
 * @note   AT+ADDR
 */
at_cmd_status_t lora_at_get_addr(char *addr)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[9];

    if(addr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "ADDR", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(addr, lora_at_dev.at_cmd_ack() + param_index, param_len);
    addr[param_len] = '\0';

    return ret;
}

/**
 * @brief  获取有人网at设备的发射功率
 * @param  pwr: 发射功率,10~20dBm.默认20dBm
 * @note   AT+PWR
 */
at_cmd_status_t lora_at_get_pwr(char *pwr)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];

    if(pwr == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PWR", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(pwr, lora_at_dev.at_cmd_ack() + param_index, param_len);
    pwr[param_len] = '\0';

    return ret;
}

/**
 * @brief  获取有人网at设备的接收超时时间
 * @param  rto: 接收超时时间,0~15000ms,默认500ms
 * @note   AT+RTO
 * @note   仅LR/LSR模式下有效。LSR模式下如果该值设置为0则模块发送数据后不开启接收。
 */
at_cmd_status_t lora_at_get_rto(char *rto)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];

    if(rto == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "RTO", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(rto, lora_at_dev.at_cmd_ack() + param_index, param_len);
    rto[param_len] = '\0';

    return ret;
}

/**
 * @brief  获取有人网at设备的快速进入低功耗使能标志
 * @param  pflag: 快速进入低功耗使能标志,1:使能 0:禁止
 * @note   AT+PFLAG
 */
at_cmd_status_t lora_at_get_pflag(lora_switch_t *pflag)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _pflag[4];

    if(pflag == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PFLAG", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_pflag, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _pflag[param_len] = '\0';
    if(strcmp((char *)_pflag, "1") == 0)
    {
        *pflag = LORA_ON;
    }
    else if(strcmp((char *)_pflag, "0") == 0)
    {
        *pflag = LORA_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/**
 * @brief  获取有人网at设备的发送完成回复标志
 * @param  sflag: 发送完成回复标志,1:使能 0:禁止
 * @note   AT+SENDOK
 */
at_cmd_status_t lora_at_get_sendok(lora_switch_t *sflag)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[10];
    uint8_t _sflag[4];

    if(sflag == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "SENDOK", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_sflag, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _sflag[param_len] = '\0';
    if(strcmp((char *)_sflag, "1") == 0)
    {
        *sflag = LORA_ON;
    }
    else if(strcmp((char *)_sflag, "0") == 0)
    {
        *sflag = LORA_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }
    
    return ret;
}

/**
 * @brief  获取有人网at设备的空闲时间
 * @param  idle_time: 空闲时间，3~240单位秒
 * @note   AT+ITM
 */
at_cmd_status_t lora_at_get_idle_time(uint8_t *idle_time)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];
    uint8_t _idle_time[4];

    if(idle_time == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "ITM", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_idle_time, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _idle_time[param_len] = '\0';
    *idle_time = atoi((char *)_idle_time);

    return ret;
}

/**
 * @brief  获取有人网at设备的信道
 * @param  ch: 信道,0~127,默认72,对应470MHz
 * @note   AT+CH
 */
at_cmd_status_t lora_at_get_ch(char *ch)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[7];

    if(ch == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "CH", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(ch, lora_at_dev.at_cmd_ack() + param_index, param_len);
    ch[param_len] = '\0';

    return ret;
}

/**
 * @brief  获取有人网at设备是否使能前向纠错
 * @param  fec: 前向纠错状态
 * @note   AT+FEC
 */
at_cmd_status_t lora_at_get_fec(lora_switch_t *fec)
{
    at_cmd_status_t ret = AT_CMD_OK;
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char cmd[8];
    uint8_t _fec[4];

    if(fec == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "FEC", NULL), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    ret = at_ack_get_normal_parameter(lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(_fec, lora_at_dev.at_cmd_ack() + param_index, param_len);
    _fec[param_len] = '\0';
    if(strcmp((char *)_fec, "ON") == 0)
    {
        *fec = LORA_ON;
    }
    else if(strcmp((char *)_fec, "OFF") == 0)
    {
        *fec = LORA_OFF;
    }
    else
    {
        return AT_CMD_ERROR;
    }

    return ret;
}

/* 设置命令 */

/**
 * @brief  设置有人网at设备的工作模式
 * @param  wmode LORA_MODE_TRANS:透传模式 LORA_MODE_FP:定点模式
 * @note   AT+WMODE=wmode
 * @retval AT_CMD_OK: 成功
*/
at_cmd_status_t lora_at_set_wmode(lora_wmode_t wmode)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[14];

    switch (wmode)
    {
    case LORA_MODE_TRANS:
        ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "WMODE", "TRANS"), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        break;
    case LORA_MODE_FP:
        ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "WMODE", "FP"), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        break;
    default:
        ret = AT_CMD_PARMINVAL;
        break;
    }

    return ret;
}

/**
 * @brief  设置有人网at设备的串口配置
 * @param  baudrate 波特率 
 * @param  databits 数据位 
 * @param  stopbits 停止位
 * @param  parity 校验位
 * @param  flow_ctrl 流控
 * @note   AT+UART=baudrate,databits,stopbits,parity,flow_ctrl
 */
at_cmd_status_t lora_at_set_uart(lora_uart_baudrate_t baudrate, lora_uart_databits_t databits, lora_uart_stopbits_t stopbits, char *parity, char *flow_ctrl)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[40];
    char cmd_para[20];

    sprintf(cmd_para, "%d,%d,%d,%s,%s", baudrate, databits, stopbits, parity, flow_ctrl);
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "UART", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的休眠模式
 * @param  pmode 休眠模式
 * @note   AT+PMODE=pmode
*/
at_cmd_status_t lora_at_set_pmode(char *pmode)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[15];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PMODE", pmode), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的空闲时间
 * @param  idle_time 空闲时间，3~240单位秒
 * @note   AT+ITM=idle_time
 */
at_cmd_status_t lora_at_set_idle_time(uint8_t idle_time)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[11];
    char cmd_para[3];

    sprintf(cmd_para, "%d", idle_time);
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "ITM", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的唤醒间隔
 * @param  wtm 唤醒间隔
 * @note   AT+WTM=wtm
 */
at_cmd_status_t lora_at_set_wtm(lora_wtm_t wtm)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[11];
    char cmd_para[4];

    sprintf(cmd_para, "%d", wtm);
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "WTM", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的空中速率
 * @param  spd 空中速率
 * @note   AT+SPD=spd
 */
at_cmd_status_t lora_at_set_spd(lora_spd_t spd)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[11];
    char cmd_para[4];

    sprintf(cmd_para, "%d", spd);
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "SPD", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的目标地址
 * @param  addr 目标地址,0~65535,十进制
 * @note   AT+ADDR=addr
 */
at_cmd_status_t lora_at_set_addr(uint16_t addr)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[15];
    char cmd_para[6];
    sprintf(cmd_para, "%d", addr);
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "ADDR", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的信道
 * @param  ch 信道,0~127,默认72,对应470MHz
 * @note   AT+CH=ch
 */
at_cmd_status_t lora_at_set_ch(char *ch)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];
    
    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "CH", ch), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备是否使能前向纠错
 * @param  fec 前向纠错状态
 * @note   AT+FEC=fec
 */
at_cmd_status_t lora_at_set_fec(lora_switch_t fec)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];

    switch (fec)
    {
    case LORA_ON:
        ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "FEC", "ON"), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        break;
    case LORA_OFF:
        ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "FEC", "OFF"), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);
        break;
    default:
        ret = AT_CMD_PARMINVAL;
        break;
    }

    return ret;
}

/**
 * @brief  设置有人网at设备的发射功率
 * @param  pwr 发射功率,10~20dBm.默认20dBm
 * @note   AT+PWR=pwr
 */
at_cmd_status_t lora_at_set_pwr(char *pwr)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[10];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PWR", pwr), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的接收超时时间
 * @param  rto 接收超时时间,0~15000ms,默认500ms
 * @note   AT+RTO=rto
 * @note   仅LR/LSR模式下有效。LSR模式下如果该值设置为0则模块发送数据后不开启接收。
 */
at_cmd_status_t lora_at_set_rto(char *rto)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[15];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "RTO", rto), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  开启有人网at设备的信号强度显示
 * @note   AT+SQT
 */
at_cmd_status_t lora_at_open_sqt(void)
{
    at_cmd_status_t ret = AT_CMD_OK;
    
    ret = at_cmd_send(&lora_at_dev, "AT+SQT", LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的信号强度数据包时间间隔
 * @param  rssi_interval 信号强度数据包时间间隔,100~60000ms
 * @note   AT+RSSI=rssi_interval
 */
at_cmd_status_t lora_at_set_sqt_time(char *rssi_interval)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[15];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "SQT", rssi_interval), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的加密字
 * @param  key 加密字,16字节HEX字符串
 * @note   AT+KEY=30313233343536373839414243444546
 * @note   不可查询
 */
at_cmd_status_t lora_at_set_key(char *key)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[40];

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "KEY", key), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置有人网at设备的快速进入低功耗使能标志
 * @param  sta 1:使能 0:禁止
 * @note    AT+QLE=sta
 */
at_cmd_status_t lora_at_set_pflag(lora_switch_t sta)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[12];
    char cmd_para[2];

    switch (sta)
    {
    case LORA_ON:
        strcpy(cmd_para, "1");
        break;
    case LORA_OFF:
        strcpy(cmd_para, "0");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "PFLAG", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief 设置有人网at设备的发送完成回复标志
 * @param  sta 1:使能 0:禁止
 * @note   AT+SENDOK=sta
 */
at_cmd_status_t lora_at_set_sendok(lora_switch_t sta)
{
    at_cmd_status_t ret = AT_CMD_OK;
    char cmd[12];
    char cmd_para[2];

    switch (sta)
    {
    case LORA_ON:
        strcpy(cmd_para, "1");
        break;
    case LORA_OFF:
        strcpy(cmd_para, "0");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    ret = at_cmd_send(&lora_at_dev, at_cmd_pack(cmd, "SENDOK", cmd_para), LORA_ACK_OK, AT_WAIT_ACK_TIMEOUT);

    return ret;
}
