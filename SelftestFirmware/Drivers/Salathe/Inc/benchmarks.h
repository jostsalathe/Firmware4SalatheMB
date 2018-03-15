/*
 * benchmarks.h
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

#ifndef _BENCHMARKS_H
#define _BENCHMARKS_H

#include "ad5592r.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

#define ALLDACS

void ad5592rLibBenchmarkDAC(SPI_HandleTypeDef *hspi);
void ad5592rRegBenchmarkDAC(SPI_HandleTypeDef *hspi);

#endif /*_BENCHMARKS_H*/
