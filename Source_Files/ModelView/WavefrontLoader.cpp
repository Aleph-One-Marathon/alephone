/*
	Alias|Wavefront Object Loader
	
	By Loren Petrich, June 16, 2001
*/

#include <ctype.h>
#include <string.h>
#include "cseries.h"
#include "WavefrontLoader.h"


// Debug-message destination
static FILE *DBOut = NULL;

// Input line will be able to stretch as much as necessary
static vector<char> InputLine(64);

// Compare input-line beginning to a keyword;
// returns pointer to rest of line if it was found,
// otherwise returns NULL
char *CompareToKeyword(char *Keyword);


void SetDebugOutput_Wavefront(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}


bool LoadModel_Wavefront(FileSpecifier& Spec, Model3D& Model)
{
	// Read buffer
	const int BufferSize = 256;
	char Buffer[BufferSize];
	
	// Intermediate lists of vertices, texture coordinates, and normals
	vector<GLfloat> Vertices;
	vector<GLfloat> TxtrCoords;
	vector<GLfloat> Normals;
	
	if (DBOut)
	{
		Spec.GetName(Buffer);
		fprintf(DBOut,"Loading Alias|Wavefront model file %s\n",Buffer);
	}
	
	OpenedFile OFile;
	if (!Spec.Open(OFile))
	{	
		if (DBOut) fprintf(DBOut,"Error opening the file\n");
		return false;
	}

	// Reading loop; create temporary lists of vertices, texture coordinates, and normals
	char c;
	
	// Load the lines, one by one, and then parse them. Be sure to take care of the continuation
	// character "\" [Wavefront files follow some Unix conventions]
	bool MoreLines = true;
	while(MoreLines)
	{
		InputLine.clear();
		
		// Fill up the line
		bool LineContinued = false;
		while(true)
		{
			// Try to read a character; if it is not possible to read anymore,
			// the line has ended
			char c;
			MoreLines = OFile.Read(1,&c);
			if (!MoreLines) break;
			
			// End-of-line characters; ignore if the line is to be continued
			if (c == '\r' || c == '\n')
			{
				if (!LineContinued)
				{
					// If the line is not empty, then break; otherwise ignore.
					// Blank lines will be ignored, and this will allow starting a line
					// at the first non-end-of-line character
					if (!InputLine.empty()) break;		
				}
			}
			// Backslash character indicates that the current line continues into the next one
			else if (c == '\\')
			{
				LineContinued = true;
			}
			else
			{
				// Continuation will stop if a non-end-of-line character is encounted
				LineContinued = false;
				
				// Add that character!
				InputLine.push_back(c);
			}
		}
		// Line-end at end of file will produce an empty line, so do this test
		if (InputLine.empty()) continue;
		
		// If the line is a comment line, then ignore it
		if (InputLine[0] == '#') continue;
		
		// Make the line look like a C string
		InputLine.push_back('\0');
		
		// Now parse the line; notice the = instead of == (substitute and test in one line)
		char *RestOfLine = NULL;
		if (RestOfLine = CompareToKeyword("v")) // Vertex position
		{
			GLfloat Vertex[Model3D::VertexDim];
			objlist_clear(Vertex,Model3D::VertexDim);
			
			sscanf(RestOfLine," %f %f %f",Vertex,Vertex+1,Vertex+2);
			
			for (int k=0; k<Model3D::VertexDim; k++)
				Vertices.push_back(Vertex[k]);
		}
		else if (RestOfLine = CompareToKeyword("vt")) // Vertex texture coordinate
		{
			GLfloat TxtrCoord[Model3D::TxtrCoordDim];
			objlist_clear(TxtrCoord,Model3D::TxtrCoordDim);
			
			sscanf(RestOfLine," %f %f",TxtrCoord,TxtrCoord+1);
			
			for (int k=0; k<Model3D::TxtrCoordDim; k++)
				TxtrCoords.push_back(TxtrCoord[k]);
		}
		else if (RestOfLine = CompareToKeyword("vn")) // Vertex normal
		{
			GLfloat Normal[Model3D::NormalDim];
			objlist_clear(Normal,Model3D::NormalDim);
			
			sscanf(RestOfLine," %f %f %f",Normal,Normal+1,Normal+2);
			
			for (int k=0; k<Model3D::NormalDim; k++)
				Normals.push_back(Normal[k]);
		}
		else if (RestOfLine = CompareToKeyword("vp")) // Vertex parameter value
		{
			// For curved objects, which are not supported here
		}
		else if (RestOfLine = CompareToKeyword("deg")) // Degree
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("bmat")) // Basis matrix
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("step")) // Step size
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("cstype")) // Curve/surface type
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("p")) // Point
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("l")) // Line
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("f")) // Face (polygon)
		{
			// This is very complicated...
		}
		else if (RestOfLine = CompareToKeyword("curv")) // Curve
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("curv2")) // 2D Curve
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("surf")) // Surface
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("parm")) // Parameter values
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("trim")) // Outer trimming loop
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("hole")) // Inner trimming loop
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("scrv")) // Special curve
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("sp")) // Special point
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("end")) // End statement
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("con")) // Connect
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("g")) // Group name
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("s")) // Smoothing group
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("mg")) // Merging group
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("o")) // Object name
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("bevel")) // Bevel interpolation
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("c_interp")) // Color interpolation
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("d_interp")) // Dissolve interpolation
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("lod")) // Level of detail
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("usemtl")) // Material name
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("mtllib")) // Material library
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("shadow_obj")) // Shadow casting
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("trace_obje")) // Ray tracing
		{
			// Not supported here
		}
		else if (RestOfLine = CompareToKeyword("ctech")) // Curve approximation technique
		{
			// Curved objects not supported here
		}
		else if (RestOfLine = CompareToKeyword("stech")) // Surface approximation technique
		{
			// Curved objects not supported here
		}
		else
		{
			if (DBOut) fprintf(DBOut,"Line has bad keyword: %s\n",&InputLine[0]);
		}
	}


	// Create the final model object
	Model.Clear();
	
	if (DBOut) fprintf(DBOut,"Successfully read the file\n");
	return true;
}


char *CompareToKeyword(char *Keyword)
{
	int KWLen = strlen(Keyword);
	
	if (InputLine.size() < KWLen) return NULL;
	
	for (int k=0; k<KWLen; k++)
		if (InputLine[k] != Keyword[k]) return NULL;
	
	return &InputLine[KWLen];
}
