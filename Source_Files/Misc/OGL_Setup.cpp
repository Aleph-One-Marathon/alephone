/*
	
	OpenGL Renderer,
	by Loren Petrich,
	March 12, 2000

	This contains implementations of functions intended for finding out OpenGL's presence
	in the host system, for setting parameters for OpenGL rendering,
	and for deciding whether to use OpenGL for rendering.
	
	June 11, 2000
	
	Had added XML parsing before that; most recently, added "opac_shift".
	
	Made semitransparency optional if the void is on one side of the texture

Oct 13, 2000 (Loren Petrich)
	Converted the OpenGL-addition accounting into Standard Template Library vectors

Nov 12, 2000 (Loren Petrich):
	Implemented texture substitution, also moved pixel-opacity editing into here;
	the code is carefully constructed to assume RGBA byte order whether integers are
	big- or little-endian.

Nov 18, 2000 (Loren Petrich):
	Added support for glow mapping; constrained it to only be present
	when a normal texture is present, and to have the same size

Nov 26, 2000 (Loren Petrich):
	Added system for reloading textures only when their filenames change.

Dec 17, 2000 (Loren Petrich):
	Eliminated fog parameters from the preferences;
	there is still a "fog present" switch, which is used to indicate
	whether fog will not be suppressed.
*/

#include <vector.h>
#include <string.h>
#include "cseries.h"

#ifdef HAVE_OPENGL
# ifdef mac
#  include <agl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#include "shape_descriptors.h"
#include "OGL_Setup.h"
#include "ColorParser.h"


// Whether or not OpenGL is present and usable
static bool _OGL_IsPresent = false;


// Initializer
bool OGL_Initialize()
{
#ifdef HAVE_OPENGL
#if defined(mac)
	// Cribbed from Apple's DrawSprocket documentation;
	// look for OpenGL function
	return (_OGL_IsPresent = ((Ptr)aglChoosePixelFormat != (Ptr)kUnresolvedCFragSymbolAddress));
	// return (_OGL_IsPresent = ((Ptr)glBegin != (Ptr)kUnresolvedCFragSymbolAddress));
#elif defined(SDL)
	// nothing to do
	return _OGL_IsPresent = true;
#else
#error OGL_Initialize() not implemented for this platform
#endif
#else
	return false;
#endif
}

// Test for presence
bool OGL_IsPresent() {return _OGL_IsPresent;}


// These are global so that Pfhortran can manipulate them
bool FogPresent = false;				// Fog inactive
GLfloat FogColor[4] = {0.5,0.5,0.5,0};	// Gray
GLfloat FogDepth = 8;					// Not too thick


// For flat landscapes:
const RGBColor DefaultLscpColors[4][2] =
{
	{
		{0xffff, 0xffff, 0x6666},		// Day
		{0x3333, 0x9999, 0xffff},
	},
	{
		{0x1818, 0x1818, 0x1010},		// Night
		{0x0808, 0x0808, 0x1010},
	},
	{
		{0x6666, 0x6666, 0x6666},		// Moon
		{0x0000, 0x0000, 0x0000},
	},
	{
		{0x0000, 0x0000, 0x0000},		// Outer Space
		{0x0000, 0x0000, 0x0000},
	},
};


// Set defaults
void OGL_SetDefaults(OGL_ConfigureData& Data)
{
	for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
	{
		OGL_Texture_Configure& TxtrData = Data.TxtrConfigList[k];
		TxtrData.NearFilter = 1;		// GL_LINEAR
		if (k == OGL_Txtr_Wall)
			TxtrData.FarFilter = 5;		// GL_LINEAR_MIPMAP_LINEAR
		else
			TxtrData.FarFilter = 0;		// GL_NEAREST
		TxtrData.Resolution = 0;		// 1x
		TxtrData.ColorFormat = 0;		// 32-bit color
	}
	Data.Flags = 0;						// Everything off
	Data.VoidColor = rgb_black;			// Self-explanatory
	for (int il=0; il<4; il++)
		for (int ie=0; ie<2; ie++)
			Data.LscpColors[il][ie] = DefaultLscpColors[il][ie];
}


