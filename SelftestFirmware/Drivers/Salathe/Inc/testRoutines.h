/*
 * testRoutines.h
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

#ifndef _TESTROUTINES_H
#define _TESTROUTINES_H

#include "FreeRTOS.h"
#include "task.h"

#include "rng.h"

#include "oled.h"
#include "leds.h"
#include "term.h"
#include "sdCard.h"
#include "ad5592r.h"

//global variables and defines
#define RAM_CHECK_FULL
#define TEST_SIZE 0x800 //variables of TEST_TYPE (0x2000 byte for one row, 0x8000 for maximum heap usage)
#define TEST_END (SDRAM_END-SDRAM_ADDR+1) //end the test right before this address offset
#define TEST_TYPE uint32_t

//type definitions

//function prototypes
void testSDRAM(SDRAM_HandleTypeDef *hsdram);
void testSDCARD();
void testAD5592R(SPI_HandleTypeDef *hspi);
void demoAD5592R(SPI_HandleTypeDef *hspi);

#endif /*_TESTROUTINES_H*/
