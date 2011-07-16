#ifndef _XML_RESOURCEFORK_
#define _XML_RESOURCEFORK_
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

	Parser of XML-containing Resource Forks
	by Loren Petrich,
	April 15, 2000
	
	This is intended to parse XML objects contained in MacOS resource forks
	
Oct 24, 2001 (Loren Petrich):
	Added "SourceName" field for indicating source of XML data for easier debugging
*/


#include "XML_Configure.h"


class XML_ResourceFork: public XML_Configure
{
	
	// Handle to the resource (assumes it was completely read in)
	Handle ResourceHandle;
	
	// Gets some XML data to parse
	bool GetData();
	
	// Reports a read error
	void ReportReadError();
	
	// Reports an XML parsing error
	void ReportParseError(const char *ErrorString, int LineNumber);
	
	// Reports an interpretation error
	void ReportInterpretError(const char *ErrorString);
	
	// Requests aborting of parsing (reasonable if there were lots of errors)
	bool RequestAbort();

public:

	// Parse a single resource;
	// returns whether the resource exists
	bool ParseResource(ResType Type, short ID);
	
	// Parse all resources in a set;
	// returns whether any resources were found
	bool ParseResourceSet(ResType Type);
	
	// Pointer to name of XML-code source for error-message convenience (C string)
	char *SourceName;
	
	XML_ResourceFork(): ResourceHandle(NULL), SourceName(NULL) {}
};



#endif
