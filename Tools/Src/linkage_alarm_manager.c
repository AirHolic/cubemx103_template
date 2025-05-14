/**
 * @file linkage_alarm_manager.c
 * @author AirHolic
 * @brief 联动告警管理器实现文件
 * @version 0.2
 * @date 2025-04-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "linkage_alarm_manager.h"
#include "crc_tools.h"
#include <string.h>
#include "main.h"

#define MAX_VAR_COUNT 20   // 最大变量数量
#define MAX_TRIGGER_NUM 10 // 最大触发任务数
#define true 1
#define false 0

// 告警任务存储区地址
#define MONITOR_STORAGE_ADDR 0x8000
#define EXECUTE_STORAGE_ADDR 0x8100

#ifdef LINKAGE_SRAM_MODE
// 监控告警任务数组,常驻内存
static LinkageAlarmFrameHeader monitor_tasks[MAX_ALARM_TASK_NUM];
static uint8_t monitor_task_num = 0; // 当前监控任务数量

typedef struct{
    uint8_t alarm_min; // 任务状态:已告警时间min，0为未告警
}monitor_task_flag_t;
static monitor_task_flag_t monitor_task_flag[MAX_ALARM_TASK_NUM]; // 监控任务标志数组
static uint8_t monitor_alarm_exist_flag = 0; // 存在已告警任务标志,兼任秒级定时器

// 执行联动任务数组,常驻内存
static LinkageExecuteFrameHeader execute_tasks[MAX_ALARM_TASK_NUM];
static uint8_t execute_task_num = 0; // 当前执行任务数量
#endif

/**
 * @brief 存储接口 - 写入数据
 * 
 * @param addr 存储地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return uint8_t 操作结果（0-成功,其他-失败)
 */
static uint8_t linkage_storage_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    // 根据实际存储驱动实现
    return W25QXX_WriteNoErase(addr, buf, len);
}

/**
 * @brief 存储接口 - 读取数据
 * 
 * @param addr 存储地址
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return uint8_t 操作结果（0-成功,其他-失败)
 */
static uint8_t linkage_storage_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    // 根据实际存储驱动实现
    return W25QXX_ReadBuffer(addr, buf, len);
}

/**
 * @brief 存储接口 - 擦除数据
 * 
 * @param addr 存储地址
 * @param len 数据长度
 * @return uint8_t 操作结果（0-成功,其他-失败)
 */
static uint8_t linkage_storage_erase(uint32_t addr, uint32_t len)
{
    // 根据实际存储驱动实现
    return W25QXX_Erase(addr, len);
}

/**
 * @brief 比较单个条件是否满足
 * 
 * @param op 比较操作符
 * @param var_value 变量值
 * @param threshold 阈值
 * @return uint8_t 比较结果（1-满足，0-不满足)
 */
static uint8_t compare_condition(CompareOperator op, float var_value, float threshold)
{
    switch (op)
    {
        case OP_GT: return var_value > threshold;
        case OP_LT: return var_value < threshold;
        case OP_EQ: return var_value == threshold;
        default: return 0;
    }
}

/**
 * @brief 判断变量是否满足告警条件
 * 
 * @param condition 条件结构体指针
 * @param var_table 变量表
 * @param var_count 变量数量
 * @param conditions_type 条件类型(AND/OR)
 * @return uint8_t 条件判断结果（1-满足触发条件，0-不满足)
 */
