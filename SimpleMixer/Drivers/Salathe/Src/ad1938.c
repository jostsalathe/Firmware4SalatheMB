/*
 * ad1938.c
 *
 *  Created on: 29.04.2018
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

#include "ad1938.h"

//variable definitions and private defines
ad1938_HandleTypeDef *had1938;

//chip select control macros
uint32_t ad1938CsMask[/*2*/] = {SPI1_CS_AD1938_0_Pin<<16, SPI1_CS_AD1938_1_Pin<<16};
uint32_t ad1938NcsMask[/*2*/] = {SPI1_CS_AD1938_0_Pin, SPI1_CS_AD1938_1_Pin};
__IO uint32_t* ad1938CsRegs[/*2*/] = {&(SPI1_CS_AD1938_0_GPIO_Port->BSRR), &(SPI1_CS_AD1938_1_GPIO_Port->BSRR)};
#define AD1938_SELECT(x)	(*(ad1938CsRegs[x]) = ad1938CsMask[x])
#define AD1938_DESELECT(x)	(*(ad1938CsRegs[x]) = ad1938NcsMask[x])

//general communication defines
#define AD1938_GLOB_ADDR	(0x04<<1)	//global address for the AD1938
#define AD1938_READ			(1)
#define AD1938_WRITE		(0)

//control registers addresses
#define AD1938_REG_CLK0			(0)			//PLL and Clock Control Register 0
#define AD1938_REG_CLK1			(1)			//PLL and Clock Control Register 1
#define AD1938_REG_DAC0			(2)			//DAC Control Register 0
#define AD1938_REG_DAC1			(3)			//DAC Control Register 1
#define AD1938_REG_DAC2			(4)			//DAC Control Register 2
#define AD1938_REG_MUTE			(5)			//DAC Individual Channel Mutes
#define AD1938_RED_ATTEN(x)		(6+x)		//DAC Volume Control of channel x(0...8) - attenuation from (0...255)*-3/8dB
#define AD1938_REG_ADC0			(14)		//ADC Control Register 0
#define AD1938_REG_ADC1			(15)		//ADC Control Register 1
#define AD1938_REG_ADC2			(16)		//ADC Control Register 2

//control register options
#define AD1938_CLK0_PWRDWN		(1<<0)		//PLL power down
#define AD1938_CLK0_IN256		(0<<1)		//...divider from master clock in of the AD1938 to 48kHz???
#define AD1938_CLK0_IN384		(1<<1)		//...
#define AD1938_CLK0_IN512		(2<<1)		//...
#define AD1938_CLK0_IN768		(3<<1)		//...
#define AD1938_CLK0_OUTXTAL		(0<<3)		//master clock output for driving crystal oscillator
#define AD1938_CLK0_OUT256		(1<<3)		//master clock output = 256 x fs VCO output
#define AD1938_CLK0_OUT512		(2<<3)		//master clock output = 512 x fs VCO output
#define AD1938_CLK0_OUTOFF		(3<<3)		//master clock output inactive
#define AD1938_CLK0_PLLXI		(0<<5)		//PLL fed by master clock in
#define AD1938_CLK0_PLLDLR		(1<<5)		//PLL fed by DAC frame clock
#define AD1938_CLK0_PLLALR		(2<<5)		//PLL fed by ADC frame clock
#define AD1938_CLK0_CLKEN		(1<<7)		//internal master clock enable

#define AD1938_CLK1_DACPLL		(0<<0)		//DAC clock fed by PLL
#define AD1938_CLK1_DACMCLK		(1<<0)		//DAC clock fed by MCLK
#define AD1938_CLK1_ADCPLL		(0<<1)		//ADC clock fed by PLL
#define AD1938_CLK1_ADCMCLK		(1<<1)		//ADC clock fed by MCLK
#define AD1938_CLK1_EXTREF		(1<<2)		//use external voltage reference instead of internal
#define AD1938_CLK1_PLLLOCK		(1<<3)		//PLL lock indicator (read only)

