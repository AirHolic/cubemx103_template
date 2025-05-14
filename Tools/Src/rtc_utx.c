#include "rtc_utx.h"
#include "rtc.h"
#include <time.h>
#if EXTERNAL_RTC == 1
#include "pcf8563_i2c_driver.h"
#endif

#if RTC_UTX_DEBUG == 1
#define RTC_UTX_LOG(fmt, ...) printf("[RTC UTX] " fmt "\r\n", ##__VA_ARGS__)
#else
#define RTC_UTX_LOG(fmt, ...)
#endif

static HAL_StatusTypeDef sync_pcf8563_to_rtc(Times *time_c);

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
 * @param uts 时间戳
 * @return 日期
 * @note 时区为东八区
 * @retval Times结构体
*/
Times uts_to_rtc(time_t uts)
{
  uts += 8 * 3600;
#ifdef __MDK5
  struct tm *t = localtime(&uts);
#endif // DEBUG
#ifdef __GCC
  struct tm *t = gmtime(&uts);
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
#ifdef RTC_UTX_DEBUG
  RTC_UTX_LOG("t->tm_year = %d\r\n", t->tm_year);
  RTC_UTX_LOG("t->tm_mon = %d\r\n", t->tm_mon);
  RTC_UTX_LOG("t->tm_mday = %d\r\n", t->tm_mday);
  RTC_UTX_LOG("t->tm_hour = %d\r\n", t->tm_hour);
  RTC_UTX_LOG("t->tm_min = %d\r\n", t->tm_min);
  RTC_UTX_LOG("t->tm_sec = %d\r\n", t->tm_sec);
#endif // RTC_UTX_DEBUG
  return rtc;
}

/**
 * @brief 将日期转换为时间戳
 * @param rtc 日期
 * @return 时间戳
 * @note 时区为东八区
 * @retval mktime返回值
*/
time_t rtc_to_uts(Times rtc)
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


/**
 * @brief  从RTC获取当前时间的时间戳
 * @param  None
 * @retval 时间戳
 * @note   根据EXTERNAL_RTC宏决定从内部RTC还是外部PCF8563获取时间
 */
time_t get_uts(void)
{
#if EXTERNAL_RTC == 1
  // 从PCF8563获取时间
  PCF8563_Time_t pcf_time;
  Times time_c;
  
  if(PCF8563_GetTime(&pcf_time) != PCF8563_OK) {
    // PCF8563读取失败，尝试使用内部RTC
    RTC_UTX_LOG("PCF8563 Get Time Error\r\n");
  }
  
  // 检查电压低标志
  if(PCF8563_CheckVL())
  {
    // 电压低，时间不可靠，使用内部RTC
    RTC_UTX_LOG("PCF8563 Voltage Low\r\n");
  }
  
  // if(vl_flag) {
  //   // 电压低，时间不可靠，使用内部RTC
  //   goto use_internal_rtc;
  // }
  
  // PCF8563时间转换为Times结构
  time_c.Year = pcf_time.year;
  time_c.Mon = pcf_time.month;
  time_c.Day = pcf_time.day;
  time_c.Hour = pcf_time.hour;
  time_c.Min = pcf_time.minute;
  time_c.Second = pcf_time.second;
  time_c.WeekDay = pcf_time.weekday;
  
  // 同步到内部RTC
  sync_pcf8563_to_rtc(&time_c);
  
//use_internal_rtc:
#else

  // 使用内部RTC
  RTC_DateTypeDef GetDate = {0}; 
  RTC_TimeTypeDef GetTime = {0};
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
#endif // EXTERNAL_RTC
#ifdef RTC_UTX_DEBUG
  RTC_UTX_LOG("time_c.Year = %d\r\n", time_c.Year);
  RTC_UTX_LOG("time_c.Mon = %d\r\n", time_c.Mon);
  RTC_UTX_LOG("time_c.Day = %d\r\n", time_c.Day);
  RTC_UTX_LOG("time_c.WeekDay = %d\r\n", time_c.WeekDay);
  RTC_UTX_LOG("time_c.Hour = %d\r\n", time_c.Hour);
  RTC_UTX_LOG("time_c.Min = %d\r\n", time_c.Min);
  RTC_UTX_LOG("time_c.Second = %d\r\n", time_c.Second);
#endif // RTC_UTX_DEBUG
  return rtc_to_uts(time_c);
}

/**
 * @brief  获取时间（小时、分钟、秒）
 * @param  hour 小时指针
 * @param  min 分钟指针
 * @param  sec 秒指针
 * @retval None
 */
