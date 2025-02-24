#include "main.h"
#include "at_cmd_tools.h"
#include "usart.h"
#include "sys_delay.h"
#include "string.h"
#include "e52_lora_at_driver.h"

at_device_t e52_lora_at_dev;

void e52_lora_at_driver_init(void)
{
    e52_lora_at_dev.at_id = 0;
    e52_lora_at_dev.at_cmd_pprintf = e52_lora_printf;
    e52_lora_at_dev.at_ack_restart = e52_lora_rx_reset;
    e52_lora_at_dev.at_delay_ms = sys_delay_ms;
    e52_lora_at_dev.at_cmd_ack = e52_lora_rx_buf;
    e52_lora_at_dev.at_hex_printf_cmd = e52_lora_hex_printf;

}

/**
 * @brief  复位e52-lora
 * @note   AT+RESET
 */
at_cmd_status_t e52_lora_at_reset(void)
{
    return at_sp_cmd_send(&e52_lora_at_dev, "AT+RESET", "OK", AT_WAIT_ACK_TIMEOUT);
}

/* 查询命令 */

/**
 * @brief  获取e52-lora的射频功率
 * @param  transmit_power: 射频功率,-9~22dBm
 * @note   AT+POWER?
 * @todo  参数显示形式为字符或其他仍需确定
 */
