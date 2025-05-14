/**
 * @file mcu_timingtask.c
 * @author AirHolic
 * @brief Try to optimize the function implementation in mcu_timingtask.c
 * @version 0.1
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"
#include "rtc_utx.h"
#include "rtc.h"
#include "sys_delay.h"
#include "string.h"
#include "usart.h"
#include "mcu_timingtask.h"

#if TIMINGTASK_STORAGE_IN_MCUFLASH == 1
#include "mcu_flash.h"
#else
#include "w25qxx_spi_driver.h"
#endif

#if MCU_TIMINGTASK_DEBUG == 1
#define MCU_TIMINGTASK_LOG(fmt, ...) printf("[MCU_TIMINGTASK] " fmt, ##__VA_ARGS__)
#else
#define MCU_TIMINGTASK_LOG(...)
#endif

// 全局变量定义
static mcu_timingtask_func_t mcu_timingtask_func;//flash函数指针
static uint8_t mcu_timingtask_num = 0;//当前任务数量
/* 
 * 读取缓存，统一使用规则:
 * - 索引0: 用于存储主要任务数据（主要操作对象）
 * - 索引1: 用于存储次要任务数据（用于比较或临时存储）
 */
static MCU_TIMINGTASK_T mcu_timingtask_read_cache[2];//读取缓存
static uint8_t mcu_timingtask_execute_sort[TIMINGTASK_NUM];//当日执行排序
static uint8_t mcu_timingtask_execute_index = 0;//当日执行索引
static uint32_t mcu_timingtask_current_addr = TIMINGTASK_ADDR;//当前存储地址

// 常量和宏定义
#define MCU_TIMINGTASK_T_SIZE sizeof(MCU_TIMINGTASK_T)
#define MCU_TIMINGTASK_T_INDEX(x) (mcu_timingtask_current_addr + x * MCU_TIMINGTASK_T_SIZE)
#define MCU_RUNNING_CYCLE_UNIT_JUDGE(running_cycle_unit, weekdate) ((running_cycle_unit >> weekdate) & 0x01)
#define WEEKTRASNFORM(weekdate) (weekdate == 0 ? 7 : weekdate)

static uint8_t check_task_validity(MCU_TIMINGTASK_T *task);

// 基础功能实现
static uint8_t set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
    return time_set_alarm(hour, min, sec);
}

