/*
	3D-Model Object Storage
	It is intended to be as OpenGL-friendly as is reasonably possible
	
	By Loren Petrich, June 16, 2001
*/
#ifndef MODEL_3D
#define MODEL_3D

using namespace std;

#include <GL/gl.h>
#include <vector>


struct Model3D
{
	// Assumed dimensions:
	// Positions: 3
	// Texture coordinates: 2
	// Normals: 3
	
	// Positions assumed to be 3-dimensional
	vector<GLfloat> Positions;
	GLfloat *PosBase() {return &Positions[0];}
	
	// Texture coordinates assumed to be 2-dimensional
	// Their indices parallel the vertex indices
	vector<GLfloat> TxtrCoords;
	GLfloat *TCBase() {return &TxtrCoords[0];}
	
	// Normals assumed to be 3-dimensional
	vector<GLfloat> Normals;
	GLfloat *NormBase() {return &Normals[0];}
	
	// Vertex colors (useful for by-hand vertex lighting)
	vector<GLfloat> Colors;
	GLfloat *ColBase() {return &Colors[0];}
	
	// List of indices into the aforementioned values;
	// the list is a list of triangles.
	vector<GLushort> VertIndices;
	GLushort *VIBase() {return &VertIndices[0];}
	int NumVI() {return VertIndices.size();}
	
	// Bounding box (0 = min, 1 = max)
	GLfloat BoundingBox[2][3];
	
	// From the position data
	void FindBoundingBox();
	
	// For debugging bounding-box-handling code
	// NULL means don't render one set of edges
	void RenderBoundingBox(const GLfloat *EdgeColor, const GLfloat *DiagonalColor);
	
	// Erase everything
	void Clear();
	
	// Constructor
	Model3D() {FindBoundingBox();}
};



#endif
