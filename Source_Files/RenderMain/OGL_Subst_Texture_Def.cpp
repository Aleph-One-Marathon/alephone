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
	March 12, 2000

	This contains implementations of functions for handling
	the OpenGL substitute textures for the walls and the sprites
*/

#include "cseries.h"
#include "OGL_Subst_Texture_Def.h"
#include "Logging.h"
#include "InfoTree.h"

#include <set>
#include <string>
#include <boost/unordered_map.hpp>

#ifdef HAVE_OPENGL

// Texture-options stuff;
// defaults for whatever might need them
static OGL_TextureOptions DefaultTextureOptions;

typedef std::pair<short, short> TOKey;
typedef boost::unordered_map<TOKey, OGL_TextureOptions> TOHash;
static TOHash Collections[NUMBER_OF_COLLECTIONS];

// Deletes a collection's texture-options sequences
void TODelete(short Collection)
{
	Collections[Collection].clear();
}

// Deletes all of them
static void TODelete_All()
{
	for (int c = 0; c < NUMBER_OF_COLLECTIONS; c++) TODelete(c);
}

int OGL_CountTextures(short Collection)
{
	return Collections[Collection].size();
}

extern void OGL_ProgressCallback(int);

void OGL_LoadTextures(short Collection)
{

	for (TOHash::iterator it = Collections[Collection].begin(); it != Collections[Collection].end(); ++it)
	{
		it->second.Load();
		OGL_ProgressCallback(1);
		
	}
}


void OGL_UnloadTextures(short Collection)
{
	for (TOHash::iterator it = Collections[Collection].begin(); it != Collections[Collection].end(); ++it)
	{
		it->second.Unload();
	}
}


OGL_TextureOptions *OGL_GetTextureOptions(short Collection, short CLUT, short Bitmap)
{
	TOHash::iterator it = Collections[Collection].find(TOKey(CLUT, Bitmap));
	if (it != Collections[Collection].end())
	{
		return &it->second;
	}
	
	if (IsInfravisionTable(CLUT) && CLUT != INFRAVISION_BITMAP_SET)
	{
		it = Collections[Collection].find(TOKey(INFRAVISION_BITMAP_SET, Bitmap));
		if (it != Collections[Collection].end())
		{
			return &it->second;
		}
	}

	if (IsSilhouetteTable(CLUT) && CLUT != SILHOUETTE_BITMAP_SET)
	{
		it = Collections[Collection].find(TOKey(SILHOUETTE_BITMAP_SET, Bitmap));
		if (it != Collections[Collection].end())
		{
			return &it->second;
		}
	}
	
	it = Collections[Collection].find(TOKey(ALL_CLUTS, Bitmap));
	if (it != Collections[Collection].end())
	{
		return &it->second;
	}

	return &DefaultTextureOptions;
}


void reset_mml_opengl_texture()
{
	TODelete_All();
}

void parse_mml_opengl_texture(const InfoTree& root)
{
	int16 coll;
	if (!root.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
		return;
	
	int16 bitmap;
	if (!root.read_indexed("bitmap", bitmap, INT16_MAX+1))
		return;
	
	int16 clut = ALL_CLUTS;
	root.read_attr_bounded<int16>("clut", clut, ALL_CLUTS, SILHOUETTE_BITMAP_SET);
	
	int16 clut_variant = CLUT_VARIANT_NORMAL;
	root.read_attr_bounded<int16>("clut_variant", clut_variant, ALL_CLUT_VARIANTS, NUMBER_OF_CLUT_VARIANTS-1);

	// translate deprecated clut options
	if (clut == INFRAVISION_BITMAP_SET)
	{
		clut = ALL_CLUTS;
		clut_variant = CLUT_VARIANT_INFRAVISION;
	}
	else if (clut == SILHOUETTE_BITMAP_SET)
	{
		clut = ALL_CLUTS;
		clut_variant = CLUT_VARIANT_SILHOUETTE;
	}
	
	// loop so we can apply "all variants" mode if needed
	for (short var = CLUT_VARIANT_NORMAL; var < NUMBER_OF_CLUT_VARIANTS; var++)
	{
		if (clut_variant != ALL_CLUT_VARIANTS && clut_variant != var)
			continue;
		
		// translate clut+variant to internal clut number
		short actual_clut = clut;
		if (var == CLUT_VARIANT_INFRAVISION)
		{
			if (clut == ALL_CLUTS)
				actual_clut = INFRAVISION_BITMAP_SET;
			else
				actual_clut = INFRAVISION_BITMAP_CLUTSPECIFIC + clut;
		}
		else if (var == CLUT_VARIANT_SILHOUETTE)
		{
			if (clut == ALL_CLUTS)
				actual_clut = SILHOUETTE_BITMAP_SET;
			else
				actual_clut = SILHOUETTE_BITMAP_CLUTSPECIFIC + clut;
		}
		
		TOHash::iterator it = Collections[coll].find(TOKey(actual_clut, bitmap));
		if (it == Collections[coll].end())
		{
			Collections[coll][TOKey(actual_clut, bitmap)] = DefaultTextureOptions;
			it = Collections[coll].find(TOKey(actual_clut, bitmap));
		}
		
		OGL_TextureOptions& def = it->second;
		root.read_indexed("opac_type", def.OpacityType, OGL_NUMBER_OF_OPACITY_TYPES);
		root.read_attr("opac_scale", def.OpacityScale);
		root.read_attr("opac_shift", def.OpacityShift);
		root.read_attr("void_visible", def.VoidVisible);
		root.read_path("normal_image", def.NormalColors);
		root.read_path("offset_image", def.OffsetMap);
		root.read_path("normal_mask", def.NormalMask);
		root.read_path("glow_image", def.GlowColors);
		root.read_path("glow_mask", def.GlowMask);
		root.read_indexed("normal_blend", def.NormalBlend, OGL_NUMBER_OF_BLEND_TYPES);
		root.read_indexed("glow_blend", def.GlowBlend, OGL_NUMBER_OF_BLEND_TYPES);
		root.read_attr("actual_height", def.actual_height);
		root.read_attr("actual_width", def.actual_width);
		root.read_attr("type", def.Type);
		root.read_attr("normal_premultiply", def.NormalIsPremultiplied);
		root.read_attr("glow_premultiply", def.GlowIsPremultiplied);
		root.read_attr("normal_bloom_scale", def.BloomScale);
		root.read_attr("normal_bloom_shift", def.BloomShift);
		root.read_attr("glow_bloom_scale", def.GlowBloomScale);
		root.read_attr("glow_bloom_shift", def.GlowBloomShift);
		root.read_attr("landscape_bloom", def.LandscapeBloom);
		root.read_attr("minimum_glow_intensity", def.MinGlowIntensity);
	}
}

void parse_mml_opengl_txtr_clear(const InfoTree& root)
{
	int16 coll;
	if (root.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS))
		TODelete(coll);
	else
		TODelete_All();
}

#endif
