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
// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
// LP (Aug 28, 2001): Added "fdprintf" -- used like dprintf, but writes to file AlephOneDebugLog.txt
#ifndef _CSERIES_STRINGS_
#define _CSERIES_STRINGS_

extern char temporary[256];
#define ptemporary (*(Str255 *)temporary)

extern short countstr(
	short resid);

extern unsigned char *getpstr(
	unsigned char *string,
	short resid,
	short item);

extern char *getcstr(
	char *string,
	short resid,
	short item);

extern unsigned char *pstrcpy(
	unsigned char *dst,
	unsigned char *src);

extern char *csprintf(
	char *buffer,
	const char *format,
	...);

extern unsigned char *psprintf(
	unsigned char *buffer,
	const char *format,
	...);

extern void dprintf(
	const char *format,
	...);

extern void fdprintf(
	const char *format,
	...);

#endif

