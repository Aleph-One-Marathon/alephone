/*
 *  csstrings_sdl.cpp - String handling, SDL implementation

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

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 *  csstrings_sdl.cpp - String handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer

// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt

 *  Sept-Nov 2001 (Woody Zenfell): added a few new pstring-oriented routines
 */

#include "cseries.h"
#include "TextStrings.h"
#include "Logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


// Global variables
char temporary[256];


/*
 *  Count number of strings with given resid
 */

size_t countstr(short resid)
{
	return TS_CountStrings(resid);
}


/*
 *  Get pascal string
 */

unsigned char *getpstr(unsigned char *string, short resid, size_t item)
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

char *getcstr(char *string, short resid, size_t item)
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

unsigned char *pstrcpy(unsigned char *dst, const unsigned char *src)
{
	memcpy(dst, src, src[0] + 1);
	return dst;
}


// ZZZ: added for safety
// Overwrites total_byte_count of Pstring 'dest' with nonoverlapping Pstring 'source' and null padding
unsigned char *pstrncpy(unsigned char *dest, const unsigned char *source, size_t total_byte_count)
{
	size_t source_count = source[0];
	if (total_byte_count <= source_count) {
		// copy truncated
		dest[0] = (unsigned char)total_byte_count - 1;
		memcpy(&dest[1], &source[1], dest[0]);
		return dest;
	} else {
		// copy full
		memcpy(dest, source, source_count + 1);
		if (source_count + 1 < total_byte_count) {
			// pad
			memset(&dest[source_count + 1], 0, total_byte_count - (source_count + 1));
		}
		return dest;
	}
}


// ZZZ: added for convenience
// Duplicate a Pstring.  Result should be free()d when no longer needed.
unsigned char *pstrdup(const unsigned char *inString)
{
	unsigned char *out = (unsigned char *)malloc(inString[0] + 1);
	pstrcpy(out, inString);
	return out;
}


/*
 *  String conversion routines (ZZZ)
 */

// a1 prefix is to avoid conflict with any already-existing functions.

// In-place conversion of Pstring to Cstring
char *a1_p2cstr(unsigned char* inoutStringBuffer)
{
	unsigned char length = inoutStringBuffer[0];
	memmove(inoutStringBuffer, &inoutStringBuffer[1], length);
	inoutStringBuffer[length] = '\0';
	return (char *)inoutStringBuffer;
}
	
// In-place conversion of Cstring to Pstring.  Quietly truncates string to 255 chars.
unsigned char *a1_c2pstr(char *inoutStringBuffer)
{
	size_t length = strlen(inoutStringBuffer);
	if (length > 255)
		length = 255;
	memmove(&inoutStringBuffer[1], inoutStringBuffer, length);
	inoutStringBuffer[0] = (char)length;
	return (unsigned char *)inoutStringBuffer;
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
	size_t length = strlen(s);
	memmove(s + 1, s, length);
	s[0] = (char)length;
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

/*
void dprintf(const char *format, ...)
{
	Str255 buffer;
	va_list list;

	va_start(list, format);
	vsprintf((char *)buffer, format, list);
	va_end(list);
	fprintf(stderr, "dprintf: %s\n", buffer);
}
*/

// dprintf() is obsolete with the general logging framework.  Migrate to log* (see Logging.h)
// (if Logging doesn't do what you need, improve it :) )
void
dprintf(const char* format, ...) {
    va_list list;
    va_start(list, format);
    GetCurrentLogger()->logMessageV(logDomain, logAnomalyLevel, "unknown", 0, format, list);
    va_end(list);
}

/*
void fdprintf(const char *format, ...)
{
	FILE *f = fopen("AlephOneDebugLog.txt", "a");
	va_list list;

	va_start(list, format);
	vfprintf(f, format, list);
	va_end(list);
	fprintf(f, "\n");
	fclose(f);
}
*/

// fdprintf() is obsolete with the general logging framework.  Migrate to log* (see Logging.h)
// (if Logging doesn't do what you need, improve it :) )
void
fdprintf(const char* format, ...) {
    va_list list;
    va_start(list, format);
    GetCurrentLogger()->logMessageV(logDomain, logAnomalyLevel, "unknown", 0, format, list);
    va_end(list);
}
