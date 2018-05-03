/*
 * oled.c
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

#include "oled.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>

//defines
#define OLED_DC_PIN		SPI1_DC_OLED_Pin
#define OLED_DC_PORT	SPI1_DC_OLED_GPIO_Port
#define OLED_CS_PIN		SPI1_CS_OLED_Pin
#define OLED_CS_PORT	SPI1_CS_OLED_GPIO_Port

#define SSD1331_WRITE_DC(x)		{HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, x?GPIO_PIN_SET:GPIO_PIN_RESET);}
#define SSD1331_WRITE_CS(x)		{HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, x?GPIO_PIN_SET:GPIO_PIN_RESET);}

#define OLED_IS_COL(x)			(x>=0 && x<OLED_WIDTH)
#define OLED_IS_ROW(y)			(y>=0 && y<OLED_HEIGHT)
#define OLED_ASSERT_COL(x)		if(!OLED_IS_COL(x)) return
#define OLED_ASSERT_ROW(y)		if(!OLED_IS_ROW(y)) return

#define OLED_GET_RED(c)			((c>>11)&0x1F)
#define OLED_GET_GREEN(c)		((c>>5)&0x3F)
#define OLED_GET_BLUE(c)		((c)&0x1F)

#define OLED_CHAR_WIDTH		5
#define OLED_CHAR_HEIGHT	8
#define OLED_HSPACE			1
#define OLED_VSPACE			2

//variables
SPI_HandleTypeDef *hspiOled;
uint8_t oledCurX, oledCurY;

//functions
uint16_t oledColor565(uint8_t r, uint8_t g, uint8_t b){
  uint16_t c;
  c = r >> 3;
  c <<= 6;
  c |= g >> 2;
  c <<= 5;
  c |= b >> 3;

  return c;
}

void oledCurSet(uint8_t x, uint8_t y){
	if(x>OLED_WIDTH-1 || y>OLED_HEIGHT-1) return;
	oledCurX = x;
	oledCurY = y;
}

void oledCurHome(){
	oledCurSet(0,0);
}

void oledPushPixels(uint16_t *color, uint16_t n){
	uint8_t *transmission;
	uint16_t i;
	transmission = (uint8_t *) malloc(n*2);
	for(i=0; i<n; ++i){
		transmission[i*2] = (uint8_t)(color[i]>>8);
		transmission[i*2+1] = (uint8_t)(color[i]);
	}
	SSD1331_WRITE_DC(1);
	SSD1331_WRITE_CS(0);
	HAL_SPI_Transmit(hspiOled, transmission, n*2, 1000);
	SSD1331_WRITE_CS(1);
	free(transmission);
}

void oledPushPixeln(uint16_t color, uint16_t n){
	uint8_t transmission[2];
	uint16_t i;
	transmission[0] = (uint8_t)(color>>8);
	transmission[1] = (uint8_t)(color);
	SSD1331_WRITE_DC(1);
	SSD1331_WRITE_CS(0);
	for(i=0; i<n; ++i){
		HAL_SPI_Transmit(hspiOled, transmission, 2, 1000);
	}
	SSD1331_WRITE_CS(1);
}

void oledPushPixel(uint16_t color){
	oledPushPixeln(color, 1);
}

void oledPutChar(char c, uint16_t color){
	c &= 0x7F;
	if(c=='\r'){
		oledCurX = 0;
	}else if(oledCurX+OLED_CHAR_WIDTH+OLED_HSPACE>OLED_WIDTH || c=='\n'){		//reached end of line
		oledCurY += OLED_CHAR_HEIGHT+OLED_VSPACE;
		oledCurX = 0;
		if(oledCurY+OLED_CHAR_HEIGHT+OLED_VSPACE>OLED_HEIGHT){		//reached bottom of display
			uint8_t shiftAmount = oledCurY+OLED_CHAR_HEIGHT+OLED_VSPACE-OLED_HEIGHT;
			oledCopyWindow(0, shiftAmount, OLED_WIDTH, OLED_HEIGHT-shiftAmount, 0, 0);
			oledCurY -= shiftAmount;
			oledClearWindow(0, oledCurY-OLED_VSPACE, OLED_WIDTH, shiftAmount);
		}
	}

	if(c==' '){
		oledFillRectangel(oledCurX, oledCurY, oledCurX+OLED_CHAR_WIDTH-1+OLED_HSPACE, oledCurY+OLED_CHAR_HEIGHT-1+OLED_VSPACE, 0);
		oledCurX += OLED_CHAR_WIDTH+OLED_HSPACE;
	}else if(c!='\n' && c!='\r'){
		oledClearWindow(oledCurX+OLED_CHAR_WIDTH, oledCurY, OLED_HSPACE, OLED_CHAR_HEIGHT);
		oledClearWindow(oledCurX, oledCurY+OLED_CHAR_HEIGHT, OLED_CHAR_WIDTH+OLED_HSPACE, OLED_VSPACE);
		oledSetWindow(oledCurX, oledCurY, OLED_CHAR_WIDTH, OLED_CHAR_HEIGHT);
		const uint8_t *charCol = oledCharMap[(uint8_t) c];
		uint8_t row=0, col=0;
		uint8_t rowMask = 0x80;
		uint16_t charMatrix[(OLED_CHAR_HEIGHT)*(OLED_CHAR_WIDTH)], i=0;
		for(row=0; row<OLED_CHAR_HEIGHT; ++row){
			for(col=0; col<OLED_CHAR_WIDTH; ++col){
				charMatrix[i] = (charCol[col]&rowMask)?color:0;
				++i;
			}
			rowMask >>= 1;
		}
		oledPushPixels(charMatrix,i);
		oledCurX += OLED_CHAR_WIDTH+OLED_HSPACE;
		oledSetWindow(0, 0, OLED_WIDTH, OLED_HEIGHT);
	}
}

void oledPutString(char *s, uint16_t color) {
	uint32_t i = 0;
	while(s[i]!='\0'){
		oledPutChar(s[i], color);
		++i;
	}
}

void oledSetup(SPI_HandleTypeDef* hspi) {
	hspiOled = hspi;
	oledCurX = 0;
	oledCurY = 0;
	//initialization sequence
	oledWriteCmd(SSD1331_CMD_NOP);
    oledWriteCmd(SSD1331_CMD_DISPLAYOFF);
    oledWriteCmd(SSD1331_CMD_SETREMAP);
    oledWriteCmd(0x72);						// RGB Color
    oledWriteCmd(SSD1331_CMD_STARTLINE);
    oledWriteCmd(0x0);
    oledWriteCmd(SSD1331_CMD_DISPLAYOFFSET);
    oledWriteCmd(0x0);
    oledWriteCmd(SSD1331_CMD_NORMALDISPLAY);
    oledWriteCmd(SSD1331_CMD_SETMULTIPLEX);
    oledWriteCmd(0x3F);  					// 1/64 duty
    oledWriteCmd(SSD1331_CMD_SETMASTER);
    oledWriteCmd(0x8E);
    oledWriteCmd(SSD1331_CMD_POWERMODE);
    oledWriteCmd(0x0B);
    oledWriteCmd(SSD1331_CMD_PRECHARGE);
    oledWriteCmd(0x31);
    oledWriteCmd(SSD1331_CMD_CLOCKDIV);
    oledWriteCmd(0xF0);  // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    oledWriteCmd(SSD1331_CMD_PRECHARGEA);
    oledWriteCmd(0x64);
    oledWriteCmd(SSD1331_CMD_PRECHARGEB);
    oledWriteCmd(0x78);
    oledWriteCmd(SSD1331_CMD_PRECHARGEC);
    oledWriteCmd(0x64);
    oledWriteCmd(SSD1331_CMD_PRECHARGELEVEL);
    oledWriteCmd(0x3A);
    oledWriteCmd(SSD1331_CMD_VCOMH);
    oledWriteCmd(0x3E);
    oledWriteCmd(SSD1331_CMD_MASTERCURRENT);
    oledWriteCmd(0x06);
    oledWriteCmd(SSD1331_CMD_CONTRASTA);
    oledWriteCmd(0x91);
    oledWriteCmd(SSD1331_CMD_CONTRASTB);
    oledWriteCmd(0x50);
    oledWriteCmd(SSD1331_CMD_CONTRASTC);
    oledWriteCmd(0x7D);
    oledWriteCmd(SSD1331_CMD_RECT_FILL);
    oledWriteCmd(0x1);
    oledWriteCmd(SSD1331_CMD_CLEAR_WINDOW);
    oledWriteCmd(0);						// Column 1
    oledWriteCmd(0);						// Row 1
    oledWriteCmd(OLED_WIDTH-1);				// Column 2
    oledWriteCmd(OLED_HEIGHT-1);			// Row 2
    oledWriteCmd(SSD1331_CMD_DISPLAYON);

}

void oledSetWindow(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY) {
	OLED_ASSERT_COL(sizeX-1);
	OLED_ASSERT_ROW(sizeY-1);
	OLED_ASSERT_COL(posX);
	OLED_ASSERT_ROW(posY);
	OLED_ASSERT_COL(posX+sizeX-1);
	OLED_ASSERT_ROW(posY+sizeY-1);
	oledWriteCmd(SSD1331_CMD_SETCOLUMN);
    oledWriteCmd(posX);						// Column start
    oledWriteCmd(posX+sizeX-1);				// Column end
	oledWriteCmd(SSD1331_CMD_SETROW);
    oledWriteCmd(posY);						// Row start
    oledWriteCmd(posY+sizeY-1);				// Row end
}

void oledClearWindow(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY) {
	OLED_ASSERT_COL(sizeX-1);
	OLED_ASSERT_ROW(sizeY-1);
	OLED_ASSERT_COL(posX);
	OLED_ASSERT_ROW(posY);
	OLED_ASSERT_COL(posX+sizeX-1);
	OLED_ASSERT_ROW(posY+sizeY-1);
    oledWriteCmd(SSD1331_CMD_CLEAR_WINDOW);	// 0x25
    oledWriteCmd(posX);						// Column 1
    oledWriteCmd(posY);						// Row 1
    oledWriteCmd(posX+sizeX-1);				// Column 2
    oledWriteCmd(posY+sizeY-1);				// Row 2
	vTaskDelay(1);
}

void oledClear() {
	oledClearWindow(0, 0, OLED_WIDTH, OLED_HEIGHT);
}

void oledCopyWindow(uint8_t sourceX, uint8_t sourceY, uint8_t sizeX, uint8_t sizeY, uint8_t targetX, uint8_t targetY) {
	OLED_ASSERT_COL(sizeX-1);
	OLED_ASSERT_ROW(sizeY-1);
	OLED_ASSERT_COL(sourceX);
	OLED_ASSERT_ROW(sourceY);
	OLED_ASSERT_COL(targetX);
	OLED_ASSERT_ROW(targetY);
	OLED_ASSERT_COL(sourceX+sizeX-1);
	OLED_ASSERT_ROW(sourceY+sizeY-1);
	oledWriteCmd(SSD1331_CMD_COPY);			// 0x23
	oledWriteCmd(sourceX);					// Column start
	oledWriteCmd(sourceY);					// Row start
	oledWriteCmd(sourceX+sizeX-1);			// Column end
	oledWriteCmd(sourceY+sizeY-1);			// Row end
	oledWriteCmd(targetX);					// Column target
	oledWriteCmd(targetY);					// Row target
	vTaskDelay(1);
}

void oledDrawRectangel(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, uint16_t cBorder, uint16_t cFill) {
	OLED_ASSERT_COL(sizeX-1);
	OLED_ASSERT_ROW(sizeY-1);
	OLED_ASSERT_COL(posX);
	OLED_ASSERT_ROW(posY);
	OLED_ASSERT_COL(posX+sizeX-1);
	OLED_ASSERT_ROW(posY+sizeY-1);
	oledWriteCmd(SSD1331_CMD_DRAWRECT);
	oledWriteCmd(posX);						// Column 1
	oledWriteCmd(posY);						// Row 1
	oledWriteCmd(posX+sizeX-1);				// Column 2
	oledWriteCmd(posY+sizeY-1);				// Row 2
	oledWriteCmd(OLED_GET_RED(cBorder)<<1);	// border color red
	oledWriteCmd(OLED_GET_GREEN(cBorder));	// border color green
	oledWriteCmd(OLED_GET_BLUE(cBorder)<<1);// border color blue
	oledWriteCmd(OLED_GET_RED(cFill)<<1);	// fill color red
	oledWriteCmd(OLED_GET_GREEN(cFill));	// fill color green
	oledWriteCmd(OLED_GET_BLUE(cFill)<<1);	// fill color blue
	vTaskDelay(1);
}

void oledFillRectangel(uint8_t posX, uint8_t posY, uint8_t sizeX, uint8_t sizeY, uint16_t color) {
	oledDrawRectangel(posX, posY, sizeX, sizeY, color, color);
}

void oledFillScreen(uint16_t color) {
	oledFillRectangel(0, 0, OLED_WIDTH, OLED_HEIGHT, color);
}

void oledProgress(float progress, uint16_t color) {
	uint8_t progr = progress*(OLED_WIDTH)+0.5;
	if (progr<OLED_WIDTH)
		oledFillRectangel(progr, OLED_HEIGHT-1, OLED_WIDTH-progr, 1, 0);
	if (progr)
		oledFillRectangel(0, OLED_HEIGHT-1, progr, 1, color);
}

void oledWriteFontset(uint16_t color){
	uint8_t i;
	for(i=0; i<FONT_NCHARS; ++i){
		oledPutChar(i, color);
	}
}

void oledWriteCmd(uint8_t c){
	SSD1331_WRITE_DC(0);
	SSD1331_WRITE_CS(0);
	HAL_SPI_Transmit(hspiOled, &c, 1, 1000);
	SSD1331_WRITE_CS(1);
}

void oledWriteData(uint8_t d){
	SSD1331_WRITE_DC(1);
	SSD1331_WRITE_CS(0);
	HAL_SPI_Transmit(hspiOled, &d, 1, 1000);
	SSD1331_WRITE_CS(1);
}
