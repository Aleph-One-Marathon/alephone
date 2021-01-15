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

	This implements texture handling for OpenGL.
	
	May 2, 2000:
	
	Fixed silhouette-texture bug: color 0 is now transparent
	
	May 24, 2000:
	
	Added support for setting landscape aspect ratios from outside;
	also added more graceful degradation for mis-sized textures.
	Walls must be a power of 2 horizontally and vertical;
	landscapes must be a power of 2 horizontally
	in order for the tiling to work properly.
	
	June 11, 2000:
	
	Added support for opacity shift factor (OpacityShift alongside OpacityScale);
	should be good for making dark colors somewhat opaque.

Jul 10, 2000:

	Fixed crashing bug when OpenGL is inactive with ResetTextures()

Sep 9, 2000:

	Restored old fix for AppleGL texturing as an option; this fix consists of setting
	the minimum size of a texture to be 128.

Nov 12, 2000 (Loren Petrich):
	Cleaned up some of the code to avoid explicit endianness usage;
	also implemented texture substitution.

Nov 18, 2000 (Loren Petrich):
	Added support for landscape vertical repeats;
	also added support for glow mapping of wall textures

Dec 16, 2000 (Loren Petrich):
	Fixed substitution of landscape textures

June 14, 2001 (Loren Petrich):
	Changed Width*Height to TxtrWidth*TxtrHeight in some places to ensure that some operations
	are done over complete textures

Nov 30, 2001 (Alexander Strange):
	Added Ian Rickard's texture purging to save VRAM.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for AGL.h

May 3, 2003 (Br'fin (Jeremy Parsons))
	Added LowLevelShape workaround for passing LowLevelShape info of sprites
	instead of abusing/overflowing shape_descriptors
*/

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <list>

#include "cseries.h"

#ifdef HAVE_OPENGL

#include "OGL_Headers.h"

#include "preferences.h"

#include "SDL.h"
#include "SDL_endian.h"
#include "interface.h"
#include "render.h"
#include "map.h"
#include "collection_definition.h"
#include "OGL_Blitter.h"
#include "OGL_Setup.h"
#include "OGL_Render.h"
#include "OGL_Textures.h"
#include "screen.h"

using std::min;
using std::max;

OGL_TexturesStats gGLTxStats = {0,0,0,500000,0,0, 0};

// Texture mapping
struct TxtrTypeInfoData
{
	GLenum NearFilter;			// OpenGL parameter for near filter (GL_NEAREST, etc.)
	GLenum FarFilter;			// OpenGL parameter for far filter (GL_NEAREST, etc.)
	int Resolution;				// 0 is full-sized, 1 is half-sized, 2 is fourth-sized
	GLenum ColorFormat;			// OpenGL parameter for stored color format (RGBA8, etc.)
};


static TxtrTypeInfoData TxtrTypeInfoList[OGL_NUMBER_OF_TEXTURE_TYPES];
static TxtrTypeInfoData ModelSkinInfo;

static bool useSGISMipmaps = false;
static bool useMirroredRepeat = false;

// Infravision: use algorithm (red + green + blue)/3 to compose intensity,
// then shade with these colors, one color for each collection.

struct InfravisionData
{
	GLfloat Red, Green, Blue;	// Infravision tint components: 0 to 1
	bool IsTinted;				// whether to use infravision with this collection
};

struct InfravisionData IVDataList[NUMBER_OF_COLLECTIONS] =
{
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false},
	{1,1,1,false}
};

// Is infravision currently active?
static bool InfravisionActive = false;

static std::list<TextureState*> sgActiveTextureStates;


// Allocate some textures and indicate whether an allocation had happened.
bool TextureState::Allocate(short txType)
{
	TextureType = txType;
	if (!IsUsed)
	{
		sgActiveTextureStates.push_front(this);
		gGLTxStats.inUse++;
		glGenTextures(NUMBER_OF_TEXTURES,IDs);
		IsUsed = true;
		unusedFrames=0;
		return true;
	}
	return false;
}

// Use a texture and indicate whether to load it
bool TextureState::Use(int Which)
{
	glBindTexture(GL_TEXTURE_2D,IDs[Which]);
	bool result = !TexGened[Which];
	TexGened[Which] = true;
	IDUsage[Which]++;
	return result;
}


// Resets the object's texture state
void TextureState::Reset()
{
	if (IsUsed)
	{
		sgActiveTextureStates.remove(this);
		gGLTxStats.inUse--;
		glDeleteTextures(NUMBER_OF_TEXTURES,IDs);
	}
	IsUsed = IsGlowing = IsBumped = TexGened[Normal] = TexGened[Glowing] = TexGened[Bump] = false;
	IDUsage[Normal] = IDUsage[Glowing] = IDUsage[Bump] = unusedFrames = 0;
}

void TextureState::FrameTick() {
	if (!IsUsed) return;
	
	gGLTxStats.totalAge += (TextureType!=OGL_Txtr_Landscape)?unusedFrames:0;
	
	bool used  = false;
	
	for (int i=0 ; i<NUMBER_OF_TEXTURES ; i++)
		if (IDUsage[i] != 0) used = true;
	
	if (used) {
		IDUsage[Normal] = IDUsage[Glowing] = unusedFrames = 0;
		
	} else {
		unusedFrames++;
		assert(TextureType != NONE);
		switch (TextureType) {
		case OGL_Txtr_Wall:
				if (unusedFrames > 300) Reset(); // at least 10 seconds till wall textures are released
				break;
		case OGL_Txtr_Landscape:
				// never release landscapes
				break;
		case OGL_Txtr_Inhabitant:
				if (unusedFrames > 450) Reset(); // release unused sprites in 15 seconds
				break;
		case OGL_Txtr_WeaponsInHand:
				if (unusedFrames > 600) Reset(); // release weapons in hand in 20 seconds
				break;
		}
	}
}

// Will distinguish by texture type as well as by collection;
// this is because different rendering modes deserve different treatment.
static CollBitmapTextureState* TextureStateSets[OGL_NUMBER_OF_TEXTURE_TYPES][MAXIMUM_COLLECTIONS];

static GLuint flatBumpTextureID = 0;
void FlatBumpTexture() {
	
	if (flatBumpTextureID == 0)
	{
		glGenTextures(1, &flatBumpTextureID);
		glBindTexture(GL_TEXTURE_2D, flatBumpTextureID);
		
		GLubyte flatTextureData[4] = {0x80, 0x80, 0xFF, 0x80};
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, flatTextureData);
	}
	else
		glBindTexture(GL_TEXTURE_2D, flatBumpTextureID);
}


