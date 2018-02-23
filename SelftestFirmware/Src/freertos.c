/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f7xx_hal.h"
#include "adc.h"
#include "fatfs.h"
#include "rng.h"
#include "rtc.h"
#include "sai.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "fmc.h"
#include "main.h"

#include "ad5592r.h"
#include "but.h"
#include "enc.h"
#include "leds.h"
#include "oled.h"
#include "term.h"
#include "font.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId bootHandle;
osThreadId GUIHandle;

/* USER CODE BEGIN Variables */
#define RAM_CHECK_FULL
#define TEST_SIZE 0x800 //variables of TEST_TYPE (0x2000 byte for one row, 0x8000 for maximum heap usage)
#define TEST_END 0x40000 //end the test right before this address offset
#define TEST_TYPE uint32_t

uint8_t booting;
uint8_t SD_Rdy;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void bootTask(void const * argument);
void guiTask(void const * argument);

extern void MX_FATFS_Init(void);
extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

void testSDRAM();
void testSDCARD();
void testGPIO();
void termReportFSfail(FRESULT r);
void logStr(FIL *file, char *s);

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	booting = 1;
	SD_Rdy = 0;
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of boot */
  osThreadDef(boot, bootTask, osPriorityLow, 0, 4096);
  bootHandle = osThreadCreate(osThread(boot), NULL);

  /* definition and creation of GUI */
  osThreadDef(GUI, guiTask, osPriorityLow, 0, 4096);
  GUIHandle = osThreadCreate(osThread(GUI), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* bootTask function */
void bootTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN bootTask */
	LED_t led[NLEDS];
	uint32_t i = 0;

	ad5592rSetup(&hspi6);
	buttonSetup();
	encSetup(&htim3, 0);
	ledSetup(&hspi4);
	oledSetup(&hspi1, SPI1_DC_OLED_GPIO_Port, SPI1_DC_OLED_Pin, SPI1_CS_OLED_GPIO_Port, SPI1_CS_OLED_Pin);
	termSetup(&huart1);

	oledFillScreen(OLED_WHITE);
	for (i = 0; i < NLEDS; ++i) {
		led[i].red = 0;
		led[i].green = 0;
		led[i].blue = 0;
	}
	ledSet(led);

	termPutString("\r\rbooting... \r");
	termPutString(" --.      |        |   |\r");
	termPutString("|         |       -|-  |     /\r");
	termPutString("'--   --  |    --  |   |--   --.\r");
	termPutString("   | |  | |   |  | |   |  | |--'\r");
	termPutString("'--' '--: '-' '--: '-' '  ' '--\r");
	termPutString("platform for developing digital\r");
	termPutString("     synthesizer modules\r");

	oledClear();
	termPutString("\r--- testing peripherals ---\r");

	testSDCARD();
//	testSDRAM();
	testGPIO();

	ledSet(led);
	termPutString("\r--- peripherals check done ---\r");

	oledPutString("press button to continue...", OLED_GREEN);
	while (!buttonFalling(ROTARYBUTTON0));

	oledClear();
	booting = 0;
	vTaskDelete(NULL);
	while(1);
  /* USER CODE END bootTask */
}

/* guiTask function */
void guiTask(void const * argument)
{
  /* USER CODE BEGIN guiTask */
	TickType_t xLastWakeTime;
	LED_t ledColor, ledOff, led[NLEDS];
	uint32_t i = 0, loopCnt = 0;

	xLastWakeTime = xTaskGetTickCount();
	while (booting) vTaskDelayUntil(&xLastWakeTime, 10);

	ledColor.red = 26;
	ledColor.green = 26;
	ledColor.blue = 26;
	ledOff.red = 0;
	ledOff.green = 0;
	ledOff.blue = 0;

	for (i = 0; i < NLEDS; ++i) {
		led[i] = ledOff; //Color;
	}

	oledClear();
	oledCurSet(0, 10);
	oledPutString("Grumpy wizards\nmake toxic brew\nfor the evil\nQueen and Jack.", OLED_BLUE);

	/* Infinite loop */
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, 100); //100ms cycle time

		if (buttonFalling(ROTARYBUTTON0)) {
			if (encValue(0) == 85) encSet(0, 170);
			else encSet(0, 85);
		}
		uint16_t pos = encValue(0);
		for (i = 0; i < 8; ++i) {
			if (pos & (1 << i)) {	//i<pos%NLEDS
				led[i] = ledColor;
			} else {
				led[i] = ledOff;
			}
		}
		ledSet(led);

		oledCurHome();
		char intBuf[6];
		oledPutString(uint2Str(pos, 5, intBuf), OLED_GREEN);
		if (loopCnt%2 == 0) {
			oledPutChar('_', OLED_GREEN);
		} else {
			oledPutChar(' ', OLED_GREEN);
		}
		++loopCnt;
	}
  /* USER CODE END guiTask */
}

/* USER CODE BEGIN Application */
void testSDRAM() {
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
		while (HAL_SDRAM_Write_32b(&hsdram1, addr, dataW, TEST_SIZE));
		while (HAL_SDRAM_Read_32b(&hsdram1, addr, dataR, TEST_SIZE));
#else
		ledProgress(0.125, on, off);
		oledProgress(0.125, OLED_GREEN);
		for (i = 0; i < TEST_SIZE; ++i) {
			HAL_SDRAM_Write_32b(&hsdram1, addrs[i], dataW+i, 1);
		}
		for (i = 0; i < TEST_SIZE; ++i) {
			HAL_SDRAM_Read_32b(&hsdram1, addrs[i], dataR+i, 1);
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
							if(mismatch){
								termPutString(" fail");
							}else{
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

void testGPIO() {
	ad5592rReg cmd;
	ad5592rReg dacData;
	uint16_t val = 0;
	uint16_t nVal = 32;
	TickType_t xLastWakeTime;

	cmd.cmd.DnC = AD5592R_SEND_CMD;
	cmd.cmd.addr = AD5592R_REG_GPO_PINS;
	cmd.cmd.data = 1<<0;
	ad5592rWriteCmd(0,cmd);
	cmd.cmd.addr = AD5592R_REG_DAC_PINS;
	cmd.cmd.data = 1<<1;
	ad5592rWriteCmd(0,cmd);

	cmd.cmd.addr = AD5592R_REG_GPO_WRITE;
	dacData.dacWrite.DnC = AD5592R_SEND_DATA;
	dacData.dacWrite.addr = 1;
	xLastWakeTime = xTaskGetTickCount();
	while (1) {
		dacData.dacWrite.data = (uint16_t) ((cos(3.1415*2/nVal*val)+1)*2047);
//		dacData.dacWrite.data = val?0xFFF:0;
		ad5592rWriteCmd(0,dacData);
//		cmd.cmd.data = (cmd.cmd.data+1)&1;
//		ad5592rWriteCmd(0,cmd);
		if((val=val+1)>=nVal) {
			val = 0;
		}
	}
}

void termReportFSfail(FRESULT r) {
	termPutString(" failed with error code (FRESULT) ");
	char intBuf[4];
	termPutString(uint2Str(r, 3, intBuf));
	termPutString("\r");
}

void logStr(FIL *file, char *s){
	if(!s) return;
	if(file){
		int i = 0, n = strlen(s);
		WCHAR *str;
		str = malloc(sizeof(WCHAR)*(n+1));
		for (i = 0; i<n; ++i){
			str[i] = ff_convert((WCHAR)s[i],1);
		}
		str[i] = '\0';
		f_puts(str, file);
		free(str);
	}else{
		termPutString(s);
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
