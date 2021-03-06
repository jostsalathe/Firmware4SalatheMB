/*
 * sdCard.h
 *
 *  Created on: 15.03.2018
 *      Author: Jost Salathe <jostsalathe@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 */

#ifndef _SD_CARD_H
#define _SD_CARD_H

//includes
#include "fatfs.h"
#include "term.h"
#include "font.h"

//global variables and defines
uint8_t SD_Rdy;

//function prototypes
//initializes the SD card if present. Returns error code != 0 when not successful
int sdCardSetup();
//prints a short error report to serial terminal
void termReportFSfail(FRESULT r);
//appends the string s to file if not NULL, otherwise redirect to termPutString()
void logStr(FIL *file, char *s);

#endif /*_SD_CARD_H*/