// Initialize the texture accounting
void OGL_StartTextures()
{
	// Initialize the texture accounting proper
	for (int it=0; it<OGL_NUMBER_OF_TEXTURE_TYPES; it++)
		for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
		{
			bool CollectionPresent = is_collection_present(ic);
			short NumberOfBitmaps =
				CollectionPresent ? get_number_of_collection_bitmaps(ic) : 0;
			TextureStateSets[it][ic] =
				(CollectionPresent && NumberOfBitmaps) ?
					(new CollBitmapTextureState[NumberOfBitmaps]) : 0;
		}
	
	// Initialize the texture-type info
	const int NUMBER_OF_NEAR_FILTERS = 2;
	const GLenum NearFilterList[NUMBER_OF_NEAR_FILTERS] =
	{
		GL_NEAREST,
		GL_LINEAR
	};
	const int NUMBER_OF_FAR_FILTERS = 6;
	const GLenum FarFilterList[NUMBER_OF_FAR_FILTERS] =
	{
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};
	const int NUMBER_OF_COLOR_FORMATS = 3;
	const GLenum ColorFormatList[NUMBER_OF_COLOR_FORMATS] = 
	{
		GL_RGBA8,
		GL_RGBA4,
		GL_RGBA2
	};
	
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	
	for (int k=0; k<OGL_NUMBER_OF_TEXTURE_TYPES; k++)
	{
		OGL_Texture_Configure& TxtrConfigure = ConfigureData.TxtrConfigList[k];
		TxtrTypeInfoData& TxtrTypeInfo = TxtrTypeInfoList[k];
		
		short NearFilter = TxtrConfigure.NearFilter;
		if (NearFilter < NUMBER_OF_NEAR_FILTERS)
			TxtrTypeInfo.NearFilter = NearFilterList[NearFilter];
		else
			TxtrTypeInfo.NearFilter = GL_NEAREST;
		
		short FarFilter = TxtrConfigure.FarFilter;
		if (FarFilter < NUMBER_OF_FAR_FILTERS)
			TxtrTypeInfo.FarFilter = FarFilterList[FarFilter];
		else
			TxtrTypeInfo.FarFilter = GL_NEAREST;
		
		TxtrTypeInfo.Resolution = TxtrConfigure.Resolution;
		
		short ColorFormat = TxtrConfigure.ColorFormat;
		if (ColorFormat < NUMBER_OF_COLOR_FORMATS)
			TxtrTypeInfo.ColorFormat = ColorFormatList[ColorFormat];
		else
			TxtrTypeInfo.ColorFormat = GL_RGBA8;
	}

	// Model skin
	{
		OGL_Texture_Configure& TxtrConfigure = ConfigureData.ModelConfig;
		TxtrTypeInfoData& TxtrTypeInfo = ModelSkinInfo;
		
		short NearFilter = TxtrConfigure.NearFilter;
		if (NearFilter < NUMBER_OF_NEAR_FILTERS)
			TxtrTypeInfo.NearFilter = NearFilterList[NearFilter];
		else
			TxtrTypeInfo.NearFilter = GL_NEAREST;
		
		short FarFilter = TxtrConfigure.FarFilter;
		if (FarFilter < NUMBER_OF_FAR_FILTERS)
			TxtrTypeInfo.FarFilter = FarFilterList[FarFilter];
		else
			TxtrTypeInfo.FarFilter = GL_NEAREST;
		
		TxtrTypeInfo.Resolution = TxtrConfigure.Resolution;
		
		short ColorFormat = TxtrConfigure.ColorFormat;
		if (ColorFormat < NUMBER_OF_COLOR_FORMATS)
			TxtrTypeInfo.ColorFormat = ColorFormatList[ColorFormat];
		else
			TxtrTypeInfo.ColorFormat = GL_RGBA8;
	}

#if defined GL_SGIS_generate_mipmap
	useSGISMipmaps = OGL_CheckExtension("GL_SGIS_generate_mipmap");
#endif
#if defined GL_ARB_texture_mirrored_repeat
	useMirroredRepeat = OGL_CheckExtension("GL_ARB_texture_mirrored_repeat");
#endif
}


// Done with the texture accounting
void OGL_StopTextures()
{
	// Clear the texture accounting
	for (int it=0; it<OGL_NUMBER_OF_TEXTURE_TYPES; it++)
		for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
			if (TextureStateSets[it][ic]) delete []TextureStateSets[it][ic];

	// clear blitters and fonts
	OGL_Blitter::StopTextures();
	FontSpecifier::OGL_ResetFonts(false);
	
	glDeleteTextures(1, &flatBumpTextureID);
	flatBumpTextureID = 0;
    
    // clear leftover infravision
    InfravisionActive = false;
}

void OGL_FrameTickTextures()
{
	std::list<TextureState*>::iterator i;
	
	for (i=sgActiveTextureStates.begin() ; i!= sgActiveTextureStates.end() ; i++) {
		(*i)->FrameTick();
	}
}

// Find an OpenGL-friendly color table from a Marathon shading table
static void FindOGLColorTable(int NumSrcBytes, byte *OrigColorTable, uint32 *ColorTable)
{
	// Stretch the original color table to 4 bytes per value for OpenGL convenience;
	// all the intermediate calculations will be done in RGBA 8888 form,
	// because that is what OpenGL prefers as a texture input
	switch(NumSrcBytes) {
	case 2:
		for (int k=0; k<MAXIMUM_SHADING_TABLE_INDEXES; k++)
		{
			byte *OrigPtr = OrigColorTable + NumSrcBytes*k;
			uint32 &Color = ColorTable[k];
			
			// Convert from ARGB 5551 to RGBA 8888; make opaque
			uint16 Intmd;
			uint8 *IntmdPtr = (uint8 *)(&Intmd);
			IntmdPtr[0] = OrigPtr[0];
			IntmdPtr[1] = OrigPtr[1];
			Color = Convert_16to32(Intmd);
		}
		break;
		
	case 4:
		for (int k=0; k<MAXIMUM_SHADING_TABLE_INDEXES; k++)
		{
			byte *OrigPtr = OrigColorTable + NumSrcBytes*k;
			uint32 &Color = ColorTable[k];
			
			// Convert from ARGB 8888 to RGBA 8888; make opaque
			uint8 *ColorPtr = (uint8 *)(&Color);
			if (PlatformIsLittleEndian()) {
				// the compiler will do the right thing and only emit
				// code for the correct path. In C++17 we can do constexpr if
				// to make that requirement explicit.
				ColorPtr[0] = OrigPtr[2];
				ColorPtr[1] = OrigPtr[1];
				ColorPtr[2] = OrigPtr[0];
				ColorPtr[3] = 0xff;
			} else {
				ColorPtr[0] = OrigPtr[1];
				ColorPtr[1] = OrigPtr[2];
				ColorPtr[2] = OrigPtr[3];
				ColorPtr[3] = 0xff;
			}
		}
		break;
	}
}


inline bool IsLandscapeFlatColored()
{
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	return TEST_FLAG(ConfigureData.Flags,OGL_Flag_FlatLand);
}


// Modify color-table index if necessary;
// makes it the infravision or silhouette one if necessary
short ModifyCLUT(short TransferMode, short CLUT)
{
	short CTable;
	
	// Tinted mode is only used for invisibility, and infravision will make objects visible
	if (TransferMode == _static_transfer) CTable = SILHOUETTE_BITMAP_CLUTSPECIFIC + CLUT;
	else if (TransferMode == _tinted_transfer) CTable = SILHOUETTE_BITMAP_CLUTSPECIFIC + CLUT;
	else if (InfravisionActive) CTable = INFRAVISION_BITMAP_CLUTSPECIFIC + CLUT;
	else CTable = CLUT;
	
	return CTable;
}

