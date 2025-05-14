/* WS8475DLS.c */
#include "WS8475DLS_driver.h"
#include "usart.h"
#include "string.h"
#include "stdio.h" // 添加stdio.h以使用printf

#define WS8475DLS_DEBUG 0
#if WS8475DLS_DEBUG == 1
#define WS8475DLS_LOG(fmt, ...) printf("[WS8475DLS] " fmt, ##__VA_ARGS__)
#else
#define WS8475DLS_LOG(fmt, ...)
#endif

#define CONFIG_BAUDRATE   9600
#define CMD_TIMEOUT_MS    1000
#define AUX_CHECK_INTERVAL 10

#define true 1
#define false 0

/* 私有函数声明 */
static uint8_t send_command(ws8475_handle_t *handle, uint8_t *cmd, uint8_t len);
static uint8_t wait_response(ws8475_handle_t *handle, uint8_t *expected, uint16_t len);
static uint8_t enter_config_mode(ws8475_handle_t *handle);
static uint8_t exit_config_mode(ws8475_handle_t *handle);

/*----------------------------------------------------------
 * 初始化与模式设置
 *---------------------------------------------------------*/
uint8_t ws8475_init(ws8475_handle_t *handle)
{
    if (!handle) {
        WS8475DLS_LOG("ws8475_init: handle is NULL\r\n");
        return false;
    }

    /* 设置默认配置 */
    //memcpy(&handle->config, config, sizeof(ws8475_config_t));

    /* 进入配置模式 */
    if (!enter_config_mode(handle)) {
        WS8475DLS_LOG("ws8475_init: enter_config_mode failed\r\n");
        return false;
    }

    /* 写入配置参数 */
    uint8_t result = ws8475_write_config(handle, true);
    WS8475DLS_LOG("ws8475_init: write_config %s\r\n", result ? "success" : "failed");
    return result;
}

/*----------------------------------------------------------
 * 核心功能实现
 *---------------------------------------------------------*/
uint8_t ws8475_set_mode(ws8475_handle_t *handle, ws8475_mode_m mode)
{
    HAL_GPIO_WritePin(LORA_MODE_GPIO_Port, LORA_MODE_Pin, mode == MODE_CONFIG ? GPIO_PIN_RESET : GPIO_PIN_SET); // 设置模式引脚
    WS8475DLS_LOG("ws8475_set_mode: set to %s mode\r\n", mode == MODE_CONFIG ? "CONFIG" : "TRANSPARENT");
    return 0;
}

uint8_t ws8475_send_data(ws8475_handle_t *handle, uint8_t *data, uint16_t len)
{
    if (!handle || !data || len == 0) {
        WS8475DLS_LOG("ws8475_send_data: invalid parameters\r\n");
        return false;
    }

    /* 等待模块就绪 */
    if (!ws8475_wait_ready(handle, CMD_TIMEOUT_MS)) {
        WS8475DLS_LOG("ws8475_send_data: wait_ready timeout\r\n");
        return false;
    }

    /* 发送数据 */
    handle->send_buf(data, len);
    WS8475DLS_LOG("ws8475_send_data: sent %d bytes\r\n", len);
    return true;
}

uint8_t ws8475_receive_data(ws8475_handle_t *handle, uint8_t *buffer, uint16_t *len)
{
    if (!handle || !buffer || !len) {
        WS8475DLS_LOG("ws8475_receive_data: invalid parameters\r\n");
        return false;
    }

    /* 等待模块就绪 */
    if (!ws8475_wait_ready(handle, CMD_TIMEOUT_MS)) {
        WS8475DLS_LOG("ws8475_receive_data: wait_ready timeout\r\n");
        return false;
    }

    /* 读取数据 */
    if(handle->recv_buflen != 0)
    {
        *len = handle->recv_buflen();
        memcpy(buffer, (const void*)handle->recv_buf(), *len);
        WS8475DLS_LOG("ws8475_receive_data: received %d bytes\r\n", *len);
        return true;
    }

    WS8475DLS_LOG("ws8475_receive_data: no data available\r\n");
    return false;
}

