/*
	Alias|Wavefront Object Loader
	
	By Loren Petrich, June 16, 2001
*/
#ifndef WAVEFRONT_LOADER
#define WAVEFRONT_LOADER

#include <stdio.h>
#include "Model3D.h"
#include "FileHandler.h"

bool LoadModel_Wavefront(FileSpecifier& Spec, Model3D& Model);

// Where to emit status messages
void SetDebugOutput_Wavefront(FILE *DebugOutput);

#endif
