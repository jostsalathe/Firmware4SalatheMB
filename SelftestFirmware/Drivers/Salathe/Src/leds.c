#include "leds.h"

//variable definitions
const uint8_t WS2812Bits[4] = {0x88, 0x8E, 0xE8, 0xEE};
uint8_t LEDsData[NSDATA];
SPI_HandleTypeDef *hspiLED;


//functions
void ledSetup(SPI_HandleTypeDef *hspi){
	uint8_t i;
	hspiLED = hspi;
	for(i=0; i<PREDATA; ++i) LEDsData[i] = 0;
	for(i=NSDATA-POSTDATA; i<NSDATA; ++i) LEDsData[i] = 0;
}

void ledSet(LED_t *led){
	uint8_t n;
	uint8_t data[NDATA];
	for(n=0; n<NLEDS; ++n){
		data[n*3+OFF_RED] = led[n].red;
		data[n*3+OFF_GREEN] = led[n].green;
		data[n*3+OFF_BLUE] = led[n].blue;
	}
	for(n=0; n<NDATA; ++n){
		uint8_t i;
		uint8_t dat = data[n];
		for(i=0; i<4; ++i){
			LEDsData[PREDATA+n*4+3-i] = WS2812Bits[dat&0x3];
			dat >>= 2;
		}
	}
	HAL_SPI_Transmit(hspiLED, LEDsData, NSDATA, 1000);
}
