/*
	CSSTRINGS.CPP

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

 // LP addition: use XML as source of strings (Apr 20, 2000)

 // LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt

 Sept-Nov 2001 (Woody Zenfell): added a few new pstring-oriented routines

 Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Carbon logging utilites directly format the pascal string as
 c2pstr is obsolete and I didn't want to allocate the memory
 to use CopyCStringToPascal

 May 10, 2005 (Woody Zenfell): merged almost-identical-but-annoyingly-not-quite
	csstrings.cpp (Macintosh) and csstrings_sdl.cpp into this file.
 */

// When building SDL, XML_ElementParser.h, when included here, whined that Str255 was undefined.
// (presumably, carbon target acts similarly without precompiled headers)
// So, here you go, XML_ElementParser.h.
// This is an ugly hack.  Please fix.
typedef unsigned char Str255[256];

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(EXPLICIT_CARBON_HEADER)
# include <Carbon/Carbon.h>
#endif

#include "csstrings.h"
#include "TextStrings.h"
#include "Logging.h"

using namespace std;

char temporary[256];



/*
 *  Count number of strings with given resid
 */
size_t countstr(
		short resid)
{
	return TS_CountStrings(resid);
}



/*
 *  Get pascal string
 */
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



/*
 *  Get C string
 */
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



/*
 *  Copy pascal string
 */
unsigned char *pstrcpy(
		       unsigned char *dst,
		       const unsigned char *src)
{
	memcpy(dst,src,src[0]+1);
	return dst;
}



/*
 *  Create String Vector
 */

const vector<string> build_stringvector_from_stringset (int resid)
{
	vector<string> result;
	int index = 0;
	
	while (TS_GetCString(resid, index) != NULL) {
		result.push_back (string (TS_GetCString(resid, index)));
		index++;
	}
	
	return result;
}

/*
 *  String format routines
 */
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

	va_start(list,format);
	vsprintf((char *)buffer+1,format,list);
	va_end(list);
	buffer[0] = strlen((char *)buffer+1);

	return buffer;
}



// dprintf() is obsolete with the general logging framework.  Migrate to log* (see Logging.h)
// (if Logging doesn't do what you need, improve it :) )
void
dprintf(const char* format, ...) {
	va_list list;
	va_start(list, format);
	GetCurrentLogger()->logMessageV(logDomain, logAnomalyLevel, "unknown", 0, format, list);
	va_end(list);
}



// fdprintf() is obsolete with the general logging framework.  Migrate to log* (see Logging.h)
// (if Logging doesn't do what you need, improve it :) )
void
fdprintf(const char* format, ...) {
	va_list list;
	va_start(list, format);
	GetCurrentLogger()->logMessageV(logDomain, logAnomalyLevel, "unknown", 0, format, list);
	va_end(list);
}



void copy_string_to_pstring (const std::string &s, unsigned char* dst, int maxlen)
{
	dst[0] = s.copy (reinterpret_cast<char *>(dst+1), maxlen);
}



void copy_string_to_cstring (const std::string &s, char* dst, int maxlen)
{
	dst [s.copy (dst, maxlen)] = '\0';
}



const std::string pstring_to_string (const unsigned char* ps)
{
	return std::string(reinterpret_cast<const char*>(ps) + 1, ps[0]);
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