#define AD1938_DAC0_PWRDWN		(1<<0)		//DAC power down
#define AD1938_DAC0_SRATE48		(0<<1)		//DAC sample rate = 48kHz
#define AD1938_DAC0_SRATE96		(1<<1)		//DAC sample rate = 96kHz
#define AD1938_DAC0_SRATE192	(2<<1)		//DAC sample rate = 192kHz
#define AD1938_DAC0_SDADEL1		(0<<3)		//SDATA delay 1 BCLK period
#define AD1938_DAC0_SDADEL0		(1<<3)		//SDATA delay 0 BCLK period
#define AD1938_DAC0_SDADEL8		(2<<3)		//SDATA delay 8 BCLK period
#define AD1938_DAC0_SDADEL12	(3<<3)		//SDATA delay 12 BCLK period
#define AD1938_DAC0_SDADEL16	(4<<3)		//SDATA delay 16 BCLK period
#define AD1938_DAC0_SFRMTST		(0<<6)		//serial data format stereo (normal)
#define AD1938_DAC0_SFRMTTDM	(1<<6)		//serial data format TDM (daisy chain)
#define AD1938_DAC0_SFRMTAUX	(2<<6)		//serial data format DAC AUX mode (ADC-, DAC-, TDM-coupled)
#define AD1938_DAC0_SFRMTDTDM	(3<<6)		//serial data format Dual-line TDM

#define AD1938_DAC1_LATEND		(1<<0)		//BCLK active edge (TDM in) at end of cycle
#define AD1938_DAC1_2CHFRM		(0<<1)		//64 BCLKs per frame (2 channels)
#define AD1938_DAC1_4CHFRM		(1<<1)		//128 BCLKs per frame (4 channels)
#define AD1938_DAC1_8CHFRM		(2<<1)		//256 BCLKs per frame (8 channels)
#define AD1938_DAC1_16CHFRM		(3<<1)		//512 BCLKs per frame (16 channels)
#define AD1938_DAC1_LRHILO		(1<<3)		//LRCLK polarity: high on left channel instead of low
#define AD1938_DAC1_LRMASTER	(1<<4)		//LRCLK master instead of slave
#define AD1938_DAC1_BMASTER		(1<<5)		//BCLK master instead of slave
#define AD1938_DAC1_BCLKINT		(1<<6)		//generate BCLK internally instead of using DBCLK pin
#define AD1938_DAC1_BCLKINV		(1<<7)		//invert BCLK

#define AD1938_DAC2_MUTE		(1<<0)		//mute all DACs
#define AD1938_DAC2_DEEMPH48	(1<<1)		//use 48kHz De-emphasis
#define AD1938_DAC2_DEEMPH44	(2<<1)		//use 44.1kHz De-emphasis
#define AD1938_DAC2_DEEMPH32	(3<<1)		//use 32kHz De-emphasis
#define AD1938_DAC2_WORD24		(0<<3)		//24 bit word width
#define AD1938_DAC2_WORD20		(1<<3)		//20 bit word width
#define AD1938_DAC2_WORD16		(3<<3)		//16 bit word width
#define AD1938_DAC2_INVOUT		(1<<5)		//invert DAC outputs

#define AD1938_ADC0_PWRDWN		(1<<0)		//ADC power down
#define AD1938_ADC0_HIPASS		(1<<1)		//enable high-pass filter
#define AD1938_ADC0_MUTE(x)		(1<<(2+x))	//mute ADC channel x(0...3)
#define AD1938_ADC0_SRATE48		(0<<6)		//ADC sample rate = 48kHz
#define AD1938_ADC0_SRATE96		(1<<6)		//ADC sample rate = 96kHz
#define AD1938_ADC0_SRATE192	(2<<6)		//ADC sample rate = 192kHz

#define AD1938_ADC1_WORD24		(0<<0)		//24 bit word width
#define AD1938_ADC1_WORD20		(1<<0)		//20 bit word width
#define AD1938_ADC1_WORD16		(3<<0)		//16 bit word width
#define AD1938_ADC1_SDADEL1		(0<<2)		//SDATA delay 1 BCLK period
#define AD1938_ADC1_SDADEL0		(1<<2)		//SDATA delay 0 BCLK period
#define AD1938_ADC1_SDADEL8		(2<<2)		//SDATA delay 8 BCLK period
#define AD1938_ADC1_SDADEL12	(3<<2)		//SDATA delay 12 BCLK period
#define AD1938_ADC1_SDADEL16	(4<<2)		//SDATA delay 16 BCLK period
#define AD1938_ADC1_SFRMTST		(0<<5)		//serial data format stereo (normal)
#define AD1938_ADC1_SFRMTTDM	(1<<5)		//serial data format TDM (daisy chain)
#define AD1938_ADC1_SFRMTAUX	(2<<5)		//serial data format DAC AUX mode (ADC-, DAC-, TDM-coupled)
#define AD1938_ADC1_LATEND		(1<<7)		//BCLK active edge (TDM in) at end of cycle

