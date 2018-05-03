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

	vTaskDelay(1000);
	oledClear();

	//wake GUI task
	if(!appGuiHandle) vTaskDelay(1000);
	if(appGuiHandle) xTaskNotifyGive(appGuiHandle);
	else termPutString("ERROR: appInit can't see appGui and therefore won't wake it up!\r");
}

void appAudio() {
	int iSamp;
	ad5592rPin_t pin;
	char convBuf[20] = {0};

	double rms[8] = {0.0};
	led_t leds[8];

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
	ad5592rSetup(&hspi6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	for (pin.number=0; pin.number<32; ++pin.number) {
		ad5592rSetPinMode(pin, ad5592rAnalogIn);
	}
	ad5592rUpdatePinModes();
	ad5592rUpdate();

	//initialize the transmission buffer with the first output interval
	for (iSamp=0; iSamp<AUDIO_BUF_SIZE*8; ++iSamp) {
		(ad1938Handle.outBuf)[iSamp] = 0;
	}

	//start the DMA transfers for audio streaming
	ad1938Start();
	HAL_TIM_Base_Start(&htim2);

	//enter the sample calculation loop
	while(1) {
		uint32_t clip[8] = {0};
		//wait for new buffers to process
		ad1938WaitOnBuffers(&freshInBuf, &freshInBufSize, &freshOutBuf, &freshOutBufSize);
		htim2.Instance->CNT = 0;

		//update control voltages
		ad5592rUpdate(); //1.2ms
		pin.number = 0;

		//handle every sample
		for (iSamp=0; iSamp<freshOutBufSize/8; ++iSamp) {
			int iIn, iOut;
			int64_t inSample[4];

			//get input samples for t=iSamp
			for (iIn=0;  iIn<4; ++iIn) {
				inSample[iIn] = freshInBuf[iSamp*4+iIn];
			}

			//calculate output samples
			for (iOut = 0; iOut < 4; ++iOut) {
				int64_t sampleSum = 0;
				pin.pin = iOut;
				for (iIn = 0; iIn < 4; ++iIn) {
					pin.chip = iIn;
					//add input attenuated by control voltage
					sampleSum += inSample[iIn]*ad5592rGetPin(pin); //13.3ms @2chOut*4chIn*2400sample
//					sampleSum += inSample[iIn];
				}
				//attenuate output with fader
				sampleSum /= (4095*4095);
				sampleSum *= potGetSmoothUI(iOut); //3.8ms @2ch*2400sample
				//check for clipping
				if (sampleSum<INT32_MIN) { //1.5ms @2ch*2400sample
					sampleSum = INT32_MIN;
					++clip[iOut];
				} else if (INT32_MAX<sampleSum) {
					sampleSum = INT32_MAX;
					++clip[iOut];
				}
				//prepare output sample
				freshOutBuf[iSamp * 8 + iOut] = sampleSum;
				//add squared output sample to rms accumulator of output iOut
				rms[iOut] += (double) sampleSum*sampleSum; //0.9ms @2ch*2400sample
			}
		}

		for (iSamp=0; iSamp<8; ++iSamp) { //0.4ms
			//calculate rms
			rms[iSamp] = rms[iSamp]/(freshOutBufSize/8);
			rms[iSamp] = sqrt(rms[iSamp]);
			//scale rms to 0...1
			rms[iSamp] = rms[iSamp]/INT32_MAX;
			//set LED value
			leds[iSamp].green = rms[iSamp]*255;
			leds[iSamp].red = (clip[iSamp])?255:0;
			leds[iSamp].blue = 0;
		}
		ledSet(leds);
		termPutString(uint2Str(htim2.Instance->CNT, 8, convBuf));
		termPutChar('\n');
	}

	//stop the DMA transfers to quit audio streaming
	ad1938Stop();
}

void appGui() {
	TickType_t xLastWakeTime;
	char convBuf[20] = {0};

	//register own handle
	appGuiHandle = xTaskGetCurrentTaskHandle();

	//wait for appInit to complete
	while(!ulTaskNotifyTake(1, 10));

	//prepare GUI

	//update GUI forever
	xLastWakeTime = xTaskGetTickCount();
	while(1) {
		vTaskDelayUntil(&xLastWakeTime, 1000); //every 100ms
		oledCurSet(0,0);
		oledPutString(uint2Str(ad5592rGetPin((ad5592rPin_t)(uint8_t)0), 4, convBuf),OLED_BLUE);
		oledPutString("  ", OLED_BLUE);
		oledPutString(uint2Str(ad5592rGetPin((ad5592rPin_t)(uint8_t)1), 4, convBuf),OLED_BLUE);
	}
}

