/*
	3D-Model Object Storage Functions
	
	By Loren Petrich, July 8, 2001
*/

#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include "cseries.h"
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


// Normalize the normals
void Model3D::NormalizeNormals()
{
	int NumNormals = Normals.size()/3;
	for (int k=0; k<NumNormals; k++)
	{
		GLfloat *NormalPtr = &Normals[3*k];
		GLfloat NormalSqr =
			NormalPtr[0]*NormalPtr[0] + NormalPtr[1]*NormalPtr[1] + NormalPtr[2]*NormalPtr[2];
		if (NormalSqr > 0)
		{
			GLfloat NormalRecip = 1/sqrt(NormalSqr);
			NormalPtr[0] *= NormalRecip;
			NormalPtr[1] *= NormalRecip;
			NormalPtr[2] *= NormalRecip;
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
