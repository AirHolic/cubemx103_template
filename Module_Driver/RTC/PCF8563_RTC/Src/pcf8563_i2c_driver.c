#include "pcf8563_i2c_driver.h"

/* 地址定义 */
#define PCF8563_ADDR              (0x51 << 1)  // PCF8563设备地址(8位地址，包含R/W位)

/* PCF8563 寄存器地址映射 */
#define PCF8563_CTRL_STATUS1      0x00  // 控制/状态寄存器1: 用于控制测试模式、禁用RTC功能等
#define PCF8563_CTRL_STATUS2      0x01  // 控制/状态寄存器2: 用于配置闹钟、定时器和时钟输出功能
#define PCF8563_SECONDS          0x02   // 秒寄存器: 0-59秒，高位为掉电标志位(VL)
#define PCF8563_MINUTES          0x03   // 分钟寄存器: 0-59分钟
#define PCF8563_HOURS            0x04   // 小时寄存器: 0-23小时
#define PCF8563_DAYS            0x05    // 日期寄存器: 1-31日
#define PCF8563_WEEKDAYS         0x06   // 星期寄存器: 0-6对应星期天到星期六
#define PCF8563_MONTHS           0x07   // 月份寄存器: 1-12月，高位为世纪位
#define PCF8563_YEARS           0x08    // 年份寄存器: 0-99年
#define PCF8563_VL_FLAG          0x80   // 掉电标志位掩码(位于秒寄存器)

/* Private variables */
static struct {
    I2C_HandleTypeDef *hi2c;      // I2C通信句柄指针
    uint8_t initialized;          // 初始化标志，1表示已初始化，0表示未初始化
} pcf8563_dev;

/* Private functions */

/**
 * @brief  写PCF8563寄存器
 * @param  reg: 寄存器地址
 * @param  data: 数据缓冲区指针
 * @param  len: 数据长度
 * @retval HAL状态: HAL_OK表示成功，HAL_ERROR表示失败等
 * @note   通过I2C接口向PCF8563指定寄存器写入数据
 */
static HAL_StatusTypeDef pcf8563_write_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Write(pcf8563_dev.hi2c, PCF8563_ADDR, reg, 
                            I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);
}

/**
 * @brief  读PCF8563寄存器
 * @param  reg: 寄存器地址
 * @param  data: 数据缓冲区指针
 * @param  len: 数据长度
 * @retval HAL状态: HAL_OK表示成功，HAL_ERROR表示失败等
 * @note   通过I2C接口从PCF8563指定寄存器读取数据
 */
static HAL_StatusTypeDef pcf8563_read_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(pcf8563_dev.hi2c, PCF8563_ADDR, reg,
                           I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);
}

/* Public functions */

/**
 * @brief 初始化 PCF8563 RTC
 * 
 * @param hi2c I2C句柄指针
 * @return pcf8563_status_t 
 */
pcf8563_status_t pcf8563_init(I2C_HandleTypeDef *hi2c)
{
    if(pcf8563_dev.initialized) {
        return PCF8563_OK;
    }

    pcf8563_dev.hi2c = hi2c;
    pcf8563_dev.initialized = 1;

    uint8_t data = 0x00;
    if(pcf8563_write_reg(PCF8563_CTRL_STATUS1, &data, 1) != HAL_OK ||
       pcf8563_write_reg(PCF8563_CTRL_STATUS2, &data, 1) != HAL_OK) {
        pcf8563_dev.initialized = 0;
        return PCF8563_ERROR;
    }

    return PCF8563_OK;
}

/**
 * @brief 设置PCF8563的时间
 * 
 * @param[in] time 指向 pcf8563_time_t 结构体的指针，用于设置时间
 * @return pcf8563_status_t 
 */
pcf8563_status_t pcf8563_set_time(pcf8563_time_t *time)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t data[7];
    data[0] = ((time->seconds / 10) << 4) | (time->seconds % 10); 
    data[1] = ((time->minutes / 10) << 4) | (time->minutes % 10);
    data[2] = ((time->hours / 10) << 4) | (time->hours % 10);
    data[3] = ((time->days / 10) << 4) | (time->days % 10);
    data[4] = time->weekdays & 0x07;
    data[5] = ((time->months / 10) << 4) | (time->months % 10);
    data[6] = ((time->years / 10) << 4) | (time->years % 10);

    if(pcf8563_write_reg(PCF8563_SECONDS, data, 7) != HAL_OK) {
        return PCF8563_ERROR;
    }

    return PCF8563_OK;
}

