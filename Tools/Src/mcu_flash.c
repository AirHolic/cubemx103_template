#include "mcu_flash.h"
#include "main.h"

/**
 * @brief Write data to flash.
 * @param addr The address of the flash.
 * @param data The data to be written.
 * @param len The length of the data.
 */
void mcu_flash_write(uint32_t addr, uint32_t *data, uint32_t len)
{
    uint32_t *PageError;
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        return;
    }
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = 1;
    if(HAL_FLASHEx_Erase(&EraseInitStruct, PageError) != HAL_OK)
    {
        printf("Erase failed: %#X\r\n", *PageError);
        HAL_FLASH_Lock();
        return;
    }

    for(uint32_t i = 0; i < len; i++)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i * 4, data[i]) != HAL_OK)
        {
            printf("Write failed: %#X\r\n", addr + i * 4);
            HAL_FLASH_Lock();
            return;
        }
    }
    HAL_FLASH_Lock();
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
