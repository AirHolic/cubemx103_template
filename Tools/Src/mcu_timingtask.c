#include "main.h"
#include "rtc_utx.h"
#include "rtc.h"
#include "sys_delay.h"
#include "mcu_flash.h"
#include "mcu_timingtask.h"

mcu_timingtask_func_t mcu_timingtask_func;

uint32_t mcu_timingtask_addr = TIMINGTASK_ADDR;//Timing task storage address
uint8_t mcu_timingtask_num = 0;//Number of timing tasks

uint8_t mcu_timingtask_excute_sort[TIMINGTASK_NUM];//Timing task execution order
uint8_t mcu_timingtask_excute_index = 0;//Timing task execution index
//example MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index])

#define MCU_TIMINGTASK_T_SIZE sizeof(MCU_TIMINGTASK_T)
#define MCU_TIMINGTASK_T_INDEX(x) (mcu_timingtask_addr + x * MCU_TIMINGTASK_T_SIZE)//request address of timing task

static uint8_t set_alarm(uint8_t hour, uint8_t min, uint8_t sec)
{
    return time_set_alarm(hour, min, sec);
}

#if TIMINGTASK_STORAGE_IN_FLASH == 1
MCU_TIMINGTASK_T mcu_timingtask_read_cache[2];

/**
 * @brief Initialize the timing task.
 */
