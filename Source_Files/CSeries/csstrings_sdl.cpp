/*
 *  csstrings_sdl.cpp - String handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */
// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt

#include "cseries.h"
#include "TextStrings.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


// Global variables
char temporary[256];


/*
 *  Count number of strings with given resid
 */

short countstr(short resid)
{
	return TS_CountStrings(resid);
}


/*
 *  Get pascal string
 */

unsigned char *getpstr(unsigned char *string, short resid, short item)
{
	unsigned char *CollString = TS_GetString(resid, item);
	if (CollString)
		memcpy(string, CollString, CollString[0] + 1);
	else
		string[0] = 0;
	return string;
}


/*
 *  Get C string
 */

char *getcstr(char *string, short resid, short item)
{
	unsigned char *CollString = TS_GetString(resid, item);
	if (CollString) {
		// The C string is offset one from the Pascal string
		memcpy(string, CollString + 1, CollString[0] + 1);
	} else
		string[0] = 0;
	return string;
}


/*
 *  Copy pascal string
 */

unsigned char *pstrcpy(unsigned char *dst, unsigned char *src)
{
	memcpy(dst, src, src[0] + 1);
	return dst;
}


/*
 *  String format routines
 */

char *csprintf(char *buffer, const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vsprintf(buffer, format, list);
	va_end(list);
	return buffer;
}

static void c2pstr(char *s)
{
	int length = strlen(s);
	memmove(s + 1, s, length);
	s[0] = length;
}

unsigned char *psprintf(unsigned char *buffer, const char *format, ...)
{
	va_list list;

	va_start(list,format);
	vsprintf((char *)buffer, format, list);
	va_end(list);
	c2pstr((char *)buffer);
	return buffer;
}

void dprintf(const char *format, ...)
{
	Str255 buffer;
	va_list list;

	va_start(list, format);
	vsprintf((char *)buffer, format, list);
	va_end(list);
	fprintf(stderr, "dprintf: %s\n", buffer);
}

void fdprintf(
	const char *format,
	...)
{
	FILE *FD = fopen("AlephOneDebugLog.txt","a");
	va_list list;

	va_start(list,format);
	vfprintf(FD,format,list);
	va_end(list);
	fprintf(FD,"\n");
	fclose(FD);
}
