#include "mcu_flash.h"
#include "crc16.h"
#include "sys_delay.h"
#include "string.h"
#include "stdlib.h"
#include "main.h"
#include "ymodem.h"

ymodem_put_t ymodem_put;
ymodem_get_t ymodem_get;
ymodem_get_len_t ymodem_get_len;
ymodem_get_restart_t ymodem_get_restart;
ymodem_write_flash_t ymodem_write_flash;
ymodem_erase_flash_t ymodem_erase_flash;

/* Ymodem 发送方 */
#define YMODEM_SOH 0x01 // 128 bytes
#define YMODEM_STX 0x02 // 1024 bytes
#define YMODEM_EOT 0x04 // End of transmission

/* Ymodem 通用 */
#define YMODEM_CA 0x18 // Cancel

/* Ymodem 接收方 */
#define YMODEM_ACK 0x06    // Acknowledge
#define YMODEM_NAK 0x15    // Not acknowledge
#define YMODEM_C 0x43      // ASCII 'C'
#define YMODEM_Abort1 0x41 // 'A'
#define YMODEM_Abort2 0x61 // 'a'

/* Ymodem 接收数据帧 */
typedef struct 
{
    uint8_t ymodem_head;             // 数据帧头
    uint8_t ymodem_num;              // 数据帧序号
    uint16_t ymodem_data_cache_len;  // 数据帧长度
    uint16_t ymodem_crc;             // 数据帧CRC
    uint8_t ymodem_data_cache[1024]; // 数据帧缓存
} ymodem_frame_t;
static ymodem_frame_t ymodem_frame;

/* Ymodem 协议状态 */
typedef struct 
{
    uint8_t ymodem_success_flag;
    uint8_t ymodem_start_or_end_flag;
    uint8_t ymodem_ctrl_flag;
    uint8_t ymodem_eot_flag;
    uint8_t ymodem_cancel_flag;
    uint8_t ymodem_recive_flag;
}ymodem_status_t;
static ymodem_status_t ymodem_status;

/* Ymodem 文件信息 */
static uint8_t ymodem_file_name[64];
static uint32_t ymodem_file_size = 0;
static uint32_t ymodem_write_addr = YMODEM_FLASH_ADDR;

static void ymodem_st_flag_write(uint8_t flag)
{
    ymodem_status.ymodem_start_or_end_flag = flag;
}

/**
 * @brief Ymodem初始化
 * @param put 发送数据函数
 * @param get 接收数据函数
 * @param len 获取数据长度函数
 * @param get_restart 重启函数
 * @return 0:成功 1:失败
 */
uint8_t ymodem_init(ymodem_put_t put, ymodem_get_t get, ymodem_get_len_t len, ymodem_get_restart_t get_restart, ymodem_write_flash_t write_flash, ymodem_erase_flash_t erase_flash)
{
    if (put == NULL || get == NULL || len == NULL || get_restart == NULL || write_flash == NULL || erase_flash == NULL)
    {
        return 1;
    }
    ymodem_put = put;
    ymodem_get = get;
    ymodem_get_len = len;
    ymodem_get_restart = get_restart;
    ymodem_write_flash = write_flash;
    ymodem_erase_flash = erase_flash;
    return 0;
}

/**
 * @brief Ymodem处理数据帧,数据存入ymodem_data_cache
 * @param buf 数据帧
 * @param len 数据长度
 * @return 0:成功 1:失败
 */
static uint8_t ymodem_data_cpy(uint8_t *buf, uint16_t len)
{
    if (buf == NULL || len == 0)
    {
        return 1;
    }

    ymodem_frame.ymodem_crc = buf[len - 2] << 8 | buf[len - 1];
    ymodem_frame.ymodem_data_cache_len = len - 5; // 数据帧长度
    ymodem_frame.ymodem_num = buf[1];             // 数据帧序号
    memcpy(ymodem_frame.ymodem_data_cache, buf + 3, len - 5);
    return 0;
}

/**
 * @brief Ymodem数据帧校验
 * @return 0:校验通过 1:校验失败
 */
static uint8_t ymodem_check(void)
{
    uint16_t crc = crc16_xmodem(ymodem_frame.ymodem_data_cache, ymodem_frame.ymodem_data_cache_len);
    if (crc == ymodem_frame.ymodem_crc) //&& (ymodem_frame.ymodem_num == 0xff - ymodem_frame.ymodem_num))
    {
        return 0;
    }
    printf("crc error!crc:%#X\r\n", crc);
    return 1;
}

/**
 * @brief Ymodem起始帧或结束帧检查处理
 * @return 0:成功 1:失败
 */
static uint8_t ymodem_start_frame_check(void)
{
    if (ymodem_check() == 0)
    {
        uint8_t i;
        for (i = 0; i < 64; i++)
        {
            ymodem_file_name[i] = ymodem_frame.ymodem_data_cache[i];
            if (ymodem_frame.ymodem_data_cache[i] == 0)
            {
                break;
            }
        }
        uint8_t size[10];
        for (uint8_t j = 1; j > 0; j++)
        {
            if (ymodem_frame.ymodem_data_cache[i + j] == 0x20)
            {
                break;
            }
            size[j - 1] = ymodem_frame.ymodem_data_cache[i + j];
        }
        if (size[0] == '0' && size[1] == 'x')
            ymodem_file_size = strtol((char *)size, NULL, 16);
        else
            ymodem_file_size = strtol((char *)size, NULL, 10);
        if (ymodem_file_size > YMODEM_FLASH_SIZE)
        {
            ymodem_st_flag_write(0);
            return 1;
        }
        else if (ymodem_file_name[0] == 0 && ymodem_file_size == 0) // 结束帧
        {
            ymodem_st_flag_write(2);
            return 0;
        }
        else
        {
            ymodem_st_flag_write(1); // 起始帧
            return 0;
        }
    }
    ymodem_st_flag_write(0);//校验失败
    return 1;

}

