#include "enc.h"

//variable definitions
TIM_HandleTypeDef *hspiEnc[ENC_NENCS];

//functions
void encSetup(TIM_HandleTypeDef *htim, uint8_t encNum){
	if(encNum<ENC_NENCS){
		hspiEnc[encNum] = htim;
		HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_1|TIM_CHANNEL_2);
	}
}

void encSet(uint8_t encNum, uint16_t value){
	__HAL_TIM_GET_COUNTER(hspiEnc[encNum]) = value;
}

uint16_t encValue(uint8_t encNum){
	if(encNum<ENC_NENCS){
		return __HAL_TIM_GET_COUNTER(hspiEnc[encNum]);
	}
	else {
		return 0;
	}
}