uint8_t evaluate_alarm_condition(LinkageMonitorConditionType *condition, const VarEntry *var_table, uint8_t var_count, ConditionType conditions_type)
{
    // 如果没有条件,默认不满足
    if (condition->conditions_num == 0) {
        return 0;
    }
    
    // 根据条件类型决定使用AND还是OR逻辑
    if (conditions_type == CONDITION_AND) {
        // 所有条件都满足才返回true（条件之间是AND逻辑)
        for (uint8_t i = 0; i < condition->conditions_num; i++) {
            LinkageMonitorCompareOperator *op = &condition->operators[i];
            uint8_t found = 0;
            
            // 在变量表中查找对应device_type,leaf_addr和leaf_port的变量
            for (uint8_t j = 0; j < var_count; j++) {
                if (var_table[j].var_id == op->device_type && var_table[j].leaf_addr == op->leaf_addr && var_table[j].leaf_port == op->leaf_port) {
                    found = 1;
                    // 比较变量值与条件
                    if (!compare_condition(op->operator, var_table[j].value.value, op->value.value)) {
                        return 0; // 有一个条件不满足,整体不满足
                    }
                    break;
                }
            }
            
            // 如果没找到对应变量,条件无法判断,默认不满足
            if (!found) {
                return 0;
            }
        }
        
        // 所有条件都满足
        return 1;
    }
    else if (conditions_type == CONDITION_OR) {
        // 任一条件满足即返回true（条件之间是OR逻辑)
        for (uint8_t i = 0; i < condition->conditions_num; i++) {
            LinkageMonitorCompareOperator *op = &condition->operators[i];
            
            // 在变量表中查找对应device_type,leaf_addr和leaf_port的变量
            for (uint8_t j = 0; j < var_count; j++) {
                if (var_table[j].var_id == op->device_type && var_table[j].leaf_addr == op->leaf_addr && var_table[j].leaf_port == op->leaf_port) {
                    // 比较变量值与条件
                    if (compare_condition(op->operator, var_table[j].value.value, op->value.value)) {
                        return 1; // 有一个条件满足,整体满足
                    }
                    break;
                }
            }
        }
        
        // 没有任何条件满足
        return 0;
    }
    
    // 未知的条件类型,默认不满足
    return 0;
}

/**
 * @brief 根据任务ID设置告警状态
 * 
 * @param msg_id 任务ID
 * @param state 告警状态(0为未告警，非0为告警分钟数)
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_set_task_alarm_state_by_id(uint16_t msg_id, uint8_t state)
{
    uint8_t index = linkage_find_monitor_task_index(msg_id);
    if (index == 0xFF) {
        return 0; // 未找到任务
    }
    
    monitor_task_flag[index].alarm_min = state;
    if (state > 0 && monitor_alarm_exist_flag == 0) {
        monitor_alarm_exist_flag = 1; // 设置全局告警标志
    }
    
    return 1;
}

void linkage_alarm_flag_set(void)
{
    if (monitor_alarm_exist_flag == 0)
    {
        monitor_alarm_exist_flag = 1; // 设置告警标志
    }
}

void linkage_alarm_timer_sec(void)
{ 
    // 定时器回调函数,每秒调用一次
    // 在这里可以添加定时任务的处理逻辑
    if(monitor_alarm_exist_flag)
    {
        monitor_alarm_exist_flag++;
        if(monitor_alarm_exist_flag > 60)
        {
            monitor_alarm_exist_flag = 0; // 重置告警标志
            for(uint8_t i = 0; i < monitor_task_num; i++)
            {
                if(monitor_task_flag[i].alarm_min != 0)
                {
                    monitor_alarm_exist_flag = 1;
                    monitor_task_flag[i].alarm_min++;
                    if(monitor_task_flag[i].alarm_min > 10)
                    {
                        // 超过10分钟,清除告警
                        monitor_task_flag[i].alarm_min = 0;
                    }
                }
            }
        }
    }
}

#ifdef LINKAGE_SRAM_MODE

/**
 * @brief 初始化联动告警存储
 */
