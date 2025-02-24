#ifndef __E52_LORA_AT_DRIVER_H__
#define __E52_LORA_AT_DRIVER_H__

#include "at_cmd_tools.h"

extern at_device_t e52_lora_at_dev;

typedef enum
{
    E52_UART_BAUD_1200 = 1200,
    E52_UART_BAUD_2400 = 2400,
    E52_UART_BAUD_4800 = 4800,
    E52_UART_BAUD_9600 = 9600,
    E52_UART_BAUD_19200 = 19200,
    E52_UART_BAUD_38400 = 38400,
    E52_UART_BAUD_57600 = 57600,
    E52_UART_BAUD_115200 = 115200,
    E52_UART_BAUD_230400 = 230400,
    E52_UART_BAUD_460800 = 460800,
}e52_lora_at_uart_baud_t;

typedef enum
{
    E52_UART_8N1 = 0,
    E52_UART_8E1 = 1,
    E52_UART_801 = 2,
}e52_lora_at_uart_partiy_t;

typedef enum
{
    E52_LORA_RATE_62_5K = 0,
    E52_LORA_RATE_21_875K = 1,
    E52_LORA_RATE_7K = 2,
}e52_lora_at_air_rate_t;

typedef enum
{
    E52_LORA_OPTION_UNICAST = 1,
    E52_LORA_OPTION_MULTICAST = 2,
    E52_LORA_OPTION_BROADCAST = 3,
    E52_LORA_OPTION_ANYCAST = 4,
}e52_lora_at_option_t;

typedef enum
{
    E52_LORA_TYPE_ROUTERS = 0,
    E52_LORA_TYPE_END_DEVICE = 1,
}e52_lora_at_type_t;

typedef enum
{
    E52_LORA_FUNCTIONAL_DISABILITY = 0,
    E52_LORA_FUNCTIONAL_ENABLEMENT = 1,
}e52_lora_at_functional_t;

typedef enum
{
    E52_LORA_DO_NOT_SAVE_TO_FLASH = 0,
    E52_LORA_SAVE_TO_FLASH = 1,
}e52_lora_at_save_t;

/**
 * @brief  e52-lora消息结构体
 * @note   直接指针转换或者memcpy
 */
typedef struct 
{
    uint8_t head_type;
    uint8_t data_len;
    uint16_t panid;
    uint16_t src_addr;
    uint16_t dst_addr;
    /**
     * @brief  自定义前置包信息
     * @param  pack_type: 0-单包 1-分包 2-预通知包（或应答）
     * @param  pack_num: 单包为0，分包为包序号，预通知包为包总数
     * @param  data: 如为预通知包，则装载大包长度
     */
    uint8_t pack_type;//0-单包 1-分包 2-预通知包（或应答，漏包时重新请求）
    uint8_t pack_num;//单包为0，分包为包序号，预通知包为包总数,分包从0开始（漏包时附上所需索引，若为包总数加1则为请求全部重发）
    uint8_t data[198];//最大200字节，其中2字节为前置包信息，前置包信息作为自定义帧头，不计入协议数据长度和crc
}e52_lora_msg_t;

typedef struct
{
    uint8_t ack_type;
    uint8_t ack_num;
    uint8_t ack_data[198];
}e52_lora_ack_t;

/**
 * @brief E52 LoRa 通信确认消息定义
 * @{
 */

/** 
 * @brief 表示操作成功的响应消息 
 */
#define E52_LORA_MSG_ACK_SUCCESS "SUCCESS"

/** 
 * @brief 表示未收到确认的响应消息 
 */
#define E52_LORA_MSG_ACK_NO_ACK "NO ACK"

/** 
 * @brief 表示无可用路由的响应消息 
 */
#define E52_LORA_MSG_ACK_NO_ROUTE "NO ROUTE"

/** 
 * @brief 表示缓存溢出的响应消息 
 */
#define E52_LORA_MSG_ACK_OUT_OF_CACHE "OUT OF CACHE"

/** 
 * @brief 表示接收到 LoRa 消息的响应 
 */
#define E52_LORA_MSG_ACK_LORA_MSG "LORA MSG"

/** 
 * @brief 表示数据包打包成功的响应消息 
 */
#define E52_LORA_MSG_ACK_PACK_RECEIVE "PACK RECEIVE"

/** 
 * @brief 表示数据包此索引打包失败的响应消息,请求重发此包
 */
#define E52_LORA_MSG_ACK_PACK_RESEND "PACK RESEND"

/** 
 * @brief 表示数据包打包失败的响应消息,请求重发所有包
 */
#define E52_LORA_MSG_ACK_PACK_FAIL "PACK FAIL"

/** @} */



void e52_lora_at_driver_init(void);

at_cmd_status_t e52_lora_at_reset(void);

at_cmd_status_t e52_lora_at_query_transmit_power(int8_t *transmit_power);
at_cmd_status_t e52_lora_at_query_channel(uint8_t *channel);
at_cmd_status_t e52_lora_at_query_ack_time(uint16_t *ack_time);
at_cmd_status_t e52_lora_at_query_router_time(uint16_t *router_time);
at_cmd_status_t e52_lora_at_info_view(void);

at_cmd_status_t e52_lora_at_set_transmit_power(int8_t transmit_power, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_channel(uint8_t channel, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_uart(e52_lora_at_uart_baud_t baud, e52_lora_at_uart_partiy_t partiy);
at_cmd_status_t e52_lora_at_set_rate(e52_lora_at_air_rate_t rate);
at_cmd_status_t e52_lora_at_set_option(e52_lora_at_option_t option, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_type(e52_lora_at_type_t type);
at_cmd_status_t e52_lora_at_set_src_addr(uint16_t src_addr, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_dst_addr(uint16_t dst_addr, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_head(e52_lora_at_functional_t enable);
at_cmd_status_t e52_lora_at_set_back(e52_lora_at_functional_t enable);
at_cmd_status_t e52_lora_at_set_panid(uint16_t panid, e52_lora_at_save_t save);
at_cmd_status_t e52_lora_at_set_security(e52_lora_at_functional_t enable);
at_cmd_status_t e52_lora_at_set_reset_time(uint8_t min);
at_cmd_status_t e52_lora_at_set_ack_time(uint16_t ack_time);
at_cmd_status_t e52_lora_at_set_router_time(uint16_t router_time);
at_cmd_status_t e52_lora_at_set_router_save(e52_lora_at_functional_t enable);
at_cmd_status_t e52_lora_at_set_router_clr(void);
at_cmd_status_t e52_lora_at_set_group_clr(void);
at_cmd_status_t e52_lora_at_set_group_del(uint16_t group_addr);
at_cmd_status_t e52_lora_at_set_group_add(uint16_t group_addr);

e52_lora_msg_t e52_lora_msg_analyse(uint8_t *data, uint16_t len);
uint8_t e52_lora_ack_analyse(uint8_t *data, uint16_t len);

#endif /* __E52_LORA_AT_DRIVER_H__ */
