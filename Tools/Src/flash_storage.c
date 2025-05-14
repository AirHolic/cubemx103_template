#include "flash_storage.h"
#include "main.h"
#include "crc_tools.h"
#include <string.h>

/* 私有全局变量 */

static FlashManager fmanager;
static StorageDriver storage_driver;

/* 私有函数声明 */

static uint8_t	validate_block(StorageBlock* block);// 校验数据块
static void format_storage_area(void);// 格式化存储区域
static void update_sector_usage(void);// 更新扇区使用计数
static void recycle_oldest_sector(void);// 回收最旧扇区
static uint32_t find_oldest_sector(void);// 查找最旧扇区

/**
 * @brief 驱动注册
 * 
 * @param driver 驱动接口结构体
 * @return void
 * @todo 考虑修改成设备注册模式
 */
void storage_driver_register(StorageDriver* driver) {
    memcpy(&storage_driver, driver, sizeof(StorageDriver));
}

/**
 * @brief 存储初始化
 * 
 * @param user_start 用户指定起始地址
 * @param user_end 用户指定结束地址
 * @return 1
 */
uint8_t storage_init(uint32_t user_start, uint32_t user_end) {
    if(!storage_driver.read || !storage_driver.write) return 0;

    memset(&fmanager, 0, sizeof(FlashManager));
    set_storage_range(user_start, user_end);

    /* 扫描现有数据 */
    uint32_t addr = fmanager.start_addr;
    while(addr < fmanager.end_addr) {
        uint8_t header[2];
        storage_driver.read(addr, header, 2);
        
        if(header[0] == 0xA5 && header[1] == 0x5A) {
            fmanager.current_addr = addr;
            storage_driver.read(addr + 2, (uint8_t*)&fmanager.sector_usage, 
                              sizeof(fmanager.sector_usage));
            break;
        }
        addr += W25QXX_SECTOR_SIZE;
    }

    /* 全新初始化 */
    if(addr >= fmanager.end_addr) {
        format_storage_area();
    }
    return 1;
}

/**
 * @brief 
 * 
 * @param block 
 * @return 0 :成功
 * @return 1 :失败
 */
uint8_t write_data_block(StorageBlock* block) {
    /* 计算校验值 */
    block->crc = crc8((uint8_t*)block, sizeof(StorageBlock)-1);
    
    /* 空间检查 */
    if(fmanager.current_addr + sizeof(StorageBlock) > fmanager.end_addr) {
        recycle_oldest_sector();
    }
    
    /* 执行写入 */
    if(!storage_driver.write(fmanager.current_addr, 
                           (uint8_t*)block, 
                           sizeof(StorageBlock))) {
        return 0;
    }
    
    /* 更新地址指针 */
    fmanager.current_addr += sizeof(StorageBlock);
    update_sector_usage();
    return 1;
}

/**
 * @brief 读取指定地址数据
 * 
 * @param addr 地址
 * @param out 
 * @return uint8_t 
 */
uint8_t read_by_address(uint32_t addr, StorageBlock* out) {
    if(addr < fmanager.start_addr || addr >= fmanager.end_addr) {
        return 0;
    }
    
    storage_driver.read(addr, (uint8_t*)out, sizeof(StorageBlock));
    return validate_block(out);
}

/**
 * @brief 批量读取最新数据
 * 
 * @param num 读取数量
 * @param out 读取数据存储区,数组长度应不小于num
 * @return uint8_t 
 */
uint8_t read_by_new_num(uint8_t num, StorageBlock* out)
{
    uint32_t addr = fmanager.current_addr;
    uint8_t count = 0;
    while(count < num && addr > fmanager.start_addr) {
        StorageBlock block;
        storage_driver.read(addr, (uint8_t*)&block, sizeof(block));
        
        if(validate_block(&block)) {
            memcpy(&out[count], &block, sizeof(StorageBlock));
            count++;
        }
        
        addr -= sizeof(StorageBlock);
    }
    return count;
}

/**
 * @brief 发现指定时间范围内的数据
 * 
 * @param start_time 
 * @param end_time 
 * @return uint32_t 
 */
uint32_t find_by_time_range(uint32_t start_time, uint32_t end_time) {
    uint32_t addr = fmanager.start_addr;
    while(addr < fmanager.current_addr) {
        StorageBlock block;
        storage_driver.read(addr, (uint8_t*)&block, sizeof(block));
        
        if(block.timestamp >= start_time && 
           block.timestamp <= end_time &&
           validate_block(&block)) {
            return addr;
        }
        
        addr += sizeof(StorageBlock);
    }
    return 0xFFFFFFFF;
}

/**
 * @brief 设置存储范围
 * 
 * @param start 
 * @param end 
 */
void set_storage_range(uint32_t start, uint32_t end) {
    // 地址对齐校验
    // assert((start % W25QXX_SECTOR_SIZE) == 0);
    // assert((end - start) >= (2 * W25QXX_SECTOR_SIZE));
    
    fmanager.start_addr = start;
    fmanager.end_addr = end;
    fmanager.current_addr = start;
}

/**
 * @brief 获取当前写入位置
 * 
 * @return uint32_t 
 */
uint32_t get_current_position(void) {
    return fmanager.current_addr;
}

/* 私有函数实现 */

static void format_storage_area(void) {
    /* 擦除起始扇区 */
    storage_driver.erase_sector(fmanager.start_addr);
    
    /* 写入初始标记 */
    uint8_t header[2] = {0xA5, 0x5A};
    storage_driver.write(fmanager.start_addr, header, 2);
    
    fmanager.current_addr = fmanager.start_addr + 2;
    fmanager.oldest_sector = fmanager.start_addr;
}

static void update_sector_usage(void) {
    uint32_t sector = fmanager.current_addr / W25QXX_SECTOR_SIZE;
    fmanager.sector_usage[sector]++;
    
    if(fmanager.sector_usage[sector] >= 0xFF) {
        recycle_oldest_sector();
    }
    
    storage_driver.write(fmanager.start_addr + 2, 
                       (uint8_t*)&fmanager.sector_usage, 
                       sizeof(fmanager.sector_usage));
}

static void recycle_oldest_sector(void) {
    uint32_t oldest = find_oldest_sector();
    
    /* 擦除旧扇区 */
    storage_driver.erase_sector(oldest);
    
    /* 更新管理信息 */
    fmanager.oldest_sector = find_oldest_sector();
    fmanager.current_addr = oldest;
    
    /* 写入新扇区标记 */
    uint8_t header[2] = {0xA5, 0x5A};
    storage_driver.write(oldest, header, 2);
}

static uint8_t validate_block(StorageBlock* block) {
    uint8_t calc_crc = crc8((uint8_t*)block, sizeof(StorageBlock)-1);
    return (block->crc == calc_crc) && 
           (block->timestamp > 1600000000);
}

static uint32_t find_oldest_sector(void) {
    uint32_t oldest = fmanager.start_addr;
    for(uint32_t i = 0; i < 256; i++) {
        if(fmanager.sector_usage[i] == 0) {
            oldest = fmanager.start_addr + i * W25QXX_SECTOR_SIZE;
            break;
        }
        
        if(fmanager.sector_usage[i] < fmanager.sector_usage[oldest]) {
            oldest = fmanager.start_addr + i * W25QXX_SECTOR_SIZE;
        }
    }
    return oldest;
}
