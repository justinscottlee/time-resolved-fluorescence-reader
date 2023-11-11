#include "trf_adc.h"
#include "stm32h7xx_hal.h"
#include "trf_system.h"
#include <malloc.h>

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

uint32_t *sample_buffer;

void ADC_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_16B;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode = ENABLE;
    hadc1.Init.Oversampling.Ratio = 256;
    hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
    hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
    hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_RESUMED_MODE;
    TRF_Assert(HAL_ADC_Init(&hadc1) == HAL_OK);

    ADC_MultiModeTypeDef ADC_MultiModeStruct = {0};
    ADC_MultiModeStruct.Mode = ADC_MODE_INDEPENDENT;
    TRF_Assert(HAL_ADCEx_MultiModeConfigChannel(&hadc1, &ADC_MultiModeStruct) == HAL_OK);

    ADC_ChannelConfTypeDef ADC_ChannelConfStruct = {0};
    ADC_ChannelConfStruct.Channel = ADC_CHANNEL_2;
    ADC_ChannelConfStruct.Rank = ADC_REGULAR_RANK_1;
    ADC_ChannelConfStruct.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    ADC_ChannelConfStruct.SingleDiff = ADC_DIFFERENTIAL_ENDED;
    ADC_ChannelConfStruct.OffsetNumber = ADC_OFFSET_NONE;
    ADC_ChannelConfStruct.Offset = 0;
    ADC_ChannelConfStruct.OffsetSignedSaturation = DISABLE;
    TRF_Assert(HAL_ADC_ConfigChannel(&hadc1, &ADC_ChannelConfStruct) == HAL_OK);

    sample_buffer = malloc(sizeof(uint32_t) * 1024);
}

void ADC_Start(void) {
    HAL_ADC_Start_DMA(&hadc1, sample_buffer, 1024);
}

void ADC_Stop(void) {
    HAL_ADC_Stop_DMA(&hadc1);
}