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

#ifdef __MVCPP__
#include <windows.h>
#endif

#ifdef HAVE_OPENGL
# if defined (__APPLE__) && defined (__MACH__)
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <OpenGL/glext.h>
# elif defined mac
#   include <gl.h>
#   include <glu.h>
#   include <glext.h>
# else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   ifdef HAVE_GL_GLEXT_H
#     include <GL/glext.h>
#   endif
# endif
#endif

#ifdef mac
# if defined(EXPLICIT_CARBON_HEADER)
#  include <AGL/agl.h>
# else
#  include <agl.h>
# endif
#endif

#include "interface.h"
#include "render.h"
#include "map.h"
#include "collection_definition.h"
#include "OGL_Setup.h"
#include "OGL_Render.h"
#include "OGL_Textures.h"

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

static list<TextureState*> sgActiveTextureStates;


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
	IsUsed = IsGlowing = TexGened[Normal] = TexGened[Glowing] = false;
	IDUsage[Normal] = IDUsage[Glowing] = unusedFrames = 0;
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
}


// Done with the texture accounting
void OGL_StopTextures()
{
	// Clear the texture accounting
	for (int it=0; it<OGL_NUMBER_OF_TEXTURE_TYPES; it++)
		for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
			if (TextureStateSets[it][ic]) delete []TextureStateSets[it][ic];
}

void OGL_FrameTickTextures()
{
	list<TextureState*>::iterator i;
	
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
			ColorPtr[0] = OrigPtr[1];
			ColorPtr[1] = OrigPtr[2];
			ColorPtr[2] = OrigPtr[3];
			ColorPtr[3] = 0xff;
		}
		break;
	}
}


inline bool IsLandscapeFlatColored()
{
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	return TEST_FLAG(ConfigureData.Flags,OGL_Flag_FlatLand);
}


#if 0
static void MakeAverage(int Length, GLuint *Buffer)
{
	float Sum[4];
	
	for (int q=0; q<4; q++) Sum[q] = 0;
	
	// Extract the bytes; the lowest byte gets done first
	for (int k=0; k<Length; k++)
	{
		GLuint PixVal = Buffer[k];
		for (int q=0; q<4; q++)
		{
			Sum[q] += (PixVal & 0x000000ff);
			PixVal >>= 8;
		}
	}
	
	// This processes the bytes from highest to lowest
	GLuint AvgVal = 0;
	for (int q=0; q<4; q++)
	{
		AvgVal <<= 8;	// Must come before adding in a byte
		AvgVal |= PIN(int(Sum[3-q]/Length + 0.5),0,255);
	}

	for (int k=0; k<Length; k++)
		Buffer[k] = AvgVal;
}
#endif


