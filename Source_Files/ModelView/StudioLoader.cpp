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

	3D Studio Max Object Loader
	
	By Loren Petrich, Sept 1, 2001
	
	Derived from the work of
	
	Jeff Lewis (werewolf@worldgate.com)
	Martin van Velsen (vvelsen@ronix.ptf.hro.nl)
	Robin Fercoq (robin@msrwww.fc-net.fr)
	Jim Pitts (jim@micronetics.com)
	Albert Szilvasy (szilvasy@almos.vein.hu)
	Terry Caton (tcaton@umr.edu)
	Matthew Fairfax
	
*/

#include "cseries.h"

#ifdef HAVE_OPENGL
#ifdef __WIN32__
#include <windows.h>
#endif

#include "StudioLoader.h"

// Use pack/unpack, but with little-endian data
#undef PACKED_DATA_IS_BIG_ENDIAN
#define PACKED_DATA_IS_LITTLE_ENDIAN
#include "Packing.h"

/*
	The various authors have different conventions for the chunk names;
	the nomenclature here is a hybrid of these.
	Only the chunks actually read will be listed here.
*/

const uint16 MASTER =				0x4d4d;
const uint16   EDITOR =				0x3d3d;
const uint16     OBJECT =			0x4000;
const uint16       TRIMESH =		0x4100;
const uint16         VERTICES =		0x4110;
const uint16         TXTR_COORDS =	0x4140;
const uint16         FACE_DATA =	0x4120;


// Debug-message destination
static FILE *DBOut = NULL;

void SetDebugOutput_Studio(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}

struct ChunkHeaderData
{
	uint16 ID;
	uint32 Size;
};
const int SIZEOF_ChunkHeaderData = 6;

// For read-in chunks
vector<uint8> ChunkBuffer;
inline uint8 *ChunkBufferBase() {return &ChunkBuffer[0];}
inline size_t ChunkBufferSize() {return ChunkBuffer.size();}
inline void SetChunkBufferSize(int Size) {ChunkBuffer.resize(Size);}

// Local prototypes;
// these functions return "false" if there was a read or reposition error
static bool ReadChunkHeader(OpenedFile& OFile, ChunkHeaderData& ChunkHeader);
static bool LoadChunk(OpenedFile& OFile, ChunkHeaderData& ChunkHeader);
static bool SkipChunk(OpenedFile& OFile, ChunkHeaderData& ChunkHeader);

// These functions read the contents of these container chunks
// to the file location "ChunkEnd".
// The generic reader takes specific readers as the last argument
static bool ReadContainer(OpenedFile& OFile, ChunkHeaderData& ChunkHeader,
	bool (*ContainerCallback)(OpenedFile&,long));
static bool ReadMaster(OpenedFile& OFile, long ParentChunkEnd);
static bool ReadEditor(OpenedFile& OFile, long ParentChunkEnd);
static bool ReadObject(OpenedFile& OFile, long ParentChunkEnd);
static bool ReadTrimesh(OpenedFile& OFile, long ParentChunkEnd);
static bool ReadFaceData(OpenedFile& OFile, long ParentChunkEnd);

// For processing the raw chunk data into appropriate forms:
static void LoadVertices();
static void LoadTextureCoordinates();

static void LoadFloats(int NVals, uint8 *Stream, GLfloat *Floats);


// For feeding into the read-in routines
static Model3D *ModelPtr = NULL;

bool LoadModel_Studio(FileSpecifier& Spec, Model3D& Model)
{
	ModelPtr = &Model;
	Model.Clear();
	
	if (DBOut)
	{
		// Name buffer
		const int BufferSize = 256;
		char Buffer[BufferSize];
		Spec.GetName(Buffer);
		fprintf(DBOut,"Loading 3D Studio Max model file %s\n",Buffer);
	}
	
	OpenedFile OFile;
	if (!Spec.Open(OFile))
	{	
		if (DBOut) fprintf(DBOut,"ERROR opening the file\n");
		return false;
	}
	
	ChunkHeaderData ChunkHeader;
	if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
	if (ChunkHeader.ID != MASTER)
	{
		if (DBOut) fprintf(DBOut,"ERROR: not a 3DS Max model file\n");
		return false;
	}
	
	if (!ReadContainer(OFile,ChunkHeader,ReadMaster)) return false;
	
	return (!Model.Positions.empty() && !Model.VertIndices.empty());
}

bool ReadChunkHeader(OpenedFile& OFile, ChunkHeaderData& ChunkHeader)
{
	uint8 Buffer[SIZEOF_ChunkHeaderData];
	if (!OFile.Read(SIZEOF_ChunkHeaderData,Buffer))
	{
		if (DBOut) fprintf(DBOut,"ERROR reading chunk header\n");
		return false;
	}
	uint8 *S = Buffer;
	StreamToValue(S,ChunkHeader.ID);
	StreamToValue(S,ChunkHeader.Size);
	return true;
}

