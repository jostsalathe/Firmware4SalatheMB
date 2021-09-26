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
#define LEDS_OFF_RED 0
#define LEDS_OFF_GREEN 1
#define LEDS_OFF_BLUE 2
extern const uint8_t LEDS_BitTimings[];
#define LEDS_N 8
#define LEDS_PREDATA 0
#define LEDS_POSTDATA 8
#define LEDS_DATA_N LEDS_N*3 //pure LED data length
#define LEDS_SDATA_N LEDS_PREDATA+LEDS_DATA_N*4+LEDS_POSTDATA //serial data length


//type definitions
typedef struct{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}led_t;

//function prototypes
void ledSetup(SPI_HandleTypeDef *hspi);
void ledSet(led_t *led);
void ledProgress(float progress, led_t on, led_t off);


#endif /*_LEDS_H*/