/*
	Routine for using some texture; it will load the texture if necessary.
	It parses a shape descriptor and checks on whether the collection's texture type
	is one of those given.
	It will check for more than one intended texture type,
	a convenience for multiple texture types sharing the same handling.
	
	It uses the transfer mode and the transfer data to work out
	what transfer modes to use (invisibility is a special case of tinted)
*/
bool TextureManager::Setup()
{

	// Parse the shape descriptor and check on whether the texture type
	// is the texture's intended type
	short CollColor = GET_DESCRIPTOR_COLLECTION(ShapeDesc);
	Collection = GET_COLLECTION(CollColor);
	CTable = ModifyCLUT(TransferMode,GET_COLLECTION_CLUT(CollColor));
	Frame = (LowLevelShape)? LowLevelShape : GET_DESCRIPTOR_SHAPE(ShapeDesc);
	Bitmap = get_bitmap_index(Collection,Frame);
	if (Bitmap == NONE) return false;
	
	// Get the texture-state info: first, per-collection, then per-bitmap
	CollBitmapTextureState *CBTSList = TextureStateSets[TextureType][Collection];
	if (CBTSList == NULL) return false;
	CollBitmapTextureState& CBTS = CBTSList[Bitmap];
	
	// Get the control info for this texture type:
	TxtrTypeInfoData& TxtrTypeInfo = TxtrTypeInfoList[TextureType];
	
	// Get the rendering options for this texture:
	TxtrOptsPtr = OGL_GetTextureOptions(Collection,CTable,Bitmap);
	
	// Get the texture-state info: per-color-table -- be sure to preserve this for later
	// Set the texture ID, and load the texture if necessary
	// If "Use()" is true, then load, otherwise, assume the texture is loaded and skip
	TxtrStatePtr = &CBTS.CTStates[CTable];
	TextureState &CTState = *TxtrStatePtr;
	if (!CTState.IsUsed)
	{
		// Initial sprite scale/offset
		U_Scale = V_Scale = 1;
		U_Offset = V_Offset = 0;
		
		// Try to load a substitute texture, and if that fails,
		// get the geometry from the shapes bitmap.
		bool substitute = LoadSubstituteTexture();
		if (!substitute) 
			if (!SetupTextureGeometry()) return false;

		// Store sprite scale/offset
		CTState.U_Scale = U_Scale;
		CTState.V_Scale = V_Scale;
		CTState.U_Offset = U_Offset;
		CTState.V_Offset = V_Offset;

		// This finding of color tables sets the glow state
		if (!substitute) 
			FindColorTables();
		else if (GlowImage.get() && GlowImage.get()->IsPresent()) {
			// Override if textures had been substituted;
			// if the normal texture had been substituted, it will be assumed to be
			// non-glowing unless the glow texture has also been substituted.
			
			IsGlowing = true;
		} else {
			IsGlowing = false;
		}
		
		CTState.IsGlowing = IsGlowing;
		
		if (substitute && OffsetImage.get() && OffsetImage.get()->IsPresent()) {
			CTState.IsBumped = true;
		} else {
			CTState.IsBumped = false;
		}
		
		// Load the fake landscape if selected
		if (TextureType == OGL_Txtr_Landscape)
		{
			if (IsLandscapeFlatColored())
			{
				NormalImage.edit(new ImageDescriptor(TxtrWidth, TxtrHeight, GetFakeLandscape()));
			}
		}
		
		// If not, then load the expected textures
		if (!NormalImage.get() || !NormalImage.get()->IsPresent())
			NormalImage.edit(new ImageDescriptor(TxtrWidth, TxtrHeight, GetOGLTexture(NormalColorTable)));	
		if (IsGlowing && (!GlowImage.get() || !GlowImage.get()->IsPresent())) 
			GlowImage.edit(new ImageDescriptor(TxtrWidth, TxtrHeight, GetOGLTexture(GlowColorTable)));
		
		// Display size: may be shrunk
		int MaxWidth = MAX(TxtrWidth >> TxtrTypeInfo.Resolution, 1);
		int MaxHeight = MAX(TxtrHeight >> TxtrTypeInfo.Resolution, 1);
		
		// Fit the image into the maximum size allowed by the OpenGL implementation in use
		GLint MaxTextureSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTextureSize);
		while (MaxWidth > MaxTextureSize || MaxHeight > MaxTextureSize)
		{
			LoadedWidth >>= 1;
			LoadedHeight >>= 1;
		}

		while (NormalImage.get()->GetWidth() > MaxWidth || NormalImage.get()->GetHeight() > MaxHeight)
		{
			if (!NormalImage.edit()->Minify()) break;
			if (GlowImage.get() && GlowImage.get()->IsPresent()) {
				if (!GlowImage.edit()->Minify()) break;
			}
			if (OffsetImage.get() && OffsetImage.get()->IsPresent()) {
				if (!OffsetImage.edit()->Minify()) break;
			}			
		}
		
		// Kludge for making top and bottom look flat
		/*
		if (TextureType == OGL_Txtr_Landscape)
		{
			MakeAverage(LoadedWidth,NormalBuffer);
			MakeAverage(LoadedWidth,NormalBuffer+LoadedWidth*(LoadedHeight-1));
		}
		*/
	}
	else
	{
		// Get sprite scale/offset
		U_Scale = CTState.U_Scale;
		V_Scale = CTState.V_Scale;
		U_Offset = CTState.U_Offset;
		V_Offset = CTState.V_Offset;
		
		// Get glow state
		IsGlowing = CTState.IsGlowing;

		// Populate NormalImage, GlowImage, OffsetImage
		NormalImage.set(&TxtrOptsPtr->NormalImg);
		GlowImage.set(&TxtrOptsPtr->GlowImg);
		OffsetImage.set(&TxtrOptsPtr->OffsetImg);
	}
		
	// Done!!!
	return true;
}


inline bool WhetherTextureFix()
{
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	return TEST_FLAG(ConfigureData.Flags,OGL_Flag_TextureFix);
}


// Conversion of color data types

inline int MakeEightBit(GLfloat Chan)
{
	return int(PIN(int(255*Chan+0.5),0,255));
}

uint32 MakeIntColor(GLfloat *FloatColor)
{
	uint32 IntColor;
	uint8 *ColorPtr = (uint8 *)(&IntColor);
	for (int k=0; k<4; k++)
		ColorPtr[k] = MakeEightBit(FloatColor[k]);
	return IntColor;
}

void MakeFloatColor(uint32 IntColor, GLfloat *FloatColor)
{
	uint8 *ColorPtr = (uint8 *)(&IntColor);
	for (int k=0; k<4; k++)
		FloatColor[k] = float(ColorPtr[k])/float(255);
}

