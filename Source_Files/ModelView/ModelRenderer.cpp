/*
	Renders 3D-model objects;

	Created by Loren Petrich, July 18, 2001
*/

#include <algorithm>
#include <functional>
#include "cseries.h"
#include "ModelRenderer.h"


void ModelRenderer::Render(Model3D& Model, unsigned int Flags)
{
	if (Model.Positions.empty()) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,Model.PosBase());
	
	if (!Model.TxtrCoords.empty() && TEST_FLAG(Flags,Textured))
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,Model.TCBase());
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	
	if (!Model.Colors.empty() && TEST_FLAG(Flags,Colored))
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3,GL_FLOAT,0,Model.ColBase());
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);
	
	// No-brainer!!!
	if (TEST_FLAG(Flags,Z_Buffered))
	{
		glDrawElements(GL_TRIANGLES,Model.NumVI(),GL_UNSIGNED_SHORT,Model.VIBase());	
		return;
	}

	// Have to do centroid depth sorting here
	
	// Find the centroids:
	int NumTriangles = Model.NumVI()/3;
	CentroidDepths.resize(NumTriangles);
	Indices.resize(NumTriangles);
	
	GLushort *VIPtr = Model.VIBase();
	for (int k=0; k<NumTriangles; k++)
	{
		GLfloat Sum = 0;
		for (int v=0; v<3; v++)
		{
			GLfloat *Pos = &Model.Positions[3*(*VIPtr)];
			for (int w=0; w<3; w++)
				Sum += Pos[w]*ViewDirection[w];
			VIPtr++;
		}
		Indices[k] = k;
		CentroidDepths[k] = Sum;
	}
	
	// Sort!
	sort(Indices.begin(),Indices.end(),*this);
	
	// This step may not be appropriate for multipass rendering
	SortedVertIndices.resize(Model.NumVI());
	for (int k=0; k<NumTriangles; k++)
	{
		GLushort *SourceTriangle = &Model.VertIndices[3*Indices[k]];
		GLushort *DestTriangle = &SortedVertIndices[3*k];
		memcpy(DestTriangle,SourceTriangle,3*sizeof(GLushort));
	}
	
	// Go!
	glDrawElements(GL_TRIANGLES,Model.NumVI(),GL_UNSIGNED_SHORT,&SortedVertIndices[0]);
	return;
}
