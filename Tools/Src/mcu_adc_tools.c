/**
 * @file mcu_adc_tools.c
 * @brief STM32 MCU ADC 工具库实现 - 简化版
 */

#include "mcu_adc_tools.h"

#ifdef ADC_TOOL_DEBUG
#define ADC_TOOL_LOG(fmt, ...) printf("[ADC] " fmt "\r\n", ##__VA_ARGS__)
#else
#define ADC_TOOL_LOG(fmt, ...)
#endif

/**
 * @brief 获取ADC分辨率
 *
 * @param channel 通道配置结构体指针
 * @return uint8_t ADC分辨率(位数)
 */
uint8_t MCU_ADC_GetResolution(const MCU_ADC_Channel_t *channel)
{
    uint8_t resolution = 12; /* 默认12位分辨率 */

#if defined(STM32F1)
    /* F1系列固定为12位分辨率 */
    resolution = 12;
#elif defined(STM32H7)
    /* H7系列默认16位分辨率 */
    resolution = 16;
#elif defined(STM32F3) || defined(STM32L4) || defined(STM32G4)
    if (channel == NULL || channel->hadc == NULL)
    {
        return resolution;
    }
    /* 可配置ADC分辨率的系列 */
    switch (channel->hadc->Init.Resolution)
    {
    case ADC_RESOLUTION_12B:
        resolution = 12;
        break;
    case ADC_RESOLUTION_10B:
        resolution = 10;
        break;
    case ADC_RESOLUTION_8B:
        resolution = 8;
        break;
    case ADC_RESOLUTION_6B:
        resolution = 6;
        break;
#if defined(ADC_RESOLUTION_14B)
    case ADC_RESOLUTION_14B:
        resolution = 14;
        break;
#endif
#if defined(ADC_RESOLUTION_16B)
    case ADC_RESOLUTION_16B:
        resolution = 16;
        break;
#endif
    default:
        resolution = 12;
        break;
    }
#elif defined(STM32F4) || defined(STM32F7)
    /* F4/F7系列默认12位分辨率 */
    resolution = 12;
#elif defined(STM32L0) || defined(STM32L1)
    /* L0/L1系列默认12位分辨率 */
    resolution = 12;
#endif

    return resolution;
}

/**
 * @brief 初始化ADC通道配置
 *
 * @param channel 通道配置结构体指针
 * @param hadc ADC句柄
 * @param adcChannel ADC通道号
 * @return HAL_StatusTypeDef 操作状态
 */
HAL_StatusTypeDef MCU_ADC_Init(MCU_ADC_Channel_t *channel, ADC_HandleTypeDef *hadc, uint32_t adcChannel)
{
    if (channel == NULL || hadc == NULL)
    {
        return HAL_ERROR;
    }

    /* 设置基本配置 */
    channel->hadc = hadc;
    channel->channel = adcChannel;
    channel->rank = 1; /* 默认为第一个转换 */
#if defined(STM32F1)
    channel->samplingTime = ADC_SAMPLETIME_239CYCLES_5; /* F1系列默认采样时间 */
#elif defined(STM32F4) || defined(STM32F7)
    channel->samplingTime = ADC_SAMPLETIME_480CYCLES_5; /* F4/F7系列默认采样时间 */
#elif defined(STM32L4) || defined(STM32G4)
    channel->samplingTime = ADC_SAMPLETIME_247CYCLES_5; /* L4/G4系列默认采样时间 */
#else
    channel->samplingTime = ADC_SAMPLETIME_239CYCLES_5; /* 通用默认值 */
#endif

#ifdef HAS_VREF_PIN
    channel->vrefPort = NULL;
    channel->vrefPin = 0;
    channel->vrefVoltage = 3.3f; /* 默认参考电压 */
#endif

    /* 默认不使用内部参考电压 */
    channel->useVrefint = 0;
    channel->vrefintCal = 0;
    channel->vrefintActual = 1.2f;

    channel->isRunning = 0;
    channel->currentMode = MCU_ADC_MODE_SINGLE;
    channel->useDMA = 0;

    /* 执行ADC校准以提高精度 */
    MCU_ADC_Calibrate(channel);

    /* 尝试初始化内部参考电压校准 */
    MCU_ADC_InitVrefintCal(channel);

    return HAL_OK;
}

