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
uint8_t muteOut[8] = {0};

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
//	encSetup(&htim3, 0);
//	encSetup(&htim4, 1);
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

	vTaskDelay(300);
	oledClear();

	//wake GUI task
	if(!appGuiHandle) vTaskDelay(1000);
	if(appGuiHandle) xTaskNotifyGive(appGuiHandle);
	else termPutString("ERROR: appInit can't see appGui and therefore won't wake it up!\r");
}

#define LOG_SMOOTH_N 256
#define LOG_SMOOTH_NS_PER_TICK 10
void logSmoothedRuntime(uint32_t ticks) { //a value of 0 prints the current average instead
	static __IO int lock = 0;
	static int i = 0;
	static uint32_t ticksBuf[LOG_SMOOTH_N] = {0};
	static uint64_t tickSum = 0;

	// thread safety
	if (lock) return;

	if (ticks) { //save new value
		ticksBuf[i%LOG_SMOOTH_N] = ticks;
		tickSum = tickSum
				- ticksBuf[(i+1)%LOG_SMOOTH_N]
				+ ticksBuf[i%LOG_SMOOTH_N];
		++i;
	} else { //print average
		char convBuf[20] = {0};
		uint64_t nsSmooth = 0;
		lock = 1;
		nsSmooth = tickSum * LOG_SMOOTH_NS_PER_TICK;
		nsSmooth /= LOG_SMOOTH_N;
		termPutString(uint2Str((nsSmooth/1000/1000/1000)%1000, 3, convBuf));
		termPutChar(',');
		termPutString(uint2Str((nsSmooth/1000/1000)%1000, 3, convBuf));
		termPutChar('.');
		termPutString(uint2Str((nsSmooth/1000)%1000, 3, convBuf));
		termPutChar(',');
		termPutString(uint2Str(nsSmooth%1000, 3, convBuf));
		termPutString("ms\n");
		lock = 0;
	}
}

void mixAudioFrameUI(ad1938SampleType *inSamples	//pointer to the four input samples of this frame
		,ad1938SampleType *outSamples				//pointer to the eight output samples of this frame
		,float32_t *dataGain						//pointer to the 8x4 gains
#ifdef AUDIO_RMS_CLIP_DISPLAY
		,uint32_t *clip								//pointer to the eight output clipping accumulators
		,float64_t *rms								//pointer to the eight output RMS level accumulators
#endif
		) {
	uint8_t iIn, iOut;

	//calculate output samples
	for (iOut = 0; iOut < 8; ++iOut) {
		int64_t sampleSum = 0;
		for (iIn = 0; iIn < 4; ++iIn) {
			//add input attenuated by control voltage
			sampleSum += (int64_t) inSamples[iIn] * ad5592rGetPin((ad5592rPin_t) {{iOut, iIn}});
		}
		//attenuate output with fader
		sampleSum *= potGetSmoothUI(iOut);
		//scale for max CV and max pot values
		sampleSum /= (4096*4096);
		//check for clipping
		if (sampleSum<INT32_MIN) {
			sampleSum = INT32_MIN;
#ifdef AUDIO_RMS_CLIP_DISPLAY
			++clip[iOut];
#endif
		} else if (INT32_MAX<sampleSum) {
			sampleSum = INT32_MAX;
#ifdef AUDIO_RMS_CLIP_DISPLAY
			++clip[iOut];
#endif
		}
		//prepare output sample
		outSamples[iOut] = sampleSum;
#ifdef AUDIO_RMS_CLIP_DISPLAY
		//add squared output sample to rms accumulator of output iOut
		rms[iOut] += (float64_t) sampleSum * sampleSum;
#endif
	}
}

