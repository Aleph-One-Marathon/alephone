/*
	3D Studio Max Object Loader
	
	By Loren Petrich, Sept 1, 2001
*/
#ifndef STUDIO_LOADER
#define STUDIO_LOADER

#include <stdio.h>
#include "Model3D.h"
#include "FileHandler.h"

bool LoadModel_Studio(FileSpecifier& Spec, Model3D& Model);

// Where to emit status messages
void SetDebugOutput_Studio(FILE *DebugOutput);

#endif
