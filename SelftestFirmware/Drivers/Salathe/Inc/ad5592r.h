#ifndef _ad5592r_H
#define _ad5592r_H

#include "main.h"
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

//global variables and defines
extern uint16_t ad5592rPins[];
extern GPIO_TypeDef* ad5592rPorts[];

#define AD5592R_SELECT(x) HAL_GPIO_WritePin(ad5592rPorts[x], ad5592rPins[x], GPIO_PIN_RESET)
#define AD5592R_DESELECT(x) HAL_GPIO_WritePin(ad5592rPorts[x], ad5592rPins[x], GPIO_PIN_SET)

#define AD5592R_SEND_CMD 0
#define AD5592R_SEND_DATA 1

//control registers addresses
#define AD5592R_REG_NOP			(0b0000) //No operation
#define AD5592R_REG_DAC_READ	(0b0001) //Selects and enables DAC readback
#define AD5592R_REG_ADC_SEQ		(0b0010) //Selects ADCs for conversion
#define AD5592R_REG_GP_CTRL		(0b0011) //DAC and ADC control register
#define AD5592R_REG_ADC_PINS	(0b0100) //Selects which pins are ADC inputs
#define AD5592R_REG_DAC_PINS	(0b0101) //Selects which pins are DAC outputs
#define AD5592R_REG_PULL_DOWN	(0b0110) //Selects which pins have a 85 kOhm pull-down resistor to GND
#define AD5592R_REG_RDBK_LDAC	(0b0111) //Selects the operation of the Load DAC (LDAC) function and/or which configuration register is read back
#define AD5592R_REG_GPO_PINS	(0b1000) //Selects which pins are general-purpose outputs
#define AD5592R_REG_GPO_WRITE	(0b1001) //Writes data to the general-purpose outputs
#define AD5592R_REG_GPI_PINS	(0b1010) //Selects which pins are general-purpose inputs
#define AD5592R_REG_PWR_DOWN	(0b1011) //Powers down DACs and enables/disables the reference
#define AD5592R_REG_GPO_TYPE	(0b1100) //Selects open-drain (1) or push/pull (0) for general-purpose outputs
#define AD5592R_REG_GPO_TRI		(0b1101) //Selects which pins are three-state
#define AD5592R_REG_RESERVED	(0b1110) //Reserved
#define AD5592R_REG_SOFT_RST	(0b1111) //Resets the AD5592R/AD5592R-1

//control register data
#define AD5592R_DATA_SOFT_RST (0x5AC) //data to write into AD5592R_SOFT_RST for triggering a reset

//control definitions
#define AD5592R_RDBK_EN (1 << 6) //Enable readback in AD5592R_RDBK_LDAC command

//type definitions
typedef union {
	struct {
		unsigned int data : 11;
		unsigned int addr : 4;
		unsigned int DnC : 1;
	}cmd;
	struct {
		unsigned int data : 12;
		unsigned int addr : 3;
		unsigned int DnC : 1;
	}dacWrite;
	uint16_t reg;
}ad5592rReg;

//function prototypes
void ad5592rSetup(SPI_HandleTypeDef *hspi);
void ad5592rWriteCmd(uint8_t chip, ad5592rReg cmd);

#endif
