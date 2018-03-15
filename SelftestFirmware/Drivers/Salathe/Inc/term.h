/*
 * term.h
 *
 *  Created on: 15.03.2018
 *      Author: Jost Salathe <jostsalathe@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
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
