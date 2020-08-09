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

	3D-Model Object Storage
	It is intended to be as OpenGL-friendly as is reasonably possible
	
	By Loren Petrich, June 16, 2001
*/

#if defined _MSC_VER
#define NOMINMAX
#include <algorithm>
#endif

#ifndef MODEL_3D
#define MODEL_3D

#include "cseries.h"

#ifdef HAVE_OPENGL

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

#include <vector>
#include "vec3.h"

using std::vector;


// For boned models; calculate vertices from these
struct Model3D_VertexSource
{
	GLfloat Position[3];	// Use this directly for the neutral positions
	GLshort Bone0, Bone1;	// Indices of the two bones (NONE is no bone)
	GLfloat Blend;			// Blend factor: limits are 0 = first bone, 1 = second bone.
};


// Bone definition: the bones are in order of traversal, with the bone tree
// specified with pushing and popping of bones on the stack.
// There is an assumed root bone below all the others.
struct Model3D_Bone
{
	enum
	{
		Pop		= 0x0001,
		Push	= 0x0002
	};
	
	// The bones' positions are all referred to the overall coordinate system
	// instead of to their parents' positions, as in Tomb Raider.
	GLfloat Position[3];
	
	// Tomb-Raider-like system for traversing a tree with reference to a stack of bones;
	// "pop" removes a bone from that stack and makes it the current one, while
	// "push" adds the current bone to that stack.
	// The pop is done before the push if both are present;
	// both are done before using the bone.
	GLushort Flags;
};


// Frame definition: a set of bone transforms, one for each bone in order.
// The angles are in a form suitable for the Marathon engine's trig-function lookup tables.
struct Model3D_Frame
{
	GLfloat Offset[3];
	GLshort Angles[3];
};

// In Brian Barnes's code, a sequence has some overall transformations in it
struct Model3D_SeqFrame: public Model3D_Frame
{
	GLshort Frame;
};


// For including a transformation from read-in model space to rendered model space;
// also used to implement the skeletal animation.
struct Model3D_Transform
{
	GLfloat M[3][4];
	
	void Identity(); // Sets to the identity matrix
};


struct Model3D
{
	// Assumed dimensions:
	// Positions: 3
	// Texture coordinates: 2
	// Normals: 3
	// Colors: 3 [should an alpha channel also be included?]
	
	// Positions assumed to be 3-dimensional
	vector<GLfloat> Positions;
	GLfloat *PosBase() {return &Positions[0];}
	
	// Texture coordinates assumed to be 2-dimensional
	// Parallel to the vertex-position array
	vector<GLfloat> TxtrCoords;
	GLfloat *TCBase() {return &TxtrCoords[0];}
	
	// Normals assumed to be 3-dimensional
	// Parallel to the vertex-position array
	vector<GLfloat> Normals;
	GLfloat *NormBase() {return &Normals[0];}
	
	// Tangents assumed to be 4-dimensional
	vector<vec4> Tangents;
	GLfloat *TangentBase() {return &Tangents[0][0];}
	
	// Vertex colors (useful for by-hand vertex lighting)
	// Parallel to the vertex-position array
	vector<GLfloat> Colors;
	GLfloat *ColBase() {return &Colors[0];}
	
	// Vertex-source indices; into vertex-source array:
	vector<GLushort> VtxSrcIndices;
	GLushort *VtxSIBase() {return &VtxSrcIndices[0];}
	
	// Vertex-source array: useful with boned models
	vector<Model3D_VertexSource> VtxSources;
	Model3D_VertexSource *VtxSrcBase() {return &VtxSources[0];}
	
	// Normal-source array; has the same dimension as the normals if used.
	vector<GLfloat> NormSources;
	GLfloat *NormSrcBase() {return &NormSources[0];}
	
	// Vertex-source inverse indices:
	// list of all the vertex indices associated with each vertex source,
	// with a list of pointer indices into that list. Which has an extra pointer
	// for just off the end of the last, to simplify the readoff
	vector<GLushort> InverseVSIndices;
	GLushort *InverseVIBase() {return &InverseVSIndices[0];}
	vector<GLushort> InvVSIPointers;
	GLushort *InvVSIPtrBase() {return &InvVSIPointers[0];}
	
