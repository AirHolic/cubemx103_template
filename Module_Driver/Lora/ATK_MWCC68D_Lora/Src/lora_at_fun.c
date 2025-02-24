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
    lora_at_dev.at_cmd_pprintf = Lora_printf;
    lora_at_dev.at_ack_restart = Lora_uart_rx_reset;
    lora_at_dev.at_delay_ms = sys_delay_ms;
    lora_at_dev.at_cmd_ack = Lora_rx_get_buf;
}

/* AT命令 */

/**
 * @brief       ATK-MWCC68D模块进入配置模式
 * @param       无
 * @retval      无MD0
 */
void lora_enter_config(void)
{
   HAL_GPIO_WritePin(MD0_GPIO_Port, MD0_Pin, GPIO_PIN_SET);
}

/**
 * @brief       ATK-MWCC68D模块退出配置模式
 * @param       无
 * @retval      无
 */
void lora_exit_config(void)
{
   HAL_GPIO_WritePin(MD0_GPIO_Port, MD0_Pin, GPIO_PIN_RESET);
}

/**
 * @brief       判断ATK-MWCC68D模块是否空闲
 * @note        仅当ATK-MWCC68D模块空闲的时候，才能发送数据
 * @param       无
 * @retval      LORA_EOK  : ATK-MWCC68D模块空闲
 *              LORA_EBUSY: ATK-MWCC68D模块忙
 */
