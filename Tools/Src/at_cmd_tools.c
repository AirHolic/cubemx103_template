#include "main.h"
#include "string.h"
#include "stdio.h"
#include "at_cmd_tools.h"

#define AT_CMD_TOOLS_DEBUG 1
#if AT_CMD_TOOLS_DEBUG == 1
#define AT_LOG(fmt, ...) printf("[AT TOOLS] " fmt "\r\n", ##__VA_ARGS__)
#else
#define AT_LOG(fmt, ...)
#endif

/**
 * @brief       以AT模块AT指令的格式获取src中的第param_index个参数到dts
 * @param       src        : AT模块的AT响应
 * @param       param_num  : 参数的索引
 * @param       param_index: 找到的参数在src中的索引
 * @param       param_len  : 参数的长度
 * @retval      ATK_IDE01_EOK   : 找到指定参数
 * @retval      ATK_IDE01_EINVAL: 函数参数错误
 * @note        针对带英文双引号的参数
 */
at_cmd_status_t at_ack_get_str_parameter(uint8_t *src, uint8_t param_num, uint16_t *param_index, uint16_t *param_len)
{
    uint8_t src_index = 0;
    uint16_t _param_index = 0;
    uint16_t _param_len = 0;
    uint8_t param_flag = 0;
    //参数索引从1开始
    if (param_num == 0)
    {
        return AT_CMD_PARMINVAL;
    }
    //找到第一个等号
    while ((src[src_index] != '\0') && (src[src_index] != '='))
    {
        src_index++;
    }
    //没有找到=
    if (src[src_index] != '=')
    {
        return AT_CMD_PARMINVAL;
    }
    //找到参数
    while (src[src_index] != '\0')
    {
        if (src[src_index] == '"')
        {
            if ((param_flag & (1 << 0)) != (1 << 0))
            {
                param_flag |= (1 << 0);
                if (param_num == 1)
                {
                    param_flag |= (1 << 1);
                    _param_index = src_index + 1;
                    if (param_len == NULL)
                    {
                        break;
                    }
                }
            }
            else
            {
                param_flag &= ~(1 << 0);
                param_num--;
                if ((param_flag & (1 << 1)) == (1 << 1))
                {
                    _param_len = src_index - _param_index;
                    break;
                }
            }
        }
        src_index++;
    }

    if (param_index != NULL)
    {
        *param_index = _param_index;
    }

    if (param_len != NULL)
    {
        *param_len = _param_len;
    }

    return AT_CMD_OK;
}

/**
 * @brief       以AT模块AT指令的格式获取src中的第param_index个参数到dts
 * @param       src        : AT模块的AT响应
 * @param       param_num  : 参数的索引
 * @param       param_index: 找到的参数在src中的索引
 * @param       param_len  : 参数的长度
 * @retval      ATK_IDE01_EOK   : 找到指定参数
 * @retval      ATK_IDE01_EINVAL: 函数参数错误
 * @note        以逗号分隔的参数
 */
at_cmd_status_t at_ack_get_normal_parameter(uint8_t *src, uint8_t param_num, uint16_t *param_index, uint16_t *param_len)
{
    uint8_t src_index = 0;
    uint16_t _param_index = 0;
    uint16_t _param_len = 0;
    uint8_t _param_num = 0;
    //参数索引从1开始
    if (param_num == 0)
    {
        return AT_CMD_PARMINVAL;
    }
    //先找到第一个等号
    while ((src[src_index] != '\0') && (src[src_index] != '='))
    {
        src_index++;
    }
    //没有找到=
    if (src[src_index] != '=')
    {
        return AT_CMD_PARMINVAL;
    }
    //找到参数
    while (src[src_index] != '\0')
    {//逗号或者等号或者回车,例“=param1,param2\r”
        if (src[src_index] == '=' || src[src_index] == ',' || src[src_index] == '\r')
        {
            _param_num++;
            if(_param_num == param_num)
            {
                _param_index = src_index + 1;

            }
            else if(_param_num == param_num + 1)
            {
                _param_len = src_index - _param_index;
                break;
            }
        }
        src_index++;
    }

    if (param_index != NULL)
    {
        *param_index = _param_index;
    }

    if (param_len != NULL)
    {
        *param_len = _param_len;
    }

    return AT_CMD_OK;
}