void mixAudioFrameF(ad1938SampleType *inSamples	//pointer to the four input samples of this frame
		,ad1938SampleType *outSamples			//pointer to the eight output samples of this frame
		,float32_t *dataGain					//pointer to the 8x4 gains
#ifdef AUDIO_RMS_CLIP_DISPLAY
		,uint32_t *clip							//pointer to the eight output clipping accumulators
		,float64_t *rms							//pointer to the eight output RMS level accumulators
#endif
		) {
	uint8_t iIn, iOut;

	float64_t inSamplesF[4];

	for (iIn=0; iIn<4; ++iIn) {
		inSamplesF[iIn] = (float64_t) inSamples[iIn];
	}

	//calculate output samples
	for (iOut = 0; iOut < 8; ++iOut) {
		if (muteOut[iOut]) {
			outSamples[iOut] = 0;
		} else {
			float64_t sampleSum = 0;
			for (iIn = 0; iIn < 4; ++iIn) {
				//add input attenuated by control voltage
				sampleSum += inSamplesF[iIn] * dataGain[iIn*8+iOut];
			}
			//attenuate output with fader
			sampleSum *= potGetSmoothF(iOut);
			//check for clipping
			if (sampleSum<(float64_t)INT32_MIN) {
				sampleSum = (float64_t)INT32_MIN;
#ifdef AUDIO_RMS_CLIP_DISPLAY
				++clip[iOut];
#endif
			} else if ((float64_t)INT32_MAX<sampleSum) {
				sampleSum = (float64_t)INT32_MAX;
#ifdef AUDIO_RMS_CLIP_DISPLAY
				++clip[iOut];
#endif
			}
			//prepare output sample
			outSamples[iOut] = sampleSum;
#ifdef AUDIO_RMS_CLIP_DISPLAY
			//add squared output sample to rms accumulator of output iOut
			rms[iOut] += sampleSum * sampleSum;
#endif
		}
	}
}

void mixAudioFrameM(ad1938SampleType *inSamples	//pointer to the 4 input samples of this frame
		,ad1938SampleType *outSamples			//pointer to the 8 output samples of this frame
		,float32_t *dataGain					//pointer to the 8x4 gains
#ifdef AUDIO_RMS_CLIP_DISPLAY
		,uint32_t *clip							//pointer to the eight output clipping accumulators
		,float64_t *rms							//pointer to the eight output RMS level accumulators
#endif
		) {
	arm_matrix_instance_f32 matIn, matGain, matInter;
	float32_t dataIn[4], dataInter[8], dataOut[8], dataPot[8];
	q31_t potIn[8];

	matIn.numCols = 4;
	matIn.numRows = 1;
	matIn.pData = dataIn;
	matGain.numCols = 8;
	matGain.numRows = 4;
	matGain.pData = dataGain;
	matInter.numCols = 8;
	matInter.numRows = 1;
	matInter.pData = dataInter;

	arm_q31_to_float(inSamples, dataIn, 4);
	arm_mat_mult_f32(&matIn, &matGain, &matInter);

	arm_shift_q31((int32_t*) potSmoothed, 11, potIn, 8);
	arm_q31_to_float(potIn, dataPot, 8);
	arm_mult_f32(dataInter, dataPot, dataOut, 8);

	arm_float_to_q31(dataOut, outSamples, 8);
}

