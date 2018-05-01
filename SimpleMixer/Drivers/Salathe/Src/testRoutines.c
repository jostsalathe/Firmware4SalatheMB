/*
 * testRoutines.c
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

#include "testRoutines.h"

//defines
#define TEST_RAM_TYPE uint32_t

#define TEST_RAM_RAND 0 //write random values to RAM
#define TEST_RAM_ALTER 1 //write alternating bit pattern to RAM
#define TEST_RAM_PATTERN TEST_RAM_RAND //pattern to use

#define TEST_RAM_FULL //check whole RAM instead of only one chunk of random addresses
#define TEST_RAM_CHUNK_SIZE 0x2000 //variables of TEST_TYPE (0x2000 byte for one row)
#define TEST_RAM_END SDRAM_SIZE //end the test right before this address offset

//#define TEST_RAM_LOG_ALL //write all tested addresses into log file not only those who failed
#define TEST_RAM_PRGR_INTER_MASK 0xFF //interval mask for progress display

#define TEST_AD5592R_MAX_DELTA 0x2A //maximum deviation of read ADC value from written DAC value

//functions
void logSDRAMline(FIL *logFile, uint32_t addr, uint32_t dataW, uint32_t dataR) {
#ifndef TEST_RAM_LOG_ALL
	if (dataW != dataR) {
#endif
	char hexBuf[9] = {0};
	logStr(logFile, "0x");
	logStr(logFile, hex2Str(addr, 8, hexBuf));
	logStr(logFile, ": 0x");
	logStr(logFile, hex2Str(dataW, 8, hexBuf));
	logStr(logFile, "->0x");
	logStr(logFile, hex2Str(dataR, 8, hexBuf));
#ifdef TEST_RAM_LOG_ALL
	if (dataW != dataR) {
#endif
	logStr(logFile, " ERROR");
#ifdef TEST_RAM_LOG_ALL
	}
#endif
	logStr(logFile, "\r");
#ifndef TEST_RAM_LOG_ALL
	}
#endif
}

int addrArrayContains(TEST_RAM_TYPE **arr, int size, TEST_RAM_TYPE *val) {
	int i, cnt=0;
	for (i=0; i<size; ++i) {
		if (arr[i] == val) {
			++cnt;
		}
	}
	return cnt;
}

int testSDRAM(SDRAM_HandleTypeDef *hsdram) {
	uint32_t err = 0, tested = 0, i = 0;
	led_t off = {0x0,0x0,0x0};
	led_t on = {0x0,0x3F,0x0};
#ifdef TEST_RAM_FULL
	TEST_RAM_TYPE *addr;
#else
	TEST_RAM_TYPE **addrs;
#endif
	TEST_RAM_TYPE *dataW;
	TEST_RAM_TYPE *dataR;
	FIL *logFile;
	char hexBuf[9] = {'\0'};

	dataW = malloc(sizeof(TEST_RAM_TYPE) * TEST_RAM_CHUNK_SIZE);
	dataR = malloc(sizeof(TEST_RAM_TYPE) * TEST_RAM_CHUNK_SIZE);
#ifndef TEST_RAM_FULL
	addrs = malloc(sizeof(TEST_RAM_TYPE *) * TEST_RAM_CHUNK_SIZE);
#endif
	logFile = malloc(sizeof(FIL));
	oledPutString("SDRAM: ", OLED_GREEN);
	termPutString("\r-- testing SDRAM --\r");
	if (!dataW || !dataR
#ifndef TEST_RAM_FULL
			|| !addrs
#endif
			|| !logFile) {
		oledPutString("malloc\n failed", OLED_RED);
		termPutString("ERROR: malloc failed!\rEntering Error handler...");
		_Error_Handler(__FILE__, __LINE__);
	}

	if (SD_Rdy) {
		FRESULT fRet;
		TCHAR fName[12] = {'R','A','M','t','e','s','t','.','l','o','g',0};
		fRet = f_open(logFile, fName, FA_CREATE_ALWAYS | FA_WRITE);
		if (fRet != FR_OK) {
			f_close(logFile);
			free(logFile);
			logFile = 0;
		}
	} else {
		free(logFile);
		logFile = 0;
	}

	ledProgress(0.0, on, off);
	oledProgress(0.0, OLED_GREEN);
#ifdef TEST_RAM_FULL
	for (addr = SDRAM_ADDR; addr+TEST_RAM_CHUNK_SIZE <= (TEST_RAM_TYPE *) (TEST_RAM_END+(uint32_t)SDRAM_ADDR); addr += TEST_RAM_CHUNK_SIZE) {
		ledProgress((float) ((uint32_t)addr-(uint32_t)SDRAM_ADDR)/(TEST_RAM_END), on, off);
		oledProgress((float) ((uint32_t)addr-(uint32_t)SDRAM_ADDR)/(TEST_RAM_END), OLED_GREEN);
#ifdef TEST_RAM_LOG_ALL
		termPutString("offset 0x");
		termPutString(hex2Str((uint32_t) addr, 8, hexBuf));
		termPutString("\r");
#endif
#endif
		for (i = 0; i < TEST_RAM_CHUNK_SIZE; ++i) {
			uint64_t randNum = 0;
#ifndef TEST_RAM_FULL
			do {
				HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) &randNum);
				randNum *= SDRAM_SIZE-1;
				randNum /= 4294967295;
				randNum &= ~0b11;
				randNum += (uint32_t) SDRAM_ADDR;
			} while(addrArrayContains(addrs, i+1, (TEST_RAM_TYPE *) (uint32_t) randNum));
			addrs[i] = (TEST_RAM_TYPE *) (uint32_t) randNum;
#endif
#if TEST_RAM_PATTERN == TEST_RAM_RAND
			HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) &randNum);
			dataW[i] = (TEST_RAM_TYPE) randNum;
#elif TEST_RAM_PATTERN == TEST_RAM_ALTER
			dataW[i] = i%2?0x55555555:0xAAAAAAAA;
#else
			dataW[i] = i;
#endif
		}
#ifdef TEST_RAM_FULL
		while (sdramWrite(addr, dataW, TEST_RAM_CHUNK_SIZE));
		while (sdramRead(addr, dataR, TEST_RAM_CHUNK_SIZE));
#else
		ledProgress(0.125, on, off);
		oledProgress(0.125, OLED_GREEN);
		for (i = 0; i < TEST_RAM_CHUNK_SIZE; ++i) {
			sdramWrite(addrs[i], dataW+i, 1);
		}
		for (i = 0; i < TEST_RAM_CHUNK_SIZE; ++i) {
			sdramRead(addrs[i], dataR+i, 1);
		}
		ledProgress(0.25, on, off);
		oledProgress(0.25, OLED_GREEN);
#endif
		for (i = 0; i < TEST_RAM_CHUNK_SIZE; ++i) {
#ifndef TEST_RAM_FULL
			if ((i&TEST_RAM_PRGR_INTER_MASK)==0) {
				float progress = 0.75*i/TEST_RAM_CHUNK_SIZE+0.25;
				ledProgress(progress, on, off);
				oledProgress(progress, OLED_GREEN);
			}
			logSDRAMline(logFile, (uint32_t) addrs[i], (uint32_t) dataW[i], (uint32_t) dataR[i]);
#else
			logSDRAMline(logFile, (uint32_t) (addr + i), (uint32_t) dataW[i], (uint32_t) dataR[i]);
#endif
			if (dataW[i] != dataR[i]) {
				++err;
			}
		}
		tested += TEST_RAM_CHUNK_SIZE;
#ifdef TEST_RAM_FULL
	}
#endif
	ledProgress(1.0, on, off);
	oledProgress(1.0, OLED_GREEN);
	termPutString(" encountered 0x");
	termPutString(hex2Str(err, 8, hexBuf));
	termPutString(" errors on 0x");
	termPutString(hex2Str(tested, 8, hexBuf));
	termPutString(" checks\r");
	termPutString("-- SDRAM check done --\r");

	if (err) {
		oledPutString("not okay:\n ", OLED_RED);
		oledPutString(hex2Str(err, 6, hexBuf), OLED_RED);
		oledPutString("h/", OLED_RED);
		oledPutString(hex2Str(tested, 6, hexBuf), OLED_RED);
		oledPutString("h\n", OLED_RED);
	} else {
		oledPutString("okay\n", OLED_GREEN);
	}

#ifndef TEST_RAM_FULL
	free(addrs);
#endif
	free(dataW);
	free(dataR);
	if (logFile) {
		f_close(logFile);
		free(logFile);
	}
	return (int) err;
}

int testSDCARD() {
	oledPutString("SDCARD: ", OLED_GREEN);
	termPutString("\r-- testing SD-Card --\r");
	int res = 0; //result code

	retSD = BSP_SD_Init();
	if (retSD == MSD_ERROR_SD_NOT_PRESENT) {
		oledPutString("notFound\n", OLED_RED);
		termPutString("no SD-Card in the slot\r");
		res = 1;
	} else if (retSD != MSD_OK) {
		oledPutString("not okay\n", OLED_RED);
		termPutString("encountered an error during initialization of SD-Card\r");
		res = 2;
	} else {
		FRESULT fatRes = FR_OK;
		oledPutString("okay\n mnt to ", OLED_GREEN);
		oledPutString(SDPath, OLED_GREEN);
		oledPutString(" ", OLED_GREEN);

		termPutString("found and initialized SD-Card\rmount SD-CARD to \"");
		termPutString(SDPath);
		termPutString("\":\r");

		fatRes = f_mount(&SDFatFS, (const TCHAR*) SDPath, 1);
		if (fatRes != FR_OK) {
			termReportFSfail(fatRes);
			res = 3;
		} else {
			TCHAR fileName[9] = {'t','e','s','t','.','t','x','t',0};
			FIL testFile;
			termPutString(" success\rtry opening test.txt for writing operation\r");

			fatRes = f_open(&testFile, fileName, FA_CREATE_ALWAYS | FA_WRITE);
			if (fatRes != FR_OK) {
				termReportFSfail(fatRes);
				res = 4;
			} else {
				uint8_t wText[12] = "Hello world";
				UINT bytesWritten = 0;
				termPutString(" success\rtry writing \"");
				termPutString((char *)wText);
				termPutString("\" to it\r");

				fatRes = f_write(&testFile, wText, 11, &bytesWritten);
				f_close(&testFile);
				if (fatRes != FR_OK) {
					termReportFSfail(fatRes);
					res = 5;
				} else {
					termPutString(" success - closing file\rtry opening test.txt for reading operation\r");

					fatRes = f_open(&testFile, fileName, FA_READ);
					if (fatRes != FR_OK) {
						termReportFSfail(fatRes);
						res = 6;
					} else {
						uint8_t rText[255] = {0};
						UINT bytesRead = 0;
						termPutString(" success\rtry reading from it\r");

						fatRes = f_read(&testFile, rText, 255, &bytesRead);
						if (fatRes != FR_OK) {
							termReportFSfail(fatRes);
							res = 7;
						} else {
							int i, mismatch = 0;
							for (i = 0; i < bytesWritten; ++i)
								if (rText[i] != wText[i])
									++mismatch;
							if (mismatch) {
								termPutString(" fail - text mismatch");
								res = 8;
							} else {
								SD_Rdy = 1;
								termPutString(" success");
							}
							termPutString(" - read: \"");
							termPutString((char*) rText);
							termPutString("\"\r");
						}
						f_close(&testFile);
					}
				}
			}
			termPutString("try deleting test.txt\r");
			fatRes = f_unlink(fileName);
			if (fatRes == FR_NO_FILE || fatRes == FR_NO_PATH) {
				termPutString(" no file to delete\r");
				res = 9;
			} else if (fatRes != FR_OK) {
				termReportFSfail(fatRes);
				res = 10;
			} else {
				termPutString(" success\r");
			}
		}
	}
	termPutString("-- SD-CARD check done --\r");
	if (SD_Rdy) {
		oledPutString("okay\n", OLED_GREEN);
	} else {
		oledPutString("fail\n", OLED_RED);
	}
	return res;
}

int testAD5592R(SPI_HandleTypeDef *hspi) {
	ad5592rPin_t pin;
	char str[11] = {0};
	uint8_t chipStatus; // [0:3]=chipActive, [4:7]=chipIOfault
	chipStatus = AD5592R_CHIP0_ACTIVE
			| AD5592R_CHIP1_ACTIVE
			| AD5592R_CHIP2_ACTIVE
			| AD5592R_CHIP3_ACTIVE;

	oledPutString("AD5592R: ", OLED_GREEN);
	termPutString("\r-- testing AD5592Rs --\rSPI response test yields:\r");

	chipStatus = ad5592rSetup(hspi, chipStatus);
	for (pin.number = 0; pin.number < 32; pin.number += 8) {
		termPutString(" AD5592R chip ");
		termPutString(uint2Str(pin.chip, 1, str));
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString(": present and responding correctly\r");
		} else { //chip inactive
			termPutString(": not present or not responding correctly\r");
		}
	}

	termPutString("\rChecking pin functions of active AD5592Rs:\r pin number   ");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString("|");
			termPutString(uint2Str(pin.chip, 1, str));
			termPutString(".");
			termPutString(uint2Str(pin.pin, 1, str));
		}
	}
	termPutString("\r -------------");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString("+---");
		}
	}
	termPutString("\r");

	//set pin config for digital IO
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPinMode(pin, ad5592rDigitalOut);
		}
	}
	ad5592rUpdatePinModes();

	//write pins HIGH
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPin(pin, 1);
		}
	}
	ad5592rUpdate();
	//read pins and compare to written
	ad5592rUpdate();
	termPutString(" digital high");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString(" | ");
			if (ad5592rGetPin(pin)==1) { //good
				termPutString("1");
			} else { //bad
				termPutString("0");
				chipStatus |= 1<<(4+pin.chip);
			}
		}
	}
	termPutString("\r");

	//write pins LOW
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPin(pin, 0);
		}
	}
	ad5592rUpdate();
	//read pins and compare to written
	ad5592rUpdate();
	termPutString(" digital low ");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString(" | ");
			if (ad5592rGetPin(pin)==0) { //good
				termPutString("0");
			} else { //bad
				termPutString("1");
				chipStatus |= 1<<(4+pin.chip);
			}
		}
	}
	termPutString("\r");

	//set pin config for analog IO
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPinMode(pin, ad5592rAnalogInOut);
		}
	}
	ad5592rUpdatePinModes();

	//write DACs max
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPin(pin, 0xFFF);
		}
	}
	ad5592rUpdate();
	//read ADCs and compare to written DAC
	ad5592rUpdate();
	termPutString(" analog max   ");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString("|");
			termPutString(hex2Str(ad5592rGetPin(pin), 3, str));
			if (ad5592rGetPin(pin)<(0xFFF-TEST_AD5592R_MAX_DELTA)) { //bad
				chipStatus |= 1<<(4+pin.chip);
			}
		}
	}
	termPutString("\r");

	//write DACs min
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPin(pin, 0x0);
		}
	}
	ad5592rUpdate();
	//read ADCs and compare to written DAC
	ad5592rUpdate();
	termPutString(" analog min   ");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString("|");
			termPutString(hex2Str(ad5592rGetPin(pin), 3, str));
			if (ad5592rGetPin(pin)>(0x0+TEST_AD5592R_MAX_DELTA)) { //bad
				chipStatus |= 1<<(4+pin.chip);
			}
		}
	}
	termPutString("\r");

	//write DACs mid
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			ad5592rSetPin(pin, 0x800);
		}
	}
	ad5592rUpdate();
	//read ADCs and compare to written DAC
	ad5592rUpdate();
	termPutString(" analog mid   ");
	for (pin.number = 0; pin.number < 32; ++pin.number) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString("|");
			termPutString(hex2Str(ad5592rGetPin(pin), 3, str));
			if (ad5592rGetPin(pin)<(0x800-TEST_AD5592R_MAX_DELTA)) { //too low
				chipStatus |= 1<<(4+pin.chip);
			} else if (ad5592rGetPin(pin)>(0x800+TEST_AD5592R_MAX_DELTA)) { //too high
				chipStatus |= 1<<(4+pin.chip);
			}
		}
	}
	termPutString("\r");

	//evaluate results
	termPutString("\rBased on a maximum deviation of +-");
	termPutString(hex2Str(TEST_AD5592R_MAX_DELTA, 3, str));
	termPutString("h between written DAC value\rand read ADC value, the active AD5592Rs performed as follows:\r");
	for (pin.number = 0; pin.number < 32; pin.number += 8) {
		if (AD5592R_CHIP_ACTIVE(pin.chip)) { //chip active
			termPutString(" AD5592R chip ");
			termPutString(uint2Str(pin.chip, 1, str));
			termPutString(": ");
			if ((1 << (4 + pin.chip)) & chipStatus) { //IO fault
				termPutString("at least one IO deviates too much\r");
				oledPutString("f", OLED_RED);
			} else { //IO okay
				termPutString("all IOs perform inside given tolerance\r");
				oledPutString("o", OLED_GREEN);
			}
		} else { //chip inactive
			oledPutString("n", oledColor565(0x33,0x33,0x0));
		}
	}

	//reset pin modes to input with pull down
	for (pin.number = 0; pin.number < 32; pin.number += 8) {
		ad5592rSetPinMode(pin, ad5592rDigitalInPullDown);
		ad5592rSetPin(pin, 0);
	}
	ad5592rUpdate();
	ad5592rUpdatePinModes();

	oledPutString("\n", OLED_GREEN);
	termPutString("-- AD5592Rs check done --\r");
	return (int) chipStatus;
}

void demoAD5592R(SPI_HandleTypeDef *hspi) {
	TickType_t xLastWakeTime;
	ad5592rPin_t pin;

	ad5592rSetup(hspi, AD5592R_CHIP0_ACTIVE);
	pin.number = 0; //sine output
	ad5592rSetPinMode(pin, ad5592rAnalogOut);
	pin.number = 1; //analog test input
	ad5592rSetPinMode(pin, ad5592rAnalogIn);
	pin.number = 2; //mirror of pin 1
	ad5592rSetPinMode(pin, ad5592rAnalogOut);
	pin.number = 3; //square wave
	ad5592rSetPinMode(pin, ad5592rDigitalOut);
	pin.number = 4; //square wave
	ad5592rSetPinMode(pin, ad5592rDigitalOutOpenDrain);
	pin.number = 5; //digital test input
	ad5592rSetPinMode(pin, ad5592rDigitalIn);
	pin.number = 6; //mirror of pin 5 (push/pull)
	ad5592rSetPinMode(pin, ad5592rDigitalOut);
	pin.number = 7; //mirror of pin 5 (open drain)
	ad5592rSetPinMode(pin, ad5592rDigitalOutOpenDrain);
	ad5592rUpdatePinModes();
	int i = 0;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		uint16_t input;
		pin.number = 0; //sine output
		ad5592rSetPin(pin, ad5592rSine[i]);
		pin.number = 1; //analog test input
		input = ad5592rGetPin(pin);
		pin.number = 2; //mirror of pin 1
		ad5592rSetPin(pin, input);
		pin.number = 3; //fast square wave
		ad5592rSetPin(pin, i&1);
		pin.number = 4; //slow square wave
		ad5592rSetPin(pin, i>=500);
		pin.number = 5; //digital test input
		input = ad5592rGetPin(pin);
		pin.number = 6; //mirror of pin 5 (push/pull)
		ad5592rSetPin(pin, input);
		pin.number = 7; //mirror of pin 5 (open drain)
		ad5592rSetPin(pin, input);
		if ((++i) >= 1000) {
			i = 0;
		}
		ad5592rUpdate();
		vTaskDelayUntil(&xLastWakeTime, 1);
	}
}