uint8_t lora_free(void)
{
    if (HAL_GPIO_ReadPin(AUX_GPIO_Port, AUX_Pin) != GPIO_PIN_RESET)
    {
        return LORA_EBUSY;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块AT指令测试
 * @param       无
 * @retval      LORA_EOK  : AT指令测试成功
 *              LORA_ERROR: AT指令测试失败
 */
uint8_t lora_at_test(void)
{
    uint8_t ret;
    uint8_t i;
    
    for (i=0; i<10; i++)
    {
        ret = at_cmd_send(&lora_at_dev, "AT", "OK", AT_WAIT_ACK_TIMEOUT);
        if (ret == LORA_EOK)
        {
            return LORA_EOK;
        }
    }
    
    return LORA_ERROR;
}

/**
 * @brief       ATK-MWCC68D模块指令回显配置
 * @param       enable: LORA_DISABLE: 关闭指令回显
 *                      LORA_ENABLE : 开启指令回显
 * @retval      LORA_EOK   : 指令回显配置成功
 *              LORA_ERROR : 指令回显配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_echo_config(lora_enable_t enable)
{
    uint8_t ret;
    char cmd[5] = {0};
    
    switch (enable)
    {
        case LORA_ENABLE:
        {
            sprintf(cmd, "ATE1");
            break;
        }
        case LORA_DISABLE:
        {
            sprintf(cmd, "ATE0");
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块软件复位
 * @param       无
 * @retval      LORA_EOK  : 软件复位成功
 *              LORA_ERROR: 软件复位失败
 */
uint8_t lora_sw_reset(void)
{
    uint8_t ret;
    
    ret = at_cmd_send(&lora_at_dev, "AT+RESET", "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块参数保存配置
 * @param       enable: LORA_DISABLE: 不保存参数
 *                      LORA_ENABLE : 保存参数
 * @retval      LORA_EOK   : 参数保存配置成功
 *              LORA_ERROR : 参数保存配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_flash_config(lora_enable_t enable)
{
    uint8_t ret;
    char cmd[11] = {0};
    
    switch (enable)
    {
        case LORA_DISABLE:
        {
            sprintf(cmd, "AT+FLASH=0");
            break;
        }
        case LORA_ENABLE:
        {
            sprintf(cmd, "AT+FLASH=1");
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块恢复出厂配置
 * @param       无
 * @retval      LORA_EOK   : 恢复出厂配置成功
 *              LORA_ERROR : 恢复出厂配置失败
 */
uint8_t lora_default(void)
{
    uint8_t ret;
    
    ret = at_cmd_send(&lora_at_dev, "AT+DEFAULT", "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块设备地址配置
 * @param       addr: 设备地址
 * @retval      LORA_EOK   : 设备地址配置成功
 *              LORA_ERROR : 设备地址配置失败
 */
uint8_t lora_addr_config(uint16_t addr)
{
    uint8_t ret;
    char cmd[14] = {0};
    
    sprintf(cmd, "AT+ADDR=%02X,%02X", (uint8_t)(addr >> 8) & 0xFF, (uint8_t)addr & 0xFF);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块发射功率配置
 * @param       tpower: LORA_TPOWER_11DBM: 11dBm
 *                      LORA_TPOWER_14DBM: 14dBm
 *                      LORA_TPOWER_17DBM: 17dBm
 *                      LORA_TPOWER_20DBM: 20dBm（默认）
 *                      LORA_TPOWER_22DBM: 22dBm
 * @retval      LORA_EOK   : 发射功率配置成功
 *              LORA_ERROR : 发射功率配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_tpower_config(lora_tpower_t tpower)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (tpower)
    {
        case LORA_TPOWER_11DBM:
        case LORA_TPOWER_14DBM:
        case LORA_TPOWER_17DBM:
        case LORA_TPOWER_20DBM:
        case LORA_TPOWER_22DBM:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+TPOWER=%d", tpower);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块工作模式配置
 * @param       workmode: LORA_WORKMODE_NORMAL  : 一般模式（默认）
 *                        LORA_WORKMODE_WAKEUP  : 唤醒模式
 *                        LORA_WORKMODE_LOWPOWER: 省电模式
 *                        LORA_WORKMODE_SIGNAL  : 信号强度模式
 *                        LORA_WORKMODE_SLEEP   : 休眠模式
 *                        LORA_WORKMODE_RELAY   : 中继模式
 * @retval      LORA_EOK   : 工作模式配置成功
 *              LORA_ERROR : 工作模式配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_workmode_config(lora_workmode_t workmode)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (workmode)
    {
        case LORA_WORKMODE_NORMAL:
        case LORA_WORKMODE_WAKEUP:
        case LORA_WORKMODE_LOWPOWER:
        case LORA_WORKMODE_SIGNAL:
        case LORA_WORKMODE_SLEEP:
        case LORA_WORKMODE_RELAY:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+CWMODE=%d", workmode);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块发送模式配置
 * @param       tmode: LORA_TMODE_TT: 透明传输（默认）
 *                     LORA_TMODE_DT: 定向传输
 * @retval      LORA_EOK   : 发送模式配置成功
 *              LORA_ERROR : 发送模式配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_tmode_config(lora_tmode_t tmode)
{
    uint8_t ret;
    char cmd[11] = {0};
    
    switch (tmode)
    {
        case LORA_TMODE_TT:
        case LORA_TMODE_DT:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+TMODE=%d", tmode);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块空中速率和信道配置
 * @param       wlrate : LORA_WLRATE_1K2 : 1.2Kbps
 *                       LORA_WLRATE_2K4 : 2.4Kbps
 *                       LORA_WLRATE_4K8 : 4.8Kbps
 *                       LORA_WLRATE_9K6 : 9.6Kbps
 *                       LORA_WLRATE_19K2: 19.2Kbps（默认）
 *                       LORA_WLRATE_38K4: 38.4Kbps
 *                       LORA_WLRATE_62K5: 62.5Kbps
 *              channel: 信道，范围0~83
 * @retval      LORA_EOK   : 空中速率和信道配置成功
 *              LORA_ERROR : 空中速率和信道配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_wlrate_channel_config(lora_wlrate_t wlrate, uint8_t channel)
{
    uint8_t ret;
    char cmd[15] = {0};
    
    switch (wlrate)
    {
        case LORA_WLRATE_1K2:
        case LORA_WLRATE_2K4:
        case LORA_WLRATE_4K8:
        case LORA_WLRATE_9K6:
        case LORA_WLRATE_19K2:
        case LORA_WLRATE_38K4:
        case LORA_WLRATE_62K5:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    if (channel > 83)
    {
        return LORA_EINVAL;
    }
    
    sprintf(cmd, "AT+WLRATE=%d,%d", channel, wlrate);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块网络地址配置
 * @param       netid: 网络地址，范围（0~255）
 * @retval      LORA_EOK   : 网络地址配置成功
 *              LORA_ERROR : 网络地址配置失败
 */
uint8_t lora_netid_config(uint8_t netid)
{
    uint8_t ret;
    char cmd[13] = {0};
    
    sprintf(cmd, "AT+NETID=%d", netid);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块休眠时间配置
 * @param       wltime: LORA_WLTIME_1S: 1秒（默认）
 *                      LORA_WLTIME_2S: 2秒
 * @retval      LORA_EOK   : 休眠时间配置成功
 *              LORA_ERROR : 休眠时间配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_wltime_config(lora_wltime_t wltime)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (wltime)
    {
        case LORA_WLTIME_1S:
        case LORA_WLTIME_2S:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+WLTIME=%d", wltime);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块数据包大小配置
 * @param       packsize: LORA_PACKSIZE_32 : 32字节
 *                        LORA_PACKSIZE_64 : 64字节
 *                        LORA_PACKSIZE_128: 128字节
 *                        LORA_PACKSIZE_240: 240字节
 * @retval      LORA_EOK   : 数据包大小配置成功
 *              LORA_ERROR : 数据包大小配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_packsize_config(lora_packsize_t packsize)
{
    uint8_t ret;
    char cmd[14] = {0};
    
    switch (packsize)
    {
        case LORA_PACKSIZE_32:
        case LORA_PACKSIZE_64:
        case LORA_PACKSIZE_128:
        case LORA_PACKSIZE_240:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+PACKSIZE=%d", packsize);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块数据加密密钥配置
 * @param       datakey: 数据加密密钥，范围0~0xFFFFFFFF
 * @retval      LORA_EOK   : 数据包大小配置成功
 *              LORA_ERROR : 数据包大小配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_datakey_config(uint32_t datakey)
{
    uint8_t ret;
    char cmd[20] = {0};
    
    sprintf(cmd, "AT+DATAKEY=%08X", datakey);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块串口配置
 * @param       baudrate: LORA_UARTRATE_1200BPS  : 1200bps
 *                        LORA_UARTRATE_2400BPS  : 2400bps
 *                        LORA_UARTRATE_4800BPS  : 4800bps
 *                        LORA_UARTRATE_9600BPS  : 9600bps
 *                        LORA_UARTRATE_19200BPS : 19200bps
 *                        LORA_UARTRATE_38400BPS : 38400bps
 *                        LORA_UARTRATE_57600BPS : 57600bps
 *                        LORA_UARTRATE_115200BPS: 115200bps（默认）
 *              parity  : LORA_UARTPARI_NONE: 无校验（默认）
 *                        LORA_UARTPARI_EVEN: 偶校验
 *                        LORA_UARTPARI_ODD : 奇校验
 * @retval      LORA_EOK   : 串口配置成功
 *              LORA_ERROR : 串口配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_uaconfig(lora_uartrate_t baudrate, lora_uartpari_t parity)
{
    uint8_t ret;
    char cmd[12] = {0};
    
    switch (baudrate)
    {
        case LORA_UARTRATE_1200BPS:
        case LORA_UARTRATE_2400BPS:
        case LORA_UARTRATE_4800BPS:
        case LORA_UARTRATE_9600BPS:
        case LORA_UARTRATE_19200BPS:
        case LORA_UARTRATE_38400BPS:
        case LORA_UARTRATE_57600BPS:
        case LORA_UARTRATE_115200BPS:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    switch (parity)
    {
        case LORA_UARTPARI_NONE:
        case LORA_UARTPARI_EVEN:
        case LORA_UARTPARI_ODD:
        {
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    sprintf(cmd, "AT+UART=%d,%d", baudrate, parity);
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}

/**
 * @brief       ATK-MWCC68D模块信道检测配置
 * @param       enable: LORA_DISABLE: 关闭信道检测
 *                      LORA_ENABLE : 打开信道检测
 * @retval      LORA_EOK   : 信道检测配置成功
 *              LORA_ERROR : 信道检测配置失败
 *              LORA_EINVAL: 输入参数有误
 */
uint8_t lora_lbt_config(lora_enable_t enable)
{
    uint8_t ret;
    char cmd[9] = {0};
    
    switch (enable)
    {
        case LORA_DISABLE:
        {
            sprintf(cmd, "AT+LBT=0");
            break;
        }
        case LORA_ENABLE:
        {
            sprintf(cmd, "AT+LBT=0");
            break;
        }
        default:
        {
            return LORA_EINVAL;
        }
    }
    
    ret = at_cmd_send(&lora_at_dev, cmd, "OK", AT_WAIT_ACK_TIMEOUT);
    if (ret != LORA_EOK)
    {
        return LORA_ERROR;
    }
    
    return LORA_EOK;
}
