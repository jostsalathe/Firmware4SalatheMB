/*
 * oled.h
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

#ifndef _OLED_H
#define _OLED_H

#include "stm32f7xx_hal.h"
#include "font.h"

//global variables and defines
#define OLED_WIDTH 96
#define OLED_HEIGHT 64
#define OLED_RED 0xF800
#define OLED_GREEN 0x07E0
#define OLED_BLUE 0x00DF
#define OLED_WHITE 0xFFFF

//type definitions

//function prototypes
uint16_t oledColor565(uint8_t r, uint8_t g, uint8_t b);
void oledCurSet(uint8_t x, uint8_t y);
void oledCurHome();
void oledClear();
void oledPushPixels(uint16_t *color, uint16_t n);
void oledPushPixeln(uint16_t color, uint16_t n);
void oledPushPixel(uint16_t color);
void oledPutChar(char c, uint16_t color);
void oledPutString(char *s, uint16_t color);
void oledSetup(SPI_HandleTypeDef* hspi);
void oledSetWindow(uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax);
void oledFillScreen(uint16_t color);
void oledFillRectangel(uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax, uint16_t color);
void oledProgress(float progress, uint16_t color);
void oledWriteFontset(uint16_t color);
void oledWriteCmd(uint8_t c);
void oledWriteData(uint8_t d);

#endif /*_OLED_H*/
