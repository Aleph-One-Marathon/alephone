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

	Text-String Collection
	by Loren Petrich,
	April 20, 2000
	
	This is the implementation of my replacement for MacOS STR# resources
*/


#include <string>
#include <map>
#include "cseries.h"
#include "TextStrings.h"
#include "InfoTree.h"

typedef std::map<short, std::string> StringSet;
typedef std::map<short, StringSet> StringSetMap;

static StringSetMap StringSetRoot;


// Public routines:


// Set up a string in the repository; a repeated call will replace an old string
void TS_PutCString(short ID, short Index, const char *String)
{
	if (Index >= 0)
	{
		StringSetRoot[ID][Index] = String;
	}
}


// Returns a pointer to a string; if the ID and the index do not point to a valid string,
// this function will then return NULL
const char *TS_GetCString(short ID, short Index)
{
	StringSetMap::const_iterator setIter = StringSetRoot.find(ID);
	if (setIter == StringSetRoot.end())
		return NULL;
	StringSet::const_iterator strIter = setIter->second.find(Index);
	if (strIter == setIter->second.end())
		return NULL;
	return strIter->second.c_str();
}


// Checks on the presence of a string set
bool TS_IsPresent(short ID)
{
	return StringSetRoot.count(ID);
}


// Count the strings (contiguous from index zero)
size_t TS_CountStrings(short ID)
{
	StringSetMap::const_iterator setIter = StringSetRoot.find(ID);
	if (setIter == StringSetRoot.end())
		return 0;
	return setIter->second.size();
}


// Deletes a string, should one ever want to do that
void TS_DeleteString(short ID, short Index)
{
	StringSetMap::iterator setIter = StringSetRoot.find(ID);
	if (setIter == StringSetRoot.end())
		return;
	setIter->second.erase(Index);
}


// Deletes the stringset with some ID
void TS_DeleteStringSet(short ID)
{
	StringSetRoot.erase(ID);
}


// Deletes all of the stringsets
void TS_DeleteAllStrings()
{
	StringSetRoot.clear();
}


void reset_mml_stringset()
{
	// no reset
}

void parse_mml_stringset(const InfoTree& root)
{
	int16 index;
	if (!root.read_attr("index", index))
		return;
	
	BOOST_FOREACH(InfoTree child, root.children_named("string"))
	{
		int16 cindex;
		if (!child.read_indexed("index", cindex, INT16_MAX))
			continue;
		
		std::string val = child.get_value<std::string>("");
		char cbuf[256];
		DeUTF8_C(val.c_str(), val.size(), cbuf, 255);
		StringSetRoot[index][cindex] = std::string(cbuf);
	}
}

// For turning UTF-8 strings into plain ASCII ones;
// needs at least (OutMaxLen) characters preallocated.
// Will not null-terminate the string or Pascalify it.
// Returns how many characters resulted.

static size_t DeUTF8(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen)
{
	// Character and masked version for bit tests;
	// unsigned char to avoid problems with interpreting the sign bit
	uint8 c, cmsk;
	
	// Result character, in its full glory
	uint32 uc;
	
	int NumExtra = 0;	// Initial: as if previous string had ended
	const int BAD_CHARACTER = -1; // NumExtra value that means
	// "end string, but don't emit the character"
	// Won't try to test for overlong characters.
	
	// How many characters processed
	size_t Len = 0;
	
	for (size_t ic=0; ic<InLen; ic++)
	{
		c = uint8(InString[ic]);
		
		if (NumExtra <= 0)
		{
			// Start character string if previous one had ended or was bad
			// Note that cmsk is calculated in each if() statement,
			// so it can be used inside of its statement block,
			// and so as to get a more elegant overall organization.
			if ((cmsk = c & 0x80) != 0x80)
			{
				// 0 to 7 bits long: straight ASCII
				uc = uint32(c & 0x7f);
				NumExtra = 0;
			}
			else if ((cmsk = c & 0xe0) != 0xe0)
			{
				if (cmsk == 0xc0)
				{
					// 8 to 11 bits long
					uc = uint32(c & 0x1f);
					NumExtra = 1;
				}
				else
					NumExtra = BAD_CHARACTER;
			}
			else if ((cmsk = c & 0xf0) != 0xf0)
			{
				if (cmsk == 0xe0)
				{
					// 12 to 16 bits long
					uc = uint32(c & 0x0f);
					NumExtra = 2;
				}
				else
					NumExtra = BAD_CHARACTER;
			}
			else if ((cmsk = c & 0xf8) != 0xf8)
			{
				if (cmsk == 0xf0)
				{
					// 17 to 21 bits long
					uc = uint32(c & 0x07);
					NumExtra = 3;
				}
				else
					NumExtra = BAD_CHARACTER;
			}
			else if ((cmsk = c & 0xfc) != 0xfc)
			{
				if (cmsk == 0xf8)
				{
					// 22 to 26 bits long
					uc = uint32(c & 0x03);
					NumExtra = 4;
				}
				else
					NumExtra = BAD_CHARACTER;
			}
			else if ((cmsk = c & 0xfe) != 0xfe)
			{
				if (cmsk == 0xfc)
				{
					// 27 to 31 bits long
					uc = uint32(c & 0x01);
					NumExtra = 5;
				}
				else
					NumExtra = BAD_CHARACTER;
			}
			else
				NumExtra = BAD_CHARACTER;
		}
		else
		{
			cmsk = c & 0xc0;
			if (cmsk == 0x80)
			{
				uc <<= 6;
				uc |= uint32(c & 0x3f);
				NumExtra--;
			}
			else
				NumExtra = BAD_CHARACTER;
		}
		
		// Overlong test would go here
		
		if (NumExtra == 0)
		{
			// Bad characters become a dollar sign
			//			uint8 oc = (uc >= 0x20 && uc != 0x7f && uc <= 0xff) ? uint8(uc) : '$';
			uint8 oc = (uc >= 0x20 && uc != 0x7f && uc < 0x10000) ? unicode_to_mac_roman((uint16) uc) : '$';
			OutString[Len++] = char(oc);
			if (Len >= OutMaxLen) break;
		}
	}
	
	return Len;
}

// Write output as a C string;
// Returns how many characters resulted.
// Needs at least (OutMaxLen + 1) characters allocated.

size_t DeUTF8_C(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen)
{
	size_t Len = DeUTF8(InString,InLen,OutString,OutMaxLen);
	OutString[Len] = 0;
	return Len;
}
