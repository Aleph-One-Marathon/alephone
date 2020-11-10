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

	3D-Model Object Storage Functions
	
	By Loren Petrich, July 8, 2001
*/

#include <string.h>
#include <math.h>

#include "VecOps.h"
#include "cseries.h"
#include "world.h"

#ifdef HAVE_OPENGL

#include "Model3D.h"

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#endif

/* Need Sgl* macros */
#include "OGL_Setup.h"

// Bone-stack and transformation-matrix locally-used arrays;
// the matrices have dimensions (output coords)(input-coord multipliers + offset for output)
static vector<Model3D_Transform> BoneMatrices;
static vector<size_t> BoneStack;


// Find transform of point (source and dest must be different arrays)
inline void TransformPoint(GLfloat *Dest, GLfloat *Src, Model3D_Transform& T)
{
	for (int ic=0; ic<3; ic++)
	{
		GLfloat *Row = T.M[ic];
		Dest[ic] = ScalarProd(Src,Row) + Row[3];
	}
}

// Like above, but a vector, such as a normal (source and dest must be different arrays)
inline void TransformVector(GLfloat *Dest, GLfloat *Src, Model3D_Transform& T)
{
	for (int ic=0; ic<3; ic++)
	{
		GLfloat *Row = T.M[ic];
		Dest[ic] = ScalarProd(Src,Row);
	}
}

// Bone and Frame (positions, angles) -> Transform Matrix
static void FindFrameTransform(Model3D_Transform& T,
	Model3D_Frame& Frame, GLfloat MixFrac, Model3D_Frame& AddlFrame);
static void FindBoneTransform(Model3D_Transform& T, Model3D_Bone& Bone,
	Model3D_Frame& Frame, GLfloat MixFrac, Model3D_Frame& AddlFrame);

// Res = A * B, in that order
static void TMatMultiply(Model3D_Transform& Res, Model3D_Transform& A, Model3D_Transform& B);

	
// Trig-function conversion:
const GLfloat TrigNorm = GLfloat(1)/GLfloat(TRIG_MAGNITUDE);


// Erase everything
void Model3D::Clear()
{
	Positions.clear();
	TxtrCoords.clear();
	Normals.clear();
	Colors.clear();
	VtxSrcIndices.clear();
	VtxSources.clear();
	NormSources.clear();
	InverseVSIndices.clear();
	InvVSIPointers.clear();
	Bones.clear();
	VertIndices.clear();
	Frames.clear();
	SeqFrames.clear();
	SeqFrmPointers.clear();
	FindBoundingBox();
}

// Normalize an individual normal; return whether the normal had a nonzero length
static bool NormalizeNormal(GLfloat *Normal)
{
	GLfloat NormalSqr =
		Normal[0]*Normal[0] + Normal[1]*Normal[1] + Normal[2]*Normal[2];
	
	if (NormalSqr <= 0) return false;
	
	GLfloat NormalRecip = (GLfloat)(1/sqrt(NormalSqr));
	
	Normal[0] *= NormalRecip;
	Normal[1] *= NormalRecip;
	Normal[2] *= NormalRecip;
	
	return true;
}

// Flagged vector for per-polygon and per-vertex normals;
// the flag is "true" for nonzero vectors and vectors meant to be used
struct FlaggedVector
{
	GLfloat Vec[3];
	bool Flag;
};

