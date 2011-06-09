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
#ifdef __WIN32__
#include <windows.h>
#endif

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

//char *GetVertIndxSet(char *Buffer, short& Presence,
//	short& PosIndx, short& TCIndx, short& NormIndx);

// Gets a vertex index and returns whether or not an index value was found
// what it was if found, and a pointer to the character just after the index value
// (either '/' or '\0'). And also whether the scanning hit the end of the set.
// Returns NULL if there are none remaining to be found.
char *GetVertIndx(char *Buffer, bool& WasFound, short& Val, bool& HitEnd);

struct VertexIndexSet {

	short Presence;
	short PosIndx;
	short TCIndx;
	short NormIndx;

	VertexIndexSet() : Presence(0), PosIndx(0), TCIndx(0), NormIndx(0) {}

	// Gets a pointer to a string of vertex-index sets and picks off one of them,
	// returning a pointer to the character just after it. Also returns the presence and values
	// picked off.
	// Returns NULL if there are none remaining to be found.
	char* get(char *Buffer) {

		// Eat initial whitespace; return NULL if end-of-string was hit
		// OK to modify Buffer, since it's called by value
		while(*Buffer == ' ' || *Buffer == '\t') { ++Buffer; }

		if(*Buffer == '\0') { return NULL; }
		
		// Hit non-whitespace; now grab the individual vertex values
		bool WasFound = false, HitEnd = false;
		Buffer = GetVertIndx(Buffer, WasFound, PosIndx, HitEnd);
		if(WasFound) Presence |= Present_Position;
		if(HitEnd) { return Buffer; }
		
		Buffer = GetVertIndx(Buffer, WasFound, TCIndx, HitEnd);
		if(WasFound) { Presence |= Present_TxtrCoord; }
		if(HitEnd) { return Buffer; }
		
		Buffer = GetVertIndx(Buffer, WasFound, NormIndx, HitEnd);
		if(WasFound) {Presence |= Present_Normal; }
		return Buffer;
	}
};

// The purpose of the sorting is to find all the unique index sets;
// this is some data for the STL sorter
struct IndexedVertListCompare
{
	const std::vector<VertexIndexSet>& VertIndxSets;

	IndexedVertListCompare(const std::vector<VertexIndexSet>& sets) : VertIndxSets(sets) {}
	
	// The comparison operation
	bool operator() (int i1, int i2) const
	{
		const VertexIndexSet& VISet1 = VertIndxSets[i1];
		const VertexIndexSet& VISet2 = VertIndxSets[i2];
		
		// Sort by position first, then texture coordinate, then normal
		if(VISet1.PosIndx != VISet2.PosIndx) {
			return VISet1.PosIndx < VISet2.PosIndx;
		}

		if(VISet1.TCIndx != VISet2.TCIndx) {
			return VISet1.TCIndx < VISet2.TCIndx;
		}

		if(VISet1.NormIndx != VISet2.NormIndx) {
			return VISet1.NormIndx < VISet2.NormIndx;
		}

		// All equal!
		return false;
	}
};

vec4 CalculateTangent(const vec3& N, const vertex3& v1, const vertex3& v2, const vertex3& v3,
	const vertex2& w1, const vertex2& w2, const vertex2& w3);

void addVertex(Model3D& Model, const vertex3& v) {

	Model.Positions.push_back(v[0]);
	Model.Positions.push_back(v[1]);
	Model.Positions.push_back(v[2]);
}

void addTexCoord(Model3D& Model, const vertex2& t) {

	Model.TxtrCoords.push_back(t[0]);
	Model.TxtrCoords.push_back(t[1]);
}

