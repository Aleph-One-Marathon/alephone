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

	Renders 3D-model objects;

	Created by Loren Petrich, July 18, 2001
*/

#include <string.h>
#include <stdlib.h>

#include "cseries.h"

#ifdef HAVE_OPENGL

#include "ModelRenderer.h"
#include <algorithm>

void ModelRenderer::Render(Model3D& Model, ModelRenderShader *Shaders, int NumShaders,
	int NumSeparableShaders, bool Use_Z_Buffer)
{
	if (NumShaders <= 0) return;
	if (!Shaders) return;
	if (Model.Positions.empty()) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,Model.PosBase());
	
	// Effective number of separable shaders when the Z-buffer is absent: none
	// (the Z-buffer enables separability).
	// Editing this arg is OK since it was called by value.
	if (!Use_Z_Buffer) NumSeparableShaders = 0;
	
	// Optimization: skip depth sorting, render, and quit
	// if all shaders are separable.
	if (NumSeparableShaders >= NumShaders)
	{
		for (int q=0; q<NumShaders; q++)
		{
			SetupRenderPass(Model,Shaders[q]);
			glDrawElements(GL_TRIANGLES,(GLsizei)Model.NumVI(),GL_UNSIGNED_SHORT,Model.VIBase());
		}
		return;			
	}
	
	// If some of the shaders are nonseparable, then the polygons have to be depth-sorted.
	// The separable ones will also use that depth-sorting, out of coding convenience.
	// OpenGL != PowerVR
	// (which can store polygons and depth-sort them in its hardware)
	
	// Find the centroids:
	size_t NumTriangles = Model.NumVI()/3;
	IndexedCentroidDepths.resize(NumTriangles);
	
	GLushort *VIPtr = Model.VIBase();
	for (unsigned short k=0; k<NumTriangles; k++)
	{
		GLfloat Sum[3] = {0, 0, 0};
		for (int v=0; v<3; v++)
		{
			GLfloat *Pos = &Model.Positions[3*(*VIPtr)];
			Sum[0] += Pos[0];
			Sum[1] += Pos[1];
			Sum[2] += Pos[2];
			VIPtr++;
		}
		IndexedCentroidDepths[k].index = k;
		IndexedCentroidDepths[k].depth = 
			Sum[0]*ViewDirection[0] + Sum[1]*ViewDirection[1] + Sum[2]*ViewDirection[2];
	}
	
	// Sort!
	std::sort(IndexedCentroidDepths.begin(), IndexedCentroidDepths.end());
	
	// Optimization: a single nonseparable shader can be rendered as if it was separable,
	// though it must still be depth-sorted.
	if (NumSeparableShaders == NumShaders - 1)
		NumSeparableShaders++;
	
	for (int q=0; q<NumSeparableShaders; q++)
	{
		// Need to do this only once
		if (q == 0)
		{
			SortedVertIndices.resize(Model.NumVI());
			GLushort *DestTriangle = &SortedVertIndices[0];
			for (size_t k=0; k<NumTriangles; k++)
			{
				GLushort *SourceTriangle = &Model.VertIndices[3*IndexedCentroidDepths[k].index];
				// Copy-over unrolled for speed
				*(DestTriangle++) = *(SourceTriangle++);
				*(DestTriangle++) = *(SourceTriangle++);
				*(DestTriangle++) = *(SourceTriangle++);
			}
		}
		
		// Separable-shader optimization: render in one swell foop
		SetupRenderPass(Model,Shaders[q]);
				
		// Go!
		glDrawElements(GL_TRIANGLES,(GLsizei)Model.NumVI(),GL_UNSIGNED_SHORT,&SortedVertIndices[0]);
	}
	
	if (NumSeparableShaders < NumShaders)
	{
		// Multishader case: each triangle separately
		for (size_t k=0; k<NumTriangles; k++)
		{
			GLushort *Triangle = &Model.VertIndices[3*IndexedCentroidDepths[k].index];
			for (int q=NumSeparableShaders; q<NumShaders; q++)
			{
				SetupRenderPass(Model,Shaders[q]);
				glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_SHORT,Triangle);
			}
		}
	}
}


/* TODO: sRGB-correct model colors. This needs to be done in the loader. The
   lighting colors are already sRGB-corrected. -SB */
void ModelRenderer::SetupRenderPass(Model3D& Model, ModelRenderShader& Shader)
{
	assert(Shader.TextureCallback);
	
	// Do textured rendering
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
	
	// Check whether to use external lighting
	if (Shader.LightingCallback && !Model.Normals.empty() && TEST_FLAG(Shader.Flags,ExtLight))
	{
		size_t NumVerts = Model.Positions.size()/3;
		size_t NumCPlanes = TEST_FLAG(Shader.Flags,EL_SemiTpt) ? 4 : 3;
		size_t NumCValues = NumCPlanes*NumVerts;
		ExtLightColors.resize(NumCValues);
		
		Shader.LightingCallback(Shader.LightingCallbackData,
			NumVerts, Model.NormBase(),Model.PosBase(),&ExtLightColors[0]);
		
		if (!Model.Colors.empty() && TEST_FLAG(Shader.Flags,Colored))
		{
			GLfloat *ExtColorPtr = &ExtLightColors[0];
			GLfloat *ColorPtr = Model.ColBase();
			if (NumCPlanes == 3)
			{
				for (size_t k=0; k<NumCValues; k++, ExtColorPtr++, ColorPtr++)
					(*ExtColorPtr) *= (*ColorPtr);
			}
			else if (NumCPlanes == 4)
			{
				for (size_t k=0; k<NumVerts; k++)
				{
					for (int chn=0; chn<3; chn++)
					{
						(*ExtColorPtr) *= (*ColorPtr);
						ExtColorPtr++, ColorPtr++;
					}
					// Nothing happens to the alpha channel
					ExtColorPtr++;
				}
			}
		}
		
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer((GLint)NumCPlanes,GL_FLOAT,0,&ExtLightColors[0]);
	}
	else
	{
		// Do colored rendering
		if (!Model.Colors.empty() && TEST_FLAG(Shader.Flags,Colored))
		{
			// May want to recover the currently-set color and do the same kind
			// of treatment as above
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_FLOAT,0,Model.ColBase());
		}
		else
			glDisableClientState(GL_COLOR_ARRAY);
	}
	
	// Do whatever texture management is necessary
	Shader.TextureCallback(Shader.TextureCallbackData);
}


void ModelRenderer::Clear()
{
	IndexedCentroidDepths.clear();
	SortedVertIndices.clear();
	ExtLightColors.clear();
}

#endif // def HAVE_OPENGL
