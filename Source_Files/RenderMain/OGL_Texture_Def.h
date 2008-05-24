#ifndef _OGL_TEXTURE_DEF_
#define _OGL_TEXTURE_DEF_
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
	
	OpenGL Texture-Definition File
	by Loren Petrich,
	May 11, 2003

	This contains the "base" definitions of the OpenGL textures,
	which are used both for wall/sprite substitutions and for
	model skins.
*/


#include <vector>
using namespace std;

#include "shape_descriptors.h"
#include "ImageLoader.h"

#ifdef HAVE_OPENGL


/*
	Since Apple OpenGL currently does not support indexed-color images in direct-color
	rendering, it's necessary to keep track of all possible images separately, and this means
	not only all possible color tables, but also infravision and silhouette images.
	OpenGL 1.2 will change all of that, however :-)
*/

enum {
	// The bitmap sets for the different color tables do not need to be listed
	INFRAVISION_BITMAP_SET = MAXIMUM_CLUTS_PER_COLLECTION,
	SILHOUETTE_BITMAP_SET,
	NUMBER_OF_OPENGL_BITMAP_SETS
};

// If the color-table value has this value, it means all color tables:
const int ALL_CLUTS = -1;


// Here are the texture-opacity types.
// Opacity is the value of the alpha channel, sometimes called transparency
enum
{
	OGL_OpacType_Crisp,		// The default: crisp edges, complete opacity
	OGL_OpacType_Flat,		// Fuzzy edges, but with flat opacity
	OGL_OpacType_Avg,		// Fuzzy edges, and opacity = average(color channel values)
	OGL_OpacType_Max,		// Fuzzy edges, and opacity = max(color channel values)
	OGL_NUMBER_OF_OPACITY_TYPES
};

// Here are the texture-blend types
enum
{
	OGL_BlendType_Crossfade,	// The default: crossfade from background to texture value
	OGL_BlendType_Add,			// Add texture value to background
	OGL_BlendType_Crossfade_Premult,
	OGL_BlendType_Add_Premult,
	OGL_NUMBER_OF_BLEND_TYPES,
	OGL_FIRST_PREMULT_ALPHA = OGL_BlendType_Crossfade_Premult
};

// Shared options for wall/sprite textures and for skins
struct OGL_TextureOptionsBase
{
	short OpacityType;		// Which type of opacity to use?
	float OpacityScale;		// How much to scale the opacity
	float OpacityShift;		// How much to shift the opacity
	bool Substitution;              // Is this a substitute texture?
	
	// Names of files to load; these will be extended ones with directory specifications
	// <dirname>/<dirname>/<filename>
	vector<char> NormalColors, NormalMask, GlowColors, GlowMask;

	// the image is premultiplied
	bool NormalIsPremultiplied, GlowIsPremultiplied;

	// hints passed into loadfromfile, in case file dimensions are POT
	short actual_height, actual_width;

	// what kind of texture this is, for texture quality purposes
	short Type;
	
	// Normal and glow-mapped images
	ImageDescriptor NormalImg, GlowImg;
	
	// Normal and glow blending
	short NormalBlend, GlowBlend;
	
	// For convenience
	void Load();
	void Unload();

	virtual int GetMaxSize();
	
	OGL_TextureOptionsBase():
	OpacityType(OGL_OpacType_Crisp), OpacityScale(1), OpacityShift(0),
		NormalBlend(OGL_BlendType_Crossfade), GlowBlend(OGL_BlendType_Crossfade), Substitution(false), NormalIsPremultiplied(false), GlowIsPremultiplied(false), actual_height(0), actual_width(0), Type(-1)
		{}
};

#endif

#endif

