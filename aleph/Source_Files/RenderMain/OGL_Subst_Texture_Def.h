#ifndef _OGL_SUBST_TEXTURE_DEF_
#define _OGL_SUBST_TEXTURE_DEF_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
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
	// How many internal units (world unit = 1024) per pixel
	float ImageScale;
	
	// Positioning of sprite's corners relative to top left corner of original bitmap,
	// in internal units. Left and Top are specified as X_Offset and Y_Offset in MML;
	// Right and Bottom are calculated from these.
	short Left;
	short Top;
	short Right;
	short Bottom;
	
	// Find Right and Bottom from Left and Top and the image size and scaling
	void FindImagePosition();

	// use this to set the original width and height before calculating ImagePosition
	void Original();

	OGL_TextureOptions():
		VoidVisible(false), ImageScale(0),
		Left(0), Top(0), Right(0), Bottom(0) {}
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