// Modify color-table index if necessary;
// makes it the infravision or silhouette one if necessary
short ModifyCLUT(short TransferMode, short CLUT)
{
	short CTable;
	
	// Tinted mode is only used for invisibility, and infravision will make objects visible
	if (TransferMode == _static_transfer) CTable = SILHOUETTE_BITMAP_SET;
	else if (TransferMode == _tinted_transfer) CTable = SILHOUETTE_BITMAP_SET;
	else if (InfravisionActive) CTable = INFRAVISION_BITMAP_SET;
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
		if (!LoadSubstituteTexture())
			if (!SetupTextureGeometry()) return false;
				
		// Store sprite scale/offset
		CBTS.U_Scale = U_Scale;
		CBTS.V_Scale = V_Scale;
		CBTS.U_Offset = U_Offset;
		CBTS.V_Offset = V_Offset;
		
		// This finding of color tables sets the glow state
		FindColorTables();
		// Override if textures had been substituted;
		// if the normal texture had been substituted, it will be assumed to be
		// non-glowing unless the glow texture has also been substituted.
		if (GlowBuffer) IsGlowing = true;
		else if (NormalBuffer) IsGlowing = false;
		
		CTState.IsGlowing = IsGlowing;
		
		// Load the fake landscape if selected
		if (TextureType == OGL_Txtr_Landscape)
		{
			if (IsLandscapeFlatColored())
			{
				if (NormalBuffer) delete []NormalBuffer;
				NormalBuffer = GetFakeLandscape();
			}
		}
		
		// If not, then load the expected textures
		if (!NormalBuffer)
			NormalBuffer = GetOGLTexture(NormalColorTable);
		if (IsGlowing && !GlowBuffer)
			GlowBuffer = GetOGLTexture(GlowColorTable);
		
		// Display size: may be shrunk
		LoadedWidth = MAX(TxtrWidth >> TxtrTypeInfo.Resolution, 1);
		LoadedHeight = MAX(TxtrHeight >> TxtrTypeInfo.Resolution, 1);
		
		// Fit the image into the maximum size allowed by the OpenGL implementation in use
		GLint MaxTextureSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTextureSize);
		while (LoadedWidth > MaxTextureSize || LoadedHeight > MaxTextureSize)
		{
			LoadedWidth >>= 1;
			LoadedHeight >>= 1;
		}
		
		if (LoadedWidth != TxtrWidth || LoadedHeight != TxtrHeight)
		{
			// Shrink it
			uint32 *NewNormalBuffer = Shrink(NormalBuffer);
			delete []NormalBuffer;
			NormalBuffer = NewNormalBuffer;
			
			if (IsGlowing)
			{
				uint32 *NewGlowBuffer = Shrink(GlowBuffer);
				delete []GlowBuffer;
				GlowBuffer = NewGlowBuffer;
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
		U_Scale = CBTS.U_Scale;
		V_Scale = CBTS.V_Scale;
		U_Offset = CBTS.U_Offset;
		V_Offset = CBTS.V_Offset;
		
		// Get glow state
		IsGlowing = CTState.IsGlowing;
	}
		
	// Done!!!
	return true;
}


