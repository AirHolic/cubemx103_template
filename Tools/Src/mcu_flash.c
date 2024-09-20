//#include "main.h"
#include "mcu_flash.h"

#define FLASH_FLAG_ALL_ERRORS FLASH_FLAG_BSY | FLASH_FLAG_EOP | \
    FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR 

static uint32_t data32[256];

/**
 * @brief Erase flash.
 * @param addr The address of the flash.
 * @param len The length of the flash.
 */
void mcu_flash_erase(uint32_t addr, uint32_t len)
{
    #if RTOS == 1
    taskENTER_CRITICAL();
    #endif
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = len / FLASH_SIZE + 1;
    //__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        return;
    }
    //if(HAL_FLASHEx_Erase_IT(&EraseInitStruct) != HAL_OK)
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return;
    }
    HAL_FLASH_Lock();
    #if RTOS == 1
    taskEXIT_CRITICAL();
    #endif
}

/**
 * @brief Write data to flash.
 * @param addr The address of the flash.
 * @param data The data to be written.
 * @param len The length of the data.Less than or equal to FLASH_SIZE.
 */
void mcu_flash_write(uint32_t addr, uint32_t *data, uint32_t len)
{
    mcu_flash_erase(addr, len);
    #if RTOS == 1
    taskENTER_CRITICAL();
    #endif
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        return;
    }
    for(uint32_t i = 0; i < len; i++)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i * 4, data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return;
        }
    }
    HAL_FLASH_Lock();
    #if RTOS == 1
    taskEXIT_CRITICAL();
    #endif
}

/**
 * @brief Write data to flash without checking.
 * @param addr The address of the flash.
 * @param data The data to be written.
 * @param len The length of the data.Less than or equal to FLASH_SIZE.
 */
void mcu_flash_nocheck_write(uint32_t addr, uint32_t *data, uint32_t len)
{
    #if RTOS == 1
    taskENTER_CRITICAL();
    #endif
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        return;
    }
    for(uint32_t i = 0; i < len; i++)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i * 4, data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return;
        }
    }
    HAL_FLASH_Lock();
    #if RTOS == 1
    taskEXIT_CRITICAL();
    #endif
}

/**
 * @brief Write uint8_t data to flash.
*/
void mcu_uint8_flash_write(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t len = size / 4 + size % 4;
    
    for(uint32_t i = 0; i < len; i++)
    {
        data32[i] = data[i * 4] | data[i * 4 + 1] << 8 | data[i * 4 + 2] << 16 | data[i * 4 + 3] << 24;
    }
    mcu_flash_nocheck_write(addr, data32, len);
}

/**
 * @brief Read data from flash.
 * @param addr The address of the flash.
 * @param data The data to be read.
 * @param len The length of the data.
*/
void mcu_flash_read(uint32_t addr, uint32_t *data, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        data[i] = *(uint32_t *)(addr + i * 4);
    }
}
