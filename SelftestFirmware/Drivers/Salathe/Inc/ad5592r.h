/*
 * ad5592r.h
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

#ifndef _AD5592R_H
#define _AD5592R_H

#include "main.h"
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

//global variables and defines
#define AD5592R_N_SINE 1000
extern uint16_t ad5592rSine[];

extern uint8_t ad5592rChipsActive;
extern uint32_t ad5592rCsMask[];
extern uint32_t ad5592rNcsMask[];
extern __IO uint32_t* ad5592rCsRegs[];
extern __IO uint16_t* ad5592rSpiDR;
extern __IO uint32_t* ad5592rSpiSR;

#define AD5592R_CHIP0_ACTIVE (0x1)
#define AD5592R_CHIP1_ACTIVE (0x2)
#define AD5592R_CHIP2_ACTIVE (0x4)
#define AD5592R_CHIP3_ACTIVE (0x8)
#define AD5592R_CHIP_ACTIVE(x) ((1<<x) & ad5592rChipsActive)

#define AD5592R_SELECT(x) (*(ad5592rCsRegs[x]) = ad5592rCsMask[x])
#define AD5592R_DESELECT(x) (*(ad5592rCsRegs[x]) = ad5592rNcsMask[x])

#define AD5592R_SPI_WRITE(x) (*ad5592rSpiDR = x)
#define AD5592R_SPI_BUSY ((*ad5592rSpiSR)&SPI_FLAG_BSY)
#define AD5592R_SPI_READ(x) (x = *ad5592rSpiDR)

#define AD5592R_SEND_CMD 0
#define AD5592R_SEND_DATA 1

//pin masks
#define AD5592R_PINMASK(x) (1<<x)
#define AD5592R_PIN0 (0)
#define AD5592R_PIN1 (1)
#define AD5592R_PIN2 (2)
#define AD5592R_PIN3 (3)
#define AD5592R_PIN4 (4)
#define AD5592R_PIN5 (5)
#define AD5592R_PIN6 (6)
#define AD5592R_PIN7 (7)
#define AD5592R_TEMP (8)

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
#define AD5592R_REG_GPO_OD		(0b1100) //Selects open-drain (1) or push/pull (0) for general-purpose outputs
#define AD5592R_REG_GPO_TRI		(0b1101) //Selects which pins are three-state
#define AD5592R_REG_RESERVED	(0b1110) //Reserved
#define AD5592R_REG_SOFT_RST	(0b1111) //Resets the AD5592R/AD5592R-1

//control register data
#define AD5592R_DATA_SOFT_RST (0x5AC)    //Data to write into AD5592R_SOFT_RST for triggering a reset

//control definitions
#define AD5592R_RDBK_GPI (1 << 10)             //Enable GPI readback in AD5592R_REG_GPI_PINS
#define AD5592r_RDBK_REG(x) ((1 << 6) | (x << 2)) //Place register address to read from and enable register readback in AD5592R_REG_RDBK_LDAC command

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
}ad5592rReg_t;

typedef union {
	struct {
		uint8_t pin : 3;
		uint8_t chip : 2;
	};
	uint8_t number;
}ad5592rPin_t;

typedef enum {
	ad5592rThreeState,
	ad5592rDigitalIn,
	ad5592rDigitalInPullDown,
	ad5592rDigitalOut,
	ad5592rDigitalOutOpenDrain,
	ad5592rAnalogIn,
	ad5592rAnalogOut,
	ad5592rAnalogInOut
}ad5592rPinMode_t;

//function prototypes
//initial setup of all AD5592Rs
uint8_t ad5592rSetup(SPI_HandleTypeDef *hspi, uint8_t activeChips);
//select mode of one pin locally
void ad5592rSetPinMode(ad5592rPin_t pin, ad5592rPinMode_t mode);
//send all pin mode changes to AD5592Rs
void ad5592rUpdatePinModes();
//change pin value locally
void ad5592rSetPin(ad5592rPin_t pin, uint16_t val);
//get locally saved pin value
uint16_t ad5592rGetPin(ad5592rPin_t pin);
//send/receive all local pin values to/from AD5592Rs according to pin configuration
void ad5592rUpdate();
//directly send a command to one AD5592R and receive the response
ad5592rReg_t ad5592rTxRxReg(uint8_t chip, ad5592rReg_t reg);

#endif /*_AD5592R_H*/