// Next power of 2; since OpenGL prefers powers of 2, it is necessary to work out
// the next one up for each texture dimension.
inline int NextPowerOfTwo(int n)
{
	int p = 1;
	while(p < n) {p <<= 1;}
	return p;
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
	// Is there a texture to be substituted?
	ImageDescriptor& NormalImg = TxtrOptsPtr->NormalImg;
	if (!NormalImg.IsPresent()) return false;
	
	// Be sure to take care of the glowing version, where supported
	ImageDescriptor& GlowImg = TxtrOptsPtr->GlowImg;
	
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
	
	int Width = NormalImg.GetWidth();
	int Height = NormalImg.GetHeight();
	
	int HeightOffset;
	int OrigHeightOffset, OGLHeightOffset, CopyingHeight;
	
	switch(TextureType)
	{
	case OGL_Txtr_Wall:
		// For tiling to be possible, the width and height must be powers of 2;
		// also, be sure to transpose the texture
		TxtrWidth = Height;
		TxtrHeight = Width;
		if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
		if (TxtrHeight != NextPowerOfTwo(TxtrHeight)) return false;
		
		NormalBuffer = new uint32[TxtrWidth*TxtrHeight];
		for (int v=0; v<Height; v++)
			for (int h=0; h<Width; h++)
				NormalBuffer[h*Height+v] = NormalImg.GetPixel(h,v);
		
		// Walls can glow...
		if (GlowImg.IsPresent())
		{
			GlowBuffer = new uint32[TxtrWidth*TxtrHeight];
			for (int v=0; v<Height; v++)
				for (int h=0; h<Width; h++)
					GlowBuffer[h*Height+v] = GlowImg.GetPixel(h,v);
		}
		
		break;
	
	case OGL_Txtr_Landscape:
		// For tiling to be possible, the width must be a power of 2;
		// the height need not be such a power.
		// Also, flip the vertical dimension to get the orientation correct.
		// Some of this code is cribbed from some other code here in order to get
		// consistent landscape loading
		TxtrWidth = Width;
		TxtrHeight = (Landscape_AspRatExp >= 0) ?
			(TxtrWidth >> Landscape_AspRatExp) :
				(TxtrWidth << (-Landscape_AspRatExp));
		if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
		
		NormalBuffer = new uint32[TxtrWidth*TxtrHeight];
		memset(NormalBuffer,0,TxtrWidth*TxtrHeight*sizeof(uint32));
		
		// only valid source and destination pixels
		// will get worked with (no off-edge ones, that is).
		HeightOffset = (TxtrHeight - Height) >> 1;
		if (HeightOffset >= 0)
		{
			CopyingHeight = Height;
			OrigHeightOffset = 0;
			OGLHeightOffset = HeightOffset;
		}
		else
		{
			CopyingHeight = TxtrHeight;
			OrigHeightOffset = - HeightOffset;
			OGLHeightOffset = 0;
		}
		
		for (int v=0; v<CopyingHeight; v++)
		{
			// This optimization is reasonable because both the read-in-image
			// and the OpenGL-texture arrays have the same width/height arrangement
			uint32 *Src = &NormalImg.GetPixel(0,OrigHeightOffset+v);
			uint32 *Dest = NormalBuffer + (OGLHeightOffset + ((CopyingHeight-1)-v))*Width;
			memcpy(Dest,Src,Width*sizeof(uint32));
		}
		
		// No glow map here
		break;
		
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_WeaponsInHand:
		// Much of the code here has been copied from elsewhere.
		// Set these for convenience; sprites are transposed, as walls are.
		BaseTxtrWidth = Height;
		BaseTxtrHeight = Width;
		
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
		
		NormalBuffer = new uint32[TxtrWidth*TxtrHeight];
		objlist_clear(NormalBuffer,TxtrWidth*TxtrHeight);
		for (int v=0; v<Height; v++)
			for (int h=0; h<Width; h++)
				NormalBuffer[(HeightOffset+h)*TxtrWidth+(WidthOffset+v)] = NormalImg.GetPixel(h,v);
		
		// Objects can glow...
		if (GlowImg.IsPresent())
		{
			GlowBuffer = new uint32[TxtrWidth*TxtrHeight];
			objlist_clear(GlowBuffer,TxtrWidth*TxtrHeight);
			for (int v=0; v<Height; v++)
				for (int h=0; h<Width; h++)
					GlowBuffer[(HeightOffset+h)*TxtrWidth+(WidthOffset+v)] = GlowImg.GetPixel(h,v);
		}
		break;
	}
	
	// Use the Tomb Raider opacity hack if selected
	SetPixelOpacities(*TxtrOptsPtr,TxtrWidth*TxtrHeight,NormalBuffer);
	
	// Modify if infravision is active
	if (CTable == INFRAVISION_BITMAP_SET)
	{
		if (NormalBuffer)
			FindInfravisionVersion(Collection,TxtrWidth*TxtrHeight,NormalBuffer);
		
		// Infravision textures don't glow
		if (GlowBuffer)
		{
			delete []GlowBuffer;
			GlowBuffer = NULL;
		}
	}
	else if (CTable == SILHOUETTE_BITMAP_SET)
	{
		if (NormalBuffer)
		{
			for (int k=0; k<TxtrWidth*TxtrHeight; k++)
			{
				// Make the color white, but keep the opacity
				uint8 *PxlPtr = (uint8 *)(NormalBuffer + k);
				PxlPtr[0] = PxlPtr[1] = PxlPtr[2] = 0xff;
			}
		}
		// Silhouette textures don't glow
		if (GlowBuffer)
		{
			delete []GlowBuffer;
			GlowBuffer = NULL;
		}
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
		TxtrWidth = BaseTxtrWidth;
		TxtrHeight = BaseTxtrHeight;
		if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
		if (TxtrHeight != NextPowerOfTwo(TxtrHeight)) return false;
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
			if (TxtrWidth != NextPowerOfTwo(TxtrWidth)) return false;
			// Use the landscape height here
			TxtrHeight = (Landscape_AspRatExp >= 0) ?
				(TxtrWidth >> Landscape_AspRatExp) :
					(TxtrWidth << (-Landscape_AspRatExp));
			
			// Offsets
			WidthOffset = (TxtrWidth - BaseTxtrWidth) >> 1;
			HeightOffset = (TxtrHeight - BaseTxtrHeight) >> 1;
		}
		
		break;
		
	case OGL_Txtr_Inhabitant:
	case OGL_Txtr_WeaponsInHand:
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
	if (CTable == SILHOUETTE_BITMAP_SET)
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
		SetPixelOpacities(*TxtrOptsPtr, MAXIMUM_SHADING_TABLE_INDEXES, NormalColorTable);
		NormalColorTable[0] = 0;
		return;
	}
	
	// Number of source bytes, for reading off of the shading table
	// IR change: dithering
	short NumSrcBytes = bit_depth / 8;
	
	// Shadeless polygons use the first, instead of the last, shading table
	byte *OrigColorTable = (byte *)ShadingTables;
	byte *OrigGlowColorTable = OrigColorTable;
	if (!IsShadeless) OrigColorTable +=
		NumSrcBytes*(number_of_shading_tables - 1)*MAXIMUM_SHADING_TABLE_INDEXES;
	
	// Find the normal color table,
	// and set its opacities as if there was no glow table.
	FindOGLColorTable(NumSrcBytes,OrigColorTable,NormalColorTable);
	SetPixelOpacities(*TxtrOptsPtr,MAXIMUM_SHADING_TABLE_INDEXES,NormalColorTable);
	
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
}


