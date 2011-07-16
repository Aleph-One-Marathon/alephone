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

	XML-Data-Block-Parser Implementation
	by Loren Petrich,
	September 14, 2000
	
Oct 24, 2001 (Loren Petrich):
	Added "SourceName" field for indicating source of XML data for easier debugging

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added CopyCStringToPascal for Carbon, in place of c2pstr

April 15, 2003 (Woody Zenfell):
        Made XML interpretation error reporting less obnoxious (uses Logging)
*/


#include <string.h>
#include "cseries.h"
#include "XML_DataBlock.h"
#include "Logging.h"


#ifdef mac
// Marathon-engine dialog boxes:
const short FatalErrorAlert = 128;
const short NonFatalErrorAlert = 129;
#endif


// Gets some XML data to parse
bool XML_DataBlock::GetData()
{
	// Check...
	assert(Buffer);
	assert(BufLen > 0);
	 
	// Only one buffer
	LastOne = true;

	return true;
}


// Reports a read error
void XML_DataBlock::ReportReadError()
{
	const char *Name = SourceName ? SourceName : "[]";

#ifdef mac
#ifdef TARGET_API_MAC_CARBON
	csprintf(temporary,
		"Error in reading data/resources from object %s",Name);
	SimpleAlert(kAlertStopAlert,temporary);
#else
	psprintf(ptemporary,"Error in reading data/resources from object %s",Name);
	ParamText(ptemporary,0,0,0);
	Alert(FatalErrorAlert,NULL);
#endif
	ExitToShell();
#endif
	
#ifdef SDL
	fprintf(stderr, "Error in reading data/resources from object %s\n",Name);
	exit(1);
#endif
}


// Reports an XML parsing error
void XML_DataBlock::ReportParseError(const char *ErrorString, int LineNumber)
{
	const char *Name = SourceName ? SourceName : "[]";

#ifdef mac
#ifdef TARGET_API_MAC_CARBON
	csprintf(temporary,
		"XML parsing error: %s at line %d in object %s",ErrorString,LineNumber,Name);
	SimpleAlert(kAlertStopAlert,temporary);
#else
	psprintf(ptemporary,"XML parsing error: %s at line %d in object %s",ErrorString,LineNumber,Name);
	ParamText(ptemporary,0,0,0);
	Alert(FatalErrorAlert,NULL);
#endif
	ExitToShell();
#endif

#ifdef SDL
	fprintf(stderr, "XML parsing error: %s at line %d in object %s\n", ErrorString, LineNumber, Name);
#endif
}


const int MaxErrorsToShow = 7;


// Reports an interpretation error
void XML_DataBlock::ReportInterpretError(const char *ErrorString)
{
        if(GetNumInterpretErrors() < MaxErrorsToShow)
                logAnomaly1("%s", ErrorString);
        else if(GetNumInterpretErrors() == MaxErrorsToShow)
                logAnomaly("(more errors not shown)");
}

// Requests aborting of parsing (reasonable if there were lots of errors)
bool XML_DataBlock::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}
