/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Carbon logging utilites directly format the pascal string as
		c2pstr is obsolete and I didn't want to allocate the memory
		to use CopyCStringToPascal
*/

// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(EXPLICIT_CARBON_HEADER)
# include <Carbon/Carbon.h>
/*
#else
# include <Resources.h>
# include <TextUtils.h>
*/
#endif

#include "csstrings.h"

// LP addition: use XML as source of strings (Apr 20, 2000)
#include "TextStrings.h"

char temporary[256];


size_t countstr(
	short resid)
{
	return TS_CountStrings(resid);
}

unsigned char *getpstr(
	unsigned char *string,
	short resid,
	size_t item)
{
	unsigned char *CollString = TS_GetString(resid,item);
	if (CollString)
	{
		memcpy(string,CollString,CollString[0]+1);
	}
	else
	{
		string[0] = 0;
	}
	return string;
}

char *getcstr(
	char *string,
	short resid,
	size_t item)
{
	unsigned char *CollString = TS_GetString(resid,item);
	if (CollString)
	{
		// The C string is offset one from the Pascal string
		memcpy(string,CollString+1,CollString[0]+1);
	}
	else
	{
		string[0] = 0;
	}
	return string;
}

unsigned char *pstrcpy(
	unsigned char *dst,
	const unsigned char *src)
{
	memcpy(dst,src,src[0]+1);
	return dst;
}

char *csprintf(
	char *buffer,
	const char *format,
	...)
{
	va_list list;

	va_start(list,format);
	vsprintf(buffer,format,list);
	va_end(list);
	return buffer;
}

unsigned char *psprintf(
	unsigned char *buffer,
	const char *format,
	...)
{
	va_list list;

//#if defined(USE_CARBON_ACCESSORS)
	va_start(list,format);
	vsprintf((char *)buffer+1,format,list);
	va_end(list);
	buffer[0] = strlen((char *)buffer+1);
/*
#else
	va_start(list,format);
	vsprintf((char *)buffer,format,list);
	va_end(list);
	c2pstr((char *)buffer);
#endif
*/

	return buffer;
}

void dprintf(
	const char *format,
	...)
{
	Str255 buffer;
	va_list list;

//#if defined(USE_CARBON_ACCESSORS)
	va_start(list,format);
	vsprintf((char *)buffer+1,format,list);
	va_end(list);
	buffer[0] = strlen((char *)buffer+1);
/*
#else
	va_start(list,format);
	vsprintf((char *)buffer,format,list);
	va_end(list);
	c2pstr((char *)buffer);
#endif
*/
	DebugStr(buffer);
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
