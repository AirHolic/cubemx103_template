/**
 * @file mcu_adc_tools.h
 * @brief STM32 MCU ADC 工具库 - 简化版
 *
 * 该库用于ADC操作和电压值计算，CubeMX负责基础初始化
 */

#ifndef __MCU_ADC_TOOLS_H__
#define __MCU_ADC_TOOLS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "main.h"

#define ADC_TOOL_DEBUG

    /**
     * @brief 定义 HAS_VREF_PIN 以使用外部 VREF 引脚
     * 注释掉则使用内部参考电压
     */
    // #define HAS_VREF_PIN

    /**
     * @brief ADC工作模式枚举
     */
    typedef enum
    {
        MCU_ADC_MODE_SINGLE,     /* 单次转换模式 */
        MCU_ADC_MODE_CONTINUOUS, /* 连续转换模式 */
        MCU_ADC_MODE_SCAN        /* 扫描模式 */
    } MCU_ADC_Mode_t;

    /**
     * @brief ADC 通道配置结构体
     */
    typedef struct
    {
        ADC_HandleTypeDef *hadc; /* ADC 句柄 */
        uint32_t channel;        /* ADC 通道 */
        uint32_t rank;           /* 序列器中的排序 */
        uint32_t samplingTime;   /* 通道采样时间 */
#ifdef HAS_VREF_PIN
        GPIO_TypeDef *vrefPort; /* VREF 引脚的 GPIO 端口 */
        uint16_t vrefPin;       /* VREF 的 GPIO 引脚 */
        float vrefVoltage;      /* VREF 电压值，单位：伏特 */
#endif
        uint8_t useVrefint;  /* 是否使用内部参考电压校准 */
        uint16_t vrefintCal; /* 内部参考电压校准值 */
        float vrefintActual; /* 实际测量的内部参考电压值 */

        /* 新增状态字段 */
        uint8_t isRunning;          /* ADC是否正在运行 */
        MCU_ADC_Mode_t currentMode; /* 当前工作模式 */
        uint8_t useDMA;             /* 是否使用DMA */
    } MCU_ADC_Channel_t;

    /**
     * @brief 初始化ADC通道配置
     *
     * @param channel 通道配置结构体指针
     * @param hadc ADC句柄
     * @param adcChannel ADC通道号
     * @return HAL_StatusTypeDef 操作状态
     */
    HAL_StatusTypeDef MCU_ADC_Init(MCU_ADC_Channel_t *channel, ADC_HandleTypeDef *hadc, uint32_t adcChannel);

    /**
     * @brief 读取ADC通道值并转换为电压
     *
     * @param channel 通道配置结构体指针
     * @return float 电压值(V)
     */
    float MCU_ADC_ReadVoltage(MCU_ADC_Channel_t *channel);

    /**
     * @brief 读取ADC通道原始值
     *
     * @param channel 通道配置结构体指针
     * @return uint16_t 原始ADC值
     */
    uint16_t MCU_ADC_ReadRaw(MCU_ADC_Channel_t *channel);

    /**
     * @brief 使用内部参考电压校准并读取电压
     *
     * @param channel 通道配置结构体指针
     * @return float 电压值(V)
     */
    float MCU_ADC_ReadCalibrated(MCU_ADC_Channel_t *channel);

    /**
     * @brief 开始ADC连续转换模式
     *
     * @param channel 通道配置结构体指针
     * @param buffer 数据缓冲区
     * @param size 缓冲区大小
     * @return HAL_StatusTypeDef 操作状态
     * @note 需提前初始化DMA
     */
    HAL_StatusTypeDef MCU_ADC_StartContinuous(MCU_ADC_Channel_t *channel, uint16_t *buffer, uint16_t size);

    /**
     * @brief 停止ADC连续转换模式
     *
     * @param channel 通道配置结构体指针
     * @return HAL_StatusTypeDef 操作状态
     * @note 仅支持使用DMA模式
     */
    HAL_StatusTypeDef MCU_ADC_StopContinuous(MCU_ADC_Channel_t *channel);

    /**
     * @brief 获取平均ADC值(多次采样)
     *
     * @param channel 通道配置结构体指针
     * @param samples 采样次数
     * @return float 平均电压值(V)
     */
    float MCU_ADC_GetAverageVoltage(MCU_ADC_Channel_t *channel, uint16_t samples);

    /**
     * @brief 配置ADC通道采样时间
     *
     * @param channel 通道配置结构体指针
     * @param samplingTime 采样时间配置
     * @return HAL_StatusTypeDef 操作状态
     */
    HAL_StatusTypeDef MCU_ADC_SetSamplingTime(MCU_ADC_Channel_t *channel, uint32_t samplingTime);

    /**
     * @brief 将原始 ADC 值转换为电压值
     *
     * @param channel 通道配置结构体指针
     * @param rawValue 待转换的原始 ADC 值
     * @return float 电压读数（单位：伏特）
     */
    float MCU_ADC_ConvertToVoltage(const MCU_ADC_Channel_t *channel, uint16_t rawValue);

    /**
     * @brief 初始化 Vrefint 校准参数
     *
     * @param channel 通道配置结构体指针
     * @return HAL_StatusTypeDef HAL 状态
     */
    HAL_StatusTypeDef MCU_ADC_InitVrefintCal(MCU_ADC_Channel_t *channel);

    /**
     * @brief 获取ADC分辨率
     *
     * @param channel 通道配置结构体指针
     * @return uint8_t ADC分辨率(位数)
     */
    uint8_t MCU_ADC_GetResolution(const MCU_ADC_Channel_t *channel);

    /**
     * @brief 执行ADC内部校准
     *
     * @param channel 通道配置结构体指针
     * @return HAL_StatusTypeDef 操作状态
     */
    HAL_StatusTypeDef MCU_ADC_Calibrate(MCU_ADC_Channel_t *channel);

    /**
     * @brief 启动ADC扫描模式采集多个通道
     *
     * @param channelGroup 通道组配置结构体数组指针
     * @param numChannels 通道数量
     * @param buffer 数据缓冲区
     * @param size 每个通道的缓冲区大小
     * @param continuous 是否连续转换 (0: 单次转换, 1: 连续转换)
     * @return HAL_StatusTypeDef 操作状态
     * @note 仅支持使用相同ADC的通道组
     * @note 仅支持使用DMA模式
     */
    HAL_StatusTypeDef MCU_ADC_StartScan(MCU_ADC_Channel_t *channelGroup, uint8_t numChannels,
                                        uint16_t *buffer, uint16_t size, uint8_t continuous);

    /**
     * @brief 获取连续采样缓冲区中的最新数据平均值
     *
     * @param buffer ADC数据缓冲区
     * @param size 缓冲区大小
     * @param channelIndex 在扫描模式下的通道索引，单通道时为0
     * @param channelCount 总通道数，单通道时为1
     * @return uint16_t 平均ADC值
     */
    uint16_t MCU_ADC_GetBufferAverage(uint16_t *buffer, uint16_t size, uint8_t channelIndex, uint8_t channelCount);

    /**
     * @brief 将电压值转换为电流值 (针对分流电阻)
     *
     * @param voltage 测量到的电压值(V)
     * @return float 计算出的电流值(mA)
     */
    float MCU_ADC_VoltageToCurrentShunt(float resistance, float voltage);

    /**
     * @brief 将adc值转换为电流值 (针对分流电阻)
     *
     * @param adcValue ADC值
     * @param resistance 分流电阻值(欧姆)
     * @return float 计算出的电流值(mA)
     * @todo 待完善
     */
    float MCU_ADC_AdcToCurrentShunt(MCU_ADC_Channel_t *channel, uint16_t adcValue, float resistance);

    /**
     * @brief 将电压值转换为电流值 (针对并联160欧姆电流环)
     *
     * @param voltage 测量到的电压值(V)
     * @return float 计算出的电流值(mA)
     */
    float MCU_ADC_VoltageToCurrentShunt160(float voltage);

    /**
     * @brief 将adc值转换为电流值 (针对160欧姆电流环)
     *
     * @param adcValue ADC值
     * @return float 计算出的电流值(mA)
     */
    float MCU_ADC_AdcToCurrentShunt160(uint16_t adcValue);

    /**
     * @brief 将float数转换为字符串,用于避免sprintf出现问题
     *
     * @param float_value
     * @param char_value
     * @return void
     */
    // void MCU_ADC_FLOAT_TO_CHAR(const float float_value, char *char_value);

#ifdef __cplusplus
}
#endif

#endif /* __MCU_ADC_TOOLS_H__ */
