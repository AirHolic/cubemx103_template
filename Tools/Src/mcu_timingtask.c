#include "main.h"
#include "rtc_utx.h"
#include "rtc.h"
#include "sys_delay.h"
#include "mcu_flash.h"
#include "string.h"
#include "usart.h"
#include "mcu_timingtask.h"

mcu_timingtask_func_t mcu_timingtask_func;

uint32_t mcu_timingtask_addr = TIMINGTASK_ADDR;//Timing task storage address
uint8_t mcu_timingtask_num = 0;//Number of timing tasks

#define MCU_TIMINGTASK_T_SIZE sizeof(MCU_TIMINGTASK_T)
#define MCU_TIMINGTASK_T_INDEX(x) (mcu_timingtask_addr + x * MCU_TIMINGTASK_T_SIZE)//request address of timing task
#define MCU_RUNNING_CYCLE_UNIT_JUDGE(running_cycle_unit, weekdate) ((running_cycle_unit >> weekdate) & 0x01)
#define WEEKTRASNFORM(weekdate) (weekdate == 0 ? 7 : weekdate)

static uint8_t set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
    return time_set_alarm(hour, min, sec);
}

#if TIMINGTASK_STORAGE_IN_FLASH == 1
MCU_TIMINGTASK_T mcu_timingtask_read_cache[2];//0:task cache,1:backup cache

uint8_t mcu_timingtask_excute_sort[TIMINGTASK_NUM];//Timing task execution order
uint8_t mcu_timingtask_excute_index = 0;//Timing task execution index
//example MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index])

/**
 * @brief Get the timing task execution buffer.
 * @return Timing task execution buffer.
 */
void mcu_timingtask_read(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len)
{
    #if RTOS == 1
    uint32_t *data32;
    data32 = pvPortMalloc(len);
    mcu_flash_read(addr, data32, len/sizeof(uint32_t));
    memcpy(data, data32, len);
    vPortFree(data32);
    #else
    uint32_t data32[len/sizeof(uint32_t)];
    mcu_flash_read(addr, data32, len/sizeof(uint32_t));
    memcpy(data, data32, len);
    #endif
}

/**
 * @brief Write the timing task to the storage address.
 */
void mcu_timingtask_nocheck_write(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t size)
{
    #if RTOS == 1
    uint32_t *data32;
    data32 = pvPortMalloc(size);
    memcpy(data32, data, size);
    mcu_flash_nocheck_write(addr, data32, size/sizeof(uint32_t));
    vPortFree(data32);
    #else
    uint32_t data32[size/sizeof(uint32_t)];
    memcpy(data32, data, size);
    mcu_flash_nocheck_write(addr, data32, size/sizeof(uint32_t));
    #endif
}

/**
 * @brief Initialize the timing task.
 */
void mcu_timingtask_init(void)
{
    mcu_timingtask_func.timingtask_load = mcu_timingtask_read;
    mcu_timingtask_func.timingtask_save = mcu_timingtask_nocheck_write;//directly write to flash without checking
    mcu_timingtask_func.timingtask_erase = mcu_flash_erase;
    mcu_timingtask_func.timingtask_set_alarm = set_alarm;
    //Load the timing task from the storage address.
    mcu_timingtask_func.timingtask_load(mcu_timingtask_addr, &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    if(mcu_timingtask_read_cache[0].timingtask_id == 0xFFFFFFFF)
    {
        mcu_timingtask_addr = TIMINGTASK_BACKUP_ADDR;//Timing task storage address
    }
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if (mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF)
        {
            mcu_timingtask_num++;
            if(mcu_timingtask_num >= TIMINGTASK_NUM)
            {
                break;
            }
        }
        else
        {
            break;
        }

        sys_delay_ms(10);
    }
    printf("mcu_timingtask_num:%d\r\n", mcu_timingtask_num);
    // mcu_timingtask_sorting();
    // mcu_timingtask_excute_index_set();
}