// 存储相关函数实现
#if TIMINGTASK_STORAGE_IN_MCUFLASH == 1
static void mcu_timingtask_read(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len)
{
    #if RTOS == 1
    // 计算需要的uint32_t数组大小，并向上取整
    uint32_t word_count = (len + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    uint32_t *data32 = pvPortMalloc(word_count * sizeof(uint32_t));
    if(data32 != NULL) {
        mcu_flash_read(addr, data32, word_count);
        memcpy(data, data32, len);
        vPortFree(data32);
    } else {
        MCU_TIMINGTASK_LOG("Memory allocation failed in mcu_timingtask_read\n");
    }
    #else
    // 计算需要的uint32_t数组大小，并向上取整
    uint32_t word_count = (len + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    uint32_t data32[word_count];
    mcu_flash_read(addr, data32, word_count);
    memcpy(data, data32, len);
    #endif
}

static void mcu_timingtask_nocheck_write(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t size)
{
    #if RTOS == 1
    // 计算需要的uint32_t数组大小，并向上取整
    uint32_t word_count = (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    uint32_t *data32 = pvPortMalloc(word_count * sizeof(uint32_t));
    if(data32 != NULL) {
        memcpy(data32, data, size);
        mcu_flash_nocheck_write(addr, data32, word_count);
        vPortFree(data32);
    } else {
        MCU_TIMINGTASK_LOG("Memory allocation failed in mcu_timingtask_nocheck_write\n");
    }
    #else
    // 计算需要的uint32_t数组大小，并向上取整
    uint32_t word_count = (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    uint32_t data32[word_count];
    memcpy(data32, data, size);
    mcu_flash_nocheck_write(addr, data32, word_count);
    #endif
}

#else
static void mcu_timingtask_read(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len)
{
    if (data == NULL) {
        MCU_TIMINGTASK_LOG("Invalid data pointer in mcu_timingtask_read\n");
        return;
    }

    #if RTOS == 1
    uint8_t *data8 = pvPortMalloc(len);
    if (data8 != NULL) {
        W25QXX_ReadBuffer(data8, addr, len);
        memcpy(data, data8, len);
        vPortFree(data8);
    } else {
        MCU_TIMINGTASK_LOG("Memory allocation failed in mcu_timingtask_read\n");
    }
    #else
    // 使用静态缓冲区，确保安全使用
    if (len <= MCU_TIMINGTASK_T_SIZE) {
        uint8_t data8[MCU_TIMINGTASK_T_SIZE];
        W25QXX_ReadBuffer(data8, addr, MCU_TIMINGTASK_T_SIZE);
        memcpy(data, data8, MCU_TIMINGTASK_T_SIZE);
    } else {
        MCU_TIMINGTASK_LOG("Data too large in mcu_timingtask_read: %d > %d\n", 
                         len, MCU_TIMINGTASK_T_SIZE);
    }
    #endif
}

static void mcu_timingtask_nocheck_write(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t size)
{
    if (data == NULL) {
        MCU_TIMINGTASK_LOG("Invalid data pointer in mcu_timingtask_nocheck_write\n");
        return;
    }
    
    #if RTOS == 1
    uint8_t *data8 = pvPortMalloc(size);
    if (data8 != NULL) {
        memcpy(data8, data, size);
        W25QXX_WriteNoErase(data8, addr, size);
        vPortFree(data8);
    } else {
        MCU_TIMINGTASK_LOG("Memory allocation failed in mcu_timingtask_nocheck_write\n");
    }
    #else
    // 使用静态缓冲区，确保安全使用
    if (size <= MCU_TIMINGTASK_T_SIZE) {
        uint8_t data8[MCU_TIMINGTASK_T_SIZE];
        memcpy(data8, data, MCU_TIMINGTASK_T_SIZE);
        W25QXX_WriteNoErase(data8, addr, MCU_TIMINGTASK_T_SIZE);
        
    } else {
        MCU_TIMINGTASK_LOG("Data too large in mcu_timingtask_nocheck_write: %d > %d\n", 
                         size, MCU_TIMINGTASK_T_SIZE);
    }
    #endif
}
#endif

// 任务管理相关函数实现
void mcu_timingtask_init(void)
{
    #if TIMINGTASK_STORAGE_IN_MCUFLASH == 1
    mcu_timingtask_func.timingtask_erase = mcu_flash_erase;
    #else
    mcu_timingtask_func.timingtask_erase = W25QXX_Erase;
    #endif
    
    mcu_timingtask_func.timingtask_load = mcu_timingtask_read;
    mcu_timingtask_func.timingtask_save = mcu_timingtask_nocheck_write;
    mcu_timingtask_func.timingtask_set_alarm = set_alarm;
    mcu_timingtask_func.timingtask_get_time = get_time;
    mcu_timingtask_func.timingtask_get_date = get_date;
    
    // 初始化任务数量，使用全局缓存
    uint8_t primary_num = 0;
    uint8_t backup_num = 0;
    
    // 检查主存储区任务数量
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        mcu_timingtask_func.timingtask_load(TIMINGTASK_ADDR + i * MCU_TIMINGTASK_T_SIZE, 
                                          &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF) {
            primary_num++;
        }
    }
    
    // 检查备份区任务数量
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        mcu_timingtask_func.timingtask_load(TIMINGTASK_BACKUP_ADDR + i * MCU_TIMINGTASK_T_SIZE, 
                                          &mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[1].timingtask_id != 0xFFFFFFFF) {
            backup_num++;
        }
    }
    
    // 选择有效任务数较多的存储区
    if(primary_num >= backup_num) {
        mcu_timingtask_current_addr = TIMINGTASK_ADDR;
        mcu_timingtask_num = primary_num;
    } else {
        mcu_timingtask_current_addr = TIMINGTASK_BACKUP_ADDR;
        mcu_timingtask_num = backup_num;
    }
}

/**
 * @brief 添加定时任务
 * 
 * @param timingtask 任务结构体指针
 * @return uint8_t 1-添加成功 0-添加失败 
 */
uint8_t mcu_timingtask_add(MCU_TIMINGTASK_T *timingtask)
{
    if(mcu_timingtask_num >= TIMINGTASK_NUM)
    {
        return 0;
    }
    
    if(check_task_validity(timingtask) == 0)
    {
        MCU_TIMINGTASK_LOG("Task validity check failed\n");
        return 0;
    }
    
    // 查找空位
    uint8_t empty_index = 0xFF;
    
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++)
    {
        // 使用缓存索引1检查空位和ID冲突
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[1], 
                                          MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[1].timingtask_id == 0xFFFFFFFF)
        {
            empty_index = i;
            MCU_TIMINGTASK_LOG("Found empty slot at index %d\n", empty_index);
            break;
        }
        if(mcu_timingtask_read_cache[1].timingtask_id == timingtask->timingtask_id && 
           timingtask->timingtask_id != 0xFFFFFFFF)
        {
            // 已存在相同ID的任务
            MCU_TIMINGTASK_LOG("Task ID %x already exists\n", timingtask->timingtask_id);
            return 0;
        }
    }
    
    if(empty_index == 0xFF)
    {
        MCU_TIMINGTASK_LOG("No empty slot available\n");
        return 0;
    }
    
    // 保存新任务到缓存索引0，然后写入到Flash
    memcpy(&mcu_timingtask_read_cache[0], timingtask, sizeof(MCU_TIMINGTASK_T));
    mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(empty_index), 
                                       &mcu_timingtask_read_cache[0], 
                                       sizeof(MCU_TIMINGTASK_T));
    mcu_timingtask_num++;
    MCU_TIMINGTASK_LOG("Task ID %x added at index %d\n", timingtask->timingtask_id, empty_index);
    MCU_TIMINGTASK_LOG("Current task count: %d\n", mcu_timingtask_num);
    return 1;
}

