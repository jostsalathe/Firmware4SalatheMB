/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define SDRAM_ADDR 0x60000000
#define SDRAM_END 0x607FFFFF

#define PUSH0_Pin GPIO_PIN_8
#define PUSH0_GPIO_Port GPIOI
#define PUSH1_Pin GPIO_PIN_13
#define PUSH1_GPIO_Port GPIOC
#define PUSH2_Pin GPIO_PIN_6
#define PUSH2_GPIO_Port GPIOF
#define PUSH3_Pin GPIO_PIN_7
#define PUSH3_GPIO_Port GPIOF
#define SPI1_CS_AD1938_0_Pin GPIO_PIN_10
#define SPI1_CS_AD1938_0_GPIO_Port GPIOF
#define POT5_Pin GPIO_PIN_1
#define POT5_GPIO_Port GPIOC
#define POT6_Pin GPIO_PIN_2
#define POT6_GPIO_Port GPIOC
#define POT7_Pin GPIO_PIN_3
#define POT7_GPIO_Port GPIOC
#define POT0_Pin GPIO_PIN_0
#define POT0_GPIO_Port GPIOA
#define POT1_Pin GPIO_PIN_1
#define POT1_GPIO_Port GPIOA
#define POT2_Pin GPIO_PIN_2
#define POT2_GPIO_Port GPIOA
#define SPI1_CS_AD1938_1_Pin GPIO_PIN_4
#define SPI1_CS_AD1938_1_GPIO_Port GPIOA
#define POT3_Pin GPIO_PIN_6
#define POT3_GPIO_Port GPIOA
#define POT4_Pin GPIO_PIN_7
#define POT4_GPIO_Port GPIOA
#define PUSH4_Pin GPIO_PIN_4
#define PUSH4_GPIO_Port GPIOC
#define SPI6_CS_0_Pin GPIO_PIN_5
#define SPI6_CS_0_GPIO_Port GPIOC
#define PUSH5_Pin GPIO_PIN_1
#define PUSH5_GPIO_Port GPIOG
#define SPI6_CS_1_Pin GPIO_PIN_6
#define SPI6_CS_1_GPIO_Port GPIOH
#define SPI6_CS_2_Pin GPIO_PIN_7
#define SPI6_CS_2_GPIO_Port GPIOH
#define ROTARY0_PUSH_Pin GPIO_PIN_11
#define ROTARY0_PUSH_GPIO_Port GPIOD
#define ROTARY1_A_Pin GPIO_PIN_12
#define ROTARY1_A_GPIO_Port GPIOD
#define ROTARY1_B_Pin GPIO_PIN_13
#define ROTARY1_B_GPIO_Port GPIOD
#define PUSH6_Pin GPIO_PIN_2
#define PUSH6_GPIO_Port GPIOG
#define USB_OTG_HS_OverCurrent_Pin GPIO_PIN_3
#define USB_OTG_HS_OverCurrent_GPIO_Port GPIOG
#define PUSH7_Pin GPIO_PIN_6
#define PUSH7_GPIO_Port GPIOG
#define SPI1_CS_OLED_Pin GPIO_PIN_7
#define SPI1_CS_OLED_GPIO_Port GPIOG
#define ROTARY0_A_Pin GPIO_PIN_6
#define ROTARY0_A_GPIO_Port GPIOC
#define ROTARY0_B_Pin GPIO_PIN_7
#define ROTARY0_B_GPIO_Port GPIOC
#define SPI1_DC_OLED_Pin GPIO_PIN_8
#define SPI1_DC_OLED_GPIO_Port GPIOA
#define ROTARY1_PUSH_Pin GPIO_PIN_9
#define ROTARY1_PUSH_GPIO_Port GPIOA
#define SDMMC1_DETECT_Pin GPIO_PIN_10
#define SDMMC1_DETECT_GPIO_Port GPIOA
#define SPI6_CS_3_Pin GPIO_PIN_10
#define SPI6_CS_3_GPIO_Port GPIOG

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
