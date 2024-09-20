#ifndef __JUMP_INTO_APP_H__
#define __JUMP_INTO_APP_H__

#include "main.h"

#define FLASH_JUMP_ADDR FLASH_APP_ADDR

void jump_to_app(uint32_t app_addr);
void main_check_and_start(void);

#endif /* _JUMP_INTO_APP_H__ */