void linkage_alarm_storage_init(void)
{
    // 初始化监控告警任务
    memset(monitor_tasks, 0, sizeof(monitor_tasks));
    monitor_task_num = 0;
    
    // 初始化执行联动任务
    memset(execute_tasks, 0, sizeof(execute_tasks));
    execute_task_num = 0;

    // 1. 从Flash中读取已存储的监控告警任务
    uint32_t offset = 0;
    uint16_t task_length = 0;
    
    // 读取第一个监控任务的长度,判断是否有存储的任务
    linkage_storage_read(MONITOR_STORAGE_ADDR, (uint8_t *)&task_length, sizeof(uint16_t));
    
    // 如果长度不为0xFFFF，说明有存储的任务
    if (task_length != 0xFFFF) {
        // 逐个读取存储的监控任务
        while (task_length != 0xFFFF && monitor_task_num < MAX_ALARM_TASK_NUM) {
            // 读取整个任务
            linkage_storage_read(MONITOR_STORAGE_ADDR + offset, 
                              (uint8_t *)&monitor_tasks[monitor_task_num], 
                              task_length);
            
            // 校验CRC
            uint8_t stored_crc = *((uint8_t *)&monitor_tasks[monitor_task_num] + task_length - 1);
            uint8_t calculated_crc = crc8((uint8_t *)&monitor_tasks[monitor_task_num], task_length - 1);
            
            if (stored_crc == calculated_crc) {
                monitor_task_num++;
            } else {
                // CRC校验失败,丢弃该任务
                memset(&monitor_tasks[monitor_task_num], 0, sizeof(LinkageAlarmFrameHeader));
            }
            
            // 移动到下一个任务
            offset += task_length;
            
            // 读取下一个任务的长度
            task_length = 0;
            linkage_storage_read(MONITOR_STORAGE_ADDR + offset, (uint8_t *)&task_length, sizeof(uint16_t));
        }
    }
    
    // 2. 从Flash中读取已存储的执行联动任务
    offset = 0;
    task_length = 0;
    
    // 读取第一个执行任务的长度,判断是否有存储的任务
    linkage_storage_read(EXECUTE_STORAGE_ADDR, (uint8_t *)&task_length, sizeof(uint16_t));
    
    // 如果长度不为0xFFFF，说明有存储的任务
    if (task_length != 0xFFFF) {
        // 逐个读取存储的执行任务
        while (task_length != 0xFFFF && execute_task_num < MAX_ALARM_TASK_NUM) {
            // 读取整个任务
            linkage_storage_read(EXECUTE_STORAGE_ADDR + offset, 
                              (uint8_t *)&execute_tasks[execute_task_num], 
                              task_length);
            
            // 校验CRC
            uint8_t stored_crc = *((uint8_t *)&execute_tasks[execute_task_num] + task_length - 1);
            uint8_t calculated_crc = crc8((uint8_t *)&execute_tasks[execute_task_num], task_length - 1);
            
            if (stored_crc == calculated_crc) {
                execute_task_num++;
            } else {
                // CRC校验失败,丢弃该任务
                memset(&execute_tasks[execute_task_num], 0, sizeof(LinkageExecuteFrameHeader));
            }
            
            // 移动到下一个任务
            offset += task_length;
            
            // 读取下一个任务的长度
            task_length = 0;
            linkage_storage_read(EXECUTE_STORAGE_ADDR + offset, (uint8_t *)&task_length, sizeof(uint16_t));
        }
    }
}

