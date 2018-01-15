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

#include "but.h"
#include "enc.h"
#include "leds.h"
#include "oled.h"
#include "term.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId bootHandle;
osThreadId GUIHandle;

/* USER CODE BEGIN Variables */
#define TEST_SIZE 0x1000 // 8192 byte for one row, 32768 for maximum heap usage
#define TEST_TYPE uint32_t

uint8_t booting;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void bootTask(void const * argument);
void guiTask(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

void testSDRAM();
void testSDCARD();
void termReportFSfail(FRESULT r);

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	booting = 1;
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

  /* USER CODE BEGIN bootTask */
	TickType_t xLastWakeTime;
	LED_t led[NLEDS];
	uint32_t i = 0;

	xLastWakeTime = xTaskGetTickCount();

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

	termPutString("\r\n\nbooting... \r\n");
	termPutString(" --.      |        |   |\r\n");
	termPutString("|         |       -|-  |     /\r\n");
	termPutString("'--   --  |    --  |   |--   --.\r\n");
	termPutString("   | |  | |   |  | |   |  | |--'\r\n");
	termPutString("'--' '--: '-' '--: '-' '  ' '--\r\n");
	termPutString("platform for developing digital\r\n");
	termPutString("     synthesizer modules\r\n");

	oledClear();
	termPutString("\r\n--- testing peripherals ---\r\n");
	oledClear();

	testSDCARD();
//	testSDRAM();

	termPutString("\n--- peripherals check done ---\r\n");

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

	while (booting);
	xLastWakeTime = xTaskGetTickCount();

	ledColor.red = 10;
	ledColor.green = 10;
	ledColor.blue = 10;
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
			encSet(0,8);
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
		oledPutInt(pos, OLED_GREEN, 5);
		if (pos%4 == 0) {
			oledPutChar('-', OLED_GREEN);
		} else if (pos%4 == 1) {
			oledPutChar('\\', OLED_GREEN);
		} else if (pos%4 == 2) {
			oledPutChar('|', OLED_GREEN);
		} else {
			oledPutChar('/', OLED_GREEN);
		}
		++loopCnt;
	}
  /* USER CODE END guiTask */
}

/* USER CODE BEGIN Application */
void testSDRAM() {
	uint32_t err = 0, tested = 0, i = 0;
	TEST_TYPE *addr;
	TEST_TYPE *dataW;
	TEST_TYPE *dataR;

	dataW = malloc(sizeof(TEST_TYPE) * TEST_SIZE);
	dataR = malloc(sizeof(TEST_TYPE) * TEST_SIZE);
	if (!dataW) {
		if (dataR) {
			free(dataR);
			dataR = 0;
		}
	}
	oledPutString("SDRAM: ", OLED_GREEN);
	if (!dataR) {
		oledPutString("malloc\n failed", OLED_RED);
		_Error_Handler(__FILE__, __LINE__);
	}

	termPutString("\n-- testing SDRAM --\r\n");
	for (addr = (TEST_TYPE *) SDRAM_ADDR; addr < (TEST_TYPE *) 0x6000FFFF/*SDRAM_END*/; addr += TEST_SIZE) {
		termPutString("offset 0x");
		termPutHex((uint32_t) addr, 8);
		termPutString("\r\n");
		for (i = 0; i < TEST_SIZE; ++i) {
//			uint32_t randNum = 0;
//			HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
			dataW[i] = i%2?0x55555555:0xAAAAAAAA;//(TEST_TYPE) randNum;
		}
		while (HAL_SDRAM_Write_32b(&hsdram1, (uint32_t *) addr, dataW, TEST_SIZE));
		while (HAL_SDRAM_Read_32b(&hsdram1, (uint32_t *) addr, dataR, TEST_SIZE));
		for (i = 0; i < TEST_SIZE; ++i) {
			if (dataW[i] != dataR[i]) {
				termPutString("Error at 0x");
				termPutHex((uint32_t) addr + i*sizeof(TEST_TYPE), 8);
				termPutString(": 0x");
				termPutHex(dataW[i], 8/*4*/);
				termPutString("->0x");
				termPutHex(dataR[i], 8/*4*/);
				termPutString("\r\n");
				++err;
			}
		}
		tested += TEST_SIZE;
	}
	termPutString(" encountered 0x");
	termPutHex(err, 8);
	termPutString(" errors on 0x");
	termPutHex(tested, 8);
	termPutString(" checks\r\n");
	termPutString("-- SDRAM check done --\r\n");

	if (err) {
		oledPutString("not okay:\n ", OLED_RED);
		oledPutHex(err*4, OLED_RED, 6);
		oledPutString("h/", OLED_RED);
		oledPutHex(tested*4, OLED_RED, 6);
		oledPutString("h\n", OLED_RED);
	} else {
		oledPutString("okay\n", OLED_GREEN);
	}

	free(dataW);
	free(dataR);
}

void testSDCARD() {
	oledPutString("SDCARD: ", OLED_GREEN);
	termPutString("\n-- testing SD-Card --\r\n");

	retSD = BSP_SD_Init();
	if (retSD == MSD_ERROR_SD_NOT_PRESENT) {
		oledPutString("notFound\n", OLED_RED);
		termPutString("no SD-Card in the slot\r\n");
	} else if (retSD != MSD_OK) {
		oledPutString("not okay\n", OLED_RED);
		termPutString("encountered an error during initialization of SD-Card\r\n");
	} else {
		FRESULT fatRes = FR_OK;
		oledPutString("okay\n mnt to ", OLED_GREEN);
		oledPutString(SDPath, OLED_GREEN);
		oledPutString(" ", OLED_GREEN);

		termPutString("found and initialized SD-Card\r\nmount SD-CARD to \"");
		termPutString(SDPath);
		termPutString("\":\r\n");

		fatRes = f_mount(&SDFatFS, (const TCHAR*) SDPath, 1);
		if (fatRes != FR_OK) {
			oledPutString("fail\n", OLED_GREEN);
			termReportFSfail(fatRes);
		} else {
			FIL testFile;
			oledPutString("okay\n", OLED_GREEN);
			termPutString(" success\r\ntry opening test.txt for writing operation\r\n");

			fatRes = f_open(&testFile, (const TCHAR*) "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
			if (fatRes != FR_OK) {
				termReportFSfail(fatRes);
			} else {
				uint8_t wText[12] = "Hello world";
				UINT bytesWritten = 0;
				termPutString(" success\r\ntry writing \"");
				termPutString((char *)wText);
				termPutString("\" to it\r\n");

				fatRes = f_write(&testFile, wText, 11, &bytesWritten);
				f_close(&testFile);
				if (fatRes != FR_OK) {
					termReportFSfail(fatRes);
				} else {
					termPutString(" success - closing file\r\ntry opening test.txt for reading operation\r\n");

					fatRes = f_open(&testFile, (const TCHAR*) "test.txt", FA_READ);
					if (fatRes != FR_OK) {
						termReportFSfail(fatRes);
					} else {
						uint8_t rText[12];
						UINT bytesRead = 0;
						termPutString(" success\r\ntry reading from it");

						fatRes = f_read(&testFile, rText, 11, &bytesRead);
						if (fatRes != FR_OK) {
							termReportFSfail(fatRes);
						} else {
							termPutString(" success - read: \"");
							termPutString("\"\r\n");
						}
						f_close(&testFile);
					}
				}
			}
		}
	}
	termPutString("-- SD-CARD check done --\r\n");
}

void termReportFSfail(FRESULT r) {
	termPutString(" failed with error code (FRESULT) ");
	termPutInt(r, 3);
	termPutString("\r\n");
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
