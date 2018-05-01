/*
 * application.c
 *
 *  Created on: 01.05.2018
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

//private global variables
TaskHandle_t appGuiHandle = 0;
TaskHandle_t appAudioHandle = 0;

//function implementations
void appInit() {
	led_t leds[LEDS_N];
	uint32_t i = 0;

	termSetup(&huart1);
	sdramSetup(&hsdram1);
	ledSetup(&hspi4);
	for (i = 0; i < LEDS_N; ++i) {
		leds[i].red = 0;
		leds[i].green = 0;
		leds[i].blue = 0;
	}
	ledSet(leds);
	oledSetup(&hspi1);
	oledFillScreen(OLED_WHITE);

	ad5592rSetup(&hspi6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	buttonSetup();
	encSetup(&htim3, 0);
	encSetup(&htim4, 1);
	potsSetup(&hadc1);

	termPutString("\r\rbooting... \r");
	termPutString(" --.      |        |   |\r");
	termPutString("|         |       -|-  |     /\r");
	termPutString("'--   --  |    --  |   |--   --.\r");
	termPutString("   | |  | |   |  | |   |  | |--'\r");
	termPutString("'--' '--: '-' '--: '-' '  ' '--\r");
	termPutString("platform for developing digital\r");
	termPutString("      synthesizer modules\r\r");
	termPutString("          SIMPLE MIXER    \r\r");

	vTaskDelay(100);
	oledClear();

	//wake GUI task
	if(!appGuiHandle) vTaskDelay(1000);
	if(appGuiHandle) xTaskNotifyGive(appAudioHandle);
	else termPutString("ERROR: appInit can't see appGui and therefore won't wake it up!\r");
}

void appAudio() {
	int iSamp;

	ad1938SampleType *freshInBuf;
	uint32_t freshInBufSize;
	ad1938SampleType *freshOutBuf;
	uint32_t freshOutBufSize;

	//register own handle
	appAudioHandle = xTaskGetCurrentTaskHandle();

	//prepare the ad1938 handle structure
	ad1938Handle.csIndex=0;
	ad1938Handle.hspi = &hspi1;
	ad1938Handle.hsaiIn = &hsai_BlockA1;
	ad1938Handle.hsaiOut = &hsai_BlockB1;
	ad1938Handle.inBuf = (ad1938SampleType*) SDRAM_ADDR;
	ad1938Handle.inBufSize = AUDIO_BUF_SIZE*4;
	ad1938Handle.outBuf = (ad1938SampleType*) SDRAM_ADDR+AUDIO_BUF_SIZE*4;
	ad1938Handle.outBufSize = AUDIO_BUF_SIZE*8;

	//initialize ad1938 and related peripherals
	ad1938Setup(&ad1938Handle);

	//initialize the transmission buffer with the first output interval
	for (iSamp=0; iSamp<AUDIO_BUF_SIZE*8; ++iSamp) {
		(ad1938Handle.outBuf)[iSamp] = 0;
	}

	//start the DMA transfers for audio streaming
	ad1938Start();

	//enter the sample calculation loop
	while(1) {
		//wait for new buffers to process
		ad1938WaitOnBuffers(&freshInBuf, &freshInBufSize, &freshOutBuf, &freshOutBufSize);

		//update control voltages
		ad5592rUpdate();

		//handle every sample
		for (iSamp=0; iSamp<freshOutBufSize/8; ++iSamp) {
			int iOut;
			for (iOut=0; iOut<8; ++iOut) {
				int32_t outSample = 0;
				int iIn;
				for (iIn=0; iIn<4; ++iIn) {
					outSample += freshInBuf[iSamp*4+iOut]/POTS_MAX_VAL*potGetSmoothUI(iOut);
				}
				freshOutBuf[iSamp*8+iOut] = outSample/2
			}
		}
		//calculate next set of samples
		//but do nothing in this demo - sine wave stays the same
	}

	//stop the DMA transfers to quit audio streaming
	ad1938Stop();
}

void appGui() {
	TickType_t xLastWakeTime;

	//register own handle
	appGuiHandle = xTaskGetCurrentTaskHandle();

	//wait for appInit to complete
	while(!ulTaskNotifyWait(1, 10));

	//prepare GUI

	//update GUI forever
	while(1) {
		vTaskDelayUntil(&xLastWakeTime, 100); //every 100ms
		;
	}
}