/**
 * @brief 根据任务ID查找任务索引
 * 
 * @param timingtask_id 任务ID
 * @return uint8_t 任务索引 0xFF表示未找到 
 */
uint8_t mcu_search_timingtask_id(uint32_t timingtask_id)
{
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++)
    {
        // 使用缓存索引0存储查找的任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[0].timingtask_id == timingtask_id)
        {
            return i;
        }
    }
    
    return 0xFF;
}

/**
 * @brief 检查任务是否在当天需要执行
 * 
 * @param task 任务结构体指针
 * @return uint8_t 1-需要执行 0-不需执行 
 */
static uint8_t is_task_executable_today(MCU_TIMINGTASK_T *task)
{
    uint8_t date, weekday;

    // 检查任务是否在有效期内
    time_t current_time = get_uts();
    MCU_TIMINGTASK_LOG("Current time: %lu\n", current_time);
    if(task->validity_time > 0 && current_time < task->validity_time) {
        return 0;  // 未到有效时间
    }
    
    if(task->invalidty_time > 0 && current_time >= task->invalidty_time) {
        return 0;  // 已超过失效时间
    }
    
    // 获取当前日期和星期
    mcu_timingtask_func.timingtask_get_date(&date, &weekday);
    MCU_TIMINGTASK_LOG("Current date: %d, weekday: %d\n", date, weekday);
    
    // 根据运行周期判断是否今日执行
    switch(task->timingtask_running_cycle) {
        case TIMINGTASK_RUNNING_CYCLE_EVERYDAY:
            return 1;  // 每天执行
            
        case TIMINGTASK_RUNNING_CYCLE_EVERYWEEK:
            // 检查周几位是否设置
            weekday = WEEKTRASNFORM(weekday);  // 转换周日为7
            return MCU_RUNNING_CYCLE_UNIT_JUDGE(task->timingtask_running_cycle_unit, weekday);
            
        case TIMINGTASK_RUNNING_CYCLE_EVERYMONTH:
            // 检查日期位是否设置
            if(date > 0 && date <= 31) {
                return MCU_RUNNING_CYCLE_UNIT_JUDGE(task->timingtask_running_cycle_unit, date);
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

/**
 * @brief 当日有效任务排序及查找下一个要执行的任务，优化内存使用
 * 
 * @param void
 * @return uint8_t 当日有效任务数 0x00表示无有效任务 
 */
uint8_t mcu_timingtask_sorting(void)
{
    uint8_t valid_task_count = 0;
    uint8_t task_indices[TIMINGTASK_NUM];
    uint8_t task_hours[TIMINGTASK_NUM];
    uint8_t task_mins[TIMINGTASK_NUM];
    uint8_t task_secs[TIMINGTASK_NUM];
    
    // 预先获取当前时间和日期，避免重复获取
    time_t current_time = get_uts();
    uint8_t current_date, current_weekday;
    mcu_timingtask_func.timingtask_get_date(&current_date, &current_weekday);
    current_weekday = WEEKTRASNFORM(current_weekday);  // 提前转换周日为7
    
    MCU_TIMINGTASK_LOG("Current time: %lu, date: %d, weekday: %d\n", 
                       current_time, current_date, current_weekday);
    
    // 第一轮：找出今日有效的任务，使用 mcu_timingtask_num 作为实际任务数量约束
    uint8_t found_count = 0;
    
    for(uint8_t i = 0; i < TIMINGTASK_NUM && found_count < mcu_timingtask_num; i++) {
        // 使用全局缓存索引0读取任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        // 检查任务ID是否有效
        if(mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF) {
            found_count++; // 找到一个有效任务
            
            MCU_TIMINGTASK_LOG("SORT TASK ID: %x, Hour: %d, Min: %d, Sec: %d\n", 
                             mcu_timingtask_read_cache[0].timingtask_id, 
                             mcu_timingtask_read_cache[0].timingtask_hour, 
                             mcu_timingtask_read_cache[0].timingtask_min, 
                             mcu_timingtask_read_cache[0].timingtask_sec);
            MCU_TIMINGTASK_LOG("Task validity time: %lu, invalid time: %lu\n", 
                             mcu_timingtask_read_cache[0].validity_time, 
                             mcu_timingtask_read_cache[0].invalidty_time);
            
            // 检查任务是否在有效期内
            if(mcu_timingtask_read_cache[0].validity_time > 0 && 
               current_time < mcu_timingtask_read_cache[0].validity_time) {
                MCU_TIMINGTASK_LOG("Task ID %x not valid yet\n", 
                                 mcu_timingtask_read_cache[0].timingtask_id);
                continue;  // 未到有效时间
            }
            
            if(mcu_timingtask_read_cache[0].invalidty_time > 0 && 
               current_time >= mcu_timingtask_read_cache[0].invalidty_time) {
                MCU_TIMINGTASK_LOG("Task ID %x expired\n", 
                                 mcu_timingtask_read_cache[0].timingtask_id);
                continue;  // 已超过失效时间
            }
            
            // 根据运行周期判断是否今日执行
            uint8_t is_executable = 0;
            
            switch(mcu_timingtask_read_cache[0].timingtask_running_cycle) {
                case TIMINGTASK_RUNNING_CYCLE_EVERYDAY:
                    is_executable = 1;  // 每天执行
                    MCU_TIMINGTASK_LOG("Task ID %x will run today\n", 
                                     mcu_timingtask_read_cache[0].timingtask_id);
                    break;
                    
                case TIMINGTASK_RUNNING_CYCLE_EVERYWEEK:
                    // 检查周几位是否设置
                    is_executable = MCU_RUNNING_CYCLE_UNIT_JUDGE(
                        mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, 
                        current_weekday);
                    MCU_TIMINGTASK_LOG("Task ID %x will run on weekday %d\n", 
                                     mcu_timingtask_read_cache[0].timingtask_id, 
                                     current_weekday);
                    break;
                    
                case TIMINGTASK_RUNNING_CYCLE_EVERYMONTH:
                    // 检查日期位是否设置
                    if(current_date > 0 && current_date <= 31) {
                        is_executable = MCU_RUNNING_CYCLE_UNIT_JUDGE(
                            mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, 
                            current_date);
                        MCU_TIMINGTASK_LOG("Task ID %x will run on date %d\n", 
                                         mcu_timingtask_read_cache[0].timingtask_id, 
                                         current_date);
                    }
                    break;
                    
                default:
                    break;
            }
            
            if(is_executable) {
                task_indices[valid_task_count] = i;
                task_hours[valid_task_count] = mcu_timingtask_read_cache[0].timingtask_hour;
                task_mins[valid_task_count] = mcu_timingtask_read_cache[0].timingtask_min;
                task_secs[valid_task_count] = mcu_timingtask_read_cache[0].timingtask_sec;
                valid_task_count++;
            }
        }
    }

    // 冒泡排序，按时间排序，只排序索引和时间值，不需要再次读取完整任务
    for(uint8_t i = 0; i < valid_task_count - 1; i++) {
        for(uint8_t j = 0; j < valid_task_count - 1 - i; j++) {
            if((task_hours[j] > task_hours[j+1]) || 
               (task_hours[j] == task_hours[j+1] && task_mins[j] > task_mins[j+1]) ||
               (task_hours[j] == task_hours[j+1] && task_mins[j] == task_mins[j+1] && 
                task_secs[j] > task_secs[j+1])) {
                
                // 交换索引
                uint8_t temp_index = task_indices[j];
                task_indices[j] = task_indices[j+1];
                task_indices[j+1] = temp_index;
                
                // 交换时间值
                uint8_t temp_hour = task_hours[j];
                task_hours[j] = task_hours[j+1];
                task_hours[j+1] = temp_hour;
                
                uint8_t temp_min = task_mins[j];
                task_mins[j] = task_mins[j+1];
                task_mins[j+1] = temp_min;
                
                uint8_t temp_sec = task_secs[j];
                task_secs[j] = task_secs[j+1];
                task_secs[j+1] = temp_sec;
            }
        }
    }
    
    // 保存排序结果
    for(uint8_t i = 0; i < valid_task_count; i++) {
        mcu_timingtask_execute_sort[i] = task_indices[i];
    }
    MCU_TIMINGTASK_LOG("Valid task count: %d\n", valid_task_count);

    return valid_task_count;
}

/**
 * @brief 设置下一个要执行的任务索引，优化内存使用
 * 
 * @return uint8_t 下一个要执行任务的索引，0xFF表示无有效任务
 */
uint8_t mcu_timingtask_execute_index_set(void)
{
    if(mcu_timingtask_num == 0) {
        return 0xFF;
    }

    uint8_t hour, min, sec;
    mcu_timingtask_func.timingtask_get_time(&hour, &min, &sec);
    MCU_TIMINGTASK_LOG("index time: %02d:%02d:%02d\n", hour, min, sec);
    // 先获取今日有效的排序任务
    uint8_t sorted_count = mcu_timingtask_sorting();
    
    if(sorted_count == 0) {
        return 0xFF;  // 无今日有效任务
    }
    
    // 找出下一个要执行的任务
    for(uint8_t i = 0; i < sorted_count; i++) {
        // 使用全局缓存索引0读取任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_execute_sort[i]), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        MCU_TIMINGTASK_LOG("Checking task ID: %x, Hour: %d, Min: %d, Sec: %d\n", 
                         mcu_timingtask_read_cache[0].timingtask_id, 
                         mcu_timingtask_read_cache[0].timingtask_hour, 
                         mcu_timingtask_read_cache[0].timingtask_min, 
                         mcu_timingtask_read_cache[0].timingtask_sec);
        
        if(mcu_timingtask_read_cache[0].timingtask_hour > hour || 
           (mcu_timingtask_read_cache[0].timingtask_hour == hour && 
            mcu_timingtask_read_cache[0].timingtask_min > min) ||
           (mcu_timingtask_read_cache[0].timingtask_hour == hour && 
            mcu_timingtask_read_cache[0].timingtask_min == min && 
            mcu_timingtask_read_cache[0].timingtask_sec > sec)) {
            
            mcu_timingtask_execute_index = i;
            return mcu_timingtask_execute_index;
        }
    }
    
    // 如果当前时间已经超过所有任务时间,则设置一个0点的任务,用于第二天重新设置任务
    mcu_timingtask_execute_index = TIMINGTASK_NUM+1;
    return mcu_timingtask_execute_index;
}

/**
 * @brief 根据任务id删除定时任务，支持批量删除，返回实际删除的任务数量
 * 
 *        删除任务时，先将未删除的任务复制到备份区，然后擦除原存储区，更新任务数量和当前存储地址
 * 
 * @param timingtask_id 任务id数组
 * @param delete_count 任务id数量
 * @return uint8_t 实际删除的任务数量
 */
uint8_t mcu_timingtask_delete(uint32_t *timingtask_id, uint8_t delete_count)
{
    uint8_t deleted_count = 0;
    uint32_t target_addr;
    
    // 确定目标备份地址
    target_addr = (mcu_timingtask_current_addr == TIMINGTASK_ADDR) ? 
                  TIMINGTASK_BACKUP_ADDR : TIMINGTASK_ADDR;
    
    // 先将未删除的任务复制到备份区
    uint8_t write_index = 0;
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        // 使用缓存索引0读取当前任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        if(mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF) {
            uint8_t should_delete = 0;
            for(uint8_t j = 0; j < delete_count; j++) {
                if(mcu_timingtask_read_cache[0].timingtask_id == timingtask_id[j]) {
                    should_delete = 1;
                    deleted_count++;
                    break;
                }
            }
            
            if(!should_delete) {
                mcu_timingtask_func.timingtask_save(target_addr + write_index * MCU_TIMINGTASK_T_SIZE,
                                                  &mcu_timingtask_read_cache[0], 
                                                  MCU_TIMINGTASK_T_SIZE);
                write_index++;
            }
        }
    }
    
    if(deleted_count > 0) {
        // 擦除原存储区
        mcu_timingtask_func.timingtask_erase(mcu_timingtask_current_addr, 
                                            TIMINGTASK_NUM * MCU_TIMINGTASK_T_SIZE);
        
        // 更新任务数量和当前存储地址
        mcu_timingtask_num -= deleted_count;
        mcu_timingtask_current_addr = target_addr;
    }
    
    return deleted_count;
}

