/**
 * @file mcu_config.c
 * @brief This file is used for modifying the MCU configuration.
 * 本文件用于修改MCU配置。
 */

#include "main.h"
#include "mcu_config.h"
#ifdef MCU_FLASH
#include "mcu_flash.h"
#endif // MCU_FLASH
#ifdef W25QXX_FLASH
#include "w25qxx_spi_driver.h"
#endif // W25QXX_FLASH
#include "string.h"

#ifdef MCU_FLASH
static mcu_config_func_t mcu_config_func = {mcu_flash_write, mcu_flash_read};
#endif // MCU_FLASH
#ifdef W25QXX_FLASH
static mcu_config_func_t mcu_config_func = {W25QXX_WriteNoErase, W25QXX_ReadBuffer};//配置项保存和加载函数
#endif // W25QXX_FLASH
// Define the configuration structure.
static struct mcu_config_t
{
    mcu_system_config_t system_config;
#if NETWORK_CONFIG == 1
    mcu_network_config_t network_config;
#endif
#if LORA_CONFIG == 1
    mcu_lora_config_t lora_config;
#endif
} mcu_config = {0};

#ifdef MCU_FLASH

/**
 * @brief Read the configuration from the flash.
 * 从flash中读取配置。
 */
void mcu_config_read_from_flash(void)
{
    uint32_t mcu_config_data[sizeof(mcu_config) / sizeof(uint32_t)];
    mcu_config_func.config_load(MCU_CONFIG_ADDR, mcu_config_data, sizeof(mcu_config) / sizeof(uint32_t));
    memcpy(&mcu_config, mcu_config_data, sizeof(mcu_config));

    //mcu_config_func.config_load(MCU_CONFIG_ADDR, (__IO uint32_t *)(&mcu_config.system_config.device_id), sizeof(mcu_config) / sizeof(uint32_t));
    // Read the configuration from the flash.
    // 从flash中读取配置。
}

/**
 * @brief Save the configuration to the flash.
 * 将配置保存到flash。
 */
void mcu_config_save_to_flash(void)
{
    uint32_t mcu_config_data[sizeof(mcu_config) / sizeof(uint32_t)];
    memcpy(mcu_config_data, &mcu_config, sizeof(mcu_config));
    mcu_config_func.config_save(MCU_CONFIG_ADDR, mcu_config_data, sizeof(mcu_config) / sizeof(uint32_t));
    //mcu_config_func.config_save(MCU_CONFIG_ADDR, (__IO uint32_t *)(&mcu_config.system_config.device_id), sizeof(mcu_config) / sizeof(uint32_t));
    // Save the configuration to the flash.
    // 将配置保存到flash。
}

#endif // MCU_FLASH

#ifdef W25QXX_FLASH

/**
 * @brief Read the configuration from the flash.
 * 从flash中读取配置。
 */
void mcu_config_read_from_flash(void)
{
    uint8_t mcu_config_data[sizeof(mcu_config) / sizeof(uint8_t)];
    mcu_config_func.config_load(mcu_config_data, MCU_CONFIG_ADDR, sizeof(mcu_config) / sizeof(uint8_t));
    memcpy(&mcu_config, mcu_config_data, sizeof(mcu_config));

    //mcu_config_func.config_load(MCU_CONFIG_ADDR, (__IO uint32_t *)(&mcu_config.system_config.device_id), sizeof(mcu_config) / sizeof(uint32_t));
    // Read the configuration from the flash.
    // 从flash中读取配置。
}

/**
 * @brief Save the configuration to the flash.
 * 将配置保存到flash。
 */
void mcu_config_save_to_flash(void)
{
    uint8_t mcu_config_data[sizeof(mcu_config) / sizeof(uint8_t)];
    memcpy(mcu_config_data, &mcu_config, sizeof(mcu_config));
    mcu_config_func.config_save(mcu_config_data, MCU_CONFIG_ADDR, sizeof(mcu_config) / sizeof(uint8_t));
    //mcu_config_func.config_save(MCU_CONFIG_ADDR, (__IO uint32_t *)(&mcu_config.system_config.device_id), sizeof(mcu_config) / sizeof(uint32_t));
    // Save the configuration to the flash.
    // 将配置保存到flash。
}

#endif // W25QXX_FLASH

/**
 * @brief Read the system configuration.
 * 读取系统配置。
 * @param system_config The system configuration.
 */
void system_config_read(mcu_system_config_t *system_config)
{
    *system_config = mcu_config.system_config;
}

/**
 * @brief Modify the system configuration.
 * 修改系统配置。
 * @param system_config The system configuration.
 */
void system_config_modify(mcu_system_config_t system_config)
{
    mcu_config.system_config = system_config;
}
#if NETWORK_CONFIG == 1
/**
 * @brief Read the network configuration.
 * 读取网络配置。
 * @param network_config The network configuration.
 */
void network_config_read(mcu_network_config_t *network_config)
{
    *network_config = mcu_config.network_config;
}

/**
 * @brief Modify the network configuration.
 * 修改网络配置。
 * @param network_config The network configuration.
 */