void Model3D::CalculateTangents()
{
	Tangents.resize(Positions.size() * 4 / 3);
	
	bool generate_normals = false;
	if(Normals.size() != Positions.size()) {
		Normals.resize(Positions.size());
		memset(NormBase(), 0, sizeof(GLfloat)*Normals.size());
		generate_normals = true;
	}
	
	for(GLushort i = 0; i < VertIndices.size(); i+= 3) {
		GLushort a = VertIndices[i];
		GLushort b = VertIndices[i+1];
		GLushort c = VertIndices[i+2];
		
		vertex3 v1(PosBase()+3*a);
		vertex3 v2(PosBase()+3*b);
		vertex3 v3(PosBase()+3*c);
		
		vertex2 w1(0, 0);
		vertex2 w2(0, 0);
		vertex2 w3(0, 0);
		if (TxtrCoords.size()) {
			w1 = vertex2(TCBase()+2*a);
			w2 = vertex2(TCBase()+2*b);
			w3 = vertex2(TCBase()+2*c);
		}
		
		float x1 = v2[0] - v1[0];
		float x2 = v3[0] - v1[0];
		float y1 = v2[1] - v1[1];
		float y2 = v3[1] - v1[1];
		float z1 = v2[2] - v1[2];
		float z2 = v3[2] - v1[2];
		
		float s1 = w2[0] - w1[0];
		float s2 = w3[0] - w1[0];
		float t1 = w2[1] - w1[1];
		float t2 = w3[1] - w1[1];
		
		float r = 1.0f / (s1 * t2 - s2 * t1);
		vec3 T((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			   (t2 * z1 - t1 * z2) * r);
		vec3 B((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			   (s1 * z2 - s2 * z1) * r);
		if ((s1 * t2 - s2 * t1) == 0.0) {
			T = (v3 - v1).norm();
			B = (v2 - v1).norm();
		}
		
		vec3 N = (v3-v1).cross(v2-v1);
		if (!generate_normals) {
			N = vec3(NormBase()+3*a) + vec3(NormBase()+3*b) + vec3(NormBase()+3*c);
		}
		
		if(N.dot(N) < 0.001) {
			N = vec3(0.0,0.0,0.0);
			if (generate_normals) {
				VecCopy(N.p(), NormBase()+3*a);
				VecCopy(N.p(), NormBase()+3*b);
				VecCopy(N.p(), NormBase()+3*c);
			}
			
			vec4 t(0.0,0.0,0.0,0.0);
			Tangents[a] = vec4(t);
			Tangents[b] = vec4(t);
			Tangents[c] = vec4(t);
		} else {
			N = N.norm();
			assert(N.dot(N) < 1.001);
			
			if (generate_normals) {
				VecCopy(N.p(), NormBase()+3*a);
				VecCopy(N.p(), NormBase()+3*b);
				VecCopy(N.p(), NormBase()+3*c);
			}
			
			float sign = (N.cross(T)).dot(B) < 0.0 ? -1.0 : 1.0;
			vec4 t = (T - N * N.dot(T)).norm();
			t[3] = sign;
			
			Tangents[a] = vec4(t);
			Tangents[b] = vec4(t);
			Tangents[c] = vec4(t);
		}
	}
}


// Normalize the normals
void Model3D::AdjustNormals(int NormalType, float SmoothThreshold)
{
	// Copy in normal sources for processing
	if (!NormSources.empty())
	{
		Normals.resize(NormSources.size());
		objlist_copy(NormBase(),NormSrcBase(),NormSources.size());
	}
	
	// Which kind of special processing?
	switch(NormalType)
	{
	case None:
		Normals.clear();
		break;
		
	case Original:
	case Reversed:
	default:
		// Normalize
		for (unsigned k=0; k<Normals.size()/3; k++)
			NormalizeNormal(&Normals[3*k]);
		break;
	
	case ClockwiseSide:
	case CounterclockwiseSide:
		// The really interesting stuff
		{
			// First, create a list of per-polygon normals
			size_t NumPolys = NumVI()/3;
			vector<FlaggedVector> PerPolygonNormalList(NumPolys);
			
			GLushort *IndxPtr = VIBase();
			for (unsigned k=0; k<NumPolys; k++)
			{
				// The three vertices:
				GLfloat *P0 = &Positions[3*(*(IndxPtr++))];
				GLfloat *P1 = &Positions[3*(*(IndxPtr++))];
				GLfloat *P2 = &Positions[3*(*(IndxPtr++))];
				// The two in-polygon vectors:
				GLfloat P01[3];
				P01[0] = P1[0] - P0[0];
				P01[1] = P1[1] - P0[1];
				P01[2] = P1[2] - P0[2];
				GLfloat P02[3];
				P02[0] = P2[0] - P0[0];
				P02[1] = P2[1] - P0[1];
				P02[2] = P2[2] - P0[2];
				// Those vectors' normal:
				FlaggedVector& PPN = PerPolygonNormalList[k];
				PPN.Vec[0] = P01[1]*P02[2] - P01[2]*P02[1];
				PPN.Vec[1] = P01[2]*P02[0] - P01[0]*P02[2];
				PPN.Vec[2] = P01[0]*P02[1] - P01[1]*P02[0];
				PPN.Flag = NormalizeNormal(PPN.Vec);
			}
			
			// Create a list of per-vertex normals
			size_t NumVerts = Positions.size()/3;
			vector<FlaggedVector> PerVertexNormalList(NumVerts);
			objlist_clear(&PerVertexNormalList[0],NumVerts);
			IndxPtr = VIBase();
			for (unsigned k=0; k<NumPolys; k++)
			{
				FlaggedVector& PPN = PerPolygonNormalList[k];
				for (unsigned c=0; c<3; c++)
				{
					GLushort VertIndx = *(IndxPtr++);
					GLfloat *V = PerVertexNormalList[VertIndx].Vec;
					*(V++) += PPN.Vec[0];
					*(V++) += PPN.Vec[1];
					*(V++) += PPN.Vec[2];
				}
			}
			
			// Normalize the per-vertex normals
			for (unsigned k=0; k<NumVerts; k++)
			{
				FlaggedVector& PVN = PerVertexNormalList[k];
				PVN.Flag = NormalizeNormal(PVN.Vec);
			}
			
			// Find the variance of each of the per-vertex normals;
			// use that to decide whether to keep them unsplit;
			// this also needs counting up the number of polygons per vertex.
			vector<GLfloat> Variances(NumVerts);
			objlist_clear(&Variances[0],NumVerts);
			vector<short> NumPolysPerVert(NumVerts);
			objlist_clear(&NumPolysPerVert[0],NumVerts);
			IndxPtr = VIBase();
			for (unsigned k=0; k<NumPolys; k++)
			{
				FlaggedVector& PPN = PerPolygonNormalList[k];
				for (unsigned c=0; c<3; c++)
				{
					GLushort VertIndx = *(IndxPtr++);
					FlaggedVector& PVN = PerVertexNormalList[VertIndx];
					if (PVN.Flag)
					{
						GLfloat *V = PVN.Vec;
						GLfloat D0 = *(V++) - PPN.Vec[0];
						GLfloat D1 = *(V++) - PPN.Vec[1];
						GLfloat D2 = *(V++) - PPN.Vec[2];
						Variances[VertIndx] += (D0*D0 + D1*D1 + D2*D2);
						NumPolysPerVert[VertIndx]++;
					}
				}
			}
			
			// Decide whether to split each vertex;
			// if the flag is "true", a vertex is not to be split
			for (unsigned k=0; k<NumVerts; k++)
			{
				// Vertices without contributions will automatically have
				// their flags be false, as a result of NormalizeNormal()
				unsigned NumVertPolys = NumPolysPerVert[k];
				if (NumVertPolys > 0 && PerVertexNormalList[k].Flag)
					PerVertexNormalList[k].Flag =
						sqrt(Variances[k]/NumVertPolys) <= SmoothThreshold;
			}
			
			// The vertex flags are now set for whether to use that vertex's normal;
			// re-count the number of polygons per vertex.
			// Use NONE for unsplit ones
			objlist_clear(&NumPolysPerVert[0],NumVerts);
			IndxPtr = VIBase();
			for (unsigned k=0; k<NumPolys; k++)
			{
				for (unsigned c=0; c<3; c++)
				{
					GLushort VertIndx = *(IndxPtr++);
					FlaggedVector& PVN = PerVertexNormalList[VertIndx];
					if (PVN.Flag)
						NumPolysPerVert[VertIndx] = NONE;
					else
						NumPolysPerVert[VertIndx]++;
				}
			}
			
			// Construct a polygon-association list; this will indicate
			// which polygons are associated with each of the resulting instances
			// of a split vertex (unsplit: NONE).
			// NumPolysPerVert will be recycled as a counter list,
			// after being used to construct a cumulative index-in-list array.
			// Finding that list will be used to find how many new vertices there are.
			vector<short> IndicesInList(NumVerts);
			short IndxInList = 0;
			for (unsigned k=0; k<NumVerts; k++)
			{
				IndicesInList[k] = IndxInList;
				FlaggedVector& PVN = PerVertexNormalList[k];
				IndxInList += PVN.Flag ? 1 : NumPolysPerVert[k];
			}
			GLushort NewNumVerts = IndxInList;
			vector<short> VertexPolygons(NewNumVerts);
			objlist_clear(&NumPolysPerVert[0],NumVerts);
			
			// In creating that list, also remap the triangles' vertices
			GLushort *VIPtr = VIBase();
			for (unsigned k=0; k<NumPolys; k++)
			{
				for (unsigned c=0; c<3; c++)
				{
					GLushort VertIndx = *VIPtr;
					GLushort NewVertIndx = IndicesInList[VertIndx];
					FlaggedVector& PVN = PerVertexNormalList[VertIndx];
					if (PVN.Flag)
					{
						NumPolysPerVert[VertIndx] = NONE;
						VertexPolygons[NewVertIndx] = NONE;
					}
					else
					{
						NewVertIndx += (NumPolysPerVert[VertIndx]++);
						VertexPolygons[NewVertIndx] = k;
					}
					*VIPtr = NewVertIndx;
					VIPtr++;
				}
			}
			
			// Split the vertices
			vector<GLfloat> NewPositions(3*NewNumVerts);
			vector<GLfloat> NewTxtrCoords;
			vector<GLfloat> NewNormals(3*NewNumVerts);
			vector<GLfloat> NewColors;
			vector<GLushort> NewVtxSrcIndices;
			
			bool TCPresent = !TxtrCoords.empty();
			if (TCPresent) NewTxtrCoords.resize(2*NewNumVerts);
			
			bool ColPresent = !Colors.empty();
			if (ColPresent) NewColors.resize(3*NewNumVerts);
			
			bool VSPresent = !VtxSrcIndices.empty();
			if (VSPresent) NewVtxSrcIndices.resize(NewNumVerts);
			
			// Use marching pointers to speed up the copy-over
			GLfloat *OldP = &Positions[0];
			GLfloat *NewP = &NewPositions[0];
			GLfloat *OldT = &TxtrCoords[0];
			GLfloat *NewT = &NewTxtrCoords[0];
			GLfloat *OldC = &Colors[0];
			GLfloat *NewC = &NewColors[0];
			GLushort *OldS = &VtxSrcIndices[0];
			GLushort *NewS = &NewVtxSrcIndices[0];
			GLfloat *NewN = &NewNormals[0];
			for (unsigned k=0; k<NumVerts; k++)
			{
				FlaggedVector& PVN = PerVertexNormalList[k];
				unsigned NumVertPolys = PVN.Flag ? 1 : NumPolysPerVert[k];
				for (unsigned c=0; c<NumVertPolys; c++)
				{
					GLfloat *OldPP = OldP;
					*(NewP++) = *(OldPP++);
					*(NewP++) = *(OldPP++);
					*(NewP++) = *(OldPP++);
				}
				if (TCPresent)
				{
					for (unsigned c=0; c<NumVertPolys; c++)
					{
						GLfloat *OldTP = OldT;
						*(NewT++) = *(OldTP++);
						*(NewT++) = *(OldTP++);
					}
				}
				if (ColPresent)
				{
					for (unsigned c=0; c<NumVertPolys; c++)
					{
						GLfloat *OldCP = OldC;
						*(NewC++) = *(OldCP++);
						*(NewC++) = *(OldCP++);
						*(NewC++) = *(OldCP++);
					}
				}
				if (VSPresent)
				{
					for (unsigned c=0; c<NumVertPolys; c++)
						*(NewS++) = *OldS;
				}
				if (PVN.Flag)
				{
					GLfloat *VP = PVN.Vec;
					*(NewN++) = *(VP++);
					*(NewN++) = *(VP++);
					*(NewN++) = *(VP++);
				}
				else
				{
					// A reference so that the incrementing can work on it
					short& IndxInList = IndicesInList[k];
					for (unsigned c=0; c<NumVertPolys; c++)
					{
						GLfloat *VP = PerPolygonNormalList[VertexPolygons[IndxInList++]].Vec;
						*(NewN++) = *(VP++);
						*(NewN++) = *(VP++);
						*(NewN++) = *(VP++);
					}	
				}
				
				// Advance!
				OldP += 3;
				if (TCPresent)
					OldT += 2;
				if (ColPresent)
					OldC += 3;
				if (VSPresent)
					OldS++;
					
			}
			assert(OldP == &Positions[3*NumVerts]);
			assert(NewP == &NewPositions[3*NewNumVerts]);
			if (TCPresent)
			{
				assert(OldT == &TxtrCoords[2*NumVerts]);
				assert(NewT == &NewTxtrCoords[2*NewNumVerts]);
			}
			if (ColPresent)
			{
				assert(OldC == &Colors[3*NumVerts]);
				assert(NewC == &NewColors[3*NewNumVerts]);
			}
			if (VSPresent)
			{
				assert(OldS == &VtxSrcIndices[NumVerts]);
				assert(NewS == &NewVtxSrcIndices[NewNumVerts]);				
			}
			assert(NewN == &NewNormals[3*NewNumVerts]);
			
			// Accept the new vectors
			Positions.swap(NewPositions);
			TxtrCoords.swap(NewTxtrCoords);
			Normals.swap(NewNormals);
			Colors.swap(NewColors);
			VtxSrcIndices.swap(NewVtxSrcIndices);
		}
		break;
	}
	
	// Now flip
	switch(NormalType)
	{
	case Reversed:
	case CounterclockwiseSide:
		{
			GLfloat *NormalPtr = NormBase();
			for (unsigned k=0; k<Normals.size(); k++)
				*(NormalPtr++) *= -1;
		}
	}
	
	// Copy back out to the normal sources;
	// do the copying out if the vertices have sources,
	// which is the case for boned models.
	if (!VtxSources.empty())
	{
		size_t NormSize = Normals.size();
		if (NormSize > 0)
		{
			NormSources.resize(NormSize);
			objlist_copy(NormSrcBase(),NormBase(),NormSize);
		}
		else
			NormSources.clear();
	}
	else
		NormSources.clear();
}

	
// From the position data
void Model3D::FindBoundingBox()
{
	size_t NumVertices = Positions.size()/3;
	if (NumVertices > 0)
	{
		// Find the min and max of the positions:
		VecCopy(&Positions[0],BoundingBox[0]);
		VecCopy(&Positions[0],BoundingBox[1]);
		for (size_t i=1; i<NumVertices; i++)
		{
			GLfloat *Pos = &Positions[3*i];
			for (int ib=0; ib<3; ib++)
			{
				BoundingBox[0][ib] = MIN(BoundingBox[0][ib],Pos[ib]);
				BoundingBox[1][ib] = MAX(BoundingBox[1][ib],Pos[ib]);
			}
		}
	}
	else
	{
		// Clear the bounding box
		objlist_clear(BoundingBox[0],3);
		objlist_clear(BoundingBox[1],3);
	}
}


#if 0
// For debugging
void Model3D::RenderBoundingBox(const GLfloat *EdgeColor, const GLfloat *DiagonalColor)
{
	GLfloat BBoxVertices[8][3];
	
	// Binary-number arrangement of expanded vertices:
	for (int i1=0; i1<2; i1++)
		for (int i2=0; i2<2; i2++)
			for (int i3=0; i3<2; i3++)
			{
				int Indx = 4*i1 + 2*i2 + i3;
				BBoxVertices[Indx][0] = BoundingBox[i1][0];
				BBoxVertices[Indx][1] = BoundingBox[i2][1];
				BBoxVertices[Indx][2] = BoundingBox[i3][2];
			}
	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,BBoxVertices[0]);
	
	if (EdgeColor)
	{
		SglColor3fv(EdgeColor);
		const int NumEdgeVerts = 24;
		const unsigned short EdgeVerts[NumEdgeVerts] = {
			0,1,
			1,3,
			3,2,
			2,0,
			
			0,4,
			1,5,
			2,6,
			3,7,
			
			4,5,
			5,7,
			7,6,
			6,4
		};
		glDrawElements(GL_LINES,NumEdgeVerts,GL_UNSIGNED_SHORT,EdgeVerts);
	}
	
	if (DiagonalColor)
	{
		SglColor3fv(DiagonalColor);
		const int NumDiagVerts = 24;
		const unsigned short DiagVerts[NumDiagVerts] = {
			0,3,
			1,2,
			
			0,5,
			1,4,
			
			1,7,
			3,5,
			
			3,6,
			2,7,
			
			2,4,
			0,6,
			
			4,7,
			5,6
		};
		glDrawElements(GL_LINES,NumDiagVerts,GL_UNSIGNED_SHORT,DiagVerts);
	}
}
#endif

void Model3D::BuildTrigTables()
{
	build_trig_tables();
}


void Model3D::BuildInverseVSIndices()
{
	if (VtxSrcIndices.empty()) return;
	
	InverseVSIndices.resize(VtxSrcIndices.size());
	InvVSIPointers.resize(VtxSources.size()+1);		// One extra member
	
	// Use the pointers as temporary storage for the count
	objlist_clear(InvVSIPtrBase(),InvVSIPointers.size());	
	for (vector<GLushort>::iterator VSI_Iter = VtxSrcIndices.begin();
		VSI_Iter < VtxSrcIndices.end();
		VSI_Iter++)
			InvVSIPointers[*VSI_Iter]++;
	
	// Find the positions from the counts
	GLushort PtrSum = 0;
	for (vector<GLushort>::iterator IVP_Iter = InvVSIPointers.begin();
		IVP_Iter < InvVSIPointers.end();
		IVP_Iter++)
		{
			GLushort NewPtrSum = PtrSum + *IVP_Iter;
			*IVP_Iter = PtrSum;
			PtrSum = NewPtrSum;
		}
	
	// Place the inverse indices
	for (unsigned k = 0; k<VtxSrcIndices.size(); k++)
		InverseVSIndices[InvVSIPointers[VtxSrcIndices[k]]++] = k;
	
	// Push the pointer values forward in the list
	// since they'd become their next values in it.
	// The reverse iteration is necessary to avoid overwriting
	for (vector<GLushort>::iterator IVP_Iter = InvVSIPointers.end()-1;
		IVP_Iter > InvVSIPointers.begin();
		IVP_Iter--)
		{
			*IVP_Iter = *(IVP_Iter - 1);
		}
	InvVSIPointers[0] = 0;
}


// Neutral case: returns whether vertex-source data was used (present in animated models)
bool Model3D::FindPositions_Neutral(bool UseModelTransform)
{
	// Positions already there
	if (VtxSrcIndices.empty()) return false;
	
	// Straight copy of the vertices:
	
	size_t NumVertices = VtxSrcIndices.size();
	Positions.resize(3*NumVertices);
	
	GLfloat *PP = PosBase();
	GLushort *IP = VtxSIBase();
	
	size_t NumVtxSources = VtxSources.size();
	
	if (UseModelTransform)
	{
		for (size_t k=0; k<NumVertices; k++, IP++, PP+=3)
		{
			size_t VSIndex = *IP;
			if (VSIndex >= 0 && VSIndex < NumVtxSources)
			{
				Model3D_VertexSource& VS = VtxSources[VSIndex];
				TransformPoint(PP,VS.Position,TransformPos);
			}
			else
			{
				GLfloat VP[3] = {0,0,0};
				TransformPoint(PP,VP,TransformPos);
			}
		}
	}
	else
	{
		for (size_t k=0; k<NumVertices; k++, IP++)
		{
			size_t VSIndex = *IP;
			if (VSIndex >= 0 && VSIndex < NumVtxSources)
			{
				Model3D_VertexSource& VS = VtxSources[VSIndex];
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
	}
	
	// Copy in the normals
	Normals.resize(NormSources.size());
	
	if (UseModelTransform)
	{
		GLfloat *NormPtr = NormBase();
		GLfloat *NormBasePtr = NormSrcBase();
		size_t NumNorms = NormSources.size()/3;
		for (size_t k=0; k<NumNorms; k++, NormPtr+=3, NormBasePtr+=3)
		{
			TransformVector(NormPtr, NormBasePtr, TransformNorm);			
		}
	}
	else
	{
		objlist_copy(NormBase(),NormSrcBase(),NormSources.size());
	}
	
	return true;
}

// Frame case
bool Model3D::FindPositions_Frame(bool UseModelTransform,
	GLshort FrameIndex, GLfloat MixFrac, GLshort AddlFrameIndex)
{
	// Bad inputs: do nothing and return false
	
	if (Frames.empty()) return false;
	
	size_t NumBones = Bones.size();
	if (FrameIndex < 0 || NumBones*FrameIndex >= Frames.size()) return false;
	
	if (InverseVSIndices.empty()) BuildInverseVSIndices();
	
	size_t NumVertices = VtxSrcIndices.size();
	Positions.resize(3*NumVertices);
	
	// Set sizes:
	BoneMatrices.resize(NumBones);
	BoneStack.resize(NumBones);
	
	// Find which frame; remember that frame data comes in [NumBones] sets
	Model3D_Frame *FramePtr = &Frames[NumBones*FrameIndex];
	Model3D_Frame *AddlFramePtr = &Frames[NumBones*AddlFrameIndex];
	
	// Find the individual-bone transformation matrices:
	for (size_t ib=0; ib<NumBones; ib++)
		FindBoneTransform(BoneMatrices[ib],Bones[ib],
			FramePtr[ib],MixFrac,AddlFramePtr[ib]);
	
	// Find the cumulative-bone transformation matrices:
	int StackIndx = -1;
	size_t Parent = UNONE;
	for (unsigned int ib=0; ib<NumBones; ib++)
	{
		Model3D_Bone& Bone = Bones[ib];
		
		// Do the pop-push with the stack
		// to get the bone's parent bone
		if (TEST_FLAG(Bone.Flags,Model3D_Bone::Pop))
		{
			if (StackIndx >= 0)
				Parent = BoneStack[StackIndx--];
			else
				Parent = UNONE;
		}
		if (TEST_FLAG(Bone.Flags,Model3D_Bone::Push))
		{
			StackIndx = MAX(StackIndx,-1);
			BoneStack[++StackIndx] = Parent;
		}
		
		// Do the transform!
		if (Parent != UNONE)
		{
			Model3D_Transform Res;
			TMatMultiply(Res,BoneMatrices[Parent],BoneMatrices[ib]);
			obj_copy(BoneMatrices[ib],Res);
		}
	
		// Default: parent of next bone is current bone
		Parent = ib;
	}
		
	bool NormalsPresent = !NormSources.empty();
	if (NormalsPresent) Normals.resize(NormSources.size());
	
	for (unsigned ivs=0; ivs<VtxSources.size(); ivs++)
	{
		Model3D_VertexSource& VS = VtxSources[ivs];
		GLfloat Position[3];
		
		if (VS.Bone0 >= 0)
		{
			Model3D_Transform& T0 = BoneMatrices[VS.Bone0];
			TransformPoint(Position,VS.Position,T0);

			if (NormalsPresent)
			{
				for (int iv=InvVSIPointers[ivs]; iv<InvVSIPointers[ivs+1]; iv++)
				{
					int Indx = 3*InverseVSIndices[iv];
					TransformVector(NormBase() + Indx, NormSrcBase() + Indx, T0);
				}
			}			
			
			GLfloat Blend = VS.Blend;
			if (VS.Bone1 >= 0 && Blend != 0)
			{
				Model3D_Transform& T1 = BoneMatrices[VS.Bone1];
				GLfloat PosExtra[3];
				GLfloat PosDiff[3];
				TransformPoint(PosExtra,VS.Position,T1);
				VecSub(PosExtra,Position,PosDiff);
				VecScalarMultTo(PosDiff,Blend);
				VecAddTo(Position,PosDiff);
				
				if (NormalsPresent)
				{
					for (int iv=InvVSIPointers[ivs]; iv<InvVSIPointers[ivs+1]; iv++)
					{
						int Indx = 3*InverseVSIndices[iv];
						GLfloat NormExtra[3];
						GLfloat NormDiff[3];
						GLfloat *OrigNorm = NormSrcBase() + Indx;
						GLfloat *Norm = NormBase() + Indx;
						TransformVector(NormExtra,OrigNorm,T1);
						VecSub(NormExtra,Norm,NormDiff);
						VecScalarMultTo(NormDiff,Blend);
						VecAddTo(Norm,NormDiff);
					}
				}			
			}
		}
		else	// The assumed root bone (identity transformation)
		{
			VecCopy(VS.Position,Position);
			if (NormalsPresent)
			{
				for (int iv=InvVSIPointers[ivs]; iv<InvVSIPointers[ivs+1]; iv++)
				{
					int Indx = 3*InverseVSIndices[iv];
					VecCopy(NormSrcBase() + Indx, NormBase() + Indx);
				}
			}
		}
		
		// Copy found position into vertex array!
		for (int iv=InvVSIPointers[ivs]; iv<InvVSIPointers[ivs+1]; iv++)
			VecCopy(Position,PosBase() + 3*InverseVSIndices[iv]);
	}
	
	if (UseModelTransform)
	{
		GLfloat *PP = PosBase();
		for (size_t k=0; k<Positions.size()/3; k++, PP+=3)
		{
			GLfloat Position[3];
			TransformPoint(Position,PP,TransformPos);
			VecCopy(Position,PP);
		}
		GLfloat *NP = NormBase();
		for (size_t k=0; k<Normals.size()/3; k++, NP+=3)
		{
			GLfloat Normal[3];
			TransformVector(Normal,NP,TransformNorm);
			VecCopy(Normal,NP);
		}
	}
	
	return true;
}

// Returns 0 for out-of-range sequence
GLshort Model3D::NumSeqFrames(GLshort SeqIndex)
{
	if (SeqFrmPointers.empty()) return 0;
	if ((SeqIndex < 0) || (SeqIndex >= GLshort(SeqFrmPointers.size()))) return 0;
	
	return (SeqFrmPointers[SeqIndex+1] - SeqFrmPointers[SeqIndex]);
}

bool Model3D::FindPositions_Sequence(bool UseModelTransform, GLshort SeqIndex,
	GLshort FrameIndex, GLfloat MixFrac, GLshort AddlFrameIndex)
{
	// Bad inputs: do nothing and return false
	
	GLshort NumSF = NumSeqFrames(SeqIndex);
	if (NumSF <= 0) return false;
	
	if (FrameIndex < 0 || FrameIndex >= NumSF) return false;
	
	Model3D_Transform TSF;
	
	Model3D_SeqFrame& SF = SeqFrames[SeqFrmPointers[SeqIndex] + FrameIndex];
	
	if (MixFrac != 0 && AddlFrameIndex != FrameIndex)
	{
		if (AddlFrameIndex < 0 || AddlFrameIndex >= NumSF) return false;
		
		Model3D_SeqFrame& ASF = SeqFrames[SeqFrmPointers[SeqIndex] + AddlFrameIndex];
		FindFrameTransform(TSF,SF,MixFrac,ASF);
		
		if (!FindPositions_Frame(false,SF.Frame,MixFrac,ASF.Frame)) return false;
	}
	else
	{
		if (!FindPositions_Frame(false,SF.Frame)) return false;
		FindFrameTransform(TSF,SF,0,SF);
	}
	
	Model3D_Transform TTot;
	if (UseModelTransform)
		TMatMultiply(TTot,TransformPos,TSF);
	else
		obj_copy(TTot,TSF);
	
	size_t NumVerts = Positions.size()/3;
	GLfloat *Pos = PosBase();
	for (size_t iv=0; iv<NumVerts; iv++)
	{
		GLfloat NewPos[3];
		TransformPoint(NewPos,Pos,TTot);
		VecCopy(NewPos,Pos);
		Pos += 3;
	}
	
	bool NormalsPresent = !NormSources.empty();
	if (NormalsPresent)
	{	
		// OK, since the bones don't change bulk
		if (UseModelTransform)
			TMatMultiply(TTot,TransformNorm,TSF);
		else
			obj_copy(TTot,TSF);
				
		GLfloat *Norm = NormBase();
		for (size_t iv=0; iv<NumVerts; iv++)
		{
			GLfloat NewNorm[3];
			TransformVector(NewNorm,Norm,TTot);
			VecCopy(NewNorm,Norm);
			Norm += 3;
		}
	}
	
	return true;
}


void Model3D_Transform::Identity()
{
	obj_clear(*this);
	M[0][0] = M[1][1] = M[2][2] = 1;
}


static int16 InterpolateAngle(int16 Angle, GLfloat MixFrac, int16 AddlAngle)
{
	if (MixFrac != 0 && AddlAngle != Angle)
	{
		int16 AngleDiff = NORMALIZE_ANGLE(AddlAngle - Angle);
		if (AngleDiff >= HALF_CIRCLE) AngleDiff -= FULL_CIRCLE;
		Angle += int16(MixFrac*AngleDiff);
	}
	return NORMALIZE_ANGLE(Angle);
}


// Bone and Frame (positions, angles) -> Transform Matrix
static void FindFrameTransform(Model3D_Transform& T,
	Model3D_Frame& Frame, GLfloat MixFrac, Model3D_Frame& AddlFrame)
{
	T.Identity();
	
	// First, do rotations:
	short Angle;

	// Right-to-left; in the right order for Dim3 (and Tomb Raider)
	
	// Z:
	Angle = InterpolateAngle(Frame.Angles[2],MixFrac,AddlFrame.Angles[2]);

	if (Angle != 0)
	{
		GLfloat C = TrigNorm*cosine_table[Angle];
		GLfloat S = TrigNorm*sine_table[Angle];
		
		for (int ic=0; ic<3; ic++)
		{
			GLfloat X = T.M[0][ic];
			GLfloat Y = T.M[1][ic];
			GLfloat XR = X*C - Y*S;
			GLfloat YR = X*S + Y*C;
			T.M[0][ic] = XR;
			T.M[1][ic] = YR;
		}
	}
	
	// X:
	Angle = InterpolateAngle(Frame.Angles[0],MixFrac,AddlFrame.Angles[0]);
	
	if (Angle != 0)
	{
		GLfloat C = TrigNorm*cosine_table[Angle];
		GLfloat S = TrigNorm*sine_table[Angle];
		
		for (int ic=0; ic<3; ic++)
		{
			GLfloat X = T.M[1][ic];
			GLfloat Y = T.M[2][ic];
			GLfloat XR = X*C - Y*S;
			GLfloat YR = X*S + Y*C;
			T.M[1][ic] = XR;
			T.M[2][ic] = YR;
		}
	}
	
	// Y:
	Angle = InterpolateAngle(Frame.Angles[1],MixFrac,AddlFrame.Angles[1]);
	
	if (Angle != 0)
	{
		GLfloat C = TrigNorm*cosine_table[Angle];
		GLfloat S = TrigNorm*sine_table[Angle];
		
		for (int ic=0; ic<3; ic++)
		{
			GLfloat X = T.M[2][ic];
			GLfloat Y = T.M[0][ic];
			GLfloat XR = X*C - Y*S;
			GLfloat YR = X*S + Y*C;
			T.M[2][ic] = XR;
			T.M[0][ic] = YR;
		}
	}
	
	// Set up overall translate:
	GLfloat *FrameOfst = Frame.Offset;
	if (MixFrac != 0)
	{
		GLfloat *AddlFrameOfst = AddlFrame.Offset;
		for (int ic=0; ic<3; ic++)
			T.M[ic][3] = FrameOfst[ic] + MixFrac*(AddlFrameOfst[ic]-FrameOfst[ic]);
	}
	else
	{
	for (int ic=0; ic<3; ic++)
		T.M[ic][3] = FrameOfst[ic];
	}
}

static void FindBoneTransform(Model3D_Transform& T, Model3D_Bone& Bone,
	Model3D_Frame& Frame, GLfloat MixFrac, Model3D_Frame& AddlFrame)
{
	FindFrameTransform(T,Frame,MixFrac,AddlFrame);
	
	// Set up overall translate:
	GLfloat *BonePos = Bone.Position;
	for (int ic=0; ic<3; ic++)
		T.M[ic][3] += BonePos[ic] - ScalarProd(T.M[ic],BonePos);
}


// Res = A * B, in that order
static void TMatMultiply(Model3D_Transform& Res, Model3D_Transform& A, Model3D_Transform& B)
{
	// Multiply the rotation parts
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
		{
			GLfloat Sum = 0;
			for (int k=0; k<3; k++)
				Sum += A.M[i][k]*B.M[k][j];
			
			Res.M[i][j] = Sum;
		}
	
	// Now the translation part
	for (int i=0; i<3; i++)
	{
		GLfloat Sum = 0;
		for (int k=0; k<3; k++)
			Sum += A.M[i][k]*B.M[k][3];
		
		Res.M[i][3] = A.M[i][3] + Sum;
	}
}


#endif // def HAVE_OPENGL
