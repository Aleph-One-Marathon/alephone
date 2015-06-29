#ifndef XML_ELEMENTPARSER
#define XML_ELEMENTPARSER
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

	XML-Element Parsing Objects
	by Loren Petrich,
	April 15, 2000
	
	These are subclassed for each kind of element to be parsed.
	
	May 3, 2000
	Added a change to adding a child so as not to add one with the same name twice.

Oct 13, 2000 (Loren Petrich)
	Changed to STL container

Dec 25, 2001 (Loren Petrich)
	Made StringsEqual case-independent for the purpose of making parsing of
	XML element names and attribute names case-independent.
*/


#include <vector>
#include <stdio.h>
using namespace std;

#include "cseries.h"
#include "cstypes.h"

extern bool XML_GetBooleanValue(const char *String, bool &Value);

// This function is case-insensitive, so there will be no need
// to specify capital and small versions of search-target strings.
// The default maximum length is a reasonable one for most likely element and attribute names
bool StringsEqual(const char *String1, const char *String2, int MaxStrLen = 32);


// For turning UTF-8 strings into plain ASCII ones;
// needs at least (OutMaxLen) characters preallocated.
// Will not null-terminate the string or Pascalify it.
// Returns how many characters resulted.
size_t DeUTF8(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen);

// Write output as a C string;
// Returns how many characters resulted.
// Needs at least (OutMaxLen + 1) characters allocated.
size_t DeUTF8_C(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen);


class XML_ElementParser
{
	// Its name, of course
	char *Name;
		
	// List of child elements
	vector<XML_ElementParser *> Children;

protected:
	// Designed for easy reading of numerical values:
	// args are the string to read, the format string (for <>scanf),
	// and the numerical-value destination
	template<class T> bool ReadNumericalValue(const char *String, const char *Format,
		T& Value)
	{
		if (sscanf(String,Format,&Value) != 1)
		{
			BadNumericalValue();
			return false;
		}
		return true;
	}
	
	// For reading of bounded numerical values, such as index values
	template<class T> bool ReadBoundedNumericalValue(const char *String, const char *Format,
		T& Value, const T& MinVal, const T& MaxVal)
	{
		if (ReadNumericalValue(String,Format,Value))
		{
			if (Value >= MinVal && Value <= MaxVal)
				return true;
			
			OutOfRange();
			return false;
		}
		return false;
	}
	
	// For reading of Boolean values (true or false)
	template<class T> bool ReadBooleanValue(const char *String, T& Value)
	{
		bool BValue;
		if (!XML_GetBooleanValue(String,BValue))
		{
			BadBooleanValue();
			return false;
		}
		Value = T(BValue);
		return true;
	}
	
	// The more common read operations encapsulated into single functions;
	// this is to help make the code less bloated
	
	bool ReadInt16Value(const char *String, int16& Value);
	bool ReadBoundedInt16Value(const char *String, int16& Value, int16 MinVal, int16 MaxVal);
	bool ReadUInt16Value(const char *String, uint16& Value);
	bool ReadBoundedUInt16Value(const char *String, uint16& Value, uint16 MinVal, uint16 MaxVal);
	
	bool ReadInt32Value(const char *String, int32& Value);
	bool ReadBoundedInt32Value(const char *String, int32& Value, int32 MinVal, int32 MaxVal);
	bool ReadUInt32Value(const char *String, uint32& Value);
	bool ReadBoundedUInt32Value(const char *String, uint32& Value, uint32 MinVal, uint32 MaxVal);
	
	bool ReadBooleanValueAsInt16(const char *String, int16& Value);
	bool ReadBooleanValueAsUInt16(const char *String, uint16& Value);
	bool ReadBooleanValueAsInt32(const char *String, int32& Value);
	bool ReadBooleanValueAsUInt32(const char *String, uint32& Value);
	bool ReadBooleanValueAsBool(const char *String, bool& Value);
	
	bool ReadFloatValue(const char *String, float& Value);
		
public:

	// The element's name:
	char *GetName() {return Name;}
	
	// Does a name match it?
	bool NameMatch(const char *_Name);
	
	// If the next routines routines return "false",
	// then set ErrorString to a character string
	// describing the error.
	
	// Start and end processing of an element
	virtual bool Start() {return true;}
	virtual bool End() {return true;}
	
	// Process an attribute tag-value set;
	// need individual parsing and attribute-finish parsing
	virtual bool HandleAttribute(const char * /*Tag*/, const char * /*Value*/) {return true;}
	virtual bool AttributesDone() {return true;}
	
	// Handle embedded string data
	virtual bool HandleString(const char * /*String*/, int /*Length*/) {return true;}

	// Restore all values as they were before any changes were made by this parser.
	virtual bool ResetValues() {return true;}

	// Error-message string
	const char *ErrorString;
	
	// Common kinds of errors:
	void UnrecognizedTag();
	void AttribsMissing();
	void BadNumericalValue();
	void OutOfRange();
	void BadBooleanValue();
	
	// When finished parsing with this object, go to parent element
	XML_ElementParser *Parent;
	
	// Constructor and destructor;
	// needs the element's name.
	XML_ElementParser(const char *_Name);
	virtual ~XML_ElementParser();
	
	// Add a child element
	void AddChild(XML_ElementParser *Child);

	// Call ResetValues() and ResetChildrenValues() for all children. 
	void ResetChildrenValues();

	// Find a child element with a matching name;
	// if one is not found, then this method returns NULL
	XML_ElementParser *FindChild(const char *_Name);
};


#endif