/**
 *  @brief timingtask sorting by a day.after init or revise timingtask,call this function.
*/
uint8_t mcu_timingtask_sorting(void)
{
    uint8_t i, j, k;
    i = 0, j = 1, k = 0;
    uint32_t i_time, j_time;
    mcu_timingtask_excute_index = 0;
    memset(mcu_timingtask_excute_sort, 0, TIMINGTASK_NUM);
    if(mcu_timingtask_num <= 1)
    {
        mcu_timingtask_excute_sort[0] = 0;
        return 0;
    }
    while(i < mcu_timingtask_num)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(j), &mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
        i_time = mcu_timingtask_read_cache[0].timingtask_hour * 3600 + mcu_timingtask_read_cache[0].timingtask_min * 60 + mcu_timingtask_read_cache[0].timingtask_sec;
        j_time = mcu_timingtask_read_cache[1].timingtask_hour * 3600 + mcu_timingtask_read_cache[1].timingtask_min * 60 + mcu_timingtask_read_cache[1].timingtask_sec;
        
        if(i_time > j_time)
        {
            k++;
            j++;
        }
        else
        {
            j++;
        }
        if(j == mcu_timingtask_num)
        {   
            while(mcu_timingtask_excute_sort[k] != 0)
            {
                k++;
            }
            mcu_timingtask_excute_sort[k] = i;
            printf("sort id:%#8X\r\n", mcu_timingtask_read_cache[0].timingtask_id);
            i++;
            j = 0;
            k = 0;
        }
    }
    
    return 0;
}

/**
 * @brief Set the timing task execution index.after sorting or revise timingtask,and set alarm,call this function.
 * @return 1:All timing tasks have been executed.
 *        0:There are still timing tasks to be executed.
 */
uint8_t mcu_timingtask_excute_index_set(void)
{
    if(mcu_timingtask_num == 0 || mcu_timingtask_excute_index >= mcu_timingtask_num)
    {
        mcu_timingtask_excute_index = 0;
        return 1;
    }
    uint32_t current_time = get_uts();
    uint8_t date, weekday;
    uint8_t hour, min, sec;
    get_time(&hour, &min, &sec);
    get_date(&date, &weekday);
    uint32_t hms_time = hour * 3600 + min * 60 + sec;
    uint32_t hms_takstime;
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        hms_takstime = mcu_timingtask_read_cache[0].timingtask_hour * 3600 + mcu_timingtask_read_cache[0].timingtask_min * 60 + mcu_timingtask_read_cache[0].timingtask_sec;
        if(hms_takstime < hms_time)
        {
            mcu_timingtask_excute_index++;
        }
        else if(mcu_timingtask_excute_index >= mcu_timingtask_num)
        {
            mcu_timingtask_excute_index = 0;
            return 1;
        }
        else break;
        //sys_delay_ms(1);
    }
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if((current_time < mcu_timingtask_read_cache[0].validity_time) || (current_time > mcu_timingtask_read_cache[0].invalidty_time))
        {//timingtask invalidty
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        else if((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x12) && (MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, WEEKTRASNFORM(weekday)))) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x13) && (MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, date))))
        {//timingtask running cycle
            return 0;
        }
        else
        {
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        sys_delay_ms(10);
    }
}

/**
 * @brief Timing task execution.
 * @param timingtask The timing task to be executed.
 * @return 0:Success 1:Over full 2:ID repeat
 */
uint8_t mcu_timingtask_add(MCU_TIMINGTASK_T *timingtask)
{
    if(mcu_timingtask_num >= TIMINGTASK_NUM)
    {
        return 1;
    }
    if(mcu_search_timingtask_id(timingtask->timingtask_id) != 0xFF)
    {
        return 2;
    }
    mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), timingtask, MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_num++;
    mcu_timingtask_sorting();
    mcu_timingtask_set_alarm();

    return 0;
}

