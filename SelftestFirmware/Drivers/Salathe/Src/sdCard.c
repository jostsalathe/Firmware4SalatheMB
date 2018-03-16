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

