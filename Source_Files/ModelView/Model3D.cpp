/*
	3D-Model Object Storage Functions
	
	By Loren Petrich, July 8, 2001
*/

#include <string.h>
#include <math.h>
#include "cseries.h"

#ifdef HAVE_OPENGL

#include <GL/gl.h>
#include "Model3D.h"


// Erase everything
void Model3D::Clear()
{
	Positions.clear();
	TxtrCoords.clear();
	Normals.clear();
	Colors.clear();
	VertIndices.clear();
	FindBoundingBox();
}

// Normalize an individual normal; return whether the normal had a nonzero length
static bool NormalizeNormal(GLfloat *Normal)
{
	GLfloat NormalSqr =
		Normal[0]*Normal[0] + Normal[1]*Normal[1] + Normal[2]*Normal[2];
	
	if (NormalSqr <= 0) return false;
	
	GLfloat NormalRecip = 1/sqrt(NormalSqr);
	
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

// Normalize the normals
void Model3D::AdjustNormals(int NormalType, float SmoothThreshold)
{
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
		for (int k=0; k<Normals.size()/3; k++)
			NormalizeNormal(&Normals[3*k]);
		break;
	
	case ClockwiseSide:
	case CounterclockwiseSide:
		// The really interesting stuff
		{
			// First, create a list of per-polygon normals
			int NumPolys = NumVI()/3;
			vector<FlaggedVector> PerPolygonNormalList(NumPolys);
			
			GLushort *IndxPtr = VIBase();
			for (int k=0; k<NumPolys; k++)
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
			int NumVerts = Positions.size()/3;
			vector<FlaggedVector> PerVertexNormalList(NumVerts);
			objlist_clear(&PerVertexNormalList[0],NumVerts);
			IndxPtr = VIBase();
			for (int k=0; k<NumPolys; k++)
			{
				FlaggedVector& PPN = PerPolygonNormalList[k];
				for (int c=0; c<3; c++)
				{
					GLushort VertIndx = *(IndxPtr++);
					GLfloat *V = PerVertexNormalList[VertIndx].Vec;
					*(V++) += PPN.Vec[0];
					*(V++) += PPN.Vec[1];
					*(V++) += PPN.Vec[2];
				}
			}
			
			// Normalize the per-vertex normals
			for (int k=0; k<NumVerts; k++)
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
			for (int k=0; k<NumPolys; k++)
			{
				FlaggedVector& PPN = PerPolygonNormalList[k];
				for (int c=0; c<3; c++)
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
			for (int k=0; k<NumVerts; k++)
			{
				// Vertices without contributions will automatically have
				// their flags be false, as a result of NormalizeNormal()
				short NumVertPolys = NumPolysPerVert[k];
				if (NumVertPolys > 0 && PerVertexNormalList[k].Flag)
					PerVertexNormalList[k].Flag =
						sqrt(Variances[k]/NumVertPolys) <= SmoothThreshold;
			}
			
			// The vertex flags are now set for whether to use that vertex's normal;
			// re-count the number of polygons per vertex.
			// Use NONE for unsplit ones
			objlist_clear(&NumPolysPerVert[0],NumVerts);
			IndxPtr = VIBase();
			for (int k=0; k<NumPolys; k++)
			{
				for (int c=0; c<3; c++)
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
			for (int k=0; k<NumVerts; k++)
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
			for (int k=0; k<NumPolys; k++)
			{
				for (int c=0; c<3; c++)
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
			
			bool TCPresent = !TxtrCoords.empty();
			if (TCPresent) NewTxtrCoords.resize(2*NewNumVerts);
			
			bool ColPresent = !Colors.empty();
			if (ColPresent) NewColors.resize(3*NewNumVerts);
			
			// Use marching pointers to speed up the copy-over
			GLfloat *OldP = &Positions[0];
			GLfloat *NewP = &NewPositions[0];
			GLfloat *OldT = &TxtrCoords[0];
			GLfloat *NewT = &NewTxtrCoords[0];
			GLfloat *OldC = &Colors[0];
			GLfloat *NewC = &NewColors[0];
			GLfloat *NewN = &NewNormals[0];
			for (int k=0; k<NumVerts; k++)
			{
				FlaggedVector& PVN = PerVertexNormalList[k];
				int NumVertPolys = PVN.Flag ? 1 : NumPolysPerVert[k];
				for (int c=0; c<NumVertPolys; c++)
				{
					GLfloat *OldPP = OldP;
					*(NewP++) = *(OldPP++);
					*(NewP++) = *(OldPP++);
					*(NewP++) = *(OldPP++);
				}
				if (TCPresent)
				{
					for (int c=0; c<NumVertPolys; c++)
					{
						GLfloat *OldTP = OldT;
						*(NewT++) = *(OldTP++);
						*(NewT++) = *(OldTP++);
					}
				}
				if (ColPresent)
				{
					for (int c=0; c<NumVertPolys; c++)
					{
						GLfloat *OldCP = OldC;
						*(NewC++) = *(OldCP++);
						*(NewC++) = *(OldCP++);
						*(NewC++) = *(OldCP++);
					}
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
					for (int c=0; c<NumVertPolys; c++)
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
			assert(NewN == &NewNormals[3*NewNumVerts]);
			
			// Accept the new vectors
			Positions.swap(NewPositions);
			TxtrCoords.swap(NewTxtrCoords);
			Normals.swap(NewNormals);
			Colors.swap(NewColors);
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
			for (int k=0; k<Normals.size(); k++)
				*(NormalPtr++) *= -1;
		}
	}
}

	
// From the position data
void Model3D::FindBoundingBox()
{
	int NumVertices = Positions.size()/3;
	if (NumVertices > 0)
	{
		// Find the min and max of the positions:
		objlist_copy(BoundingBox[0],&Positions[0],3);
		objlist_copy(BoundingBox[1],&Positions[0],3);
		for (int i=1; i<NumVertices; i++)
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
		glColor3fv(EdgeColor);
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
		glColor3fv(DiagonalColor);
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

#endif // def HAVE_OPENGL
