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

#include "fmc.h"
#include "spi.h"
#include "tim.h"

#include "benchmarks.h"
#include "testRoutines.h"
#include "ad1938.h"
#include "ad5592r.h"
#include "but.h"
#include "enc.h"
#include "leds.h"
#include "oled.h"
#include "pots.h"
#include "sdCard.h"
#include "sdram.h"
#include "term.h"

//global variables and defines

//type definitions

//function prototypes
void appInit();
void appAudio();
void appGui();

#endif /* _APPLICATION_H */
