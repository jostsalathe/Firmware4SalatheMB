/*
 * font.h
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

#ifndef _FONT_C
#define _FONT_C

#include "stm32f7xx_hal.h"

#define FONT_NCHARS 128
#define FONT_BYTEPERCHAR 5

extern const char hexDigits[16];
extern const uint8_t oledCharMap[FONT_NCHARS][FONT_BYTEPERCHAR];
char *hex2Str(uint32_t num, uint8_t digits, char *str);
char *uint2Str(uint32_t num, uint8_t digits, char *str);

#endif /*_FONT_C*/
