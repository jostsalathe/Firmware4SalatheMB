#ifndef _enc_H
#define _enc_H

#include "stm32f7xx_hal.h"

//global variables and defines
#ifndef ENC_NENCS
#define ENC_NENCS 2
#endif
//type definitions

//function prototypes
void encSetup(TIM_HandleTypeDef *htim, uint8_t encNum);
void encSet(uint8_t encNum, uint16_t value);
uint16_t encValue(uint8_t encNum);

#endif