bool TextureManager::LoadSubstituteTexture()
{
	// don't load replacements for bitmaps that have been patched
	if (Texture->flags & _PATCHED_BIT) return false;

	// Is there a texture to be substituted?
	ImageDescriptor& NormalImg = TxtrOptsPtr->NormalImg;
	if (!NormalImg.IsPresent()) return false;
	
	// Idiot-proofing
	if (NormalBuffer)
	{
		delete []NormalBuffer;
		NormalBuffer = NULL;
	}
	if (GlowBuffer)
	{
		delete []GlowBuffer;
		GlowBuffer = NULL;
	}

	NormalImage.set(&TxtrOptsPtr->NormalImg);
	GlowImage.set(&TxtrOptsPtr->GlowImg);
	OffsetImage.set(&TxtrOptsPtr->OffsetImg);

	int Width = NormalImg.GetWidth();
	int Height = NormalImg.GetHeight();
	
	switch(TextureType)
	{
	case OGL_Txtr_Wall:
		// For tiling to be possible, the width and height must be powers of 2;
		// also, be sure to transpose the texture
		TxtrHeight = Height;
		TxtrWidth = Width;
		if (!npotTextures) 
		{
			if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
			if (TxtrHeight != NextPowerOfTwo(TxtrHeight)) return false;
		}
		TxtrOptsPtr->Substitution = true;
		break;
	
	case OGL_Txtr_Landscape:
		// For tiling to be possible, the width must be a power of 2;
		// the height need not be such a power.
		TxtrWidth = Width;
		TxtrHeight = (Landscape_AspRatExp >= 0) ?
			(TxtrWidth >> Landscape_AspRatExp) :
			(TxtrWidth << (-Landscape_AspRatExp));
		if (!npotTextures && TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
		
		// the renderer doesn't use these,
		// so I'll use them to get the texture matrix set up right
		U_Scale = (float) TxtrHeight / NormalImg.GetHeight();
		U_Offset = -1.0 + ((TxtrHeight - NormalImg.GetHeight()) / 2.0 / TxtrHeight) + (1.0 - NormalImg.GetUScale()) / 2.0;
		
		TxtrOptsPtr->Substitution = true;
		
		GlowImage.set((ImageDescriptor *) NULL);
		break;
		
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_WeaponsInHand:
		// Much of the code here has been copied from elsewhere.
		// Set these for convenience; sprites are transposed, as walls are.
		TxtrHeight = Height;
		TxtrWidth = Width;
		
		if (!npotTextures) 
		{
			// ImageLoader now stores these as powers of two sized
			if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
			if (TxtrHeight != NextPowerOfTwo(TxtrHeight)) return false;
		}
			
		// We can calculate the scales and offsets here
		V_Scale = NormalImg.GetVScale();
		V_Offset = 0;
		U_Scale = NormalImg.GetUScale();
		U_Offset = 0;

		TxtrOptsPtr->Substitution = true;
		break;
	}
	
	// Use the Tomb Raider opacity hack if selected
	SetPixelOpacities(*TxtrOptsPtr, NormalImage);
	
	// Modify if infravision is active
	if (IsInfravisionTable(CTable))
	{
		// Infravision textures don't glow
		GlowImage.set((ImageDescriptor *) NULL);
		
		// FIXME: bump maps don't load properly under infravision
		OffsetImage.set((ImageDescriptor *) NULL);
	}
	else if (IsSilhouetteTable(CTable))
	{
		FindSilhouetteVersion(NormalImage);
		GlowImage.set((ImageDescriptor *) NULL);
	}
	return true;
}

bool TextureManager::SetupTextureGeometry()
{	
	// How many rows (scanlines) and columns
	if (Texture->flags&_COLUMN_ORDER_BIT)
	{
		BaseTxtrWidth = Texture->height;
		BaseTxtrHeight = Texture->width;
	}
	else
	{
		BaseTxtrWidth = Texture->width;
		BaseTxtrHeight = Texture->height;
	}
	
	short RowBytes = Texture->bytes_per_row;
	if (RowBytes != NONE)
		if (BaseTxtrWidth != RowBytes) return false;
	
	// The default
	WidthOffset = HeightOffset = 0;
	
	switch(TextureType)
	{
	case OGL_Txtr_Wall:
		// For tiling to be possible, the width and height must be powers of 2
		// Match M1 engine, and truncate larger textures to 128px square
		TxtrWidth = std::min(static_cast<int>(BaseTxtrWidth), 128);
		TxtrHeight = std::min(static_cast<int>(BaseTxtrHeight), 128);
		if (!npotTextures) 
		{
			if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
			if (TxtrHeight != NextPowerOfTwo(TxtrHeight)) return false;
		}
		break;
		
	case OGL_Txtr_Landscape:
		if (IsLandscapeFlatColored())
		{
			TxtrWidth = 128;
			TxtrHeight = 128;
		}
		else
		{
			// Width is horizontal direction here
			TxtrWidth = BaseTxtrWidth;
			if (!npotTextures && TxtrWidth != NextPowerOfTwo(TxtrWidth)) 
				return false;

			if (npotTextures) 
			{
				// Use the landscape height here
				TxtrHeight = (Landscape_AspRatExp >= 0) ?
					(TxtrWidth >> Landscape_AspRatExp) :
					(TxtrWidth << (-Landscape_AspRatExp));
				U_Scale = (double) TxtrHeight / BaseTxtrHeight;
				U_Offset =  -(TxtrHeight - BaseTxtrHeight) / 2.0 / TxtrHeight;
				TxtrHeight = BaseTxtrHeight;
			} 
			else
			{
				// Use the landscape height here
				TxtrHeight = (Landscape_AspRatExp >= 0) ?
					(TxtrWidth >> Landscape_AspRatExp) :
					(TxtrWidth << (-Landscape_AspRatExp));
				
				// Offsets
				WidthOffset = (TxtrWidth - BaseTxtrWidth) >> 1;
				HeightOffset = (TxtrHeight - BaseTxtrHeight) >> 1;
			}
		}
		
		break;
		
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_WeaponsInHand:
		{
			if (npotTextures) 
			{
				TxtrWidth = BaseTxtrWidth;
				TxtrHeight = BaseTxtrHeight;
			} 
			else 
			{
				// The 2 here is so that there will be an empty border around a sprite,
				// so that the texture can be conveniently mipmapped.
				TxtrWidth = NextPowerOfTwo(BaseTxtrWidth+2);
				TxtrHeight = NextPowerOfTwo(BaseTxtrHeight+2);
			
				// This kludge no longer necessary
				// Restored due to some people still having AppleGL 1.1.2
				if (WhetherTextureFix())
				{
					TxtrWidth = MAX(TxtrWidth,128);
					TxtrHeight = MAX(TxtrHeight,128);
				}
						
				// Offsets
				WidthOffset = (TxtrWidth - BaseTxtrWidth) >> 1;
				HeightOffset = (TxtrHeight - BaseTxtrHeight) >> 1;
			
				// We can calculate the scales and offsets here
				double TWidRecip = 1/double(TxtrWidth);
				double THtRecip = 1/double(TxtrHeight);
				U_Scale = TWidRecip*double(BaseTxtrWidth);
				U_Offset = TWidRecip*WidthOffset;
				V_Scale = THtRecip*double(BaseTxtrHeight);
				V_Offset = THtRecip*HeightOffset;
			}
		}
		break;
	}
	
	// Success!
	return true;
}


void TextureManager::FindColorTables()
{
	// Default
	IsGlowing = false;
	
	// The silhouette case is easy
	if (IsSilhouetteTable(CTable))
	{
		NormalColorTable[0] = 0;
		for (int k=1; k<MAXIMUM_SHADING_TABLE_INDEXES; k++)
			NormalColorTable[k] = 0xffffffff;
		return;
	}

	// Interface collection? Then use the CLUT directly
	if (Collection == 0) {
		int num_colors;
		struct rgb_color_value *q = get_collection_colors(0, 0, num_colors);
		uint8 *p = (uint8 *)NormalColorTable;
		for (int k=0; k<num_colors; k++) {
			int idx = q[k].value;
			p[idx * 4 + 0] = q[k].red >> 8;
			p[idx * 4 + 1] = q[k].green >> 8;
			p[idx * 4 + 2] = q[k].blue >> 8;
			p[idx * 4 + 3] = 0xff;
		}
		SetPixelOpacitiesRGBA(*TxtrOptsPtr, MAXIMUM_SHADING_TABLE_INDEXES, NormalColorTable);
		NormalColorTable[0] = 0;
		return;
	}
	
	// Number of source bytes, for reading off of the shading table
	// IR change: dithering
	short NumSrcBytes = bit_depth / 8;
	
	// Shadeless polygons use the first, instead of the last, shading table
	byte *OrigColorTable = (byte *)ShadingTables;
	byte *OrigGlowColorTable = OrigColorTable;
	if (IsInfravisionTable(CTable) || !IsShadeless) OrigColorTable +=
		NumSrcBytes*(number_of_shading_tables - 1)*MAXIMUM_SHADING_TABLE_INDEXES;
	
	// Find the normal color table,
	// and set its opacities as if there was no glow table.
	FindOGLColorTable(NumSrcBytes,OrigColorTable,NormalColorTable);
	SetPixelOpacitiesRGBA(*TxtrOptsPtr,MAXIMUM_SHADING_TABLE_INDEXES,NormalColorTable);
	
	// Find the glow-map color table;
	// only inhabitants are glowmapped.
	// Also, it seems that only infravision textures are shadeless.
	if (!IsShadeless && (TextureType != OGL_Txtr_Landscape))
	{
		// Find the glow table from the lowest-illumination color table
		FindOGLColorTable(NumSrcBytes,OrigGlowColorTable,GlowColorTable);
		
		// Search for self-luminous colors; ignore the first one as the transparent one
		for (int k=1; k<MAXIMUM_SHADING_TABLE_INDEXES; k++)
		{
			// Check for illumination-independent colors
			uint8 *NormalEntry = (uint8 *)(NormalColorTable + k);
			uint8 *GlowEntry = (uint8 *)(GlowColorTable + k);
			
			bool EntryIsGlowing = false;
			for (int q=0; q<3; q++)
				if (GlowEntry[q] >= 0x0f) EntryIsGlowing = true;
			
			// Make the glow color the original color, to get continuity
			for (int q=0; q<3; q++)
				GlowEntry[q] = NormalEntry[q];
			
			if (EntryIsGlowing && NormalEntry[3])
			{
				IsGlowing = true;
				// Make half-opaque, to get more like the software rendering
				float Opacity = NormalEntry[3]/float(0xff);
				NormalEntry[3] = MakeEightBit(Opacity/(2-Opacity));
				GlowEntry[3] = MakeEightBit(Opacity/2);
			}
			else
			{
				// Make transparent, to get appropriate continuity
				GlowEntry[3] = 0;
			}
		}
	}
		
	// The first color is always the transparent color,
	// except if it is a landscape color
	if (TextureType != OGL_Txtr_Landscape)
		{NormalColorTable[0] = 0; GlowColorTable[0] = 0;}

//	PremultiplyColorTables();
}

void TextureManager::PremultiplyColorTables()
{
	uint32 alphaMask = PlatformIsLittleEndian() ? 0xff000000 : 0x000000ff;

	uint32 *tables[2];
	tables[0] = NormalColorTable;
	if (!IsShadeless && (TextureType != OGL_Txtr_Landscape))
		tables[1] = GlowColorTable;
	else
		tables[1] = 0;

	for (int table = 0; table < 2; table++)
	{
		if (!tables[table]) continue;
		for (int k = 0; k < MAXIMUM_SHADING_TABLE_INDEXES; k++)
		{
			if ((tables[table][k] & alphaMask) == alphaMask)
				continue;
			if ((tables[table][k] & alphaMask) == 0) {
				tables[table][k] = 0;
				continue;
			}
			
			short r, g, b, a;
			uint8 *PxlPtr = (uint8 *) &tables[table][k];
			
			r = PxlPtr[0];
			g = PxlPtr[1];
			b = PxlPtr[2];
			a = PxlPtr[3];
			
			r = (a * r + 127) / 255;
			g = (a * g + 127) / 255;
			b = (a * b + 127) / 255;
			
			PxlPtr[0] = (uint8) r;
			PxlPtr[1] = (uint8) g;
			PxlPtr[2] = (uint8) b;
		}
	}
}

uint32 *TextureManager::GetOGLTexture(uint32 *ColorTable)
{
	// Allocate pixel buffer
	int NumPixels = int(TxtrWidth)*int(TxtrHeight);
	uint32 *Buffer = new uint32[NumPixels];
	
	// Calculate the rows to move to the OpenGL buffer.
	short OGLHeightOffset, OGLHeightFinish, OrigHeightDiff;
	if (HeightOffset >= 0)
	{
		OGLHeightOffset = HeightOffset;
		OGLHeightFinish = std::min(static_cast<int>(TxtrHeight), HeightOffset + BaseTxtrHeight);
		OrigHeightDiff = -HeightOffset;
	}
	else
	{
		OGLHeightOffset = 0;
		OGLHeightFinish = TxtrHeight;
		OrigHeightDiff = -HeightOffset;
	}

	// Calculate the pixels within each row to move.
	// If we have a constant row size, we can calculate the
	// offsets once for every row.
	short OrigWidthOffset, OGLWidthOffset, OGLWidthFinish;
	if (Texture->bytes_per_row != NONE)
	{
		if (WidthOffset >= 0)
		{
			OrigWidthOffset = 0;
			OGLWidthOffset = WidthOffset;
			OGLWidthFinish = std::min(static_cast<int>(TxtrWidth), WidthOffset + BaseTxtrWidth);
		}
		else
		{
			OrigWidthOffset = -WidthOffset;
			OGLWidthOffset = 0;
			OGLWidthFinish = TxtrWidth;
		}
	}
	else
	{
		// set later for each row
		OrigWidthOffset = 0;
		OGLWidthOffset = 0;
		OGLWidthFinish = 0;
	}

	uint32 rgb_mask = PlatformIsLittleEndian() ? 0x00ffffff : 0xffffff00;
	
	for (short h = OGLHeightOffset; h < OGLHeightFinish; h++)
	{
		byte *OrigStrip = Texture->row_addresses[h + OrigHeightDiff];
		uint32 *OGLStrip = &Buffer[TxtrWidth * h];
		
		if (Texture->bytes_per_row == NONE)
		{
			// Determine the offsets for this row
			// This is the Marathon 2 sprite-interpretation scheme;
			// assumes big-endian data
			
			// First destination location
			uint16 First = uint16(*(OrigStrip++)) << 8;
			First |= uint16(*(OrigStrip++));
			// Last destination location (last pixel is just before it)
			uint16 Last = uint16(*(OrigStrip++)) << 8;
			Last |= uint16(*(OrigStrip++));
			
			if (WidthOffset + First >= 0)
			{
				OrigWidthOffset = 0;
				OGLWidthOffset = WidthOffset + First;
			}
			else
			{
				OrigWidthOffset = -(WidthOffset + First);
				OGLWidthOffset = 0;
			}
			OGLWidthFinish = std::min(static_cast<int>(TxtrWidth), WidthOffset + Last);
		}
		OrigStrip += OrigWidthOffset;
		
		// smear first pixel to left edge
		for (short w = 0; w < OGLWidthOffset; w++)
			*(OGLStrip++) = ColorTable[*OrigStrip] & rgb_mask;
		
		for (short w = OGLWidthOffset; w < OGLWidthFinish; w++)
			*(OGLStrip++) = ColorTable[*(OrigStrip++)];

		// smear last pixel to right edge
		for (short w = OGLWidthFinish; w < TxtrWidth; w++)
			*(OGLStrip++) = ColorTable[*(OrigStrip - 1)] & rgb_mask;
	}
	
	// smear first pixel row to top edge
	for (short h = 0; h < OGLHeightOffset; h++)
	{
		uint32 *SrcStrip;
		if (TextureType == OGL_Txtr_Landscape) 
		{
			SrcStrip = &Buffer[TxtrWidth * (2 * OGLHeightOffset - h) - 1];
		} 
		else
		{
			SrcStrip = &Buffer[TxtrWidth * OGLHeightOffset];
		}
		uint32 *OGLStrip = &Buffer[TxtrWidth * h];

		for (short w = 0; w < TxtrWidth; w++)
			*(OGLStrip++) = *(SrcStrip++) & rgb_mask;
	}
	// smear last pixel row to bottom edge
	for (short h = OGLHeightFinish; h < TxtrHeight; h++)
	{
		uint32 *SrcStrip;
		if (TextureType == OGL_Txtr_Landscape) 
		{
			SrcStrip = &Buffer[TxtrWidth * (2 * OGLHeightFinish - h - 1)];
		} 
		else 
		{
			SrcStrip = &Buffer[TxtrWidth * (OGLHeightFinish - 1)];
		}
		uint32 *OGLStrip = &Buffer[TxtrWidth * h];
		
		for (short w = 0; w < TxtrWidth; w++)
			*(OGLStrip++) = *(SrcStrip++) & rgb_mask;
	}
	
	return Buffer;
}


uint32 *TextureManager::GetFakeLandscape()
{
	// Allocate and set to black and transparent
	int NumPixels = int(TxtrWidth)*int(TxtrHeight);
	uint32 *Buffer = new uint32[NumPixels];
	objlist_clear(Buffer,NumPixels);
	
	// Set up land and sky colors;
	// be sure to idiot-proof out-of-range ones
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	int LscpIndx = static_world->song_index;
	if (!LandscapesLoaded || (LscpIndx < 0 && LscpIndx >= 4))
	{
		memset(Buffer,0,NumPixels*sizeof(uint32));
		return Buffer;
	}
	
	RGBColor OrigLandColor = ConfigureData.LscpColors[LscpIndx][0];
	RGBColor OrigSkyColor = ConfigureData.LscpColors[LscpIndx][1];
	
	// Set up floating-point ones, complete with alpha channel
	GLfloat LandColor[4], SkyColor[4];
	MakeFloatColor(OrigLandColor,LandColor);
	LandColor[3] = 1;
	MakeFloatColor(OrigSkyColor,SkyColor);
	SkyColor[3] = 1;
	
	uint32 TxtrLandColor = MakeIntColor(LandColor);
	uint32 TxtrSkyColor = MakeIntColor(SkyColor);
	
	// Textures' vertical dimension is upward;
	// put in the land after the sky
	uint32 *BufPtr = Buffer;
	for (int h=0; h<TxtrHeight/2; h++)
		for (int w=0; w<TxtrWidth; w++)
			*(BufPtr++) = TxtrLandColor;
	for (int h=0; h<TxtrHeight/2; h++)
		for (int w=0; w<TxtrWidth; w++)
			*(BufPtr++) = TxtrSkyColor;
		
	return Buffer;
}


uint32 *TextureManager::Shrink(uint32 *Buffer)
{
	int NumPixels = int(LoadedWidth)*int(LoadedHeight);
	GLuint *NewBuffer = new GLuint[NumPixels];
	gluScaleImage(GL_RGBA, TxtrWidth, TxtrHeight, GL_UNSIGNED_BYTE, Buffer,
		LoadedWidth, LoadedHeight, GL_UNSIGNED_BYTE, NewBuffer);
	
	return (uint32 *)NewBuffer;
}


// This places a texture into the OpenGL software and gives it the right
// mapping attributes
void TextureManager::PlaceTexture(const ImageDescriptor *Image, bool normal_map)
{

	bool mipmapsLoaded = false;

	TxtrTypeInfoData& TxtrTypeInfo = TxtrTypeInfoList[TextureType];

	GLenum internalFormat = TxtrTypeInfo.ColorFormat;
	// some optimizations here:
	if (TextureType == 1) // landscape
	{
		if (internalFormat == GL_RGBA8)
			internalFormat = GL_RGB8;
		else if (internalFormat == GL_RGBA4)
			internalFormat = GL_RGB5;
	} 
	else if (!IsBlended() && internalFormat == GL_RGBA4)
	{
		internalFormat = GL_RGB5_A1;
	}

	bool load_as_sRGB = (Wanting_sRGB && !normal_map &&
						 Collection != _collection_interface &&
						 Collection != _collection_weapons_in_hand);
	
	if(load_as_sRGB) {
	  switch(internalFormat) {
	  case GL_RGB:
	  case GL_R3_G3_B2:
	  case GL_RGB4:
	  case GL_RGB5:
	  case GL_RGB8:
	  case GL_RGB10:
	  case GL_RGB12:
	  case GL_RGB16:
	    internalFormat = GL_SRGB;
	    break;
	  case GL_RGBA:
	  case GL_RGBA2:
	  case GL_RGBA4:
	  case GL_RGB5_A1:
	  case GL_RGBA8:
	  case GL_RGB10_A2:
	  case GL_RGBA12:
	  case GL_RGBA16:
	    internalFormat = GL_SRGB_ALPHA;
	    break;
#if defined(GL_ARB_texture_compression) && defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	    /* These might not do anything... */
	  case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	    internalFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
	    break;
	  case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	    internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
	    break;
	  case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	    internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
	    break;
	  case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	    internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
	    break;
#endif
	  }
	}

	if (Image->GetFormat() == ImageDescriptor::RGBA8) {
		switch (TxtrTypeInfo.FarFilter)
		{
		case GL_NEAREST:
		case GL_LINEAR:
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Image->GetWidth(), Image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Image->GetBuffer());
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
			if (Image->GetMipMapCount() > 1) {
#ifdef GL_SGIS_generate_mipmap
	if (useSGISMipmaps) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
	}
#endif
				int i = 0;
				for (i = 0; i < Image->GetMipMapCount(); i++) {
					glTexImage2D(GL_TEXTURE_2D, i, internalFormat, max(1, Image->GetWidth() >> i), max(1, Image->GetHeight() >> i), 0, GL_RGBA, GL_UNSIGNED_BYTE, Image->GetMipMapPtr(i));
				}
				mipmapsLoaded = true;
			} else {
#ifdef GL_SGIS_generate_mipmap
			if (useSGISMipmaps) {
				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Image->GetWidth(), Image->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Image->GetBuffer());
			} else 
#endif
			{
				gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, Image->GetWidth(), Image->GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, Image->GetBuffer());
			}
			mipmapsLoaded = true;
			}
			break;
		default:
			assert(false);
		}
	} else if (Image->GetFormat() == ImageDescriptor::DXTC1 ||
		   Image->GetFormat() == ImageDescriptor::DXTC3 ||
		   Image->GetFormat() == ImageDescriptor::DXTC5)
	{
#if defined(GL_ARB_texture_compression) && defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
		if (Image->GetFormat() == ImageDescriptor::DXTC1)
		  internalFormat = (load_as_sRGB) ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		else if (Image->GetFormat() == ImageDescriptor::DXTC3)
		  internalFormat = (load_as_sRGB) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else if (Image->GetFormat() == ImageDescriptor::DXTC5)
		  internalFormat = (load_as_sRGB) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		
		switch(TxtrTypeInfo.FarFilter)
		{
		case GL_NEAREST:
		case GL_LINEAR:
			glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, Image->GetWidth(), Image->GetHeight(), 0, Image->GetMipMapSize(0), Image->GetBuffer());
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
			if (Image->GetMipMapCount() > 1) {
#ifdef GL_SGIS_generate_mipmap
				if (useSGISMipmaps) {
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
				}
#endif
				int i = 0;
				for (i = 0; i < Image->GetMipMapCount(); i++) {
					glCompressedTexImage2DARB(GL_TEXTURE_2D, i, internalFormat, max(1, Image->GetWidth() >> i), max(1, Image->GetHeight() >> i), 0, Image->GetMipMapSize(i), Image->GetMipMapPtr(i));
				}
				mipmapsLoaded = true;
			} else {
#if defined GL_SGIS_generate_mipmap
				if (useSGISMipmaps) {
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
					mipmapsLoaded = true;
				}  
#endif
				glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, Image->GetWidth(), Image->GetHeight(), 0, Image->GetMipMapSize(0), Image->GetBuffer());
			}
			break;
			
		default:
			// Shouldn't happen
			assert(false);
		}
