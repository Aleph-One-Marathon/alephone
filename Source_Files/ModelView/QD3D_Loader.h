/*
	QuickDraw 3D / Quesa Object Loader
	
	By Loren Petrich, September 1, 2001
*/
#ifndef QD3D_LOADER
#define QD3D_LOADER

#include <stdio.h>
#include "Model3D.h"
#include "FileHandler.h"

bool LoadModel_QD3D(FileSpecifier& Spec, Model3D& Model);

// Where to emit status messages
void SetDebugOutput_QD3D(FILE *DebugOutput);

#endif
