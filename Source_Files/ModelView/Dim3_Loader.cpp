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


// XML root parser stuff; set up for a lazy init.
// Child parsers are toward the end of the file.
static XML_Dim3DataBlock XML_DataBlockLoader;
static XML_ElementParser Dim3_RootParser(""), Dim3_Parser("Model");
bool Dim3_ParserInited = false;

static void Dim3_SetupParseTree();


// Bone-tag intermediate arrays:

const int BoneTagSize = 8;

struct BoneTagWrapper
{
	char Tag0[BoneTagSize], Tag1[BoneTagSize];
};

// For VertexBoneTags, this means major bone tag, then minor bone tag.
// For BoneOwnTags, this means its own tag, then its parent tag.
static vector<BoneTagWrapper> VertexBoneTags, BoneOwnTags;


// For feeding into the read-in routines
static Model3D *ModelPtr = NULL;

bool LoadModel_Dim3(FileSpecifier& Spec, Model3D& Model)
{
	ModelPtr = &Model;
	// Clear everything
	Model.Clear();
	VertexBoneTags.clear();
	BoneOwnTags.clear();
	
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
	if (Len <= 0) return false;
	
	vector<char> FileContents(Len);
	if (!OFile.Read(Len,&FileContents[0])) return false;
	
	char FileName[256];
	Spec.GetName(FileName);
	FileName[31] = 0;	// Use only first 31 characters of filename (MacOS Classic)
	// fdprintf("Loading from text file %s",FileName);
	
	XML_DataBlockLoader.SourceName = FileName;
	if (!XML_DataBlockLoader.ParseData(&FileContents[0],Len))
	{
		if (DBOut) fprintf(DBOut, "There were parsing errors in Dim3 model file %s\n",FileName);
	}
	
	// Fill in the blanks
	
	// First, find the neutral-position vertices
	
	int NumVertices = Model.VtxSrcIndices.size();
	Model.Positions.resize(3*NumVertices);
	
	GLfloat *PP = Model.PosBase();
	GLushort *IP = Model.VtxSIBase();
	
	int NumVtxSources = Model.VtxSources.size();
	
	for (int k=0; k<NumVertices; k++)
	{
		int VSIndex = *(IP++);
		if (VSIndex >= 0 && VSIndex < NumVtxSources)
		{
			Model3D_VertexSource& VS = Model.VtxSources[VSIndex];
			GLfloat *VP = VS.Position;
			*(PP++) = *(VP++);
			*(PP++) = *(VP++);
			*(PP++) = *(VP++);
		}
		else
		{
			*(PP++) = 0;
			*(PP++) = 0;
			*(PP++) = 0;
		}
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
		fprintf(DBOut, "%s\n",ErrorString);
}

// Requests aborting of parsing (reasonable if there were lots of errors)
bool XML_Dim3DataBlock::RequestAbort()
{
	return false;
}


// Dummy elements:
static XML_ElementParser
	CreatorParser("Creator"),
	ViewBoxParser("View_Box"),
	VerticesParser("Vertexes"),
	BonesParser("Bones"),
	EffectsParser("Effects"),
	EffectParser("Effect"),
	FillsParser("Fills"),
	FillParser("Fill"),
	D3ColorsParser("Colors"),
	D3ColorParser("Color"),
	D3ImagesParser("Images"),
	D3ImageParser("Image"),
	TrianglesParser("Triangles");


// "Real" elements:

class XML_BoundingBoxParser: public XML_ElementParser
{
	GLfloat x_size, y_size, z_size, x_offset, y_offset, z_offset;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_BoundingBoxParser(): XML_ElementParser("Bound_Box") {}
};



bool XML_BoundingBoxParser::Start()
{
	x_size = y_size = z_size = x_offset = y_offset = z_offset = 0;
	
	return true;
}

bool XML_BoundingBoxParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"x_size"))
	{
		return ReadFloatValue(Value,x_size);
	}
	else if (StringsEqual(Tag,"y_size"))
	{
		return ReadFloatValue(Value,y_size);
	}
	else if (StringsEqual(Tag,"z_size"))
	{
		return ReadFloatValue(Value,z_size);
	}
	else if (StringsEqual(Tag,"x_offset"))
	{
		return ReadFloatValue(Value,x_offset);
	}
	else if (StringsEqual(Tag,"y_offset"))
	{
		return ReadFloatValue(Value,y_offset);
	}
	else if (StringsEqual(Tag,"z_offset"))
	{
		return ReadFloatValue(Value,z_offset);
	}

	UnrecognizedTag();
	return false;
}

bool XML_BoundingBoxParser::AttributesDone()
{
	// Inconsistent resizing: weird bug in ggadwa's code
	
	ModelPtr->BoundingBox[0][0] = x_offset - x_size/2;
	ModelPtr->BoundingBox[0][1] = y_offset - y_size;
	ModelPtr->BoundingBox[0][2] = z_offset - z_size/2;
	
	ModelPtr->BoundingBox[1][0] = x_offset + x_size/2;
	ModelPtr->BoundingBox[1][1] = y_offset;
	ModelPtr->BoundingBox[1][2] = z_offset + z_size/2;
	
	return true;
}

static XML_BoundingBoxParser BoundingBoxParser;


class XML_VertexParser: public XML_ElementParser
{
	Model3D_VertexSource Data;
	
	// For adding to the bone-tag array as each vertex is added
	BoneTagWrapper BT;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_VertexParser(): XML_ElementParser("v") {}
};


