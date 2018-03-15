/*
 * benchmarks.h
 *
 *  Created on: 15.03.2018
 *      Author: Jost Salathe
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
