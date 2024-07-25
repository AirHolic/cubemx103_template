#include "rtc_utx.h"
#include <time.h>
/**
 * @brief 时间戳日期数组
 */
const short __mday[13] =
    {
        0,
        (31),
        (31 + 28),
        (31 + 28 + 31),
        (31 + 28 + 31 + 30),
        (31 + 28 + 31 + 30 + 31),
        (31 + 28 + 31 + 30 + 31 + 30),
        (31 + 28 + 31 + 30 + 31 + 30 + 31),
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
        (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31),
};

/**
 * @brief 判断是否为闰年
 * @param year 年份
 * @return 1为闰年，0为平年
 */
int __isleap(int year)
{
  return (!(year % 4) && ((year % 100) || !(year % 400)));
}

/**
 * @brief mktime函数的重写
 * @param t 时间结构体
 * @return 时间戳
 */
time_t mymktime(struct tm *const t)
{
  time_t day;
  time_t i;
  time_t years = t->tm_year - 70;

  if (t->tm_sec > 60)
  {
    t->tm_min += t->tm_sec / 60;
    t->tm_sec %= 60;
  }

  if (t->tm_min > 60)
  {
    t->tm_hour += t->tm_min / 60;
    t->tm_min %= 60;
  }

  if (t->tm_hour > 24)
  {
    t->tm_mday += t->tm_hour / 24;
    t->tm_hour %= 24;
  }

  if (t->tm_mon > 11) // 原12，-1为0-11
  {
    t->tm_year += t->tm_mon / 12;
    t->tm_mon %= 12;
    years = t->tm_year - 70;
  }

  while (t->tm_mday > __mday[1 + t->tm_mon]) // 0-11
  {
    if (t->tm_mon == 1 && __isleap(t->tm_year + 1900))
    {
      --t->tm_mday;
    }
    t->tm_mday -= __mday[t->tm_mon];
    ++t->tm_mon;

    if (t->tm_mon > 11)
    {
      t->tm_mon = 0;
      ++t->tm_year;
    }
  }

  if (t->tm_year < 70)
    return (time_t)-1;

  /* 1970年以来的天数等于365 *年数+ 1970年以来的闰年数 */
  day = years * 365 + (years + 1) / 4;

  /* 2100年以后，计算闰年的方式不一样了，每400年减去3个闰年，大多数mktime实现不支持2059年后的日期，所以可以把这个省略掉 */
  if ((int)(years -= 131) >= 0)
  {
    years /= 100;
    day -= (years >> 2) * 3 + 1;

    if ((years &= 3) == 3)
      years--;

    day -= years;
  }

  day += t->tm_yday = __mday[t->tm_mon] + t->tm_mday - 1 + (__isleap(t->tm_year + 1900) & (t->tm_mon > 1));

  /* 现在是自1970年1月1日以来的天数 */
  i = 7;
  t->tm_wday = (day + 4) % i; /* 星期天=0, 星期一=1, ..., 星期六=6 */

  i = 24;
  day *= i;
  i = 60;
  return ((day + t->tm_hour) * i + t->tm_min) * i + t->tm_sec;
}

/**
 * @brief 将时间戳转换为日期
 * @param utx 时间戳
 * @return 日期
 * @note 时区为东八区
 * @retval Times结构体
*/
Times utx_to_rtc(time_t utx)
{
  utx += 8 * 3600;
#ifdef __MDK5
  struct tm *t = localtime(&utx);
#endif // DEBUG
#ifdef __GCC
  struct tm *t = gmtime(&utx);
#endif // DEBUG
  Times rtc;
  rtc.Year = t->tm_year - 100;
  rtc.Mon = t->tm_mon + 1;
  rtc.Day = t->tm_mday;
  rtc.Hour = t->tm_hour;
  rtc.Min = t->tm_min;
  rtc.Second = t->tm_sec;
  rtc.WeekDay = t->tm_wday;
  if(rtc.WeekDay == 0)
  {
    rtc.WeekDay = 7;
  }
#ifdef RTC_DBUG
  Lora_printf("t->tm_year = %d\r\n", t->tm_year);
  Lora_printf("t->tm_mon = %d\r\n", t->tm_mon);
  Lora_printf("t->tm_mday = %d\r\n", t->tm_mday);
  Lora_printf("t->tm_hour = %d\r\n", t->tm_hour);
  Lora_printf("t->tm_min = %d\r\n", t->tm_min);
  Lora_printf("t->tm_sec = %d\r\n", t->tm_sec);
#endif
  return rtc;
}