/**
 * @brief  Send AT command
 * @param  at_dev: AT device
 * @param  cmd: AT command
 * @param  ack: AT command ack
 * @param  timeout: AT command timeout
 * @retval AT_CMD_OK: AT command OK
 * @retval AT_CMD_TIMEOUT: AT command timeout
 * @retval AT_CMD_ERROR: AT command error
 */
at_cmd_status_t at_cmd_send(at_device_t *at_dev, char *cmd, char *ack, uint32_t timeout)
{
    if (cmd == NULL || ack == NULL)
    {
        return AT_CMD_ERROR;
    }
    at_dev->at_ack_restart();
    uint8_t *ret = NULL;
    at_dev->at_cmd_pprintf("%s\r\n", cmd);
    AT_LOG("cmd: %s\r\n", cmd);
    if (timeout == 0)
    {
        return AT_CMD_OK;
    }

    while (timeout > 0)
    {
        ret = at_dev->at_cmd_ack();
        if (ret != NULL)
        {
            AT_LOG("ret: %s\r\n", ret);
            if (ack != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return AT_CMD_OK;
                }
                else
                {
                    at_dev->at_ack_restart();
                }
            }
            else
            {
                return AT_CMD_OK;
            }
        }
        timeout--;
        HAL_Delay(1);
    }
    return AT_CMD_TIMEOUT;
}

/**
 * @brief  Send SP AT command
 * @param  at_dev: AT device
 * @param  cmd: AT command
 * @param  ack: AT command ack
 * @param  timeout: AT command timeout
 * @retval AT_CMD_OK: AT command OK
 * @retval AT_CMD_TIMEOUT: AT command timeout
 * @retval AT_CMD_ERROR: AT command error
 * @note   SP AT command is without /r/n
 */
at_cmd_status_t at_sp_cmd_send(at_device_t *at_dev, char *cmd, char *ack, uint32_t timeout)
{
    if (cmd == NULL || ack == NULL)
    {
        return AT_CMD_ERROR;
    }
    at_dev->at_ack_restart();
    uint8_t *ret = NULL;
    at_dev->at_cmd_pprintf(AT_SP_SEND_CMD, cmd);
    AT_LOG("cmd: %s", cmd);
    if (timeout == 0)
    {
        return AT_CMD_OK;
    }

    while (timeout > 0)
    {
        ret = at_dev->at_cmd_ack();
        if (ret != NULL)
        {
            AT_LOG("ret: %s\r\n", ret);
            if (ack != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return AT_CMD_OK;
                }
                else
                {
                    at_dev->at_ack_restart();
                }
            }
            else
            {
                return AT_CMD_OK;
            }
        }
        timeout--;
        HAL_Delay(1);
    }
    return AT_CMD_TIMEOUT;
}

/**
 * @brief  打包AT指令
 * @param  at_cmd : AT指令存放的缓冲区
 * @param  cmd_code: AT指令码
 * @param  cmd_para: AT指令参数,没有参数传NULL
 * @retval at_cmd : 打包好的AT指令
 * @note   参数为不带双引号的参数，如需添加双引号，需要在cmd_para中添加
 */
char *at_cmd_pack(char *at_cmd, char *cmd_code, char *cmd_para)
{
    if(at_cmd == NULL || cmd_code == NULL)
        return NULL;
    if(cmd_para != NULL)
        sprintf(at_cmd, "AT+%s=%s\r\n", cmd_code, cmd_para);
    else
        sprintf(at_cmd, "AT+%s\r\n", cmd_code);
    return at_cmd;
}
