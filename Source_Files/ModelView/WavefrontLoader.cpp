/*
	Alias|Wavefront Object Loader
	
	By Loren Petrich, June 16, 2001
*/

#include <ctype.h>
#include <string.h>
#include "WavefrontLoader.h"

static FILE *DBOut = NULL;

void SetDebugOutput_Wavefront(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}


bool LoadModel_Wavefront(FileSpecifier& Spec, Model3D& Model)
{
	// Read buffer
	const int BufferSize = 256;
	char Buffer[BufferSize];

	// Initially, nothing in the model
	Model.Clear();
	
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
	
	// This loop is for one line at a time
	/*
	while(true)
	{
	
	
	}
	*/
}
