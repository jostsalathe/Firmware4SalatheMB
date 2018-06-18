/*
 * application.c
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

#include "application.h"

uint8_t booting = 1;
ad1938_HandleTypeDef ad1938Handle;

void appInit() {
	led_t leds[LEDS_N];
	uint32_t i = 0;

	ad5592rSetup(&hspi6, &htim6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	buttonSetup();
	encSetup(&htim3, 0);
	encSetup(&htim4, 1);
	ledSetup(&hspi4);
	oledSetup(&hspi1);
	potsSetup(&hadc1);
	termSetup(&huart1);
	sdramSetup(&hsdram1);
	sdCardSetup();

	oledFillScreen(OLED_WHITE);
	for (i = 0; i < LEDS_N; ++i) {
		leds[i].red = 0;
		leds[i].green = 0;
		leds[i].blue = 0;
	}
	ledSet(leds);

	termPutString("\r\rbooting... \r");
	termPutString(" --.      |        |   |\r");
	termPutString("|         |       -|-  |     /\r");
	termPutString("'--   --  |    --  |   |--   --.\r");
	termPutString("   | |  | |   |  | |   |  | |--'\r");
	termPutString("'--' '--: '-' '--: '-' '  ' '--\r");
	termPutString("platform for developing digital\r");
	termPutString("     synthesizer modules\r");

//	vTaskDelay(1000);

	oledClear();

	termPutString("\r--- testing peripherals ---\r");
/*	testAD5592R(&hspi6, &htim6);
	testSDCARD();
	testSDRAM(&hsdram1);
*/
	ledSet(leds);
	termPutString("\r--- peripherals check done ---\r");

	vTaskDelay(1000);

	oledClear();
	booting = 0;
	appAudio();
}

void appAudio() {
	int i, bufSize = 480;
	int64_t iSamp = 0;
	ad1938SampleType *freshInBuf;
	uint32_t freshInBufSize;
	ad1938SampleType *freshOutBuf;
	uint32_t freshOutBufSize;

	//prepare the ad1938 handle structure
	ad1938Handle.csIndex=0;
	ad1938Handle.hspi = &hspi1;
	ad1938Handle.hsaiIn = &hsai_BlockA1;
	ad1938Handle.hsaiOut = &hsai_BlockB1;
	ad1938Handle.inBuf = (ad1938SampleType*) SDRAM_ADDR;
	ad1938Handle.inBufSize = bufSize*4;
	ad1938Handle.outBuf = (ad1938SampleType*) SDRAM_ADDR+bufSize*4;
	ad1938Handle.outBufSize = bufSize*8;

	//initialize ad1938 and related peripherals
	ad1938Setup(&ad1938Handle);

	//initialize the transmission buffer with the first output interval. In this case a sine wave for demo purpose
	for (i=0; i<bufSize; ++i) {
		int j;
		for (j=0; j<8; ++j) {
//			(ad1938Handle.outBuf)[i*8+j] = (ad1938SampleType) iSamp;
			(ad1938Handle.outBuf)[i*8+j] = (ad1938SampleType) ad5592rSine[iSamp]<<16;
		}
//		if((iSamp+=2000000)>=2000000000) iSamp=-2000000000;
		if((iSamp+=4)>=1000) iSamp=0;
	}

	//start the DMA transfers for audio streaming
	ad1938Start();

	//enter the sample calculation loop
	while(1) {
		//wait for new buffers to process
		ad1938WaitOnBuffers(&freshInBuf, &freshInBufSize, &freshOutBuf, &freshOutBufSize);
		for (i=0; i<freshOutBufSize/8; ++i) {
			int j;
//			ad1938SampleType sample = (ad1938SampleType) iSamp;
			ad1938SampleType sample = (ad1938SampleType) ad5592rSine[iSamp]<<16;
			for (j=0; j<4; ++j) {
				freshOutBuf[i*8+j] = freshInBuf[i*4+j]/POTS_MAX_VAL*potGetSmoothUI(j);
				freshOutBuf[i*8+j+4] = freshInBuf[i*4+j]/POTS_MAX_VAL*potGetSmoothUI(j);
			}
			for (j=4; j<8; ++j) {
//				freshOutBuf[i*8+j] = (ad1938SampleType) sample/POTS_MAX_VAL*potGetSmoothUI(j);
			}
//			if((iSamp+=2000000)>=2000000000) iSamp=-2000000000;
			if((iSamp+=4)>=1000) iSamp=0;
		}
		//calculate next set of samples
		//but do nothing in this demo - sine wave stays the same
	}

	//stop the DMA transfers to quit audio streaming
	ad1938Stop();
}