void network_config_modify(mcu_network_config_t network_config)
{
    mcu_config.network_config = network_config;
}
#endif
#if LORA_CONFIG == 1
/**
 * @brief Read the LORA configuration.
 * 读取LORA配置。
 * @param lora_config The LORA configuration.
 */
void lora_config_read(mcu_lora_config_t *lora_config)
{
    *lora_config = mcu_config.lora_config;
}

/**
 * @brief Modify the LORA configuration.
 * 修改LORA配置。
 * @param lora_config The LORA configuration.
 */
void lora_config_modify(mcu_lora_config_t lora_config)
{
    mcu_config.lora_config = lora_config;
}
#endif

uint8_t mcu_system_config_id_modify(uint16_t device_id)
{
    mcu_config.system_config.device_id = device_id;
    return CONFIG8_SUCCESS;
}

uint16_t mcu_system_config_id_read(void)
{
    return mcu_config.system_config.device_id;
}

#if NETWORK_CONFIG == 1
uint8_t mcu_network_config_tcp_server_ip_modify(char *tcp_server_ip)
{
    if(strlen(tcp_server_ip) > 16)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.tcp_server_ip, tcp_server_ip);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_tcp_server_ip_read(void)
{
    return mcu_config.network_config.tcp_server_ip;
}

uint8_t mcu_network_config_tcp_server_port_modify(char *tcp_server_port)
{
    if(strlen(tcp_server_port) > 6)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.tcp_server_port, tcp_server_port);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_tcp_server_port_read(void)
{
    return mcu_config.network_config.tcp_server_port;
}

uint8_t mcu_network_config_dhcp_flag_modify(uint8_t dhcp_flag)
{
    mcu_config.network_config.dhcp_flag = dhcp_flag;
    return CONFIG8_SUCCESS;
}

uint8_t mcu_network_config_dhcp_flag_read(void)
{
    return mcu_config.network_config.dhcp_flag;
}

uint8_t mcu_network_config_local_mask_modify(char *local_mask)
{
    if(strlen(local_mask) > 16)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.local_mask, local_mask);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_local_mask_read(void)
{
    return mcu_config.network_config.local_mask;
}

uint8_t mcu_network_config_local_gateway_modify(char *local_gateway)
{
    if(strlen(local_gateway) > 16)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.local_gateway, local_gateway);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_local_gateway_read(void)
{
    return mcu_config.network_config.local_gateway;
}

uint8_t mcu_network_config_static_local_ip_modify(char *static_local_ip)
{
    if(strlen(static_local_ip) > 16)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.static_local_ip, static_local_ip);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_static_local_ip_read(void)
{
    return mcu_config.network_config.static_local_ip;
}

uint8_t mcu_network_config_static_local_port_modify(char *static_local_port)
{
    if(strlen(static_local_port) > 6)
    {
        return CONFIG8_ERROR;
    }
    strcpy(mcu_config.network_config.static_local_port, static_local_port);
    return CONFIG8_SUCCESS;
}

char *mcu_network_config_static_local_port_read(void)
{
    return mcu_config.network_config.static_local_port;
}
#endif

#if LORA_CONFIG == 1
uint8_t mcu_lora_config_lora_addr_modify(uint16_t lora_addr)
{
    mcu_config.lora_config.lora_addr = lora_addr;
    return CONFIG8_SUCCESS;
}

uint16_t mcu_lora_config_lora_addr_read(void)
{
    return mcu_config.lora_config.lora_addr;
}

uint8_t mcu_lora_config_lora_channel_modify(uint16_t lora_channel)
{
    mcu_config.lora_config.lora_channel = lora_channel;
    return CONFIG8_SUCCESS;
}

uint16_t mcu_lora_config_lora_channel_read(void)
{
    return mcu_config.lora_config.lora_channel >= 100 ? 75 : mcu_config.lora_config.lora_channel;
}

uint8_t mcu_lora_config_lora_air_rate_modify(uint8_t lora_air_rate)
{
    mcu_config.lora_config.lora_air_rate = lora_air_rate;
    return CONFIG8_SUCCESS;
}

uint8_t mcu_lora_config_lora_air_rate_read(void)
{
    return mcu_config.lora_config.lora_air_rate;
}

uint8_t mcu_lora_config_lora_txwork_mode_modify(uint8_t lora_txwork_mode)
{
    mcu_config.lora_config.lora_txwork_mode = lora_txwork_mode;
    return CONFIG8_SUCCESS;
}

uint8_t mcu_lora_config_lora_txwork_mode_read(void)
{
    return mcu_config.lora_config.lora_txwork_mode;
}

uint8_t mcu_lora_config_lora_dt_addr_modify(uint16_t lora_dt_addr)
{
    mcu_config.lora_config.lora_dt_addr = lora_dt_addr;
    return CONFIG8_SUCCESS;
}

uint16_t mcu_lora_config_lora_dt_addr_read(void)
{
    return mcu_config.lora_config.lora_dt_addr;
}

uint8_t mcu_lora_config_lora_dt_channel_modify(uint16_t lora_dt_channel)
{
    mcu_config.lora_config.lora_dt_channel = lora_dt_channel;
    return CONFIG8_SUCCESS;
}

uint16_t mcu_lora_config_lora_dt_channel_read(void)
{
    return mcu_config.lora_config.lora_dt_channel;
}
#endif