#else
		assert(false);
#endif
	}
	
	// Set texture-mapping features
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TxtrTypeInfo.NearFilter);
	if ((TxtrTypeInfo.FarFilter == GL_NEAREST_MIPMAP_NEAREST || TxtrTypeInfo.FarFilter == GL_LINEAR_MIPMAP_NEAREST || TxtrTypeInfo.FarFilter == GL_NEAREST_MIPMAP_LINEAR || TxtrTypeInfo.FarFilter == GL_LINEAR_MIPMAP_LINEAR) && !mipmapsLoaded)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TxtrTypeInfo.FarFilter);
	}

	switch(TextureType)
	{
	case OGL_Txtr_Wall:
		// Walls are tiled in both direction
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#if defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
                // enable anisotropic filtering
                {
                    float anisoLevel = Get_OGL_ConfigureData().AnisotropyLevel;
                    if (anisoLevel > 0.0) {
                        GLfloat max_aniso;
                        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
                        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F + ((anisoLevel-1.0F)/15.0F)*(max_aniso-1.0F));
                    }
                }
#endif
		break;
		
	case OGL_Txtr_Landscape:
		// Landscapes repeat horizontally, have vertical limits or repeats vertically
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		if (LandscapeVertRepeat)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		else
		{
#if defined(GL_ARB_texture_mirrored_repeat)
			if (useMirroredRepeat)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB);
			}
			else
