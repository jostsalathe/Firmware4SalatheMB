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

//fundamental commands
#define SSD1331_CMD_SETCOLUMN 		0x15
#define SSD1331_CMD_SETROW    		0x75
#define SSD1331_CMD_CONTRASTA 		0x81
#define SSD1331_CMD_CONTRASTB 		0x82
#define SSD1331_CMD_CONTRASTC		0x83
#define SSD1331_CMD_MASTERCURRENT 	0x87
#define SSD1331_CMD_PRECHARGEA 		0x8A
#define SSD1331_CMD_PRECHARGEB 		0x8B
#define SSD1331_CMD_PRECHARGEC 		0x8C
#define SSD1331_CMD_SETREMAP 		0xA0
#define SSD1331_CMD_STARTLINE 		0xA1
#define SSD1331_CMD_DISPLAYOFFSET 	0xA2
#define SSD1331_CMD_NORMALDISPLAY 	0xA4
#define SSD1331_CMD_DISPLAYALLON  	0xA5
#define SSD1331_CMD_DISPLAYALLOFF 	0xA6
#define SSD1331_CMD_INVERTDISPLAY 	0xA7
#define SSD1331_CMD_SETMULTIPLEX  	0xA8
#define SSD1331_CMD_DIMMODE			0xAB
#define SSD1331_CMD_SETMASTER 		0xAD
#define SSD1331_CMD_DISPLAYONDIM   	0xAC
#define SSD1331_CMD_DISPLAYOFF 		0xAE
#define SSD1331_CMD_DISPLAYON     	0xAF
#define SSD1331_CMD_POWERMODE 		0xB0
#define SSD1331_CMD_PRECHARGE 		0xB1
#define SSD1331_CMD_CLOCKDIV 		0xB3
#define SSD1331_CMD_GRAYSCALE		0xB8
#define SSD1331_CMD_LINGREYEN		0xB9
#define SSD1331_CMD_PRECHARGELEVEL 	0xBB
#define SSD1331_CMD_VCOMH 			0xBE
#define SSD1331_CMD_NOP				0xE3
#define SSD1331_CMD_LOCK			0xFD

//graphic acceleration commands
#define SSD1331_CMD_DRAWLINE 		0x21
#define SSD1331_CMD_DRAWRECT 		0x22
#define SSD1331_CMD_COPY	 		0x23
#define SSD1331_CMD_DIM_WINDOW		0x24
#define SSD1331_CMD_CLEAR_WINDOW	0x25
#define SSD1331_CMD_RECT_FILL		0x26
#define SSD1331_CMD_SCROLL_SETUP	0x27
#define SSD1331_CMD_SCROLL_END		0x2E
#define SSD1331_CMD_SCROLL_START	0x2F


//function prototypes
uint16_t oledColor565(uint8_t r, uint8_t g, uint8_t b);
void oledCurSet(uint8_t x, uint8_t y);
void oledCurHome();
void oledPushPixels(uint16_t *color, uint16_t n);
void oledPushPixeln(uint16_t color, uint16_t n);
void oledPushPixel(uint16_t color);
void oledPutChar(char c, uint16_t color);
void oledPutString(char *s, uint16_t color);
void oledSetup(SPI_HandleTypeDef* hspi);
void oledSetWindow(uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax);
void oledClearWindow(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY);
void oledClear();
void oledCopyWindow(uint8_t sourceX, uint8_t sourceY, uint8_t sizeX, uint8_t sizeY, uint8_t targetX, uint8_t targetY);
void oledDrawRectangel(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, uint16_t cBorder, uint16_t cFill);
void oledFillRectangel(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, uint16_t color);
void oledFillScreen(uint16_t color);
void oledProgress(float progress, uint16_t color);
void oledWriteFontset(uint16_t color);
void oledWriteCmd(uint8_t c);
void oledWriteData(uint8_t d);

#endif /*_OLED_H*/
