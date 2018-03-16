/*
 * leds.c
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

#include "leds.h"

//variable definitions
const uint8_t WS2812Bits[4] = {0x88, 0x8E, 0xE8, 0xEE};
uint8_t LEDsData[NSDATA];
SPI_HandleTypeDef *hspiLED;


//functions
void ledSetup(SPI_HandleTypeDef *hspi){
	uint8_t i;
	hspiLED = hspi;
	for(i=0; i<NSDATA; ++i) LEDsData[i] = 0;
	taskENTER_CRITICAL();
	HAL_SPI_Transmit(hspiLED, LEDsData, NSDATA, 100);
	taskEXIT_CRITICAL();
}

void ledSet(LED_t *led){
	uint8_t i;
	uint8_t data[NDATA];
	for(i=0; i<NLEDS; ++i){
		data[i*3+OFF_RED] = led[i].red;
		data[i*3+OFF_GREEN] = led[i].green;
		data[i*3+OFF_BLUE] = led[i].blue;
	}
	for(i=0; i<NDATA; ++i){
		uint8_t j;
		uint8_t dat = data[i];
		for(j=0; j<4; ++j){
			LEDsData[PREDATA+i*4+3-j] = WS2812Bits[dat&0x3];
			dat >>= 2;
		}
	}
	// transmission takes roughly 1/4 of a millisecond (266.24us of pure data)
	taskENTER_CRITICAL();
	HAL_SPI_Transmit(hspiLED, LEDsData, NSDATA, 100);
	taskEXIT_CRITICAL();
}

void ledProgress(float progress, LED_t on, LED_t off){
	LED_t leds[NLEDS];
	uint32_t i;
	for (i = 0; i < NLEDS; ++i) {
		float localProgress = progress * 8 - i;
		if (localProgress < 0.0)
			localProgress = 0.0;
		else if (localProgress > 1.0)
			localProgress = 1.0;
		leds[i].red = off.red+(on.red-off.red)*localProgress+0.5;
		leds[i].green = off.green+(on.green-off.green)*localProgress+0.5;
		leds[i].blue = off.blue+(on.blue-off.blue)*localProgress+0.5;
	}
	ledSet(leds);
}
