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
	
	OpenGL Texture Manager,
	by Loren Petrich,
	March 12, 2000

	This contains functions for handling the texture management for OpenGL.

Nov 18, 2000 (Loren Petrich):
	Added support for landscape vertical repeats

May 3, 2003 (Br'fin (Jeremy Parsons))
	Added LowLevelShape workaround for passing LowLevelShape info of sprites
	instead of abusing/overflowing shape_descriptors
*/

#ifndef _OGL_TEXTURES
#define _OGL_TEXTURES

#include "OGL_Headers.h"
#include "OGL_Subst_Texture_Def.h"
#include "scottish_textures.h"

// Initialize the texture accounting
void OGL_StartTextures();

// Done with the texture accounting
void OGL_StopTextures();

// Call this after every frame for housekeeping stuff
void OGL_FrameTickTextures();

// State of an individual texture set:
struct TextureState
{
	// Which member textures?
	enum
	{
		Normal,		// Used for all normally-shaded and shadeless textures
		Glowing,	// Used for self-luminous textures
		Bump,	    // Bump map for textures
		NUMBER_OF_TEXTURES
	};
	GLuint IDs[NUMBER_OF_TEXTURES];		// Texture ID's
	bool IsUsed;						// Is the texture set being used?
	bool IsGlowing;						// Does the texture have a glow map?
	bool IsBumped;						// Does the texture have a bump map?
	bool TexGened[NUMBER_OF_TEXTURES];	// Which ID's have had their textures generated?
	int IDUsage[NUMBER_OF_TEXTURES];	// Which ID's are being used?  Reset every frame.
	int unusedFrames;					// How many frames have passed since we were last used.
	short TextureType;
    
    GLdouble U_Scale;
    GLdouble V_Scale;
    GLdouble U_Offset;
    GLdouble V_Offset;
	
	TextureState() {IsUsed = false; Reset(); TextureType = NONE; U_Scale = V_Scale = 1; U_Offset = V_Offset = 0;}
	~TextureState() {Reset();}
	
	// Allocate some textures and indicate whether an allocation had happened.
	bool Allocate(short txType);
	
	// These indicate that some texture
	bool Use(int Which);
	bool UseNormal() {return Use(Normal);}
	bool UseGlowing() {return Use(Glowing);}
	bool UseBump() {return Use(Bump);}
	
	void FrameTick();
	
	// Reset the texture to unused and force a reload if necessary
	void Reset();
};


/*
	This is the collected texture states per collection bitmap;
	it contains both texture-mapping info and the state for each color-table value.

	The GLdouble stuff is for setting up offsets of the texture coordinates, using:
	Texture Coordinate = (offset) + (scale) * (clip position)
	where (scale) = (sprite size) / (texture size)
	and (clip position) = ((clip screen position) - (left screen position)) /
		((right screen position) - (left screen position))	
*/
struct CollBitmapTextureState
{
	TextureState CTStates[NUMBER_OF_OPENGL_BITMAP_SETS];
	
	// Sensible default
	CollBitmapTextureState() {}
};


// Modify color-table index if necessary;
// makes it the infravision or silhouette one if necessary
short ModifyCLUT(short TransferMode, short CLUT);


// Intended to be the no-texture default argument
const int BadTextureType = 32767;

/*
	Texture-manager object: contains all the handling of textures;
	it finds their setting up, their dimensions, and so forth.
	
	It is designed to be created and destroyed
	once each rendering go-around with each texture.
*/

class TextureManager
{	
	// Width: along scanlines; height; from scanline to scanline
	// Wall textures and sprites are horizontal-vertical flipped

	// Various texture ID's:
	short Collection;
	short CTable;
	short Frame;
	short Bitmap;

	// Info transmitted from the setting-up phase
	bool IsGlowing;
			
	// Width and height and whether to do RLE
	// These are, in order, for the original texture, for the OpenGL texture,
	// and the offsets in the original in the OpenGL texture.
	// Also, the loaded-texture dimensions, in case the textures get shrunk.
	short BaseTxtrWidth, BaseTxtrHeight;
	short TxtrWidth, TxtrHeight;
	short WidthOffset, HeightOffset;
	short LoadedWidth, LoadedHeight;
	
	// Color table: need it for going from indexed to true color;
	// both normal and glow-mapped versions are needed
	uint32 NormalColorTable[MAXIMUM_SHADING_TABLE_INDEXES];
	uint32 GlowColorTable[MAXIMUM_SHADING_TABLE_INDEXES];
	
	// Texture buffers for OpenGL
	uint32 *NormalBuffer, *GlowBuffer;

	// New texture buffers
	ImageDescriptorManager NormalImage, GlowImage, OffsetImage;
	
	// Pointer to the appropriate texture-state object
	TextureState *TxtrStatePtr;
	
	// Pointer to texture-options object
	OGL_TextureOptions *TxtrOptsPtr;
		
	// Private methods
	
	// This one tries to load substitute textures into NormalBuffer and GlowBuffer;
	// it returns whether such textures were loaded.
	bool LoadSubstituteTexture();
	
	// This one finds the width, height, etc. of a texture type;
	// it returns "false" if some texture's dimensions do not fit.
	bool SetupTextureGeometry();
	
	// This one finds the color tables
	void FindColorTables();

	// premultiplies color table alpha
	void PremultiplyColorTables();
	
	// This one allocates an OpenGL texture buffer and uses a color table
	uint32 *GetOGLTexture(uint32 *ColorTable);
	
	// This one creates a fake landscape
	uint32 *GetFakeLandscape();
	
	// This is for shrinking a texture
	uint32 *Shrink(uint32 *Buffer);
	
	// This is for placing a texture in OpenGL
	void PlaceTexture(const ImageDescriptor *, bool normal_map = false);

public:

	// Inputs: get off of texture object passed to scottish_textures.
	shape_descriptor ShapeDesc;
	uint16 LowLevelShape;
	bitmap_definition *Texture;
	void *ShadingTables;
	short TransferMode;
	short TransferData;
	bool IsShadeless;
	short TextureType;
	bool LandscapeVertRepeat;

	bool FastPath;
	
	// The width of a landscape texture will be 2^(-Landscape_AspRatExp) * (the height)
	short Landscape_AspRatExp;
	
	// Sets up all the texture stuff:	
	bool Setup();
	
	// Results:
	
	// Various texture ID's:
	short GetCollection() {return Collection;}
	short GetCTable() {return CTable;}
	short GetFrame() {return Frame;}
	short GetBitmap() {return Bitmap;}

	// Info transmitted from the setting-up phase:
	// texture type, whether it is glowmapped,
	// and whether the textures are blended rather than all-or-nothing crisp-edged
	int GetTextureType() {return TextureType;}	
	bool IsGlowMapped() {return IsGlowing;}
	bool IsBlended() {return (TxtrOptsPtr->OpacityType != OGL_OpacType_Crisp);}
	bool VoidVisible() {return (TxtrOptsPtr->VoidVisible);}
	short NormalBlend() {return (TxtrOptsPtr->NormalBlend) + ((NormalImage.get() && NormalImage.get()->IsPremultiplied() && TxtrOptsPtr->NormalBlend < OGL_FIRST_PREMULT_ALPHA) ? OGL_FIRST_PREMULT_ALPHA : 0); }
	short GlowBlend() {return (TxtrOptsPtr->GlowBlend) + ((GlowImage.get() && GlowImage.get()->IsPremultiplied() && TxtrOptsPtr->GlowBlend < OGL_FIRST_PREMULT_ALPHA) ? OGL_FIRST_PREMULT_ALPHA : 0); }
	float MinGlowIntensity() {return (TxtrOptsPtr->MinGlowIntensity);}
	float BloomScale() {return (TxtrOptsPtr->BloomScale);}
	float BloomShift() {return (TxtrOptsPtr->BloomShift);}
	float GlowBloomScale() {return (TxtrOptsPtr->GlowBloomScale);}
	float GlowBloomShift() {return (TxtrOptsPtr->GlowBloomShift);}
	float LandscapeBloom() {return (TxtrOptsPtr->LandscapeBloom);}
	
	// Scaling and offset of the current texture;
	// important for sprites, which will be padded to make them OpenGL-friendly.
	GLdouble U_Scale, V_Scale, U_Offset, V_Offset;
	
	// What to render:
	
	// Always call this one and call it first; safe to allocate texture ID's in it
	void RenderNormal();
	// Call this one after RenderNormal()
	void RenderGlowing();
	void RenderBump();

	void SetupTextureMatrix();
	void RestoreTextureMatrix();
	
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator= (const TextureManager&) = delete;
	
	TextureManager();
	~TextureManager();
};


// Color-transformation macros:

// Five-to-eight translation:
// takes a 5-bit color value and expands it to 8 bytes
inline byte FiveToEight(byte x) {return (x << 3) | ((x >> 2) & 0x07);}

// 16-bit-to-32-bit with opacity = 1;
// ARGB 1555 to RGBA 8888
inline GLuint Convert_16to32(uint16 InPxl)
{
	if (PlatformIsLittleEndian()) {
		// perfect target for constexpr-if in C++17
		// Alpha preset
		GLuint OutPxl = 0xff000000;
		GLuint Chan;
		// Red
		Chan = FiveToEight(InPxl >> 10);
		OutPxl |= Chan;
		// Green
		Chan = FiveToEight(InPxl >> 5);
		OutPxl |= Chan << 8;
		// Blue
		Chan = FiveToEight(InPxl & 0x1F);
		OutPxl |= Chan << 16;
		return OutPxl;
	} else {
		// Alpha preset
		GLuint OutPxl = 0x000000ff;
		GLuint Chan;
		// Red
		Chan = FiveToEight(InPxl >> 10);
		OutPxl |= Chan << 24;
		// Green
		Chan = FiveToEight(InPxl >> 5);
		OutPxl |= Chan << 16;
		// Blue
		Chan = FiveToEight(InPxl);
		OutPxl |= Chan << 8;
		return OutPxl;
	}
	
}


// Make floating-point colors
inline void MakeFloatColor(RGBColor& InColor, GLfloat *OutColor)
{
	OutColor[0] = InColor.red/65535.0F;
	OutColor[1] = InColor.green/65535.0F;
	OutColor[2] = InColor.blue/65535.0F;
}

inline void MakeFloatColor(rgb_color& InColor, GLfloat *OutColor)
{
	OutColor[0] = InColor.red/65535.0F;
	OutColor[1] = InColor.green/65535.0F;
	OutColor[2] = InColor.blue/65535.0F;
}

/*
// Stuff for doing 16->32 pixel-format conversion, 1555 ARGB to 8888 RGBA
// The table is created only when necessary, and retained as long as it is necessary
extern GLuint *ConversionTable_16to32;

extern void MakeConversion_16to32(int BitDepth);
*/

void LoadModelSkin(ImageDescriptor& Image, short Collection, short CLUT);

void SetPixelOpacities(OGL_TextureOptions& Options, ImageDescriptorManager &imageManager);

// Does this for a set of several pixel values or color-table values;
// the pixels are assumed to be in OpenGL-friendly byte-by-byte RGBA format.
void SetPixelOpacitiesRGBA(OGL_TextureOptions& Options, int NumPixels, uint32 *Pixels);

// Infravision (I'm blue, are you?)
bool& IsInfravisionActive();

// Sets the infravision tinting color for a shapes collection, and whether to use such tinting;
// the color values are from 0 to 1.
bool SetInfravisionTint(short Collection, bool IsTinted, float Red, float Green, float Blue);

// Finds the infravision version of a color;
// it makes no change if infravision is inactive.
void FindInfravisionVersionRGBA(short Collection, GLfloat *Color);

void FindSilhouetteVersion(ImageDescriptorManager &imageManager);

struct OGL_TexturesStats {
	int inUse;
	int binds, totalBind, minBind, maxBind;
	int longNormalSetups, longGlowSetups, longBumpSetups;
	int totalAge;
};

extern OGL_TexturesStats gGLTxStats;

#endif
