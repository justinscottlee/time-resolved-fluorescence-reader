#include "trf_adc.h"
#include "stm32h7xx_hal.h"
#include "trf_system.h"

#define TIM_CLK 275000000

ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim15;

#define ADC_BUFFER_SIZE 2048
uint32_t adc_buffer[ADC_BUFFER_SIZE];
uint32_t adc_write_index;

void ADC_Init(void) {
    __HAL_RCC_TIM15_CLK_ENABLE();

    htim15.Instance = TIM15;
    htim15.Init.Prescaler = 1;
    htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim15.Init.Period = 4096;
    htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim15.Init.RepetitionCounter = 0;
    htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    TRF_Assert(HAL_TIM_Base_Init(&htim15) == HAL_OK);

    TIM_ClockConfigTypeDef TIM_ClockConfigStruct = {0};
    TIM_ClockConfigStruct.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    TRF_Assert(HAL_TIM_ConfigClockSource(&htim15, &TIM_ClockConfigStruct) == HAL_OK);

    TIM_MasterConfigTypeDef TIM_MasterConfigStruct = {0};
    TIM_MasterConfigStruct.MasterOutputTrigger = TIM_TRGO_UPDATE;
    TIM_MasterConfigStruct.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    TRF_Assert(HAL_TIMEx_MasterConfigSynchronization(&htim15, &TIM_MasterConfigStruct) == HAL_OK);

    TRF_Assert(HAL_TIM_Base_Start(&htim15) == HAL_OK);

    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    RCC_PeriphCLKInitStruct.PLL2.PLL2M = 1;
    RCC_PeriphCLKInitStruct.PLL2.PLL2N = 24;
    RCC_PeriphCLKInitStruct.PLL2.PLL2P = 2;
    RCC_PeriphCLKInitStruct.PLL2.PLL2Q = 2;
    RCC_PeriphCLKInitStruct.PLL2.PLL2R = 2;
    RCC_PeriphCLKInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    RCC_PeriphCLKInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    RCC_PeriphCLKInitStruct.PLL2.PLL2FRACN = 0;
    RCC_PeriphCLKInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    TRF_Assert(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) == HAL_OK);

    __HAL_RCC_ADC12_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_16B;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T15_TRGO;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode = DISABLE;
    TRF_Assert(HAL_ADC_Init(&hadc1) == HAL_OK);

    ADC_MultiModeTypeDef ADC_MultiModeStruct = {0};
    ADC_MultiModeStruct.Mode = ADC_MODE_INDEPENDENT;
    TRF_Assert(HAL_ADCEx_MultiModeConfigChannel(&hadc1, &ADC_MultiModeStruct) == HAL_OK);

    ADC_ChannelConfTypeDef ADC_ChannelConfStruct = {0};
    ADC_ChannelConfStruct.Channel = ADC_CHANNEL_2;
    ADC_ChannelConfStruct.Rank = ADC_REGULAR_RANK_1;
    ADC_ChannelConfStruct.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    ADC_ChannelConfStruct.SingleDiff = ADC_SINGLE_ENDED;
    ADC_ChannelConfStruct.OffsetNumber = ADC_OFFSET_NONE;
    ADC_ChannelConfStruct.Offset = 0;
    ADC_ChannelConfStruct.OffsetSignedSaturation = DISABLE;
    TRF_Assert(HAL_ADC_ConfigChannel(&hadc1, &ADC_ChannelConfStruct) == HAL_OK);

    HAL_ADC_Start_IT(&hadc1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc == &hadc1) {
        adc_buffer[adc_write_index] = HAL_ADC_GetValue(&hadc1);
        adc_write_index = (adc_write_index + 1) % ADC_BUFFER_SIZE;
    }
}

void ADC_SetSampleRate(uint32_t samples_per_second) {
    uint32_t prescaler = (TIM_CLK >> 16) / samples_per_second;
    uint32_t period = (TIM_CLK / (prescaler + 1) + samples_per_second >> 1) / samples_per_second - 1;
    __HAL_TIM_SET_PRESCALER(&htim15, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim15, period);
    __HAL_TIM_SET_COUNTER(&htim15, 0);
}

uint32_t ADC_Read(int index) {
    return adc_buffer[index];
}