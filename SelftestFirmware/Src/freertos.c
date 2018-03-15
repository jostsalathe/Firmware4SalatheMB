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

#include "benchmarks.h"
#include "testRoutines.h"
#include "ad5592r.h"
#include "but.h"
#include "enc.h"
#include "leds.h"
#include "oled.h"
#include "sdCard.h"
#include "term.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId bootHandle;
osThreadId GUIHandle;

/* USER CODE BEGIN Variables */

uint8_t booting;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void bootTask(void const * argument);
void guiTask(void const * argument);

extern void MX_FATFS_Init(void);
extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

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
	LED_t leds[NLEDS];
	uint32_t i = 0;

	ad5592rSetup(&hspi6, AD5592R_CHIP0_ACTIVE | AD5592R_CHIP1_ACTIVE | AD5592R_CHIP2_ACTIVE | AD5592R_CHIP3_ACTIVE);
	buttonSetup();
	encSetup(&htim3, 0);
	ledSetup(&hspi4);
	oledSetup(&hspi1, SPI1_DC_OLED_GPIO_Port, SPI1_DC_OLED_Pin, SPI1_CS_OLED_GPIO_Port, SPI1_CS_OLED_Pin);
	termSetup(&huart1);

	oledFillScreen(OLED_WHITE);
	for (i = 0; i < NLEDS; ++i) {
		leds[i].red = 0;
		leds[i].green = 0;
		leds[i].blue = 0;
	}
	ledSet(leds);

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
	testSDRAM(&hsdram1);

	ledSet(leds);
	termPutString("\r--- peripherals check done ---\r");

	oledPutString("press button to continue...", OLED_GREEN);
	while (!(buttonFalling(BUTTONENC0)||buttonFalling(BUTTONENC1)));

	oledClear();
	booting = 0;
	demoAD5592R(&hspi6);
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

		if (buttonFalling(BUTTONENC0)) {
			if (encValue(0) == 85) encSet(0, 170);
			else encSet(0, 85);
		}
		if (buttonFalling(BUTTONENC1)) {
			encSet(0, 0);
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
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
