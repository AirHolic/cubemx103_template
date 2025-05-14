/**
 * @file mcu_config.h
 * @author AirHolic
 * @brief mcu配置文件,用于存储设备配置,网络配置,LORA配置等
 * @version 0.5
 * @date 2025-03-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __MCU_CONFIG_H__
#define __MCU_CONFIG_H__

#define W25QXX_FLASH
#define NETWORK_CONFIG 1
#define CONFIG_NULL 0xFFFFFFFF
#define CONFIG32_SUCCESS 0x00000000
#define CONFIG32_ERROR 0xFFFFFFFF
#define CONFIG16_SUCCESS 0x0000
#define CONFIG16_ERROR 0xFFFF
#define CONFIG8_SUCCESS 0x00
#define CONFIG8_ERROR 0xFF

#ifdef MCU_FLASH

#define MCU_CONFIG_ADDR FLASH_PAGEx(19)//配置存储地址

typedef void (*config_save_t)(uint32_t addr, uint32_t *data, uint32_t size);
typedef void (*config_load_t)(uint32_t addr, uint32_t *data, uint32_t size);

#endif

#ifdef W25QXX_FLASH

#include "w25qxx_spi_driver.h"
#define MCU_CONFIG_ADDR W25QXX_SECTOR_ADDR(0)//配置存储地址

typedef void (*config_save_t)(uint8_t *data, uint32_t addr, uint32_t size);
typedef void (*config_load_t)(uint8_t *data, uint32_t addr, uint32_t size);

#endif

//配置项保存和加载函数
typedef struct
{
    config_save_t config_save;
    config_load_t config_load;
}mcu_config_func_t;

//设备配置
typedef struct
{
    uint16_t device_id;
} mcu_system_config_t;

#if NETWORK_CONFIG == 1
//网络配置
typedef struct
{    
    uint8_t dhcp_flag;
    char tcp_server_ip[16];
    char tcp_server_port[6];
    char local_mask[16];
    char local_gateway[16];
    char static_local_ip[16];
    char static_local_port[6];
} mcu_network_config_t;
#endif

#if LORA_CONFIG == 1
//LORA配置
typedef struct
{
    uint8_t lora_air_rate;
    uint8_t lora_txwork_mode;
    uint16_t lora_addr;
    uint16_t lora_channel;
    uint16_t lora_dt_addr;
    uint16_t lora_dt_channel;
} mcu_lora_config_t;
#endif

void mcu_config_read_from_flash(void);
void mcu_config_save_to_flash(void);
void system_config_read(mcu_system_config_t *system_config);
void system_config_modify(mcu_system_config_t system_config);
#if NETWORK_CONFIG == 1
void network_config_read(mcu_network_config_t *network_config);
void network_config_modify(mcu_network_config_t network_config);
#endif
#if LORA_CONFIG == 1
void lora_config_read(mcu_lora_config_t *lora_config);
void lora_config_modify(mcu_lora_config_t lora_config);
#endif

/* 配置结构体单成员修改与读取函数 */

uint8_t mcu_system_config_id_modify(uint16_t device_id);
uint16_t mcu_system_config_id_read(void);

#if NETWORK_CONFIG == 1
uint8_t mcu_network_config_tcp_server_ip_modify(char *tcp_server_ip);
char *mcu_network_config_tcp_server_ip_read(void);
uint8_t mcu_network_config_tcp_server_port_modify(char *tcp_server_port);
char *mcu_network_config_tcp_server_port_read(void);
uint8_t mcu_network_config_dhcp_flag_modify(uint8_t dhcp_flag);
uint8_t mcu_network_config_dhcp_flag_read(void);
uint8_t mcu_network_config_local_mask_modify(char *local_mask);
char *mcu_network_config_local_mask_read(void);
uint8_t mcu_network_config_local_gateway_modify(char *local_gateway);
char *mcu_network_config_local_gateway_read(void);
uint8_t mcu_network_config_static_local_ip_modify(char *static_local_ip);
char *mcu_network_config_static_local_ip_read(void);
uint8_t mcu_network_config_static_local_port_modify(char *static_local_port);
char *mcu_network_config_static_local_port_read(void);
#endif

#if LORA_CONFIG == 1
uint8_t mcu_lora_config_lora_addr_modify(uint16_t lora_addr);
uint16_t mcu_lora_config_lora_addr_read(void);
uint8_t mcu_lora_config_lora_channel_modify(uint16_t lora_channel);
uint16_t mcu_lora_config_lora_channel_read(void);
uint8_t mcu_lora_config_lora_air_rate_modify(uint8_t lora_air_rate);
uint8_t mcu_lora_config_lora_air_rate_read(void);
uint8_t mcu_lora_config_lora_txwork_mode_modify(uint8_t lora_txwork_mode);
uint8_t mcu_lora_config_lora_txwork_mode_read(void);
uint8_t mcu_lora_config_lora_dt_addr_modify(uint16_t lora_dt_addr);
uint16_t mcu_lora_config_lora_dt_addr_read(void);
uint8_t mcu_lora_config_lora_dt_channel_modify(uint16_t lora_dt_channel);
uint16_t mcu_lora_config_lora_dt_channel_read(void);
#endif

#endif /* _MCU_CONFIG_H__ */
