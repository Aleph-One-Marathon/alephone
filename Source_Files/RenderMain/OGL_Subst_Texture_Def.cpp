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
	March 12, 2000

	This contains implementations of functions for handling
	the OpenGL substitute textures for the walls and the sprites
*/

#include "cseries.h"
#include "OGL_Subst_Texture_Def.h"

#include <set>
#include <string>
#include <boost/unordered_map.hpp>

#ifdef HAVE_OPENGL

void OGL_TextureOptions::FindImagePosition()
{
	Right = Left + short(ImageScale*NormalImg.GetWidth() + 0.5);
	Bottom = Top + short(ImageScale*NormalImg.GetHeight() + 0.5);
}

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
		// Find adjusted-frame image-data positioning;
		// this is for doing sprites with textures with sizes different from the originals
		it->second.FindImagePosition();

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
	short Collection, CLUT, Bitmap;

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
	else if (StringsEqual(Tag,"bitmap"))
	{
		if (ReadBoundedInt16Value(Value,Bitmap,0,MAXIMUM_SHAPES_PER_COLLECTION-1))
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
	else if (StringsEqual(Tag,"image_scale"))
	{
		return ReadFloatValue(Value,Data.ImageScale);
	}
	else if (StringsEqual(Tag,"x_offset"))
	{
		return ReadInt16Value(Value,Data.Left);
	}
	else if (StringsEqual(Tag,"y_offset"))
	{
		return ReadInt16Value(Value,Data.Top);
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
	
	TOHash::iterator it = Collections[Collection].find(TOKey(CLUT, Bitmap));
	if (it == Collections[Collection].end())
	{
		Collections[Collection][TOKey(CLUT, Bitmap)] = Data;
		return true;
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

	if (Attributes.count("image_scale"))
	{
		it->second.ImageScale = Data.ImageScale;
	}

	if (Attributes.count("x_offset"))
	{
		it->second.Left = Data.Left;
	}

	if (Attributes.count("y_offset"))
	{
		it->second.Top = Data.Top;
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

#endif
