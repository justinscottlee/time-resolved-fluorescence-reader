#ifndef TRF_USART_H_
#define TRF_USART_H_

#include "stdint.h"

void USART_Init(void);

void USART_Transmit(uint8_t *buffer, uint16_t length);
int USART_Receive(uint8_t *buffer, uint16_t length);

#endif