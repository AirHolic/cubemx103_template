#ifndef __MCU_CONFIG_H__
#define __MCU_CONFIG_H__

typedef void (*config_save_t)(uint32_t addr, uint32_t *data, uint32_t size);
typedef void (*config_load_t)(uint32_t addr, uint32_t *data, uint32_t size);

//配置项保存和加载函数
typedef struct
{
    config_save_t config_save;
    config_load_t config_load;
}mcu_config_func_t;

//设备ID
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

mcu_config_func_t mcu_config_func = {mcu_flash_write, mcu_flash_read};//配置项保存和加载函数

#define MCU_CONFIG_PAGE 120//配置存储页

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
// uint8_t mcu_system_config_timestamp_modify(uint32_t timestamp);
// uint32_t mcu_system_config_timestamp_read(void);

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
