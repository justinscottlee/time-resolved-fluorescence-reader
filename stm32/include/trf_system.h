#ifndef TRF_SYSTEM_H_
#define TRF_SYSTEM_H_

#include <stdbool.h>

void TRF_Assert(bool condition);
void TRF_Init(void);
void LCD_Print(char *msg);

#endif