// Texture-options stuff
static OGL_TextureOptions DefaultTextureOptions;

// If the color-table value has this value, it means all color tables:
const int ALL_CLUTS = -1;


// Store landscape stuff as a linked list;
// do in analogy with animated textures.
struct TextureOptionsEntry
{
	// Which color table and bitmap to apply to:
	short CLUT, Bitmap;
	
	// Make a member for more convenient access
	OGL_TextureOptions OptionsData;
	
	// Most recent files that the textures had been loaded from
	vector<char> NormalColors, NormalMask, GlowColors, GlowMask;
	
	TextureOptionsEntry(): CLUT(ALL_CLUTS), Bitmap(NONE) {}
};

// Separate texture-options sequence lists for each collection ID,
// to speed up searching
static vector<TextureOptionsEntry> TOList[NUMBER_OF_COLLECTIONS];

// Deletes a collection's texture-options sequences
static void TODelete(int c)
{
	TOList[c].clear();
}

// Deletes all of them
static void TODeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) TODelete(c);
}


OGL_TextureOptions *OGL_GetTextureOptions(short Collection, short CLUT, short Bitmap)
{
	// Silhouette textures always have the default
	if (CLUT == SILHOUETTE_BITMAP_SET) return &DefaultTextureOptions;
	
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		if (TOIter->CLUT == CLUT || TOIter->CLUT == ALL_CLUTS)
			if (TOIter->Bitmap == Bitmap)
				return &TOIter->OptionsData;
	}
	
	return &DefaultTextureOptions;
}



// Does this for a set of several pixel values or color-table values;
// the pixels are assumed to be in OpenGL-friendly byte-by-byte RGBA format.
void SetPixelOpacities(OGL_TextureOptions& Options, int NumPixels, uint32 *Pixels)
{
	for (int k=0; k<NumPixels; k++)
	{
		uint8 *PxlPtr = (uint8 *)(Pixels + k);
		
		// This won't be scaled to (0,1), but will be left at (0,255) here
		float Opacity;
		switch(Options.OpacityType)
		{
		// Two versions of the Tomb Raider texture-opacity hack
		case OGL_OpacType_Avg:
			{
				uint32 Red = uint32(PxlPtr[0]);
				uint32 Green = uint32(PxlPtr[1]);
				uint32 Blue = uint32(PxlPtr[2]);
				Opacity = (Red + Green + Blue)/3.0;
			}
			break;
			
		case OGL_OpacType_Max:
			{
				uint32 Red = uint32(PxlPtr[0]);
				uint32 Green = uint32(PxlPtr[1]);
				uint32 Blue = uint32(PxlPtr[2]);
				Opacity = MAX(MAX(Red,Green),Blue);
			}
			break;
		
		// Use pre-existing alpha value; useful if the opacity was loaded from a mask image
		default:
			Opacity = PxlPtr[3];
			break;
		}
		
		// Scale, shift, and put back the edited opacity;
		// round off and pin to the appropriate range.
		// The shift has to be scaled to the color-channel range (1 -> 255).
		PxlPtr[3] = PIN(int32(Options.OpacityScale*Opacity + 255*Options.OpacityShift + 0.5),0,255);
	}
}


inline bool StringPresent(vector<char>& String)
{
	return (String.size() > 1);
}

static bool StringsEqual(vector<char>& Str1, vector<char>& Str2)
{
	bool Pres1 = StringPresent(Str1);
	bool Pres2 = StringPresent(Str2);
	if (Pres1 || Pres2)
	{
		if (Pres1 && Pres2)
		{
			return strcmp(&Str1[0],&Str2[0]);
		}
		else
			return false;
	}
	else
		return false;
	return false;
}

static void StringCopy(vector<char>& StrDest, vector<char>& StrSrc)
{
	int Len = StrSrc.size();
	StrDest.resize(Len);
	memcpy(&StrDest[0],&StrSrc[0],Len);
}


