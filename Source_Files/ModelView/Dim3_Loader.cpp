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

#include <math.h>

#include "Dim3_Loader.h"
#include "world.h"
#include "XML_Configure.h"
#include "XML_ElementParser.h"


const float DegreesToInternal = float(FULL_CIRCLE)/float(360);

// Convert angle from degrees to the Marathon engine's internal units
static int16 GetAngle(float InAngle)
{
	float A = DegreesToInternal*InAngle;
	int16 IA = (A >= 0) ? int16(A + 0.5) : - int16(-A + 0.5);
	return NORMALIZE_ANGLE(IA);
}


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


// Local globals; these are to be persistent across calls when loading several files.

// Bone-tag and name-tag intermediate arrays:

const int BoneTagSize = 8;

struct BoneTagWrapper
{
	char Tag0[BoneTagSize], Tag1[BoneTagSize];
};

// For VertexBoneTags, this means major bone tag, then minor bone tag.
// For BoneOwnTags, this means its own tag, then its parent tag.
static vector<BoneTagWrapper> VertexBoneTags, BoneOwnTags;

// Translation from read-in bone order to "true" order
static vector<size_t> BoneIndices;

// Names of frames and seqeunces:

const int NameTagSize = 32;

struct NameTagWrapper
{
	char Tag[NameTagSize];
};

static vector<NameTagWrapper> FrameTags;

// Where the data for each frame goes before it's loaded into the model array;
// the bones may be only partially listed or not listed at all.
static vector<Model3D_Frame> ReadFrame;

// Normals (per vertex source)
static vector<GLfloat> Normals;


// For feeding into the read-in routines
static Model3D *ModelPtr = NULL;

