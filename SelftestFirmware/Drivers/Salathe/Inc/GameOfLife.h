#ifndef _GameOfLife_H
#define _GameOfLife_H

#include "stm32f7xx_hal.h"

//global variables and defines
#define NUM_X 128
#define WOR_X (NUM_X-1)/8+1
#define NUM_Y 96
#define NUM_PIX (NUM_X*NUM_Y)

//type definitions

//function prototypes
char GOL_cellAlive(int x, int y);
char GOL_cellAliveIntern(uint8_t world[WOR_X][NUM_Y], int x, int y);
void GOL_setCell(int x, int y, char alive);
void GOL_nextGen();
void GOL_toggleWall();

#endif
