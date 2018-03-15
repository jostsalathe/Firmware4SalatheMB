#ifndef _SD_CARD_H
#define _SD_CARD_H

//includes
#include "fatfs.h"
#include "term.h"
#include "font.h"

//global variables and defines
uint8_t SD_Rdy;

//function prototypes
void termReportFSfail(FRESULT r);
void logStr(FIL *file, char *s);

#endif /*_SD_CARD_H*/