// for managing the image loading and unloading
void OGL_LoadImages(int Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);
	
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		FileSpecifier File;
		
		// Load the normal image with alpha channel
		try
		{
			// Check to see if loading needs to be done;
			// it does not need to be if the filenames are the same and an image is present.
			// The image is always cleared before loading, for safety in case of invalidity.
			if (StringsEqual(TOIter->OptionsData.NormalColors,TOIter->NormalColors) &&
				StringsEqual(TOIter->OptionsData.NormalMask,TOIter->NormalMask))
			{
				if (TOIter->OptionsData.NormalImg.IsPresent())
					throw 0;
				else
					TOIter->OptionsData.NormalImg.Clear();
			}
			else
			{
				StringCopy(TOIter->NormalColors,TOIter->OptionsData.NormalColors);
				StringCopy(TOIter->NormalMask,TOIter->OptionsData.NormalMask);
				TOIter->OptionsData.NormalImg.Clear();
			}
			
			// Load the normal image if it has a filename specified for it
			if (!StringPresent(TOIter->OptionsData.NormalColors))
				throw 0;
#ifdef mac
			if (!File.SetToApp()) throw 0;
#endif
			if (!File.SetNameWithPath(&TOIter->OptionsData.NormalColors[0])) throw 0;
			if (!LoadImageFromFile(TOIter->OptionsData.NormalImg,File,ImageLoader_Colors)) throw 0;
			
			// Load the normal mask if it has a filename specified for it;
			// only loaded if an image has been loaded for it
			if (!StringPresent(TOIter->OptionsData.NormalMask)) throw 0;
#ifdef mac
			if (!File.SetToApp()) throw 0;
#endif
			if (!File.SetNameWithPath(&TOIter->OptionsData.NormalMask[0])) throw 0;
			if (!LoadImageFromFile(TOIter->OptionsData.NormalImg,File,ImageLoader_Opacity)) throw 0;
		}
		catch(...)
		{}
		
		// Load the glow image with alpha channel
		try
		{
			// Check to see if loading needs to be done;
			// it does not need to be if the filenames are the same and an image is present.
			// The image is always cleared before loading, for safety in case of invalidity.
			if (StringsEqual(TOIter->OptionsData.GlowColors,TOIter->GlowColors) &&
				StringsEqual(TOIter->OptionsData.GlowMask,TOIter->GlowMask))
			{
				if (TOIter->OptionsData.GlowImg.IsPresent())
					throw 0;
				else
					TOIter->OptionsData.GlowImg.Clear();
			}
			else
			{
				StringCopy(TOIter->GlowColors,TOIter->OptionsData.GlowColors);
				StringCopy(TOIter->GlowMask,TOIter->OptionsData.GlowMask);
				TOIter->OptionsData.GlowImg.Clear();
			}
			
			// Load the glow image if it has a filename specified for it
			if (!StringPresent(TOIter->OptionsData.GlowColors)) throw 0;
#ifdef mac
			if (!File.SetToApp()) throw 0;
#endif
			if (!File.SetNameWithPath(&TOIter->OptionsData.GlowColors[0])) throw 0;
			if (!LoadImageFromFile(TOIter->OptionsData.GlowImg,File,ImageLoader_Colors)) throw 0;
			
			// Load the glow mask if it has a filename specified for it;
			// only loaded if an image has been loaded for it
			if (!StringPresent(TOIter->OptionsData.GlowMask)) throw 0;
#ifdef mac
			if (!File.SetToApp()) throw 0;
#endif
			if (!File.SetNameWithPath(&TOIter->OptionsData.GlowMask[0])) throw 0;
			if (!LoadImageFromFile(TOIter->OptionsData.GlowImg,File,ImageLoader_Opacity)) throw 0;
		}
		catch(...)
		{}
		
		// The rest of the code is made simpler by these constraints:
		// that the glow texture only be present if the normal texture is also present,
		// and that the normal and glow textures have the same dimensions
		if (TOIter->OptionsData.NormalImg.IsPresent())
		{
			int W0 = TOIter->OptionsData.NormalImg.GetWidth();
			int W1 = TOIter->OptionsData.GlowImg.GetWidth();
			int H0 = TOIter->OptionsData.NormalImg.GetHeight();
			int H1 = TOIter->OptionsData.GlowImg.GetHeight();
			if ((W1 != W0) || (H1 != H0)) TOIter->OptionsData.GlowImg.Clear();
		}
		else
		{
			TOIter->OptionsData.GlowImg.Clear();
		}
	}
}

