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


class XML_TO_ClearParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_TO_ClearParser(): XML_ElementParser("txtr_clear") {}
};

bool XML_TO_ClearParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_TO_ClearParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,0,NUMBER_OF_COLLECTIONS-1))
		{
			IsPresent = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_TO_ClearParser::AttributesDone()
{
	if (IsPresent)
		TODelete(Collection);
	else
		TODelete_All();
	
	return true;
}

static XML_TO_ClearParser TO_ClearParser;


class XML_TextureOptionsParser: public XML_ElementParser
{
	bool CollIsPresent, BitmapIsPresent;
	short Collection, CLUT, Bitmap, CLUT_Variant;

	std::set<std::string> Attributes;
	
	OGL_TextureOptions Data;

	bool _HandleAttribute(const char *Tag, const char *Value);

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool ResetValues();

	XML_TextureOptionsParser(): XML_ElementParser("texture") {}
};

bool XML_TextureOptionsParser::Start()
{
	Data = DefaultTextureOptions;
	CollIsPresent = BitmapIsPresent = false;
	CLUT = ALL_CLUTS;
	CLUT_Variant = CLUT_VARIANT_NORMAL;
	Attributes.clear();
		
	return true;
}

bool XML_TextureOptionsParser::_HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"coll"))
	{
		if (ReadBoundedInt16Value(Value,Collection,0,NUMBER_OF_COLLECTIONS-1))
		{
			CollIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"clut"))
	{
		return ReadBoundedInt16Value(Value,CLUT,short(ALL_CLUTS),short(SILHOUETTE_BITMAP_SET));
	}
	else if (StringsEqual(Tag,"clut_variant"))
	{
		return ReadBoundedInt16Value(Value,CLUT_Variant,short(ALL_CLUT_VARIANTS),short(NUMBER_OF_CLUT_VARIANTS)-1);
	}
	else if (StringsEqual(Tag,"bitmap"))
	{
		if (ReadBoundedInt16Value(Value,Bitmap,0,INT16_MAX))
		{
			BitmapIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"opac_type"))
	{
		return ReadBoundedInt16Value(Value,Data.OpacityType,0,OGL_NUMBER_OF_OPACITY_TYPES-1);
	}
	else if (StringsEqual(Tag,"opac_scale"))
	{
		return ReadFloatValue(Value,Data.OpacityScale);
	}
	else if (StringsEqual(Tag,"opac_shift"))
	{
		return ReadFloatValue(Value,Data.OpacityShift);
	}
	else if (StringsEqual(Tag,"void_visible"))
	{
		return ReadBooleanValueAsBool(Value,Data.VoidVisible);
	}
	else if (StringsEqual(Tag,"normal_image"))
	{
		Data.NormalColors.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"offset_image"))
	{
		Data.OffsetMap.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"normal_mask"))
	{
		Data.NormalMask.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"glow_image"))
	{
		Data.GlowColors.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"glow_mask"))
	{
		Data.GlowMask.SetNameWithPath(Value);
		return true;
	}
	else if (StringsEqual(Tag,"normal_blend"))
	{
		return ReadBoundedInt16Value(Value,Data.NormalBlend,0,OGL_NUMBER_OF_BLEND_TYPES-1);
	}
	else if (StringsEqual(Tag,"glow_blend"))
	{
		return ReadBoundedInt16Value(Value,Data.GlowBlend,0,OGL_NUMBER_OF_BLEND_TYPES-1);
	}
	else if (StringsEqual(Tag, "image_scale"))
	{
		logWarning("Ignoring deprecated image_scale tag");
		return true;
	}
	else if (StringsEqual(Tag, "x_offset"))
	{
		logWarning("Ignoring deprecated x_offset tag");
		return true;
	}
	else if (StringsEqual(Tag, "y_offset"))
	{
		logWarning("Ignoring deprecated y_offset tag");
		return true;
	}
	else if (StringsEqual(Tag,"shape_width"))
	{
		logWarning("Ignoring deprecated shape_width tag");
		return true;
	}
	else if (StringsEqual(Tag,"shape_height"))
	{
		logWarning("Ignoring deprecated shape_height tag");
		return true;
	}
	else if (StringsEqual(Tag,"offset_x"))
	{
		logWarning("Ignoring deprecated offset_x tag");
		return true;
	}
	else if (StringsEqual(Tag,"offset_y"))
	{
		logWarning("Ignoring deprecated offset_y tag");
		return true;
	}
	else if (StringsEqual(Tag,"actual_height"))
	{
		return ReadInt16Value(Value, Data.actual_height);
	} 
	else if (StringsEqual(Tag, "actual_width"))
	{
		return ReadInt16Value(Value, Data.actual_width);
	}
	else if (StringsEqual(Tag, "type"))
	{
		return ReadInt16Value(Value, Data.Type);
	}
	else if (StringsEqual(Tag, "normal_premultiply"))
	{
		return ReadBooleanValueAsBool(Value, Data.NormalIsPremultiplied);
	}
	else if (StringsEqual(Tag, "glow_premultiply"))
	{
		return ReadBooleanValueAsBool(Value, Data.GlowIsPremultiplied);
	}
	else if (StringsEqual(Tag,"normal_bloom_scale"))
	{
		return ReadFloatValue(Value,Data.BloomScale);
	}
	else if (StringsEqual(Tag,"normal_bloom_shift"))
	{
		return ReadFloatValue(Value,Data.BloomShift);
	}
	else if (StringsEqual(Tag,"glow_bloom_scale"))
	{
		return ReadFloatValue(Value,Data.GlowBloomScale);
	}
	else if (StringsEqual(Tag,"glow_bloom_shift"))
	{
		return ReadFloatValue(Value,Data.GlowBloomShift);
	}
	else if (StringsEqual(Tag,"landscape_bloom"))
	{
		return ReadFloatValue(Value,Data.LandscapeBloom);
	}
	else if (StringsEqual(Tag,"minimum_glow_intensity"))
	{
		return ReadFloatValue(Value,Data.MinGlowIntensity);
	}
	UnrecognizedTag();
	return false;
}