uint32 *TextureManager::GetOGLTexture(uint32 *ColorTable)
{
	// Allocate and set to black and transparent
	int NumPixels = int(TxtrWidth)*int(TxtrHeight);
	uint32 *Buffer = new uint32[NumPixels];
	objlist_clear(Buffer,NumPixels);
	
	// The dimension, the offset in the original texture, and the offset in the OpenGL texture
	short Width, OrigWidthOffset, OGLWidthOffset;
	short Height, OrigHeightOffset, OGLHeightOffset;
	
	// Calculate original-texture and OpenGL-texture offsets
	// and how many scanlines to do.
	// The loop start points and counts are set so that
	// only valid source and destination pixels
	// will get worked with (no off-edge ones, that is).
	if (HeightOffset >= 0)
	{
		Height = BaseTxtrHeight;
		OrigHeightOffset = 0;
		OGLHeightOffset = HeightOffset;
	}
	else
	{
		Height = TxtrHeight;
		OrigHeightOffset = - HeightOffset;
		OGLHeightOffset = 0;
	}

	if (Texture->bytes_per_row == NONE)
	{
		short horig = OrigHeightOffset;
		uint32 *OGLRowStart = Buffer + TxtrWidth*OGLHeightOffset;
		for (short h=0; h<Height; h++)
		{
			byte *OrigStrip = Texture->row_addresses[horig];
			uint32 *OGLStrip = OGLRowStart;

			// Cribbed from textures.c:
			// This is the Marathon 2 sprite-interpretation scheme;
			// assumes big-endian data
				
			// First destination location
			uint16 First = uint16(*(OrigStrip++)) << 8;
			First |= uint16(*(OrigStrip++));
			// Last destination location (last pixel is just before it)
			uint16 Last = uint16(*(OrigStrip++)) << 8;
			Last |= uint16(*(OrigStrip++));
			
			// Calculate original-texture and OpenGL-texture offsets
			// and how many pixels to do
			OrigWidthOffset = 0;
			OGLWidthOffset = WidthOffset + First;
			
			if (OGLWidthOffset < 0)
			{
				OrigWidthOffset -= OGLWidthOffset;
				OGLWidthOffset = 0;
			}
			
			short OrigWidthFinish = Last - First;
			short OGLWidthFinish = WidthOffset + Last;
			
			short OGLWidthExcess = OGLWidthFinish - TxtrWidth;
			if (OGLWidthExcess > 0)
			{
				OrigWidthFinish -= OGLWidthExcess;
				OGLWidthFinish = TxtrWidth;
			}
			
			short Width = OrigWidthFinish - OrigWidthOffset;
			OrigStrip += OrigWidthOffset;
			OGLStrip += OGLWidthOffset;
			
			for (short w=0; w<Width; w++)
				*(OGLStrip++) = ColorTable[*(OrigStrip++)];
			horig++;
			OGLRowStart += TxtrWidth;
		}
	}
	else
	{
		// Calculate original-texture and OpenGL-texture offsets
		// and how many pixels to do
		if (WidthOffset >= 0)
		{
			Width = BaseTxtrWidth;
			OrigWidthOffset = 0;
			OGLWidthOffset = WidthOffset;
		}
		else
		{
			Width = TxtrWidth;
			OrigWidthOffset = - WidthOffset;
			OGLWidthOffset = 0;
		}
		short horig = OrigHeightOffset;
		uint32 *OGLRowStart = Buffer + TxtrWidth*OGLHeightOffset + OGLWidthOffset;
		for (short h=0; h<Height; h++)
		{
			byte *OrigStrip = Texture->row_addresses[horig] + OrigWidthOffset;
			uint32 *OGLStrip = OGLRowStart;
			for (short w=0; w<Width; w++)
				*(OGLStrip++) = ColorTable[*(OrigStrip++)];
			horig++;
			OGLRowStart += TxtrWidth;
		}
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
	
	// Modify if infravision is active
	if (CTable == INFRAVISION_BITMAP_SET)
	{
		FindInfravisionVersion(Collection,LandColor);
		FindInfravisionVersion(Collection,SkyColor);
	}
	
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
void TextureManager::PlaceTexture(uint32 *Buffer)
{

	TxtrTypeInfoData& TxtrTypeInfo = TxtrTypeInfoList[TextureType];

	// Load the texture
	switch(TxtrTypeInfo.FarFilter)
	{
	case GL_NEAREST:
	case GL_LINEAR:
		glTexImage2D(GL_TEXTURE_2D, 0, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
		gluBuild2DMipmaps(GL_TEXTURE_2D, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
			GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		break;
	
	default:
		// Shouldn't happen
		assert(false);
	}
	
	// Set texture-mapping features
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TxtrTypeInfo.NearFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TxtrTypeInfo.FarFilter);
	
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
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
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
		assert(NormalBuffer);
		PlaceTexture(NormalBuffer);
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
		assert(GlowBuffer);
		PlaceTexture(GlowBuffer);
	}

		gGLTxStats.binds++;
		int time = 0;
		gGLTxStats.totalBind += time;
		if (gGLTxStats.minBind > time) gGLTxStats.minBind = time;
		if (gGLTxStats.maxBind < time) gGLTxStats.maxBind = time;
		if (time>2) gGLTxStats.longGlowSetups++;
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

extern void ResetScreenFont();
extern void OGL_ResetMapFonts(bool IsStarting);
extern void OGL_ResetHUDFonts(bool IsStarting);

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
	ResetScreenFont();
	OGL_ResetMapFonts(false);
	OGL_ResetHUDFonts(false);
}


void LoadModelSkin(ImageDescriptor& Image, short Collection, short CLUT)
{
	// A lot of this is copies of TextureManager member code
	
	// Texture-buffer management
	GLuint *Buffer;
	vector<GLuint> ImageBuffer;
	vector<GLuint> ShrunkBuffer;
	
	int TxtrWidth = Image.GetWidth();
	int TxtrHeight = Image.GetHeight();
	Buffer = (GLuint*)Image.GetPixelBasePtr();
	
	bool IsInfravision = (CLUT == INFRAVISION_BITMAP_SET);
	bool IsSilhouette = (CLUT == SILHOUETTE_BITMAP_SET);
	
	// Use special texture buffer because the originals may be needed for "normal" textures
	int ImageSize = Image.GetNumPixels();
	if (IsInfravision || IsSilhouette)
	{
		ImageBuffer.resize(ImageSize);
		objlist_copy(&ImageBuffer[0],Buffer,ImageSize);
		Buffer = (GLuint*)&ImageBuffer[0];
	}
	
	if (IsInfravision)
		FindInfravisionVersion(Collection,ImageSize,(uint32*)Buffer);//AS added cast for Darwin compileability
	
	if (IsSilhouette)
	{
		for (int k=0; k<ImageSize; k++)
		{
			// Make the color white, but keep the opacity
			uint8 *PxlPtr = (uint8 *)((uint32*)Buffer + k);
			PxlPtr[0] = PxlPtr[1] = PxlPtr[2] = 0xff;
		}
	}
	
	TxtrTypeInfoData& TxtrTypeInfo = TxtrTypeInfoList[OGL_Txtr_Inhabitant];

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
	
	if (LoadedWidth != TxtrWidth || LoadedHeight != TxtrHeight)
	{
		int NumPixels = int(LoadedWidth)*int(LoadedHeight);
		ShrunkBuffer.resize(NumPixels);
		gluScaleImage(GL_RGBA, TxtrWidth, TxtrHeight, GL_UNSIGNED_BYTE, Buffer,
			LoadedWidth, LoadedHeight, GL_UNSIGNED_BYTE, &ShrunkBuffer[0]);
		
		Buffer = &ShrunkBuffer[0];
	}

	// Load the texture
	switch(TxtrTypeInfo.FarFilter)
	{
	case GL_NEAREST:
	case GL_LINEAR:
		glTexImage2D(GL_TEXTURE_2D, 0, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:
		gluBuild2DMipmaps(GL_TEXTURE_2D, TxtrTypeInfo.ColorFormat, LoadedWidth, LoadedHeight,
			GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		break;
	
	default:
		// Shouldn't happen
		assert(false);
	}
	
	// Set texture-mapping features
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TxtrTypeInfo.NearFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TxtrTypeInfo.FarFilter);

	
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
void FindInfravisionVersion(short Collection, GLfloat *Color)
{
	if (!InfravisionActive) return;
	
	InfravisionData& IVData = IVDataList[Collection];
	if (!IVData.IsTinted) return;
	
	GLfloat AvgColor = (Color[0] + Color[1] + Color[2])/3;
	Color[0] = IVData.Red*AvgColor;
	Color[1] = IVData.Green*AvgColor;
	Color[2] = IVData.Blue*AvgColor;
}


// Mass-production version of above; suitable for textures
void FindInfravisionVersion(short Collection, int NumPixels, uint32 *Pixels)
{
	if (!InfravisionActive) return;
	
	InfravisionData& IVData = IVDataList[Collection];
	if (!IVData.IsTinted) return;
	
	// OK to use marching-pointer optimization here;
	// the float-to-int and int-to-float conversions have been simplified,
	// because the infravision-value-finding does not care if the values
	// had been multipled by 255 (int <-> float color-value multiplier/divider)
	for (int k=0; k<NumPixels; k++, Pixels++)
	{
		uint8 *PxlPtr = (uint8 *)Pixels;
		GLfloat AvgColor = GLfloat(int(PxlPtr[0]) + int(PxlPtr[1]) + int(PxlPtr[2]))/3;
		PxlPtr[0] = PIN(int(IVData.Red*AvgColor + 0.5),0,255);
		PxlPtr[1] = PIN(int(IVData.Green*AvgColor + 0.5),0,255);
		PxlPtr[2] = PIN(int(IVData.Blue*AvgColor + 0.5),0,255);
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
