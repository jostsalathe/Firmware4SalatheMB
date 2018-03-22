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

void appInit() {
	LED_t leds[NLEDS];
	uint32_t i = 0;

	ad5592rSetup(&hspi6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	buttonSetup();
	encSetup(&htim3, 0);
	encSetup(&htim4, 1);
	ledSetup(&hspi4);
	oledSetup(&hspi1);
	termSetup(&huart1);
	sdramSetup(&hsdram1);

	oledFillScreen(OLED_WHITE);
	for (i = 0; i < NLEDS; ++i) {
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

	oledClear();
	termPutString("\r--- testing peripherals ---\r");

	testSDCARD();
	for (i = 0; i<128; ++i) {
		char str[10] = {0};
		termPutString("\rRAM check no ");
		termPutString(hex2Str(i, 3, str));
		testSDRAM(&hsdram1);
	}

	ledSet(leds);
	termPutString("\r--- peripherals check done ---\r");

	oledPutString("press button to continue...", OLED_GREEN);
	while (!(buttonFalling(BUTTONENC0)||buttonFalling(BUTTONENC1)));

	oledClear();
	booting = 0;
	demoAD5592R(&hspi6);
}

void appGui() {
	TickType_t xLastWakeTime;
	LED_t ledColor, ledOff, led[NLEDS];
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

	for (i = 0; i < NLEDS; ++i) {
		led[i] = ledOff; //Color;
	}

	oledClear();
	oledCurSet(0, 10);
	oledPutString("Grumpy wizards\nmake toxic brew\nfor the evil\nQueen and Jack.", OLED_BLUE);

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
		oledCurSet(44,0);
		if (loopCnt%2 == 0) {
			oledPutChar('_', OLED_GREEN);
		} else {
			oledPutChar(' ', OLED_GREEN);
		}
		++loopCnt;
	}
}
