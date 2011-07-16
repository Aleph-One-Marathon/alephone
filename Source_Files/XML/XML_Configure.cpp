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

	XML-Configuration-Parser Implementation
	by Loren Petrich,
	April 14, 2000
	
	May 2, 2000
	Changed error message for unrecognized child element; it no longer has "at line %s" at the end,
	since that is redundant.
*/


#include <stdio.h>
#include <stdarg.h>
#include "cseries.h"
#include "XML_Configure.h"


void XML_Configure::StaticStartElement(void *UserData, const char *Name, const char **Attributes)
{
	XML_Configure *This = (XML_Configure *)UserData;
	This->StartElement(Name,Attributes);
}

void XML_Configure::StaticEndElement(void *UserData, const char *Name)
{
	XML_Configure *This = (XML_Configure *)UserData;
	This->EndElement(Name);
}

void XML_Configure::StaticCharacterData(void *UserData, const char *String, int Length)
{
	XML_Configure *This = (XML_Configure *)UserData;
	This->CharacterData(String, Length);
}


// Approriate calls on this object
void XML_Configure::StartElement(const char *Name, const char **Attributes)
{
	// Check to see if the current element has a child with that name
	XML_ElementParser *ChildElement = CurrentElement->FindChild(Name);
	
	if (ChildElement)
	{
		// A child was found: make it the current element
		ChildElement->Parent = CurrentElement;
		CurrentElement = ChildElement;
	}
	else
	{
		ComposeInterpretError("Error at line %d: element of type %s has no child of type %s",
			XML_GetCurrentLineNumber(Parser),CurrentElement->GetName(),Name);
		return;
	}

	// Start with the element
	if (!CurrentElement->Start())
	{
		ComposeInterpretError("Error at line %d in starting with element of type %s: %s",
			XML_GetCurrentLineNumber(Parser),Name,CurrentElement->ErrorString);
	}

	// Handle its attributes
	const char **AttrPtr = Attributes;
	while(*AttrPtr)
	{
		const char *Tag = *(AttrPtr++);
		const char *Value = *(AttrPtr++);
		assert(Value);
		if (!CurrentElement->HandleAttribute(Tag,Value))
		{
			ComposeInterpretError("Error at line %d with element of type %s, attribute tag %s and value %s: %s",
				XML_GetCurrentLineNumber(Parser),Name,Tag,Value,CurrentElement->ErrorString);
		}
	}
	
	// Finish with attribute handling before moving on to other things
	if (!CurrentElement->AttributesDone())
	{
		ComposeInterpretError("Error at line %d in handling the attributes of element of type %s: %s",
			XML_GetCurrentLineNumber(Parser),Name,CurrentElement->ErrorString);
	}
}

void XML_Configure::EndElement(const char *Name)
{

	// Skip if current element does not have this name,
	// as can happen for an unrecognized child:
	if (!CurrentElement->NameMatch(Name)) return;

	// Finish with an element
	if (!CurrentElement->End())
	{
		ComposeInterpretError("Error at line %d in finishing with element of type %s: %s",
			XML_GetCurrentLineNumber(Parser),Name,CurrentElement->ErrorString);
	}
	CurrentElement = CurrentElement->Parent;
}

void XML_Configure::CharacterData(const char *String, int Length)
{
	if (!CurrentElement->HandleString(String,Length))
	{
		ComposeInterpretError("Error at line %d in character data for element of type %s: %s",
			CurrentElement->GetName(),XML_GetCurrentLineNumber(Parser),CurrentElement->ErrorString);
	}
}
	

// Does parsing; indicates whether the parsing was successful or not
bool XML_Configure::DoParse()
{
	// Sanity check
	if (!CurrentElement) return false;

	// Create the parser
	Parser = XML_ParserCreate("iso-8859-1");
	
	// Set up the callbacks
	XML_SetUserData(Parser, this);
	XML_SetElementHandler(Parser, StaticStartElement, StaticEndElement);
	XML_SetCharacterDataHandler(Parser, StaticCharacterData);
	
	NumInterpretErrors = 0;
	LastOne = true;
	do
	{
		if (!GetData())
		{
			// Report a read error
			ReportReadError();
			XML_ParserFree(Parser);
			return false;
		}

		// Expat should really be using size_t for BufLen
		if (!XML_Parse(Parser, Buffer, (int)BufLen, LastOne))
		{
			// Report a parse error
			ReportParseError(
				XML_ErrorString(XML_GetErrorCode(Parser)),
					XML_GetCurrentLineNumber(Parser));

			XML_ParserFree(Parser);
			return false;
		}
		if (RequestAbort())
		{
			XML_ParserFree(Parser);
			return false;
		}
	}
	while (!LastOne);

	XML_ParserFree(Parser);
	return (NumInterpretErrors == 0);
}


// Compose an interpretation error; use <>printf syntax
void XML_Configure::ComposeInterpretError(const char *Format, ...)
{
	char ErrorString[256];

	va_list List;
	va_start(List,Format);
	vsprintf(ErrorString,Format,List);
	va_end(List);
	
	NumInterpretErrors++;
	ReportInterpretError(ErrorString);
}