/**
 * @brief 添加新的监控告警任务
 * 
 * @param task 任务指针
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_add_monitor_task(LinkageAlarmFrameHeader *task)
{
    // 检查任务数量是否已达上限
    if (monitor_task_num >= MAX_ALARM_TASK_NUM) {
        return 0;
    }
    
    // 计算并添加CRC校验
    uint8_t crc_value = crc8((uint8_t *)task, task->length - 1);
    *((uint8_t *)task + task->length - 1) = crc_value;
    
    // 拷贝任务到内存
    memcpy(&monitor_tasks[monitor_task_num], task, task->length);
    
    // 计算存储偏移量
    uint32_t offset = 0;
    for (uint8_t i = 0; i < monitor_task_num; i++) {
        offset += monitor_tasks[i].length;
    }
    
    // 写入Flash存储
    linkage_storage_write(MONITOR_STORAGE_ADDR + offset, (uint8_t *)task, task->length);
    
    // 增加任务计数
    monitor_task_num++;
    
    return 1;
}

/**
 * @brief 添加新的执行联动任务
 * 
 * @param task 任务指针
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_add_execute_task(LinkageExecuteFrameHeader *task)
{
    // 检查任务数量是否已达上限
    if (execute_task_num >= MAX_ALARM_TASK_NUM) {
        return 0;
    }
    
    // 计算并添加CRC校验
    uint8_t crc_value = crc8((uint8_t *)task, task->length - 1);
    *((uint8_t *)task + task->length - 1) = crc_value;
    
    // 拷贝任务到内存
    memcpy(&execute_tasks[execute_task_num], task, task->length);
    
    // 计算存储偏移量
    uint32_t offset = 0;
    for (uint8_t i = 0; i < execute_task_num; i++) {
        offset += execute_tasks[i].length;
    }
    
    // 写入Flash存储
    linkage_storage_write(EXECUTE_STORAGE_ADDR + offset, (uint8_t *)task, task->length);
    
    // 增加任务计数
    execute_task_num++;
    
    return 1;
}

/**
 * @brief 通过ID查找监控告警任务索引
 * 
 * @param msg_id 任务ID
 * @return uint8_t 任务索引(0xFF表示未找到)
 */
uint8_t linkage_find_monitor_task_index(uint16_t msg_id)
{
    for (uint8_t i = 0; i < monitor_task_num; i++) {
        if (monitor_tasks[i].msg_id == msg_id) {
            return i;
        }
    }
    return 0xFF; // 未找到
}

/**
 * @brief 通过ID查找执行联动任务索引
 * 
 * @param msg_id 任务ID
 * @return uint8_t 任务索引(0xFF表示未找到)
 */
uint8_t linkage_find_execute_task_index(uint16_t msg_id)
{
    for (uint8_t i = 0; i < execute_task_num; i++) {
        if (execute_tasks[i].msg_id == msg_id) {
            return i;
        }
    }
    return 0xFF; // 未找到
}

/**
 * @brief 通过ID获取监控告警任务
 * 
 * @param msg_id 任务ID
 * @return LinkageAlarmFrameHeader* 任务指针（NULL表示未找到)
 */
LinkageAlarmFrameHeader* linkage_get_monitor_task_by_id(uint16_t msg_id)
{
    uint8_t index = linkage_find_monitor_task_index(msg_id);
    if (index == 0xFF) {
        return NULL;
    }
    return &monitor_tasks[index];
}

/**
 * @brief 通过ID获取执行联动任务
 * 
 * @param msg_id 任务ID
 * @return LinkageExecuteFrameHeader* 任务指针（NULL表示未找到)
 */
LinkageExecuteFrameHeader* linkage_get_execute_task_by_id(uint16_t msg_id)
{
    uint8_t index = linkage_find_execute_task_index(msg_id);
    if (index == 0xFF) {
        return NULL;
    }
    return &execute_tasks[index];
}

/**
 * @brief 通过索引获取监控告警任务
 * 
 * @param index 任务索引
 * @return LinkageAlarmFrameHeader* 任务指针（NULL表示索引无效)
 */
LinkageAlarmFrameHeader* linkage_get_monitor_task_by_index(uint8_t index)
{
    if (index >= monitor_task_num) {
        return NULL;
    }
    return &monitor_tasks[index];
}

/**
 * @brief 通过索引获取执行联动任务
 * 
 * @param index 任务索引
 * @return LinkageExecuteFrameHeader* 任务指针（NULL表示索引无效)
 */
LinkageExecuteFrameHeader* linkage_get_execute_task_by_index(uint8_t index)
{
    if (index >= execute_task_num) {
        return NULL;
    }
    return &execute_tasks[index];
}