#endif
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);			
			}	
		}
		break;
		
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_WeaponsInHand:
		// Sprites have both horizontal and vertical limits
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		break;
	}
}

// What to render:

// Always call this one and call it first; safe to allocate texture ID's in it
void TextureManager::RenderNormal()
{


	TxtrStatePtr->Allocate(TextureType);
	
	if (TxtrStatePtr->UseNormal())
	{
		assert(NormalBuffer || (NormalImage.get() && NormalImage.get()->IsPresent()));
		if (NormalImage.get() && NormalImage.get()->IsPresent()) {
			PlaceTexture(NormalImage.get());
		}
	}
	
		gGLTxStats.binds++;
		int time = 0;
		gGLTxStats.totalBind += time;
		if (gGLTxStats.minBind > time) gGLTxStats.minBind = time;
		if (gGLTxStats.maxBind < time) gGLTxStats.maxBind = time;
		if (time>2) gGLTxStats.longNormalSetups++;
}

// Call this one after RenderNormal()
void TextureManager::RenderGlowing()
{


	if (TxtrStatePtr->UseGlowing())
	{
		assert(GlowBuffer || (GlowImage.get() && GlowImage.get()->IsPresent()));
		if (GlowImage.get() && GlowImage.get()->IsPresent()) {
			PlaceTexture(GlowImage.get());
		}
	}
	
		gGLTxStats.binds++;
		int time = 0;
		gGLTxStats.totalBind += time;
		if (gGLTxStats.minBind > time) gGLTxStats.minBind = time;
		if (gGLTxStats.maxBind < time) gGLTxStats.maxBind = time;
		if (time>2) gGLTxStats.longGlowSetups++;
}

void TextureManager::RenderBump()
{
	if (TxtrStatePtr->IsBumped) {
		if (TxtrStatePtr->UseBump() && OffsetImage.get() && OffsetImage.get()->IsPresent())
			PlaceTexture(OffsetImage.get(), true);
	} else {
		FlatBumpTexture();
	}
	
	gGLTxStats.binds++;
	int time = 0;
	gGLTxStats.totalBind += time;
	if (gGLTxStats.minBind > time) gGLTxStats.minBind = time;
	if (gGLTxStats.maxBind < time) gGLTxStats.maxBind = time;
	if (time>2) gGLTxStats.longBumpSetups++;
}

void TextureManager::SetupTextureMatrix()
{
	// set up the texture matrix
	switch(TextureType)
	{
	case OGL_Txtr_Wall:
	case OGL_Txtr_WeaponsInHand:
	case OGL_Txtr_Inhabitant:
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		if (TxtrOptsPtr->Substitution) {
			// these come in right side up, but the renderer
			// expects them to be upside down and sideways
			glRotatef(90.0, 0.0, 0.0, 1.0);
			glScalef(1.0, -1.0, 1.0);
		}
		glMatrixMode(GL_MODELVIEW);
		break;
	case OGL_Txtr_Landscape:
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		if (TxtrOptsPtr->Substitution) {
			// these come in right side up, and un-centered
			// the renderer expects them upside down, and centered
			glScalef(1.0, -U_Scale, 1.0);
			glTranslatef(0.0, U_Offset, 0.0);
		} else {
			glScalef(1.0, U_Scale, 1.0);
			glTranslatef(0.0, U_Offset, 0.0);
		}
		glMatrixMode(GL_MODELVIEW);
		break;
	}
}