/**
 * @brief 删除无效任务，使用与常规删除相同的备份机制
 * 
 * @return uint8_t 删除的任务数量
 */
uint8_t mcu_timingtask_delete_invalid(void)
{
    uint8_t deleted_count = 0;
    uint32_t target_addr;
    time_t current_timestamp = get_uts();
    
    // 确定目标备份地址
    target_addr = (mcu_timingtask_current_addr == TIMINGTASK_ADDR) ? 
                  TIMINGTASK_BACKUP_ADDR : TIMINGTASK_ADDR;
    
    // 先将有效期内的任务复制到备份区
    uint8_t write_index = 0;
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        // 使用缓存索引0读取当前任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        if(mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF) {
            // 检查是否已过期
            if(mcu_timingtask_read_cache[0].invalidty_time != 0 && 
               current_timestamp >= mcu_timingtask_read_cache[0].invalidty_time) {
                deleted_count++;
                MCU_TIMINGTASK_LOG("Deleting expired task ID: %x\n", 
                                 mcu_timingtask_read_cache[0].timingtask_id);
            } else {
                // 未过期，复制到备份区
                mcu_timingtask_func.timingtask_save(target_addr + write_index * MCU_TIMINGTASK_T_SIZE,
                                                  &mcu_timingtask_read_cache[0], 
                                                  MCU_TIMINGTASK_T_SIZE);
                write_index++;
            }
        }
    }
    
    if(deleted_count > 0) {
        // 擦除原存储区
        mcu_timingtask_func.timingtask_erase(mcu_timingtask_current_addr, 
                                           TIMINGTASK_NUM * MCU_TIMINGTASK_T_SIZE);
        
        // 更新任务数量和当前存储地址
        mcu_timingtask_num -= deleted_count;
        mcu_timingtask_current_addr = target_addr;
    }
    
    return deleted_count;
}

