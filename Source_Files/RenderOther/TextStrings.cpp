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

#include "XML_ElementParser.h"

typedef std::map<short, std::string> StringSet;
typedef std::map<short, StringSet> StringSetMap;

static StringSetMap StringSetRoot;

// Error strings
static const char IndexNotFound[] = "\"index\" attribute not found";


// Parser of a set of strings

class XML_StringSetParser: public XML_ElementParser
{
public:
	// Pointer to current stringset
	short Index;
	bool IndexPresent;
		
	// Callbacks
	bool Start() {IndexPresent = false; return true;}
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_StringSetParser(): XML_ElementParser("stringset") {}
};

bool XML_StringSetParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadInt16Value(Value,Index))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StringSetParser::AttributesDone()
{
	if (!IndexPresent)
	{
		ErrorString = IndexNotFound;
		return false;
	}
	return true;
}

static XML_StringSetParser StringSetParser;


// Parser of a single string


class XML_StringParser: public XML_ElementParser
{
	// Check presence of index; having a DTD-using XML parser would
	// make this check unnecessary
	bool IndexPresent;
	
	// Build up the string; Expat may not return all characters at once
	std::string CurBuffer;

public:
	// Callbacks
	bool Start();
	bool End();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool HandleString(const char *String, int Length);
	
	// The string's index value
	short Index;

	XML_StringParser(): XML_ElementParser("string") {}
};


bool XML_StringParser::Start() {

	IndexPresent = false;
	CurBuffer = "";
	return true;
}

bool XML_StringParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		if (ReadBoundedInt16Value(Value,Index,0,INT16_MAX))
		{
			IndexPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StringParser::HandleString(const char *String, int Length)
{
	char cbuf[256];
	DeUTF8_C(String, Length, cbuf, 255);
	CurBuffer += cbuf;
	return true;
}

bool XML_StringParser::AttributesDone()
{
	if (!IndexPresent)
	{
		ErrorString = IndexNotFound;
		return false;
	}
	return true;
}
	
bool XML_StringParser::End()
{
	assert(StringSetParser.IndexPresent);
	
	StringSetRoot[StringSetParser.Index][Index] = CurBuffer.substr(0, 255);
	return true;
}

static XML_StringParser StringParser;


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


// Set up a text-string XML parser and return a pointer to it
// Don't try to delete it when one is finished with it
XML_ElementParser *TS_GetParser()
{
	StringSetParser.AddChild(&StringParser);
	return &StringSetParser;
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
