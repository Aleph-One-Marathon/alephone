/*
	3D-Model Object Storage
	It is intended to be as OpenGL-friendly as is reasonably possible
	
	By Loren Petrich, June 16, 2001
*/
#ifndef MODEL_3D
#define MODEL_3D

using namespace std;

#include <vector>
#include <GL/gl.h>


struct Model3D
{
	// Assumed dimensions:
	enum {
		VertexDim = 3,
		TxtrCoordDim = 2,
		NormalDim = 3
	};
	
	// Vertex positions assumed to be 3-dimensional
	vector<GLfloat> Vertices;
	GLfloat *VertBase() {return &Vertices[0];}
	
	// Texture coordinates assumed to be 2-dimensional
	// Their indices parallel the vertex indices
	vector<GLfloat> TxtrCoords;
	GLfloat *TCBase() {return &TxtrCoords[0];}
	
	// Normals assumed to be 3-dimensional
	vector<GLfloat> Normals;
	GLfloat *NormBase() {return &Normals[0];}
	
	// List of indices into the aforementioned values;
	// the list is a list of triangles.
	vector<GLushort> VertIndices;
	GLushort *VIBase() {return &VertIndices[0];}
	int NumVI() {return VertIndices.size();}
	
	// Erase everything
	void Clear() {Vertices.clear(); TxtrCoords.clear(); Normals.clear(); VertIndices.clear();}
};



#endif
