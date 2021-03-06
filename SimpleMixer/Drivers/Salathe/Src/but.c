/*
 * but.c
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

#include "but.h"

//variable definitions
uint16_t buttonPins[BUTTONN] = {
		PUSH0_Pin,
		PUSH1_Pin,
		PUSH2_Pin,
		PUSH3_Pin,
		PUSH4_Pin,
		PUSH5_Pin,
		PUSH6_Pin,
		PUSH7_Pin,
		ROTARY0_PUSH_Pin,
		ROTARY1_PUSH_Pin
};
GPIO_TypeDef* buttonPorts[BUTTONN] = {
		PUSH0_GPIO_Port,
		PUSH1_GPIO_Port,
		PUSH2_GPIO_Port,
		PUSH3_GPIO_Port,
		PUSH4_GPIO_Port,
		PUSH5_GPIO_Port,
		PUSH6_GPIO_Port,
		PUSH7_GPIO_Port,
		ROTARY0_PUSH_GPIO_Port,
		ROTARY1_PUSH_GPIO_Port
};
uint16_t buttonState, buttonLastState, buttonRegisteredRising, buttonRegisteredFalling;

//functions
void buttonSetup() {
	buttonState = 0;
	buttonLastState = 0;
	buttonRegisteredRising = 0;
	buttonRegisteredFalling = 0;
}

void buttonPoll() {
	uint16_t mask = 1;
	buttonLastState = buttonState;
	buttonState = 0;
	int i;
	for (i = 0; i < BUTTONN; ++i) {
		if (HAL_GPIO_ReadPin(buttonPorts[i], buttonPins[i])) { //is set
			buttonState |= mask;
			if (!(buttonLastState & mask)) { //was reset => rising edge
				buttonRegisteredRising |= mask;
				buttonRegisteredFalling &= ~mask;
			}
		} else { //is reset
			if (buttonLastState & mask) { //was set => falling edge
				buttonRegisteredFalling |= mask;
				buttonRegisteredRising &= ~mask;
			}
		}
		mask <<= 1;
	}
}

uint8_t buttonRising(uint8_t buttonNumber) {
	uint16_t mask = 1;
	mask <<= buttonNumber;
	buttonPoll();
	if (buttonRegisteredRising & mask) {
		buttonRegisteredRising &= ~mask;
		return 1;
	}
	return 0;
}

uint8_t buttonFalling(uint8_t buttonNumber) {
	uint16_t mask = 1;
	mask <<= buttonNumber;
	buttonPoll();
	if (buttonRegisteredFalling & mask) {
		buttonRegisteredFalling &= ~mask;
		return 1;
	}
	return 0;
}

uint8_t buttonRead(uint8_t buttonNumber) {
	buttonPoll();
	return 1 & (buttonState >> buttonNumber);
}