/**
 * @brief 初始化 Vrefint 校准参数
 *
 * @param channel 通道配置结构体指针
 * @return HAL_StatusTypeDef HAL 状态
 */
HAL_StatusTypeDef MCU_ADC_InitVrefintCal(MCU_ADC_Channel_t *channel)
{
    if (channel == NULL)
    {
        return HAL_ERROR;
    }

    /* 启用 Vrefint 校准 */
    channel->useVrefint = 1;

    /* 从不同 STM32 系列的出厂校准值读取 Vrefint 校准值 */
#if defined(STM32F0) || defined(STM32F1) || defined(STM32F3)
#if defined(STM32F0) || defined(STM32F3)
    channel->vrefintCal = *(__IO uint16_t *)(0x1FFFF7BA); /* F0/F3 系列 */
#elif defined(STM32F1)
    channel->vrefintCal = *(__IO uint16_t *)(0x1FFFF7E8); /* F1 系列 */
#endif
    channel->vrefintActual = 1.2f; /* Vrefint 标称值为 1.2V */

#elif defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    /* F2/F4/F7 系列 */
    channel->vrefintCal = *(__IO uint16_t *)(0x1FFF7A2A);
    channel->vrefintActual = 1.21f;

#elif defined(STM32L0) || defined(STM32L1)
/* L0/L1 系列 */
#if defined(STM32L0)
    channel->vrefintCal = *(__IO uint16_t *)(0x1FF80078);
#elif defined(STM32L1)
    channel->vrefintCal = *(__IO uint16_t *)(0x1FF800F8);
#endif
    channel->vrefintActual = 1.224f;

#elif defined(STM32L4) || defined(STM32G4)
    /* L4/G4 系列 */
    channel->vrefintCal = *(__IO uint16_t *)(0x1FFF75AA);
    channel->vrefintActual = 1.212f;

#elif defined(STM32G0)
    /* G0 系列 */
    channel->vrefintCal = *(__IO uint16_t *)(0x1FFF75AA);
    channel->vrefintActual = 1.212f;

#elif defined(STM32H7)
    /* H7 系列 */
    channel->vrefintCal = *(__IO uint16_t *)(0x1FF1E860);
    channel->vrefintActual = 1.21f;

#else
    /* 对于未列出的系列，使用通用值 */
    channel->vrefintCal = 0;
    channel->vrefintActual = 1.2f;
    return HAL_ERROR;
#endif

    return HAL_OK;
}

/**
 * @brief 配置ADC通道采样时间
 *
 * @param channel 通道配置结构体指针
 * @param samplingTime 采样时间配置
 * @return HAL_StatusTypeDef 操作状态
 */
