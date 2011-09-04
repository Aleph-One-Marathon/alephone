/* csstrings.h

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html


// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt

    Sept-Nov 2001 (Woody Zenfell): added new routines; gave pstrcpy a "const" parameter qualifier.
*/

#ifndef _CSERIES_STRINGS_
#define _CSERIES_STRINGS_

#if defined(__GNUC__)
#define PRINTF_STYLE_ARGS(n,m) __attribute__((format(printf,n,m)))
#else
#define PRINTF_STYLE_ARGS(n,m)
#endif

#include "cstypes.h"
#include <string>
#include <vector>

extern char temporary[256];
#define ptemporary (*(Str255 *)temporary)

extern size_t countstr(
	short resid);

extern unsigned char *getpstr(
	unsigned char *string,
	short resid,
	size_t item);

extern char *getcstr(
	char *string,
	short resid,
	size_t item);

// jkvw addition
extern const std::vector<std::string> build_stringvector_from_stringset (int resid);

// ZZZ: changed to 'const' on src parameter
extern unsigned char *pstrcpy(
	unsigned char *dst,
	const unsigned char *src);

// START ZZZ additions
extern unsigned char* pstrncpy(
	unsigned char* dest,
	const unsigned char* source,
	size_t total_byte_count);

extern unsigned char* pstrdup(
	const unsigned char* source);

extern unsigned char* a1_c2pstr(
	char* inoutStringBuffer);

extern char* a1_p2cstr(
	unsigned char* inoutStringBuffer);
// END ZZZ additions

extern char *csprintf(
	char *buffer,
	const char *format,
	...) PRINTF_STYLE_ARGS(2,3);

extern unsigned char *psprintf(
	unsigned char *buffer,
	const char *format,
	...) PRINTF_STYLE_ARGS(2,3);

extern void dprintf(
	const char *format,
	...) PRINTF_STYLE_ARGS(1,2);

extern void fdprintf(
	const char *format,
	...) PRINTF_STYLE_ARGS(1,2);

// jkvw additions
extern void copy_string_to_pstring (const std::string &s, unsigned char* dst, int maxlen = 255);
extern void copy_string_to_cstring (const std::string &s, char* dst, int maxlen = 255);

extern const std::string pstring_to_string (const unsigned char* ps);

extern uint16 mac_roman_to_unicode(char c);
extern char unicode_to_mac_roman(uint16 c);
extern void mac_roman_to_unicode(const char *input, uint16 *output);
extern void mac_roman_to_unicode(const char *input, uint16 *output, int max_len);
std::string mac_roman_to_utf8(const std::string& input);
std::string utf8_to_mac_roman(const std::string& input);

// Substitute special variables like application name or version
std::string expand_app_variables(const std::string& input);
void expand_app_variables_inplace(std::string& str);
void expand_app_variables(char *dest, const char *src);

#endif
