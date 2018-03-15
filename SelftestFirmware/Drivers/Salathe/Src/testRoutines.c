#include "testRoutines.h"

//variable definitions

//functions
void testSDRAM(SDRAM_HandleTypeDef *hsdram) {
	uint32_t err = 0, tested = 0, i = 0;
	LED_t off = {0x0,0x0,0x0};
	LED_t on = {0x0,0x3F,0x0};
#ifdef RAM_CHECK_FULL
	TEST_TYPE *addr;
#else
	TEST_TYPE **addrs;
#endif
	TEST_TYPE *dataW;
	TEST_TYPE *dataR;
	FIL *log;
	char hexBuf[9] = {'\0'};

	dataW = malloc(sizeof(TEST_TYPE) * TEST_SIZE);
	dataR = malloc(sizeof(TEST_TYPE) * TEST_SIZE);
#ifndef RAM_CHECK_FULL
	addrs = malloc(sizeof(TEST_TYPE *) * TEST_SIZE);
#endif
	log = malloc(sizeof(FIL));
	oledPutString("SDRAM: ", OLED_GREEN);
	termPutString("\r-- testing SDRAM --\r");
	if (!dataW || !dataR
#ifndef RAM_CHECK_FULL
			|| !addrs
#endif
			|| !log) {
		oledPutString("malloc\n failed", OLED_RED);
		termPutString("ERROR: malloc failed!\rEntering Error handler...");
		_Error_Handler(__FILE__, __LINE__);
	}

	if (SD_Rdy) {
		FRESULT fRet;
		TCHAR fName[12] = {'R','A','M','t','e','s','t','.','l','o','g',0};
		fRet = f_open(log, fName, FA_CREATE_ALWAYS | FA_WRITE);
		if (fRet != FR_OK) {
			f_close(log);
			free(log);
			log = 0;
		}
	} else {
		free(log);
		log = 0;
	}

	ledProgress(0.0, on, off);
	oledProgress(0.0, OLED_GREEN);
#ifdef RAM_CHECK_FULL
	for (addr = (TEST_TYPE *) SDRAM_ADDR; addr+TEST_SIZE <= (TEST_TYPE *) (TEST_END+SDRAM_ADDR); addr += TEST_SIZE) {
		ledProgress((float) ((uint32_t)addr-SDRAM_ADDR)/(TEST_END), on, off);
		oledProgress((float) ((uint32_t)addr-SDRAM_ADDR)/(TEST_END), OLED_GREEN);
		termPutString("offset 0x");
		termPutString(hex2Str((uint32_t) addr, 8, hexBuf));
		termPutString("\r");
#endif
		for (i = 0; i < TEST_SIZE; ++i) {
#ifndef RAM_CHECK_FULL
			uint64_t randNum = 0;
			HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) &randNum);
			randNum *= SDRAM_END-SDRAM_ADDR;
			randNum /= 4294967295;
			randNum /= 4;
			randNum *= 4;
			randNum += SDRAM_ADDR;
			addrs[i] = (TEST_TYPE *) (uint32_t) randNum;
#endif
			dataW[i] = i%2?0x55555555:0xAAAAAAAA;//(TEST_TYPE) randNum;
		}
#ifdef RAM_CHECK_FULL
		while (HAL_SDRAM_Write_32b(hsdram, addr, dataW, TEST_SIZE));
		while (HAL_SDRAM_Read_32b(hsdram, addr, dataR, TEST_SIZE));
#else
		ledProgress(0.125, on, off);
		oledProgress(0.125, OLED_GREEN);
		for (i = 0; i < TEST_SIZE; ++i) {
			HAL_SDRAM_Write_32b(hsdram, addrs[i], dataW+i, 1);
		}
		for (i = 0; i < TEST_SIZE; ++i) {
			HAL_SDRAM_Read_32b(hsdram, addrs[i], dataR+i, 1);
		}
		ledProgress(0.25, on, off);
		oledProgress(0.25, OLED_GREEN);