HAL_StatusTypeDef MCU_ADC_SetSamplingTime(MCU_ADC_Channel_t *channel, uint32_t samplingTime)
{
    if (channel == NULL || channel->hadc == NULL)
    {
        return HAL_ERROR;
    }

    channel->samplingTime = samplingTime;

    /* 重新配置通道采样时间(仅当ADC未进行转换时) */
    if (HAL_ADC_GetState(channel->hadc) & HAL_ADC_STATE_READY)
    {
        ADC_ChannelConfTypeDef sConfig = {0};

        sConfig.Channel = channel->channel;
        sConfig.Rank = channel->rank;
        sConfig.SamplingTime = channel->samplingTime;
#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
        sConfig.SingleDiff = ADC_SINGLE_ENDED; /* 单端输入 */
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;
#endif

        if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief 读取ADC通道原始值
 *
 * @param channel 通道配置结构体指针
 * @return uint16_t 原始ADC值
 */
uint16_t MCU_ADC_ReadRaw(MCU_ADC_Channel_t *channel)
{
    if (channel == NULL || channel->hadc == NULL)
    {
        return 0;
    }

    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adcValue = 0;

    /* 配置ADC通道 */
    sConfig.Channel = channel->channel;
    sConfig.Rank = channel->rank;
    sConfig.SamplingTime = channel->samplingTime;
#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
    sConfig.SingleDiff = ADC_SINGLE_ENDED; /* 单端输入 */
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
#endif

    /* 配置ADC通道 */
    if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
    {
        return 0;
    }

    /* 启动ADC转换 */
    if (HAL_ADC_Start(channel->hadc) != HAL_OK)
    {
        return 0;
    }

    /* 等待转换完成 */
    if (HAL_ADC_PollForConversion(channel->hadc, 100) == HAL_OK)
    {
        /* 读取ADC值 */
        adcValue = HAL_ADC_GetValue(channel->hadc);
    }

    /* 停止ADC转换 */
    HAL_ADC_Stop(channel->hadc);

    return adcValue;
}

/**
 * @brief 将原始 ADC 值转换为电压值
 *
 * @param channel 通道配置结构体指针
 * @param rawValue 待转换的原始 ADC 值
 * @return float 电压读数（单位：伏特）
 */
float MCU_ADC_ConvertToVoltage(const MCU_ADC_Channel_t *channel, uint16_t rawValue)
{
    if (channel == NULL || channel->hadc == NULL)
    {
        ADC_TOOL_LOG("ADC通道未初始化或无效\r\n");
    }

    uint8_t resolution = MCU_ADC_GetResolution(channel);
    float maxValue = (float)((1UL << resolution) - 1);
    float voltage = 0.0f;

    /* 根据参考电压计算实际电压值 */
#ifdef HAS_VREF_PIN
    /* 使用外部参考电压 */
    voltage = ((float)rawValue / maxValue) * channel->vrefVoltage;
#else
    /* 使用默认VDDA参考电压 */
#if defined(STM32L0) || defined(STM32L1)
    voltage = ((float)rawValue / maxValue) * 3.0f;
#else
    voltage = ((float)rawValue / maxValue) * 3.3f;
#endif
#endif

    return voltage;
}

/**
 * @brief 读取ADC通道值并转换为电压
 *
 * @param channel 通道配置结构体指针
 * @return float 电压值(V)
 */
float MCU_ADC_ReadVoltage(MCU_ADC_Channel_t *channel)
{
    uint16_t rawValue = MCU_ADC_ReadRaw(channel);
    return MCU_ADC_ConvertToVoltage(channel, rawValue);
}

/**
 * @brief 使用内部参考电压校准并读取电压
 *
 * @param channel 通道配置结构体指针
 * @return float 电压值(V)
 * @note cks不适用//电源稳定时切换公式
 * @todo 当前环境暂不支持继续使用,暂时弃用
 */
float MCU_ADC_ReadCalibrated(MCU_ADC_Channel_t *channel)
{
    if (channel == NULL || channel->hadc == NULL || channel->useVrefint == 0)
    {
        return 0.0f;
    }

    uint16_t adcValue = 0;
    uint16_t vrefintValue = 0;
    ADC_ChannelConfTypeDef sConfig = {0};
    float voltage = 0.0f;

    /* 1. 配置并读取VREFINT通道 */
    sConfig.Rank = 1; // 设置为第一个转换
#if defined(STM32F0) || defined(STM32F3)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#elif defined(STM32F1)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#elif defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#elif defined(STM32L0) || defined(STM32L1) || defined(STM32L4)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#elif defined(STM32G0) || defined(STM32G4)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#elif defined(STM32H7)
    sConfig.Channel = ADC_CHANNEL_VREFINT;
#else
    return MCU_ADC_ReadVoltage(channel);
#endif
    sConfig.SamplingTime = channel->samplingTime;

#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
#endif

    if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
    {
        return 0.0f;
    }

    /* 启动ADC转换并读取VREFINT通道值 */
    if (HAL_ADC_Start(channel->hadc) != HAL_OK)
    {
        return 0.0f;
    }

    if (HAL_ADC_PollForConversion(channel->hadc, 100) == HAL_OK)
    {
        vrefintValue = HAL_ADC_GetValue(channel->hadc);
    }

    HAL_ADC_Stop(channel->hadc);

    /* 2. 配置并读取目标通道 */
    sConfig.Channel = channel->channel;
    sConfig.Rank = channel->rank;
    sConfig.SamplingTime = channel->samplingTime;

    if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
    {
        return 0.0f;
    }

    /* 启动ADC转换并读取目标通道值 */
    if (HAL_ADC_Start(channel->hadc) != HAL_OK)
    {
        return 0.0f;
    }

    if (HAL_ADC_PollForConversion(channel->hadc, 100) == HAL_OK)
    {
        adcValue = HAL_ADC_GetValue(channel->hadc);
    }

    HAL_ADC_Stop(channel->hadc);

    /* 计算校准后的电压值 */
    if (vrefintValue > 0)
    {
        /* 按照公式: Vchx = Vrefint * (ADchx/ADrefint) */
        voltage = channel->vrefintActual * ((float)adcValue / (float)vrefintValue); // ldo使用
        //(ADO_RAW * VREFINT_CAL * VREFINT_NOMINAL) / (VREFINT_RAW X ADC_RESOLUTION) =V_channel//利用出厂校准值//cks系列不适用//电源不稳定时使用
    }
    else
    {
        /* 无法校准时使用默认计算 */
        voltage = MCU_ADC_ConvertToVoltage(channel, adcValue);
    }

    return voltage;
}

/**
 * @brief 开始ADC连续转换模式
 *
 * @param channel 通道配置结构体指针
 * @param buffer 数据缓冲区
 * @param size 缓冲区大小
 * @return HAL_StatusTypeDef 操作状态
 * @note 需提前初始化DMA
 */
HAL_StatusTypeDef MCU_ADC_StartContinuous(MCU_ADC_Channel_t *channel, uint16_t *buffer, uint16_t size)
{
    if (channel == NULL || channel->hadc == NULL || buffer == NULL || size == 0)
    {
        return HAL_ERROR;
    }

    // 检查ADC当前状态，如果已经在运行，先停止
    uint32_t adcState = HAL_ADC_GetState(channel->hadc);
    if (adcState & HAL_ADC_STATE_BUSY_REG || adcState & HAL_ADC_STATE_BUSY_INJ)
    {
        // ADC已在运行，先停止当前操作
        if (channel->useDMA)
        {
            HAL_ADC_Stop_DMA(channel->hadc);
        }
        else
        {
            HAL_ADC_Stop(channel->hadc);
        }

        // 添加短暂延时确保ADC完全停止
        HAL_Delay(1);
    }

    /* 配置ADC通道 */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel->channel;
    sConfig.Rank = channel->rank;
    sConfig.SamplingTime = channel->samplingTime;
#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
#endif

    if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // 设置ADC为连续转换模式
#if defined(STM32F1) || defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    // F系列使用HAL库API设置连续模式
    channel->hadc->Init.ContinuousConvMode = ENABLE;
    if (HAL_ADC_Init(channel->hadc) != HAL_OK)
    {
        return HAL_ERROR;
    }
#elif defined(STM32G0) || defined(STM32G4) || defined(STM32L4) || defined(STM32H7)
    // G/L4/H7系列使用标准HAL库API
    ADC_ChannelConfTypeDef tempConfig = {0};
    CLEAR_BIT(channel->hadc->Instance->CFGR, ADC_CFGR_CONT);
    HAL_Delay(1); // 确保设置生效
    SET_BIT(channel->hadc->Instance->CFGR, ADC_CFGR_CONT);
#elif defined(STM32F0) || defined(STM32F3) || defined(STM32L0) || defined(STM32L1)
    // 其他系列
    CLEAR_BIT(channel->hadc->Instance->CR2, ADC_CR2_CONT);
    HAL_Delay(1); // 确保设置生效
    SET_BIT(channel->hadc->Instance->CR2, ADC_CR2_CONT);
#endif

    /* 启动连续ADC转换（DMA模式） */
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(channel->hadc, (uint32_t *)buffer, size);
    if (status == HAL_OK)
    {
        // 更新状态标志
        channel->isRunning = 1;
        channel->currentMode = MCU_ADC_MODE_CONTINUOUS;
        channel->useDMA = 1;
    }

    return status;
}

/**
 * @brief 停止ADC连续转换模式
 *
 * @param channel 通道配置结构体指针
 * @return HAL_StatusTypeDef 操作状态
 * @note 仅在使用DMA时有效
 */
HAL_StatusTypeDef MCU_ADC_StopContinuous(MCU_ADC_Channel_t *channel)
{
    if (channel == NULL || channel->hadc == NULL)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status;

    // 检查ADC是否已在运行中
    if (!(HAL_ADC_GetState(channel->hadc) & HAL_ADC_STATE_BUSY))
    {
        // ADC未运行，无需停止
        channel->isRunning = 0;
        return HAL_OK;
    }

    // 根据不同的工作模式选择停止方法
    if (channel->useDMA)
    {
        status = HAL_ADC_Stop_DMA(channel->hadc);
    }
    else
    {
        status = HAL_ADC_Stop(channel->hadc);
    }

    if (status == HAL_OK)
    {
        channel->isRunning = 0;
    }

    // 禁用连续转换模式
#if defined(STM32F1) || defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    channel->hadc->Init.ContinuousConvMode = DISABLE;
    HAL_ADC_Init(channel->hadc);
#elif defined(STM32G0) || defined(STM32G4) || defined(STM32L4) || defined(STM32H7)
    CLEAR_BIT(channel->hadc->Instance->CFGR, ADC_CFGR_CONT);
#elif defined(STM32F0) || defined(STM32F3) || defined(STM32L0) || defined(STM32L1)
    CLEAR_BIT(channel->hadc->Instance->CR2, ADC_CR2_CONT);
#endif

    return status;
}

/**
 * @brief 获取平均ADC值(多次采样)
 *
 * @param channel 通道配置结构体指针
 * @param samples 采样次数
 * @return float 平均电压值(V)
 */
float MCU_ADC_GetAverageVoltage(MCU_ADC_Channel_t *channel, uint16_t samples)
{
    if (channel == NULL || channel->hadc == NULL || samples == 0)
    {
        return 0.0f;
    }

    uint32_t sum = 0;
    uint16_t validSamples = 0;
    ADC_ChannelConfTypeDef sConfig = {0};

    /* 配置ADC通道 */
    sConfig.Channel = channel->channel;
    sConfig.Rank = channel->rank;
    sConfig.SamplingTime = channel->samplingTime;
#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
#endif

    if (HAL_ADC_ConfigChannel(channel->hadc, &sConfig) != HAL_OK)
    {
        return 0.0f;
    }

    /* 多次采样并求和 */
    for (uint16_t i = 0; i < samples; i++)
    {
        /* 启动ADC转换 */
        if (HAL_ADC_Start(channel->hadc) != HAL_OK)
        {
            continue;
        }

        /* 等待转换完成 */
        if (HAL_ADC_PollForConversion(channel->hadc, 10) == HAL_OK)
        {
            /* 读取ADC值并累加 */
            sum += HAL_ADC_GetValue(channel->hadc);
            validSamples++;
        }

        /* 停止ADC转换 */
        HAL_ADC_Stop(channel->hadc);
    }

    /* 如果没有有效样本，返回0 */
    if (validSamples == 0)
    {
        return 0.0f;
    }

    /* 计算平均ADC值 */
    uint16_t avgRawValue = (uint16_t)(sum / validSamples);

    /* 转换为电压值并返回 */
    return MCU_ADC_ConvertToVoltage(channel, avgRawValue);
}

/**
 * @brief 执行ADC内部校准
 *
 * @param channel 通道配置结构体指针
 * @return HAL_StatusTypeDef 操作状态
 */
HAL_StatusTypeDef MCU_ADC_Calibrate(MCU_ADC_Channel_t *channel)
{
    if (channel == NULL || channel->hadc == NULL)
    {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;

    /* 确保ADC处于停止状态 */
    HAL_ADC_Stop(channel->hadc);

#if defined(STM32F1)
    /* F1系列不需要执行校准步骤，直接执行校准 */
    status = HAL_ADCEx_Calibration_Start(channel->hadc);

#elif defined(STM32F3) || defined(STM32L4) || defined(STM32G4)
    /* 这些系列需要先注入校准，然后执行常规校准 */
    status = HAL_ADCEx_Calibration_Start(channel->hadc, ADC_SINGLE_ENDED);

#elif defined(STM32F0) || defined(STM32G0) || defined(STM32L0)
    /* 这些系列直接执行校准 */
    status = HAL_ADCEx_Calibration_Start(channel->hadc);

#elif defined(STM32H7)
    /* H7系列使用线性校准 */
    status = HAL_ADCEx_Calibration_Start(channel->hadc, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

#elif defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    /* F2/F4/F7系列没有内部校准函数，跳过 */
    status = HAL_OK;
#else
/* 其他系列尝试使用通用校准方法 */
#if defined(HAL_ADCEx_Calibration_Start)
    status = HAL_ADCEx_Calibration_Start(channel->hadc);
#else
    status = HAL_OK;
#endif
#endif

    return status;
}

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
 */
HAL_StatusTypeDef MCU_ADC_StartScan(MCU_ADC_Channel_t *channelGroup, uint8_t numChannels,
                                    uint16_t *buffer, uint16_t size, uint8_t continuous)
{
    if (channelGroup == NULL || numChannels == 0 || buffer == NULL || size == 0)
    {
        return HAL_ERROR;
    }

    // 检查输入参数
    if (numChannels > 16)
    {
        return HAL_ERROR; // ADC通道数不应超过16
    }

    // 假设所有通道使用同一个ADC
    ADC_HandleTypeDef *hadc = channelGroup[0].hadc;
    if (hadc == NULL)
    {
        return HAL_ERROR;
    }

    // 检查DMA句柄是否可用
    if (hadc->DMA_Handle == NULL)
    {
        return HAL_ERROR;
    }

    // 检查并停止当前ADC状态
    uint32_t adcState = HAL_ADC_GetState(hadc);
    if (adcState & HAL_ADC_STATE_BUSY_REG || adcState & HAL_ADC_STATE_BUSY_INJ)
    {
        // ADC已在运行，先停止当前操作
        HAL_ADC_Stop_DMA(hadc);
        HAL_Delay(1); // 短暂延时确保ADC完全停止
    }

    // 配置所有通道
    for (uint8_t i = 0; i < numChannels; i++)
    {
        if (channelGroup[i].hadc != hadc)
        {
            return HAL_ERROR; // 确保所有通道使用同一个ADC
        }

        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = channelGroup[i].channel;
        sConfig.Rank = i + 1; // 按顺序分配Rank
        sConfig.SamplingTime = channelGroup[i].samplingTime;

#if defined(STM32F3) || defined(STM32L4) || defined(STM32G4) || defined(STM32H7)
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;
#endif

        if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }

    // 配置ADC为扫描模式，使用HAL库函数尽量避免直接操作寄存器
#if defined(STM32F1) || defined(STM32F2) || defined(STM32F4) || defined(STM32F7)
    // F系列的配置方式
    hadc->Init.ScanConvMode = ENABLE;
    hadc->Init.NbrOfConversion = numChannels;
    hadc->Init.ContinuousConvMode = continuous ? ENABLE : DISABLE;
    if (HAL_ADC_Init(hadc) != HAL_OK)
    {
        return HAL_ERROR;
    }
#elif defined(STM32G0) || defined(STM32G4) || defined(STM32L4) || defined(STM32H7)
    // 使用HAL库API设置扫描模式
    ADC_MultiModeTypeDef multimode = {0};
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(hadc, &multimode) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // 设置扫描和连续模式
    if (continuous)
    {
        SET_BIT(hadc->Instance->CFGR, ADC_CFGR_CONT | ADC_CFGR_SCAN);
    }
    else
    {
        CLEAR_BIT(hadc->Instance->CFGR, ADC_CFGR_CONT);
        SET_BIT(hadc->Instance->CFGR, ADC_CFGR_SCAN);
    }
#elif defined(STM32F0) || defined(STM32F3) || defined(STM32L0) || defined(STM32L1)
    // 使用标准API配置多模式
    if (HAL_ADCEx_MultiModeConfigChannel != NULL)
    {
        ADC_MultiModeTypeDef multimode = {0};
        multimode.Mode = ADC_MODE_INDEPENDENT;
        HAL_ADCEx_MultiModeConfigChannel(hadc, &multimode);
    }

    // 设置扫描和连续模式
    SET_BIT(hadc->Instance->CR1, ADC_CR1_SCAN);
    if (continuous)
    {
        SET_BIT(hadc->Instance->CR2, ADC_CR2_CONT);
    }
    else
    {
        CLEAR_BIT(hadc->Instance->CR2, ADC_CR2_CONT);
    }
#endif

    // 使用HAL库函数启用DMA，避免直接操作DMA寄存器
    // 启用DMA传输
#if defined(STM32F0) || defined(STM32F1) || defined(STM32F2) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7)
    SET_BIT(hadc->Instance->CR2, ADC_CR2_DMA);
#elif defined(STM32G0) || defined(STM32G4) || defined(STM32L4) || defined(STM32H7)
    SET_BIT(hadc->Instance->CFGR, ADC_CFGR_DMAEN);
#elif defined(STM32L0) || defined(STM32L1)
    SET_BIT(hadc->Instance->CR1, ADC_CR1_DMAEN);
#endif

    // 启动ADC扫描模式
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(hadc, (uint32_t *)buffer, size * numChannels);

    // 更新所有通道的状态
    if (status == HAL_OK)
    {
        for (uint8_t i = 0; i < numChannels; i++)
        {
            channelGroup[i].isRunning = 1;
            channelGroup[i].currentMode = MCU_ADC_MODE_SCAN;
            channelGroup[i].useDMA = 1;
        }
    }

    return status;
}

/**
 * @brief 获取连续采样缓冲区中的最新数据平均值
 *
 * @param buffer ADC数据缓冲区
 * @param size 缓冲区大小
 * @param channelIndex 在扫描模式下的通道索引，单通道时为0
 * @param channelCount 总通道数，单通道时为1
 * @return uint16_t 平均ADC值
 */
uint16_t MCU_ADC_GetBufferAverage(uint16_t *buffer, uint16_t size, uint8_t channelIndex, uint8_t channelCount)
{
    if (buffer == NULL || size == 0 || channelCount == 0 || channelIndex >= channelCount)
    {
        return 0;
    }

    uint32_t sum = 0;
    uint16_t count = 0;

    // 计算对应通道的数据平均值
    // 在扫描模式下，数据是按照 CH0,CH1,CH2,CH0,CH1,CH2... 的顺序存储的
    for (uint16_t i = channelIndex; i < size; i += channelCount)
    {
        if (buffer[i] != 0)
        { // 忽略值为0的样本，避免未初始化数据干扰
            sum += buffer[i];
            count++;
        }
    }

    // 防止除以零错误
    if (count == 0)
    {
        return 0;
    }

    return (uint16_t)(sum / count);
}

/**
 * @brief 将电压值转换为电流值 (针对分流电阻)
 *
 * @param resistance 分流电阻值(欧姆)
 * @param voltage 测量到的电压值(V)
 * @return float 计算出的电流值(mA)
 * @note 基于欧姆定律 I = V/R，结果单位为毫安(mA)
 */
float MCU_ADC_VoltageToCurrentShunt(float resistance, float voltage)
{
    if (resistance <= 0.0f)
    {
        return 0.0f; // 防止无效电阻输入
    }

    if (voltage < 0.0f)
    {
        return 0.0f; // 防止负电压输入
    }

    // 使用欧姆定律 I = V/R
    // 1000倍乘数将安培转换为毫安
    return (voltage / resistance) * 1000.0f;
}

/**
 * @brief 将adc值转换为电流值 (针对分流电阻)
 *
 * @param adcValue ADC值
 * @param resistance 分流电阻值(欧姆)
 * @return float 计算出的电流值(mA)
 * @todo 待完善
 */
float MCU_ADC_AdcToCurrentShunt(MCU_ADC_Channel_t *channel, uint16_t adcValue, float resistance)
{
    if (resistance <= 0.0f)
    {
        return 0.0f; // 防止无效电阻输入
    }

    // 计算电压值
    float voltage = MCU_ADC_ConvertToVoltage(channel, adcValue); // 假设使用默认通道

    // 使用欧姆定律 I = V/R
    return MCU_ADC_VoltageToCurrentShunt(resistance, voltage);
}

/**
 * @brief 将电压值转换为电流值 (针对并联160欧姆电流环)
 *
 * @param voltage 测量到的电压值(V)
 * @return float 计算出的电流值(mA)
 * @note 基于欧姆定律 I = V/R，其中R为160欧姆，结果单位为毫安(mA)
 */
float MCU_ADC_VoltageToCurrentShunt160(float voltage)
{
    return MCU_ADC_VoltageToCurrentShunt(160.0f, voltage);
}

/**
 * @brief 将adc值转换为电流值 (针对并联160欧姆电流环)
 *
 * @param adcValue ADC值
 * @return float 计算出的电流值(mA)
 */
float MCU_ADC_AdcToCurrentShunt160(uint16_t adcValue)
{
    return MCU_ADC_AdcToCurrentShunt(NULL, adcValue, 160.0f);
}

// /**
//  * @brief 将float数转换为字符串,用于避免sprintf出现问题
//  * 
//  * @param float_value 
//  * @param char_value
//  * @return void 
//  */
// void MCU_ADC_FLOAT_TO_CHAR(const float float_value, char *char_value)
// {
//     sprintf(char_value, "%d.%0.3d", (uint16_t)float_value, (uint16_t)((float_value-(uint16_t)float_value)*1e03));
// }