bool LoadModel_Dim3(FileSpecifier& Spec, Model3D& Model, int WhichPass)
{
	ModelPtr = &Model;
	
	if (WhichPass == LoadModelDim3_First)
	{
		// Clear everything
		Model.Clear();
		VertexBoneTags.clear();
		BoneOwnTags.clear();
		BoneIndices.clear();
		FrameTags.clear();
		Normals.clear();
	}
	
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
	
	int32 Len = 0;
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
	
	// Set these up now
	if (Model.InverseVSIndices.empty()) Model.BuildInverseVSIndices();
	
	if (!Normals.empty())
	{
		Model.NormSources.resize(3*Model.VtxSrcIndices.size());
		Model.Normals.resize(Model.NormSources.size());
		for (unsigned k=0; k<Model.VtxSources.size(); k++)
		{
			GLfloat *Norm = &Normals[3*k];
			GLushort P0 = Model.InvVSIPointers[k];
			GLushort P1 = Model.InvVSIPointers[k+1];
			for (int p=P0; p<P1; p++)
			{
				GLfloat *DestNorm = &Model.NormSources[3*Model.InverseVSIndices[p]];
				for (int c=0; c<3; c++)
					DestNorm[c] = Norm[c];
			}
		}
		// All done with them
		
		Normals.clear();
	}
	
	// First, find the neutral-position vertices
	Model.FindPositions_Neutral(false);
	
	// Work out the sorted order for the bones; be sure not to repeat this if already done.
	if (BoneIndices.empty() && !Model.Bones.empty())
	{
		size_t NumBones = Model.Bones.size();
		BoneIndices.resize(NumBones);
		fill(BoneIndices.begin(),BoneIndices.end(),(size_t)UNONE);	// No bones listed -- yet
		vector<Model3D_Bone> SortedBones(NumBones);
		vector<size_t> BoneStack(NumBones);
		vector<bool> BonesUsed(NumBones);
		fill(BonesUsed.begin(),BonesUsed.end(),false);
		
		// Add the bones, one by one;
		// the bone stack's height is originally zero
		int StackTop = -1;
		for (vector<size_t>::value_type ib=0; ib<NumBones; ib++)
		{		
			// Scan down the bone stack to find a bone that's the parent of some unlisted bone;
			vector<size_t>::value_type ibsrch = 
				static_cast<vector<size_t>::value_type>(NumBones);	// "Bone not found" value
			int ibstck = -1;		// Empty stack
			for (ibstck=StackTop; ibstck>=0; ibstck--)
			{
				// Note: the bone stack is indexed relative to the original,
				// as is the bones-used list
				char *StackBoneTag = BoneOwnTags[BoneStack[ibstck]].Tag0;
				for (ibsrch=0; ibsrch<NumBones; ibsrch++)
				{
					if (BonesUsed[ibsrch]) continue;
					char *ParentTag = BoneOwnTags[ibsrch].Tag1;
					if (strncmp(ParentTag,StackBoneTag,BoneTagSize)==0)
						break;
				}
				// If a bone was found, then readjust the stack size appropriately and quit.
				if (ibsrch < NumBones)
				{
					if (ibstck < StackTop)
					{
						// Be sure to get the traversal push/pop straight.
						Model.Bones[BoneStack[ibstck+1]].Flags |= Model3D_Bone::Push;
						Model.Bones[ibsrch].Flags |= Model3D_Bone::Pop;
						StackTop = ibstck;
					}
					break;
				}
			}
			// If none was found, then the bone's parent is the assumed root bone.
			if (ibstck < 0)
			{
				for (ibsrch=0; ibsrch<NumBones; ibsrch++)
				{
					if (BonesUsed[ibsrch]) continue;
					
					// Check if the parent is not one of the bones
					char *ParentTag = BoneOwnTags[ibsrch].Tag1;
					size_t ibsx;
					for (ibsx=0; ibsx<NumBones; ibsx++)
					{
						if (strncmp(ParentTag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
							break;
					}
					// If a match was not found, then quit searching
					if (ibsx >= NumBones) break;
				}
				
				// Not sure how to handle this sort of error;
				// it could be produced by circular bone references:
				// B1 -> B2 -> B3 -> ... -> B1
				assert(ibsrch < NumBones);
				
				// Be sure to get the traversal push/pop straight.
				if (StackTop >= 0)
				{
					Model.Bones[BoneStack[0]].Flags |= Model3D_Bone::Push;
					Model.Bones[ibsrch].Flags |= Model3D_Bone::Pop;
					StackTop = -1;
				}
			}
			
			// Add the bone to the stack
			BoneStack[++StackTop] = ibsrch;
			
			// Don't look for it anymore
			BonesUsed[ibsrch] = true;
			
			// Index for remapping
			BoneIndices[ibsrch] = ib;
		}
		
		// Reorder the bones
		for (size_t ib=0; ib<NumBones; ib++)
			SortedBones[BoneIndices[ib]] = Model.Bones[ib];
		
		// Put them back into the model in one step
		Model.Bones.swap(SortedBones);
		
		// Find the vertex bone indices; this assumes that the vertices have already been read in.
		for (unsigned iv=0; iv<Model.VtxSources.size(); iv++)
		{
			Model3D_VertexSource& VS = Model.VtxSources[iv];
			size_t ibsx;
			char *Tag;
			
			Tag = VertexBoneTags[iv].Tag0;
			for (ibsx=0; ibsx<NumBones; ibsx++)
			{
				if (strncmp(Tag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
				break;
			}
			VS.Bone0 = ibsx < NumBones ? BoneIndices[ibsx] : static_cast<GLshort>(NONE);
			
			Tag = VertexBoneTags[iv].Tag1;
			for (ibsx=0; ibsx<NumBones; ibsx++)
			{
				if (strncmp(Tag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
				break;
			}
			VS.Bone1 = ibsx < NumBones ? BoneIndices[ibsx] : static_cast<GLshort>(NONE);
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
	CenterParser("Center"),
	LightParser("Light"),
	ShadingParser("Shading"),
	ShadowBoxParser("Shadow_Box"),
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
	TrianglesParser("Triangles"),
	FramesParser("Poses"),
	FrameBonesParser("Bones"),
	SequencesParser("Animations"),
	SeqLoopParser("Loop"),
	SeqFramesParser("Poses");


// "Real" elements:

class XML_BoundingBoxParser: public XML_ElementParser
{
	GLfloat x_size, y_size, z_size, x_offset, y_offset, z_offset;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_BoundingBoxParser(const char *Name): XML_ElementParser(Name) {}
};



bool XML_BoundingBoxParser::Start()
{
	x_size = y_size = z_size = x_offset = y_offset = z_offset = 0;
	
	return true;
}

bool XML_BoundingBoxParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"size"))
	{
		return (sscanf(Value,"%f,%f,%f",&x_size,&y_size,&z_size) == 3);
	}
	else if (StringsEqual(Tag,"offset"))
	{
		return (sscanf(Value,"%f,%f,%f",&x_offset,&y_offset,&z_offset) == 3);
	}
	else if (StringsEqual(Tag,"x_size"))
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

static XML_BoundingBoxParser BoundingBoxParser("Bound_Box");
static XML_BoundingBoxParser ViewBoxParser("View_Box");


class XML_VertexParser: public XML_ElementParser
{
	Model3D_VertexSource Data;
	GLfloat Norm[3];
	
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
		Norm[c] = Data.Position[c] = 0;
	
	// Initially: no bones
	Data.Bone0 = Data.Bone1 = (GLushort)NONE;
	Data.Blend = 0;
	
	// No bone: zero-length strings:
	BT.Tag0[0] = BT.Tag1[0] = 0;
	
	return true;
}

bool XML_VertexParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"c3"))
	{
		GLfloat *Pos = Data.Position;
		return (sscanf(Value,"%f,%f,%f",&Pos[0],&Pos[1],&Pos[2]) == 3);
	}
	else if (StringsEqual(Tag,"n3"))
	{
		return (sscanf(Value,"%f,%f,%f",&Norm[0],&Norm[1],&Norm[2]) == 3);
	}
	else if (StringsEqual(Tag,"x"))
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
	// Also always handle normal data for that reason.
	ModelPtr->VtxSources.push_back(Data);
	for (int c=0; c<3; c++)
		Normals.push_back(Norm[c]);
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
	
	// Initially: don't do anything special
	// (might produce screwy models without further processing)
	Data.Flags = 0;
	
	// No bone: zero-length strings:
	BT.Tag0[0] = BT.Tag1[0] = 0;
	
	return true;
}

bool XML_BoneParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"c3"))
	{
		GLfloat *Pos = Data.Position;
		return (sscanf(Value,"%f,%f,%f",&Pos[0],&Pos[1],&Pos[2]) == 3);
	}
	else if (StringsEqual(Tag,"x"))
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
	float Norm_X, Norm_Y, Norm_Z;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_TriVertexParser(): XML_ElementParser("v") {}
};


bool XML_TriVertexParser::Start()
{
	// Reasonable defaults:
	ID = (uint16)NONE;
	Txtr_X = 0.5, Txtr_Y = 0.5;
	Norm_X = Norm_Y = Norm_Z = 0;
	
	return true;
}

bool XML_TriVertexParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"ID"))
	{
		return ReadUInt16Value(Value,ID);
	}
	else if (StringsEqual(Tag,"uv"))
	{
		return (sscanf(Value,"%f,%f",&Txtr_X,&Txtr_Y) == 2);
	}
	else if (StringsEqual(Tag,"n"))
	{
		return (sscanf(Value,"%f,%f,%f",&Norm_X,&Norm_Z,&Norm_Y) == 3);	// Dim3's odd order
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
	// Older dim3-Animator normal support suppressed
	/*
	// Normalize the normal, if nonzero
	float NSQ = Norm_X*Norm_X + Norm_Y*Norm_Y + Norm_Z*Norm_Z;
	if (NSQ != 0)
	{
		float NMult = (float)(1/sqrt(NSQ));
		Norm_X *= NMult;
		Norm_Y *= NMult;
		Norm_Z *= NMult;
	}
	*/

	GLushort Index = ((GLushort)ModelPtr->VertIndices.size());
	ModelPtr->VertIndices.push_back(Index);
	ModelPtr->VtxSrcIndices.push_back(ID);
	ModelPtr->TxtrCoords.push_back(Txtr_X);
	ModelPtr->TxtrCoords.push_back(Txtr_Y);
	/*
	ModelPtr->Normals.push_back(Norm_X);
	ModelPtr->Normals.push_back(Norm_Y);
	ModelPtr->Normals.push_back(Norm_Z);
	*/
	return true;
}

