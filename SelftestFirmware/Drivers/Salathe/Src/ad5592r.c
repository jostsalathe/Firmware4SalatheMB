#include "ad5592r.h"

//variable definitions
SPI_HandleTypeDef *hspiAD5592R;
uint16_t ad5592rPins[/*4*/] = {SPI6_CS_0_Pin, SPI6_CS_1_Pin, SPI6_CS_2_Pin, SPI6_CS_3_Pin};
GPIO_TypeDef* ad5592rPorts[/*4*/] = {SPI6_CS_0_GPIO_Port, SPI6_CS_1_GPIO_Port, SPI6_CS_2_GPIO_Port, SPI6_CS_3_GPIO_Port};



//functions
void ad5592rSetup(SPI_HandleTypeDef *hspi) {
	uint8_t i;
	for(i=0; i<4; ++i)
		AD5592R_DESELECT(i);
	hspiAD5592R = hspi;
}

void ad5592rWriteCmd(uint8_t chip, ad5592rReg cmd) {
	if (chip>=4) return;
	AD5592R_SELECT(chip);
	HAL_SPI_Transmit(hspiAD5592R, (uint8_t) &cmd.reg, 1, 100);
	AD5592R_DESELECT(chip);
}