/**
 * @brief 将日期转换为时间戳
 * @param rtc 日期
 * @return 时间戳
 * @note 时区为东八区
 * @retval mktime返回值
*/
time_t rtc_to_utx(Times rtc)
{
  struct tm t;
  t.tm_year = rtc.Year + 100;
  t.tm_mon = rtc.Mon - 1;
  t.tm_mday = rtc.Day;
  t.tm_hour = rtc.Hour-8;
  t.tm_min = rtc.Min;
  t.tm_sec = rtc.Second;
  t.tm_wday = rtc.WeekDay;
  return mymktime(&t);
}

#ifdef __RTC_EXAMPLE
/**
  * @brief  获取当前时间的时间戳
  * @param  None
  * @retval 时间戳
  */
time_t get_uts(void)
{
  RTC_DateTypeDef GetDate = {0}; // 获取日期结构体
  RTC_TimeTypeDef GetTime = {0}; // 获取时间结构体

  Times time_c;

  HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);

  time_c.Year = GetDate.Year;
  time_c.Mon = GetDate.Month;
  time_c.Day = GetDate.Date;
  time_c.Hour = GetTime.Hours;
  time_c.Min = GetTime.Minutes;
  time_c.Second = GetTime.Seconds;
  time_c.WeekDay = GetDate.WeekDay;
#ifdef RTC_DBUG
  Lora_printf("time_c.Year = %d\r\n", time_c.Year);
  Lora_printf("time_c.Mon = %d\r\n", time_c.Mon);
  Lora_printf("time_c.Day = %d\r\n", time_c.Day);
  Lora_printf("time_c.WeekDay = %d\r\n", time_c.WeekDay);
  Lora_printf("time_c.Hour = %d\r\n", time_c.Hour);
  Lora_printf("time_c.Min = %d\r\n", time_c.Min);
  Lora_printf("time_c.Second = %d\r\n", time_c.Second);
#endif // RTC_DBUG
  return rtc_to_utx(time_c);
}

/**
 * @brief  通过时间戳设置时间
 * @param  utx 时间戳
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef uts_set_time(time_t utx)
{
  Times rtc = utx_to_rtc(utx);
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
#ifdef RTC_DBUG
  Lora_printf("rtc.Year = %d\r\n", rtc.Year);
  Lora_printf("rtc.Mon = %d\r\n", rtc.Mon);
  Lora_printf("rtc.Day = %d\r\n", rtc.Day);
  Lora_printf("rtc.WeekDay = %d\r\n", rtc.WeekDay);
  Lora_printf("rtc.Hour = %d\r\n", rtc.Hour);
  Lora_printf("rtc.Min = %d\r\n", rtc.Min);
  Lora_printf("rtc.Second = %d\r\n", rtc.Second);
#endif // RTC_DBUG
  sTime.Hours = rtc.Hour;
  sTime.Minutes = rtc.Min;
  sTime.Seconds = rtc.Second;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)//
  {
    return HAL_ERROR;
  }

  DateToUpdate.WeekDay = rtc.WeekDay;
  DateToUpdate.Month = rtc.Mon;
  DateToUpdate.Date = rtc.Day;
  DateToUpdate.Year = rtc.Year;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}
#endif // __RTC_EXAMPLE