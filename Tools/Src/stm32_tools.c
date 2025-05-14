#include "stm32_tools.h"

static const AI_t AI[8] = {
    {AI0_GPIO_Port, AI0_Pin},
    {AI1_GPIO_Port, AI1_Pin},
    {AI2_GPIO_Port, AI2_Pin},
    {AI3_GPIO_Port, AI3_Pin},
    {AI4_GPIO_Port, AI4_Pin},
    {AI5_GPIO_Port, AI5_Pin},
    {AI6_GPIO_Port, AI6_Pin},
    {AI7_GPIO_Port, AI7_Pin},
};

static const DI_t DI[8] = {
    {DI0_GPIO_Port, DI0_Pin},
    {DI1_GPIO_Port, DI1_Pin},
    {DI2_GPIO_Port, DI2_Pin},
    {DI3_GPIO_Port, DI3_Pin},
    {DI4_GPIO_Port, DI4_Pin},
    {DI5_GPIO_Port, DI5_Pin},
    {DI6_GPIO_Port, DI6_Pin},
    {DI7_GPIO_Port, DI7_Pin},
};

static const DO_t DO[4] = {
    {DO0_GPIO_Port, DO0_Pin},
    {DO1_GPIO_Port, DO1_Pin},
    {DO2_GPIO_Port, DO2_Pin},
    {DO3_GPIO_Port, DO3_Pin},
};

/* power_start */

void stm32_reboot(void)
{
    __ASM volatile("cpsid i"); /* 关闭所有中断 ENABLE_INT*/
    HAL_NVIC_SystemReset();    /* 重启 */
}

void stm32_sleep_now(void)
{
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void stm32_sleep_on_exit(void)
{
    HAL_PWR_EnableSleepOnExit();
}

void stm32_sleep_off_exit(void)
{
    HAL_PWR_DisableSleepOnExit();
}

/* power_end */

/* io_manager_start */

void stm32_gpio_deinit(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

uint32_t adc_channel_get(GPIO_TypeDef *gpio_port, uint16_t gpio_pin)
{
    // For GPIOA pins
    if (gpio_port == GPIOA) {
        switch(gpio_pin) {
            case GPIO_PIN_0: return ADC_CHANNEL_0;
            case GPIO_PIN_1: return ADC_CHANNEL_1;
            case GPIO_PIN_2: return ADC_CHANNEL_2;
            case GPIO_PIN_3: return ADC_CHANNEL_3;
            case GPIO_PIN_4: return ADC_CHANNEL_4;
            case GPIO_PIN_5: return ADC_CHANNEL_5;
            case GPIO_PIN_6: return ADC_CHANNEL_6;
            case GPIO_PIN_7: return ADC_CHANNEL_7;
            default: return 0;
        }
    }
    // For GPIOB pins
    else if (gpio_port == GPIOB) {
        switch(gpio_pin) {
            case GPIO_PIN_0: return ADC_CHANNEL_8;
            case GPIO_PIN_1: return ADC_CHANNEL_9;
            // Add more GPIOB channels as needed
            default: return 0;
        }
    }
    // For GPIOC pins
    else if (gpio_port == GPIOC) {
        switch(gpio_pin) {
            case GPIO_PIN_0: return ADC_CHANNEL_10;
            case GPIO_PIN_1: return ADC_CHANNEL_11;
            case GPIO_PIN_2: return ADC_CHANNEL_12;
            case GPIO_PIN_3: return ADC_CHANNEL_13;
            case GPIO_PIN_4: return ADC_CHANNEL_14;
            case GPIO_PIN_5: return ADC_CHANNEL_15;
            // Add more GPIOC channels as needed
            default: return 0;
        }
    }
    
    return 0; // Default return if port/pin combination is not valid for ADC
}

const AI_t *stm32_get_ai(uint8_t index)
{
    if (index >= 8)
    {
        return NULL;
    }
    return &AI[index];
}

const DI_t *stm32_get_di(uint8_t index)
{
    if (index >= 8)
    {
        return NULL;
    }
    return &DI[index];
}

const DO_t *stm32_get_do(uint8_t index)
{
    if (index >= 4)
    {
        return NULL;
    }
    return &DO[index];
}

/* io_manager_end */
