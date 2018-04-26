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
#include "sdram.h"
#include "sdCard.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

//function declarations
// tests the maximum sample rate when configuring all pins of CHIP0 as digital output (returns sample rate in Hz)
uint64_t ad5592rBenchmarkGPO(SPI_HandleTypeDef *hspi);
// tests the maximum sample rate when configuring all pins of CHIP0 as analog output (returns sample rate in Hz)
uint64_t ad5592rBenchmarkDAC(SPI_HandleTypeDef *hspi);
// test the writing transfer rate of the SDRAM (returns transfer rate in bit/s)
uint64_t sdramBenchmarkWrite();
// test the reading transfer rate of the SDRAM (returns transfer rate in bit/s)
uint64_t sdramBenchmarkRead();
// test the writing transfer rate of the SD card (returns transfer rate in bit/s)
uint64_t sdCardBenchmarkWrite();
// test the reading transfer rate of the SD card (returns transfer rate in bit/s)
uint64_t sdCardBenchmarkRead();

#endif /*_BENCHMARKS_H*/
