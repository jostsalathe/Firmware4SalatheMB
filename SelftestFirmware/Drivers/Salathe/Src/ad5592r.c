#include "ad5592r.h"

//variable definitions

SPI_HandleTypeDef *hspiAD5592R;
uint8_t ad5592rChipsActive = 0;
uint32_t ad5592rCsMask[/*4*/] = {SPI6_CS_0_Pin<<16, SPI6_CS_1_Pin<<16, SPI6_CS_2_Pin<<16, SPI6_CS_3_Pin<<16};
uint32_t ad5592rNcsMask[/*4*/] = {SPI6_CS_0_Pin, SPI6_CS_1_Pin, SPI6_CS_2_Pin, SPI6_CS_3_Pin};
__IO uint32_t* ad5592rCsRegs[/*4*/] = {&(SPI6_CS_0_GPIO_Port->BSRR), &(SPI6_CS_1_GPIO_Port->BSRR), &(SPI6_CS_2_GPIO_Port->BSRR), &(SPI6_CS_3_GPIO_Port->BSRR)};
__IO uint16_t* ad5592rSpiDR;
__IO uint32_t* ad5592rSpiSR;

uint16_t ad5592rGPIpins[4] = {0,0,0,0};
uint16_t ad5592rGPIpd[4] = {0,0,0,0};
uint16_t ad5592rGPOpins[4] = {0,0,0,0};
uint16_t ad5592rGPOod[4] = {0,0,0,0};
uint16_t ad5592rADCpins[4] = {0,0,0,0};
uint16_t ad5592rDACpins[4] = {0,0,0,0};

uint16_t ad5592rPinVals[32];

//functions
void ad5592rSetup(SPI_HandleTypeDef *hspi, uint8_t activeChips) {
	uint8_t i;
	ad5592rChipsActive = activeChips & 0xF;
	hspiAD5592R = hspi;
	ad5592rSpiDR = (uint16_t *) &(hspi->Instance->DR);
	ad5592rSpiSR = &(hspi->Instance->SR);
	for (i=0; i<4; ++i) {
		AD5592R_DESELECT(i);
	}
	__HAL_SPI_ENABLE(hspi);
	//flush the SPI RXFIFO
	AD5592R_SPI_READ(ad5592rPinVals[0]);
	AD5592R_SPI_READ(ad5592rPinVals[0]);
	for (i=0; i<32; ++i) {
		ad5592rPinVals[i] = 0;
	}
	ad5592rWritePinModes();
}

