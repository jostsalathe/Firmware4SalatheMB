/*
 * application.h
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

#ifndef _APPLICATION_H
#define _APPLICATION_H

//includes
#include "FreeRTOS.h"
#include "task.h"

#include "benchmarks.h"
#include "testRoutines.h"
#include "ad5592r.h"
#include "but.h"
#include "enc.h"
#include "leds.h"
#include "oled.h"
#include "sdCard.h"
#include "term.h"

//global variables and defines

//type definitions

//function prototypes
void appInit(SPI_HandleTypeDef *hspi_ad5592r,
		TIM_HandleTypeDef *htim_enc0,
		TIM_HandleTypeDef *htim_enc1,
		SPI_HandleTypeDef *hspi_led,
		SPI_HandleTypeDef *hspi_oled_ad1938,
		UART_HandleTypeDef *huart_term,
		SDRAM_HandleTypeDef *hsdram);
void appGui();

#endif /* _APPLICATION_H */