#endif
		for (i = 0; i < TEST_SIZE; ++i) {
			logStr(log, "0x");
#ifdef RAM_CHECK_FULL
			logStr(log, hex2Str((uint32_t) (addr + i), 8, hexBuf));
#else
			if ((i&0x1F)==0)
				ledProgress((float) i/TEST_SIZE*0.75+0.25, on, off);
			logStr(log, hex2Str((uint32_t) addrs[i], 8, hexBuf));
#endif
			logStr(log, ": 0x");
			logStr(log, hex2Str(dataW[i], sizeof(TEST_TYPE)*2, hexBuf));
			logStr(log, "->0x");
			logStr(log, hex2Str(dataR[i], sizeof(TEST_TYPE)*2, hexBuf));
			if (dataW[i] != dataR[i]) {
				++err;
				logStr(log, " ERROR");
			}
			logStr(log, "\r");
		}
		tested += TEST_SIZE;
#ifdef RAM_CHECK_FULL
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
		oledPutString(hex2Str(err*4, 6, hexBuf), OLED_RED);
		oledPutString("h/", OLED_RED);
		oledPutString(hex2Str(tested*4, 6, hexBuf), OLED_RED);
		oledPutString("h\n", OLED_RED);
	} else {
		oledPutString("okay\n", OLED_GREEN);
	}

	free(dataW);
	free(dataR);
	if (log) {
		f_close(log);
		free(log);
	}
}

void testSDCARD() {
	oledPutString("SDCARD: ", OLED_GREEN);
	termPutString("\r-- testing SD-Card --\r");

	retSD = BSP_SD_Init();
	if (retSD == MSD_ERROR_SD_NOT_PRESENT) {
		oledPutString("notFound\n", OLED_RED);
		termPutString("no SD-Card in the slot\r");
	} else if (retSD != MSD_OK) {
		oledPutString("not okay\n", OLED_RED);
		termPutString("encountered an error during initialization of SD-Card\r");
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
		} else {
			TCHAR fileName[9] = {'t','e','s','t','.','t','x','t',0};
			FIL testFile;
			termPutString(" success\rtry opening test.txt for writing operation\r");

			fatRes = f_open(&testFile, fileName, FA_CREATE_ALWAYS | FA_WRITE);
			if (fatRes != FR_OK) {
				termReportFSfail(fatRes);
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
				} else {
					termPutString(" success - closing file\rtry opening test.txt for reading operation\r");

					fatRes = f_open(&testFile, fileName, FA_READ);
					if (fatRes != FR_OK) {
						termReportFSfail(fatRes);
					} else {
						uint8_t rText[255] = {0};
						UINT bytesRead = 0;
						termPutString(" success\rtry reading from it\r");

						fatRes = f_read(&testFile, rText, 255, &bytesRead);
						if (fatRes != FR_OK) {
							termReportFSfail(fatRes);
						} else {
							int i, mismatch = 0;
							for (i = 0; i < bytesWritten; ++i)
								if (rText[i] != wText[i])
									++mismatch;
							if (mismatch) {
								termPutString(" fail - text mismatch");
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
			} else if (fatRes != FR_OK) {
				termReportFSfail(fatRes);
			} else {
				termPutString(" success\r");
			}
		}
	}
	termPutString("-- SD-CARD check done --\r");
	if(SD_Rdy){
		oledPutString("okay\n", OLED_GREEN);
	}else{
		oledPutString("fail\n", OLED_GREEN);
	}
}

void testAD5592R(SPI_HandleTypeDef *hspi) {

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
		pin.number = 3; //square wave
		ad5592rSetPin(pin, i<500);
		pin.number = 4; //square wave
		ad5592rSetPin(pin, i>=500);
		pin.number = 5; //digital test input
		input = ad5592rGetPin(pin);
		pin.number = 6; //mirror of pin 5 (push/pull)
		ad5592rSetPin(pin, input);
		pin.number = 7; //mirror of pin 5 (open drain)
		ad5592rSetPin(pin, input);
		if (++i == 1000) {
			i = 0;
		}
		ad5592rUpdate();
		vTaskDelayUntil(&xLastWakeTime, 1);
	}
}

