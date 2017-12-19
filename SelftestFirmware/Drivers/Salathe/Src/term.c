#include "term.h"
#include "font.h"

//variable definitions
UART_HandleTypeDef *hterm;

//functions
void termSetup(UART_HandleTypeDef *termHandle) {
	hterm = termHandle;
}

void termPutChar(char c) {
	HAL_UART_Transmit(hterm, (uint8_t *)&c, 1, 1000);
}

void termPutString(char *s) {
	uint32_t n = 0;
	while (s[n] != '\0')
		++n;
	HAL_UART_Transmit(hterm, (uint8_t *)s, n, 1000);
}

void termPutInt(uint32_t num, uint8_t digits) {
	char str[11] = {'\0'};
	uint8_t i = 0;
	uint32_t devider = 1; // 1000000000 for 10 digits
	uint32_t modulator = 1;
	if (digits < 1) digits = 1;
	else if (digits > 10) digits = 10;
	for (; --digits > 0;)
		devider *= 10;
	modulator = devider;
	str[0] = num/devider+'0';
	for (devider/=10; devider>0; devider/=10) {
		str[++i] = (num%modulator)/devider+'0';
		modulator = devider;
	}
	termPutString(str);
}

void termPutHex(uint32_t num, uint8_t digits){
	char str[9] = {'\0'};
	uint8_t i = 0;
	if (digits < 1) digits = 1;
	else if (digits > 8) digits = 8;
	while (digits>0) {
		str[i++] = hexDigits[(num>>(--digits*4)) & 0xF];
	}
	termPutString(str);
}

