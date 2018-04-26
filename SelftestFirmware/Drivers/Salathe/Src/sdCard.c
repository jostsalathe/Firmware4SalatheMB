/*
 * sdCard.c
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

#include "sdCard.h"

//variable definitions

//functions
int sdCardSetup() {
	int res = 0; //result code

	retSD = BSP_SD_Init();
	if (retSD == MSD_ERROR_SD_NOT_PRESENT) {
		res = 1;
	} else if (retSD != MSD_OK) {
		res = 2;
	} else {
		FRESULT fatRes = FR_OK;

		fatRes = f_mount(&SDFatFS, (const TCHAR*) SDPath, 1);
		if (fatRes != FR_OK) {
			res = 3;
		} else {
			TCHAR fileName[9] = {'t','e','s','t','.','t','x','t',0};
			FIL testFile;

			fatRes = f_open(&testFile, fileName, FA_CREATE_ALWAYS | FA_WRITE);
			if (fatRes != FR_OK) {
				res = 4;
			} else {
				uint8_t wText[12] = "Hello world";
				UINT bytesWritten = 0;

				fatRes = f_write(&testFile, wText, 11, &bytesWritten);
				f_close(&testFile);
				if (fatRes != FR_OK) {
					res = 5;
				} else {

					fatRes = f_open(&testFile, fileName, FA_READ);
					if (fatRes != FR_OK) {
						res = 6;
					} else {
						uint8_t rText[255] = {0};
						UINT bytesRead = 0;

						fatRes = f_read(&testFile, rText, 255, &bytesRead);
						if (fatRes != FR_OK) {
							res = 7;
						} else {
							int i, mismatch = 0;
							for (i = 0; i < bytesWritten; ++i)
								if (rText[i] != wText[i])
									++mismatch;
							if (mismatch) {
								res = 8;
							} else {
								SD_Rdy = 1;
								res = 0;
							}
						}
						f_close(&testFile);
					}
				}
			}
			fatRes = f_unlink(fileName);
			if (fatRes == FR_NO_FILE || fatRes == FR_NO_PATH) {
				res = 9;
			} else if (fatRes != FR_OK) {
				res = 10;
			}
		}
	}
	if (res) SD_Rdy = 0;
	return res;
}

void logStr(FIL *file, char *s) {
	if (!s)
		return;
	if (file) {
		int i = 0, n = strlen(s);
		WCHAR *str;
		str = malloc(sizeof(WCHAR) * (n + 1));
		for (i = 0; i < n; ++i) {
			str[i] = ff_convert((WCHAR) s[i], 1);
		}
		str[i] = '\0';
		f_puts(str, file);
		free(str);
	} else {
		termPutString(s);
	}
}

void termReportFSfail(FRESULT r) {
	termPutString(" failed with error code (FRESULT) ");
	char intBuf[4];
	termPutString(uint2Str(r, 3, intBuf));
	termPutString("\r");
}