bool LoadChunk(OpenedFile& OFile, ChunkHeaderData& ChunkHeader)
{
	if (DBOut)
		fprintf(DBOut,"Loading chunk 0x%04hx size %lu\n",ChunkHeader.ID,ChunkHeader.Size);
	long DataSize = ChunkHeader.Size - SIZEOF_ChunkHeaderData;
	SetChunkBufferSize(DataSize);
	if (!OFile.Read(DataSize,ChunkBufferBase()))
	{
		if (DBOut) fprintf(DBOut,"ERROR reading chunk contents\n");
		return false;
	}
	
	return true;
}

bool SkipChunk(OpenedFile& OFile, ChunkHeaderData& ChunkHeader)
{
	if (DBOut)
		fprintf(DBOut,"Skipping chunk 0x%04hx size %lu\n",ChunkHeader.ID,ChunkHeader.Size);
	long DataSize = ChunkHeader.Size - SIZEOF_ChunkHeaderData;
	
	long Location = 0;
	OFile.GetPosition(Location);
	if (!OFile.SetPosition(Location + DataSize)) return false;
	return true;
}

// Generic container-chunk reader
bool ReadContainer(OpenedFile& OFile, ChunkHeaderData& ChunkHeader,
	bool (*ContainerCallback)(OpenedFile&,long))
{
	if (DBOut)
		fprintf(DBOut,"Entering chunk 0x%04hx size %lu\n",ChunkHeader.ID,ChunkHeader.Size);
	
	long ChunkEnd = 0;
	OFile.GetPosition(ChunkEnd);
	ChunkEnd += ChunkHeader.Size - SIZEOF_ChunkHeaderData;
	
	if (!ContainerCallback(OFile,ChunkEnd)) return false;
	
	if (DBOut)
		fprintf(DBOut,"Exiting chunk 0x%04hx size %lu\n",ChunkHeader.ID,ChunkHeader.Size);
	return true;
}


// For reading the master chunk (ideally, whole file)
static bool ReadMaster(OpenedFile& OFile, long ParentChunkEnd)
{
	long Location = 0;
	OFile.GetPosition(Location);
	
	while(Location < ParentChunkEnd)
	{
		ChunkHeaderData ChunkHeader;
		if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
		
		switch(ChunkHeader.ID)
		{
		case EDITOR:
			if (!ReadContainer(OFile,ChunkHeader,ReadEditor)) return false;
			break;
				
		default:
			if (!SkipChunk(OFile,ChunkHeader)) return false;
		}
		
		// Where are we now?
		OFile.GetPosition(Location);
	}
	
	if (Location > ParentChunkEnd)
	{
		if (DBOut)
			fprintf(DBOut,"ERROR: Overran parent chunk: %ld > %ld\n",Location,ParentChunkEnd);
		return false;
	}
	return true;
}

// For reading the editor-data chunk
static bool ReadEditor(OpenedFile& OFile, long ParentChunkEnd)
{
	long Location = 0;
	OFile.GetPosition(Location);
	
	while(Location < ParentChunkEnd)
	{
		ChunkHeaderData ChunkHeader;
		if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
		
		switch(ChunkHeader.ID)
		{
		case OBJECT:
			if (!ReadContainer(OFile,ChunkHeader,ReadObject)) return false;
			break;
			
		default:
			if (!SkipChunk(OFile,ChunkHeader)) return false;
		}
		
		// Where are we now?
		OFile.GetPosition(Location);
	}
	
	if (Location > ParentChunkEnd)
	{
		if (DBOut)
			fprintf(DBOut,"ERROR: Overran parent chunk: %ld > %ld\n",Location,ParentChunkEnd);
		return false;
	}
	return true;
}

// For reading the object-data chunk
static bool ReadObject(OpenedFile& OFile, long ParentChunkEnd)
{
	// Read the name
	if (DBOut) fprintf(DBOut,"Object Name: ");
	while(true)
	{
		char c;
		if (!OFile.Read(1,&c))
		{
			if (DBOut) fprintf(DBOut,"ERROR in reading name");
			return false;
		}
		
		if (c == 0)
		{
			if (DBOut) fprintf(DBOut,"\n");
			break;
		}
		else
		{
			if (DBOut) fprintf(DBOut,"%c",c);
		}
	}
	
	long Location = 0;
	OFile.GetPosition(Location);
	
	while(Location < ParentChunkEnd)
	{
		ChunkHeaderData ChunkHeader;
		if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
		
		switch(ChunkHeader.ID)
		{
		case TRIMESH:
			if (!ReadContainer(OFile,ChunkHeader,ReadTrimesh)) return false;
			break;
			
		default:
			if (!SkipChunk(OFile,ChunkHeader)) return false;
		}
		
		// Where are we now?
		OFile.GetPosition(Location);
	}
	
	if (Location > ParentChunkEnd)
	{
		if (DBOut)
			fprintf(DBOut,"ERROR: Overran parent chunk: %ld > %ld\n",Location,ParentChunkEnd);
		return false;
	}
	return true;
}

