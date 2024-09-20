#ifndef __MCU_FLASH_H__
#define __MCU_FLASH_H__

#include "main.h"

#define FLASH_START_ADDR 0x08000000
#define FLASH_TYPE FLASH_PAGE
#define FLASH_TOTAL_NUM 127
#define LARGE_FLASH

#if (FLASH_TYPE == FLASH_PAGE)

#ifdef MIDDLE_FLASH
#define FLASH_SIZE 1024
#endif

#ifdef LARGE_FLASH
#define FLASH_SIZE 2048
//#define FLASH_PAGE_SIZE 0x800U
#endif

#define FLASH_PAGEx(x) (FLASH_START_ADDR+FLASH_SIZE*(x))//x < FLASH_TOTAL_NUM

#endif

void mcu_flash_erase(uint32_t addr, uint32_t len);
void mcu_flash_write(uint32_t addr, uint32_t *data, uint32_t size);
void mcu_flash_nocheck_write(uint32_t addr, uint32_t *data, uint32_t size);
void mcu_uint8_flash_write(uint32_t addr, uint8_t *data, uint32_t size);
void mcu_flash_read(uint32_t addr, uint32_t *data, uint32_t size);

#endif /* _MCU_FLASH_H__ */
