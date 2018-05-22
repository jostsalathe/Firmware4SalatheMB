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

#ifdef POTS_SMOOTH_N
uint16_t potValArray[POTS_N][POTS_SMOOTH_N] = {0};
uint32_t potSmoothed[POTS_N] = {0};
uint32_t potValInd = 0;
#endif /*POTS_SMOOTH_N*/

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
	return (float) potGetUI(potIdx)/POTS_MAX_VAL;
}

#ifdef POTS_SMOOTH_N
uint16_t potGetSmoothUI(uint8_t potIdx) {
	if (potIdx >= POTS_N) return 0;
	else return (uint16_t) (potSmoothed[potIdx]/POTS_SMOOTH_N);
}

float potGetSmoothF(uint8_t potIdx) {
	return (float) potGetSmoothUI(potIdx)/POTS_MAX_VAL;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc == hadcPots) {
		uint8_t iPot;
		for (iPot=0; iPot<8; ++iPot) {
			potValArray[iPot][potValInd%POTS_SMOOTH_N] = potValues[iPot];
			potSmoothed[iPot] = potSmoothed[iPot]
								- potValArray[iPot][(potValInd+1)%POTS_SMOOTH_N]
								+ potValArray[iPot][potValInd%POTS_SMOOTH_N];
		}
		++potValInd;
	}
}
#endif /*POTS_SMOOTH_N*/
