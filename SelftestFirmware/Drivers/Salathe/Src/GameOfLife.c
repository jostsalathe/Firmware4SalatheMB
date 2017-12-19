#include "GameOfLife.h"

//variable definitions
char WALL=1;
uint8_t world[WOR_X][NUM_Y];

//functions

char GOL_cellAlive(int x, int y){
	return GOL_cellAliveIntern(world, x, y);
}

char GOL_cellAliveIntern(uint8_t world[WOR_X][NUM_Y], int x, int y){
  if(WALL){
    if(x<0 || y<0 || x>=NUM_X || y>=NUM_Y)  //constrain boarders
      return 0;
  }else{   //wrap borders
    if(x<0) x=NUM_X-1;
    else if(x>=NUM_X) x=0;
    if(y<0) y=NUM_Y-1;
    else if(y>=NUM_Y) y=0;
  }
  return world[x/8][y]&(0x80>>(x%8));
}

void GOL_setCell(int x, int y, char alive){
  if(x<0 || y<0 || x>=NUM_X || y>=NUM_Y) return;  //constrain boarders
  if(alive) world[x/8][y] |= (0x80>>(x%8));   //set cell
  else world[x/8][y] &= ~(0x80>>(x%8));     //delete cell
}

void GOL_nextGen(){
  uint8_t old[WOR_X][NUM_Y];
  for(int y=0; y<NUM_Y; ++y){
    for(int x=0; x<WOR_X; ++x){
      old[x][y] = world[x][y];
    }
  }
  for(int y=0; y<NUM_Y; ++y){
    for(int x=0; x<NUM_X; ++x){
      uint8_t neighbors=0;
      if(GOL_cellAliveIntern(old, x-1, y-1)) ++neighbors;
      if(GOL_cellAliveIntern(old, x-1, y  )) ++neighbors;
      if(GOL_cellAliveIntern(old, x-1, y+1)) ++neighbors;
      if(GOL_cellAliveIntern(old, x,   y+1)) ++neighbors;
      if(GOL_cellAliveIntern(old, x+1, y+1)) ++neighbors;
      if(GOL_cellAliveIntern(old, x+1, y  )) ++neighbors;
      if(GOL_cellAliveIntern(old, x+1, y-1)) ++neighbors;
      if(GOL_cellAliveIntern(old, x,   y-1)) ++neighbors;

      if(GOL_cellAliveIntern(world, x, y)){
        if(neighbors<2) GOL_setCell(x,y,0);
        if(neighbors>3) GOL_setCell(x,y,0);
      }else if(neighbors==3) GOL_setCell(x,y,1);
    }
  }
}

void GOL_toggleWall(){
  WALL = !WALL;
}