#define AD1938_ADC2_LRPULSE		(1<<0)		//LRCLK format is 32BCLK-pulse instead of 50/50
#define AD1938_ADC2_BCLKINV		(1<<1)		//invert BCLK
#define AD1938_ADC2_LRHILO		(1<<2)		//LRCLK polarity: high on left channel instead of low
#define AD1938_ADC2_LRMASTER	(1<<3)		//LRCLK master instead of slave
#define AD1938_ADC2_2CHFRM		(0<<4)		//64 BCLKs per frame (2 channels)
#define AD1938_ADC2_4CHFRM		(1<<4)		//128 BCLKs per frame (4 channels)
#define AD1938_ADC2_8CHFRM		(2<<4)		//256 BCLKs per frame (8 channels)
#define AD1938_ADC2_16CHFRM		(3<<4)		//512 BCLKs per frame (16 channels)
#define AD1938_ADC2_BMASTER		(1<<6)		//BCLK master instead of slave
#define AD1938_ADC2_BCLKINT		(1<<7)		//generate BCLK internally instead of using ABCLK pin


//notification flags for ISR callback functions
#define AD1938_TXHALFCPLT_FLAG	(1<<0)
#define AD1938_TXCPLT_FLAG		(1<<1)
#define AD1938_RXHALFCPLT_FLAG	(1<<2)
#define AD1938_RXCPLT_FLAG		(1<<3)

//functions
void ad1938WriteReg(uint8_t regAddr, uint8_t regVal) {
//	SPI_InitTypeDef prevSpiInit;
	uint8_t buf[3];
	buf[0] = AD1938_GLOB_ADDR|AD1938_WRITE;
	buf[1] = regAddr;
	buf[2] = regVal;

	//possibly store previous SPI configuration
/*	prevSpiInit = had1938->hspi->Init;

	//possibly reconfigure SPI here
	had1938->hspi->Init = had1938->spiInit;
	if (HAL_SPI_Init(had1938->hspi) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
*/
	AD1938_SELECT(had1938->csIndex?1:0);
	HAL_SPI_Transmit(had1938->hspi, buf, 3, 10);
	AD1938_DESELECT(had1938->csIndex?1:0);

	//possibly restore previous SPI configuration
/*	had1938->hspi->Init = prevSpiInit;
	if (HAL_SPI_Init(had1938->hspi) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}*/
}

void ad1938Setup(ad1938_HandleTypeDef *had1938_) {
	had1938 = had1938_;

	//prepare SPI alternate configuration
/*	had1938->spiInit.Mode = SPI_MODE_MASTER;
	had1938->spiInit.Direction = SPI_DIRECTION_2LINES;
	had1938->spiInit.DataSize = SPI_DATASIZE_8BIT;
	had1938->spiInit.CLKPolarity = SPI_POLARITY_LOW;	//changed!
	had1938->spiInit.CLKPhase = SPI_PHASE_1EDGE;	//changed!
	had1938->spiInit.NSS = SPI_NSS_SOFT;
	had1938->spiInit.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	had1938->spiInit.FirstBit = SPI_FIRSTBIT_MSB;
	had1938->spiInit.TIMode = SPI_TIMODE_DISABLE;
	had1938->spiInit.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	had1938->spiInit.CRCPolynomial = 7;
	had1938->spiInit.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	had1938->spiInit.NSSPMode = SPI_NSS_PULSE_DISABLE;*/
	AD1938_DESELECT(had1938->csIndex?1:0);

	//configure AD1938
	ad1938WriteReg(AD1938_REG_ADC2,
			AD1938_ADC2_LRMASTER |
			AD1938_ADC2_4CHFRM |
			AD1938_ADC2_BMASTER |
			AD1938_ADC2_BCLKINT);
	ad1938WriteReg(AD1938_REG_ADC1,
			AD1938_ADC1_SFRMTTDM);
	ad1938WriteReg(AD1938_REG_ADC0,
			AD1938_ADC0_SRATE48);
	ad1938WriteReg(AD1938_REG_DAC2,
			AD1938_DAC2_WORD24);
	ad1938WriteReg(AD1938_REG_DAC1,
			AD1938_DAC1_8CHFRM |
			AD1938_DAC1_LRMASTER |
			AD1938_DAC1_BMASTER |
			AD1938_DAC1_BCLKINT);
	ad1938WriteReg(AD1938_REG_DAC0,
			AD1938_DAC0_SRATE48 |
			AD1938_DAC0_SFRMTTDM);
	ad1938WriteReg(AD1938_REG_CLK1,
			AD1938_CLK1_ADCPLL |
			AD1938_CLK1_DACPLL);
/*	ad1938WriteReg(AD1938_REG_CLK0,
			AD1938_CLK0_IN512 |
			AD1938_CLK0_OUTXTAL |
			AD1938_CLK0_PLLXI |
			AD1938_CLK0_CLKEN);*/
}