void OGL_UnloadImages(int Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);
	
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		TOIter->OptionsData.NormalImg.Clear();
		TOIter->OptionsData.GlowImg.Clear();
	}
}


// XML-parsing stuff

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
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Collection,short(0),short(NUMBER_OF_COLLECTIONS-1)))
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
		TODeleteAll();
	
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
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Collection,short(0),short(NUMBER_OF_COLLECTIONS-1)))
		{
			CollIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"clut") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",CLUT,short(ALL_CLUTS),short(INFRAVISION_BITMAP_SET)));
	}
	else if (strcmp(Tag,"bitmap") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Bitmap,short(0),short(MAXIMUM_SHAPES_PER_COLLECTION-1)))
		{
			BitmapIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"opac_type") == 0)
	{
		return (ReadBoundedNumericalValue(Value,"%hd",Data.OpacityType,short(0),short(OGL_NUMBER_OF_OPACITY_TYPES-1)));
	}
	else if (strcmp(Tag,"opac_scale") == 0)
	{
		return (ReadNumericalValue(Value,"%f",Data.OpacityScale));
	}
	else if (strcmp(Tag,"opac_shift") == 0)
	{
		return (ReadNumericalValue(Value,"%f",Data.OpacityShift));
	}
	else if (strcmp(Tag,"void_visible") == 0)
	{
		return (ReadBooleanValue(Value,Data.VoidVisible));
	}
	else if (strcmp(Tag,"normal_image") == 0)
	{
		int nchars = strlen(Value)+1;
		Data.NormalColors.resize(nchars);
		memcpy(&Data.NormalColors[0],Value,nchars);
		return true;
	}
	else if (strcmp(Tag,"normal_mask") == 0)
	{
		int nchars = strlen(Value)+1;
		Data.NormalMask.resize(nchars);
		memcpy(&Data.NormalMask[0],Value,nchars);
		return true;
	}
	else if (strcmp(Tag,"glow_image") == 0)
	{
		int nchars = strlen(Value)+1;
		Data.GlowColors.resize(nchars);
		memcpy(&Data.GlowColors[0],Value,nchars);
		return true;
	}
	else if (strcmp(Tag,"glow_mask") == 0)
	{
		int nchars = strlen(Value)+1;
		Data.GlowMask.resize(nchars);
		memcpy(&Data.GlowMask[0],Value,nchars);
		return true;
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

static XML_TextureOptionsParser TextureOptionsParser;


class XML_FogParser: public XML_ElementParser
{
	rgb_color Color;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool End();
	
	XML_FogParser(): XML_ElementParser("fog") {}
};

bool XML_FogParser::Start()
{
	Color.red = PIN(int(65535*FogColor[0]+0.5),0,65535);
	Color.green = PIN(int(65535*FogColor[1]+0.5),0,65535);
	Color.blue = PIN(int(65535*FogColor[2]+0.5),0,65535);
	Color_SetArray(&Color);		
	return true;
}

bool XML_FogParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"on") == 0)
	{
		return (ReadBooleanValue(Value,FogPresent));
	}
	else if (strcmp(Tag,"depth") == 0)
	{
		return (ReadNumericalValue(Value,"%f",FogDepth));
	}
	UnrecognizedTag();
	return false;
}

bool XML_FogParser::End()
{
	FogColor[0] = Color.red/65535.0;
	FogColor[1] = Color.green/65535.0;
	FogColor[2] = Color.blue/65535.0;
	return true;
}

static XML_FogParser FogParser;

static XML_ElementParser OpenGL_Parser("opengl");


// XML-parser support
XML_ElementParser *OpenGL_GetParser()
{
	OpenGL_Parser.AddChild(&TextureOptionsParser);
	OpenGL_Parser.AddChild(&TO_ClearParser);
	FogParser.AddChild(Color_GetParser());
	OpenGL_Parser.AddChild(&FogParser);
	
	return &OpenGL_Parser;
}

