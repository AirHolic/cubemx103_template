#ifndef __RTC_UTX_H__
#define __RTC_UTX_H__

#define __MDK5

#include "main.h"
#ifdef __GCC
#include <sys/time.h>
#endif /* GCC */
#ifdef __MDK5
#include <time.h>
#endif /* MDK5 */

#define EXTERNAL_RTC 1
#define RTC_UTX_DEBUG 1

/**
 * @brief 时间日期结构体
 */
typedef struct times
{
    int Year;
    int Mon;
    int Day;
    int Hour;
    int Min;
    int Second;
    int WeekDay;
} Times;

Times uts_to_rtc(time_t uts);
time_t rtc_to_uts(Times rtc);

time_t get_uts(void);
void get_time(uint8_t *hour, uint8_t *min, uint8_t *sec);
void get_date(uint8_t *date, uint8_t *weekday);
HAL_StatusTypeDef uts_set_time(time_t uts);
HAL_StatusTypeDef time_set_time(uint8_t hour, uint8_t min, uint8_t sec);
HAL_StatusTypeDef uts_set_alarm(time_t uts);
HAL_StatusTypeDef time_set_alarm(uint8_t hour, uint8_t min, uint8_t sec);

#endif /* _RTC_UTX_H__ */