void TextureManager::RestoreTextureMatrix()
{
	switch(TextureType)
	{
	case OGL_Txtr_Wall:
	case OGL_Txtr_WeaponsInHand:
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_Landscape:
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
}



// Init
TextureManager::TextureManager()
{
	NormalBuffer = 0;
	GlowBuffer = 0;

	ShadingTables = NULL;
	TransferMode = 0;
	TransferData = 0;
	IsShadeless = false;
	TextureType = 0;
	LandscapeVertRepeat = false;
	
	TxtrStatePtr = 0;
	TxtrOptsPtr = 0;

	FastPath = 0;
	
	LowLevelShape = 0;
	
	// Marathon default
	Landscape_AspRatExp = 1;
}

// Cleanup
TextureManager::~TextureManager()
{
	if (NormalBuffer != 0) delete []NormalBuffer;
	if (GlowBuffer != 0) delete []GlowBuffer;
}

void OGL_ResetTextures()
{
	// Fix for crashing bug when OpenGL is inactive
	if (!OGL_IsActive()) return;
	
	// Reset the textures:
	for (int it=0; it<OGL_NUMBER_OF_TEXTURE_TYPES; it++)
		for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
		{
			bool CollectionPresent = is_collection_present(ic);
			short NumberOfBitmaps =
				CollectionPresent ? get_number_of_collection_bitmaps(ic) : 0;
			
			CollBitmapTextureState *CBTSSet = TextureStateSets[it][ic];
			for (int ib=0; ib<NumberOfBitmaps; ib++)
			{
				TextureState *TSSet = CBTSSet[ib].CTStates;
				for (int ist=0; ist<NUMBER_OF_OPENGL_BITMAP_SETS; ist++)
					TSSet[ist].Reset();
			}
		}
	
	// Reset the surface textures for all the models:
	OGL_ResetModelSkins(OGL_IsActive());
	
	// Reset the font textures
	FontSpecifier::OGL_ResetFonts(false);
	
	// Reset blitters
	OGL_Blitter::StopTextures();

	glDeleteTextures(1, &flatBumpTextureID);
	flatBumpTextureID = 0;
}


void LoadModelSkin(ImageDescriptor& SkinImage, short Collection, short CLUT)
{
	// A lot of this is copies of TextureManager member code
	
	ImageDescriptorManager Image;
	Image.set(&SkinImage);
	
	int TxtrWidth = Image.get()->GetWidth();
	int TxtrHeight = Image.get()->GetHeight();

	bool IsInfravision = IsInfravisionTable(CLUT);
	bool IsSilhouette = IsSilhouetteTable(CLUT);
	
	if (IsSilhouette)
		FindSilhouetteVersion(Image);
	
	TxtrTypeInfoData& TxtrTypeInfo = ModelSkinInfo;

	// Display size: may be shrunk
	int LoadedWidth = MAX(TxtrWidth >> TxtrTypeInfo.Resolution, 1);
	int LoadedHeight = MAX(TxtrHeight >> TxtrTypeInfo.Resolution, 1);
	
	// Fit the image into the maximum size allowed by the OpenGL implementation in use
	GLint MaxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTextureSize);
	while (LoadedWidth > MaxTextureSize || LoadedHeight > MaxTextureSize)
	{
		LoadedWidth >>= 1;
		LoadedHeight >>= 1;
	}
	
	while (Image.get()->GetWidth() > LoadedWidth || Image.get()->GetHeight() > LoadedHeight)
	{
		if (!Image.edit()->Minify()) break;
	}

	bool mipmapsLoaded = false;

	// Load the texture
	GLenum internalFormat = TxtrTypeInfo.ColorFormat;
	if (Image.get()->GetFormat() == ImageDescriptor::RGBA8)
	{
		switch(TxtrTypeInfo.FarFilter)
		{
		case GL_NEAREST:
		case GL_LINEAR:
			glTexImage2D(GL_TEXTURE_2D, 0, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
				     0, GL_RGBA, GL_UNSIGNED_BYTE, Image.get()->GetBuffer());
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
			if (Image.get()->GetMipMapCount() > 1) 
			{
#ifdef GL_SGIS_generate_mipmap
				if (useSGISMipmaps) {
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
				}
#endif
				int i = 0;
				for (i = 0; i < Image.get()->GetMipMapCount(); i++) 
				{
					glTexImage2D(GL_TEXTURE_2D, i, internalFormat, max(1, Image.get()->GetWidth() >> i), max(1, Image.get()->GetHeight() >> i), 0, GL_RGBA, GL_UNSIGNED_BYTE, Image.get()->GetMipMapPtr(i));
				}
				mipmapsLoaded = true;
			}
			else
			{
#ifdef GL_SGIS_generate_mipmap
				if (useSGISMipmaps)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Image.get()->GetWidth(), Image.get()->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, Image.get()->GetBuffer());
				}
				else
#endif
				{
					gluBuild2DMipmaps(GL_TEXTURE_2D, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
							  GL_RGBA, GL_UNSIGNED_BYTE, Image.get()->GetBuffer());
				}
				mipmapsLoaded = true;
			}
			break;
			
		default:
			// Shouldn't happen
			assert(false);
		}
	}
	else if (Image.get()->GetFormat() == ImageDescriptor::DXTC1 ||
		 Image.get()->GetFormat() == ImageDescriptor::DXTC3 ||
		 Image.get()->GetFormat() == ImageDescriptor::DXTC5)
	{
#if defined (GL_ARB_texture_compression) && defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
		if (Image.get()->GetFormat() == ImageDescriptor::DXTC1)
			internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		else if (Image.get()->GetFormat() == ImageDescriptor::DXTC3)
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else if (Image.get()->GetFormat() == ImageDescriptor::DXTC5)
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

		switch (TxtrTypeInfo.FarFilter)
		{
		case GL_NEAREST:
		case GL_LINEAR:
			glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, Image.get()->GetWidth(), Image.get()->GetHeight(), 0, Image.get()->GetMipMapSize(0), Image.get()->GetBuffer());
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
			if (Image.get()->GetMipMapCount() > 1)
			{
#ifdef GL_SGIS_generate_mipmap
				if (useSGISMipmaps)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
				}
#endif
				int i = 0;
				for (i = 0; i < Image.get()->GetMipMapCount(); i++)
				{
					glCompressedTexImage2DARB(GL_TEXTURE_2D, i, internalFormat, max(1, Image.get()->GetWidth() >> i), max(1, Image.get()->GetHeight() >> i), 0, Image.get()->GetMipMapSize(i), Image.get()->GetMipMapPtr(i));
				}
				mipmapsLoaded = true;
			}
			else
			{
#ifdef GL_SGIS_generate_mipmap
				if (useSGISMipmaps) 
				{
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
					mipmapsLoaded = true;
				}
#endif
				glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, Image.get()->GetWidth(), Image.get()->GetHeight(), 0, Image.get()->GetMipMapSize(0), Image.get()->GetBuffer());
			}
			break;

		default:
			// Shouldn't happen
			assert(false);
		}
#else
		assert(false);
#endif
	}
	
	// Set texture-mapping features
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TxtrTypeInfo.NearFilter);
	if ((TxtrTypeInfo.FarFilter == GL_NEAREST_MIPMAP_NEAREST || TxtrTypeInfo.FarFilter == GL_LINEAR_MIPMAP_NEAREST || TxtrTypeInfo.FarFilter == GL_NEAREST_MIPMAP_LINEAR || TxtrTypeInfo.FarFilter == GL_LINEAR_MIPMAP_LINEAR) && !mipmapsLoaded)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TxtrTypeInfo.FarFilter);
	}

	
	// Like sprites, model textures have both horizontal and vertical limits
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

}


// Infravision (I'm blue, are you?)
bool& IsInfravisionActive() {return InfravisionActive;}


// Sets the infravision tinting color for a shapes collection, and whether to use such tinting;
// the color values are from 0 to 1.
bool SetInfravisionTint(short Collection, bool IsTinted, float Red, float Green, float Blue)
{	
	assert(Collection >= 0 && Collection < NUMBER_OF_COLLECTIONS);
	InfravisionData& IVData = IVDataList[Collection];
	
	IVData.Red = Red;
	IVData.Green = Green;
	IVData.Blue = Blue;
	IVData.IsTinted = IsTinted;
	
	return true;
}

// Finds the infravision version of a color for some collection set;
// it makes no change if infravision is inactive.
void FindInfravisionVersionRGBA(short Collection, GLfloat *Color)
{
	if (!InfravisionActive) return;
	
	InfravisionData& IVData = IVDataList[Collection];
	if (!IVData.IsTinted) return;
	
	GLfloat AvgColor = (Color[0] + Color[1] + Color[2])/3;
	Color[0] = IVData.Red*AvgColor;
	Color[1] = IVData.Green*AvgColor;
	Color[2] = IVData.Blue*AvgColor;
}

void FindSilhouetteVersionDXTC1(int NumBytes, unsigned char *buffer)
{
	uint16 *pixels = (uint16 *) buffer;

	for (int i = 0; i < NumBytes / 4; i++)
	{
		if (SDL_SwapLE16(pixels[i * 4]) > SDL_SwapLE16(pixels[i * 4 + 1]))
		{
			pixels[i * 4 + 1] = PlatformIsLittleEndian() ? 0xffdf : 0xdfff;
		} 
		else
		{
			pixels[i * 4 + 1] = 0xffff;
		}
		pixels[i * 4] = 0xffff;
	}
}

void FindSilhouetteVersionDXTC35(int NumBytes, unsigned char *buffer)
{
	uint16 *pixels = (uint16 *) buffer;
	
	for (int i = 0; i < NumBytes / 8; i++)
	{
		pixels[i * 8 + 4] = 0xffff;
		pixels[i * 8 + 5] = PlatformIsLittleEndian() ? 0xffdf : 0xdfff;
	}
}

