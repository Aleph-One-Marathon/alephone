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


#include <string.h>
#include "cseries.h"
#include "TextStrings.h"

#include "XML_ElementParser.h"


// Private objects: the string collections, which form a linked list.
class StringSet
{
	short ID;
	size_t NumStrings;
	// Pointer to string pointers:
	unsigned char **Strings;

public:
	// What's the ID
	short GetID() {return ID;}
	
	// How many strings (contiguous from index zero)
	size_t CountStrings();
	
	// Create a stringset with some ID
	StringSet(short _ID);
	~StringSet();

	// Assumes a MacOS Pascal string; the resulting string will have
	// a null byte at the end.
	void Add(size_t Index, unsigned char *String);
        
	// Get a string; return NULL if not found
	unsigned char *GetString(size_t Index);
	
	// Delete a string
	void Delete(size_t Index);
	
	// For making a linked list (I'll let Rhys Hill find more efficient data structures)
	StringSet *Next;
};


// How many strings (contiguous from index zero)
size_t StringSet::CountStrings()
{
	size_t StringCount = 0;
	
	for (size_t k=0; k<NumStrings; k++)
	{
		if (Strings[k])
			StringCount++;
		else
			break;
	}
	
	return StringCount;
}


StringSet::StringSet(short _ID)
{
	// Of course
	ID = _ID;
	
	NumStrings = 16;	// Reasonable starting number; what Sun uses in Java
	Strings = new unsigned char *[NumStrings];
	
	// Set all the string pointers to NULL
	objlist_clear(Strings,NumStrings);
	
	// Last, but not least:
	Next = NULL;
}

StringSet::~StringSet()
{
	for (size_t k=0; k<NumStrings; k++)
	{
		unsigned char *StringPtr = Strings[k];
		if (StringPtr) delete []StringPtr;
	}
	delete []Strings;
}

// Assumes a MacOS Pascal string; the resulting string will have
// a null byte at the end.
void StringSet::Add(size_t Index, unsigned char *String)
{
	if (Index < 0) return;
	
	// Replace string list with longer one if necessary
	size_t NewNumStrings = NumStrings;
	while (Index >= NewNumStrings) {NewNumStrings <<= 1;}
	
	if (NewNumStrings > NumStrings)
	{
		unsigned char **NewStrings = new unsigned char *[NewNumStrings];
		objlist_clear(NewStrings+NumStrings,(NewNumStrings-NumStrings));
		objlist_copy(NewStrings,Strings,NumStrings);
		delete []Strings;
		Strings = NewStrings;
		NumStrings = NewNumStrings;
	}
	
	// Delete the old string if necessary
	if (Strings[Index]) delete []Strings[Index];
	
	unsigned short Length = String[0];
	unsigned char *_String = new unsigned char[Length+2];
	memcpy(_String,String,Length+2);
	_String[Length+1] = 0;	//  for making an in-place C string
	
	Strings[Index] = _String;
}

// Get a string; return NULL if not found
unsigned char *StringSet::GetString(size_t Index)
{
	if (Index < 0 || Index >= NumStrings) return NULL;
	
	return Strings[Index];
}

// Delete a string
void StringSet::Delete(size_t Index)
{
	if (Index < 0 || Index >= NumStrings) return;
	
	unsigned char *StringPtr = Strings[Index];
	if (StringPtr) {delete []StringPtr; Strings[Index] = 0;}
}

static StringSet *StringSetRoot = NULL;

static StringSet *FindStringSet(short ID)
{
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	while(CurrStringSet)
	{
		PrevStringSet = CurrStringSet;
		if (CurrStringSet->GetID() == ID) break;
		CurrStringSet = CurrStringSet->Next;
	}
	
	// Add to the end if not found
	if (!CurrStringSet)
	{
		StringSet *NewStringSet = new StringSet(ID);
		assert(NewStringSet);
		if (PrevStringSet)
			PrevStringSet->Next = NewStringSet;
		else
			StringSetRoot = NewStringSet;
		CurrStringSet = NewStringSet;
	}
	
	return CurrStringSet;
}

// Error strings
static const char IndexNotFound[] = "\"index\" attribute not found";


// Parser of a set of strings

class XML_StringSetParser: public XML_ElementParser
{
public:
	// Pointer to current stringset
	StringSet *CurrStringSet;
		
	// Callbacks
	bool Start() {CurrStringSet = NULL; return true;}
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_StringSetParser(): XML_ElementParser("stringset") {}
};

