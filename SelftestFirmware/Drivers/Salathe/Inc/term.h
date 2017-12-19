#ifndef _term_H
#define _term_H

#include "stm32f7xx_hal.h"
#include "usart.h"

//global variables and defines

//type definitions

//function prototypes
void termSetup(UART_HandleTypeDef *termHandle);
void termPutChar(char c);
void termPutString(char *s);
void termPutInt(uint32_t num, uint8_t digits);
void termPutHex(uint32_t num, uint8_t digits);

#endif
