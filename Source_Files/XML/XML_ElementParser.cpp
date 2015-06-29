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
	
	This is the base-class implementation of these objects

Dec 25, 2001 (Loren Petrich)
	Made StringsEqual case-independent for the purpose of making parsing of
	XML element names and attribute names case-independent.
*/

#include "cseries.h"

#include <string.h>
#include <ctype.h>
#include "XML_ElementParser.h"


// StringsEqual and DeUTF8 stuff moved to the bottom


bool XML_ElementParser::ReadInt16Value(const char *String, int16& Value)
{
	return ReadNumericalValue(String,"%hd",Value);
}

bool XML_ElementParser::ReadBoundedInt16Value(const char *String, int16& Value, int16 MinVal, int16 MaxVal)
{
	return ReadBoundedNumericalValue(String,"%hd",Value,MinVal,MaxVal);
}

bool XML_ElementParser::ReadUInt16Value(const char *String, uint16& Value)
{
	return ReadNumericalValue(String,"%hu",Value);
}

bool XML_ElementParser::ReadBoundedUInt16Value(const char *String, uint16& Value, uint16 MinVal, uint16 MaxVal)
{
	return ReadBoundedNumericalValue(String,"%hu",Value,MinVal,MaxVal);
}

	
bool XML_ElementParser::ReadInt32Value(const char *String, int32& Value)
{
	return ReadNumericalValue(String,"%d",Value);
}

bool XML_ElementParser::ReadBoundedInt32Value(const char *String, int32& Value, int32 MinVal, int32 MaxVal)
{
	return ReadBoundedNumericalValue(String,"%d",Value,MinVal,MaxVal);
}

bool XML_ElementParser::ReadUInt32Value(const char *String, uint32& Value)
{
	return ReadNumericalValue(String,"%u",Value);
}

bool XML_ElementParser::ReadBoundedUInt32Value(const char *String, uint32& Value, uint32 MinVal, uint32 MaxVal)
{
	return ReadBoundedNumericalValue(String,"%u",Value,MinVal,MaxVal);
}


bool XML_ElementParser::ReadFloatValue(const char *String, float& Value)
{
	return ReadNumericalValue(String,"%f",Value);
}

	
bool XML_ElementParser::ReadBooleanValueAsInt16(const char *String, int16& Value)
{
	return ReadBooleanValue(String,Value);
}

bool XML_ElementParser::ReadBooleanValueAsUInt16(const char *String, uint16& Value)
{
	return ReadBooleanValue(String,Value);
}

bool XML_ElementParser::ReadBooleanValueAsInt32(const char *String, int32& Value)
{
	return ReadBooleanValue(String,Value);
}

bool XML_ElementParser::ReadBooleanValueAsUInt32(const char *String, uint32& Value)
{
	return ReadBooleanValue(String,Value);
}

bool XML_ElementParser::ReadBooleanValueAsBool(const char *String, bool& Value)
{
	return ReadBooleanValue(String,Value);
}


bool XML_GetBooleanValue(const char *String, bool &Value)
{
	if (StringsEqual(String,"1"))
	{
		Value = true;
		return true;
	}
	else if (StringsEqual(String,"t"))
	{
		Value = true;
		return true;
	}
	else if (StringsEqual(String,"true"))
	{
		Value = true;
		return true;
	}
	else if (StringsEqual(String,"0"))
	{
		Value = false;
		return true;
	}
	else if (StringsEqual(String,"f"))
	{
		Value = false;
		return true;
	}
	else if (StringsEqual(String,"false"))
	{
		Value = false;
		return true;
	}
	return false;
}


// Error strings; these are globals so that they can be referenced without confusion.
static char InitialErrorString[] = "initial error string";
static char UnrecognizedTagString[] = "unrecognized tag";
static char AttribsMissingString[] = "attributes missing";
static char BadNumericalValueString[] = "bad numerical value";
static char OutOfRangeString[] = "out of range";
static char BadBooleanValueString[] = "bad boolean value";


bool XML_ElementParser::NameMatch(const char *_Name)
{
	return (StringsEqual(Name,_Name));
}


XML_ElementParser::XML_ElementParser(const char *_Name)
{
	Name = new char[strlen(_Name)+1];
	strcpy(Name,_Name);
	
	// Set it to something reasonable
	ErrorString = InitialErrorString;
}


XML_ElementParser::~XML_ElementParser()
{
	delete []Name;
}

	
// Add a child element; be sure not to add one with the same name twice
void XML_ElementParser::AddChild(XML_ElementParser *Child)
{
	// Check to see if the child has already been added
	char *ChildName = Child->GetName();
	for (unsigned k=0; k<Children.size(); k++)
		if (Children[k]->NameMatch(ChildName)) return;
	
	// Go!
	Children.push_back(Child);
}


// Restore all values as they were before any changes were made for all the children. Recursive.
void XML_ElementParser::ResetChildrenValues()
{
	for (unsigned k = 0; k < Children.size(); k++) {
		Children[k]->ResetValues();
		Children[k]->ResetChildrenValues();
	}
}


XML_ElementParser *XML_ElementParser::FindChild(const char *_Name)
{
	XML_ElementParser *FoundChild = NULL;
	
	for (unsigned k=0; k<Children.size(); k++)
	{
		XML_ElementParser *TestChild = Children[k];
		if (TestChild->NameMatch(_Name))
		{
			FoundChild = TestChild;
			break;
		}
	}
	
	return FoundChild;
}


// Error-message emitters; these use the global definitions given above
void XML_ElementParser::UnrecognizedTag() {ErrorString = UnrecognizedTagString;}
void XML_ElementParser::AttribsMissing() {ErrorString = AttribsMissingString;}
void XML_ElementParser::BadNumericalValue() {ErrorString = BadNumericalValueString;}
void XML_ElementParser::OutOfRange() {ErrorString = OutOfRangeString;}
void XML_ElementParser::BadBooleanValue() {ErrorString = BadBooleanValueString;}


bool StringsEqual(const char *String1, const char *String2, int MaxStrLen)
{
	// Convert and do the comparison by hand:
	const char *S1 = String1;
	const char *S2 = String2;
	
	for (int k=0; k<MaxStrLen; k++, S1++, S2++)
	{
		// Make the characters the same case
		char c1 = toupper(*S1);
		char c2 = toupper(*S2);
		
		// Compare!
		if (c1 == 0 && c2 == 0) return true;	// All in both strings equal
		else if (c1 != c2) return false;		// At least one unequal
		// else equal but non-terminating; continue comparing
	}
	
	// All those within the length range are equal
	return true;
}


// For turning UTF-8 strings into plain ASCII ones;
// needs at least (OutMaxLen) characters preallocated.
// Will not null-terminate the string or Pascalify it.
// Returns how many characters resulted.

size_t DeUTF8(const char *InString, size_t InLen, char *OutString, size_t OutMaxLen)
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
