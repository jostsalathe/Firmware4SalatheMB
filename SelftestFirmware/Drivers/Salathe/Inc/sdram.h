/*
 * sdram.h
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

#ifndef _SDRAM_H
#define _SDRAM_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_sdram.h"

//global variables and defines
#define SDRAM_ADDR        ((uint32_t)0xC0000000)
#define SDRAM_SIZE        ((uint32_t)0x00800000)

//type definitions

//function prototypes
void sdramSetup(SDRAM_HandleTypeDef* hsdram);

#endif /*_SDRAM_H*/