bool XML_StringSetParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"index"))
	{
		short ID;
		if (ReadInt16Value(Value,ID))
		{
			CurrStringSet = FindStringSet(ID);
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_StringSetParser::AttributesDone()
{
	if (!CurrStringSet)
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
	Str255 CurBuffer;

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
	CurBuffer[0] = 0;
	return true;
}

bool XML_StringParser::HandleAttribute(const char *Tag, const char *Value)
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

bool XML_StringParser::HandleString(const char *String, int Length)
{
	// Copy into Pascal string
	Str255 StringBuffer;
	DeUTF8_Pas(String,Length,StringBuffer,255);
	
	int curlen = CurBuffer[0];
	int newlen = StringBuffer[0];
	assert(curlen + newlen <= 255);
	memcpy(&CurBuffer[curlen + 1], &StringBuffer[1], newlen);
	CurBuffer[0] = curlen + newlen;
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
	assert(StringSetParser.CurrStringSet);
	StringSetParser.CurrStringSet->Add(Index,CurBuffer);
	return true;
}

static XML_StringParser StringParser;


// Public routines:


// Set up a string in the repository; a repeated call will replace an old string
void TS_PutCString(short ID, short Index, const char *String)
{
	// Search for string set: (FindStringSet() creates a new one if necessary)
	StringSet *CurrStringSet = FindStringSet(ID);
        
        // Create a PString of the incoming CString, truncate to fit
        unsigned char	thePStringStagingBuffer[256];

        size_t theStringLength = strlen(String);

        if(theStringLength > 255)
            theStringLength = 255;

        // Fill in the string length.
        thePStringStagingBuffer[0] = (char)theStringLength;
        
        // Copy exactly the string bytes (no termination etc.)
        memcpy(&(thePStringStagingBuffer[1]), String, theStringLength);
        
        // Add() copies the string, so using the stack here is OK.
	CurrStringSet->Add(Index,thePStringStagingBuffer);
}


// Returns a pointer to a string; if the ID and the index do not point to a valid string,
// this function will then return NULL
char *TS_GetCString(short ID, size_t Index)
{
	// Search for string set:
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		CurrStringSet = CurrStringSet->Next;
	}
	
	if (!CurrStringSet) return NULL;
	
	unsigned char *String = CurrStringSet->GetString(Index);
	if (!String) return NULL;
	
	// Move away from the length byte to the first content byte
	return (char *)(String+1);
}


// Checks on the presence of a string set
bool TS_IsPresent(short ID)
{
	// Search for string set:
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) return true;
		CurrStringSet = CurrStringSet->Next;
	}
	
	return false;
}


// Count the strings (contiguous from index zero)
size_t TS_CountStrings(short ID)
{
	// Search for string set:
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		CurrStringSet = CurrStringSet->Next;
	}
	
	if (!CurrStringSet) return 0;
	
	return CurrStringSet->CountStrings();
}


// Deletes a string, should one ever want to do that
void TS_DeleteString(short ID, short Index)
{
	// Search for string set:
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		CurrStringSet = CurrStringSet->Next;
	}

	if (!CurrStringSet) return;
	
	CurrStringSet->Delete(Index);
}


// Deletes the stringset with some ID
void TS_DeleteStringSet(short ID)
{
	// Search for string set:
	StringSet *PrevStringSet = NULL, *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		if (CurrStringSet->GetID() == ID) break;
		PrevStringSet = CurrStringSet;
		CurrStringSet = CurrStringSet->Next;
	}

	if (!CurrStringSet) return;
	
	// Get the next string set:
	StringSet *NextStringSet = CurrStringSet->Next;
	
	// Clip out that string set
	if (PrevStringSet)
		PrevStringSet->Next = NextStringSet;
	else
		StringSetRoot = NextStringSet;
	
	delete CurrStringSet;
}


// Deletes all of the stringsets
void TS_DeleteAllStrings()
{
	StringSet *CurrStringSet = StringSetRoot;
	
	while(CurrStringSet)
	{
		StringSet *NextStringSet = CurrStringSet->Next;
		delete CurrStringSet;
		CurrStringSet = NextStringSet;
	}
	
	StringSetRoot = NULL;
}


// Set up a text-string XML parser and return a pointer to it
// Don't try to delete it when one is finished with it
XML_ElementParser *TS_GetParser()
{
	StringSetParser.AddChild(&StringParser);
	return &StringSetParser;
}

