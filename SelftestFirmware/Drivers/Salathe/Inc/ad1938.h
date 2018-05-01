/*
 * ad1938.h
 *
 *  Created on: 29.04.2018
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

#ifndef _AD1938_H
#define _AD1938_H

#include "sai.h"

#include "main.h"
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

//global variables and defines
#define AD1938_1L	(0)	//index of stereo channel 1 left
#define AD1938_1R	(1)	//index of stereo channel 1 right
#define AD1938_2L	(2)	//index of stereo channel 2 left
#define AD1938_2R	(3)	//index of stereo channel 2 right
#define AD1938_3L	(4)	//index of stereo channel 3 left
#define AD1938_3R	(5)	//index of stereo channel 3 right
#define AD1938_4L	(6)	//index of stereo channel 4 left
#define AD1938_4R	(7)	//index of stereo channel 4 right

//type definitions
typedef int32_t ad1938SampleType;

typedef struct {
	uint8_t csIndex;					//which chip select line should be used?
	SPI_HandleTypeDef	*hspi;			//handle for SPI control port
	SAI_HandleTypeDef	*hsaiOut;		//handle for SAI port transmitting to DACs
	SAI_HandleTypeDef	*hsaiIn;		//handle for SAI port receiving from ADCs
	ad1938SampleType	*outBuf;		//on setup assign a buffer
	ad1938SampleType	*inBuf;			//on setup assign a buffer
	uint32_t			outBufSize;		//and note its size
	uint32_t			inBufSize;		//and note its size
//	SPI_InitTypeDef		spiInit;		//maybe not needed because the SPI config of the OLED may be compatible after all
	TaskHandle_t		taskToNotify;	//(gets updated by ad1938WaitOnBuffers)acquired from xTaskGetCurrentTaskHandle()
}ad1938_HandleTypeDef;


//function prototypes
//Initial setup of AD1938
void ad1938Setup(ad1938_HandleTypeDef *had1938_);

//set DAC volume (not suited for live modification because it is too slow and too coarse)
void ad1938SetVol(uint8_t iDac, uint8_t vol);

//Start streaming audio - ensure that outBuf is completely filled before calling this.
// NOTE: ad1938WaitOnBuffers won't return until after the first cycle!
void ad1938Start();

//Returns as soon as the buffers need new data
void ad1938WaitOnBuffers(
		ad1938SampleType **inBuf,	//returns pointer to readily filled input buffer
		uint32_t *inBufSize,			//returns size of that buffer
		ad1938SampleType **outBuf,	//returns pointer to output buffer that has to be filled
		uint32_t *outBufSize			//returns size of that buffer
);

//stop streaming audio
void ad1938Stop();

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);

#endif /*_AD1938_H*/
