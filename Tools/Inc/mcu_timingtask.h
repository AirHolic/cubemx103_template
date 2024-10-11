#ifndef __MCU_TIMINGTASK_H__
#define __MCU_TIMINGTASK_H__

#include "main.h"

#define TIMINGTASK_STORAGE_IN_FLASH 1

#define TIMINGTASK_ADDR FLASH_PAGEx(90)

#if TIMINGTASK_STORAGE_IN_FLASH == 1
#define TIMINGTASK_BACKUP_ADDR FLASH_PAGEx(100)
#endif

#define TIMINGTASK_NUM 20
#define TIMINGTASK_BUF_SIZE 500

typedef void (*timingtask_save)(uint32_t addr, uint32_t *data, uint32_t len);
typedef void (*timingtask_load)(uint32_t addr, uint32_t *data, uint32_t len);
typedef void (*timingtask_erase)(uint32_t addr, uint32_t len);
typedef uint8_t (*timingtask_set_alarm)(uint8_t hour, uint8_t min, uint8_t sec);

/**
 * @brief Timing task function pointer.
 */
typedef struct
{
    timingtask_save timingtask_save;
    timingtask_load timingtask_load;
    timingtask_erase timingtask_erase;
    timingtask_set_alarm timingtask_set_alarm;
}mcu_timingtask_func_t;

/**
 * @brief Timing task stroage structure.
 */
typedef struct 
{
    uint32_t validity_time;
    uint32_t invalidty_time;
    uint32_t timingtask_id;
    uint8_t timingtask_running_cycle:5;//0x11:everyday,0x12:everyweek,0x13:everymonth
    uint8_t timingtask_running_cycle_unit:5;//everyweek:0x01-0x07/everymonth:0x01-0x1F
    uint8_t timingtask_hour:5;//running hour 0-23
    uint8_t timingtask_min:6;//running minute 0-59
    uint8_t timingtask_sec:6;//running second 0-59
    uint8_t timingtask_buf[TIMINGTASK_BUF_SIZE];
}MCU_TIMINGTASK_T;

typedef enum
{
    TIMINGTASK_RUNNING_CYCLE_EVERYDAY = 0x11,
    TIMINGTASK_RUNNING_CYCLE_EVERYWEEK = 0x12,
    TIMINGTASK_RUNNING_CYCLE_EVERYMONTH = 0x13
}TIMINGTASK_RUNNING_CYCLE_T;

void mcu_timingtask_init(void);
uint8_t mcu_timingtask_sorting(void);
uint8_t mcu_timingtask_excute_index_set(void);
uint8_t mcu_timingtask_add(MCU_TIMINGTASK_T *timingtask);
uint8_t mcu_search_timingtask_id(uint32_t timingtask_id);
uint8_t mcu_timingtask_delete(uint32_t *timingtask_id, uint8_t delete_count);
void mcu_timingtask_set_alarm(void);
uint8_t *mcu_timingtask_alarm_buf(void);
uint16_t mcu_timingtask_alarm_buf_len(void);

#endif /* __MCU_TIMINGTASK_H__ */