	// Bone array: the bones are in traversal order
	vector<Model3D_Bone> Bones;
	Model3D_Bone *BoneBase() {return &Bones[0];}
	
	// List of indices into the aforementioned vertices;
	// the list is a list of triangles.
	vector<GLushort> VertIndices;
	GLushort *VIBase() {return &VertIndices[0];}
	size_t NumVI() {return VertIndices.size();}
	
	// Frame array: each member is actually the transform to do on each bone;
	// each frame has [number of bones] of these.
	vector<Model3D_Frame> Frames;
	Model3D_Frame *FrameBase() {return &Frames[0];}
	
	// True number of frames: the above number divided by the number of bones;
	// return zero if no bones
	size_t TrueNumFrames() {return Bones.empty() ? 0 : (Frames.size()/Bones.size());}
	
	// Sequence frames:
	vector<Model3D_SeqFrame> SeqFrames;
	Model3D_SeqFrame *SeqFrmBase() {return &SeqFrames[0];}
	
	// Sequence-frame pointer indices: actually one more than there are sequences,
	// to simplify finding which frames are members -- much like the vertex-source
	// inverse indices.
	vector<GLushort> SeqFrmPointers;
	GLushort *SFPtrBase() {return &SeqFrmPointers[0];}
	
	// True number of sequences:
	int TrueNumSeqs() {return std::max(int(SeqFrmPointers.size()-1),0);}
	
	// Add-on transforms for the positions and the normals
	Model3D_Transform TransformPos, TransformNorm;
	
	// Bounding box (first index: 0 = min, 1 = max)
	GLfloat BoundingBox[2][3];
	
	// From the position data
	void FindBoundingBox();
	
#if 0
	// For debugging bounding-box-handling code
	// NULL means don't render one set of edges
	void RenderBoundingBox(const GLfloat *EdgeColor, const GLfloat *DiagonalColor);
#endif
	
	// Process the normals in various ways
	enum
	{
		None,					// Gets rid of them
		Original,				// Uses the model's original normals
		Reversed,				// Reverses the direction of the original normals
		ClockwiseSide,			// Normals point in the sides' clockwise direction
		CounterclockwiseSide,	// Normals point in the sides' counterclockwise direction
		NUMBER_OF_NORMAL_TYPES
	};
	// The second is for deciding whether a vertex is to have
	// the average of its neighboring polygons' normals
	// or whether a vertex is to be split into separate vertices,
	// each with a polygon's normal
	void AdjustNormals(int NormalType, float SmoothThreshold = 0.5);
	
	// So they all have length 1
	void NormalizeNormals() {AdjustNormals(Original);}
	void CalculateTangents();

	// Erase everything
	void Clear();
	
	// Build the trig tables for use in doing the transformations for frames and sequences;
	// do this if build_trig_tables() in world.h was not already called elsewhere.
	static void BuildTrigTables();
	
	// Build vertex-source inverse indices; these are for speeding up the translation process.
	void BuildInverseVSIndices();
	
	// Find positions of vertices
	// when a vertex-source array, bones, frames, and sequences are present.
	// No arguments is for the model's neutral position (uses only source array);
	// this is also the case for bad frame and sequence indices.
	
	// The position-finding has an argument that determines whether to use the model's
	// overall transform -- only applied to animated models.
	
	// The frame and sequence position finders have a crossfade fraction
	// and a second frame ID; these are for making smooth animations.
	
	// Returns whether vertex-source data was used (present in animated models)
	bool FindPositions_Neutral(bool UseModelTransform);
	
	// Returns whether or not the indices were within range.
	bool FindPositions_Frame(bool UseModelTransform,
		GLshort FrameIndex, GLfloat MixFrac = 0, GLshort AddlFrameIndex = 0);
	
	// returns 0 for out-of-range sequence
	GLshort NumSeqFrames(GLshort SeqIndex);
	
	// Returns whether or not the indices were in range.
	bool FindPositions_Sequence(bool UseModelTransform, GLshort SeqIndex,
		GLshort FrameIndex, GLfloat MixFrac = 0, GLshort AddlFrameIndex = 0);
	
	// Constructor
	Model3D() {FindBoundingBox(); TransformPos.Identity(); TransformNorm.Identity();}
};

#endif
struct Model3D;
#endif
