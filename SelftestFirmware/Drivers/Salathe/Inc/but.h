#ifndef _but_H
#define _but_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

//global variables and defines
#define BUTTONN 10
#define BUTTON0 0
#define BUTTON1 1
#define BUTTON2 2
#define BUTTON3 3
#define BUTTON4 4
#define BUTTON5 5
#define BUTTON6 6
#define BUTTON7 7
#define ROTARYBUTTON0 8
#define ROTARYBUTTON1 9

//type definitions

//function prototypes
void pollButtons();
void buttonSetup();
uint8_t buttonRising(uint8_t buttonNumber);
uint8_t buttonFalling(uint8_t buttonNumber);
uint8_t buttonRead(uint8_t buttonNumber);
#endif
