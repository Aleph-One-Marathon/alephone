/*
	Renders 3D-model objects;

	Created by Loren Petrich, July 18, 2001
*/

#include <algorithm>
#include <functional>
#include "cseries.h"
#include "ModelRenderer.h"


void ModelRenderer::Render(Model3D& Model, bool Use_Z_Buffer, ModelRenderShader *Shaders, int NumShaders)
{
	if (NumShaders <= 0) return;
	if (!Shaders) return;
	if (Model.Positions.empty()) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,Model.PosBase());
	
	if (Use_Z_Buffer)
	{
		for (int q=0; q<NumShaders; q++)
		{
			SetupRenderPass(Model,Shaders[q]);
			glDrawElements(GL_TRIANGLES,Model.NumVI(),GL_UNSIGNED_SHORT,Model.VIBase());
		}
		return;			
	}
	// Fall-through: don't use Z-buffer

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
	
	if (NumShaders > 1)
	{
		// Multishader case: each triangle separately
		for (int k=0; k<NumTriangles; k++)
		{
			GLushort *Triangle = &Model.VertIndices[3*Indices[k]];
			for (int q=0; q<NumShaders; q++)
			{
				SetupRenderPass(Model,Shaders[0]);
				glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_SHORT,Triangle);
			}
		}
	}
	else
	{
		// Single-shader optimization: render in one swell foop
		SetupRenderPass(Model,Shaders[0]);
		SortedVertIndices.resize(Model.NumVI());
		for (int k=0; k<NumTriangles; k++)
		{
			GLushort *SourceTriangle = &Model.VertIndices[3*Indices[k]];
			GLushort *DestTriangle = &SortedVertIndices[3*k];
			objlist_copy(DestTriangle,SourceTriangle,3);
		}
		
		// Go!
		glDrawElements(GL_TRIANGLES,Model.NumVI(),GL_UNSIGNED_SHORT,&SortedVertIndices[0]);
	}
	return;
}


void ModelRenderer::SetupRenderPass(Model3D& Model, ModelRenderShader& Shader)
{
	assert(Shader.TextureCallback);
	
	// Test textured rendering
	if (!Model.TxtrCoords.empty() && TEST_FLAG(Shader.Flags,Textured))
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
	
	// Test colored rendering
	if (!Model.Colors.empty() && TEST_FLAG(Shader.Flags,Colored))
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3,GL_FLOAT,0,Model.ColBase());
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);
	
	// Do whatever texture management is necessary
	Shader.TextureCallback(Shader.TextureCallbackData);
}