bool XML_TextureOptionsParser::HandleAttribute(const char* Tag, const char* Value)
{
	if (_HandleAttribute(Tag, Value))
	{
		Attributes.insert(Tag);
		return true;
	}
	return false;
}

bool XML_TextureOptionsParser::AttributesDone()
{
	// Verify...
	if (!CollIsPresent || !BitmapIsPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// translate deprecated clut options
	if (CLUT == INFRAVISION_BITMAP_SET)
	{
		CLUT = ALL_CLUTS;
		CLUT_Variant = CLUT_VARIANT_INFRAVISION;
	}
	else if (CLUT == SILHOUETTE_BITMAP_SET)
	{
		CLUT = ALL_CLUTS;
		CLUT_Variant = CLUT_VARIANT_SILHOUETTE;
	}
	
	// loop so we can apply "all variants" mode if needed
	for (short var = CLUT_VARIANT_NORMAL; var < NUMBER_OF_CLUT_VARIANTS; var++)
	{
		if (CLUT_Variant != ALL_CLUT_VARIANTS && CLUT_Variant != var)
			continue;
		
		// translate clut+variant to internal clut number
		short actual_clut = CLUT;
		if (var == CLUT_VARIANT_INFRAVISION)
		{
			if (CLUT == ALL_CLUTS)
				actual_clut = INFRAVISION_BITMAP_SET;
			else
				actual_clut = INFRAVISION_BITMAP_CLUTSPECIFIC + CLUT;
		}
		else if (var == CLUT_VARIANT_SILHOUETTE)
		{
			if (CLUT == ALL_CLUTS)
				actual_clut = SILHOUETTE_BITMAP_SET;
			else
				actual_clut = SILHOUETTE_BITMAP_CLUTSPECIFIC + CLUT;
		}
	
	TOHash::iterator it = Collections[Collection].find(TOKey(actual_clut, Bitmap));
	if (it == Collections[Collection].end())
	{
		Collections[Collection][TOKey(actual_clut, Bitmap)] = Data;
		continue;
	}
		
	if (Attributes.count("opac_type"))
	{
		it->second.OpacityType = Data.OpacityType;
	}

	if (Attributes.count("opac_scale"))
	{
		it->second.OpacityScale = Data.OpacityScale;
	}

	if (Attributes.count("opac_shift"))
	{
		it->second.OpacityShift = Data.OpacityShift;
	}

	if (Attributes.count("void_visible"))
	{
		it->second.VoidVisible = Data.VoidVisible;
	}

	if (Attributes.count("normal_image"))
	{
		it->second.NormalColors = Data.NormalColors;
	}

	if (Attributes.count("offset_image"))
	{
		it->second.OffsetMap = Data.OffsetMap;
	}	

	if (Attributes.count("normal_mask"))
	{
		it->second.NormalMask = Data.NormalMask;
	}	

	if (Attributes.count("glow_image"))
	{
		it->second.GlowColors = Data.GlowColors;
	}

	if (Attributes.count("glow_mask"))
	{
		it->second.GlowMask = Data.GlowMask;
	}

	if (Attributes.count("normal_blend"))
	{
		it->second.NormalBlend = Data.NormalBlend;
	}

	if (Attributes.count("glow_blend"))
	{
		it->second.GlowBlend = Data.GlowBlend;
	}

	if (Attributes.count("actual_height"))
	{
		it->second.actual_height = Data.actual_height;
	}

	if (Attributes.count("actual_width"))
	{
		it->second.actual_width = Data.actual_width;
	}

	if (Attributes.count("type"))
	{
		it->second.Type = Data.Type;
	}

	if (Attributes.count("normal_premultiply"))
	{
		it->second.NormalIsPremultiplied = Data.NormalIsPremultiplied;
	}

	if (Attributes.count("glow_premultiply"))
	{
		it->second.GlowIsPremultiplied = Data.GlowIsPremultiplied;
	}

	if (Attributes.count("normal_bloom_scale"))
	{
		it->second.BloomScale = Data.BloomScale;
	}

	if (Attributes.count("normal_bloom_shift"))
	{
		it->second.BloomShift = Data.BloomShift;
	}

	if (Attributes.count("glow_bloom_scale"))
	{
		it->second.GlowBloomScale = Data.GlowBloomScale;
	}

	if (Attributes.count("glow_bloom_shift"))
	{
		it->second.GlowBloomShift = Data.GlowBloomShift;
	}

	if (Attributes.count("landscape_bloom"))
	{
		it->second.LandscapeBloom = Data.LandscapeBloom;
	}
	
	if (Attributes.count("minimum_glow_intensity"))
	{
		it->second.MinGlowIntensity = Data.MinGlowIntensity;
	}
	}
	
	return true;
}

bool XML_TextureOptionsParser::ResetValues()
{
	TODelete_All();
	return true;
}

static XML_TextureOptionsParser TextureOptionsParser;


// XML-parser support:
XML_ElementParser *TextureOptions_GetParser() {return &TextureOptionsParser;}
XML_ElementParser *TO_Clear_GetParser() {return &TO_ClearParser;}

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
