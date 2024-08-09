#ifndef __RTC_UTX_H__
#define __RTC_UTX_H__

#define __MDK5

#ifdef __GCC
#include <sys/time.h>
#endif /* GCC */
#ifdef __MDK5
#include <time.h>
#endif /* MDK5 */

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

Times utx_to_rtc(time_t utx);
time_t rtc_to_utx(Times rtc);

#endif /* _RTC_UTX_H__ */