/**
 * @brief Ymodem接收数据帧
 * @param buf 数据帧
 * @param len 数据长度
 * @return 0:成功 1:失败
 * @note 循环调用,直至结束传输
 */
static uint8_t ymodem_data_recv()
{
    uint8_t *buf = ymodem_get();
    uint16_t len = ymodem_get_len();
    if (buf == NULL || len == 0) // 无数据
    {
        printf("No data received\r\n");
        //ymodem_get_restart();
        return 1;
    }
    ymodem_frame.ymodem_head = buf[0]; // 数据帧头
    if (len == 1)                      // 其他控制帧
    {
        ymodem_status.ymodem_ctrl_flag = 1;
        switch (ymodem_frame.ymodem_head)
        {
        case YMODEM_EOT:
            ymodem_status.ymodem_eot_flag++;
            break;
        case YMODEM_CA:
            ymodem_status.ymodem_cancel_flag++;
            break;
        case YMODEM_Abort1:
            ymodem_status.ymodem_cancel_flag++;
            break;
        case YMODEM_Abort2:
            ymodem_status.ymodem_cancel_flag++;
            break;
        }
        ymodem_get_restart();
        return 0;
    }

    ymodem_data_cpy(buf, len);
    ymodem_get_restart();

    if (ymodem_frame.ymodem_num == 0 && (ymodem_frame.ymodem_head == 1 || ymodem_frame.ymodem_head == 2)) // 起始帧
    { // 判断是否为起始帧或结束帧
        uint8_t ret = ymodem_start_frame_check();
        return ret;
    }
    if (ymodem_check() == 1)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief Ymodem发送应答帧
 * @param buf 应答帧
 * @param len 数据长度
 * @return 0:成功 1:失败
 */
static uint8_t ymodem_send_ack(uint8_t ack)
{
    uint8_t buf[2];
    buf[0] = ack;
    if (ymodem_put(buf, 1) == 0)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief Ymodem请求发送文件
 */
static uint8_t ymodem_start_request(void)
{
    while (1)
    {
        sys_delay_ms(1000);
        ymodem_send_ack(YMODEM_C); // 发送C
        sys_delay_ms(3000);
        if (ymodem_data_recv() == 0) // 处理起始帧
        {
            if (ymodem_status.ymodem_start_or_end_flag == 1)
                break;
            else if (ymodem_status.ymodem_cancel_flag == 1)
                return 1;
        }
        printf("Request timeout\r\n");
    }
    ymodem_send_ack(YMODEM_ACK);
    sys_delay_ms(50);
    ymodem_send_ack(YMODEM_C); // 发送C
    sys_delay_ms(30);
    return 0;
}

/**
 * @brief Ymodem状态机
 * @return 0:成功 1:失败
 */
uint8_t ymodem_recv_status_fun(void)
{
    while (1)
    {
        switch (ymodem_status.ymodem_start_or_end_flag)
        {
        case 0:
            if (ymodem_start_request() == 1)
                return 1;
					ymodem_erase_flash(YMODEM_FLASH_ADDR, ymodem_file_size);
            break;
        case 1:
            if (ymodem_status.ymodem_ctrl_flag == 1) // 其他控制帧
            {
                switch (ymodem_status.ymodem_eot_flag)
                {
                case 1:
                    ymodem_send_ack(YMODEM_NAK);
                    ymodem_status.ymodem_ctrl_flag = 0;
                    sys_delay_ms(500);
                    break;
                case 2:
                    ymodem_send_ack(YMODEM_ACK);
                    ymodem_status.ymodem_ctrl_flag = 0;
                    sys_delay_ms(500);
                    return 0;
                default:
                    ymodem_status.ymodem_ctrl_flag = 0;
                    ymodem_send_ack(YMODEM_ACK);
                    sys_delay_ms(500);
                    return 0;
                }
                switch (ymodem_status.ymodem_cancel_flag)
                {
                case 1:
                    ymodem_send_ack(YMODEM_ACK);
                    ymodem_status.ymodem_ctrl_flag = 0;
                    return 0;
                default:
                    ymodem_status.ymodem_ctrl_flag = 0;
                    ymodem_send_ack(YMODEM_NAK);
                    break;
                }
                break;
            }
            else
            {
                ymodem_write_flash(ymodem_write_addr, ymodem_frame.ymodem_data_cache, ymodem_frame.ymodem_data_cache_len);
                ymodem_write_addr += ymodem_frame.ymodem_data_cache_len;
                ymodem_send_ack(YMODEM_ACK);
				break;
            }
            
        case 2:
            ymodem_send_ack(YMODEM_ACK);
            ymodem_status.ymodem_success_flag = 1;
            return 0;
        default:
            return 1;
        }
        sys_delay_ms(150);
        while(ymodem_data_recv() == 1)
        {
            ymodem_send_ack(YMODEM_NAK);
            sys_delay_ms(50);
        }
        
    }
}
