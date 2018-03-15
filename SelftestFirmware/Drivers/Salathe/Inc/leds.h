/*
 * leds.h
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

#ifndef _LEDS_H
#define _LEDS_H

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

//global variables and defines
#define OFF_RED 0
#define OFF_GREEN 1
#define OFF_BLUE 2
extern const uint8_t WS2812Bits[];
#define NLEDS 8
#define PREDATA 0
#define POSTDATA 8
#define NDATA NLEDS*3 //pure LED data length
#define NSDATA PREDATA+NDATA*4+POSTDATA //serial data length


//type definitions
typedef struct{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}LED_t;

//function prototypes
void ledSetup(SPI_HandleTypeDef *hspi);
void ledSet(LED_t *led);
void ledProgress(float progress, LED_t on, LED_t off);


#endif /*_LEDS_H*/
