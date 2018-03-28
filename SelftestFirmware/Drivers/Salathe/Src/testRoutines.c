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
#define TEST_TYPE uint32_t

#define RAM_TEST_RAND 0
#define RAM_TEST_ALTER 1
#define RAM_TESTPATTERN RAM_TEST_RAND

#define RAM_TEST_FULL
#define RAM_TEST_CHUNK_SIZE 0x2000 //variables of TEST_TYPE (0x2000 byte for one row)
#define RAM_TEST_END SDRAM_SIZE //end the test right before this address offset

//#define RAM_TEST_LOG_ALL
#define RAM_TEST_PRGR_INTER_MASK 0xFF

//functions
void logSDRAMline(FIL *logFile, uint32_t addr, uint32_t dataW, uint32_t dataR) {
#ifndef RAM_TEST_LOG_ALL
	if (dataW != dataR) {
#endif
	char hexBuf[9] = {0};
	logStr(logFile, "0x");
	logStr(logFile, hex2Str(addr, 8, hexBuf));
	logStr(logFile, ": 0x");
	logStr(logFile, hex2Str(dataW, 8, hexBuf));
	logStr(logFile, "->0x");
	logStr(logFile, hex2Str(dataR, 8, hexBuf));
#ifdef RAM_TEST_LOG_ALL
	if (dataW != dataR) {
#endif
	logStr(logFile, " ERROR");
#ifdef RAM_TEST_LOG_ALL
	}
#endif
	logStr(logFile, "\r");
#ifndef RAM_TEST_LOG_ALL
	}
#endif
}

int addrArrayContains(TEST_TYPE **arr, int size, TEST_TYPE *val) {
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
	LED_t off = {0x0,0x0,0x0};
	LED_t on = {0x0,0x3F,0x0};
#ifdef RAM_TEST_FULL
	TEST_TYPE *addr;
#else
	TEST_TYPE **addrs;
#endif
	TEST_TYPE *dataW;
	TEST_TYPE *dataR;
	FIL *logFile;
	char hexBuf[9] = {'\0'};

	dataW = malloc(sizeof(TEST_TYPE) * RAM_TEST_CHUNK_SIZE);
	dataR = malloc(sizeof(TEST_TYPE) * RAM_TEST_CHUNK_SIZE);
#ifndef RAM_TEST_FULL
	addrs = malloc(sizeof(TEST_TYPE *) * RAM_TEST_CHUNK_SIZE);
#endif
	logFile = malloc(sizeof(FIL));
	oledPutString("SDRAM: ", OLED_GREEN);
	termPutString("\r-- testing SDRAM --\r");
	if (!dataW || !dataR
#ifndef RAM_TEST_FULL
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
#ifdef RAM_TEST_FULL
	for (addr = (TEST_TYPE *) SDRAM_ADDR; addr+RAM_TEST_CHUNK_SIZE <= (TEST_TYPE *) (RAM_TEST_END+SDRAM_ADDR); addr += RAM_TEST_CHUNK_SIZE) {
		ledProgress((float) ((uint32_t)addr-SDRAM_ADDR)/(RAM_TEST_END), on, off);
		oledProgress((float) ((uint32_t)addr-SDRAM_ADDR)/(RAM_TEST_END), OLED_GREEN);
#ifdef RAM_TEST_LOG_ALL
		termPutString("offset 0x");
		termPutString(hex2Str((uint32_t) addr, 8, hexBuf));
		termPutString("\r");
#endif
#endif
		for (i = 0; i < RAM_TEST_CHUNK_SIZE; ++i) {
			uint64_t randNum = 0;
#ifndef RAM_TEST_FULL
			do {
				HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) &randNum);
				randNum *= SDRAM_SIZE-1;
				randNum /= 4294967295;
				randNum &= ~0b11;
				randNum += SDRAM_ADDR;
			} while(addrArrayContains(addrs, i+1, (TEST_TYPE *) (uint32_t) randNum));
			addrs[i] = (TEST_TYPE *) (uint32_t) randNum;
#endif
#if RAM_TESTPATTERN == RAM_TEST_RAND
			HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) &randNum);
			dataW[i] = (TEST_TYPE) randNum;
#elif RAM_TESTPATTERN == RAM_TEST_ALTER
			dataW[i] = i%2?0x55555555:0xAAAAAAAA;
#else
			dataW[i] = i;
#endif
		}
#ifdef RAM_TEST_FULL
		while (HAL_SDRAM_Write_32b(hsdram, addr, dataW, RAM_TEST_CHUNK_SIZE));
		while (HAL_SDRAM_Read_32b(hsdram, addr, dataR, RAM_TEST_CHUNK_SIZE));
#else
		ledProgress(0.125, on, off);
		oledProgress(0.125, OLED_GREEN);
		for (i = 0; i < RAM_TEST_CHUNK_SIZE; ++i) {
			HAL_SDRAM_Write_32b(hsdram, addrs[i], dataW+i, 1);
		}
		for (i = 0; i < RAM_TEST_CHUNK_SIZE; ++i) {
			HAL_SDRAM_Read_32b(hsdram, addrs[i], dataR+i, 1);
		}
		ledProgress(0.25, on, off);
		oledProgress(0.25, OLED_GREEN);
#endif
		for (i = 0; i < RAM_TEST_CHUNK_SIZE; ++i) {
#ifndef RAM_TEST_FULL
			if ((i&RAM_TEST_PRGR_INTER_MASK)==0) {
				float progress = 0.75*i/RAM_TEST_CHUNK_SIZE+0.25;
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
		tested += RAM_TEST_CHUNK_SIZE;
#ifdef RAM_TEST_FULL
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

#ifndef RAM_TEST_FULL
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
		oledPutString("fail\n", OLED_GREEN);
	}
	return res;
}

int testAD5592R(SPI_HandleTypeDef *hspi) {
	int res = 0; //result code
	return res;
}
void demoAD5592R(SPI_HandleTypeDef *hspi) {
	TickType_t xLastWakeTime;
	ad5592rPin_t pin;

	ad5592rSetup(hspi, AD5592R_CHIP0_ACTIVE);
	pin.number = 0; //sine output
	ad5592rSelectPinMode(pin, ad5592rAnalogOut);
	pin.number = 1; //analog test input
	ad5592rSelectPinMode(pin, ad5592rAnalogIn);
	pin.number = 2; //mirror of pin 1
	ad5592rSelectPinMode(pin, ad5592rAnalogOut);
	pin.number = 3; //square wave
	ad5592rSelectPinMode(pin, ad5592rDigitalOut);
	pin.number = 4; //square wave
	ad5592rSelectPinMode(pin, ad5592rDigitalOutOpenDrain);
	pin.number = 5; //digital test input
	ad5592rSelectPinMode(pin, ad5592rDigitalIn);
	pin.number = 6; //mirror of pin 5 (push/pull)
	ad5592rSelectPinMode(pin, ad5592rDigitalOut);
	pin.number = 7; //mirror of pin 5 (open drain)
	ad5592rSelectPinMode(pin, ad5592rDigitalOutOpenDrain);
	ad5592rWritePinModes();
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

