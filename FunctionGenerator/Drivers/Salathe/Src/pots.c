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
// measured values
uint16_t potValArray[POTS_N][POTS_SMOOTH_N];
// rolling averages
uint32_t potSmoothed[POTS_N];
// value index
uint32_t potValInd;
#endif /*POTS_SMOOTH_N*/

//functions
void potsSetup(ADC_HandleTypeDef* hadc) {
	hadcPots = hadc;
#ifdef POTS_SMOOTH_N
	// initialize variables for rolling averages
	int i,j;
	potValInd = 0;
	for (i=0; i<POTS_N; ++i) {
		// offset for correct rounding
		potSmoothed[i] = POTS_SMOOTH_N/2;
		for (j=0; j<POTS_SMOOTH_N; ++j) {
			potValArray[i][j] = 0;
		}
	}
#endif /*POTS_SMOOTH_N*/
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
	else {
		while (hadcPots->Lock);
		return (uint16_t) (potSmoothed[potIdx]/POTS_SMOOTH_N);
	}
}

float potGetSmoothF(uint8_t potIdx) {
	return (float) potGetSmoothUI(potIdx)/POTS_MAX_VAL;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if (hadc == hadcPots) {
		hadc->Lock = HAL_LOCKED;
		uint8_t iPot;
		for (iPot=0; iPot<8; ++iPot) {
			potSmoothed[iPot] = potSmoothed[iPot]
								- potValArray[iPot][potValInd%POTS_SMOOTH_N]
								+ potValues[iPot];
			potValArray[iPot][potValInd%POTS_SMOOTH_N] = potValues[iPot];
		}
		++potValInd;
		hadc->Lock = HAL_UNLOCKED;
	}
}
#endif /*POTS_SMOOTH_N*/