void ad1938SetVol(uint8_t iDac, uint8_t vol) {
	if(iDac<8)
		ad1938WriteReg(AD1938_RED_ATTEN(iDac), (uint8_t) ((int16_t) vol * -1 + 255));
}

void ad1938Start() {
	//start SAI and DMA transfers
	HAL_SAI_Transmit_DMA(had1938->hsaiOut, (uint8_t *) had1938->outBuf, had1938->outBufSize);
	HAL_SAI_Receive_DMA(had1938->hsaiIn, (uint8_t *) had1938->inBuf, had1938->inBufSize);
	//enable codec
	ad1938WriteReg(AD1938_REG_CLK0,
			AD1938_CLK0_IN512 |
			AD1938_CLK0_OUTXTAL |
			AD1938_CLK0_PLLXI |
			AD1938_CLK0_CLKEN);
}

void ad1938WaitOnBuffers(
		ad1938SampleType **inBuf,	//returns pointer to readily filled input buffer
		uint32_t *inBufSize,			//returns size of that buffer
		ad1938SampleType **outBuf,	//returns pointer to output buffer that has to be filled
		uint32_t *outBufSize			//returns size of that buffer
) {
	int inBufPending = 1, outBufPending = 1;
	uint32_t notification = 0;

	//register this task as handler
	had1938->taskToNotify = xTaskGetCurrentTaskHandle();

	//set buffer sizes
	*inBufSize = had1938->inBufSize/2;
	*outBufSize = had1938->outBufSize/2;

	//wait for both buffers to be ready with/for new data
	while(inBufPending || outBufPending) {
		//wait for a notification from an ISR
		if(xTaskNotifyWait(0x00, 0xFFFFFFFF, &notification, 10)) { //got a notification or timed out?
			if(notification & AD1938_TXHALFCPLT_FLAG) {
				//return first half of outBuf
				*outBuf = had1938->outBuf;
				outBufPending = 0;
			}
			if(notification & AD1938_TXCPLT_FLAG) {
				//return second half of outBuf
				*outBuf = &(had1938->outBuf[had1938->outBufSize/2]);
				outBufPending = 0;
			}
			if(notification & AD1938_RXHALFCPLT_FLAG) {
				//return first half of inBuf
				*inBuf = had1938->inBuf;
				inBufPending = 0;
			}
			if(notification & AD1938_RXCPLT_FLAG) {
				//return second half of inBuf
				*inBuf = &(had1938->inBuf[had1938->inBufSize/2]);
				inBufPending = 0;
			}
			notification = 0;
		}
	}
}

void ad1938Stop() {
	//disable codec
	ad1938WriteReg(AD1938_REG_CLK0,
			AD1938_CLK0_IN512 |
			AD1938_CLK0_OUTXTAL |
			AD1938_CLK0_PLLXI);
	//stop SAI and DMA transfers
	HAL_SAI_DMAStop(had1938->hsaiOut);
	HAL_SAI_DMAStop(had1938->hsaiIn);
}


void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
	//signal for porYIELD_FROM_ISR()
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//unblock the handling task
	xTaskNotifyFromISR( had1938->taskToNotify,
			AD1938_TXHALFCPLT_FLAG,	//notification bit to set
			eSetBits,
			&xHigherPriorityTaskWoken );

	//force a context switch for a higher priority task to ensure it continues immediately
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
	//signal for porYIELD_FROM_ISR()
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//unblock the handling task
	xTaskNotifyFromISR( had1938->taskToNotify,
			AD1938_TXCPLT_FLAG,	//notification bit to set
			eSetBits,
			&xHigherPriorityTaskWoken );

	//force a context switch for a higher priority task to ensure it continues immediately
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
	//signal for porYIELD_FROM_ISR()
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//unblock the handling task
	xTaskNotifyFromISR( had1938->taskToNotify,
			AD1938_RXHALFCPLT_FLAG,	//notification bit to set
			eSetBits,
			&xHigherPriorityTaskWoken );

	//force a context switch for a higher priority task to ensure it continues immediately
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
	//signal for porYIELD_FROM_ISR()
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	//unblock the handling task
	xTaskNotifyFromISR( had1938->taskToNotify,
			AD1938_RXCPLT_FLAG,	//notification bit to set
			eSetBits,
			&xHigherPriorityTaskWoken );

	//force a context switch for a higher priority task to ensure it continues immediately
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

