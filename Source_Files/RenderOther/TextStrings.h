#ifndef _TEXT_STRINGS_
#define _TEXT_STRINGS_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Text-String Collection Interface
	by Loren Petrich,
	April 20, 2000
	
	These functions replace the getting of MacOS STR# resources,
	and are called in much the same fashion.
	
	They are referenced by resource-ID number, which defines a string set,
	and an index inside that string set, which starts from 0.
*/

#include <stddef.h>

// Set up a string in the repository; a repeated call will replace an old string
void TS_PutCString(short ID, short Index, const char *String);

// Returns a pointer to a string;
// if the ID and the index do not point to a valid string,
// this function will then return NULL
const char *TS_GetCString(short ID, short Index);

// Checks on the presence of a string set
bool TS_IsPresent(short ID);

// Count the strings (contiguous from index zero)
size_t TS_CountStrings(short ID);

// Deletes a string, should one ever want to do that
void TS_DeleteString(short ID, short Index);

// Deletes the stringset with some ID
void TS_DeleteStringSet(short ID);

// Deletes all of the stringsets
void TS_DeleteAllStrings();

// Write output as a C string;
// Returns how many characters resulted.
// Needs at least (OutMaxLen + 1) characters allocated.
size_t DeUTF8_C(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen);

class InfoTree;
void parse_mml_stringset(const InfoTree& root);
void reset_mml_stringset();

#endif
