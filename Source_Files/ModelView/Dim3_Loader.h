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

	"Dim3" Object Loader
	
	By Loren Petrich, Dec 29, 2001
*/
#ifndef DIM3_LOADER
#define DIM3_LOADER

#include "Model3D.h"
#include "FileHandler.h"


// Do multifile models by doing multiple passes;
// the first one is special because it sets up for the run
enum
{
	LoadModelDim3_First,
	LoadModelDim3_Rest
};

bool LoadModel_Dim3(FileSpecifier& Spec, Model3D& Model, int WhichPass);

#endif