void FindSilhouetteVersionRGBA(int NumPixels, uint32 *Pixels)
{
	for (int i = 0; i < NumPixels; i++) 
	{
		Pixels[i] |= PlatformIsLittleEndian() ? 0x00ffffff : 0xffffff00;
	}
}

void FindSilhouetteVersion(ImageDescriptorManager &imageManager)
{
	if (imageManager.get()->GetFormat() == ImageDescriptor::RGBA8)
	{
		FindSilhouetteVersionRGBA(imageManager.edit()->GetBufferSize() / 4, imageManager.edit()->GetBuffer());
	}
	else if (imageManager.get()->GetFormat() == ImageDescriptor::DXTC1)
	{
		FindSilhouetteVersionDXTC1(imageManager.edit()->GetBufferSize(), (unsigned char *) imageManager.edit()->GetBuffer());
	}
	else if (imageManager.get()->GetFormat() == ImageDescriptor::DXTC3 || imageManager.get()->GetFormat() == ImageDescriptor::DXTC5) 
	{
		FindSilhouetteVersionDXTC35(imageManager.edit()->GetBufferSize(), (unsigned char *) imageManager.edit()->GetBuffer());
	}
	imageManager.edit()->PremultipliedAlpha = false;
}

static inline uint16 SetPixelOpacitiesDXTC3Row(int scale, int shift, uint16 alpha)
{
	uint16 a1 = (alpha >> 12) & 0xf;
	uint16 a2 = (alpha >> 8) & 0xf;
	uint16 a3 = (alpha >> 4) & 0xf;
	uint16 a4 = alpha & 0xf;

	a1 = PIN((a1 * scale) / 16 + shift, 0, 15);
	a2 = PIN((a2 * scale) / 16 + shift, 0, 15);
	a3 = PIN((a3 * scale) / 16 + shift, 0, 15);
	a4 = PIN((a4 * scale) / 16 + shift, 0, 15);

	return ((a1 << 12) | (a2 << 8) | (a3 << 4) | a4);
}

void SetPixelOpacitiesDXTC3(OGL_TextureOptions& Options, int NumBytes, unsigned char *buffer)
{
	assert(NumBytes % 16 == 0);

	uint16 *rows = (uint16 *) buffer;

	int scale = PIN(int(Options.OpacityScale * 16), 0, 16);
	int shift = PIN(int(Options.OpacityShift * 16), -16, 16);
	
	for (int i = 0; i < NumBytes / 8; i++) {
		uint16 *a1 = &rows[i * 8];
		uint16 *a2 = &rows[i * 8 + 1];
		uint16 *a3 = &rows[i * 8 + 2];
		uint16 *a4 = &rows[i * 8 + 3];
		*a1 = SetPixelOpacitiesDXTC3Row(scale, shift, *a1);
		*a2 = SetPixelOpacitiesDXTC3Row(scale, shift, *a2);
		*a3 = SetPixelOpacitiesDXTC3Row(scale, shift, *a3);
		*a4 = SetPixelOpacitiesDXTC3Row(scale, shift, *a4);
	}
		
}

static inline uint16 SetPixelOpacitiesDXTC5Pair(int scale, int shift, uint16 alpha)
{
	uint16 a1 = alpha >> 8;
	uint16 a2 = alpha & 0xff;

	uint16 new_a1 = PIN((a1 * scale) / 256 + shift, 0, 255);
	uint16 new_a2 = PIN((a2 * scale) / 256 + shift, 0, 255);

	if (new_a1 == new_a2 && a1 != a2)
		if (a1 > a2)
			if (new_a2) new_a2--;
			else new_a1++;
		else 
			if (new_a1) new_a1--;
			else new_a2++;
	else if ((new_a1 > new_a2) != (a1 > a2))
		SWAP(new_a1, new_a2);
	
	return (new_a1 << 8 | new_a2);
}

void SetPixelOpacitiesDXTC5(OGL_TextureOptions& Options, int NumBytes, unsigned char *buffer)
{
	assert (NumBytes % 16 == 0);

	uint16 *pixels = (uint16 *) buffer;
	
	int scale = PIN(int(Options.OpacityScale * 256), 0, 256);
	int shift = PIN(int(Options.OpacityShift * 256), -256, 256);

	for (int i = 0; i < NumBytes / 8; i++) {
		pixels[i * 8] = SDL_SwapLE16(SetPixelOpacitiesDXTC5Pair(scale, shift, SDL_SwapLE16(pixels[i * 8])));
	}
}


void SetPixelOpacities(OGL_TextureOptions& Options, ImageDescriptorManager &imageManager)
{
	if (Options.OpacityType != OGL_OpacType_Avg && Options.OpacityType != OGL_OpacType_Max && Options.OpacityScale == 1.0 && Options.OpacityShift == 0.0)
		return;

	if (imageManager.get()->GetFormat() == ImageDescriptor::RGBA8) {

		SetPixelOpacitiesRGBA(Options, imageManager.edit()->GetBufferSize() / 4, imageManager.edit()->GetBuffer());

	} else if (imageManager.get()->GetFormat() == ImageDescriptor::DXTC3) {

		// to do opac_type we have to decompress the texture
		if (Options.OpacityType == OGL_OpacType_Avg || Options.OpacityType == OGL_OpacType_Max) {
			if (imageManager.edit()->MakeRGBA()) {
				SetPixelOpacitiesRGBA(Options, imageManager.edit()->GetBufferSize() / 4, imageManager.edit()->GetBuffer());
			} else if (Options.OpacityScale == 1.0 && Options.OpacityShift == 0.0) {
				return;
			} else {
				SetPixelOpacitiesDXTC3(Options, imageManager.edit()->GetBufferSize() / 4, (unsigned char *) imageManager.edit()->GetBuffer());
			}
		} else {
			// if it's just scale/shift, we can do without decompressing
			SetPixelOpacitiesDXTC3(Options, imageManager.edit()->GetBufferSize(), (unsigned char *) imageManager.edit()->GetBuffer());
		}

	} else if (imageManager.get()->GetFormat() == ImageDescriptor::DXTC5) {
		if (Options.OpacityType == OGL_OpacType_Avg || Options.OpacityType == OGL_OpacType_Max) {
			if (imageManager.edit()->MakeRGBA()) {
				SetPixelOpacitiesRGBA(Options, imageManager.edit()->GetBufferSize() / 4, imageManager.edit()->GetBuffer());
			} else if (Options.OpacityScale == 1.0 && Options.OpacityShift == 0.0) {
				return;
			} else {
				SetPixelOpacitiesDXTC5(Options, imageManager.edit()->GetBufferSize() / 4, (unsigned char *) imageManager.edit()->GetBuffer());
			}
		} else {
			SetPixelOpacitiesDXTC5(Options, imageManager.edit()->GetBufferSize(), (unsigned char *) imageManager.edit()->GetBuffer());
		}
	} else {
		// we have to decompress DXTC1 to do anything
		if (imageManager.edit()->MakeRGBA()) {
			SetPixelOpacitiesRGBA(Options, imageManager.edit()->GetBufferSize() / 4, imageManager.edit()->GetBuffer());
		}
	}

}


// Does this for a set of several pixel values or color-table values;
// the pixels are assumed to be in OpenGL-friendly byte-by-byte RGBA format.
void SetPixelOpacitiesRGBA(OGL_TextureOptions& Options, int NumPixels, uint32 *Pixels)
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
				Opacity = (Red + Green + Blue)/3.0F;
			}
			break;
			
		case OGL_OpacType_Max:
			{
				uint32 Red = uint32(PxlPtr[0]);
				uint32 Green = uint32(PxlPtr[1]);
				uint32 Blue = uint32(PxlPtr[2]);
				Opacity = (float)MAX(MAX(Red,Green),Blue);
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

/*
// Stuff for doing 16->32 pixel-format conversion, 1555 ARGB to 8888 RGBA
GLuint *ConversionTable_16to32 = NULL;

void MakeConversion_16to32(int BitDepth)
{
	// This is for allocating a 16->32 conversion table only when necessary
	if (BitDepth == 16 && (!ConversionTable_16to32))
	{
		// Allocate it
		int TableSize = (1 << 15);
		ConversionTable_16to32 = new GLuint[TableSize];
	
		// Fill it
		for (word InVal = 0; InVal < TableSize; InVal++)
			ConversionTable_16to32[InVal] = Convert_16to32(InVal);
	}
	else if (ConversionTable_16to32)
	{
		// Get rid of it
		delete []ConversionTable_16to32;
		ConversionTable_16to32 = NULL;
	}
}
*/

#endif // def HAVE_OPENGL