/**
 * @brief 读取PCF8563的时间
 * 
 * @param[out] time 指向 pcf8563_time_t 结构体的指针，用于存储读取的时间
 * @return pcf8563_status_t 
 */
pcf8563_status_t pcf8563_get_time(pcf8563_time_t *time)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t data[7];
    if(pcf8563_read_reg(PCF8563_SECONDS, data, 7) != HAL_OK) {
        return PCF8563_ERROR;
    }

    time->seconds = ((data[0] >> 4) * 10) + (data[0] & 0x0F);
    time->minutes = ((data[1] >> 4) * 10) + (data[1] & 0x0F);
    time->hours = ((data[2] >> 4) * 10) + (data[2] & 0x0F);
    time->days = ((data[3] >> 4) * 10) + (data[3] & 0x0F);
    time->weekdays = data[4] & 0x07;
    time->months = ((data[5] >> 4) * 10) + (data[5] & 0x0F);
    time->years = ((data[6] >> 4) * 10) + (data[6] & 0x0F);

    return PCF8563_OK;
}


/**
 * @brief 检查 PCF8563 RTC 是否运行
 * 
 * 此函数读取状态寄存器并检查时钟是否运行。
 * 如果 Control/Status1 寄存器的 STOP 位（位 5）被设置，则认为时钟已停止。
 * 
 * @param[out] running 指针，用于存储运行状态：
 *                     如果时钟在运行，则为 1；如果停止，则为 0
 * 
 * @return PCF8563_OK 如果成功
 * @return PCF8563_ERROR 如果设备未初始化或通信失败
 */
pcf8563_status_t pcf8563_is_running(uint8_t *running)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t data;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS1, &data, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    *running = !(data & 0x20);
    return PCF8563_OK;
}

/**
 * @brief  检查PCF8563的电压低检测标志(VL)状态
 * @param  vl_detected: 返回电压低状态，1表示检测到电压低，0表示未检测到
 * @retval PCF8563_OK: 成功读取VL标志
 *         PCF8563_ERROR: 读取失败
 * @note   VL标志位于秒寄存器(0x02)的位7，如果该位为1，表示检测到电源电压低
 */