char generateSpinner(uint32_t val) {
	switch (val%4) {
	case 0: return '|';
	case 1: return '/';
	case 2: return '-';
	case 3: return '\\';
	default: return ' ';
	}
}

void appGui() {
	TickType_t xLastWakeTime;
	led_t ledColor, ledOff, led[LEDS_N];
	uint32_t i = 0, loopCnt = 0;
	const uint32_t enc1Max = 96;

	xLastWakeTime = xTaskGetTickCount();
	while (booting) vTaskDelayUntil(&xLastWakeTime, 10);

	ledColor.red = 26;
	ledColor.green = 26;
	ledColor.blue = 26;
	ledOff.red = 0;
	ledOff.green = 0;
	ledOff.blue = 0;

	for (i = 0; i < LEDS_N; ++i) {
		led[i] = ledOff; //Color;
	}

	oledClear();
	oledCurSet(34,10);
	oledPutChar('0', OLED_BLUE);
	oledCurSet(44,10);
	oledPutChar('P', OLED_BLUE);
	oledCurSet(54,10);
	oledPutChar('1', OLED_BLUE);
	oledCurSet(34,20);
	oledPutChar('2', OLED_BLUE);
	oledCurSet(44,20);
	oledPutChar('O', OLED_BLUE);
	oledCurSet(54,20);
	oledPutChar('3', OLED_BLUE);
	oledCurSet(34,30);
	oledPutChar('4', OLED_BLUE);
	oledCurSet(44,30);
	oledPutChar('T', OLED_BLUE);
	oledCurSet(54,30);
	oledPutChar('5', OLED_BLUE);
	oledCurSet(34,40);
	oledPutChar('6', OLED_BLUE);
	oledCurSet(44,40);
	oledPutChar('I', OLED_BLUE);
	oledCurSet(54,40);
	oledPutChar('7', OLED_BLUE);

	/* Infinite loop */
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, 100); //100ms cycle time

		if (buttonFalling(BUTTONENC0)) {
			if (encValue(0) == 85) encSet(0, 170);
			else encSet(0, 85);
		}
		if (buttonFalling(BUTTONENC1)) {
			encSet(0, 0);
		}
		uint16_t pos0 = encValue(0);
		uint16_t pos1 = encValue(1);

		for (i = 0; i < 8; ++i) {
			if (pos0 & (1 << i)) {	//i<pos%NLEDS
				led[i] = ledColor;
			} else {
				led[i] = ledOff;
			}
		}
		ledSet(led);

		if (pos1 > ENC_MAX-16) {
			pos1 = enc1Max;
			encSet(1, pos1);
		} else if (pos1 > enc1Max) {
			pos1 = 0;
			encSet(1, pos1);
		}
		oledProgress((float) pos1/enc1Max, OLED_GREEN);

		char intBuf[6];
		oledCurSet(0,0);
		oledPutString(uint2Str(pos0, 5, intBuf), OLED_GREEN);
		oledCurSet(65,0);
		oledPutString(uint2Str(pos1, 5, intBuf), OLED_GREEN);
		oledCurSet(34,0);
		oledPutChar(generateSpinner(pos0), OLED_GREEN);
		oledCurSet(44,0);
		oledPutChar(generateSpinner(loopCnt), OLED_RED|OLED_GREEN);
		oledCurSet(54,0);
		oledPutChar(generateSpinner(pos1), OLED_GREEN);

		oledCurSet(0,10);
		oledPutString(uint2Str(potGetSmoothUI(0), 4, intBuf), OLED_BLUE);
		oledCurSet(71,10);
		oledPutString(uint2Str(potGetSmoothUI(1), 4, intBuf), OLED_BLUE);
		oledCurSet(0,20);
		oledPutString(uint2Str(potGetSmoothUI(2), 4, intBuf), OLED_BLUE);
		oledCurSet(71,20);
		oledPutString(uint2Str(potGetSmoothUI(3), 4, intBuf), OLED_BLUE);
		oledCurSet(0,30);
		oledPutString(uint2Str(potGetSmoothUI(4), 4, intBuf), OLED_BLUE);
		oledCurSet(71,30);
		oledPutString(uint2Str(potGetSmoothUI(5), 4, intBuf), OLED_BLUE);
		oledCurSet(0,40);
		oledPutString(uint2Str(potGetSmoothUI(6), 4, intBuf), OLED_BLUE);
		oledCurSet(71,40);
		oledPutString(uint2Str(potGetSmoothUI(7), 4, intBuf), OLED_BLUE);

		++loopCnt;
	}
}
