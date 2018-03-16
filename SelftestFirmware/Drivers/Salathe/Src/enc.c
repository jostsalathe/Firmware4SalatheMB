/*
 * enc.c
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

#include "enc.h"

//variable definitions
TIM_HandleTypeDef *hspiEnc[ENC_NENCS];

//functions
void encSetup(TIM_HandleTypeDef *htim, uint8_t encNum) {
	if (encNum < ENC_NENCS) {
		hspiEnc[encNum] = htim;
		HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_1 | TIM_CHANNEL_2);
		encSet(encNum,0);
	}
}

void encSet(uint8_t encNum, uint16_t value) {
	if (encNum < ENC_NENCS) {
		__HAL_TIM_GET_COUNTER(hspiEnc[encNum]) = value*ENC_EPD+ENC_EPD/2;
	}
}

uint16_t encValue(uint8_t encNum) {
	if (encNum < ENC_NENCS) {
		return (__HAL_TIM_GET_COUNTER(hspiEnc[encNum]))/ENC_EPD;
	} else {
		return 0;
	}
}