pcf8563_status_t pcf8563_check_voltage_low(uint8_t *vl_detected)
{
    if(!pcf8563_dev.initialized || vl_detected == NULL) {
        return PCF8563_ERROR;
    }

    uint8_t seconds_reg;
    if(pcf8563_read_reg(PCF8563_SECONDS, &seconds_reg, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    *vl_detected = (seconds_reg & PCF8563_VL_FLAG) ? 1 : 0;
    return PCF8563_OK;
}

/**
 * @brief  清除PCF8563的电压低检测标志(VL)
 * @retval PCF8563_OK: 成功清除VL标志
 *         PCF8563_ERROR: 清除失败
 * @note   VL标志位于秒寄存器(0x02)的位7，此函数将该位清零
 *         清除VL标志的同时，当前的秒值会被保留
 */
pcf8563_status_t pcf8563_clear_voltage_low(void)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t seconds_reg;
    
    // 读取当前秒寄存器值
    if(pcf8563_read_reg(PCF8563_SECONDS, &seconds_reg, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }
    
    // 清除VL标志位(位7)，保留其他位
    seconds_reg &= ~PCF8563_VL_FLAG;
    
    // 写回秒寄存器
    if(pcf8563_write_reg(PCF8563_SECONDS, &seconds_reg, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }
    
    return PCF8563_OK;
}

/**
 * @brief  设置PCF8563的闹钟
 * @param  alarm: 闹钟结构体指针
 * @param  alarm_mask: 闹钟配置掩码，可以是以下值的组合:
 *         PCF8563_ALARM_MINUTE: 使能分钟闹钟
 *         PCF8563_ALARM_HOUR: 使能小时闹钟
 *         PCF8563_ALARM_DAY: 使能日期闹钟
 *         PCF8563_ALARM_WEEKDAY: 使能星期闹钟
 * @retval PCF8563_OK: 设置成功
 *         PCF8563_ERROR: 设置失败
 */
pcf8563_status_t pcf8563_set_alarm(pcf8563_alarm_t *alarm, uint8_t alarm_mask)
{
    if(!pcf8563_dev.initialized || alarm == NULL) {
        return PCF8563_ERROR;
    }

    uint8_t data[4];
    
    // 根据掩码设置闹钟寄存器
    data[0] = (alarm_mask & PCF8563_ALARM_MINUTE) ? 
              (((alarm->minute / 10) << 4) | (alarm->minute % 10)) : 0x80;
    
    data[1] = (alarm_mask & PCF8563_ALARM_HOUR) ? 
              (((alarm->hour / 10) << 4) | (alarm->hour % 10)) : 0x80;
    
    data[2] = (alarm_mask & PCF8563_ALARM_DAY) ? 
              (((alarm->day / 10) << 4) | (alarm->day % 10)) : 0x80;
    
    data[3] = (alarm_mask & PCF8563_ALARM_WEEKDAY) ? 
              (alarm->weekday & 0x07) : 0x80;

    // 写入闹钟寄存器
    if(pcf8563_write_reg(PCF8563_MINUTE_ALARM, data, 4) != HAL_OK) {
        return PCF8563_ERROR;
    }

    // 设置闹钟使能位
    uint8_t ctrl2;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }
    
    if(alarm->enabled) {
        ctrl2 |= 0x02;  // 使能闹钟中断
    } else {
        ctrl2 &= ~0x02; // 禁用闹钟中断
    }
    
    if(pcf8563_write_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    return PCF8563_OK;
}

/**
 * @brief  获取PCF8563的闹钟设置
 * @param  alarm: 闹钟结构体指针
 * @retval PCF8563_OK: 获取成功
 *         PCF8563_ERROR: 获取失败
 */
pcf8563_status_t pcf8563_get_alarm(pcf8563_alarm_t *alarm)
{
    if(!pcf8563_dev.initialized || alarm == NULL) {
        return PCF8563_ERROR;
    }

    uint8_t data[4];
    if(pcf8563_read_reg(PCF8563_MINUTE_ALARM, data, 4) != HAL_OK) {
        return PCF8563_ERROR;
    }

    // 解析闹钟设置
    if(!(data[0] & 0x80)) {
        alarm->minute = ((data[0] >> 4) * 10) + (data[0] & 0x0F);
    }
    
    if(!(data[1] & 0x80)) {
        alarm->hour = ((data[1] >> 4) * 10) + (data[1] & 0x0F);
    }
    
    if(!(data[2] & 0x80)) {
        alarm->day = ((data[2] >> 4) * 10) + (data[2] & 0x0F);
    }
    
    if(!(data[3] & 0x80)) {
        alarm->weekday = data[3] & 0x07;
    }

    // 获取闹钟使能状态
    uint8_t ctrl2;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }
    alarm->enabled = (ctrl2 & 0x02) ? 1 : 0;

    return PCF8563_OK;
}

/**
 * @brief  使能/禁用PCF8563的闹钟功能
 * @param  enable: 1-使能闹钟，0-禁用闹钟
 * @retval PCF8563_OK: 设置成功
 *         PCF8563_ERROR: 设置失败
 */
pcf8563_status_t pcf8563_enable_alarm(uint8_t enable)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t ctrl2;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    if(enable) {
        ctrl2 |= 0x02;  // 使能闹钟中断
    } else {
        ctrl2 &= ~0x02; // 禁用闹钟中断
    }

    if(pcf8563_write_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    return PCF8563_OK;
}

/**
 * @brief  获取PCF8563的闹钟触发标志
 * @param  triggered: 返回闹钟触发状态，1-已触发，0-未触发
 * @retval PCF8563_OK: 获取成功
 *         PCF8563_ERROR: 获取失败
 */
pcf8563_status_t pcf8563_get_alarm_flag(uint8_t *triggered)
{
    if(!pcf8563_dev.initialized || triggered == NULL) {
        return PCF8563_ERROR;
    }

    uint8_t ctrl2;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    *triggered = (ctrl2 & 0x08) ? 1 : 0;
    return PCF8563_OK;
}

/**
 * @brief  清除PCF8563的闹钟触发标志
 * @retval PCF8563_OK: 清除成功
 *         PCF8563_ERROR: 清除失败
 */
pcf8563_status_t pcf8563_clear_alarm_flag(void)
{
    if(!pcf8563_dev.initialized) {
        return PCF8563_ERROR;
    }

    uint8_t ctrl2;
    if(pcf8563_read_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    ctrl2 &= ~0x08; // 清除闹钟标志位
    
    if(pcf8563_write_reg(PCF8563_CTRL_STATUS2, &ctrl2, 1) != HAL_OK) {
        return PCF8563_ERROR;
    }

    return PCF8563_OK;
}