/*
 * pots.h
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

#ifndef _POTS_H
#define _POTS_H

#include "stm32f7xx_hal.h"
#include "adc.h"

//global variables and defines
#define POTS_N 8
#define POTS_MAX_VAL 4095
#define POTS_SMOOTH_N 256 //256 for 80.61ms round trip (choose power of 2!)


//type definitions

//function prototypes
void potsSetup(ADC_HandleTypeDef* hadc);
uint16_t potGetUI(uint8_t potIdx);
float potGetF(uint8_t potIdx);
#ifdef POTS_SMOOTH_N
uint16_t potGetSmoothUI(uint8_t potIdx);
float potGetSmoothF(uint8_t potIdx);
#endif /*POTS_SMOOTH_N*/

#endif /*_POTS_H*/
