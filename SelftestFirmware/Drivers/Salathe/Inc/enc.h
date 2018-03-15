/*
 * enc.h
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

#ifndef _ENC_H
#define _ENC_H

#include "stm32f7xx_hal.h"

//global variables and defines
#ifndef ENC_NENCS
#define ENC_NENCS 2
#endif
//type definitions

//function prototypes
void encSetup(TIM_HandleTypeDef *htim, uint8_t encNum);
void encSet(uint8_t encNum, uint16_t value);
uint16_t encValue(uint8_t encNum);

#endif /*_ENC_H*/
