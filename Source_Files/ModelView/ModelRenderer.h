#ifndef MODEL_RENDERER
#define MODEL_RENDERER
/*
	Renders 3D-model objects;
	it can render either with or without a Z-buffer;
	without a Z-buffer, it depth-sorts polygons

	Created by Loren Petrich, July 18, 2001
*/

#include "Model3D.h"

struct ModelRenderShader
{
	unsigned int Flags;
	void (* TextureCallback)(void *);
	void *TextureCallbackData;
	
	ModelRenderShader() {obj_clear(*this);}
};

class ModelRenderer
{
	// Kept here to avoid unnecessary re-allocation
	vector<GLfloat> CentroidDepths;
	vector<unsigned short> Indices;
	vector<GLushort> SortedVertIndices;
	
	void SetupRenderPass(Model3D& Model, ModelRenderShader& Shader);
	
public:
	
	// Needed for depth-sorting the model triangles by centroid
	GLfloat ViewDirection[3];
	
	// Render flags:
	enum {
		Textured = 0x0001,
		Colored = 0x0002
	};
	
	// Does the actual rendering; args:
	// A 3D model (of course!)
	// Whether or not to assume a Z-buffer is present
	// Array of shaders to be used for multipass rendering
	// How many shaders in that array to use
	void Render(Model3D& Model, bool Use_Z_Buffer, ModelRenderShader *Shaders, int NumShaders);
	
	// In case one wants to start over again with these persistent arrays
	void Clear() {CentroidDepths.clear(); Indices.clear(); SortedVertIndices.clear();}

	// Compare function for sorting;
	// this is for back-to-front sorting, thus the >
	operator()(int i1, int i2)
	{
		return (CentroidDepths[i1] > CentroidDepths[i2]);
	}
};


#endif
