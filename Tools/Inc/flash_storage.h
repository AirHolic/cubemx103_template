/**
 * @file flash_storage.h
 * @author AirHolic
 * @brief 存储日志数据到Flash(传感器数据等)
 * @version 0.1
 * @date 2025-03-13
 * @todo 待进一步完善
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __FLASH_STORAGE_H__
#define __FLASH_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "w25qxx_spi_driver.h"

#define STORAGE_BLOCK_DATA_SIZE 12 // 数据块内容大小,后续可考虑存入配置

/* 驱动接口结构体 */
typedef struct {//1:成功 0:失败
    uint8_t (*read)(uint32_t addr, uint8_t* buf, uint32_t len);
    uint8_t (*write)(uint32_t addr, uint8_t* buf, uint32_t len);
    uint8_t (*erase_sector)(uint32_t sector_addr);
    uint8_t (*erase)(void);
} StorageDriver;

/* 数据块结构 */
#pragma pack(push, 1)
typedef struct {
    uint16_t  length;                           // 数据长度（2字节）
    uint16_t  type;                             // 数据类型（2字节）
    uint32_t  id;                               // 数据标识（4字节）
    uint8_t   data[STORAGE_BLOCK_DATA_SIZE];    // 数据内容（12字节）//考虑使用union 以节省空间 
    uint32_t  timestamp;                        // Unix时间戳（4字节）
    uint8_t   crc;                              // CRC8校验（1字节）
} StorageBlock;                                 // 总长度19字节
#pragma pack(pop)

/* 存储管理器状态 */
typedef struct {
    uint32_t  start_addr;        // 用户指定起始地址
    uint32_t  end_addr;          // 用户指定结束地址
    uint32_t  current_addr;      // 当前写入地址
    uint32_t  oldest_sector;     // 最早数据所在扇区
    uint16_t  sector_usage[256]; // 扇区使用计数
} FlashManager;

/* 公有接口 */ //后续考虑添加最新数据读取接口

void storage_driver_register(StorageDriver* driver);
uint8_t storage_init(uint32_t user_start, uint32_t user_end);
uint8_t write_data_block(StorageBlock* block);
uint8_t read_by_address(uint32_t addr, StorageBlock* out);
uint8_t read_by_new_num(uint8_t num, StorageBlock* out);
uint32_t find_by_time_range(uint32_t start_time, uint32_t end_time);
void set_storage_range(uint32_t start, uint32_t end);
uint32_t get_current_position(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_STORAGE_H__ */