at_cmd_status_t e52_lora_at_query_transmit_power(int8_t *transmit_power)
{
    at_cmd_status_t ret;
    uint16_t param_index;
    uint16_t param_len;
    int8_t _transmit_power[2];

    if(transmit_power == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+POWER=?", "OK", AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(e52_lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    memcpy(_transmit_power, e52_lora_at_dev.at_cmd_ack() + param_index, param_len);
    _transmit_power[param_len] = '\0';
    printf("transmit power: %s\r\n", _transmit_power);

    return ret;
}

/**
 * @brief  获取e52-lora的信道
 * @param  channel: 信道0~99(400NW),0~79(900NW)
 * @note   AT+CH?
 */
at_cmd_status_t e52_lora_at_query_channel(uint8_t *channel)
{
    at_cmd_status_t ret;
    uint16_t param_index;
    uint16_t param_len;
    uint8_t _channel[2];

    if(channel == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+CH=?", "OK", AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(e52_lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    memcpy(_channel, e52_lora_at_dev.at_cmd_ack() + param_index, param_len);
    _channel[param_len] = '\0';
    printf("channel: %s\r\n", _channel);

    return ret;
}

/**
 * @brief  获取e52-lora的ACK时间
 * @param  ack_time: ACK时间,1000~65535ms
 * @note   AT+ACK_TIME=?
 */
at_cmd_status_t e52_lora_at_query_ack_time(uint16_t *ack_time)
{
    at_cmd_status_t ret;
    uint16_t param_index;
    uint16_t param_len;
    uint8_t _ack_time[5];

    if(ack_time == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+ACK_TIME=?", "OK", AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(e52_lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    memcpy(_ack_time, e52_lora_at_dev.at_cmd_ack() + param_index, param_len);
    _ack_time[param_len] = '\0';
    printf("ack time: %s\r\n", _ack_time);

    return ret;
}

/**
 * @brief  获取e52-lora的重建路由时间
 * @param  router_time: 重建路由时间,1000~65535ms
 * @note   AT+ROUTER_TIME=?
 */
at_cmd_status_t e52_lora_at_query_router_time(uint16_t *router_time)
{
    at_cmd_status_t ret;
    uint16_t param_index;
    uint16_t param_len;
    uint8_t _router_time[5];

    if(router_time == NULL)
    {
        return AT_CMD_PARMINVAL;
    }

    ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+ROUTER_TIME=?", "OK", AT_WAIT_ACK_TIMEOUT);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(e52_lora_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if(ret != AT_CMD_OK)
    {
        return ret;
    }

    memcpy(_router_time, e52_lora_at_dev.at_cmd_ack() + param_index, param_len);
    _router_time[param_len] = '\0';
    printf("router time: %s\r\n", _router_time);

    return ret;
}

/**
 * @brief  获取e52-lora的信息
 * @note   AT+INFO=?
 */
at_cmd_status_t e52_lora_at_info_view(void)
{
    at_cmd_status_t ret;
    ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+INFO=?", "DEVTYPE", AT_WAIT_ACK_TIMEOUT);
    //printf("major info: %s\r\n", e52_lora_at_dev.at_cmd_ack());
    // ret = at_sp_cmd_send(&e52_lora_at_dev, "AT+INFO=0", "DEVID", AT_WAIT_ACK_TIMEOUT);
    // printf("high info: %s\r\n", e52_lora_at_dev.at_cmd_ack());
    e52_lora_at_dev.at_ack_restart();
    return ret;
}

/* 查询命令 */

/* 设置命令 */

/**
 * @brief  设置e52-lora的射频功率
 * @param  transmit_power: 射频功率,-9~22dBm,默认22dBm
 * @note   AT+POWER=
 */
at_cmd_status_t e52_lora_at_set_transmit_power(int8_t transmit_power, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[15];
    char param[5];
    if(transmit_power < -9 || transmit_power > 22)
    {
        transmit_power = 22;
    }
    sprintf(param, "%d,%d", transmit_power, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "POWER", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的信道
 * @param  channel: 信道,0~255,默认60
 * @note   AT+CHANNEL=channel,save
 * @note   重启后生效
 */
at_cmd_status_t e52_lora_at_set_channel(uint8_t channel, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[18];
    char param[5];
    if(channel >= 255)
    {
        channel = 60;
    }
    sprintf(param, "%d,%d", channel, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "CHANNEL", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的串口
 * @param  baud: 波特率,默认115200
 * @param  partiy: 校验位,默认8N1
 * @note   AT+UART=baud,partiy
 * @note   重启后生效
 */
at_cmd_status_t e52_lora_at_set_uart(e52_lora_at_uart_baud_t baud, e52_lora_at_uart_partiy_t partiy)
{
    at_cmd_status_t ret;
    char cmd[20];
    char param[10];
    sprintf(param, "%d,%d", baud, partiy);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "UART", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的空中速率
 * @param  rate: 空中速率,默认62.5K
 * @note   AT+RATE=rate
 */
at_cmd_status_t e52_lora_at_set_rate(e52_lora_at_air_rate_t rate)
{
    at_cmd_status_t ret;
    char cmd[10];
    char param[5];
    if(rate > E52_LORA_RATE_7K)
    {
        rate = E52_LORA_RATE_62_5K;
    }
    sprintf(param, "%d", rate);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "RATE", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的选项
 * @param  option: 选项,默认单播
 * @param  save: 是否保存到flash
 * @note   AT+OPTION=option,save
 */
at_cmd_status_t e52_lora_at_set_option(e52_lora_at_option_t option, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[10];
    char param[5];
    if(option < E52_LORA_OPTION_UNICAST || option > E52_LORA_OPTION_ANYCAST)
    {
        option = E52_LORA_OPTION_UNICAST;
    }
    sprintf(param, "%d,%d", option, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "OPTION", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的PANID
 * @param  panid: PANID,0~65534
 * @param  save: 是否保存到flash
 * @note   AT+PANID=panid,save
 */
at_cmd_status_t e52_lora_at_set_panid(uint16_t panid, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[15];
    char param[10];
    sprintf(param, "%d,%d", panid, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "PANID", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的类型
 * @param  type: 类型,0为路由节点,1为终端设备,默认0
 * @note   AT+TYPE=type
 */
at_cmd_status_t e52_lora_at_set_type(e52_lora_at_type_t type)
{
    at_cmd_status_t ret;
    char cmd[10];
    char param[5];
    if(type > E52_LORA_TYPE_END_DEVICE)
    {
        type = E52_LORA_TYPE_ROUTERS;
    }
    sprintf(param, "%d", type);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "TYPE", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的源地址
 * @param  src_addr: 源地址,路由节点0x0000~0x7FFF,终端设备0x8000~0xFFFE
 * @param  save: 是否保存到flash
 * @note   AT+SRC_ADDR=src_addr,save
 */
at_cmd_status_t e52_lora_at_set_src_addr(uint16_t src_addr, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[24];
    char param[12];
    sprintf(param, "%d,%d", src_addr, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "SRC_ADDR", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的目的地址
 * @param  dst_addr: 目的地址,0x0000~0xFFFE
 * @param  save: 是否保存到flash
 * @note   AT+DST_ADDR=dst_addr,save
 */
at_cmd_status_t e52_lora_at_set_dst_addr(uint16_t dst_addr, e52_lora_at_save_t save)
{
    at_cmd_status_t ret;
    char cmd[24];
    char param[12];
    sprintf(param, "%d,%d", dst_addr, save);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "DST_ADDR", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的头部
 * @param  enable: 是否启用,默认启用
 * @note   AT+HEAD=enable
 */
at_cmd_status_t e52_lora_at_set_head(e52_lora_at_functional_t enable)
{
    at_cmd_status_t ret;
    char cmd[10];
    char param[5];
    if(enable > E52_LORA_FUNCTIONAL_ENABLEMENT)
    {
        enable = E52_LORA_FUNCTIONAL_ENABLEMENT;
    }
    sprintf(param, "%d", enable);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "HEAD", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的回传
 * @param  enable: 是否启用,默认启用
 * @note   AT+BACK=enable
 */
at_cmd_status_t e52_lora_at_set_back(e52_lora_at_functional_t enable)
{
    at_cmd_status_t ret;
    char cmd[10];
    char param[5];
    if(enable > E52_LORA_FUNCTIONAL_ENABLEMENT)
    {
        enable = E52_LORA_FUNCTIONAL_ENABLEMENT;
    }
    sprintf(param, "%d", enable);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "BACK", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的安全
 * @param  enable: 是否启用
 * @note   AT+SECURITY=enable
 */
at_cmd_status_t e52_lora_at_set_security(e52_lora_at_functional_t enable)
{
    at_cmd_status_t ret;
    char cmd[14];
    char param[5];
    sprintf(param, "%d", enable);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "SECURITY", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的重启时间
 * @param  min: 重启时间,0~255min,默认5分钟,0为不重启
 * @note   AT+RESET_TIME=min
 */
at_cmd_status_t e52_lora_at_set_reset_time(uint8_t min)
{
    at_cmd_status_t ret;
    char cmd[14];
    char param[5];
    if(min >= 255)
    {
        min = 5;
    }
    sprintf(param, "%d", min);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "RESET_TIME", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的响应时间
 * @param  ack_time: 响应时间,1000~65535ms,默认2500ms
 * @note   AT+ACK_TIME=ack_time
 */
at_cmd_status_t e52_lora_at_set_ack_time(uint16_t ack_time)
{
    at_cmd_status_t ret;
    char cmd[14];
    char param[5];
    if(ack_time < 1000 || ack_time > 65535)
    {
        ack_time = 2500;
    }
    sprintf(param, "%d", ack_time);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "ACK_TIME", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的重建路由时间
 * @param  router_time: 重建路由时间,1000~65535ms,默认2500ms
 * @note   AT+ROUTER_TIME=router_time
 */
at_cmd_status_t e52_lora_at_set_router_time(uint16_t router_time)
{
    at_cmd_status_t ret;
    char cmd[18];
    char param[5];
    if(router_time < 1000 || router_time > 65535)
    {
        router_time = 2500;
    }
    sprintf(param, "%d", router_time);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "ROUTER_TIME", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  添加e52-lora的组地址
 * @param  group_addr: 组地址
 * @note   AT+GROUP_ADD=group_addr
 */
at_cmd_status_t e52_lora_at_set_group_add(uint16_t group_addr)
{
    at_cmd_status_t ret;
    char cmd[18];
    char param[5];
    sprintf(param, "%d", group_addr);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "GROUP_ADD", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  删除e52-lora的组地址
 * @param  group_addr: 组地址
 * @note   AT+GROUP_DEL=group_addr
 */
at_cmd_status_t e52_lora_at_set_group_del(uint16_t group_addr)
{
    at_cmd_status_t ret;
    char cmd[18];
    char param[5];
    sprintf(param, "%d", group_addr);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "GROUP_DEL", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  删除e52-lora的所有组地址
 * @note   AT+GROUP_CLR
 */
at_cmd_status_t e52_lora_at_set_group_clr(void)
{
    at_cmd_status_t ret;
    char cmd[15];

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "GROUP_CLR", "1"), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  清除e52-lora的路由表
 * @note   AT+ROUTER_CLR
 */
at_cmd_status_t e52_lora_at_set_router_clr(void)
{
    at_cmd_status_t ret;
    char cmd[18];

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "ROUTER_CLR", "1"), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/**
 * @brief  设置e52-lora的路由表是否保存到flash
 * @param  enable: 是否保存到flash
 * @note   AT+ROUTER_SAVE=enable
 */
at_cmd_status_t e52_lora_at_set_router_save(e52_lora_at_functional_t enable)
{
    at_cmd_status_t ret;
    char cmd[18];
    char param[5];
    sprintf(param, "%d", enable);

    ret = at_sp_cmd_send(&e52_lora_at_dev, at_cmd_pack(cmd, "ROUTER_SAVE", param), "OK", AT_WAIT_ACK_TIMEOUT);

    return ret;
}

/* 额外解析函数 */

/**
 * @brief  解析e52-lora的LORA MSG,AT+HEAD=E52_LORA_FUNCTIONAL_ENABLEMENT时有效
 * @param  data: 需解析的数据
 * @param  len: 数据长度
 */
e52_lora_msg_t e52_lora_msg_analyse(uint8_t *data, uint16_t len)
{
    //e52_lora_msg_t *head = (e52_lora_msg_t *)data;
    e52_lora_msg_t head;
    memcpy(&head, data, len);
    head.data_len -= 2;//减去自定义前置包信息
    printf("head type: %#02X\r\n", head.head_type);
    printf("data len: %d\r\n", head.data_len);
    printf("panid: %#02X\r\n", head.panid);
    printf("src addr: %#02X\r\n", head.src_addr);
    printf("dst addr: %#02X\r\n", head.dst_addr);
    printf("pack type: %#02X\r\n", head.pack_type);
    printf("pack num: %#02X\r\n", head.pack_num);
    printf("data: ");
    for(uint16_t i = 0; i < head.data_len; i++)
    {
        printf("%#02X ", head.data[i]);
    }
    printf("\r\n");
    return head;
}

/**
 * @brief  解析e52-lora的ack, AT+BACK=E52_LORA_FUNCTIONAL_ENABLEMENT时有效
 * @param  data: 需解析的数据
 * @param  len: 数据长度
 * @note   0: SUCCESS
 * @note   1: NO ACK
 * @note   2: NO ROUTE
 * @note   3: OUT OF CACHE
 * @note   4: LORA MSG
 * @note   5: PACK RECEIVE
 * @note   6: PACK RESEND
 * @note   7: PACK FAIL
 */
uint8_t e52_lora_ack_analyse(uint8_t *data, uint16_t len)
{
    char *ack[] = {E52_LORA_MSG_ACK_SUCCESS, E52_LORA_MSG_ACK_NO_ACK, E52_LORA_MSG_ACK_NO_ROUTE, E52_LORA_MSG_ACK_OUT_OF_CACHE, E52_LORA_MSG_ACK_LORA_MSG, E52_LORA_MSG_ACK_PACK_RECEIVE, E52_LORA_MSG_ACK_PACK_RESEND, E52_LORA_MSG_ACK_PACK_FAIL};

    for (uint8_t i = 0; i < 7; i++)
    {   
        if(i == 4)
        {
            continue;
        }
        size_t ack_len = strlen(ack[i]);
        for (uint16_t pos = 0; pos <= len - ack_len; pos++)
        {
            if (memcmp(data + pos, ack[i], ack_len) == 0)
            {
                printf("ack: %s\r\n", ack[i]);
                // if (i == 4)
                //     i += 1;
                return i;
            }
        }
    }
    return 4;
}
