#ifndef _font_C
#define _font_C

#include "stm32f7xx_hal.h"

#define FONT_NCHARS 128
#define FONT_BYTEPERCHAR 5

extern const char hexDigits[16];
extern const uint8_t oledCharMap[FONT_NCHARS][FONT_BYTEPERCHAR];
char *hex2Str(uint32_t num, uint8_t digits, char *str);
char *uint2Str(uint32_t num, uint8_t digits, char *str);

#endif
