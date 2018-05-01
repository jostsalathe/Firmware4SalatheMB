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
#define OLED_DC_PIN   SPI1_DC_OLED_Pin
#define OLED_DC_PORT  SPI1_DC_OLED_GPIO_Port
#define OLED_CS_PIN   SPI1_CS_OLED_Pin
#define OLED_CS_PORT  SPI1_CS_OLED_GPIO_Port

#define SSD1331_CMD_DRAWLINE 		0x21
#define SSD1331_CMD_DRAWRECT 		0x22
#define SSD1331_CMD_COPY	 		0x23
#define SSD1331_CMD_CLEAR_WINDOW	0x25
#define SSD1331_CMD_FILL 			0x26
#define SSD1331_CMD_SETCOLUMN 		0x15
#define SSD1331_CMD_SETROW    		0x75
#define SSD1331_CMD_CONTRASTA 		0x81
#define SSD1331_CMD_CONTRASTB 		0x82
#define SSD1331_CMD_CONTRASTC		0x83
#define SSD1331_CMD_MASTERCURRENT 	0x87
#define SSD1331_CMD_SETREMAP 		0xA0
#define SSD1331_CMD_STARTLINE 		0xA1
#define SSD1331_CMD_DISPLAYOFFSET 	0xA2
#define SSD1331_CMD_NORMALDISPLAY 	0xA4
#define SSD1331_CMD_DISPLAYALLON  	0xA5
#define SSD1331_CMD_DISPLAYALLOFF 	0xA6
#define SSD1331_CMD_INVERTDISPLAY 	0xA7
#define SSD1331_CMD_SETMULTIPLEX  	0xA8
#define SSD1331_CMD_SETMASTER 		0xAD
#define SSD1331_CMD_DISPLAYOFF 		0xAE
#define SSD1331_CMD_DISPLAYON     	0xAF
#define SSD1331_CMD_POWERMODE 		0xB0
#define SSD1331_CMD_PRECHARGE 		0xB1
#define SSD1331_CMD_CLOCKDIV 		0xB3
#define SSD1331_CMD_PRECHARGEA 		0x8A
#define SSD1331_CMD_PRECHARGEB 		0x8B
#define SSD1331_CMD_PRECHARGEC 		0x8C
#define SSD1331_CMD_PRECHARGELEVEL 	0xBB
#define SSD1331_CMD_VCOMH 			0xBE
#define SSD1331_WRITE_DC(x) {HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, x?GPIO_PIN_SET:GPIO_PIN_RESET);}
#define SSD1331_WRITE_CS(x) {HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, x?GPIO_PIN_SET:GPIO_PIN_RESET);}

#define OLED_CHAR_WIDTH 5
#define OLED_CHAR_HEIGHT 8
#define OLED_HSPACE 1
#define OLED_VSPACE 2

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

