/*
	XML-Data-Block-Parser Implementation
	by Loren Petrich,
	September 14, 2000
*/


#include <string.h>
#include "cseries.h"
#include "XML_DataBlock.h"


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
#ifdef mac
	ParamText("\pError in reading data block",0,0,0);
	Alert(FatalErrorAlert,NULL);
	ExitToShell();
#endif
	
#ifdef SDL
	fprintf(stderr, "Error in reading resources\n");
	exit(1);
#endif
}


// Reports an XML parsing error
void XML_DataBlock::ReportParseError(const char *ErrorString, int LineNumber)
{
#ifdef mac
	psprintf(ptemporary,"XML parsing error: %s at line %d",ErrorString,LineNumber);
	ParamText(ptemporary,0,0,0);
	Alert(FatalErrorAlert,NULL);
	ExitToShell();
#endif

#ifdef SDL
	fprintf(stderr, "XML parsing error: %s at line %d\n", ErrorString, LineNumber);
	exit(1);
#endif
}


const int MaxErrorsToShow = 7;


// Reports an interpretation error
void XML_DataBlock::ReportInterpretError(const char *ErrorString)
{
#ifdef mac
	strncpy(temporary,ErrorString,255);
	c2pstr(temporary);
	ParamText(ptemporary,0,0,0);
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		Alert(NonFatalErrorAlert,NULL);
#endif

#ifdef SDL
	if (GetNumInterpretErrors() < MaxErrorsToShow)
		fprintf(stderr, ErrorString);
#endif
}

// Requests aborting of parsing (reasonable if there were lots of errors)
bool XML_DataBlock::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}