// 报警设置相关函数实现
void mcu_timingtask_set_alarm(void)
{
    if(mcu_timingtask_num == 0) {
        return;
    }
    
    uint8_t next_task_index = mcu_timingtask_execute_index_set();
    MCU_TIMINGTASK_LOG("Next task index: %d\n", next_task_index);
    
    if(next_task_index != 0xFF && next_task_index < TIMINGTASK_NUM) {
        // 使用全局缓存索引0读取任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_execute_sort[next_task_index]), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        MCU_TIMINGTASK_LOG("Task ID: %X\n", mcu_timingtask_read_cache[0].timingtask_id);
        MCU_TIMINGTASK_LOG("Set alarm: %02d:%02d:%02d\n",
                         mcu_timingtask_read_cache[0].timingtask_hour,
                         mcu_timingtask_read_cache[0].timingtask_min,
                         mcu_timingtask_read_cache[0].timingtask_sec);
        
        mcu_timingtask_func.timingtask_set_alarm(mcu_timingtask_read_cache[0].timingtask_hour,
                                                mcu_timingtask_read_cache[0].timingtask_min,
                                                mcu_timingtask_read_cache[0].timingtask_sec);
    }
    else if(next_task_index == TIMINGTASK_NUM+1 || next_task_index == 0xFF) {
        // 如果没有有效任务，设置一个0点的任务，用于第二天重新设置任务
        mcu_timingtask_func.timingtask_set_alarm(3, 0, 0);//设置一个0点的任务,用于第二天重新设置任务
    }
}