void ad5592rSelectPinMode(ad5592rPin_t pin, ad5592rPinMode_t mode) {
	int pinmask = 1<<pin.pin;
	int chip =  pin.chip;
	if (!((1<<chip) & ad5592rChipsActive)) return;
	switch(mode) {
	case ad5592rDigitalIn: {
		//activate GPI function
		ad5592rGPIpins[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rGPOpins[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		ad5592rADCpins[chip] &= ~pinmask;
		ad5592rDACpins[chip] &= ~pinmask;
		break;}
	case ad5592rDigitalInPullDown: {
		//activate GPI function
		ad5592rGPIpins[chip] |= pinmask;
		ad5592rGPIpd[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPOpins[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		ad5592rADCpins[chip] &= ~pinmask;
		ad5592rDACpins[chip] &= ~pinmask;
		break;}
	case ad5592rDigitalOut: {
		//activate GPO function
		ad5592rGPOpins[chip] |= pinmask;
		ad5592rGPIpins[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		ad5592rADCpins[chip] &= ~pinmask;
		ad5592rDACpins[chip] &= ~pinmask;
		break;}
	case ad5592rDigitalOutOpenDrain: {
		//activate GPO function
		ad5592rGPIpins[chip] |= pinmask;
		ad5592rGPOpins[chip] |= pinmask;
		ad5592rGPOod[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rADCpins[chip] &= ~pinmask;
		ad5592rDACpins[chip] &= ~pinmask;
		break;}
	case ad5592rAnalogIn: {
		//activate ADC function
		ad5592rADCpins[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpins[chip] &= ~pinmask;
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rGPOpins[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		ad5592rDACpins[chip] &= ~pinmask;
		break;}
	case ad5592rAnalogOut: {
		//activate DAC function
		ad5592rDACpins[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpins[chip] &= ~pinmask;
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rGPOpins[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		ad5592rADCpins[chip] &= ~pinmask;
		break;}
	case ad5592rAnalogInOut: {
		//activate ADC and DAC function
		ad5592rDACpins[chip] |= pinmask;
		ad5592rADCpins[chip] |= pinmask;
		//deactivate other functions
		ad5592rGPIpins[chip] &= ~pinmask;
		ad5592rGPIpd[chip] &= ~pinmask;
		ad5592rGPOpins[chip] &= ~pinmask;
		ad5592rGPOod[chip] &= ~pinmask;
		break;}
	default: return;
	}
}

void ad5592rWritePinModes() {
	//for detecting changes and only transmitting those
	static uint16_t lastAd5592rGPIpins[4] = {1,1,1,1};
	static uint16_t lastAd5592rGPIpd[4] = {1,1,1,1};
	static uint16_t lastAd5592rGPOpins[4] = {1,1,1,1};
	static uint16_t lastAd5592rGPOod[4] = {1,1,1,1};
	static uint16_t lastAd5592rADCpins[4] = {1,1,1,1};
	static uint16_t lastAd5592rDACpins[4] = {1,1,1,1};
	int chip;
	ad5592rReg_t msg;
	msg.cmd.DnC = AD5592R_SEND_CMD;
	for (chip=0; chip<4; ++chip){
		if ((1<<chip) & ad5592rChipsActive) {
			if (lastAd5592rGPIpins[chip] != ad5592rGPIpins[chip]) {
				msg.cmd.addr = AD5592R_REG_GPI_PINS;
				msg.cmd.data = ad5592rGPIpins[chip];
				lastAd5592rGPIpins[chip] = ad5592rGPIpins[chip];
				ad5592rTxRxReg(chip, msg);
			}
			if (lastAd5592rGPIpd[chip] != ad5592rGPIpd[chip]) {
				msg.cmd.addr = AD5592R_REG_PULL_DOWN;
				msg.cmd.data = ad5592rGPIpd[chip];
				lastAd5592rGPIpd[chip] = ad5592rGPIpd[chip];
				ad5592rTxRxReg(chip, msg);
			}
			if (lastAd5592rGPOpins[chip] != ad5592rGPOpins[chip]) {
				msg.cmd.addr = AD5592R_REG_GPO_PINS;
				msg.cmd.data = ad5592rGPOpins[chip];
				lastAd5592rGPOpins[chip] = ad5592rGPOpins[chip];
				ad5592rTxRxReg(chip, msg);
			}
			if (lastAd5592rGPOod[chip] != ad5592rGPOod[chip]) {
				msg.cmd.addr = AD5592R_REG_GPO_OD;
				msg.cmd.data = ad5592rGPOod[chip];
				lastAd5592rGPOod[chip] = ad5592rGPOod[chip];
				ad5592rTxRxReg(chip, msg);
			}
			if (lastAd5592rADCpins[chip] != ad5592rADCpins[chip]) {
				msg.cmd.addr = AD5592R_REG_ADC_PINS;
				msg.cmd.data = ad5592rADCpins[chip];
				lastAd5592rADCpins[chip] = ad5592rADCpins[chip];
				ad5592rTxRxReg(chip, msg);
			}
			if (lastAd5592rDACpins[chip] != ad5592rDACpins[chip]) {
				msg.cmd.addr = AD5592R_REG_DAC_PINS;
				msg.cmd.data = ad5592rDACpins[chip];
				lastAd5592rDACpins[chip] = ad5592rDACpins[chip];
				ad5592rTxRxReg(chip, msg);
			}
		}
	}
}

void ad5592rSetPin(ad5592rPin_t pin, uint16_t val){
	ad5592rPinVals[pin.number] = val;
}

uint16_t ad5592rGetPin(ad5592rPin_t pin){
	return ad5592rPinVals[pin.number];
}

void ad5592rUpdate(){
	int chip, pin, iAdc, nAdc;
	ad5592rReg_t cmdMsg, dacMsg;
	cmdMsg.cmd.DnC = AD5592R_SEND_CMD;
	//treat one chip at a time
	for (chip=0; chip<4; ++chip) {
		if ((1<<chip) & ad5592rChipsActive) {
			uint16_t pinVals[8];
			//save the set values locally
			for (pin=0; pin<8; ++pin) {
				pinVals[pin] = ad5592rPinVals[chip*8+pin];
			}
			//handle digital pins
			if (ad5592rGPIpins[chip]) {
				cmdMsg.cmd.addr = AD5592R_REG_GPI_PINS;
				cmdMsg.cmd.data = AD5592R_GPI_RDBK | ad5592rGPIpins[chip];
				//send GPI readback command
				AD5592R_SELECT(0);
				AD5592R_SPI_WRITE(cmdMsg.reg);
				//prepare digital output data or nop
				if (ad5592rGPOpins[chip]) {
					cmdMsg.cmd.addr = AD5592R_REG_GPO_WRITE;
					cmdMsg.cmd.data = 0;
					for (pin=0; pin<8; ++pin) {
						if (((ad5592rGPOpins[chip]>>pin)&1)
								&& pinVals[pin]) {
							cmdMsg.cmd.data |= 1<<pin;
						}
					}
				} else {
					cmdMsg.reg = 0; //nop
				}
				//wait for transmission complete
				while (AD5592R_SPI_BUSY) vTaskDelay(0);
				AD5592R_DESELECT(0);
				AD5592R_SPI_READ(dacMsg.reg);

				//send prepared command (digital output data or nop)
				cmdMsg = ad5592rTxRxReg(chip, cmdMsg);

				for (pin=0; pin<8; ++pin) {
					if (ad5592rGPIpins[chip] & 1<<pin) {
						ad5592rPinVals[chip*8+pin] = (cmdMsg.reg>>pin)&1;
					}
				}
			}

			//handle analog pins if configured (always temperature)
			iAdc = 0;
			nAdc = 0;
			//count number of active ADCs
			if (ad5592rADCpins[chip]) {
				for (pin=0; pin<8; ++pin) {
					if ((ad5592rADCpins[chip]>>pin)&1) {
						++nAdc;
					}
				}
				//initiate ADC conversion sequence if necessary
				if (nAdc) {
					cmdMsg.cmd.addr = AD5592R_REG_ADC_SEQ;
					cmdMsg.cmd.data = ad5592rADCpins[chip];
					cmdMsg = ad5592rTxRxReg(chip, cmdMsg);
					iAdc = -1; //for reading rubbish data once
				}
			}

			if (ad5592rDACpins[chip]) {
				dacMsg.dacWrite.DnC = AD5592R_SEND_DATA;
				for (pin=0; pin<8; ++pin) {
					if ((ad5592rDACpins[chip]>>pin)&1) {
						//send dac value
						dacMsg.dacWrite.addr = pin;
						dacMsg.dacWrite.data = pinVals[pin];
						cmdMsg = ad5592rTxRxReg(chip, dacMsg);

						//treat received data
						if (iAdc<nAdc && iAdc>=0) { //to skip first rx after starting ADC sequence
							ad5592rPinVals[chip*8+cmdMsg.dacWrite.addr] = cmdMsg.dacWrite.data;
						}
						++iAdc;
					}
				}
			}
			dacMsg.reg = 0;
			while (iAdc<nAdc) {
				//receive data via nop
				cmdMsg = ad5592rTxRxReg(chip, dacMsg);
				//treat received data
				if (iAdc>=0) { //to skip first rx after starting ADC sequence
					ad5592rPinVals[chip*8+cmdMsg.dacWrite.addr] = cmdMsg.dacWrite.data;
				}
				++iAdc;
			}
		}
	}
}

ad5592rReg_t ad5592rTxRxReg(uint8_t chip, ad5592rReg_t reg) {
	if (!((1<<chip) & ad5592rChipsActive)) {
		reg.reg = 0;
		return reg;
	}
	AD5592R_SELECT(chip);
	AD5592R_SPI_WRITE(reg.reg);
	while (AD5592R_SPI_BUSY) vTaskDelay(0);
	AD5592R_DESELECT(chip);
	AD5592R_SPI_READ(reg.reg);
	return reg;
}