/**
 * @brief Search the timing task by ID.
 * @param timingtask_id The timing task ID to be searched.
 * @return The index of the timing task.
 *          0xFF: The timing task is not found.
 */
uint8_t mcu_search_timingtask_id(uint32_t timingtask_id)
{
    for(uint32_t i = 0; i < mcu_timingtask_num; i++)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[0].timingtask_id == timingtask_id)
        {
            return i;
        }
    }
    return 0xFF;
}

/**
 * @brief Retrun all timingtask id.
 * @param timingtask_id The timing task ID to be returned.
 * @return The number of timing tasks.
 */
uint8_t mcu_return_all_timingtask_id(uint8_t *timingtask_id)
{
    for(uint8_t i = 0; i < mcu_timingtask_num * 4; i+=4)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i/4), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        timingtask_id[i] = mcu_timingtask_read_cache[0].timingtask_id >> 24;
        timingtask_id[i + 1] = mcu_timingtask_read_cache[0].timingtask_id >> 16;
        timingtask_id[i + 2] = mcu_timingtask_read_cache[0].timingtask_id >> 8;
        timingtask_id[i + 3] = mcu_timingtask_read_cache[0].timingtask_id;
    }
    return mcu_timingtask_num;
}

/**
 * @brief Delete the timing task.
 * @param timingtask_id The timing task ID to be deleted.
 * @return 0:Success 1:Failure
 */
