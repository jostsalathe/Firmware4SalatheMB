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

