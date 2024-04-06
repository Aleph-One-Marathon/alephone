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

	Alias|Wavefront Object Loader
	
	By Loren Petrich, June 16, 2001
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "cseries.h"

#include "Logging.h"

#ifdef HAVE_OPENGL

#include "WavefrontLoader.h"


// Which of these is present in the vertex-info data:
enum {
	Present_Position	= 0x0001,
	Present_TxtrCoord	= 0x0002,
	Present_Normal		= 0x0004
};

static const char *Path = NULL;	  // Path to model file.

// Input line will be able to stretch as much as necessary
static vector<char> InputLine(64);

// Compare input-line beginning to a keyword;
// returns pointer to rest of line if it was found,
// otherwise returns NULL
char *CompareToKeyword(const char *Keyword);

// Gets a pointer to a string of vertex-index sets and picks off one of them,
// returning a pointer to the character just after it. Also returns the presence and values
// picked off.
// Returns NULL if there are none remaining to be found.
char *GetVertIndxSet(char *Buffer, short& Presence,
	short& PosIndx, short& TCIndx, short& NormIndx);

// Gets a vertex index and returns whether or not an index value was found
// what it was if found, and a pointer to the character just after the index value
// (either '/' or '\0'). And also whether the scanning hit the end of the set.
// Returns NULL if there are none remaining to be found.
char *GetVertIndx(char *Buffer, bool& WasFound, short& Val, bool& HitEnd);


// The purpose of the sorting is to find all the unique index sets;
// this is some data for the STL sorter
struct IndexedVertListCompare
{
	short *VertIndxSets;
	
	// The comparison operation
	bool operator() (int i1, int i2) const
	{
		short *VISet1 = VertIndxSets + 4*i1;
		short *VISet2 = VertIndxSets + 4*i2;
		
		// Sort by position first, then texture coordinate, then normal
		
		if (VISet1[1] > VISet2[1])
			return false;
		else if (VISet1[1] < VISet2[1])
			return true;
		
		if (VISet1[2] > VISet2[2])
			return false;
		else if (VISet1[2] < VISet2[2])
			return true;
		
		if (VISet1[3] > VISet2[3])
			return false;
		else if (VISet1[3] < VISet2[3])
			return true;

		// All equal!
//		return true;
		return false;
	}
};