uint8_t mcu_timingtask_delete(uint32_t *timingtask_id, uint8_t delete_count)
{
    if(delete_count > mcu_timingtask_num)
    {
        return 1;
    }
    #if RTOS == 1
    uint8_t *index = pvPortMalloc(delete_count);
    #else
    uint8_t index[delete_count];
    #endif
    for(uint8_t i = 0; i < delete_count; i++)
    {
        index[i] = mcu_search_timingtask_id(timingtask_id[i]);
        if(index[i] == 0xFF)
        {
            i--;
            delete_count--;
        }
    }
    sys_delay_ms(10);
    if(delete_count == 0)
    {
        #if RTOS == 1
        vPortFree(index);
        #endif
        return 1;
    }
    uint8_t save_index = 0;
    
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        for(uint8_t j = 0; j < delete_count; j++)
        {
            if(i == index[j])
                i++;
        }
        if(i == mcu_timingtask_num)
            break;
        switch (mcu_timingtask_addr)
        {
            case TIMINGTASK_ADDR:
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                mcu_timingtask_func.timingtask_save(TIMINGTASK_BACKUP_ADDR + save_index * MCU_TIMINGTASK_T_SIZE, &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                save_index++;
                break;
            case TIMINGTASK_BACKUP_ADDR:
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                mcu_timingtask_func.timingtask_save(TIMINGTASK_ADDR + save_index * MCU_TIMINGTASK_T_SIZE, &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                save_index++;
                break;
            default:
                break;
        }
    }
    mcu_timingtask_func.timingtask_erase(mcu_timingtask_addr, mcu_timingtask_num * MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_num -= delete_count;
    mcu_timingtask_addr = (mcu_timingtask_addr == TIMINGTASK_ADDR) ? TIMINGTASK_BACKUP_ADDR : TIMINGTASK_ADDR;

    mcu_timingtask_sorting();
    mcu_timingtask_set_alarm();
    #if RTOS == 1
    vPortFree(index);
    #endif
    return 0;
}

/**
 * @brief Set the alarm.init or revise timingtask,call this function.auto to set mcu_timingtask_excute_index;
 */
void mcu_timingtask_set_alarm(void)
{
    // uint8_t hour, min, sec;
    // get_time(&hour, &min, &sec);
    // LORA_printf("hour = %d, min = %d, sec = %d\r\n", hour, min, sec);
    // mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    // while(1)
    // {
    //     if((hour > mcu_timingtask_read_cache[0].timingtask_hour) || \
    //         (hour == mcu_timingtask_read_cache[0].timingtask_hour && min > mcu_timingtask_read_cache[0].timingtask_min) || \
    //         (hour == mcu_timingtask_read_cache[0].timingtask_hour && min == mcu_timingtask_read_cache[0].timingtask_min && sec >= mcu_timingtask_read_cache[0].timingtask_sec))
    //         {
    //             LORA_printf("mcu_timingtask_excute_index = %d\r\n", mcu_timingtask_excute_index);
    //             mcu_timingtask_excute_index++;
    //         }
    //     sys_delay_ms(10);
    // }

    if(mcu_timingtask_excute_index_set() == 0)
    {
        memset(&mcu_timingtask_read_cache[0], 0, MCU_TIMINGTASK_T_SIZE);

        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        mcu_timingtask_func.timingtask_set_alarm(mcu_timingtask_read_cache[0].timingtask_hour, mcu_timingtask_read_cache[0].timingtask_min, mcu_timingtask_read_cache[0].timingtask_sec);
        printf("set alarm id:%#8X\r\n", mcu_timingtask_read_cache[0].timingtask_id);
    }
    else 
    {
        mcu_timingtask_func.timingtask_set_alarm(0, 0, 3);
        mcu_timingtask_excute_index = 0;
        printf("set alarm end\r\n");
    }
}

/**
 * @brief Delete the invalid timing task.
 */
uint8_t mcu_timingtask_delete_invalid(void)
{
    if(mcu_timingtask_num == 0)
    {
        return 0;
    }
    uint32_t current_time = get_uts();
    uint32_t delete_invalid_id[mcu_timingtask_num];
    uint8_t delete_count = 0;
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
        if(current_time > mcu_timingtask_read_cache[1].invalidty_time)
        {
            delete_invalid_id[delete_count] = mcu_timingtask_read_cache[1].timingtask_id;
            delete_count++;
        }
    }
    if(delete_count > 0)
    {
        mcu_timingtask_delete(delete_invalid_id, delete_count);
        return delete_count;
    }
    return 0;
}

/**
 * @brief return alarm buf
*/
uint8_t *mcu_timingtask_alarm_buf(uint32_t *cid)
{
    uint8_t index;
    if(mcu_timingtask_excute_index == 0 || mcu_timingtask_num == 1)
    {
        index = 0;
    }
    else
    {
        index = mcu_timingtask_excute_index;
    }
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    *cid = mcu_timingtask_read_cache[0].timingtask_id;
    return mcu_timingtask_read_cache[0].timingtask_buf;
}

/**
 * @brief return the same timingtask buf
*/
uint8_t *same_timingtask(uint32_t *cid)
{
    if(mcu_timingtask_num == 1)
    {
        return NULL;
    }
    uint8_t date, weekday;
    uint32_t timea, timeb ,current_time;
    current_time = get_uts();
    timea = mcu_timingtask_read_cache[0].timingtask_hour * 3600 + mcu_timingtask_read_cache[0].timingtask_min * 60 + mcu_timingtask_read_cache[0].timingtask_sec;
    get_date(&date, &weekday);
    while(1)
    {
        if(mcu_timingtask_excute_index + 1 >= mcu_timingtask_num)
        {
            return NULL;
        }
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]), &mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
        timeb = mcu_timingtask_read_cache[1].timingtask_hour * 3600 + mcu_timingtask_read_cache[1].timingtask_min * 60 + mcu_timingtask_read_cache[1].timingtask_sec;
        if((timeb == timea) && (mcu_timingtask_read_cache[1].validity_time <= current_time) && (mcu_timingtask_read_cache[1].invalidty_time >= current_time))
        {
            mcu_timingtask_excute_index++;
            if((mcu_timingtask_read_cache[1].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[1].timingtask_running_cycle == 0x12) &&  MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[1].timingtask_running_cycle_unit, WEEKTRASNFORM(weekday))) || \
            ((mcu_timingtask_read_cache[1].timingtask_running_cycle == 0x13) &&  MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[1].timingtask_running_cycle_unit, date)))
            {
                *cid = mcu_timingtask_read_cache[1].timingtask_id;
                return mcu_timingtask_read_cache[1].timingtask_buf;
            }
        }
        else
        {
            return NULL;
        }
        sys_delay_ms(10);
    }
}

