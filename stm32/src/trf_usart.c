#include "trf_usart.h"
#include "trf_system.h"
#include "stm32h7xx_hal.h"

UART_HandleTypeDef huart3;

#define RECEIVE_BUFFER_SIZE 512
uint8_t receive_buffer[RECEIVE_BUFFER_SIZE];
int receive_head_index, receive_tail_index;

void USART_Init(void) {
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    RCC_PeriphCLKInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    TRF_Assert(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) == HAL_OK);
    __HAL_RCC_USART3_CLK_ENABLE();
    
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    TRF_Assert(HAL_UART_Init(&huart3) == HAL_OK);
    TRF_Assert(HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) == HAL_OK);
    TRF_Assert(HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) == HAL_OK);
    TRF_Assert(HAL_UARTEx_DisableFifoMode(&huart3) == HAL_OK);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    HAL_UART_Receive_IT(&huart3, &receive_buffer[receive_tail_index], 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        receive_tail_index = (receive_tail_index + 1) % RECEIVE_BUFFER_SIZE;
        HAL_UART_Receive_IT(&huart3, &receive_buffer[receive_tail_index], 1);
    }
}

void USART_Transmit(uint8_t *buffer, uint16_t length) {
    // Fire-and-forget transmission.
    (void)HAL_UART_Transmit(&huart3, buffer, length, 10);
}

void USART_Receive(uint8_t *buffer, uint16_t length) {
    for (int i = 0; i < length; i++) {
        if (receive_head_index == receive_tail_index) {
            return; // Ring buffer emptied.
        }
        *buffer = receive_buffer[receive_head_index];
        receive_head_index = (receive_head_index + 1) % RECEIVE_BUFFER_SIZE;
        buffer++;
    }
}

