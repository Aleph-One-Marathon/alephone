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
	
	// This callback takes only one argument, whatever data is appropriate
	// (the pointer after it)
	void (* TextureCallback)(void *);
	void *TextureCallbackData;
	
	// This callback takes not only whatever data is appropriate
	// (the pointer after it),
	// but also the number of vertices, the normals, the vertex positions,
	// and the vertex-lighting color values, in that order.
	// The normals and the positions are in model coordinates.
	void (* LightingCallback)(void *, int, GLfloat *, GLfloat *, GLfloat *);
	void *LightingCallbackData;
	
	ModelRenderShader() {obj_clear(*this);}
};

class ModelRenderer
{
	// Kept here to avoid unnecessary re-allocation
	vector<GLfloat> CentroidDepths;
	vector<unsigned short> Indices;
	vector<GLushort> SortedVertIndices;
	vector<GLfloat> ExtLightColors;
	
	void SetupRenderPass(Model3D& Model, ModelRenderShader& Shader);
	
public:
	
	// Needed for depth-sorting the model triangles by centroid;
	// it is in model coordinates.
	GLfloat ViewDirection[3];
	
	// External lighting now done with a shader callback
		
	// Render flags:
	enum {
		Textured	= 0x0001,
		Colored		= 0x0002,
		ExtLight	= 0x0004,
	};
	
	// Does the actual rendering; args:
	// A 3D model (of course!)
	// Whether or not to assume a Z-buffer is present
	// Array of shaders to be used for multipass rendering
	// How many shaders in that array to use
	void Render(Model3D& Model, bool Use_Z_Buffer, ModelRenderShader *Shaders, int NumShaders);
	
	// In case one wants to start over again with these persistent arrays
	void Clear();
};


#endif