/**
 * @brief 删除监控告警任务
 * 
 * @param msg_id 任务ID
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_delete_monitor_task(uint16_t msg_id)
{
    uint8_t index = linkage_find_monitor_task_index(msg_id);
    if (index == 0xFF) {
        return 0; // 未找到任务
    }
    
    // 计算存储偏移量
    uint32_t offset = 0;
    for (uint8_t i = 0; i < index; i++) {
        offset += monitor_tasks[i].length;
    }
    
    // 将后面的任务前移
    for (uint8_t i = index; i < monitor_task_num - 1; i++) {
        memcpy(&monitor_tasks[i], &monitor_tasks[i + 1], monitor_tasks[i + 1].length);
    }
    
    // 清空最后一个任务
    memset(&monitor_tasks[monitor_task_num - 1], 0, sizeof(LinkageAlarmFrameHeader));
    
    // 更新任务数量
    monitor_task_num--;
    
    // 重新写入Flash存储
    // 只重写从删除位置开始的数据
    for (uint8_t i = index; i < monitor_task_num; i++) {
        linkage_storage_write(MONITOR_STORAGE_ADDR + offset, (uint8_t *)&monitor_tasks[i], monitor_tasks[i].length);
        offset += monitor_tasks[i].length;
    }
    
    // 将最后的位置标记为空
    uint16_t end_marker = 0xFFFF;
    linkage_storage_write(MONITOR_STORAGE_ADDR + offset, (uint8_t *)&end_marker, sizeof(uint16_t));
    
    return 1;
}

/**
 * @brief 删除执行联动任务
 * 
 * @param msg_id 任务ID
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_delete_execute_task(uint16_t msg_id)
{
    uint8_t index = linkage_find_execute_task_index(msg_id);
    if (index == 0xFF) {
        return 0; // 未找到任务
    }
    
    // 计算存储偏移量
    uint32_t offset = 0;
    for (uint8_t i = 0; i < index; i++) {
        offset += execute_tasks[i].length;
    }
    
    // 将后面的任务前移
    for (uint8_t i = index; i < execute_task_num - 1; i++) {
        memcpy(&execute_tasks[i], &execute_tasks[i + 1], execute_tasks[i + 1].length);
    }
    
    // 清空最后一个任务
    memset(&execute_tasks[execute_task_num - 1], 0, sizeof(LinkageExecuteFrameHeader));
    
    // 更新任务数量
    execute_task_num--;
    
    // 重新写入Flash存储
    // 只重写从删除位置开始的数据
    for (uint8_t i = index; i < execute_task_num; i++) {
        linkage_storage_write(EXECUTE_STORAGE_ADDR + offset, (uint8_t *)&execute_tasks[i], execute_tasks[i].length);
        offset += execute_tasks[i].length;
    }
    
    // 将最后的位置标记为空
    uint16_t end_marker = 0xFFFF;
    linkage_storage_write(EXECUTE_STORAGE_ADDR + offset, (uint8_t *)&end_marker, sizeof(uint16_t));
    
    return 1;
}

/**
 * @brief 更新监控告警任务
 * 
 * @param task 新的任务内容
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_update_monitor_task(LinkageAlarmFrameHeader *task)
{
    // 先删除旧任务,再添加新任务
    if (!linkage_delete_monitor_task(task->msg_id)) {
        return 0; // 删除失败
    }
    return linkage_add_monitor_task(task);
}

/**
 * @brief 更新执行联动任务
 * 
 * @param task 新的任务内容
 * @return uint8_t 操作结果（1-成功，0-失败)
 */
uint8_t linkage_update_execute_task(LinkageExecuteFrameHeader *task)
{
    // 先删除旧任务,再添加新任务
    if (!linkage_delete_execute_task(task->msg_id)) {
        return 0; // 删除失败
    }
    return linkage_add_execute_task(task);
}

/**
 * @brief 获取当前监控告警任务数量
 * 
 * @return uint8_t 任务数量
 */
uint8_t linkage_get_monitor_task_count(void)
{
    return monitor_task_num;
}

/**
 * @brief 获取当前执行联动任务数量
 * 
 * @return uint8_t 任务数量
 */
uint8_t linkage_get_execute_task_count(void)
{
    return execute_task_num;
}

