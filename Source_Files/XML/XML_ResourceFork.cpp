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

	XML-Resource-Fork-Parser Implementation
	by Loren Petrich,
	April 15, 2000

Nov 29, 2000 (Loren Petrich):
	Put in STL's sorter (more efficient than the stupidsort I had used)
	
Oct 24, 2001 (Loren Petrich):
	Added "SourceName" field for indicating source of XML data for easier debugging

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added CopyCStringToPascal for Carbon, in place of c2pstr
*/


#include <algorithm>
#include <string.h>
#include "cseries.h"
#include "XML_ResourceFork.h"

// Marathon-engine dialog boxes:
const short FatalErrorAlert = 128;
const short NonFatalErrorAlert = 129;


// Gets some XML data to parse
bool XML_ResourceFork::GetData()
{
	if (!ResourceHandle) return false;

	// Dereference
	Buffer = *ResourceHandle;
	BufLen = GetHandleSize(ResourceHandle);
	LastOne = true;

	return true;
}


// Reports a read error
void XML_ResourceFork::ReportReadError()
{
	const char *Name = SourceName ? SourceName : "[]";
#ifdef TARGET_API_MAC_CARBON
	csprintf(temporary,
		"Error in reading resource fork of object %s",Name);
	SimpleAlert(kAlertStopAlert,temporary);
#else
	psprintf(ptemporary,
		"Error in reading resource fork of object %s",Name);
	ParamText(ptemporary,0,0,0);
	Alert(FatalErrorAlert,NULL);
#endif
	ExitToShell();
}


// Reports an XML parsing error
void XML_ResourceFork::ReportParseError(const char *ErrorString, int LineNumber)
{
	const char *Name = SourceName ? SourceName : "[]";
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
}


const int MaxErrorsToShow = 7;


// Reports an interpretation error
void XML_ResourceFork::ReportInterpretError(const char *ErrorString)
{
#ifdef TARGET_API_MAC_CARBON
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		SimpleAlert(kAlertNoteAlert,ErrorString);
#else
	CopyCStringToPascal(ErrorString, ptemporary);
	ParamText(ptemporary,0,0,0);
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		Alert(NonFatalErrorAlert,NULL);
#endif
}

// Requests aborting of parsing (reasonable if there were lots of errors)
bool XML_ResourceFork::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}


// Parse a single resource
bool XML_ResourceFork::ParseResource(ResType Type, short ID)
{
	ResourceHandle = Get1Resource(Type,ID);
	if (ResourceHandle == NULL) return false;
	
	HLock(ResourceHandle);
	if (!DoParse())
	{
		const char *Name = SourceName ? SourceName : "[]";
#ifdef TARGET_API_MAC_CARBON
		csprintf(temporary,
			"There were configuration-file parsing errors in resource %hd of object %s",ID,Name);
		SimpleAlert(kAlertStopAlert,temporary);
#else
		psprintf(ptemporary,
			"There were configuration-file parsing errors in resource %hd of object %s",ID,Name);
		ParamText(ptemporary,0,0,0);
		Alert(FatalErrorAlert,NULL);
#endif
		ExitToShell();
	}
	HUnlock(ResourceHandle);
	ReleaseResource(ResourceHandle);
	return true;
}
	
// Parse all resources in a set
bool XML_ResourceFork::ParseResourceSet(ResType Type)
{
	short NumResources = Count1Resources(Type);
	
	if (NumResources <= 0) return false;
	
	// This sorting is necessary so that the resources will be read in ascending order of ID's
	short *IDList = new short[NumResources];
	
	// Get the resource ID's
	SetResLoad(false);
	
	for (int ir=0; ir<NumResources; ir++)
	{
		// Zero-based to one-based indexing
		ResourceHandle = Get1IndResource(Type,ir+1);
		ResType _Type;
		Str255 Name;
		GetResInfo(ResourceHandle,&IDList[ir],&_Type,Name);
		ReleaseResource(ResourceHandle);
	}
	
	SetResLoad(true);
	
	// Using a better sorter; STL will succeed where Rhys Hill had failed :-)
	sort(IDList,IDList+NumResources);
	
	// Now get the resources
	for (int ir=0; ir<NumResources; ir++)
	{
		ParseResource(Type,IDList[ir]);
	}
	
	delete []IDList;
	return true;
}