// For reading the triangle-mesh-data chunk
static bool ReadTrimesh(OpenedFile& OFile, long ParentChunkEnd)
{
	long Location = 0;
	OFile.GetPosition(Location);
	
	assert(ModelPtr);
	
	while(Location < ParentChunkEnd)
	{
		ChunkHeaderData ChunkHeader;
		if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
		
		switch(ChunkHeader.ID)
		{
		case VERTICES:
			if (!LoadChunk(OFile,ChunkHeader)) return false;
			LoadVertices();
			break;
			
		case TXTR_COORDS:
			if (!LoadChunk(OFile,ChunkHeader)) return false;
			LoadTextureCoordinates();
			break;
			
		case FACE_DATA:
			if (!ReadContainer(OFile,ChunkHeader,ReadFaceData)) return false;
			break;
			
		default:
			if (!SkipChunk(OFile,ChunkHeader)) return false;
		}
		
		// Where are we now?
		OFile.GetPosition(Location);
	}
	
	if (Location > ParentChunkEnd)
	{
		if (DBOut)
			fprintf(DBOut,"ERROR: Overran parent chunk: %ld > %ld\n",Location,ParentChunkEnd);
		return false;
	}
	return true;
}


// For reading the face-data chunk
static bool ReadFaceData(OpenedFile& OFile, long ParentChunkEnd)
{
	uint8 NFBuffer[2];
	uint16 NumFaces;
	if (!OFile.Read(2,NFBuffer))
	{
		if (DBOut) fprintf(DBOut,"ERROR reading number of faces\n");
		return false;
	}
	uint8 *S = NFBuffer;
	StreamToValue(S,NumFaces);
	
	long DataSize = 4*sizeof(uint16)*int(NumFaces);
	SetChunkBufferSize(DataSize);
	if (!OFile.Read(DataSize,ChunkBufferBase()))
	{
		if (DBOut) fprintf(DBOut,"ERROR reading face-chunk contents\n");
		return false;
	}
	
	S = ChunkBufferBase();
	ModelPtr->VertIndices.resize(3*NumFaces);
	for (int k=0; k<NumFaces; k++)
	{
		uint16 *CurrPoly = ModelPtr->VIBase() + 3*k;
		uint16 Flags;
		StreamToList(S,CurrPoly,3);
		StreamToValue(S,Flags);
	}
	
	long Location = 0;
	OFile.GetPosition(Location);
	
	while(Location < ParentChunkEnd)
	{
		ChunkHeaderData ChunkHeader;
		if (!ReadChunkHeader(OFile,ChunkHeader)) return false;
		
		switch(ChunkHeader.ID)
		{
		/*
		case OBJECT:
			if (!ReadContainer(OFile,ChunkHeader,ReadObject)) return false;
			break;
		*/
		default:
			if (!SkipChunk(OFile,ChunkHeader)) return false;
		}
		
		// Where are we now?
		OFile.GetPosition(Location);
	}
	
	if (Location > ParentChunkEnd)
	{
		if (DBOut)
			fprintf(DBOut,"ERROR: Overran parent chunk: %ld > %ld\n",Location,ParentChunkEnd);
		return false;
	}
	return true;
}


static void LoadVertices()
{
	uint8 *S = ChunkBufferBase();
	uint16 Size;
	StreamToValue(S,Size);
	
	int NVals = 3*int(Size);
	
	ModelPtr->Positions.resize(NVals);
	
	LoadFloats(NVals,S,ModelPtr->PosBase());
}

static void LoadTextureCoordinates()
{
	uint8 *S = ChunkBufferBase();
	uint16 Size;
	StreamToValue(S,Size);
	
	int NVals = 2*int(Size);
	
	ModelPtr->TxtrCoords.resize(NVals);
	
	LoadFloats(NVals,S,ModelPtr->TCBase());
}


void LoadFloats(int NVals, uint8 *Stream, GLfloat *Floats)
{
	// Test to see whether the destination floating-point values are the right size:
	assert(sizeof(GLfloat) == 4);
	
	uint32 IntVal;
	GLfloat *FloatPtr = Floats;
	for (int k=0; k<NVals; k++, FloatPtr++)
	{
		// Intermediate step: 4-byte integer
		// (won't have the right value if interpreted as an integer!)
		StreamToValue(Stream,IntVal);
		
		// This will work on any platform where
		// GLfloat is an IEEE 754 4-byte float.
		// Otherwise, use whatever appropriate conversion tricks are appropriate
		uint8 *SrcPtr = (uint8 *)(&IntVal);
		uint8 *DestPtr = (uint8 *)FloatPtr;
		for (int c=0; c<4; c++)
			DestPtr[c] = SrcPtr[c];
	}
}

#endif // def HAVE_OPENGL