#else
MCU_TIMINGTASK_T mcu_timingtask_read_cache[TIMINGTASK_NUM];

/**
 * @brief Get the timing task execution buffer.
 * @return Timing task execution buffer.
 */
void mcu_timingtask_read(uint32_t addr, MCU_TIMINGTASK_T *data, uint32_t len)
{
    #if RTOS == 1
    uint32_t *data32;
    data32 = pvPortMalloc(len);
    mcu_flash_read(addr, data32, len/sizeof(uint32_t));
    memcpy(data, data32, len);
    vPortFree(data32);
    #else
    uint32_t data32[len/sizeof(uint32_t)];
    mcu_flash_read(addr, data32, len/sizeof(uint32_t));
    memcpy(data, data32, len);
    #endif
}

/**
 * @brief Write the timing task to the storage address.
 */
void mcu_timingtask_nocheck_write(uint32_t addr, uint32_t *data, uint32_t size)
{
    #if RTOS == 1
    uint32_t *data32;
    data32 = pvPortMalloc(size);
    memcpy(data32, data, size);
    mcu_flash_nocheck_write(addr, data32, size/sizeof(uint32_t));
    vPortFree(data32);
    #else
    uint32_t data32[size/sizeof(uint32_t)];
    memcpy(data32, data, size);
    mcu_flash_nocheck_write(addr, data32, size/sizeof(uint32_t));
    #endif
}

/**
 * @brief Initialize the timing task.
 */
void mcu_timingtask_init(void)
{
    mcu_timingtask_func.timingtask_load = mcu_timingtask_read;
    mcu_timingtask_func.timingtask_save = mcu_timingtask_nocheck_write//directly write to flash without checking
    mcu_timingtask_func.timingtask_erase = mcu_flash_erase;
    mcu_timingtask_func.timingtask_set_alarm = set_alarm;
    //Load the timing task from the storage address.
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), &mcu_timingtask_read_cache[mcu_timingtask_num], MCU_TIMINGTASK_T_SIZE);
        if (mcu_timingtask_read_cache[0].timingtask_id != 0xFFFFFFFF)
        {
            mcu_timingtask_num++;
            if(mcu_timingtask_num >= TIMINGTASK_NUM)
            {
                break;
            }
        }
        else
        {
            break;
        }

        sys_delay_ms(10);
    }
    //LORA_printf("mcu_timingtask_num = %d\r\n", mcu_timingtask_num);
    // mcu_timingtask_sorting();
    // mcu_timingtask_excute_index_set();
}


/**
 *  @brief timingtask sorting by a day.after init or revise timingtask,call this function.
*/
uint8_t mcu_timingtask_sorting(void)
{
    int i, j, k;
    i = 0;
    j = 1;
    k = 0;
    mcu_timingtask_excute_index = 0;
    // mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(0), &mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
    // mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(1), &mcu_timingtask_read_cache[j], MCU_TIMINGTASK_T_SIZE);

    while (i < mcu_timingtask_num - 1)
    {
        if (mcu_timingtask_read_cache[i].timingtask_hour > mcu_timingtask_read_cache[j].timingtask_hour)
        {
            k++;
        }
        else if (mcu_timingtask_read_cache[i].timingtask_hour == mcu_timingtask_read_cache[j].timingtask_hour)
        {
            if (mcu_timingtask_read_cache[i].timingtask_min > mcu_timingtask_read_cache[j].timingtask_min)
            {
                k++;
            }
            else if (mcu_timingtask_read_cache[i].timingtask_min == mcu_timingtask_read_cache[j].timingtask_min)
            {
                if (mcu_timingtask_read_cache[i].timingtask_sec > mcu_timingtask_read_cache[j].timingtask_sec)
                {
                    k++;
                }
            }
        }

        j++;
        if (j == mcu_timingtask_num)
        {
            while (mcu_timingtask_excute_sort[k] != 0)
            {
                k++;
            }
            mcu_timingtask_excute_sort[k] = i;
            i++;
            j = i + 1;
            k = 0;
            // if (i < mcu_timingtask_num - 1)
            // {
            //     mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
            //     mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(j), &mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
            // }
        }
        sys_delay_ms(10);
    }
    return 0;
}