/*----------------------------------------------------------
 * 配置管理
 *---------------------------------------------------------*/
uint8_t ws8475_write_config(ws8475_handle_t *handle, uint8_t save_to_flash)
{
    uint8_t cmd[7] = {0};
    
    /* 构造配置命令 */
    cmd[0] = save_to_flash ? 0xC2 : 0xC0;//0:C0,1:C2
    cmd[1] = handle->config.address_h;
    cmd[2] = handle->config.address_l;
    cmd[3] = ((handle->config.check_bit & 0x03) << 6) | 
             ((handle->config.baudrate & 0x07) << 3) | 
             (handle->config.air_rate & 0x07);
    cmd[4] = handle->config.channel;
    cmd[5] = ((handle->config.fp_mode_enable & 0x01) << 7) | 
             ((handle->config.wor_mode_enable & 0x01) << 6) |
             ((handle->config.wor_wireless_wakeup_time & 0x07) << 3) |
             ((handle->config.lbt_enble & 0x01) << 2) |
             (handle->config.tx_power & 0x03);
    cmd[6] = (handle->config.rssi_enable & 0x01) << 7;
    WS8475DLS_LOG("ws8475_write_config: cmd[0]: 0x%02X\r\n", cmd[0]);
    WS8475DLS_LOG("ws8475_write_config: cmd[1]: 0x%02X\r\n", cmd[1]);
    WS8475DLS_LOG("ws8475_write_config: cmd[2]: 0x%02X\r\n", cmd[2]);
    WS8475DLS_LOG("ws8475_write_config: cmd[3]: 0x%02X\r\n", cmd[3]);
    WS8475DLS_LOG("ws8475_write_config: cmd[4]: 0x%02X\r\n", cmd[4]);
    WS8475DLS_LOG("ws8475_write_config: cmd[5]: 0x%02X\r\n", cmd[5]);
    WS8475DLS_LOG("ws8475_write_config: cmd[6]: 0x%02X\r\n", cmd[6]);

    /* 发送配置命令 */
    uint8_t result = send_command(handle, cmd, sizeof(cmd));
    WS8475DLS_LOG("ws8475_write_config: %s\r\n", result ? "success" : "failed");
    return result;
}

uint8_t ws8475_read_config(ws8475_handle_t *handle)
{
    uint8_t cmd[3] = {0xC1, 0xC1, 0xC1};
    uint8_t timeout = 100;
    /* 进入配置模式 */
    if (!enter_config_mode(handle)) {
        WS8475DLS_LOG("ws8475_read_config: enter_config_mode failed\r\n");
        return false;
    }
    ws8475_send_data(handle, cmd, sizeof(cmd));
    uint8_t *cmd_ack;
    uint16_t len = 0;
    while(timeout--)
    {
        cmd_ack = handle->recv_buf();
        if(cmd_ack != NULL)
        {
            len = handle->recv_buflen();
            
            handle->config.address_h = cmd_ack[1];
            handle->config.address_l = cmd_ack[2];
            handle->config.check_bit = (cmd_ack[3] >> 6) & 0x03;
            handle->config.baudrate = (cmd_ack[3] >> 3) & 0x07;
            handle->config.air_rate = cmd_ack[3] & 0x07;
            handle->config.channel = cmd_ack[4];
            handle->config.fp_mode_enable = (cmd_ack[5] >> 7) & 0x01;
            handle->config.wor_mode_enable = (cmd_ack[5] >> 6) & 0x01;
            handle->config.wor_wireless_wakeup_time = (cmd_ack[5] >> 3) & 0x07;
            handle->config.lbt_enble = (cmd_ack[5] >> 2) & 0x01;
            handle->config.tx_power = cmd_ack[5] & 0x03;
            handle->config.rssi_enable = (cmd_ack[6] >> 7) & 0x01;

            WS8475DLS_LOG("ws8475_read_config: received %d bytes\r\n", len);
            for(int i = 0; i < len; i++)
            {
                WS8475DLS_LOG("0x%02X ", cmd_ack[i]);
            }
            WS8475DLS_LOG("\r\n");
            handle->recv_buf_reset();
            WS8475DLS_LOG("ws8475_read_config: success\r\n");
            /* 退出配置模式 */
            exit_config_mode(handle);
            return true;
        }
        handle->delay(10);
    }
    WS8475DLS_LOG("ws8475_read_config: timeout\r\n");
    /* 退出配置模式 */
    exit_config_mode(handle);
    return false;
}

