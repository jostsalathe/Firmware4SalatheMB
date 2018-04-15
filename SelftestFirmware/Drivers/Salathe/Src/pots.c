/*
 * pots.c
 *
 *  Created on: 14.04.2018
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

#include "pots.h"

//variable definitions
ADC_HandleTypeDef* hadcPots = NULL;
uint16_t potValues[POTS_N] = {0};

//functions
void potsSetup(ADC_HandleTypeDef* hadc) {
	hadcPots = hadc;
	HAL_ADC_Start_DMA(hadcPots, (uint32_t*) potValues, POTS_N);
}

uint16_t potGetUI(uint8_t potIdx) {
	if (potIdx >= POTS_N) return 0;
	else return potValues[potIdx];
}

float potGetF(uint8_t potIdx) {
	if (potIdx >= POTS_N) return 0.0;
	else return (float) potValues[potIdx]/POTS_MAX_VAL;
}
