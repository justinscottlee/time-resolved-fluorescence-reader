#include "trf_system.h"
#include "stm32h7xx_hal.h"
#include "trf_stepper.h"
#include "trf_lcd.h"

#include <string.h>
#include <stdio.h>

TIM_HandleTypeDef htim14;
UART_HandleTypeDef huart4;

// Returns if condition is true. Initiates system reset if condition is false.
void TRF_Assert(bool condition) {
    if (condition) {
        return;
    } else {
        HAL_NVIC_SystemReset();
    }
}

void LCD_Print(char *msg) {
    lcd_clear();
    lcd_reset_cursor();
    lcd_setline1();
    char *newline_pos = strchr(msg, '\n');
    if (newline_pos == NULL) {
        char first_line[17];
        sprintf(first_line, "%.*s", 16, msg);
        lcd_send_msg(first_line);
    } else {
        int first_line_length = newline_pos - msg;
        char first_line[17];
        sprintf(first_line, "%.*s", first_line_length, msg);

        char *second_line_start = newline_pos + 1;
        char second_line[17];
        sprintf(second_line, "%.*s", 16, second_line_start);

        lcd_send_msg(first_line);
        lcd_reset_cursor();
        lcd_setline2();
        lcd_send_msg(second_line);
    }
}

void LED_PWM_Init(void) {
    int prescaler = 200;
    int period = 64475;
    int pulse = 16384;

    __HAL_RCC_TIM14_CLK_ENABLE();

    htim14.Instance = TIM14;
    htim14.Init.Prescaler = prescaler;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = period;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    TRF_Assert(HAL_TIM_Base_Init(&htim14) == HAL_OK);
    TRF_Assert(HAL_TIM_PWM_Init(&htim14) == HAL_OK);

    TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.Pulse = pulse;
    TIM_OC_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TRF_Assert(HAL_TIM_PWM_ConfigChannel(&htim14, &TIM_OC_InitStruct, TIM_CHANNEL_1) == HAL_OK);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM14;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    TRF_Assert(HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1) == HAL_OK);
}

void LCD_UART_Init(void) {
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    RCC_PeriphCLKInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    TRF_Assert(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) == HAL_OK);
    __HAL_RCC_UART4_CLK_ENABLE();

    huart4.Instance = UART4;
    huart4.Init.BaudRate = 9600;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    TRF_Assert(HAL_UART_Init(&huart4) == HAL_OK);
    TRF_Assert(HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) == HAL_OK);
    TRF_Assert(HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) == HAL_OK);
    TRF_Assert(HAL_UARTEx_DisableFifoMode(&huart4) == HAL_OK);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// Initializes all the hardware peripherals.
void TRF_Init(void) {
	TRF_Assert(HAL_Init() == HAL_OK);
	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();
	Stepper_Init();	
    LED_PWM_Init();
}

extern TIM_HandleTypeDef htim16;
// Timer callback to tick the stepper motor logic for steps/sec control.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim16) {
        Stepper_Tick();
    }
}