#ifndef _OGL_SUBST_TEXTURE_DEF_
#define _OGL_SUBST_TEXTURE_DEF_
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
	
	OpenGL Substitute-Texture-Definition File
	by Loren Petrich,
	May 11, 2003

	This contains the definitions of all the OpenGL substitute textures
	for the walls and the sprites
*/

#include "OGL_Texture_Def.h"
#include "XML_ElementParser.h"


#ifdef HAVE_OPENGL


// Options for wall textures and sprites
struct OGL_TextureOptions: public OGL_TextureOptionsBase
{
	bool VoidVisible;		// Can see the void through texture if semitransparent
	
	// Parameters for mapping substitute sprites (inhabitants, weapons in hand)
	
	// shape bounding box, in pixel units (need to apply scale factor)
	short shape_width;
	short shape_height;
	
	// shape origin / keypoint offset, in pixel units (need to apply scale factor)
	short offset_x;
	short offset_y;
	
	OGL_TextureOptions():
		VoidVisible(false),
		shape_width(0), shape_height(0),
		offset_x(0), offset_y(0) {}
};


// Get the texture options that are currently set
OGL_TextureOptions *OGL_GetTextureOptions(short Collection, short CLUT, short Bitmap);

// for managing the texture loading and unloading;
int OGL_CountTextures(short Collection);
void OGL_LoadTextures(short Collection);
void OGL_UnloadTextures(short Collection);

// XML support:
XML_ElementParser *TextureOptions_GetParser();
XML_ElementParser *TO_Clear_GetParser();

#endif // def HAVE_OPENGL

#endif