/**
 * @brief Set the timing task execution index.after sorting or revise timingtask,and set alarm,call this function.
 * @return 1:All timing tasks have been executed.
 *        0:There are still timing tasks to be executed.
 */
uint8_t mcu_timingtask_excute_index_set(void)
{
    if(mcu_timingtask_num == 0 || mcu_timingtask_excute_index >= mcu_timingtask_num)
    {
        mcu_timingtask_excute_index = 0;
        return 1;
    }
    uint32_t current_time = get_uts();
    uint8_t date, weekday;
    uint8_t hour, min, sec;
    get_time(&hour, &min, &sec);
    get_date(&date, &weekday);
    uint32_t hms_time = hour * 3600 + min * 60 + sec;
    uint32_t hms_takstime;
    while(1)
    {
        hms_takstime = mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_hour * 3600 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_min * 60 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_sec;
        if(hms_takstime < hms_time)
        {
            mcu_timingtask_excute_index++;
        }
        else if(mcu_timingtask_excute_index >= mcu_timingtask_num)
        {
            mcu_timingtask_excute_index = 0;
            return 1;
        }
        else break;
    }

    while(1)
    {
        //mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(current_time < mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].validity_time || current_time > mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].invalidty_time)
        {//timingtask invalidty
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        else if((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x12) && (MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, WEEKTRASNFORM(weekday)))) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x13) && (MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[0].timingtask_running_cycle_unit, date))))
        {//timingtask running cycle
            return 0;
        }
        else
        {
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        sys_delay_ms(10);
    }
}

/**
 * @brief Timing task execution.
 * @param timingtask The timing task to be executed.
 */
uint8_t mcu_timingtask_add(MCU_TIMINGTASK_T *timingtask)
{
    if(mcu_timingtask_num >= TIMINGTASK_NUM)
    {
        return 1;
    }
    if(mcu_search_timingtask_id(timingtask->timingtask_id) != 0xFF)
    {
        return 2;
    }
    memcpy(&mcu_timingtask_read_cache[mcu_timingtask_num], timingtask, MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), timingtask, MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_num++;
    mcu_timingtask_sorting();
    mcu_timingtask_set_alarm();
    return 0;
}

/**
 * @brief Search the timing task by ID.
 * @param timingtask_id The timing task ID to be searched.
 * @return The index of the timing task.
 */
uint8_t mcu_search_timingtask_id(uint32_t timingtask_id)
{
    for(uint32_t i = 0; i < mcu_timingtask_num; i++)
    {
        //mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[i].timingtask_id == timingtask_id)
        {
            return i;
        }
    }
    return 0xFF;
}

/**
 * @brief Retrun all timingtask id.
 * @param timingtask_id The timing task ID to be returned.
 * @return The number of timing tasks.
 */
uint8_t mcu_return_all_timingtask_id(uint8_t *timingtask_id)
{
    for(uint8_t i = 0; i < mcu_timingtask_num * 4; i+=4)
    {
        //mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i/4), &mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        timingtask_id[i] = mcu_timingtask_read_cache[i/4].timingtask_id >> 24;
        timingtask_id[i + 1] = mcu_timingtask_read_cache[i/4].timingtask_id >> 16;
        timingtask_id[i + 2] = mcu_timingtask_read_cache[i/4].timingtask_id >> 8;
        timingtask_id[i + 3] = mcu_timingtask_read_cache[i/4].timingtask_id;
    }
    return mcu_timingtask_num;
}