/**
 * @brief 检查所有告警任务条件
 * 
 * @param var_table 变量表
 * @param var_count 变量数量
 * @param triggered_tasks 触发的任务ID数组(输出)
 * @param max_triggered 最大可记录的触发任务数
 * @return uint8_t 触发的任务数量
 */
uint8_t linkage_check_alarm_conditions(const VarEntry *var_table, uint8_t var_count, 
                                      uint16_t *triggered_tasks, uint8_t max_triggered)
{
    uint8_t triggered_count = 0;
    
    // 遍历所有监控告警任务
    for (uint8_t i = 0; i < monitor_task_num && triggered_count < max_triggered; i++) {

        if(monitor_tasks[i].msg_id == 0xFFFF) {
            continue; // 跳过无效任务
        }
        // 获取任务的条件部分
        LinkageMonitorConditionType *condition = &monitor_tasks[i].condition;
        
        // 使用任务中存储的条件类型
        ConditionType conditions_type = monitor_tasks[i].conditions_type;
        
        // 评估条件是否满足
        if (evaluate_alarm_condition(condition, var_table, var_count, conditions_type)) {
            // 记录触发的任务ID
            triggered_tasks[triggered_count++] = monitor_tasks[i].msg_id;
        }
    }
    
    return triggered_count;
}

/**
 * @brief 根据触发的告警任务ID查找对应的执行任务
 * 
 * @param triggered_id 触发的告警任务ID
 * @return LinkageExecuteFrameHeader* 对应的执行任务（NULL表示未找到)
 */
LinkageExecuteFrameHeader* linkage_find_execute_task_by_trigger(uint16_t triggered_id)
{
    // 在执行任务中查找匹配的msg_id
    for (uint8_t i = 0; i < execute_task_num; i++) {
        if (execute_tasks[i].msg_id == triggered_id) {
            return &execute_tasks[i];
        }
    }
    return NULL;
}

/**
 * @brief 查找特定叶子节点地址的监控告警任务
 * 
 * @param leaf_addr 叶子节点地址
 * @param task_ids 匹配的任务ID数组(输出)
 * @param max_tasks 最大可记录的任务数
 * @return uint8_t 匹配的任务数量
 */
uint8_t linkage_find_monitor_tasks_by_leaf_addr(uint16_t leaf_addr, uint16_t *task_ids, uint8_t max_tasks)
{
    uint8_t found_count = 0;
    
    // 遍历所有监控告警任务
    for (uint8_t i = 0; i < monitor_task_num && found_count < max_tasks; i++) {
        // 检查任务中的所有条件,只要有一个条件的叶子节点地址匹配即可
        for (uint8_t j = 0; j < monitor_tasks[i].condition.conditions_num; j++) {
            if (monitor_tasks[i].condition.operators[j].leaf_addr == leaf_addr) {
                // 记录匹配的任务ID
                task_ids[found_count++] = monitor_tasks[i].msg_id;
                break;  // 同一个任务不重复记录
            }
        }
    }
    
    return found_count;
}

/**
 * @brief 查找特定叶子节点地址的执行联动任务
 * 
 * @param leaf_addr 叶子节点地址
 * @param task_ids 匹配的任务ID数组(输出)
 * @param max_tasks 最大可记录的任务数
 * @return uint8_t 匹配的任务数量
 */
uint8_t linkage_find_execute_tasks_by_leaf_addr(uint16_t leaf_addr, uint16_t *task_ids, uint8_t max_tasks)
{
    uint8_t found_count = 0;
    
    // 遍历所有执行联动任务
    for (uint8_t i = 0; i < execute_task_num && found_count < max_tasks; i++) {
        if (execute_tasks[i].execute.leaf_addr == leaf_addr) {
            // 记录匹配的任务ID
            task_ids[found_count++] = execute_tasks[i].msg_id;
        }
    }
    
    return found_count;
}

#endif // LINKAGE_SRAM_MODE

#ifdef LINKAGE_FLASH_MODE
// 在需要时实现Flash存储模式的相关函数
#endif // LINKAGE_FLASH_MODE
