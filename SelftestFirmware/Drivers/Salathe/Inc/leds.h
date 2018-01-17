#ifndef _leds_H
#define _leds_H

#include "stm32f7xx_hal.h"

//global variables and defines
#define OFF_RED 0
#define OFF_GREEN 1
#define OFF_BLUE 2
extern const uint8_t WS2812Bits[];
#define NLEDS 8
#define PREDATA 0
#define POSTDATA 8
#define NDATA NLEDS*3 //pure LED data length
#define NSDATA PREDATA+NDATA*4+POSTDATA //serial data length


//type definitions
typedef struct{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}LED_t;

//function prototypes
void ledSetup(SPI_HandleTypeDef *hspi);
void ledSet(LED_t *led);

#endif
