/**
 * @file stm32_tools.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __STM32_TOOLS_H__
#define __STM32_TOOLS_H__

#include "main.h"

//根据硬件实际情况修改,以下是示例
//端口重命名,假设相关端口已由CubeMX配置
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
}AI_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
}DI_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
}DO_t;

typedef enum
{
    ADC_INPUT = 1,
    DIGITAL_INPUT,
    DIGITAL_OUTPUT,
    //添加其他外设类型
}PeripheralType_t;//0为无效值

void stm32_reboot(void);
void stm32_sleep_now(void);
void stm32_sleep_on_exit(void);
void stm32_sleep_off_exit(void);

void stm32_gpio_deinit(GPIO_TypeDef *port, uint16_t pin);
uint32_t adc_channel_get(GPIO_TypeDef *gpio_port, uint16_t gpio_pin);
const AI_t *stm32_get_ai(uint8_t index);
const DI_t *stm32_get_di(uint8_t index);
const DO_t *stm32_get_do(uint8_t index);

#endif // __STM32_TOOLS_H__