/**
 * @brief 读取至全局变量
 * 
 * @param index 
 * @param cache_index 
 * @return MCU_TIMINGTASK_T* 
 */
static MCU_TIMINGTASK_T* get_task_with_cache(uint8_t index, uint8_t cache_index)
{
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(index), 
                                      &mcu_timingtask_read_cache[cache_index], 
                                      MCU_TIMINGTASK_T_SIZE);
    return &mcu_timingtask_read_cache[cache_index];
}

/**
 * @brief 返回当前报警应执行的任务缓存
 * 
 * @param[out] cid 任务id指针
 * @return mcu_timingtask_content_t* 
 */
mcu_timingtask_content_t *mcu_timingtask_alarm_buf(uint32_t *cid)
{
    if(mcu_timingtask_num == 0) {
        return NULL;
    }
    
    MCU_TIMINGTASK_T *temp_task = get_task_with_cache(mcu_timingtask_execute_sort[mcu_timingtask_execute_index], 0);
    
    *cid = temp_task->timingtask_id;
    return &temp_task->timingtask_content;
}

// 任务ID管理相关函数
uint8_t mcu_return_all_timingtask_id(uint8_t *timingtask_id)
{
    uint8_t count = 0;
    
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        // 使用全局缓存索引0读取任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[0], 
                                          MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF) {
            timingtask_id[count++] = i;
        }
    }
    
    return count;
}