void get_time(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
#if EXTERNAL_RTC == 1
  // 从PCF8563获取时间
  PCF8563_Time_t pcf_time;
  
  if(PCF8563_GetTime(&pcf_time) == PCF8563_OK) {
    if(!PCF8563_CheckVL()) {
      *hour = pcf_time.hour;
      *min = pcf_time.minute;
      *sec = pcf_time.second;
      return;
    }
  }
  // 如果PCF8563读取失败，回退到内部RTC
#else

  RTC_TimeTypeDef GetTime = {0}; 
  HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);

  *hour = GetTime.Hours;
  *min = GetTime.Minutes;
  *sec = GetTime.Seconds;
  return;
#endif
}

/**
 * @brief  获取日期（日期、星期）
 * @param  date 日期指针
 * @param  weekday 星期指针
 * @retval None
 */
void get_date(uint8_t *date, uint8_t *weekday)
{
#if EXTERNAL_RTC
  // 从PCF8563获取日期
  PCF8563_Time_t pcf_time;
  
  if(PCF8563_GetTime(&pcf_time) == PCF8563_OK) {
    if(!PCF8563_CheckVL()) {
      *date = pcf_time.day;
      *weekday = pcf_time.weekday;
      return;
    }
  }
  // 如果PCF8563读取失败，回退到内部RTC
#endif

  RTC_DateTypeDef GetDate = {0};
  HAL_RTC_GetDate(&hrtc, &GetDate, RTC_FORMAT_BIN);

  *date = GetDate.Date;
  *weekday = GetDate.WeekDay;
}

/**
 * @brief  通过时间戳设置时间
 * @param  uts 时间戳
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef uts_set_time(time_t uts)
{
  Times rtc = uts_to_rtc(uts);
  //HAL_StatusTypeDef status;

#if EXTERNAL_RTC
  // 设置PCF8563时间
  PCF8563_Time_t pcf_time;
  pcf_time.second = rtc.Second;
  pcf_time.minute = rtc.Min;
  pcf_time.hour = rtc.Hour;
  pcf_time.day = rtc.Day;
  pcf_time.weekday = rtc.WeekDay;
  pcf_time.month = rtc.Mon;
  pcf_time.year = rtc.Year;
  
  // 设置PCF8563时间
  int pcf_status = PCF8563_SetTime(&pcf_time);
#endif

  // 同时设置内部RTC时间
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
  
  sTime.Hours = rtc.Hour;
  sTime.Minutes = rtc.Min;
  sTime.Seconds = rtc.Second;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
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

#if EXTERNAL_RTC
  return (pcf_status == PCF8563_OK) ? HAL_OK : HAL_ERROR;
#else
  return HAL_OK;
#endif
}

#if EXTERNAL_RTC
/**
 * @brief  将PCF8563时间同步到内部RTC
 * @param  time_c 时间结构体指针
 * @retval HAL_StatusTypeDef
 */
static HAL_StatusTypeDef sync_pcf8563_to_rtc(Times *time_c)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
  
  sTime.Hours = time_c->Hour;
  sTime.Minutes = time_c->Min;
  sTime.Seconds = time_c->Second;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }

  DateToUpdate.WeekDay = time_c->WeekDay;
  DateToUpdate.Month = time_c->Mon;
  DateToUpdate.Date = time_c->Day;
  DateToUpdate.Year = time_c->Year;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK;
}
#endif

#ifdef STM32F1
/**
 * @brief  通过时间戳设置闹钟
 * @param  uts 时间戳
 * @retval HAL_StatusTypeDef
 * @note   闹钟时间为UTC时间,F1芯片的RTC只支持1个闹钟时间的时分秒设置
 */
HAL_StatusTypeDef uts_set_alarm(time_t uts)
{
  Times rtc = uts_to_rtc(uts);
  RTC_AlarmTypeDef sAlarm = {0};
  RTC_TimeTypeDef sTime = {0};

  sTime.Hours = rtc.Hour;
  sTime.Minutes = rtc.Min;
  sTime.Seconds = rtc.Second;
  sAlarm.AlarmTime = sTime;
  
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef time_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
  RTC_TimeTypeDef sTime = {0};
  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
} 

/**
 * @brief  外置rtc条件下设置秒级闹钟
 * 
 * @param hour 
 * @param min 
 * @param sec 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef time_set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
  RTC_AlarmTypeDef sAlarm = {0};
  RTC_TimeTypeDef sTime = {0};

  sTime.Hours = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;
  sAlarm.AlarmTime = sTime;
  
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}
#endif


