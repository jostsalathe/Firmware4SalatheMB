/*
 * term.h
 *
 *  Created on: 15.03.2018
 *      Author: Jost Salathe
 */

#ifndef _TERM_H
#define _TERM_H

#include "stm32f7xx_hal.h"
#include "usart.h"
#include "font.h"
#include <string.h>

//global variables and defines

//type definitions

//function prototypes
void termSetup(UART_HandleTypeDef *termHandle);
void termPutChar(char c);
void termPutString(char *s);

#endif /*_TERM_H*/
