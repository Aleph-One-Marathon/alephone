/*
	3D-Model Object Storage Functions
	
	By Loren Petrich, July 8, 2001
*/

#include <string.h>
#include "cseries.h"
#include "Model3D.h"


// Erase everything
void Model3D::Clear()
{
	Positions.clear();
	TxtrCoords.clear();
	Normals.clear();
	VertIndices.clear();
	SetBoundingBox();
}

	
// From the position data
void Model3D::SetBoundingBox()
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