static XML_TriVertexParser TriVertexParser;



class XML_FrameParser: public XML_ElementParser
{
	// For adding to the frame-name array as frames are added
	NameTagWrapper NT;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool End();
	
	XML_FrameParser(): XML_ElementParser("Pose") {}
};


bool XML_FrameParser::Start()
{
	// Be sure to have the right number of frame members --
	// and blank them out
	size_t NumBones = ModelPtr->Bones.size();
	ReadFrame.resize(NumBones);
	objlist_clear(&ReadFrame[0],NumBones);
	
	// No name: zero-length name
	NT.Tag[0] = 0;
	
	return true;
}

bool XML_FrameParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"name"))
	{
		strncpy(NT.Tag,Value,NameTagSize);
		return true;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_FrameParser::End()
{
	// Some of the data was set up by child elements, so all the processing
	// can be back here.
	for (size_t b=0; b<ReadFrame.size(); b++)
		ModelPtr->Frames.push_back(ReadFrame[b]);
	
	FrameTags.push_back(NT);
	
	return true;
}

static XML_FrameParser FrameParser;



class XML_FrameBoneParser: public XML_ElementParser
{
	Model3D_Frame Data;
	
	// The bone tag to look for
	char BoneTag[BoneTagSize];

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_FrameBoneParser(): XML_ElementParser("Bone") {}
};


bool XML_FrameBoneParser::Start()
{
	// Clear everything out:
	obj_clear(Data);
	
	// Empty string
	BoneTag[0] = 0;
	
	return true;
}


// Some of angles have their signs reversed to translate BB's sign conventions
// into more my more geometrically-elegant ones.

bool XML_FrameBoneParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"rot"))
	{
		float InAngle[3];
		if (sscanf(Value,"%f,%f,%f",&InAngle[0],&InAngle[1],&InAngle[2]) == 3)
		{
			for (int c=0; c<3; c++)
				Data.Angles[c] = GetAngle(- InAngle[c]);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"move"))
	{
		GLfloat *Ofst = Data.Offset;
		return (sscanf(Value,"%f,%f,%f",&Ofst[0],&Ofst[1],&Ofst[2]) == 3);
	}
	else if (StringsEqual(Tag,"acceleration"))
	{
		// Ignore the acceleration for now
		return true;
	}
	else if (StringsEqual(Tag,"xmove"))
	{
		return ReadFloatValue(Value,Data.Offset[0]);
	}
	else if (StringsEqual(Tag,"ymove"))
	{
		return ReadFloatValue(Value,Data.Offset[1]);
	}
	else if (StringsEqual(Tag,"zmove"))
	{
		return ReadFloatValue(Value,Data.Offset[2]);
	}
	else if (StringsEqual(Tag,"xrot"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[0] = GetAngle(InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"yrot"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[1] = GetAngle(-InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"zrot"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[2] = GetAngle(-InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"tag"))
	{
		strncpy(BoneTag,Value,BoneTagSize);
		return true;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_FrameBoneParser::AttributesDone()
{
	// Place the bone info into the appropriate temporary-array location
	size_t NumBones = BoneOwnTags.size();
	size_t ib;
	for (ib=0; ib<NumBones; ib++)
	{
		// Compare tag to bone's self tag
		if (strncmp(BoneTag,BoneOwnTags[ib].Tag0,BoneTagSize) == 0)
			break;
	}
	if (ib < NumBones)
		obj_copy(ReadFrame[BoneIndices[ib]],Data);
	
	return true;
}

static XML_FrameBoneParser FrameBoneParser;


class XML_SequenceParser: public XML_ElementParser
{

public:
	bool End();
	
	XML_SequenceParser(): XML_ElementParser("Animation") {}
};

bool XML_SequenceParser::End()
{
	// Add pointer index to end of sequences list;
	// create that list if it had been absent.
	if (ModelPtr->SeqFrmPointers.empty())
	{
		ModelPtr->SeqFrmPointers.push_back(0);
	}
	ModelPtr->SeqFrmPointers.push_back((GLushort)(ModelPtr->SeqFrames.size()));
	return true;
}

static XML_SequenceParser SequenceParser;


class XML_SeqFrameParser: public XML_ElementParser
{
	Model3D_SeqFrame Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_SeqFrameParser(): XML_ElementParser("Pose") {}
};


bool XML_SeqFrameParser::Start()
{
	// Clear everything out:
	obj_clear(Data);
	
	// No frame
	Data.Frame = NONE;
	
	return true;
}

// Some of angles have their signs reversed to translate BB's sign conventions
// into more my more geometrically-elegant ones.

bool XML_SeqFrameParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"sway"))
	{
		float InAngle[3];
		if (sscanf(Value,"%f,%f,%f",&InAngle[0],&InAngle[1],&InAngle[2]) == 3)
		{
			for (int c=0; c<3; c++)
				Data.Angles[c] = GetAngle(- InAngle[c]);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"move"))
	{
		GLfloat *Ofst = Data.Offset;
		return (sscanf(Value,"%f,%f,%f",&Ofst[0],&Ofst[1],&Ofst[2]) == 3);
	}
	else if (StringsEqual(Tag,"xmove"))
	{
		return ReadFloatValue(Value,Data.Offset[0]);
	}
	else if (StringsEqual(Tag,"ymove"))
	{
		return ReadFloatValue(Value,Data.Offset[1]);
	}
	else if (StringsEqual(Tag,"zmove"))
	{
		return ReadFloatValue(Value,Data.Offset[2]);
	}
	else if (StringsEqual(Tag,"xsway"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[0] = GetAngle(InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"ysway"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[1] = GetAngle(-InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"zsway"))
	{
		float InAngle;
		if (ReadFloatValue(Value,InAngle))
		{
			Data.Angles[2] = GetAngle(-InAngle);
			return true;
		}
		else
			return false;
	}
	else if (StringsEqual(Tag,"name"))
	{
		// Find which frame
		size_t ifr;
		size_t NumFrames = FrameTags.size();
		for (ifr=0; ifr<NumFrames; ifr++)
		{
			if (strncmp(Value,FrameTags[ifr].Tag,BoneTagSize) == 0) break;
		}
		if (ifr >= NumFrames) ifr = static_cast<size_t>(NONE);
		Data.Frame = (GLshort)ifr;
		return true;
	}
	else if (StringsEqual(Tag,"time"))
	{
		// Ignore; all timing info will come from the shapes file
		return true;
	}
	
	UnrecognizedTag();
	return false;
}

bool XML_SeqFrameParser::AttributesDone()
{
	// Add the frame
	ModelPtr->SeqFrames.push_back(Data);
	
	return true;
}

static XML_SeqFrameParser SeqFrameParser;


void Dim3_SetupParseTree()
{
	// Lazy init
	if (Dim3_ParserInited) return;

	// Set up the root object
	Dim3_RootParser.AddChild(&Dim3_Parser);
	
	Dim3_Parser.AddChild(&CreatorParser);
	Dim3_Parser.AddChild(&CenterParser);
	Dim3_Parser.AddChild(&LightParser);
	Dim3_Parser.AddChild(&BoundingBoxParser);	
	Dim3_Parser.AddChild(&ViewBoxParser);
	Dim3_Parser.AddChild(&ShadingParser);
	Dim3_Parser.AddChild(&ShadowBoxParser);

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
	
	FrameBonesParser.AddChild(&FrameBoneParser);
	FrameParser.AddChild(&FrameBonesParser);
	FramesParser.AddChild(&FrameParser);
	Dim3_Parser.AddChild(&FramesParser);
	
	SeqFramesParser.AddChild(&SeqFrameParser);
	SequenceParser.AddChild(&SeqFramesParser);
	SequenceParser.AddChild(&SeqLoopParser);
	SequencesParser.AddChild(&SequenceParser);
	Dim3_Parser.AddChild(&SequencesParser);
	
	Dim3_ParserInited = true;
}


// HAVE_OPENGL
#endif
