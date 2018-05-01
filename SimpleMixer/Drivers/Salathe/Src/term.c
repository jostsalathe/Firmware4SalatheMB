/*
 * term.c
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

#include "term.h"
#include "font.h"

//variable definitions
UART_HandleTypeDef *hterm;

//functions
void termSetup(UART_HandleTypeDef *termHandle) {
	hterm = termHandle;
}

void termPutChar(char c) {
	HAL_UART_Transmit(hterm, (uint8_t *)&c, 1, 1000);
}

void termPutString(char *s) {
	HAL_UART_Transmit(hterm, (uint8_t *)s, strlen(s), 1000);
}
