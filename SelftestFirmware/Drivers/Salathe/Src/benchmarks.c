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
#define SDRAM_BENCHMARK_BUFFER_SIZE 2048 //in words (maximum buffer size is 2048 words)

//functions
uint64_t ad5592rBenchmark(SPI_HandleTypeDef *hspi, ad5592rPinMode_t pinMode) {
	uint64_t n;
	uint32_t tickCnt;

	//setup only the first AD5592R and return 0 if CHIP0 could not be initialized successfully
	if (!ad5592rSetup(hspi, AD5592R_CHIP0_ACTIVE)) return 0;

	//set pin modes and values
	for (n=0; n<8; ++n) {
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

uint64_t ad5592rBenchmarkGPO(SPI_HandleTypeDef *hspi) {
	return ad5592rBenchmark(hspi, ad5592rDigitalOut);
}

uint64_t ad5592rBenchmarkDAC(SPI_HandleTypeDef *hspi) {
	return ad5592rBenchmark(hspi, ad5592rAnalogOut);
}

uint64_t sdramBenchmarkWrite() {
	uint64_t n = 0;
	uint32_t tickCnt;
	uint32_t buf[SDRAM_BENCHMARK_BUFFER_SIZE];

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick());

	//set end of test period ([duration in s]*1000ticks/s)
	tickCnt += 1000*BENCHMARK_DURATION;

	//write to buf until the period has passed and increase a counter each time
	while (tickCnt > HAL_GetTick()) {
		sdramWrite((uint32_t *) SDRAM_ADDR, buf, SDRAM_BENCHMARK_BUFFER_SIZE);
		++n;
	}

	//calculate the measured transfer rate in bit/s
	return n*SDRAM_BENCHMARK_BUFFER_SIZE*(32/BENCHMARK_DURATION);
}

uint64_t sdramBenchmarkRead() {
	uint64_t n = 0;
	uint32_t tickCnt;
	uint32_t buf[SDRAM_BENCHMARK_BUFFER_SIZE];

	//wait for a system tick transition
	tickCnt = HAL_GetTick()+1;
	while (tickCnt > HAL_GetTick());

	//set end of test period ([duration in s]*1000ticks/s)
	tickCnt += 1000*BENCHMARK_DURATION;

	//read into buf until the period has passed and increase a counter each time
	while (tickCnt > HAL_GetTick()) {
		sdramRead((uint32_t *) SDRAM_ADDR, buf, SDRAM_BENCHMARK_BUFFER_SIZE);
		++n;
	}

	//calculate the measured transfer rate in bit/s
	return n*SDRAM_BENCHMARK_BUFFER_SIZE*(32/BENCHMARK_DURATION);
}

uint64_t sdCardBenchmarkWrite() {
	//check if an SD card is mounted - if not, try to mount it
	if (!SD_Rdy) //if the setup fails, return undone
		if (sdCardSetup()) return 0;

	return 0;
}

uint64_t sdCardBenchmarkRead() {
	//check if an SD card is mounted - if not, return
	if (!SD_Rdy) return 0;

	return 0;
}

