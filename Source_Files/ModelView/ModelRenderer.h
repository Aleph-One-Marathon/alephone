#ifndef MODEL_RENDERER
#define MODEL_RENDERER
/*
	Renders 3D-model objects;
	it can render either with or without a Z-buffer;
	without a Z-buffer, it depth-sorts polygons

	Created by Loren Petrich, July 18, 2001
*/

#include "Model3D.h"

class ModelRenderer
{
	// Kept here to avoid unnecessary re-allocation
	vector<GLfloat> CentroidDepths;
	vector<unsigned short> Indices;
	vector<GLushort> SortedVertIndices;
	
public:
	
	// Needed for depth-sorting the model triangles by centroid
	GLfloat ViewDirection[3];
	
	// Render flags:
	enum {
		Z_Buffered = 0x0001,
		Textured = 0x0002,
		Colored = 0x0004
	};
	
	// The Z-buffering flag can be switched off for semitransparent textures,
	// since rendering of these is not commutative, as is the case for all-or-nothing
	// transparency (Z-buffering is useful there).
	// The rendering flags are for doing colors and texture coordinates;
	void Render(Model3D& Model, unsigned int flags);
	
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