bool XML_VertexParser::Start()
{
	for (int c=0; c<3; c++)
		Data.Position[c] = 0;
	
	// Initially: no bones
	Data.Bone0 = Data.Bone1 = NONE;
	Data.Blend = 0;
	
	// No bone: zero-length strings:
	BT.Tag0[0] = BT.Tag1[0] = 0;
	
	return true;
}

bool XML_VertexParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"x"))
	{
		return ReadFloatValue(Value,Data.Position[0]);
	}
	else if (StringsEqual(Tag,"y"))
	{
		return ReadFloatValue(Value,Data.Position[1]);
	}
	else if (StringsEqual(Tag,"z"))
	{
		return ReadFloatValue(Value,Data.Position[2]);
	}
	else if (StringsEqual(Tag,"major"))
	{
		strncpy(BT.Tag0,Value,BoneTagSize);
		return true;
	}
	else if (StringsEqual(Tag,"minor"))
	{
		strncpy(BT.Tag1,Value,BoneTagSize);
		return true;
	}
	else if (StringsEqual(Tag,"factor"))
	{
		GLfloat Factor;
		if (ReadFloatValue(Value,Factor))
		{
			// Convert from ggadwa's definition (100 to 0) to mine (0 to 1)
			// for first to second bone.
			Data.Blend = 1 - Factor/100;
			return true;
		}
		else return false;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_VertexParser::AttributesDone()
{
	// Always handle the bone data, even for a blank bone, to maintain coherence.
	ModelPtr->VtxSources.push_back(Data);
	VertexBoneTags.push_back(BT);
	
	return true;
}

static XML_VertexParser VertexParser;


class XML_BoneParser: public XML_ElementParser
{
	Model3D_Bone Data;
	
	// For adding to the bone-tag array as each bone is added
	BoneTagWrapper BT;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_BoneParser(): XML_ElementParser("Bone") {}
};


bool XML_BoneParser::Start()
{
	for (int c=0; c<3; c++)
		Data.Position[c] = 0;
	
	// Initially: descent only from the assumed root bone
	Data.Flags = Model3D_Bone::Pop;
	
	// No bone: zero-length strings:
	BT.Tag0[0] = BT.Tag1[0] = 0;
	
	return true;
}

bool XML_BoneParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"x"))
	{
		return ReadFloatValue(Value,Data.Position[0]);
	}
	else if (StringsEqual(Tag,"y"))
	{
		return ReadFloatValue(Value,Data.Position[1]);
	}
	else if (StringsEqual(Tag,"z"))
	{
		return ReadFloatValue(Value,Data.Position[2]);
	}
	else if (StringsEqual(Tag,"tag"))
	{
		strncpy(BT.Tag0,Value,BoneTagSize);
		return true;
	}
	else if (StringsEqual(Tag,"parent"))
	{
		strncpy(BT.Tag1,Value,BoneTagSize);
		return true;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_BoneParser::AttributesDone()
{
	// Always handle the bone data, even for a blank bone, to maintain coherence.
	ModelPtr->Bones.push_back(Data);
	BoneOwnTags.push_back(BT);
	
	return true;
}

static XML_BoneParser BoneParser;



class XML_TriVertexParser: public XML_ElementParser
{
	uint16 ID;
	float Txtr_X, Txtr_Y;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_TriVertexParser(): XML_ElementParser("v") {}
};


bool XML_TriVertexParser::Start()
{
	// Reasonable defaults:
	ID = NONE;
	Txtr_X = 0.5, Txtr_Y = 0.5;
	
	return true;
}

bool XML_TriVertexParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"ID"))
	{
		return ReadUInt16Value(Value,ID);
	}
	else if (StringsEqual(Tag,"xtxt"))
	{
		return ReadFloatValue(Value,Txtr_X);
	}
	else if (StringsEqual(Tag,"ytxt"))
	{
		return ReadFloatValue(Value,Txtr_Y);
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_TriVertexParser::AttributesDone()
{
	GLushort Index = ModelPtr->VertIndices.size();
	ModelPtr->VertIndices.push_back(Index);
	ModelPtr->VtxSrcIndices.push_back(ID);
	ModelPtr->TxtrCoords.push_back(Txtr_X);
	ModelPtr->TxtrCoords.push_back(Txtr_Y);	
	return true;
}

static XML_TriVertexParser TriVertexParser;


void Dim3_SetupParseTree()
{
	// Lazy init
	if (Dim3_ParserInited) return;

	// Set up the root object
	Dim3_RootParser.AddChild(&Dim3_Parser);
	
	Dim3_Parser.AddChild(&CreatorParser);
	Dim3_Parser.AddChild(&BoundingBoxParser);	
	Dim3_Parser.AddChild(&ViewBoxParser);

	VerticesParser.AddChild(&VertexParser);
	Dim3_Parser.AddChild(&VerticesParser);
	
	BonesParser.AddChild(&BoneParser);
	Dim3_Parser.AddChild(&BonesParser);
	
	EffectsParser.AddChild(&EffectParser);
	Dim3_Parser.AddChild(&EffectsParser);
	
	D3ColorsParser.AddChild(&D3ColorParser);
	D3ImagesParser.AddChild(&D3ImageParser);
	TrianglesParser.AddChild(&TriVertexParser);
	
	FillParser.AddChild(&D3ColorsParser);
	FillParser.AddChild(&D3ImagesParser);
	FillParser.AddChild(&TrianglesParser);
	
	FillsParser.AddChild(&FillParser);
	Dim3_Parser.AddChild(&FillsParser);
	
	Dim3_ParserInited = true;
}


// HAVE_OPENGL
#endif
