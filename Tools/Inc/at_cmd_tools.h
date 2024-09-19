#ifndef __AT_CMD_TOOLS_H__
#define __AT_CMD_TOOLS_H__

typedef uint8_t (*p_printf_cmd)(char *fmt, ...);
typedef uint8_t (*p_hex_printf_cmd)(uint8_t *buf, uint16_t len);
typedef void (*p_ack_restart)(void);
typedef void (*p_delay_ms)(uint32_t ms);
typedef uint8_t* (*p_get_buf)(void);

/**
 * @brief  AT device structure
 * @param  at_id: AT device ID
 * @param  at_cmd_pprintf: AT command printf function
 * @param  at_ack_restart: AT ack restart function
 * @param  at_delay_ms: AT delay ms function
 * @param  at_cmd_ack: AT command ack buffer
 */
typedef struct __at_device_t
{
    uint8_t at_id;
    p_printf_cmd at_cmd_pprintf;
    p_ack_restart at_ack_restart;
    p_delay_ms at_delay_ms;
    p_get_buf at_cmd_ack;
    p_hex_printf_cmd at_hex_printf_cmd;
} at_device_t;

typedef enum
{
    AT_CMD_OK = 0x00U,
    AT_CMD_ERROR = 0x01U,
    AT_CMD_TIMEOUT = 0x02U,
    AT_CMD_PARMINVAL = 0x03U,
} at_cmd_status_t;

#define AT_WAIT_ACK_TIMEOUT 3000

at_cmd_status_t at_ack_get_str_parameter(uint8_t *src, uint8_t param_num, uint16_t *param_index, uint16_t *param_len);
at_cmd_status_t at_ack_get_normal_parameter(uint8_t *src, uint8_t param_num, uint16_t *param_index, uint16_t *param_len);
at_cmd_status_t at_cmd_send(at_device_t *at_dev, char *cmd, char *ack, uint32_t timeout);
at_cmd_status_t at_sp_cmd_send(at_device_t *at_dev, char *cmd, char *ack, uint32_t timeout);
char *at_cmd_pack(char *at_cmd, char *cmd_code, char *cmd_para);

#endif /* _AT_CMD_TOOLS_H__ */
