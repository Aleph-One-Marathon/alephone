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
*/


#ifdef mac
#include <agl.h>
#endif

#include "cseries.h"
#include "shape_descriptors.h"
#include "OGL_Setup.h"

#include <string.h>


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
#else
#error OLG_Initialize() not implemented for this platform
#endif
#endif
}

// Test for presence
bool OGL_IsPresent() {return _OGL_IsPresent;}


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

const RGBColor DefaultFogColor = {0x8000,0x8000,0x8000};


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
	Data.FogColor = DefaultFogColor;
	Data.FogDepth = 8*1024;
}


// Texture-options stuff
static OGL_TextureOptions DefaultTextureOptions;

// If the color-table value has this value, it means all color tables:
const int ALL_CLUTS = -1;


// Store landscape stuff as a linked list;
// do in analogy with animated textures.
struct TextureOptionsLink: public OGL_TextureOptions
{
	// Which color table and bitmap to apply to:
	short CLUT, Bitmap;
	TextureOptionsLink *Next;
	
	TextureOptionsLink(): CLUT(ALL_CLUTS), Bitmap(-1), Next(NULL) {}
};

// Separate texture-options sequence lists for each collection ID,
// to speed up searching
static TextureOptionsLink *TORootList[NUMBER_OF_COLLECTIONS] =
{
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL,
};

// Deletes a collection's texture-options sequences
static void TODelete(int c)
{
	TextureOptionsLink *TOPtr = TORootList[c];
	while(TOPtr)
	{
		TextureOptionsLink *NextTOPtr = TOPtr->Next;
		delete TOPtr;
		TOPtr = NextTOPtr;
	}
	TORootList[c] = NULL;
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
		
	TextureOptionsLink *TOPtr = TORootList[Collection];
	while(TOPtr)
	{
		if (TOPtr->CLUT == CLUT || TOPtr->CLUT == ALL_CLUTS)
			if (TOPtr->Bitmap == Bitmap)
				return TOPtr;
		TOPtr = TOPtr->Next;
	}
	
	return &DefaultTextureOptions;
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
	TextureOptionsLink *TOPtr = TORootList[Collection], *PrevTOPtr = NULL;
	while(TOPtr)
	{
		if (TOPtr->CLUT == CLUT && TOPtr->Bitmap == Bitmap)
		{
			// Replace the data
			*((OGL_TextureOptions *)TOPtr) = Data;
			return true;
		}
		PrevTOPtr = TOPtr;
		TOPtr = TOPtr->Next;
	}
	// If not, then add a new frame
	TextureOptionsLink *NewTOPtr = new TextureOptionsLink;
	NewTOPtr->CLUT = CLUT;
	NewTOPtr->Bitmap = Bitmap;
	*((OGL_TextureOptions *)NewTOPtr) = Data;
	if (PrevTOPtr)
		PrevTOPtr->Next = NewTOPtr;
	else
		 TORootList[Collection] = NewTOPtr;
	
	return true;
}

static XML_TextureOptionsParser TextureOptionsParser;


static XML_ElementParser OpenGL_Parser("opengl");


// XML-parser support
XML_ElementParser *OpenGL_GetParser()
{
	OpenGL_Parser.AddChild(&TextureOptionsParser);
	OpenGL_Parser.AddChild(&TO_ClearParser);
	
	return &OpenGL_Parser;
}

