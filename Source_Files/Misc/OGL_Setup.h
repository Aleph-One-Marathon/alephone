#ifndef _OGL_SETUP_
#define _OGL_SETUP_
/*
	
	OpenGL Interface File,
	by Loren Petrich,
	March 12, 2000

	This contains functions intended for finding out OpenGL's presence
	in the host system, for setting parameters for OpenGL rendering,
	and for deciding whether to use OpenGL for rendering.
	
	May 27, 2000 (Loren Petrich)
	
	Added support for flat static effect

	Added XML support
	
	May 31, 2000 (Loren Petrich)
	
	Added support for texture resetting. This clears all the textures in memory
	and forces them to be reloaded. This may be good in cases of textures dropping out.
	
	June 11, 2000 (Loren Petrich)
	
	Added support for see-through liquids as an OpenGL parameter.
	Also added an opacity-value shift for making dark areas more visible
*/

#include "shape_descriptors.h"
#include "XML_ElementParser.h"

// These functions return whether OpenGL can be present

// Initializer
bool OGL_Initialize();

// Test for presence
bool OGL_IsPresent();

// Here are some OpenGL configuration options and how to access them
// (they are in the preferences data)

// There are separate texturing options for each kind of texture,
// as listed below; this is so that one can degrade texture quality independently,
// and have (say) high-quality walls and weapons in hand, medium-quality inhabitant sprites,
// and low-quality landscapes.
enum
{
	OGL_Txtr_Wall,
	OGL_Txtr_Landscape,
	OGL_Txtr_Inhabitant,
	OGL_Txtr_WeaponsInHand,
	OGL_NUMBER_OF_TEXTURE_TYPES
};

/*
	All enumeration starts from 0, in contrast to MacOS popup menus, for example,
	which start from one.
	
	The filters are OpenGL filter types
	GL_NEAREST		(Pixelated)
	GL_LINEAR		(Smoothed)
	GL_NEAREST_MIPMAP_NEAREST
	GL_LINEAR_MIPMAP_NEAREST
	GL_NEAREST_MIPMAP_LINEAR
	GL_LINEAR_MIPMAP_LINEAR
	
	Nearby textures have only the first two;
	distant textures have the additional four, which feature mipmapping.
	
	The resolutions are how much to shrink the textures before using them,
	in order to save VRAM space; they are, in order
	x1
	x1/2
	x1/4
	
	The color depths indicate what numbers of bits for each color channel:
	
	32-bit (8888)
	16-bit (5551)
	8-bit  (2222)
*/

struct OGL_Texture_Configure
{
	byte NearFilter;
	byte FarFilter;
	byte Resolution;
	byte ColorFormat;
};

// Here are some control flags
enum
{
	OGL_Flag_ZBuffer	= 0x0001,	// Whether to use a Z-buffer
	OGL_Flag_VoidColor	= 0x0002,	// Whether to color the void
	OGL_Flag_FlatLand	= 0x0004,	// Whether to use flat-textured landscapes
	OGL_Flag_Fog		= 0x0008,	// Whether to make fog
	OGL_Flag_SnglPass	= 0x0010,	// Whether to do two textures in one rendering pass
	OGL_Flag_2DGraphics	= 0x0020,	// Whether to pipe 2D graphics through OpenGL
	OGL_Flag_FlatStatic	= 0x0040,	// Whether to make the "static" effect look flat
	OGL_Flag_Fader		= 0x0080,	// Whether to do the fader effects in OpenGL
	OGL_Flag_LiqSeeThru	= 0x0100,	// Whether the liquids can be seen through
	OGL_Flag_Map		= 0x0200,	// Whether to do the overhead map with OpenGL
};

struct OGL_ConfigureData
{
	// Configure textures
	OGL_Texture_Configure TxtrConfigList[OGL_NUMBER_OF_TEXTURE_TYPES];

	// Overall rendering flags
	short Flags;
	
	// Color of the Void
	RGBColor VoidColor;
	
	// Landscape Flat Colors
	// First index: which landscape
	// (day, night, moon, outer space)
	// Second index: ground, sky
	RGBColor LscpColors[4][2];
	
	// Fog Features
	RGBColor FogColor;
	// In internal units
	long FogDepth;
};

OGL_ConfigureData& Get_OGL_ConfigureData();

// The OpenGL-configuration dialog box; returns whether its changes had been selected
bool OGL_ConfigureDialog(OGL_ConfigureData& Data);

// Set defaults
void OGL_SetDefaults(OGL_ConfigureData& Data);


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


// Here are the texture-opacity types.
// Opacity is the value of the alpha channel
enum
{
	OGL_OpacType_Crisp,		// The default: crisp edges, complete opacity
	OGL_OpacType_Flat,		// Fuzzy edges, but with flat opacity
	OGL_OpacType_Avg,		// Fuzzy edges, and opacity = max(color channel values)
	OGL_OpacType_Max,		// Fuzzy edges, and opacity = average(color channel values)
	OGL_NUMBER_OF_OPACITY_TYPES
};


struct OGL_TextureOptions
{
	short OpacityType;		// Which type of opacity to use?
	float OpacityScale;		// How much to scale the opacity
	float OpacityShift;		// How much to shift the opacity
	bool VoidVisible;		// Can see the void through texture if semitransparent
	
	OGL_TextureOptions(): OpacityType(OGL_OpacType_Crisp), OpacityScale(1), OpacityShift(0),
		VoidVisible(false) {}
};


// Get the texture options that are currently set
OGL_TextureOptions *OGL_GetTextureOptions(short Collection, short CLUT, short Bitmap);


// Reset the textures (good if they start to crap out)
void OGL_ResetTextures();


// XML support:
XML_ElementParser *OpenGL_GetParser();

#endif