/**
 * @brief Delete the timing task.
 * @param timingtask_id The timing task ID to be deleted.
 */
uint8_t mcu_timingtask_delete(uint32_t *timingtask_id, uint8_t delete_count)
{
    if(delete_count > mcu_timingtask_num)
    {
        return 1;
    }
    uint8_t index[delete_count];
    for(uint8_t i = 0; i < delete_count; i++)
    {
        index[i] = mcu_search_timingtask_id(timingtask_id[i]);
    }
    sys_delay_ms(10);
    uint8_t save_index = 0;
    mcu_timingtask_func.timingtask_erase(mcu_timingtask_addr, delete_count * MCU_TIMINGTASK_T_SIZE);
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        for(uint8_t j = 0; j < delete_count; j++)
        {
            if(i == index[j])
                i++;
        }
        if(i == mcu_timingtask_num)
            break;
        mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(save_index), &mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
        save_index++;
    }
    mcu_timingtask_num -= delete_count;
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), &mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
    }
    mcu_timingtask_sorting();
    mcu_timingtask_set_alarm();
    return 0;
}

/**
 * @brief Set the alarm.init or revise timingtask,call this function.
 */
void mcu_timingtask_set_alarm(void)
{
    if(mcu_timingtask_excute_index_set() == 0)
    {
        mcu_timingtask_func.timingtask_set_alarm(mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_hour, mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_min, mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_sec);
    }
    else 
    {
        mcu_timingtask_func.timingtask_set_alarm(0, 0, 3);
        mcu_timingtask_excute_index = 0;
    }
}

/**
 * @brief Delete the invalid timing task.
 */
uint8_t mcu_timingtask_delete_invalid(void)
{
    if(mcu_timingtask_num == 0)
    {
        return 0;
    }
    uint32_t current_time = get_uts();
    uint32_t delete_invalid_id[mcu_timingtask_num];
    uint8_t delete_count = 0;
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        if(current_time > mcu_timingtask_read_cache[i].invalidty_time)
        {
            delete_invalid_id[delete_count] = mcu_timingtask_read_cache[i].timingtask_id;
            delete_count++;
        }
    }
    if(delete_count > 0)
    {
        mcu_timingtask_delete(delete_invalid_id, delete_count);
        return delete_count;
    }
    return 0;
}

/**
 * @brief return alarm buf
*/
uint8_t *mcu_timingtask_alarm_buf(void)
{
    return mcu_timingtask_read_cache[(mcu_timingtask_excute_sort[mcu_timingtask_excute_index])].timingtask_buf;
}

/**
 * @brief return the same timingtask buf
*/
uint8_t *same_timingtask(void)
{
    if(mcu_timingtask_num == 1)
    {
        return NULL;
    }
    uint8_t date, weekday;
    uint32_t timea, timeb ,current_time;
    current_time = get_uts();
    timea = mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_hour * 3600 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_min * 60 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_sec;
    get_date(&date, &weekday);
    while(1)
    {
        if(mcu_timingtask_excute_index + 1 >= mcu_timingtask_num)
        {
            return NULL;
        }
        timeb = mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_hour * 3600 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_min * 60 + mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_sec;
        if((timeb == timea) && (mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].validity_time <= current_time) && (mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].invalidty_time >= current_time))
        {
            mcu_timingtask_excute_index++;
            if((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_running_cycle == 0x12) &&  MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_running_cycle_unit, WEEKTRASNFORM(weekday))) || \
            ((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_running_cycle == 0x13) &&  MCU_RUNNING_CYCLE_UNIT_JUDGE(mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_running_cycle_unit, date)))
            {
                return mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index + 1]].timingtask_buf;
            }
        }
        else
        {
            return NULL;
        }
        sys_delay_ms(10);
}


#endif