void addNormal(Model3D& Model, const vec3& n) {

	Model.Normals.push_back(n[0]);
	Model.Normals.push_back(n[1]);
	Model.Normals.push_back(n[2]);
}

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
	vector<VertexIndexSet> VertIndxSets;
	
	Path = Spec.GetPath();
	logNote1("Loading Alias|Wavefront model file %s",Path);
	
	OpenedFile OFile;
	if (!Spec.Open(OFile))
	{	
		logError1("ERROR opening %s",Path);
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
		if (InputLine.empty()) { continue; }
		
		// If the line is a comment line, then ignore it
		if (InputLine[0] == '#') { continue; }
		
		// Make the line look like a C string
		InputLine.push_back('\0');
		
		// Now parse the line; notice the = instead of == (substitute and test in one line)
		// Unhandled keywords are currently commented out for speed;
		// many of those are for handling curved surfaces, which are currently ignored.
		char *RestOfLine = NULL;
		if ((RestOfLine = CompareToKeyword("v")) != NULL) // Vertex position
		{
			vertex3 Position(0.0f, 0.0f, 0.0f);
			
			sscanf(RestOfLine, " %f %f %f", &Position[0], &Position[1], &Position[2]);
			
			for (int k=0; k<3; k++) {
				Positions.push_back(Position[k]);
			}
		}
		else if ((RestOfLine = CompareToKeyword("vt")) != NULL) // Vertex texture coordinate
		{
			vertex2 TxtrCoord(0.0f, 0.0f);
			
			sscanf(RestOfLine, " %f %f", &TxtrCoord[0], &TxtrCoord[1]);
			
			for (int k=0; k<2; k++) {
				TxtrCoords.push_back(TxtrCoord[k]);
			}
		}
		else if ((RestOfLine = CompareToKeyword("vn")) != NULL) // Vertex normal
		{
			vec3 Normal(0.0f, 0.0f, 0.0f);
			
			sscanf(RestOfLine," %f %f %f", &Normal[0], &Normal[1], &Normal[2]);
			
			for (int k=0; k<3; k++) {
				Normals.push_back(Normal[k]);
			}
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
			
			VertexIndexSet vis;

			while((RestOfLine = vis.get(RestOfLine)) != NULL)
			{			
				NumVertices++;
				
				// Wavefront vertex-index conventions:
				// Positive is 1-based indexing
				// Negative is from end of current list
				
				if (vis.PosIndx < 0) {
					vis.PosIndx += static_cast<short>(Positions.size())/3;
				} else {
					vis.PosIndx--;
				}

				if (vis.TCIndx < 0) {
					vis.TCIndx += static_cast<short>(TxtrCoords.size())/2;
				} else {
					vis.TCIndx--;
				}

				if (vis.NormIndx < 0) {
					vis.NormIndx += static_cast<short>(Normals.size())/3;
				} else {
					vis.NormIndx--;
				}

				// Add!
				VertIndxSets.push_back(vis);
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
		logError1("ERROR: the model in %s has no polygons",Path);
		return false;
	}
		
	// How many vertices do the polygons have?
	for (unsigned k=0; k<PolygonSizes.size(); k++)
	{
		short PSize = PolygonSizes[k];
		if (PSize < 3)
		{
			logWarning3("WARNING: polygon ignored; it had bad size %u: %d in %s",k,PSize,Path);
		}
	}
	
	// What is the lowest common denominator of the polygon data
	// (which is present of vertex positions, texture coordinates, and normals)
	short WhatsPresent = Present_Position | Present_TxtrCoord | Present_Normal;
	
	for (unsigned k=0; k<VertIndxSets.size(); k++)
	{
		const short& Presence = VertIndxSets[k].Presence;
		WhatsPresent &= Presence;
		if (!(Presence & Present_Position))
		{
			logError2("ERROR: Vertex has no position index: %u in %s",k,Path);
		}
	}
	
	if (!(WhatsPresent & Present_Position)) return false;
	
	bool AllInRange = true;
	
	for (unsigned k=0; k<VertIndxSets.size(); k++)
	{
		const short& PosIndx = VertIndxSets[k].PosIndx;
		if (PosIndx < 0 || PosIndx >= int(Positions.size()))
		{
			logError4("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,PosIndx,(unsigned long)Positions.size()-1,Path);
			AllInRange = false;
		}
		
		if (WhatsPresent & Present_TxtrCoord)
		{
			const short& TCIndx = VertIndxSets[k].TCIndx;
			if (TCIndx < 0 || TCIndx >= int(TxtrCoords.size()))
			{
				logError4("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,TCIndx,(unsigned long)(TxtrCoords.size()-1),Path);
				AllInRange = false;
			}
		}
		else {
			VertIndxSets[k].TCIndx = -1; // What "0" gets turned into by the Wavefront-conversion-translation code
		}

		if (WhatsPresent & Present_Normal)
		{
			const short& NormIndx = VertIndxSets[k].NormIndx;
			if (NormIndx < 0 || NormIndx >= int(Normals.size()))
			{
				logError4("ERROR: Out of range vertex position: %u: %d (0,%lu) in %s",k,NormIndx,(unsigned long)(Normals.size()-1),Path);
				AllInRange = false;
			}
		}
		else {
			VertIndxSets[k].NormIndx = -1; // What "0" gets turned into by the Wavefront-conversion-translation code
		}
	}
	
	if (!AllInRange) return false;

	/* http://wiki.blender.org/index.php/Dev:Shading/Tangent_Space_Normal_Maps */
	/* SPLIT ALL EDGES, needed for proper tangent space */
	/* note:
		this procedure still does not get rid of all hard edges,
		maybe we need to build the btn matrix in the fragment shader instead of vertex shader
	*/

	unsigned IndxBase = 0;
	unsigned Indx = 0;
	for(unsigned i = 0; i < PolygonSizes.size(); ++i) {

		const short& n = PolygonSizes[i];
		vertex3 V[n];
		vertex2 C[n];
		vec3 N[n];

		for(int j = 0; j < n; ++j) {

			const VertexIndexSet& a = VertIndxSets[IndxBase+j];
			V[j] = vertex3(&Positions[3*a.PosIndx]);
			if(WhatsPresent & Present_TxtrCoord) {
				C[j] = vertex2(&TxtrCoords[2*a.TCIndx]);
			}
			if (WhatsPresent & Present_Normal) {
				N[j] = vec3(&Normals[3*a.NormIndx]);
			}
		}

		// triangulate

		if(n == 4) {

			unsigned order[6];

			if((V[2]-V[0]).d() < (V[3]-V[1]).d()) {

				memcpy(order, (unsigned[]) {0, 1, 3, 1, 2, 3}, sizeof(order));
				// but contrary to the blender wiki this cut is across the longer side ?!?
				// but vice versa it looks worse

			} else if((V[2]-V[0]).d() > (V[3]-V[1]).d()) {

				memcpy(order, (unsigned[]) {0, 1, 2, 0, 2, 3}, sizeof(order));
				// ditto

			} else {
	
				if((WhatsPresent & Present_TxtrCoord) && (C[0].distance(C[2]) < C[1].distance(C[3]))) {

					memcpy(order, (unsigned[]) {0, 1, 3, 1, 2, 3}, sizeof(order));
					// ditto

				} else {

					memcpy(order, (unsigned[]) {0, 1, 2, 0, 2, 3}, sizeof(order));
					// ditto
				}
			}

			for(unsigned j = 0; j < 6; ++j) {

				addVertex(Model, V[order[j]]);

				if(WhatsPresent & Present_TxtrCoord) {
					addTexCoord(Model, C[order[j]]);
				}
				if (WhatsPresent & Present_Normal) {
					addNormal(Model, N[order[j]]);
				}
				Model.VertIndices.push_back(Indx++);		
			}

		} else {

			// TODO properly cut if more than 4 sides
			for(unsigned  j = 0; j < n-2; ++j) {

				addVertex(Model, V[0]);
				addVertex(Model, V[j+1]);
				addVertex(Model, V[j+2]);

				if(WhatsPresent & Present_TxtrCoord) {
					addTexCoord(Model, C[0]);
					addTexCoord(Model, C[j+1]);
					addTexCoord(Model, C[j+2]);
				}

				if (WhatsPresent & Present_Normal) {
					addNormal(Model, N[0]);
					addNormal(Model, N[j+1]);
					addNormal(Model, N[j+2]);
				}

				Model.VertIndices.push_back(Indx++);
				Model.VertIndices.push_back(Indx++);
				Model.VertIndices.push_back(Indx++);
			}
		}

		IndxBase += n;
	}
/*
	// Find unique vertex sets:
	
	// First, do an index sort of them
	vector<int> VertIndxRefs(VertIndxSets.size());
	for (unsigned k=0; k<VertIndxRefs.size(); k++) {
		VertIndxRefs[k] = k;
	}
	
	IndexedVertListCompare Compare(VertIndxSets);
//	Compare.VertIndxSets = &VertIndxSets[0];
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

		const VertexIndexSet& vis = VertIndxSets[n];
		
		if (vis.PosIndx == PrevPosIndx && vis.TCIndx == PrevTCIndx && vis.NormIndx == PrevNormIndx)
		{
			WhichUniqueSet[n] = NumUnique;
			continue;
		}
		
		// Found a unique set
		WhichUniqueSet[n] = ++NumUnique;
		
		// These are all for the model object
		
		// Load the positions
		{
			GLfloat *PosPtr = &Positions[3*vis.PosIndx];
			for (int m=0; m<3; m++)
				Model.Positions.push_back(*(PosPtr++));
		}
		
		// Load the texture coordinates
		if (WhatsPresent & Present_TxtrCoord)
		{
			GLfloat *TCPtr = &TxtrCoords[2*vis.TCIndx];
			for (int m=0; m<2; m++)
				Model.TxtrCoords.push_back(*(TCPtr++));
		}
		
		// Load the normals
		if (WhatsPresent & Present_Normal)
		{
			GLfloat *NormPtr = &Normals[3*vis.NormIndx];
			for (int m=0; m<3; m++)
				Model.Normals.push_back(*(NormPtr++));
		}
		
		// Save these new unique-set values for comparison to the next ones
		PrevPosIndx = vis.PosIndx;
		PrevTCIndx = vis.TCIndx;
		PrevNormIndx = vis.NormIndx;
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
*/
	if (Model.VertIndices.size() <= 0)
	{
		logError1("ERROR: the model in %s has no good polygons",Path);
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
