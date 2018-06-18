/*
 * benchmarks.c
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

#include "benchmarks.h"

#define BENCHMARK_DURATION 8 //in seconds (maximum duration is 32s, should be a power of 2)
#define BENCHMARK_SDRAM_BUFFER_SIZE 2048 //in words (maximum buffer size is 2048 words)
#define BENCHMARK_SDCARD_BYTES_PER_BUFFER 8192 //in bytes (maximum buffer size is 8192 words)
#define BENCHMARK_SDCARD_BUFFERS_PER_FILE 3000 //number of times the buffer gets written to one file (choose multiple of 1000)

//functions
uint64_t ad5592rBenchmark(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim, ad5592rPinMode_t pinMode) {
	//variable declarations
	uint64_t n;
	uint32_t tickCnt;

	//setup only the first AD5592R and return 0 if CHIP0 could not be initialized successfully
	if (!ad5592rSetup(hspi, htim, 0xf)) return 0;

	//set pin modes and values
	for (n=0; n<32; ++n) {
		ad5592rSetPinMode((ad5592rPin_t) (uint8_t) n, pinMode);
		ad5592rSetPin((ad5592rPin_t) (uint8_t) n, 0x800);
	}
	ad5592rUpdatePinModes();
	n = 0;

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick());

	//set end of test period ([duration in s]*1000ticks/s)
	tickCnt += 1000*BENCHMARK_DURATION;

	//update the ad5592r until the period has passed and increase a counter each time
	while (tickCnt > HAL_GetTick()) {
		ad5592rUpdate();
		++n;
	}

	//calculate the measured sample rate in sample/s
	return n/BENCHMARK_DURATION;
}

uint64_t ad5592rBenchmarkGPO(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim) {
	return ad5592rBenchmark(hspi, htim, ad5592rDigitalOut);
}

uint64_t ad5592rBenchmarkADC(SPI_HandleTypeDef *hspi, TIM_HandleTypeDef *htim) {
	return ad5592rBenchmark(hspi, htim, ad5592rAnalogIn);
}

uint64_t sdramBenchmarkWrite() {
	//variable declarations
	uint64_t n = 0;
	uint32_t tickCnt;
	uint32_t buf[BENCHMARK_SDRAM_BUFFER_SIZE];

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick());

	//set end of test period ([duration in s]*1000ticks/s)
	tickCnt += 1000*BENCHMARK_DURATION;

	//write to buf until the period has passed and increase a counter each time
	while (tickCnt > HAL_GetTick()) {
		sdramWrite(SDRAM_ADDR, buf, BENCHMARK_SDRAM_BUFFER_SIZE);
		++n;
	}

	//calculate the measured transfer rate in bit/s
	return n*BENCHMARK_SDRAM_BUFFER_SIZE*(32/BENCHMARK_DURATION);
}

uint64_t sdramBenchmarkRead() {
	//variable declarations
	uint64_t n = 0;
	uint32_t tickCnt;
	uint32_t buf[BENCHMARK_SDRAM_BUFFER_SIZE];

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick());

	//set end of test period ([duration in s]*1000ticks/s)
	tickCnt += 1000*BENCHMARK_DURATION;

	//read into buf until the period has passed and increase a counter each time
	while (tickCnt > HAL_GetTick()) {
		sdramRead(SDRAM_ADDR, buf, BENCHMARK_SDRAM_BUFFER_SIZE);
		++n;
	}

	//calculate the measured transfer rate in bit/s
	return n*BENCHMARK_SDRAM_BUFFER_SIZE*(32/BENCHMARK_DURATION);
}

uint64_t sdCardBenchmarkWriteFatFS() {
	//variable declarations
	int i;
	char dataBuf[BENCHMARK_SDCARD_BYTES_PER_BUFFER];
	FRESULT fRet;
	TCHAR fName[16] = {'s','d','S','p','e','e','d','T','e','s','t','.','t','x','t',0};
	FIL file;
	UINT bytesWritten;
	uint32_t tickCnt;

	//check if an SD card is mounted - if not, try to mount it
	if (!SD_Rdy) {//if the setup fails, return undone
		if (sdCardSetup()) {
			return 1;
		}
	}

	//open file
	fRet = f_open(&file, fName, FA_CREATE_ALWAYS | FA_WRITE);
	if (fRet != FR_OK) {
		f_close(&file);
		return 2;
	}

	//initialize with random characters
	for (i=0; i<BENCHMARK_SDCARD_BYTES_PER_BUFFER; ++i) {
		dataBuf[i] = (char) HAL_RNG_GetRandomNumber(&hrng)%('z'-'a')+'a';
	}

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick()); //tickCnt now contains the starting time of the test

	//run test
	for (i=0; i<BENCHMARK_SDCARD_BUFFERS_PER_FILE; ++i) {
		f_write(&file, dataBuf, BENCHMARK_SDCARD_BYTES_PER_BUFFER, &bytesWritten);
		if (bytesWritten != BENCHMARK_SDCARD_BYTES_PER_BUFFER) { //not the whole buffer got written -> abort
			f_close(&file);
			return 3;
		}
	}

	//collect time measurement
	tickCnt = HAL_GetTick() - tickCnt; //tickCnt now contains the time the write test took

	//clean up and prepare result
	f_close(&file);
	return (uint64_t) BENCHMARK_SDCARD_BYTES_PER_BUFFER*BENCHMARK_SDCARD_BUFFERS_PER_FILE*8*1000/tickCnt; //8 bits/byte and 1000 ticks/second
}

uint64_t sdCardBenchmarkWriteRaw() {
	//variable declarations
	int i;
	char dataBuf[BENCHMARK_SDCARD_BYTES_PER_BUFFER];
	uint32_t tickCnt;

	//initialize SD card
	if (SD_Driver.disk_initialize(0)) {
		return 1;
	}

	//initialize with random characters
	for (i=0; i<BENCHMARK_SDCARD_BYTES_PER_BUFFER; ++i) {
		dataBuf[i] = (char) HAL_RNG_GetRandomNumber(&hrng)%('z'-'a')+'a';
	}

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick()); //tickCnt now contains the starting time of the test

	//run test
	for (i=0; i<BENCHMARK_SDCARD_BUFFERS_PER_FILE; ++i) {
		SD_Driver.disk_write(0, (BYTE*) dataBuf, 0, BENCHMARK_SDCARD_BYTES_PER_BUFFER/BLOCKSIZE);
	}

	//collect time measurement
	tickCnt = HAL_GetTick() - tickCnt; //tickCnt now contains the time the write test took

	//prepare result
	return (uint64_t) BENCHMARK_SDCARD_BYTES_PER_BUFFER*BENCHMARK_SDCARD_BUFFERS_PER_FILE*8*1000/tickCnt; //8 bits/byte and 1000 ticks/second
}

uint64_t sdCardBenchmarkReadFatFS() {
	//variable declarations
	int i;
	char dataBuf[BENCHMARK_SDCARD_BYTES_PER_BUFFER];
	FRESULT fRet;
	TCHAR fName[16] = {'s','d','S','p','e','e','d','T','e','s','t','.','t','x','t',0};
	FIL file;
	UINT bytesRead;
	uint32_t tickCnt;

	//check if an SD card is mounted - if not, abort
	if (!SD_Rdy) {
		return 1;
	}

	//open file
	fRet = f_open(&file, fName, FA_READ);
	if (fRet != FR_OK) {
		f_close(&file);
		return 2;
	}

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick()); //tickCnt now contains the starting time of the test

	//run test
	for (i=0; i<BENCHMARK_SDCARD_BUFFERS_PER_FILE; ++i) {
		f_read(&file, dataBuf, BENCHMARK_SDCARD_BYTES_PER_BUFFER, &bytesRead);
		if (bytesRead != BENCHMARK_SDCARD_BYTES_PER_BUFFER) { //not the whole buffer got read -> abort
			f_close(&file);
			return 3;
		}
	}

	//collect time measurement
	tickCnt = HAL_GetTick() - tickCnt; //tickCnt now contains the time the write test took

	//clean up and prepare result
	f_close(&file);
	f_unlink(fName);
	return (uint64_t) BENCHMARK_SDCARD_BYTES_PER_BUFFER*BENCHMARK_SDCARD_BUFFERS_PER_FILE*8*1000/tickCnt; //8 bits/byte and 1000 ticks/second
}

uint64_t sdCardBenchmarkReadRaw() {
	//variable declarations
	int i;
	char dataBuf[BENCHMARK_SDCARD_BYTES_PER_BUFFER];
	uint32_t tickCnt;

	//initialize SD card
/*	if (SD_Driver.disk_initialize(0)) {
		return 1;
	}
*/
	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick()); //tickCnt now contains the starting time of the test

	//run test
	for (i=0; i<BENCHMARK_SDCARD_BUFFERS_PER_FILE; ++i) {
		SD_Driver.disk_read(0, (BYTE*) dataBuf, 0, BENCHMARK_SDCARD_BYTES_PER_BUFFER/BLOCKSIZE);
	}

	//collect time measurement
	tickCnt = HAL_GetTick() - tickCnt; //tickCnt now contains the time the write test took

	//prepare result
	return (uint64_t) BENCHMARK_SDCARD_BYTES_PER_BUFFER*BENCHMARK_SDCARD_BUFFERS_PER_FILE*8*1000/tickCnt; //8 bits/byte and 1000 ticks/second
}

