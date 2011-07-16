/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

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

// Set the QD3D tesselation factor:
// The first argument means
//    if true, tesselation length = length in world-geometry units
//    if false, tesselation length = constant subdivision factor
// The second argument is that length
void SetTesselation_QD3D(bool IsWorldLength, float TessLength);

// Set to defaults
void SetDefaultTesselation_QD3D();

#endif
