#ifndef __PCF8563_I2C_DRIVER_H__
#define __PCF8563_I2C_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Device struct */

/**
 * @brief PCF8563 RTC设备结构体
 * @note 包含I2C句柄和设备地址以及读写寄存器的函数指针
 * 
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t dev_addr;
    HAL_StatusTypeDef (*write_reg)(uint8_t reg, uint8_t *data, uint16_t len);
    HAL_StatusTypeDef (*read_reg)(uint8_t reg, uint8_t *data, uint16_t len); 
} pcf8563_dev_t;

/* Time struct */
typedef struct {
    uint8_t seconds;
    uint8_t minutes; 
    uint8_t hours;
    uint8_t days;
    uint8_t weekdays;
    uint8_t months;
    uint8_t years;
} pcf8563_time_t;

/* Alarm structure */
typedef struct {
    uint8_t minute;     // 0-59, 0x80 to disable
    uint8_t hour;       // 0-23, 0x80 to disable
    uint8_t day;        // 1-31, 0x80 to disable
    uint8_t weekday;    // 0-6,  0x80 to disable
    uint8_t enabled;    // 使能位
} pcf8563_alarm_t;

/* Status enum */
typedef enum {
    PCF8563_OK = 0,
    PCF8563_ERROR = 1,
    PCF8563_BUSY = 2,
    PCF8563_TIMEOUT = 3
} pcf8563_status_t;

/* Device Address ------------------------------------------------------------*/
#define PCF8563_I2C_ADDR              0x51    // 7-bit address

/* Register Addresses -------------------------------------------------------*/
#define PCF8563_CTRL_STATUS1          0x00
#define PCF8563_CTRL_STATUS2          0x01
#define PCF8563_SECONDS               0x02
#define PCF8563_MINUTES               0x03
#define PCF8563_HOURS                 0x04
#define PCF8563_DAYS                  0x05
#define PCF8563_WEEKDAYS              0x06
#define PCF8563_MONTHS                0x07
#define PCF8563_YEARS                 0x08
#define PCF8563_MINUTE_ALARM          0x09
#define PCF8563_HOUR_ALARM            0x0A
#define PCF8563_DAY_ALARM             0x0B
#define PCF8563_WEEKDAY_ALARM         0x0C
#define PCF8563_CLKOUT_CONTROL        0x0D
#define PCF8563_TIMER_CONTROL         0x0E
#define PCF8563_TIMER                 0x0F

/* Status flags */
#define PCF8563_VL_FLAG               0x80    // Voltage Low flag in seconds register

/* Alarm Configuration Flags -----------------------------------------------*/
#define PCF8563_ALARM_MINUTE         0x01
#define PCF8563_ALARM_HOUR          0x02
#define PCF8563_ALARM_DAY           0x04
#define PCF8563_ALARM_WEEKDAY       0x08

#define PCF8563_ALARM_ENABLED        0x80
#define PCF8563_ALARM_ALL           (PCF8563_ALARM_MINUTE | PCF8563_ALARM_HOUR | \
                                    PCF8563_ALARM_DAY | PCF8563_ALARM_WEEKDAY)

/* Function Declarations ---------------------------------------------------*/
pcf8563_status_t pcf8563_init(I2C_HandleTypeDef *hi2c);
pcf8563_status_t pcf8563_set_time(pcf8563_time_t *time);
pcf8563_status_t pcf8563_get_time(pcf8563_time_t *time); 
pcf8563_status_t pcf8563_is_running(uint8_t *running);
pcf8563_status_t pcf8563_check_voltage_low(uint8_t *vl_detected);
pcf8563_status_t pcf8563_clear_voltage_low(void);

pcf8563_status_t pcf8563_set_alarm(pcf8563_alarm_t *alarm, uint8_t alarm_mask);
pcf8563_status_t pcf8563_get_alarm(pcf8563_alarm_t *alarm);
pcf8563_status_t pcf8563_enable_alarm(uint8_t enable);
pcf8563_status_t pcf8563_get_alarm_flag(uint8_t *triggered);
pcf8563_status_t pcf8563_clear_alarm_flag(void);

#ifdef __cplusplus
}
#endif

#endif /* __PCF8563_I2C_DRIVER_H__ */