bool LoadModel_Wavefront(FileSpecifier& Spec, Model3D& Model)
{
	// Clear out the final model object
	Model.Clear();

	// Intermediate lists of positions, texture coordinates, and normals
	vector<GLfloat> Positions;
	vector<GLfloat> TxtrCoords;
	vector<GLfloat> Normals;
	
	// Intermediate list of polygon features:
	// Polygon sizes (how many vertices):
	vector<short> PolygonSizes;
	// Vertex indices (how many read, position, txtr-coord, normal)
	vector<short> VertIndxSets;
	
	Path = Spec.GetPath();
	logNote("Loading Alias|Wavefront model file %s",Path);
	
	OpenedFile OFile;
	if (!Spec.Open(OFile))
	{	
		logError("ERROR opening %s",Path);
		return false;
	}

	// Reading loop; create temporary lists of positions, texture coordinates, and normals

	// Load the lines, one by one, and then parse them. Be sure to take care of the continuation
	// character "\" [Wavefront files follow some Unix conventions]
	bool MoreLines = true;
	while(MoreLines)
	{
		InputLine.clear();
		
		// Fill up the line
		bool LineContinued = false;
		while(true)
		{
			// Try to read a character; if it is not possible to read anymore,
			// the line has ended
			char c;
			MoreLines = OFile.Read(1,&c);
			if (!MoreLines) break;
			
			// End-of-line characters; ignore if the line is to be continued
			if (c == '\r' || c == '\n')
			{
				if (!LineContinued)
				{
					// If the line is not empty, then break; otherwise ignore.
					// Blank lines will be ignored, and this will allow starting a line
					// at the first non-end-of-line character
					if (!InputLine.empty()) break;		
				}
			}
			// Backslash character indicates that the current line continues into the next one
			else if (c == '\\')
			{
				LineContinued = true;
			}
			else
			{
				// Continuation will stop if a non-end-of-line character is encounted
				LineContinued = false;
				
				// Add that character!
				InputLine.push_back(c);
			}
		}
		// Line-end at end of file will produce an empty line, so do this test
		if (InputLine.empty()) continue;
		
		// If the line is a comment line, then ignore it
		if (InputLine[0] == '#') continue;
		
		// Make the line look like a C string
		InputLine.push_back('\0');
		
		// Now parse the line; notice the = instead of == (substitute and test in one line)
		// Unhandled keywords are currently commented out for speed;
		// many of those are for handling curved surfaces, which are currently ignored.
		char *RestOfLine = NULL;
		if ((RestOfLine = CompareToKeyword("v")) != NULL) // Vertex position
		{
			GLfloat Position[3];
			objlist_clear(Position,3);
			
			sscanf(RestOfLine," %f %f %f",Position,Position+1,Position+2);
			
			for (int k=0; k<3; k++)
				Positions.push_back(Position[k]);
		}
		else if ((RestOfLine = CompareToKeyword("vt")) != NULL) // Vertex texture coordinate
		{
			GLfloat TxtrCoord[2];
			objlist_clear(TxtrCoord,2);
			
			sscanf(RestOfLine," %f %f",TxtrCoord,TxtrCoord+1);
			
			for (int k=0; k<2; k++)
				TxtrCoords.push_back(TxtrCoord[k]);
		}
		else if ((RestOfLine = CompareToKeyword("vn")) != NULL) // Vertex normal
		{
			GLfloat Normal[3];
			objlist_clear(Normal,3);
			
			sscanf(RestOfLine," %f %f %f",Normal,Normal+1,Normal+2);
			
			for (int k=0; k<3; k++)
				Normals.push_back(Normal[k]);
		}
		/*
		else if ((RestOfLine = CompareToKeyword("vp")) // Vertex parameter value
		{
			// For curved objects, which are not supported here
		}
		else if ((RestOfLine = CompareToKeyword("deg")) != NULL) // Degree
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("bmat")) != NULL) // Basis matrix
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("step")) != NULL) // Step size
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("cstype")) != NULL) // Curve/surface type
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("p")) != NULL) // Point
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("l")) != NULL) // Line
		{
			// Not supported here
		}
		*/
		else if ((RestOfLine = CompareToKeyword("f")) != NULL) // Face (polygon)
		{
			// Pick off the face vertices one by one;
			// stuff their contents into a token and then process that token
			int NumVertices = 0;
			
			short Presence = 0, PosIndx = 0, TCIndx = 0, NormIndx = 0;
			while((RestOfLine = GetVertIndxSet(RestOfLine, Presence, PosIndx, TCIndx, NormIndx)) != NULL)
			{			
				NumVertices++;
				
				// Wavefront vertex-index conventions:
				// Positive is 1-based indexing
				// Negative is from end of current list
				
				if (PosIndx < 0)
					PosIndx += static_cast<short>(Positions.size())/3;
				else
					PosIndx--;
				
				if (TCIndx < 0)
					TCIndx += static_cast<short>(TxtrCoords.size())/2;
				else
					TCIndx--;
				
				if (NormIndx < 0)
					NormIndx += static_cast<short>(Normals.size())/3;
				else
					NormIndx--;
				
				// Add!
				VertIndxSets.push_back(Presence);
				VertIndxSets.push_back(PosIndx);
				VertIndxSets.push_back(TCIndx);
				VertIndxSets.push_back(NormIndx);
			}
			// Polygon complete!
			PolygonSizes.push_back(NumVertices);
		}
		/*
		else if ((RestOfLine = CompareToKeyword("curv")) != NULL) // Curve
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("curv2")) != NULL) // 2D Curve
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("surf")) != NULL) // Surface
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("parm")) != NULL) // Parameter values
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("trim")) != NULL) // Outer trimming loop
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("hole")) != NULL) // Inner trimming loop
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("scrv")) != NULL) // Special curve
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("sp")) != NULL) // Special point
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("end")) != NULL) // End statement
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("con")) != NULL) // Connect
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("g")) != NULL) // Group name
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("s")) != NULL) // Smoothing group
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("mg")) != NULL) // Merging group
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("o")) != NULL) // Object name
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("bevel")) != NULL) // Bevel interpolation
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("c_interp")) != NULL) // Color interpolation
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("d_interp")) != NULL) // Dissolve interpolation
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("lod")) != NULL) // Level of detail
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("usemtl")) != NULL) // Material name
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("mtllib")) != NULL) // Material library
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("shadow_obj")) != NULL) // Shadow casting
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("trace_obje")) != NULL) // Ray tracing
		{
			// Not supported here
		}
		else if ((RestOfLine = CompareToKeyword("ctech")) != NULL) // Curve approximation technique
		{
			// Curved objects not supported here
		}
		else if ((RestOfLine = CompareToKeyword("stech")) != NULL) // Surface approximation technique
		{
			// Curved objects not supported here
		}
		*/
	}
	
	if (PolygonSizes.size() <= 0)
	{
		logError("ERROR: the model in %s has no polygons",Path);
		return false;
	}
		
	// How many vertices do the polygons have?
	for (unsigned k=0; k<PolygonSizes.size(); k++)
	{
		short PSize = PolygonSizes[k];
		if (PSize < 3)
		{
			logWarning("WARNING: polygon ignored; it had bad size %u: %d in %s",k,PSize,Path);
		}
	}
	
	// What is the lowest common denominator of the polygon data
	// (which is present of vertex positions, texture coordinates, and normals)
	short WhatsPresent = Present_Position | Present_TxtrCoord | Present_Normal;
	
	for (unsigned k=0; k<VertIndxSets.size()/4; k++)
	{
		short Presence = VertIndxSets[4*k];
		WhatsPresent &= Presence;
		if (!(Presence & Present_Position))
		{
			logError("ERROR: Vertex has no position index: %u in %s",k,Path);
		}
	}
	
	if (!(WhatsPresent & Present_Position)) return false;
	
	bool AllInRange = true;
	
	for (unsigned k=0; k<VertIndxSets.size()/4; k++)
	{
		short PosIndx = VertIndxSets[4*k+1];
		if (PosIndx < 0 || PosIndx >= int(Positions.size()))
		{
			logError("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,PosIndx,(unsigned long)Positions.size()-1,Path);
			AllInRange = false;
		}
		
		if (WhatsPresent & Present_TxtrCoord)
		{
			short TCIndx = VertIndxSets[4*k+2];
			if (TCIndx < 0 || TCIndx >= int(TxtrCoords.size()))
			{
				logError("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,TCIndx,(unsigned long)(TxtrCoords.size()-1),Path);
				AllInRange = false;
			}
		}
		else
			VertIndxSets[4*k+2] = -1; // What "0" gets turned into by the Wavefront-conversion-translation code
		
		if (WhatsPresent & Present_Normal)
		{
			short NormIndx = VertIndxSets[4*k+3];
			if (NormIndx < 0 || NormIndx >= int(Normals.size()))
			{
				logError("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,NormIndx,(unsigned long)(Normals.size()-1),Path);
				AllInRange = false;
			}
		}
		else
			VertIndxSets[4*k+3] = -1; // What "0" gets turned into by the Wavefront-conversion-translation code
	}
	
	if (!AllInRange) return false;
	
	// Find unique vertex sets:
	
	// First, do an index sort of them
	vector<int> VertIndxRefs(VertIndxSets.size()/4);
	for (unsigned k=0; k<VertIndxRefs.size(); k++)
		VertIndxRefs[k] = k;
	
	IndexedVertListCompare Compare;
	Compare.VertIndxSets = &VertIndxSets[0];
	sort(VertIndxRefs.begin(),VertIndxRefs.end(),Compare);
	
	// Find the unique entries:
	vector<int> WhichUniqueSet(VertIndxRefs.size());
	
	// Previous index values:
	short PrevPosIndx = -1, PrevTCIndx = -1, PrevNormIndx = -1;
	// For doing zero-based indexing
	int NumUnique = -1;
	
	// Scan the vertices in index-sort order:
	for (unsigned k=0; k<VertIndxRefs.size(); k++)
	{
		int n = VertIndxRefs[k];
		
		short *VISet = &VertIndxSets[4*n];
		short PosIndx = VISet[1];
		short TCIndx = VISet[2];
		short NormIndx = VISet[3];
		
		if (PosIndx == PrevPosIndx && TCIndx == PrevTCIndx && NormIndx == PrevNormIndx)
		{
			WhichUniqueSet[n] = NumUnique;
			continue;
		}
		
		// Found a unique set
		WhichUniqueSet[n] = ++NumUnique;
		
		// These are all for the model object
		
		// Load the positions
		{
			GLfloat *PosPtr = &Positions[3*PosIndx];
			for (int m=0; m<3; m++)
				Model.Positions.push_back(*(PosPtr++));
		}
		
		// Load the texture coordinates
		if (WhatsPresent & Present_TxtrCoord)
		{
			GLfloat *TCPtr = &TxtrCoords[2*TCIndx];
			for (int m=0; m<2; m++)
				Model.TxtrCoords.push_back(*(TCPtr++));
		}
		
		// Load the normals
		if (WhatsPresent & Present_Normal)
		{
			GLfloat *NormPtr = &Normals[3*NormIndx];
			for (int m=0; m<3; m++)
				Model.Normals.push_back(*(NormPtr++));
		}
		
		// Save these new unique-set values for comparison to the next ones
		PrevPosIndx = PosIndx;
		PrevTCIndx = TCIndx;
		PrevNormIndx = NormIndx;
	}
	
	// Decompose the polygons into triangles by turning them into fans
	int IndxBase = 0;
	for (unsigned k=0; k<PolygonSizes.size(); k++)
	{
		short PolySize = PolygonSizes[k];
		int *PolyIndices = &WhichUniqueSet[IndxBase];
		
		for (int m=0; m<PolySize-2; m++)
		{
			Model.VertIndices.push_back(PolyIndices[0]);
			Model.VertIndices.push_back(PolyIndices[m+1]);
			Model.VertIndices.push_back(PolyIndices[m+2]);
		}
		
		IndxBase += PolySize;
	}
	
	if (Model.VertIndices.size() <= 0)
	{
		logError("ERROR: the model in %s has no good polygons",Path);
		return false;
	}
	
	logTrace("Successfully read the file:");
	if (WhatsPresent & Present_Position)  logTrace("    Positions");
	if (WhatsPresent & Present_TxtrCoord) logTrace("    TxtrCoords");
	if (WhatsPresent & Present_Normal)    logTrace("    Normals");
	return true;
}


char *CompareToKeyword(const char *Keyword)
{
	size_t KWLen = strlen(Keyword);
	
	if (InputLine.size() < KWLen) return NULL;
	
	for (unsigned k=0; k<KWLen; k++)
		if (InputLine[k] != Keyword[k]) return NULL;
	
	char *RestOfLine = &InputLine[KWLen];
	
	while(RestOfLine - &InputLine[0] < int(InputLine.size()))
	{
		// End of line?
		if (*RestOfLine == '\0') return RestOfLine;
		
		// Other than whitespace -- assume it to be part of the keyword if just after it;
		// otherwise, it is to be returned to the rest of the code to work on
		if (!(*RestOfLine == ' ' || *RestOfLine == '\t'))
			return ((RestOfLine == &InputLine[KWLen]) ? NULL : RestOfLine);
		
		// Whitespace: move on to the next character
		RestOfLine++;
	}
	
	// Shouldn't happen
	return NULL;
}


char *GetVertIndxSet(char *Buffer, short& Presence,
	short& PosIndx, short& TCIndx, short& NormIndx)
{
	// Initialize...
	Presence = 0; PosIndx = 0; TCIndx = 0; NormIndx = 0;
	
	// Eat initial whitespace; return NULL if end-of-string was hit
	// OK to modify Buffer, since it's called by value
	while(*Buffer == ' ' || *Buffer == '\t')
	{
		Buffer++;
	}
	if (*Buffer == '\0') return NULL;
	
	// Hit non-whitespace; now grab the individual vertex values
	bool WasFound = false, HitEnd = false;
	Buffer = GetVertIndx(Buffer,WasFound,PosIndx,HitEnd);
	if (WasFound) Presence |= Present_Position;
	if (HitEnd) return Buffer;
	
	Buffer = GetVertIndx(Buffer,WasFound,TCIndx,HitEnd);
	if (WasFound) Presence |= Present_TxtrCoord;
	if (HitEnd) return Buffer;
	
	Buffer = GetVertIndx(Buffer,WasFound,NormIndx,HitEnd);
	if (WasFound) Presence |= Present_Normal;
	return Buffer;
}

char *GetVertIndx(char *Buffer, bool& WasFound, short& Val, bool& HitEnd)
{
	const int VIBLen = 64;
	char VIBuffer[VIBLen];
	int VIBIndx = 0;
	
	// Load the vertex-index buffer and make it a C string
	HitEnd = false;
	WasFound = false;
	bool HitInternalBdry = false;	// Use this variable to avoid duplicating an evaluation
	while (!(HitInternalBdry = (*Buffer == '/')))
	{
		HitEnd = (*Buffer == ' ' || *Buffer == '\t' || *Buffer == '\0');
		if (HitEnd) break;
		
		if (VIBIndx < VIBLen-1)
			VIBuffer[VIBIndx++] = *Buffer;
		
		Buffer++;
	}
	if (HitInternalBdry) Buffer++;
	VIBuffer[VIBIndx] = '\0';
	
	// Interpret it!
	WasFound = (sscanf(VIBuffer,"%hd",&Val) > 0);
	
	return Buffer;
}

// Load a Wavefront model and convert its vertex and texture coordinates from
// OBJ's right-handed coordinate system to Aleph One's left-handed system.
bool LoadModel_Wavefront_RightHand(FileSpecifier& Spec, Model3D& Model)
{
	bool Result = LoadModel_Wavefront(Spec, Model);
	if (!Result) return Result;

	logTrace("Converting handedness.");

	// OBJ files produced by Blender and Wings 3D are oriented with
	// y increasing upwards, and the front of a Blender model faces in the
	// positive-Z direction.  (Wings 3D does not distinguish a "front"
	// view.)  In Aleph One's coordinate system Z increases upwards, and
	// items that have been placed with 0 degrees of rotation face in the
	// positive-x direction.
	for (unsigned XPos = 0; XPos < Model.Positions.size(); XPos += 3)
	{
		GLfloat X = Model.Positions[XPos];
		Model.Positions[XPos] = Model.Positions[XPos + 2];
		Model.Positions[XPos + 2] = Model.Positions[XPos + 1];
		Model.Positions[XPos + 1] = -X;
	}

	// Ditto for vertex normals, if present.
	for (unsigned XPos = 0; XPos < Model.Normals.size(); XPos += 3)
	{
		GLfloat X = Model.Normals[XPos];
		Model.Normals[XPos] = Model.Normals[XPos + 2];
		Model.Normals[XPos + 2] = Model.Normals[XPos + 1];
		Model.Normals[XPos + 1] = -X;
	}

	// Vertices of each face are now listed in clockwise order.
	// Reverse them.
	for (unsigned IPos = 0; IPos < Model.VertIndices.size(); IPos += 3)
	{
		int Index = Model.VertIndices[IPos + 1];
		Model.VertIndices[IPos + 1] = Model.VertIndices[IPos];
		Model.VertIndices[IPos] = Index;
	}

	// Switch texture coordinates from right-handed (x,y) to
	// left-handed (row,column).
	for (unsigned YPos = 1; YPos < Model.TxtrCoords.size(); YPos += 2)
	{
		Model.TxtrCoords[YPos] = 1.0 - Model.TxtrCoords[YPos];
	}

	return true;
}

#endif // def HAVE_OPENGL
