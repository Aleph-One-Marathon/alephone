/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "GNU_GeneralPublicLicense.txt",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
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