uint8_t ws8475_reboot(ws8475_handle_t *handle)
{
    uint8_t cmd[3] = {0xC4, 0xC4, 0xC4};
    uint8_t result = send_command(handle, cmd, sizeof(cmd));
    WS8475DLS_LOG("ws8475_reboot: %s\r\n", result ? "success" : "failed");
    return result;
}

uint8_t ws8475_factory_reset(ws8475_handle_t *handle)
{
    uint8_t cmd[3] = {0xC5, 0xC5, 0xC5};
    uint8_t result = send_command(handle, cmd, sizeof(cmd));
    WS8475DLS_LOG("ws8475_factory_reset: %s\r\n", result ? "success" : "failed");
    return result;
}

/*----------------------------------------------------------
 * 私有工具函数
 *---------------------------------------------------------*/
static uint8_t enter_config_mode(ws8475_handle_t *handle)
{
    /* 设置模式引脚为11（需外部硬件支持） */
    /* 此处需要用户根据硬件连接实现具体模式切换 */
    ws8475_set_mode(handle, MODE_CONFIG);
    /* 等待模块进入配置模式 */
    handle->delay(100);
    uint8_t result = ws8475_wait_ready(handle, 500);
    WS8475DLS_LOG("enter_config_mode: %s\r\n", result ? "success" : "failed");
    return result;
}

static uint8_t exit_config_mode(ws8475_handle_t *handle)
{
    /* 设置模式引脚为00（需外部硬件支持） */
    /* 此处需要用户根据硬件连接实现具体模式切换 */
    ws8475_set_mode(handle, MODE_TRANSPARENT);
    /* 等待模块进入正常模式 */
    handle->delay(100);
    uint8_t result = ws8475_wait_ready(handle, 500);
    WS8475DLS_LOG("exit_config_mode: %s\r\n", result ? "success" : "failed");
    return result;
}

static uint8_t send_command(ws8475_handle_t *handle, uint8_t *cmd, uint8_t len)
{
    if (!handle || !cmd || len == 0) {
        WS8475DLS_LOG("send_command: invalid parameters\r\n");
        return false;
    }
    handle->recv_buf_reset();
    /* 发送命令字节 */
    handle->send_buf(cmd, len);
    WS8475DLS_LOG("send_command: sent %d bytes\r\n", len);
    /* 等待确认响应 */
    handle->delay(20);
    uint8_t result = wait_response(handle, cmd, len);
    WS8475DLS_LOG("send_command: %s\r\n", result ? "success" : "failed");
    return result;
}

static uint8_t wait_response(ws8475_handle_t *handle, uint8_t *expected, uint16_t len)
{
    uint8_t timeout = 20;
    while(timeout--)
    {
        if(memcmp((const void*)handle->recv_buf(), expected, len) == 0)
        {
            handle->recv_buf_reset();
            return true;
        }
        handle->delay(10);
    }
    exit_config_mode(handle);
    WS8475DLS_LOG("wait_response: timeout waiting for response\r\n");
    return false;
}

/*----------------------------------------------------------
 * 状态检查函数
 *---------------------------------------------------------*/
uint8_t ws8475_wait_ready(ws8475_handle_t *handle, uint32_t timeout_ms)
{
    uint32_t start = 0; /* 需要用户实现获取时间戳 */
    while (timeout_ms-10) {
        if (handle->read_aux()) {
            WS8475DLS_LOG("ws8475_wait_ready: device ready\r\n");
            return true;
        }
        handle->delay(AUX_CHECK_INTERVAL);
    }
    WS8475DLS_LOG("ws8475_wait_ready: timeout\r\n");
    return false;
}
