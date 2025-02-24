#include "stdlib.h"
#include "string.h"
#include "usart.h"
#include "ble_at_fun.h"

#define BLE_ACK_OK "OK"

at_device_t ble_at_dev;

/**
 * @brief  BLE AT初始化
*/
void ble_at_init(void)
{
    ble_at_dev.at_id = 0x03U;
    ble_at_dev.at_cmd_pprintf = BLE_printf;
    ble_at_dev.at_ack_restart = BLE_rx_reset;
    ble_at_dev.at_delay_ms = sys_delay_ms;
    ble_at_dev.at_cmd_ack = BLE_rx_get_buf;
}

/* 基础命令 */

/**
 * @brief  BLE AT设备恢复出厂设置
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+DEFAULT
*/
at_cmd_status_t ble_at_default(void)
{
    return at_sp_cmd_send(&ble_at_dev, "AT+DEFAULT", BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  BLE AT设备重启
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+RESET
*/
at_cmd_status_t ble_at_reset(void)
{
    return at_sp_cmd_send(&ble_at_dev, "AT+RESET", BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  BLE AT设备未连线状态下清除最后记忆的从机地址
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+CLEAR
*/
at_cmd_status_t ble_at_clear(void)
{
    return at_sp_cmd_send(&ble_at_dev, "AT+CLEAR", BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Test AT command
 * @retval AT_CMD_OK: AT command OK
 * @retval AT_CMD_TIMEOUT: AT command timeout
 * @note   AT
*/
at_cmd_status_t ble_at_test(void)
{
    return at_sp_cmd_send(&ble_at_dev, "AT", BLE_ACK_OK, 500);
}


/* 查询命令 */

/**
 * @brief  Get AT 波特率和校验位
 * @param  baudrate: 波特率
 * @param  parity: 校验位
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BAUD=?
*/
at_cmd_status_t ble_at_get_baud(ble_uart_baudrate_t *baudrate, char *parity)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char _baudrate[6] = {0};

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BAUD", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    
    memcpy(_baudrate, ble_at_dev.at_cmd_ack() + param_index, param_len);
    _baudrate[param_len] = '\0';
    switch (atoi(_baudrate))
    {
    case 1200:
        *baudrate = BLE_BAUDRATE_1200;
        break;
    case 2400:
        *baudrate = BLE_BAUDRATE_2400;
        break;
    case 4800:
        *baudrate = BLE_BAUDRATE_4800;
        break;
    case 9600:
        *baudrate = BLE_BAUDRATE_9600;
        break;
    case 19200:
        *baudrate = BLE_BAUDRATE_19200;
        break;
    case 38400:
        *baudrate = BLE_BAUDRATE_38400;
        break;
    case 57600:
        *baudrate = BLE_BAUDRATE_57600;
        break;
    case 115200:
        *baudrate = BLE_BAUDRATE_115200;
        break;
    case 230400:
        *baudrate = BLE_BAUDRATE_230400;
        break;
    case 460800:
        *baudrate = BLE_BAUDRATE_460800;
        break;
    case 921600:
        *baudrate = BLE_BAUDRATE_921600;
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 2, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(parity, ble_at_dev.at_cmd_ack() + param_index, param_len);
    parity[param_len] = '\0';

    return AT_CMD_OK;

}

/**
 * @brief  Get AT 设备静默工作模式
 * @param  switch_t: 静默工作模式
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BTMODE=?
*/
at_cmd_status_t ble_at_get_btmode(ble_switch_t *switch_t)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char _switch[2] = {0};

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BTMODE", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    
    memcpy(_switch, ble_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    switch (atoi(_switch))
    {
    case 0:
        *switch_t = BLE_OFF;
        break;
    case 1:
        *switch_t = BLE_ON;
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备角色
 * @param  role: 角色
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+ROLE=?
 * @note   默认从机,设置后模块将自动重启,重启250ms后可再进行新的操作!
*/
at_cmd_status_t ble_at_get_role(char *role)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "ROLE", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    
    memcpy(role, ble_at_dev.at_cmd_ack() + param_index, param_len);
    role[param_len] = '\0';

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备蓝牙名称
 * @param  name: 蓝牙名称,最大长度为16
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+NAME=?
*/
at_cmd_status_t ble_at_get_name(char *name)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "NAME", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    if(param_len > 16)
    {
        return AT_CMD_PARMINVAL;
    }
    memcpy(name, ble_at_dev.at_cmd_ack() + param_index, param_len);
    name[param_len] = '\0';

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备PIN码
 * @param  pin: PIN码,最大长度为16
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+PIN=?
*/
at_cmd_status_t ble_at_get_pin(char *pin)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "PIN", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    if(param_len > 16)
    {
        return AT_CMD_PARMINVAL;
    }
    memcpy(pin, ble_at_dev.at_cmd_ack() + param_index, param_len);
    pin[param_len] = '\0';

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备mac地址
 * @param  addr: mac地址,16进制大写字符,仅修改后10位,前两位固定04
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+ADDR=?
*/
at_cmd_status_t ble_at_get_addr(char *addr)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "ADDR", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    memcpy(addr, ble_at_dev.at_cmd_ack() + param_index, param_len);
    addr[param_len] = '\0';

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备蓝牙简单安全配对
 * @param  switch_t: 简单安全配对,0:关闭,配队需要密码,1:开启,配对不需要密码
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+SSP=?
*/
at_cmd_status_t ble_at_get_ssp(ble_switch_t *switch_t)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char _switch[2] = {0};

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "SSP", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    
    memcpy(_switch, ble_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    switch (atoi(_switch))
    {
    case 1:
        *switch_t = BLE_OFF;
        break;
    case 0:
        *switch_t = BLE_ON;
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备BLE模式下是否广播
 * @param  switch_t: 广播,0:关闭,1:开启
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BLE=?
*/
at_cmd_status_t ble_at_get_ble(ble_switch_t *switch_t)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;
    char _switch[2] = {0};

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BLE", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    
    memcpy(_switch, ble_at_dev.at_cmd_ack() + param_index, param_len);
    _switch[param_len] = '\0';
    switch (atoi(_switch))
    {
    case 0:
        *switch_t = BLE_OFF;
        break;
    case 1:
        *switch_t = BLE_ON;
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备BLE蓝牙名称
 * @param  name: 蓝牙名称,最大长度为14
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BNAME=?
 */
at_cmd_status_t ble_at_get_bname(char *name)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BNAME", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    if(param_len > 14)
    {
        return AT_CMD_PARMINVAL;
    }
    memcpy(name, ble_at_dev.at_cmd_ack() + param_index, param_len);
    name[param_len] = '\0';

    return AT_CMD_OK;
}

/**
 * @brief  Get AT 设备BLE蓝牙地址
 * @param  addr: 蓝牙地址,最大长度为10,16进制大写字符,前两位固定C4
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BADDR=?
 */
at_cmd_status_t ble_at_get_baddr(char *addr)
{
    char cmd[10] = {0};
    uint16_t param_index = 0;
    uint16_t param_len = 0;

    at_cmd_status_t ret = at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BADDR", "?"), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }

    ret = at_ack_get_normal_parameter(ble_at_dev.at_cmd_ack(), 1, &param_index, &param_len);
    if (ret != AT_CMD_OK)
    {
        return ret;
    }
    if(param_len > 10)
    {
        return AT_CMD_PARMINVAL;
    }
    memcpy(addr, ble_at_dev.at_cmd_ack() + param_index, param_len);
    addr[param_len] = '\0';

    return AT_CMD_OK;
}

/* 设置命令 */

/**
 * @brief  Set AT 波特率和校验位
 * @param  baudrate: 波特率
 * @param  parity: 校验位, NULL表示保持原有校验位
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BAUD=baudrate,parity
 */
at_cmd_status_t ble_at_set_baud(ble_uart_baudrate_t baudrate, char *parity)
{
    char cmd[20] = {0};
    char baud[6] = {0};
    char param[8] = {0};
    switch (baudrate)
    {
    case BLE_BAUDRATE_1200:
        strcpy(baud, "1200");
        break;
    case BLE_BAUDRATE_2400:
        strcpy(baud, "2400");
        break;
    case BLE_BAUDRATE_4800:
        strcpy(baud, "4800");
        break;
    case BLE_BAUDRATE_9600:
        strcpy(baud, "9600");
        break;
    case BLE_BAUDRATE_19200:
        strcpy(baud, "19200");
        break;
    case BLE_BAUDRATE_38400:
        strcpy(baud, "38400");
        break;
    case BLE_BAUDRATE_57600:
        strcpy(baud, "57600");
        break;
    case BLE_BAUDRATE_115200:
        strcpy(baud, "115200");
        break;
    case BLE_BAUDRATE_230400:
        strcpy(baud, "230400");
        break;
    case BLE_BAUDRATE_460800:
        strcpy(baud, "460800");
        break;
    case BLE_BAUDRATE_921600:
        strcpy(baud, "921600");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    if(parity == NULL)
    {
        return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BAUD", baud), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    }
    else
    {
        sprintf(param, "%s,%s", baud, parity);
        return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BAUD", param), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
    }
    
}

/**
 * @brief  Set AT 设备静默工作模式
 * @param  switch_t: 静默工作模式
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BTMODE=switch_t
*/
at_cmd_status_t ble_at_set_btmode(ble_switch_t switch_t)
{
    char cmd[10] = {0};
    char param[2] = {0};
    switch (switch_t)
    {
    case BLE_OFF:
        strcpy(param, "0");
        break;
    case BLE_ON:
        strcpy(param, "1");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BTMODE", param), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备角色
 * @param  role: 角色
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+ROLE=role
*/
at_cmd_status_t ble_at_set_role(char *role)
{
    char cmd[12] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "ROLE", role), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备蓝牙名称,设置完自动重启
 * @param  name: 蓝牙名称,最大长度为16
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+NAME=name
*/
at_cmd_status_t ble_at_set_name(char *name)
{
    if(strlen(name) > 16)
    {
        return AT_CMD_PARMINVAL;
    }
    char cmd[30] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "NAME", name), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备PIN码
 * @param  pin: PIN码,最大长度为16
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+PIN=pin
*/
at_cmd_status_t ble_at_set_pin(char *pin)
{
    if(strlen(pin) > 16)
    {
        return AT_CMD_PARMINVAL;
    }
    char cmd[30] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "PIN", pin), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备mac地址
 * @param  addr: mac地址,16进制大写字符,仅修改后10位,前两位固定04
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+ADDR=addr
*/
at_cmd_status_t ble_at_set_addr(char *addr)
{
    if(strlen(addr) != 10)
    {
        return AT_CMD_PARMINVAL;
    }
    for(uint8_t i = 0; i < 10; i++)
    {
        if((addr[i] < '0' || addr[i] > '9') && (addr[i] < 'A' || addr[i] > 'F'))
        {
            return AT_CMD_PARMINVAL;
        }
    }

    char cmd[30] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "ADDR", addr), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备蓝牙简单安全配对
 * @param  switch_t: 简单安全配对,0:关闭,配队需要密码,1:开启,配对不需要密码
*/
at_cmd_status_t ble_at_set_ssp(ble_switch_t *switch_t)
{
    char cmd[10] = {0};
    char param[2] = {0};
    switch (*switch_t)
    {
    case BLE_OFF:
        strcpy(param, "1");
        break;
    case BLE_ON:
        strcpy(param, "0");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }
    printf("param:\n");
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "SSP", param), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备BLE模式下是否广播
 * @param  switch_t: 广播,0:关闭,1:开启
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BLE=switch_t
*/
at_cmd_status_t ble_at_set_ble(ble_switch_t *switch_t)
{
    char cmd[10] = {0};
    char param[2] = {0};
    switch (*switch_t)
    {
    case BLE_OFF:
        strcpy(param, "0");
        break;
    case BLE_ON:
        strcpy(param, "1");
        break;
    default:
        return AT_CMD_PARMINVAL;
    }

    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BLE", param), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备BLE蓝牙名称
 * @param  name: 蓝牙名称,最大长度为14
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BNAME=name
*/
at_cmd_status_t ble_at_set_bname(char *name)
{
    if(strlen(name) > 14)
    {
        return AT_CMD_PARMINVAL;
    }
    char cmd[30] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BNAME", name), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}

/**
 * @brief  Set AT 设备BLE蓝牙地址
 * @param  addr: 蓝牙地址,最大长度为10,16进制大写字符,前两位固定C4
 * @retval AT_CMD_OK: AT command OK
 * @note   AT+BADDR=addr
*/
at_cmd_status_t ble_at_set_baddr(char *addr)
{
    if(strlen(addr) != 10)
    {
        return AT_CMD_PARMINVAL;
    }
    for(uint8_t i = 0; i < 10; i++)
    {
        if((addr[i] < '0' || addr[i] > '9') && (addr[i] < 'A' || addr[i] > 'F'))
        {
            return AT_CMD_PARMINVAL;
        }
    }

    char cmd[30] = {0};
    return at_sp_cmd_send(&ble_at_dev, at_cmd_pack(cmd, "BADDR", addr), BLE_ACK_OK, AT_WAIT_ACK_TIMEOUT);
}
