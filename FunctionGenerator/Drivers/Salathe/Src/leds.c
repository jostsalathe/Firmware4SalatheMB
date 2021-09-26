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
const uint8_t LEDS_BitTimings[4] = {0x88, 0x8E, 0xE8, 0xEE};
uint8_t LEDsData[LEDS_SDATA_N];
SPI_HandleTypeDef *hspiLED;


//functions
void ledSetup(SPI_HandleTypeDef *hspi){
	uint8_t i;
	hspiLED = hspi;
	for(i=0; i<LEDS_SDATA_N; ++i) LEDsData[i] = 0;
	for(i=0; i<LEDS_DATA_N*4; ++i) LEDsData[i+LEDS_PREDATA] = LEDS_BitTimings[0];
	HAL_SPI_Transmit_DMA(hspiLED, LEDsData, LEDS_SDATA_N);
}

void ledSet(led_t *led){
	uint8_t i;
	uint8_t data[LEDS_DATA_N];
	for(i=0; i<LEDS_N; ++i){
		data[i*3+LEDS_OFF_RED] = led[i].red;
		data[i*3+LEDS_OFF_GREEN] = led[i].green;
		data[i*3+LEDS_OFF_BLUE] = led[i].blue;
	}
	for(i=0; i<LEDS_DATA_N; ++i){
		uint8_t j;
		uint8_t dat = data[i];
		for(j=0; j<4; ++j){
			LEDsData[LEDS_PREDATA+i*4+3-j] = LEDS_BitTimings[dat&0x3];
			dat >>= 2;
		}
	}
	// transmission takes roughly 1/4 of a millisecond (266.24us of pure data)
	HAL_SPI_Transmit_DMA(hspiLED, LEDsData, LEDS_SDATA_N);
}

void ledProgress(float progress, led_t on, led_t off){
	led_t leds[LEDS_N];
	uint32_t i;
	for (i = 0; i < LEDS_N; ++i) {
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
