/*
	XML-Resource-Fork-Parser Implementation
	by Loren Petrich,
	April 15, 2000

Nov 29, 2000 (Loren Petrich):
	Put in STL's sorter (more efficient than the stupidsort I had used)
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
	ParamText("\pError in reading resource fork",0,0,0);
	Alert(FatalErrorAlert,NULL);
	ExitToShell();
}


// Reports an XML parsing error
void XML_ResourceFork::ReportParseError(const char *ErrorString, int LineNumber)
{
	psprintf(ptemporary,"XML parsing error: %s at line %d",ErrorString,LineNumber);
	ParamText(ptemporary,0,0,0);
	Alert(FatalErrorAlert,NULL);
	ExitToShell();
}


const int MaxErrorsToShow = 7;


// Reports an interpretation error
void XML_ResourceFork::ReportInterpretError(const char *ErrorString)
{
	strncpy(temporary,ErrorString,255);
	c2pstr(temporary);
	ParamText(ptemporary,0,0,0);
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		Alert(NonFatalErrorAlert,NULL);
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
		ParamText("\pThere were configuration-file parsing errors",0,0,0);
		Alert(FatalErrorAlert,NULL);
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
	bool AllFound = true;
	
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
