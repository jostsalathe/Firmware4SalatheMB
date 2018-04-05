/*
 * but.h
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

#ifndef _BUT_H
#define _BUT_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

//global variables and defines
#define BUTTONN 10
#define BUTTON0 0
#define BUTTON1 1
#define BUTTON2 2
#define BUTTON3 3
#define BUTTON4 4
#define BUTTON5 5
#define BUTTON6 6
#define BUTTON7 7
#define BUTTONENC0 8
#define BUTTONENC1 9

//type definitions

//function prototypes
void buttonPoll();
void buttonSetup();
uint8_t buttonRising(uint8_t buttonNumber);
uint8_t buttonFalling(uint8_t buttonNumber);
uint8_t buttonRead(uint8_t buttonNumber);

#endif /*_BUT_H*/
