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


// Defined in OGL_Setup.cpp
extern void SetPixelOpacities(OGL_TextureOptions& Options, int NumPixels, uint32 *Pixels);


void OGL_TextureOptions::FindImagePosition()
{
	Right = Left + short(ImageScale*NormalImg.GetWidth() + 0.5);
	Bottom = Top + short(ImageScale*NormalImg.GetHeight() + 0.5);
}



// Texture-options stuff;
// defaults for whatever might need them
static OGL_TextureOptions DefaultTextureOptions;


// Store texture-options stuff in a set of STL vectors
struct TextureOptionsEntry
{
	// Which color table and bitmap to apply to:
	short CLUT, Bitmap;
	
	// Make a member for more convenient access
	OGL_TextureOptions OptionsData;
	
	TextureOptionsEntry(): CLUT(ALL_CLUTS), Bitmap(NONE) {}
};

// Separate texture-options sequence lists for each collection ID,
// to speed up searching
static vector<TextureOptionsEntry> TOList[NUMBER_OF_COLLECTIONS];

// Texture-options hash table for extra-fast searching;
// the top bit of the hashtable index is set if some specific CLUT had been matched to.
// If it is clear, then an ALL_CLUTS texture-options entry had been used.
// This is OK because the maximum reasonable number of texture-option entries per collection
// is around 10*256 or 2560, much less than 32K.
const uint16 Specific_CLUT_Flag = 0x8000;
static vector<int16> TOHash[NUMBER_OF_COLLECTIONS];

// Hash-table size and function
const int TOHashSize = 1 << 8;
const int TOHashMask = TOHashSize - 1;
inline uint8 TOHashFunc(short CLUT, short Bitmap)
{
	// This function will avoid collisions when accessing bitmaps with close indices
	return (uint8)((CLUT << 4) ^ Bitmap);
}


// Deletes a collection's texture-options sequences
void TODelete(short Collection)
{
	int c = Collection;
	TOList[c].clear();
	TOHash[c].clear();
}

// Deletes all of them
static void TODelete_All()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) TODelete(c);
}


void OGL_LoadTextures(short Collection)
{
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		// Load the images
		TOIter->OptionsData.Load();
		
		// Find adjusted-frame image-data positioning;
		// this is for doing sprites with textures with sizes different from the originals
		TOIter->OptionsData.FindImagePosition();
	}
}


void OGL_UnloadTextures(short Collection)
{
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		TOIter->OptionsData.Unload();
	}
}


OGL_TextureOptions *OGL_GetTextureOptions(short Collection, short CLUT, short Bitmap)
{
	// Initialize the hash table if necessary
	if (TOHash[Collection].empty())
	{
		TOHash[Collection].resize(TOHashSize);
		objlist_set(&TOHash[Collection][0],NONE,TOHashSize);
	}
	
	// Set up a *reference* to the appropriate hashtable entry;
	// this makes setting this entry a bit more convenient
	int16& HashVal = TOHash[Collection][TOHashFunc(CLUT,Bitmap)];
	
	// Check to see if the texture-option entry is correct;
	// if it is, then we're done.
	// Be sure to blank out the specific-CLUT flag when indexing the texture-options list with the hash value.
	if (HashVal != NONE)
	{
		vector<TextureOptionsEntry>::iterator TOIter = TOList[Collection].begin() + (HashVal & ~Specific_CLUT_Flag);
		bool Specific_CLUT_Set = (TOIter->CLUT == CLUT);
		bool Hash_SCS = TEST_FLAG(HashVal,Specific_CLUT_Flag);
		if ((Specific_CLUT_Set && Hash_SCS) || ((TOIter->CLUT == ALL_CLUTS) && !Hash_SCS))
			if (TOIter->Bitmap == Bitmap)
			{
				return &TOIter->OptionsData;
			}
	}
	
	// Fallback for the case of a hashtable miss;
	// do a linear search and then update the hash entry appropriately.
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	int16 Indx = 0;
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++, Indx++)
	{
		bool Specific_CLUT_Set = (TOIter->CLUT == CLUT);
		if (Specific_CLUT_Set || (TOIter->CLUT == ALL_CLUTS))
			if (TOIter->Bitmap == Bitmap)
			{
				HashVal = Indx;
				SET_FLAG(HashVal,Specific_CLUT_Flag,Specific_CLUT_Set);
				return &TOIter->OptionsData;
			}
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
	
	OGL_TextureOptions Data;

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
		
	return true;
}

bool XML_TextureOptionsParser::HandleAttribute(const char *Tag, const char *Value)
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
		size_t nchars = strlen(Value)+1;
		Data.NormalColors.resize(nchars);
		memcpy(&Data.NormalColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"normal_mask"))
	{
		size_t nchars = strlen(Value)+1;
		Data.NormalMask.resize(nchars);
		memcpy(&Data.NormalMask[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_image"))
	{
		size_t nchars = strlen(Value)+1;
		Data.GlowColors.resize(nchars);
		memcpy(&Data.GlowColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_mask"))
	{
		size_t nchars = strlen(Value)+1;
		Data.GlowMask.resize(nchars);
		memcpy(&Data.GlowMask[0],Value,nchars);
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
	UnrecognizedTag();
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
	
	// Check to see if a frame is already accounted for
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		if (TOIter->CLUT == CLUT && TOIter->Bitmap == Bitmap)
		{
			// Replace the data
			TOIter->OptionsData = Data;
			return true;
		}
	}
	
	// If not, then add a new frame entry
	TextureOptionsEntry DataEntry;
	DataEntry.CLUT = CLUT;
	DataEntry.Bitmap = Bitmap;
	DataEntry.OptionsData = Data;
	TOL.push_back(DataEntry);
	
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