void appAudio() {
	int i;
	ad5592rPin_t pin;
#ifdef AUDIO_SHOW_RUNTIME
	__IO uint32_t *tim2Cnt = &(htim2.Instance->CNT);
	uint32_t usTimeMeas;
#endif


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
	ad5592rSetup(&hspi6, &htim6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	for (pin.number=0; pin.number<32; ++pin.number) {
		ad5592rSetPinMode(pin, ad5592rAnalogIn);
	}
	ad5592rUpdatePinModes();
	ad5592rUpdate();

	//initialize the transmission buffer with the first output interval
	for (i=0; i<AUDIO_BUF_SIZE*8; ++i) {
		(ad1938Handle.outBuf)[i] = 0;
	}

	//start the DMA transfers for audio streaming
	ad1938Start();
#ifdef AUDIO_SHOW_RUNTIME
	HAL_TIM_Base_Start(&htim2);
#endif

	//enter the sample calculation loop
	while(1) {
#ifdef AUDIO_RMS_CLIP_DISPLAY
		uint32_t clip[8] = {0};
		float64_t rms[8] = {0.0};
		led_t leds[8];
#endif
		int16_t iGains[32];
		float32_t fGains[32];

		//wait for new buffers to process
		ad1938WaitOnBuffers(&freshInBuf, &freshInBufSize, &freshOutBuf, &freshOutBufSize);
#ifdef AUDIO_SHOW_RUNTIME
		*tim2Cnt = 0;
#endif

		//update control voltages
		ad5592rUpdate();
		arm_shift_q15((q15_t *) ad5592rPinValsR, 3, iGains, 32);
		arm_q15_to_float(iGains, fGains, 32);

		//handle every sample
		for (i=0; i<freshOutBufSize/8; ++i) {
			mixAudioFrameF(freshInBuf, freshOutBuf, fGains
#ifdef AUDIO_RMS_CLIP_DISPLAY
					, clip, rms
#endif
					);
			//move buffer pointers one time frame forward
			freshInBuf+=4;
			freshOutBuf+=8;
		}

#ifdef AUDIO_RMS_CLIP_DISPLAY
		//handle LEDs
		for (i=0; i<8; ++i) {
			if (muteOut[i]){
				leds[i].green = 0;
				leds[i].red = 0;
				leds[i].blue = 64;
			} else {
				//calculate rms
				rms[i] = rms[i]/(freshOutBufSize/8);
				rms[i] = sqrt(rms[i]);
				//scale rms to 0...1
				rms[i] = rms[i]/INT32_MAX;
				//set LED value
				if (clip[i]) {
					leds[i].red = rms[i]*255;
					leds[i].green = 0;
				} else {
					leds[i].red = 0;
					leds[i].green = rms[i]*255;
				}
				leds[i].blue = 0;
			}
		}
		ledSet(leds);
#endif
#ifdef AUDIO_SHOW_RUNTIME
		//display run time measurement on terminal
		usTimeMeas = *tim2Cnt;
		logSmoothedRuntime(usTimeMeas);
#endif
	}

	//stop the DMA transfers to quit audio streaming
	ad1938Stop();
}

void appGui() {
	TickType_t xLastWakeTime;
	int32_t loopCnt = 0, i;

	//register own handle
	appGuiHandle = xTaskGetCurrentTaskHandle();

	//wait for appInit to complete
	while(!ulTaskNotifyTake(1, 10));

	//update GUI forever
	xLastWakeTime = xTaskGetTickCount();
	while(1) {
		// loop every 1ms
		vTaskDelayUntil(&xLastWakeTime, 1);

		//check buttons
		for (i=0; i<8; ++i) {
			if (buttonFalling(i)) {
				//mute output on falling edge, e.g. button press
				muteOut[i] = !muteOut[i];
			}
		}

		if (++loopCnt==100) {
			// update OLED every 100ms
			// count pot section (i=-1) and every ad5592r chip (i=0...3)
			for (i=-1; i<4; ++i) {
				int32_t j;
				// count every pin of pot or ad5592r chip
				for (j=0; j<8; ++j) {
					uint8_t lvl;
					// distinguish pots from ad5592r chips and get value scaled to 8bit
					if (i==-1) {
						lvl = potGetSmoothUI(j)>>4;
					} else {
						lvl = ad5592rPinValsR[i*8+j]>>4;
					}
					// calculate white color dimmed according to input level
					uint16_t color = oledColor565(lvl,lvl,lvl);
					// draw the corresponding rectangle
					oledDrawRectangel(j*12, (i<0)?(0):(16+i*12), 12, 12, 0, color);
				}
			}

#ifdef AUDIO_SHOW_RUNTIME
			// also update run time measurements
			logSmoothedRuntime(0);
#endif

			//reset counter
			loopCnt = 0;
		}
	}
}

