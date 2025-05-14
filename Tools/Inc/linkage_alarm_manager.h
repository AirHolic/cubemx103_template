/**
 * @file linkage_alarm_manager.h
 * @author AirHolic
 * @brief 联动告警管理器头文件,用于联动告警的存储和提供条件比较函数接口
 * @version 0.1
 * @date 2025-03-13
 * @todo 疑似为联动告警任务存于叶子端,需重构为由节点端决策的形式
 * @todo 即移除触发端,从叶子端获取数据后与缓存任务进行比较,再执行后续动作(命令叶子端执行动作并返回结果,节点端进行相关上报等
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __LINKAGE_ALARM_MANAGER_H__
#define __LINKAGE_ALARM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "w25qxx_spi_driver.h"

#define MAX_ALARM_TASK_NUM  30  // 最大任务数
#define LINKAGE_SRAM_MODE

#define MIN(A,B)    ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })

#pragma pack(push, 1)

/* 基础类型定义 */
typedef union {
    uint32_t storage;
    float value;
} float32_u;

/* 协议命令码 */
typedef enum {//待修改为告警联动,自动联动,告警上报等
    CMD_ALARMUP = 0x01,
    CMD_ALARM_LINKAGE = 0x02,
    CMD_AUTO_LINKAGE = 0x03
} ProtocolCommand;

/* 比较操作符 */
typedef enum {
    OP_GT = 0x00,  // >
    OP_LT = 0x01,  // <
    OP_EQ = 0x02,  // =
} CompareOperator;

/* 条件类型 */
typedef enum {
    CONDITION_AND = 0x00, // 与
    CONDITION_OR = 0x01,  // 或
} ConditionType;

/**
 * @brief 传感器变量表,用于全局变量存储
 * @todo 待添加叶子节点字段或其他方式
 * 
 */
typedef struct {//需考虑不同叶子节点同一设备类型的情况
    uint16_t leaf_addr; // 叶子节点地址
    uint16_t leaf_port; // 叶子节点端口号
    uint16_t var_id;    // 变量ID(设备类型)
    float32_u value;    // 变量当前值
} VarEntry;

/**
 * @brief 用于存储单比较式的结构体
 * 
 */
typedef struct{
    uint16_t leaf_addr; // 监控端叶子节点地址
    uint16_t leaf_port; // 监控端口号
    uint16_t device_type; // 设备类型
    CompareOperator operator; // 比较操作符
    float32_u value; // 比较值
}LinkageMonitorCompareOperator;

typedef struct{
    uint16_t leaf_addr; // 执行端叶子节点地址
    uint16_t leaf_port; // 执行端口号
    uint16_t device_type;// 设备类型,若为485则用于分辨寄存器地址
    uint8_t cmd; // 执行命令,如 digital开关,485写入等
}LinkageExecuteCommand;

/**
 * @brief 用于多比较式的结构体,a>1&&a<3实现范围比较
 * 
 */
typedef struct{
    uint8_t conditions_num; // 条件数
    LinkageMonitorCompareOperator operators[5]; // 比较式数
}LinkageMonitorConditionType;

/* 监测协议存储结构 */
typedef struct{
    uint16_t length;
    ProtocolCommand cmd_code;//命令码,告警,联动等
    uint16_t msg_id;//任务id
    ConditionType conditions_type; // 条件类型,0-与,1-或
    LinkageMonitorConditionType condition; // 条件数据,格式见行27
}LinkageAlarmFrameHeader;

/* 执行协议存储结构 */
typedef struct{
    uint16_t length;
    ProtocolCommand cmd_code;//命令码,告警,联动等
    uint16_t msg_id;//任务id
    LinkageExecuteCommand execute; // 执行数据,格式见行27
}LinkageExecuteFrameHeader;

#pragma pack(pop)

/* 函数声明 */

uint8_t linkage_set_task_alarm_state_by_id(uint16_t msg_id, uint8_t state); // 设置告警状态
void linkage_alarm_flag_set(void); // 设置告警标志
void linkage_alarm_timer_sec(void);

#ifdef LINKAGE_SRAM_MODE
void linkage_alarm_storage_init(void); // 初始化函数
uint8_t linkage_add_monitor_task(LinkageAlarmFrameHeader *task); // 添加监控告警任务
uint8_t linkage_add_execute_task(LinkageExecuteFrameHeader *task); // 添加执行联动任务
uint8_t linkage_delete_monitor_task(uint16_t msg_id); // 删除监控告警任务
uint8_t linkage_delete_execute_task(uint16_t msg_id); // 删除执行联动任务
LinkageAlarmFrameHeader* linkage_get_monitor_task_by_id(uint16_t msg_id); // 通过ID获取监控告警任务
LinkageExecuteFrameHeader* linkage_get_execute_task_by_id(uint16_t msg_id); // 通过ID获取执行联动任务
LinkageAlarmFrameHeader* linkage_get_monitor_task_by_index(uint8_t index); // 通过索引获取监控告警任务
LinkageExecuteFrameHeader* linkage_get_execute_task_by_index(uint8_t index); // 通过索引获取执行联动任务
uint8_t linkage_get_monitor_task_count(void); // 获取当前监控告警任务数量
uint8_t linkage_get_execute_task_count(void); // 获取当前执行联动任务数量
uint8_t linkage_check_alarm_conditions(const VarEntry *var_table, uint8_t var_count, 
                                       uint16_t *triggered_tasks, uint8_t max_triggered); // 检查所有告警任务条件
uint8_t linkage_find_monitor_tasks_by_leaf_addr(uint16_t leaf_addr, uint16_t *task_ids, uint8_t max_tasks); // 查找特定叶子节点地址的监控告警任务
LinkageExecuteFrameHeader* linkage_find_execute_task_by_trigger(uint16_t triggered_id); // 根据触发的告警任务ID查找对应的执行任务
uint8_t linkage_find_monitor_task_index(uint16_t msg_id); // 查找监控告警任务索引
uint8_t linkage_find_execute_task_index(uint16_t msg_id); // 查找执行联动任务索引
uint8_t linkage_find_monitor_tasks_by_leaf_addr(uint16_t leaf_addr, uint16_t *task_ids, uint8_t max_tasks); // 查找特定叶子节点地址的监控告警任务
uint8_t linkage_find_execute_tasks_by_leaf_addr(uint16_t leaf_addr, uint16_t *task_ids, uint8_t max_tasks); // 查找特定叶子节点地址的执行联动任务
uint8_t evaluate_alarm_condition(LinkageMonitorConditionType *condition, const VarEntry *var_table, uint8_t var_count, ConditionType conditions_type); // 判断变量是否满足告警条件
uint8_t linkage_update_monitor_task(LinkageAlarmFrameHeader *task); // 更新监控告警任务
uint8_t linkage_update_execute_task(LinkageExecuteFrameHeader *task); // 更新执行联动任务

#endif

#ifdef __cplusplus
}
#endif

#endif // !__LINKAGE_ALARM_MANAGER_H__
