#ifndef __MCU_TIMINGTASK_H__
#define __MCU_TIMINGTASK_H__

#include "main.h"
#include "w25qxx_spi_driver.h"

// Configuration
#define TIMINGTASK_STORAGE_IN_MCUFLASH 0

#if (TIMINGTASK_STORAGE_IN_MCUFLASH == 1)
#define TIMINGTASK_ADDR FLASH_PAGEx(90)
#define TIMINGTASK_BACKUP_ADDR FLASH_PAGEx(100)
#else
#define TIMINGTASK_ADDR W25QXX_SECTOR_ADDR(1)
#define TIMINGTASK_BACKUP_ADDR W25QXX_SECTOR_ADDR(40)
#endif

#define TIMINGTASK_NUM 20
#define TIMINGTASK_BUF_SIZE 500

#define MCU_TIMINGTASK_DEBUG 1

typedef struct
{
    uint16_t cmd_code;
    uint16_t timingtask_buf_len;
    uint8_t timingtask_buf[TIMINGTASK_BUF_SIZE];
}mcu_timingtask_content_t;

// Core data structures
#pragma pack(push, 1)

typedef struct 
{
    uint32_t validity_time;//生效时间戳
    uint32_t invalidty_time;//失效时间戳
    uint32_t timingtask_id;//timing task id
    uint32_t timingtask_running_cycle_unit;//bit1-7:everyweek,bit1-31:everymonth,bit0:no use,1:valid,0:invalid
    uint8_t timingtask_running_cycle:5;//0x11:everyday,0x12:everyweek,0x13:everymonth
    uint8_t timingtask_hour:5;//running hour 0-23
    uint8_t timingtask_min:6;//running minute 0-59
    uint8_t timingtask_sec:6;//running second 0-59
    mcu_timingtask_content_t timingtask_content;
    //uint8_t timingtask_buf[TIMINGTASK_BUF_SIZE];//timing task data,length is buf[0] and buf[1]
}MCU_TIMINGTASK_T;

#pragma pack(pop)

// Function pointer types
typedef void (*timingtask_save)(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len);
typedef void (*timingtask_load)(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len);
typedef void (*timingtask_erase)(uint32_t addr, uint32_t len);
typedef uint8_t (*timingtask_set_alarm)(uint8_t hour, uint8_t min, uint8_t sec);
typedef void (*timingtask_get_time)(uint8_t *hour, uint8_t *min, uint8_t *sec);
typedef void (*timingtask_get_date)(uint8_t *date, uint8_t *weekday);

typedef struct
{
    timingtask_save timingtask_save;
    timingtask_load timingtask_load;
    timingtask_erase timingtask_erase;
    timingtask_set_alarm timingtask_set_alarm;
    timingtask_get_time timingtask_get_time;  // 获取当前时间
    timingtask_get_date timingtask_get_date;  // 获取当前日期
}mcu_timingtask_func_t;

// Enums
typedef enum
{
    TIMINGTASK_RUNNING_CYCLE_EVERYDAY = 0x11,
    TIMINGTASK_RUNNING_CYCLE_EVERYWEEK = 0x12,
    TIMINGTASK_RUNNING_CYCLE_EVERYMONTH = 0x13
}TIMINGTASK_RUNNING_CYCLE_T;

// Function declarations
void mcu_timingtask_init(void);
uint8_t mcu_timingtask_sorting(void);
uint8_t mcu_timingtask_execute_index_set(void);
uint8_t mcu_timingtask_add(MCU_TIMINGTASK_T *timingtask);
uint8_t mcu_search_timingtask_id(uint32_t timingtask_id);
uint8_t mcu_return_all_timingtask_id(uint8_t *timingtask_id);
uint8_t mcu_timingtask_delete(uint32_t *timingtask_id, uint8_t delete_count);
void mcu_timingtask_set_alarm(void);

mcu_timingtask_content_t *mcu_timingtask_alarm_buf(uint32_t *cid);
uint8_t mcu_timingtask_delete_invalid(void);
mcu_timingtask_content_t *same_timingtask(uint32_t *cid);

#endif /* __MCU_TIMINGTASK_H__ */
