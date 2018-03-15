#include "benchmarks.h"

//functions

void ad5592rLibBenchmarkDAC(SPI_HandleTypeDef *hspi) {
#ifdef ALLDACS
	ad5592rPin_t pin;
	ad5592rSetup(hspi, 0xF);
	for (pin.number = 0; pin.number<32; ++pin.number) {
		ad5592rSelectPinMode(pin, ad5592rAnalogOut);
	}
	ad5592rWritePinModes();
	int i = 0, val;
	while (1) {
		val = ad5592rSine[i];
		for (pin.number = 0; pin.number<32; ++pin.number) {
			ad5592rSetPin(pin, val);
		}
#else
	ad5592rPin_t pin;
	ad5592rSetup(hspi, 0x1);
	pin.number = 0;
	ad5592rSelectPinMode(pin, ad5592rAnalogOut);
	ad5592rWritePinModes();
	int i = 0;
	while (1) {
		ad5592rSetPin(pin, ad5592rSine[i]);
#endif
		if (++i == AD5592R_N_SINE) {
			i = 0;
		}
		ad5592rUpdate();
	}
}

void ad5592rRegBenchmarkDAC(SPI_HandleTypeDef *hspi) {
	//SPI6 data register
	__IO uint16_t *spiDR = (uint16_t *) &(hspi->Instance->DR);
	//SPI6 status register
	__IO uint32_t *spiSR = &(hspi->Instance->SR);
	//Bit Set and Reset Registers for the four NCS lines
	__IO uint32_t *BSRRs[4] = {
			&(SPI6_CS_0_GPIO_Port->BSRR),
			&(SPI6_CS_1_GPIO_Port->BSRR),
			&(SPI6_CS_2_GPIO_Port->BSRR),
			&(SPI6_CS_3_GPIO_Port->BSRR)};
	//bit masks for the four NCS lines the respective BSRR
	uint32_t csPins[4] = {
			SPI6_CS_0_Pin,
			SPI6_CS_1_Pin,
			SPI6_CS_2_Pin,
			SPI6_CS_3_Pin,
	};
	//data structure for controlling the AD5592R
	ad5592rReg_t cmd;
	//counter variable
	uint32_t iSample = 0;
	//command for setting DAC values
	uint16_t dataCmd = 0x8000+ad5592rSine[iSample];
	//index of DAC
	uint32_t iDAC = 0;
#ifdef ALLDACS
	//index of AD5592R
	uint32_t iChip = 0;
#endif
	//next bit reset register
	__IO uint32_t *csReg = BSRRs[0];
	//next value for bit reset register (select)
	uint32_t csVal = csPins[0]<<16;
	//next bit set register
	__IO uint32_t *ncsReg;
	//next value for bit set register (deselect)
	uint32_t ncsVal;

	ad5592rSetup(hspi, 0xF);

	//setup command to enable all DACs
	cmd.cmd.DnC = AD5592R_SEND_CMD;
	cmd.cmd.addr = AD5592R_REG_DAC_PINS;
	cmd.cmd.data = 0xFF;
	ad5592rTxRxReg(0,cmd);
	ad5592rTxRxReg(1,cmd);
	ad5592rTxRxReg(2,cmd);
	ad5592rTxRxReg(3,cmd);

	//wait for the SPI bus to be ready
	while (!__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXE));
	while (1) {
		//activate chip select
		*csReg = csVal;
		//write data to the SPI bus
		*spiDR = dataCmd;
		//load next chip deselect parameters
		ncsReg = csReg;
		ncsVal = csVal>>16;
#ifdef ALLDACS
		//increment the DAC index and reset if necessary (8 DACs)
		++iDAC;
		if (iDAC==8) {
			iDAC = 0;
			//increment the chip index and reset if necessary (4 chips)
			++iChip;
			if (iChip==4) {
				iChip = 0;
#endif
				//increment the counter and reset if necessary (1000 values in lookup table)
				++iSample;
				if (iSample==AD5592R_N_SINE) {
					iSample = 0;
				}
#ifdef ALLDACS
			}
			//load next chip select parameters
			csReg = BSRRs[iChip];
			csVal = csPins[iChip]<<16;
		}
#endif
		//prepare next data command
		dataCmd = ((0x8|iDAC)<<12) | ad5592rSine[iSample];
		//wait for SPI transmission to end
		while ((*spiSR)&SPI_FLAG_BSY);
		//deactivate chip select
		*ncsReg = ncsVal;
	}
}
