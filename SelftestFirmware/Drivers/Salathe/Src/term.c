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
	HAL_UART_Transmit(hterm, (uint8_t *)s, strlen(s), 1000);
}
