#ifndef _XML_DATABLOCK_
#define _XML_DATABLOCK_
/*
	Parser of XML-containing Data Blocks
	by Loren Petrich,
	September 14, 2000
	
	This is intended to parse XML objects contained in data blocks in memory
*/


#include "XML_Configure.h"


class XML_DataBlock: public XML_Configure
{
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

	// Parse a data block:
	bool ParseData(char *_Buffer, int _BufLen)
	{Buffer = _Buffer; BufLen = _BufLen; return DoParse();}
		
	XML_DataBlock() {Buffer = NULL;}
};


#endif