void mcu_timingtask_init(void)
{
    mcu_timingtask_func.timingtask_load = mcu_flash_read;
    mcu_timingtask_func.timingtask_save = mcu_flash_nocheck_write;//directly write to flash without checking
    mcu_timingtask_func.timingtask_erase = mcu_flash_erase;
    mcu_timingtask_func.timingtask_set_alarm = set_alarm;
    //Load the timing task from the storage address.
    mcu_timingtask_func.timingtask_load(mcu_timingtask_addr, (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    if(mcu_timingtask_read_cache[0].timingtask_id == 0xFFFFFFFF)
    {
        mcu_timingtask_addr = TIMINGTASK_BACKUP_ADDR;//Timing task storage address
    }
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
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
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(0), (uint32_t *)&mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(1), (uint32_t *)&mcu_timingtask_read_cache[j], MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_excute_index = 0;

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
            mcu_timingtask_excute_sort[i] = k;
            i++;
            j = i + 1;
            k = 0;
            if (i < mcu_timingtask_num - 1)
            {
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(j), (uint32_t *)&mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
            }
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
    uint32_t current_time = get_uts();
    uint8_t date, weekday;
    get_date(&date, &weekday);
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(current_time < mcu_timingtask_read_cache[0].validity_time || current_time > mcu_timingtask_read_cache[0].invalidty_time)
        {//timingtask invalidty
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        else if((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x12) && (mcu_timingtask_read_cache[0].timingtask_running_cycle_unit == weekday)) || \
            ((mcu_timingtask_read_cache[0].timingtask_running_cycle == 0x13) && (mcu_timingtask_read_cache[0].timingtask_running_cycle_unit == date)))
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
    mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), (uint32_t *)timingtask, MCU_TIMINGTASK_T_SIZE);
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
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[0].timingtask_id == timingtask_id)
        {
            return i;
        }
    }
    return 0xFF;
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

    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        for(uint8_t j = 0; j < delete_count; j++)
        {
            if(i == index[j])
                i++;
        }
        switch (mcu_timingtask_addr)
        {
            case TIMINGTASK_ADDR:
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                mcu_timingtask_func.timingtask_save(TIMINGTASK_BACKUP_ADDR + save_index * MCU_TIMINGTASK_T_SIZE, (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                save_index++;
                break;
            case TIMINGTASK_BACKUP_ADDR:
                mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                mcu_timingtask_func.timingtask_save(TIMINGTASK_ADDR + save_index * MCU_TIMINGTASK_T_SIZE, (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
                save_index++;
                break;
            default:
                break;
        }
    }
    mcu_timingtask_num -= delete_count;
    mcu_timingtask_func.timingtask_erase(mcu_timingtask_addr, delete_count * MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_addr = (mcu_timingtask_addr == TIMINGTASK_ADDR) ? TIMINGTASK_BACKUP_ADDR : TIMINGTASK_ADDR;
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
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        mcu_timingtask_func.timingtask_set_alarm(mcu_timingtask_read_cache[0].timingtask_hour, mcu_timingtask_read_cache[0].timingtask_min, mcu_timingtask_read_cache[0].timingtask_sec);
    }
    else 
    {
        mcu_timingtask_func.timingtask_set_alarm(0, 0, 3);
    }
}

/**
 * @brief return alarm buf
*/
uint8_t *mcu_timingtask_alarm_buf(void)
{
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    return mcu_timingtask_read_cache[0].timingtask_buf;
}

uint16_t mcu_timingtask_alarm_buf_len(void)
{
    mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
    //return mcu_timingtask_read_cache[0].timingtask_buf_len;
    return sizeof(mcu_timingtask_read_cache[0].timingtask_buf);
}

#else
MCU_TIMINGTASK_T mcu_timingtask_read_cache[TIMINGTASK_NUM];
void mcu_timingtask_init(void)
{
    mcu_timingtask_func.timingtask_load = mcu_flash_read;
    mcu_timingtask_func.timingtask_save = mcu_flash_nocheck_write;//directly write to flash without checking
    mcu_timingtask_func.timingtask_erase = mcu_flash_erase;
    mcu_timingtask_func.timingtask_set_alarm = set_alarm;
    //Load the timing task from the storage address.
    while(1)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
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
    // mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(0), (uint32_t *)&mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
    // mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(1), (uint32_t *)&mcu_timingtask_read_cache[j], MCU_TIMINGTASK_T_SIZE);

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
            mcu_timingtask_excute_sort[i] = k;
            i++;
            j = i + 1;
            k = 0;
            // if (i < mcu_timingtask_num - 1)
            // {
            //     mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
            //     mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(j), (uint32_t *)&mcu_timingtask_read_cache[1], MCU_TIMINGTASK_T_SIZE);
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
    uint32_t current_time = get_uts();
    uint8_t date, weekday;
    get_date(&date, &weekday);
    while(1)
    {
        //mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_excute_sort[mcu_timingtask_excute_index]), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(current_time < mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].validity_time || current_time > mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].invalidty_time)
        {//timingtask invalidty
            mcu_timingtask_excute_index++;
            if(mcu_timingtask_excute_index == mcu_timingtask_num)
            {
                mcu_timingtask_excute_index = 0;
                return 1;
            }
        }
        else if((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_running_cycle == 0x11) || \
            ((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_running_cycle == 0x12) && (mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_running_cycle_unit == weekday)) || \
            ((mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_running_cycle == 0x13) && (mcu_timingtask_read_cache[mcu_timingtask_excute_sort[mcu_timingtask_excute_index]].timingtask_running_cycle_unit == date)))
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
    memcpy(&mcu_timingtask_read_cache[mcu_timingtask_num], timingtask, MCU_TIMINGTASK_T_SIZE);
    mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(mcu_timingtask_num), (uint32_t *)timingtask, MCU_TIMINGTASK_T_SIZE);
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
        //mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[0], MCU_TIMINGTASK_T_SIZE);
        if(mcu_timingtask_read_cache[i].timingtask_id == timingtask_id)
        {
            return i;
        }
    }
    return 0xFF;
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
        mcu_timingtask_func.timingtask_save(MCU_TIMINGTASK_T_INDEX(save_index), (uint32_t *)&mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
        save_index++;
    }
    mcu_timingtask_num -= delete_count;
    for(uint8_t i = 0; i < mcu_timingtask_num; i++)
    {
        mcu_timingtask_func.timingtask_load(MCU_TIMINGTASK_T_INDEX(i), (uint32_t *)&mcu_timingtask_read_cache[i], MCU_TIMINGTASK_T_SIZE);
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
        mcu_timingtask_func.timingtask_set_alarm(0, 0, 5);
    }
}

/**
 * @brief return alarm buf
*/
uint8_t *mcu_timingtask_alarm_buf(void)
{
    return mcu_timingtask_read_cache[(mcu_timingtask_excute_sort[mcu_timingtask_excute_index])].timingtask_buf;
}

uint16_t mcu_timingtask_alarm_buf_len(void)
{
    return sizeof(mcu_timingtask_read_cache[(mcu_timingtask_excute_sort[mcu_timingtask_excute_index])].timingtask_buf);
}
#endif