/**
 * @brief 查找与指定任务时间相同的任务
 * 
 * @param cid 任务id指针
 * @return uint8_t* 任务数据指针
 */
mcu_timingtask_content_t *same_timingtask(uint32_t *cid)
{
    uint8_t current_index = mcu_search_timingtask_id(*cid);
    
    if(current_index == 0xFF) {
        return NULL;
    }
    
    // 使用全局缓存的第一个元素存储当前任务
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(current_index), 
                                      &mcu_timingtask_read_cache[0], 
                                      MCU_TIMINGTASK_T_SIZE);
    
    for(uint8_t i = 0; i < TIMINGTASK_NUM; i++) {
        if(i == current_index) {
            continue;
        }
        
        // 使用全局缓存的第二个元素存储比较任务
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), 
                                          &mcu_timingtask_read_cache[1], 
                                          MCU_TIMINGTASK_T_SIZE);
        
        if(mcu_timingtask_read_cache[1].timingtask_id != 0xFFFFFFFF &&
           mcu_timingtask_read_cache[1].timingtask_hour == mcu_timingtask_read_cache[0].timingtask_hour &&
           mcu_timingtask_read_cache[1].timingtask_min == mcu_timingtask_read_cache[0].timingtask_min &&
           mcu_timingtask_read_cache[1].timingtask_sec == mcu_timingtask_read_cache[0].timingtask_sec &&
           mcu_timingtask_read_cache[1].timingtask_running_cycle == mcu_timingtask_read_cache[0].timingtask_running_cycle &&
           mcu_timingtask_read_cache[1].timingtask_running_cycle_unit == mcu_timingtask_read_cache[0].timingtask_running_cycle_unit) {
            
            *cid = mcu_timingtask_read_cache[1].timingtask_id;
            return &mcu_timingtask_read_cache[1].timingtask_content;
        }
    }
    
    return NULL;
}
// 任务有效性检查函数
static uint8_t check_task_validity(MCU_TIMINGTASK_T *task)
{
    // 检查时间格式是否有效
    if(task->timingtask_hour >= 24 ||
       task->timingtask_min >= 60 ||
       task->timingtask_sec >= 60) {
        return 0;
    }
    
    // 检查运行周期是否有效
    switch(task->timingtask_running_cycle) {
        case TIMINGTASK_RUNNING_CYCLE_EVERYDAY:
            break;
        case TIMINGTASK_RUNNING_CYCLE_EVERYWEEK:
            if(!(task->timingtask_running_cycle_unit & 0x7F)) {
                return 0;
            }
            break;
        case TIMINGTASK_RUNNING_CYCLE_EVERYMONTH:
            if(!(task->timingtask_running_cycle_unit & 0x7FFFFFFF)) {
                return 0;
            }
            break;
        default:
            return 0;
    }
    
    return 1;
}
