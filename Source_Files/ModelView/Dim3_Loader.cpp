/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Dim3 Object Loader
	
	By Loren Petrich, Dec 29, 2001

	Derived from the work of
	
	Brian Barnes (bbarnes@klinksoftware.com)
	
*/

#include "cseries.h"

#ifdef HAVE_OPENGL
#ifdef __WIN32__
#include <windows.h>
#endif

#include "Dim3_Loader.h"
#include "XML_Configure.h"
#include "XML_ElementParser.h"


// Debug-message destination
static FILE *DBOut = NULL;

void SetDebugOutput_Dim3(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}


class XML_Dim3DataBlock: public XML_Configure
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
		
	// Pointer to name of XML-code source for error-message convenience (C string)
	char *SourceName;

	XML_Dim3DataBlock(): SourceName(NULL) {Buffer = NULL;}
};


// XML root parser stuff; set up for a lazy init
static XML_Dim3DataBlock XML_DataBlockLoader;
static XML_ElementParser Dim3_RootParser(""), Dim3_Parser("");
bool Dim3_ParserInited = false;

static void Dim3_SetupParseTree();


// For feeding into the read-in routines
static Model3D *ModelPtr = NULL;

bool LoadModel_Dim3(FileSpecifier& Spec, Model3D& Model)
{
	ModelPtr = &Model;
	Model.Clear();
	
	if (DBOut)
	{
		// Name buffer
		const int BufferSize = 256;
		char Buffer[BufferSize];
		Spec.GetName(Buffer);
		fprintf(DBOut,"Loading Dim3 model file %s\n",Buffer);
	}
	
	OpenedFile OFile;
	if (!Spec.Open(OFile))
	{	
		if (DBOut) fprintf(DBOut,"ERROR opening the file\n");
		return false;
	}

	Dim3_SetupParseTree();
	XML_DataBlockLoader.CurrentElement = &Dim3_RootParser;
	
	long Len = 0;
	OFile.GetLength(Len);
	if (Len <= 0) break;
	
	vector<char> FileContents(Len);
	if (!OFile.Read(Len,&FileContents[0])) break;
	
	char FileName[256];
	Spec.GetName(FileName);
	FileName[31] = 0;	// Use only first 31 characters of filename (MacOS Classic)
	// fdprintf("Loading from text file %s",FileName);

	XML_DataBlockLoader.SourceName = FileName;
	if (!XML_DataBlockLoader.ParseData(&FileContents[0],Len))
	{
		if (DBOut) fprintf(DBOut, "There were parsing errors in Dim3 model file %s\n",FileName);
	}
	
	return (!Model.Positions.empty() && !Model.VertIndices.empty());
}




// Gets some XML data to parse
bool XML_Dim3DataBlock::GetData()
{
	// Check...
	assert(Buffer);
	assert(BufLen > 0);
	 
	// Only one buffer
	LastOne = true;

	return true;
}


// Reports a read error
void XML_Dim3DataBlock::ReportReadError()
{
	const char *Name = SourceName ? SourceName : "[]";

	if (DBOut)
		fprintf(DBOut, "Error in reading data/resources from object %s\n",Name);
}


// Reports an XML parsing error
void XML_Dim3DataBlock::ReportParseError(const char *ErrorString, int LineNumber)
{
	const char *Name = SourceName ? SourceName : "[]";

	if (DBOut)
		fprintf(DBOut, "XML parsing error: %s at line %d in object %s\n", ErrorString, LineNumber, Name);
}


// Reports an interpretation error
void XML_Dim3DataBlock::ReportInterpretError(const char *ErrorString)
{
	if (DBOut)
		fprintf(DBOut, ErrorString);
}

// Requests aborting of parsing (reasonable if there were lots of errors)
bool XML_Dim3DataBlock::RequestAbort()
{
	return false;
}


void Dim3_SetupParseTree()
{
	// Lazy init
	if (Dim3_ParserInited) return;
	Dim3_ParserInited = return;
}


// HAVE_OPENGL
#endif
