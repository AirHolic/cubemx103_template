#include "rtc_utx.h"
#include "rtc.h"
#if EXTERNAL_RTC
#include "pcf8563_i2c_driver.h"
#endif
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
static int __isleap(int year)
{
  return (!(year % 4) && ((year % 100) || !(year % 400)));
}

/**
 * @brief mktime函数的重写，使用无符号32位整数
 * @param t 时间结构体
 * @return 无符号时间戳
 */
static timestamp_t mymktime(struct tm *const t)
{
  timestamp_t day;
  timestamp_t i;
  timestamp_t years = t->tm_year - 70;

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
    return 0;  // 改为返回0而非-1，因为我们使用无符号整数

  /* 1970年以来的天数等于365 *年数+ 1970年以来的闰年数 */
  day = years * 365 + (years + 1) / 4;

  /* 2100年以后，计算闰年的方式不一样了，每400年减去3个闰年 */
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
 * @param utx 无符号时间戳
 * @return 日期
 * @note 时区为东八区
 * @retval Times结构体
*/
Times utx_to_rtc(timestamp_t utx)
{
  // 加上时区偏移
  utx += 8 * 3600;
  
  // 由于我们不能直接使用标准库的 localtime/gmtime
  // 需要自行计算日期时间
  
  Times rtc;
  uint32_t seconds, minutes, hours, days, year, month, dayOfWeek;
  uint32_t daysPerMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  // 计算秒、分、时
  seconds = utx % 60;
  utx /= 60;
  minutes = utx % 60;
  utx /= 60;
  hours = utx % 24;
  utx /= 24; // utx现在表示从1970年1月1日开始的天数
  
  // 计算星期几 (1970年1月1日是星期四)
  dayOfWeek = (utx + 4) % 7;
  if (dayOfWeek == 0)
    dayOfWeek = 7; // 将星期日从0调整到7
  
  // 计算年份
  year = 1970;
  days = utx;
  
  while (1)
  {
    uint32_t daysInYear = __isleap(year) ? 366 : 365;
    if (days < daysInYear)
      break;
    days -= daysInYear;
    year++;
  }
  
  // 计算月份
  month = 1;
  if (__isleap(year))
    daysPerMonth[2] = 29;
  
  while (days >= daysPerMonth[month])
  {
    days -= daysPerMonth[month];
    month++;
  }
  
  // 填充结果
  rtc.Year = year - 1900 - 100; // 转换为RTC使用的年份格式 (year-2000)
  rtc.Mon = month;
  rtc.Day = days + 1; // 天数从1开始
  rtc.Hour = hours;
  rtc.Min = minutes;
  rtc.Second = seconds;
  rtc.WeekDay = dayOfWeek;
  
#ifdef RTC_DBUG
  Lora_printf("rtc.Year = %d\r\n", rtc.Year);
  Lora_printf("rtc.Mon = %d\r\n", rtc.Mon);
  Lora_printf("rtc.Day = %d\r\n", rtc.Day);
  Lora_printf("rtc.Hour = %d\r\n", rtc.Hour);
  Lora_printf("rtc.Min = %d\r\n", rtc.Min);
  Lora_printf("rtc.Second = %d\r\n", rtc.Second);
#endif
  
  return rtc;
}

/**
 * @brief 将日期转换为时间戳
 * @param rtc 日期
 * @return 无符号时间戳
 * @note 时区为东八区
 * @retval 无符号32位时间戳
*/
timestamp_t rtc_to_utx(Times rtc)
{
  struct tm t;
  t.tm_year = rtc.Year + 100;
  t.tm_mon = rtc.Mon - 1;
  t.tm_mday = rtc.Day;
  t.tm_hour = rtc.Hour-8;  // 减去时区偏移
  t.tm_min = rtc.Min;
  t.tm_sec = rtc.Second;
  t.tm_wday = rtc.WeekDay;
  
  return (timestamp_t)mymktime(&t);  // 使用自定义的mymktime函数返回无符号时间戳
}


/**
 * @brief  从RTC获取当前时间的时间戳
 * @param  None
 * @retval 无符号时间戳
 * @note   根据EXTERNAL_RTC宏决定从内部RTC还是外部PCF8563获取时间
 */
timestamp_t get_uts(void)
{
#if EXTERNAL_RTC
  // 从PCF8563获取时间
  pcf8563_time_t pcf_time;
  Times time_c;
  
  if(pcf8563_get_time(&pcf_time) != PCF8563_OK) {
    // PCF8563读取失败，尝试使用内部RTC
    goto use_internal_rtc;
  }
  
  // 检查电压低标志
  uint8_t vl_flag = 0;
  pcf8563_check_voltage_low(&vl_flag);
  
  // PCF8563时间转换为Times结构
  time_c.Year = pcf_time.years;
  time_c.Mon = pcf_time.months;
  time_c.Day = pcf_time.days;
  time_c.Hour = pcf_time.hours;
  time_c.Min = pcf_time.minutes;
  time_c.Second = pcf_time.seconds;
  time_c.WeekDay = pcf_time.weekdays;
  
  // 同步到内部RTC
  sync_pcf8563_to_rtc(&time_c);
  
  return rtc_to_utx(time_c);
  
//use_internal_rtc:
#endif

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
 * @brief  获取时间（小时、分钟、秒）
 * @param  hour 小时指针
 * @param  min 分钟指针
 * @param  sec 秒指针
 * @retval None
 */
void get_time(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
#if EXTERNAL_RTC
  // 从PCF8563获取时间
  pcf8563_time_t pcf_time;
  
  if(pcf8563_get_time(&pcf_time) == PCF8563_OK) {
    uint8_t vl_flag = 0;
    pcf8563_check_voltage_low(&vl_flag);
    if(!vl_flag) {
      *hour = pcf_time.hours;
      *min = pcf_time.minutes;
      *sec = pcf_time.seconds;
      return;
    }
  }
  // 如果PCF8563读取失败，回退到内部RTC
#endif

  RTC_TimeTypeDef GetTime = {0}; 
  HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);

  *hour = GetTime.Hours;
  *min = GetTime.Minutes;
  *sec = GetTime.Seconds;
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
  pcf8563_time_t pcf_time;
  
  if(pcf8563_get_time(&pcf_time) == PCF8563_OK) {
    uint8_t vl_flag = 0;
    pcf8563_check_voltage_low(&vl_flag);
    if(!vl_flag) {
      *date = pcf_time.days;
      *weekday = pcf_time.weekdays;
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
 * @param  utx 无符号时间戳
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef uts_set_time(timestamp_t utx)
{
  Times rtc = utx_to_rtc(utx);
  HAL_StatusTypeDef status;

#if EXTERNAL_RTC
  // 设置PCF8563时间
  pcf8563_time_t pcf_time;
  pcf_time.seconds = rtc.Second;
  pcf_time.minutes = rtc.Min;
  pcf_time.hours = rtc.Hour;
  pcf_time.days = rtc.Day;
  pcf_time.weekdays = rtc.WeekDay;
  pcf_time.months = rtc.Mon;
  pcf_time.years = rtc.Year;
  
  // 清除掉电标志
  pcf8563_clear_voltage_low();
  
  // 设置PCF8563时间
  pcf8563_status_t pcf_status = pcf8563_set_time(&pcf_time);
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
 * @param  utx 无符号时间戳
 * @retval HAL_StatusTypeDef
 * @note   闹钟时间为UTC时间,F1芯片的RTC只支持1个闹钟时间的时分秒设置
 */
HAL_StatusTypeDef uts_set_alarm(timestamp_t utx)
{
  Times rtc = utx_to_rtc(utx);
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