void oledClear(){
    oledWriteCmd(SSD1331_CMD_CLEAR_WINDOW);	// 0x25
    oledWriteCmd(0);						// Column 1
    oledWriteCmd(0);						// Row 1
    oledWriteCmd(OLED_WIDTH-1);				// Column 2
    oledWriteCmd(OLED_HEIGHT-1);			// Row 2
    oledCurX = 0;
    oledCurY = 0;
    vTaskDelay(10);
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
			oledWriteCmd(SSD1331_CMD_COPY);				// 0x23
			oledWriteCmd(0);							// Column start
			oledWriteCmd(shiftAmount);					// Row start
			oledWriteCmd(OLED_WIDTH-1);					// Column end
			oledWriteCmd(OLED_HEIGHT-1);				// Row end
			oledWriteCmd(0);							// Column target
			oledWriteCmd(0);							// Row target
			oledCurY -= shiftAmount;
			vTaskDelay(10);
			oledWriteCmd(SSD1331_CMD_CLEAR_WINDOW);		// 0x25
			oledWriteCmd(0);							// Column 1
			oledWriteCmd(oledCurY-OLED_VSPACE);			// Row 1
			oledWriteCmd(OLED_WIDTH-1);					// Column 2
			oledWriteCmd(OLED_HEIGHT-1);				// Row 2
			vTaskDelay(10);
		}
	}

	if(c==' '){
		oledFillRectangel(oledCurX, oledCurY, oledCurX+OLED_CHAR_WIDTH-1+OLED_HSPACE, oledCurY+OLED_CHAR_HEIGHT-1+OLED_VSPACE, 0);
		oledCurX += OLED_CHAR_WIDTH+OLED_HSPACE;
	}else if(c!='\n' && c!='\r'){
		oledSetWindow(oledCurX, oledCurY, oledCurX+OLED_CHAR_WIDTH-1+OLED_HSPACE, oledCurY+OLED_CHAR_HEIGHT-1+OLED_VSPACE);
		const uint8_t *charCol = oledCharMap[(uint8_t) c];
		uint8_t row=0, col=0;
		uint8_t rowMask = 0x80;
		uint16_t charMatrix[(OLED_CHAR_HEIGHT+OLED_VSPACE)*(OLED_CHAR_WIDTH+OLED_HSPACE)], i=0;
		for(row=0; row<OLED_CHAR_HEIGHT+OLED_VSPACE; ++row){
			for(col=0; col<OLED_CHAR_WIDTH+OLED_HSPACE; ++col){
				if(col<OLED_CHAR_WIDTH && row<OLED_CHAR_HEIGHT){
					charMatrix[i] = (charCol[col]&rowMask)?color:0;
				}else{
					charMatrix[i] = 0;
				}
				++i;
			}
			rowMask >>= 1;
		}
		oledPushPixels(charMatrix,i);
		oledCurX += OLED_CHAR_WIDTH+OLED_HSPACE;
		oledSetWindow(0, 0, OLED_WIDTH-1, OLED_HEIGHT-1);
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
    oledWriteCmd(SSD1331_CMD_DISPLAYOFF);  	// 0xAE
    oledWriteCmd(SSD1331_CMD_SETREMAP); 	// 0xA0
    oledWriteCmd(0x72);						// RGB Color
    oledWriteCmd(SSD1331_CMD_STARTLINE); 	// 0xA1
    oledWriteCmd(0x0);
    oledWriteCmd(SSD1331_CMD_DISPLAYOFFSET);// 0xA2
    oledWriteCmd(0x0);
    oledWriteCmd(SSD1331_CMD_NORMALDISPLAY);// 0xA4
    oledWriteCmd(SSD1331_CMD_SETMULTIPLEX); // 0xA8
    oledWriteCmd(0x3F);  					// 0x3F 1/64 duty
    oledWriteCmd(SSD1331_CMD_SETMASTER);  	// 0xAD
    oledWriteCmd(0x8E);
    oledWriteCmd(SSD1331_CMD_POWERMODE);  	// 0xB0
    oledWriteCmd(0x0B);
    oledWriteCmd(SSD1331_CMD_PRECHARGE);  	// 0xB1
    oledWriteCmd(0x31);
    oledWriteCmd(SSD1331_CMD_CLOCKDIV);  	// 0xB3
    oledWriteCmd(0xF0);  // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    oledWriteCmd(SSD1331_CMD_PRECHARGEA);  	// 0x8A
    oledWriteCmd(0x64);
    oledWriteCmd(SSD1331_CMD_PRECHARGEB);  	// 0x8B
    oledWriteCmd(0x78);
    oledWriteCmd(SSD1331_CMD_PRECHARGEC);  	// 0x8C
    oledWriteCmd(0x64);
    oledWriteCmd(SSD1331_CMD_PRECHARGELEVEL);// 0xBB
    oledWriteCmd(0x3A);
    oledWriteCmd(SSD1331_CMD_VCOMH);  		// 0xBE
    oledWriteCmd(0x3E);
    oledWriteCmd(SSD1331_CMD_MASTERCURRENT);// 0x87
    oledWriteCmd(0x06);
    oledWriteCmd(SSD1331_CMD_CONTRASTA);  	// 0x81
    oledWriteCmd(0x91);
    oledWriteCmd(SSD1331_CMD_CONTRASTB);  	// 0x82
    oledWriteCmd(0x50);
    oledWriteCmd(SSD1331_CMD_CONTRASTC);  	// 0x83
    oledWriteCmd(0x7D);
    oledWriteCmd(SSD1331_CMD_CLEAR_WINDOW);	// 0x25
    oledWriteCmd(0);						// Column 1
    oledWriteCmd(0);						// Row 1
    oledWriteCmd(OLED_WIDTH-1);				// Column 2
    oledWriteCmd(OLED_HEIGHT-1);			// Row 2
    oledWriteCmd(SSD1331_CMD_DISPLAYON);	//--turn on oled panel

}

void oledSetWindow(uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax){
	oledWriteCmd(SSD1331_CMD_SETCOLUMN);
    oledWriteCmd(xMin);										// Column start
    oledWriteCmd(xMax);	// Column end
	oledWriteCmd(SSD1331_CMD_SETROW);
    oledWriteCmd(yMin);										// Row start
    oledWriteCmd(yMax);	// Row end
}

void oledFillRectangel(uint8_t xMin, uint8_t yMin, uint8_t xMax, uint8_t yMax, uint16_t color) {
	oledSetWindow(xMin, yMin, xMax, yMax);
	oledPushPixeln(color, (xMax-xMin+1) * (yMax-yMin+1));
	oledSetWindow(0, 0, OLED_WIDTH-1, OLED_HEIGHT-1);
}

void oledProgress(float progress, uint16_t color) {
	uint8_t progr = progress*(OLED_WIDTH)+0.5;
	if (progr<OLED_WIDTH)
		oledFillRectangel(progr, OLED_HEIGHT-1, OLED_WIDTH-1, OLED_HEIGHT-1, 0);
	if (progr)
		oledFillRectangel(0, OLED_HEIGHT-1, progr-1, OLED_HEIGHT-1, color);
}

void oledFillScreen(uint16_t color) {
	oledFillRectangel(0, 0, OLED_WIDTH-1, OLED_HEIGHT-1, color);
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
