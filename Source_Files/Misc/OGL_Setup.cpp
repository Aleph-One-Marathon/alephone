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

Apr 27, 2001 (Loren Petrich):
	Modified the OpenGL fog support so as to enable below-liquid fogs

Jul 8, 2001 (Loren Petrich):
	Made it possible to read in silhouette bitmaps; one can now use the silhouette index
	as a MML color-table index

Aug 21, 2001 (Loren Petrich):
	Adding support for 3D-model inhabitant objects

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for OpenGL.h, AGL.h
	Removed QuickDraw3D support from Carbon

*/

#include <vector>
#include <string.h>
#include <math.h>
#include "cseries.h"

#ifdef HAVE_OPENGL

#ifdef __MVCPP__
#include <windows.h>
#endif

#if defined(mac)
# if defined(TARGET_API_MAC_CARBON)
#  include <OpenGL/gl.h>
#  include <AGL/agl.h>
# else
#  include <agl.h>
# endif
#elif defined(SDL)
# if defined (__APPLE__) && defined (__MACH__)
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#endif

#include "shape_descriptors.h"
#include "OGL_Setup.h"
#include "ColorParser.h"

#include "StudioLoader.h"
#include "WavefrontLoader.h"
#include "QD3D_Loader.h"

#ifdef __WIN32__
#include "OGL_Win32.h"
#endif

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
#if defined(__WIN32__)
	setup_gl_extensions();
#endif	
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


// Sensible defaults for the fog:
static OGL_FogData FogData[OGL_NUMBER_OF_FOG_TYPES] = 
{
	{{0x8000,0x8000,0x8000},8,false},
	{{0x8000,0x8000,0x8000},8,false}
};


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
			TxtrData.FarFilter = 1;		// GL_LINEAR
		TxtrData.Resolution = 0;		// 1x
		TxtrData.ColorFormat = 0;		// 32-bit color
	}
#ifdef SDL
	// Reasonable default flags ("static" effect causes massive slowdown, so we turn it off)
	Data.Flags = OGL_Flag_FlatStatic | OGL_Flag_Fader | OGL_Flag_Map |
		OGL_Flag_HUD | OGL_Flag_LiqSeeThru | OGL_Flag_3D_Models;
#else
	// Reasonable default flags
	Data.Flags = OGL_Flag_Map | OGL_Flag_LiqSeeThru | OGL_Flag_3D_Models;
#endif
	Data.VoidColor = rgb_black;			// Self-explanatory
	for (int il=0; il<4; il++)
		for (int ie=0; ie<2; ie++)
			Data.LscpColors[il][ie] = DefaultLscpColors[il][ie];
}


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
const int16 Specific_CLUT_Flag = 0x8000;
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
static void TODelete(int c)
{
	TOList[c].clear();
	TOHash[c].clear();
}

// Deletes all of them
static void TODeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) TODelete(c);
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
		bool Hash_SCS = (TEST_FLAG(HashVal,Specific_CLUT_Flag) != 0);
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


#ifdef HAVE_OPENGL

// Model-data stuff;
// defaults for whatever might need them
// Including skin stuff for convenience here
static OGL_ModelData DefaultModelData;
static OGL_SkinData DefaultSkinData;

// Store model-data stuff in a set of STL vectors
struct ModelDataEntry
{
	// Which sequence to apply to:
	// For models to be mapped to multiple sequences,
	// such as animated models,
	// this will need to be more complicated
	short Sequence;
	
	// Make a member for more convenient access
	OGL_ModelData ModelData;
	
	ModelDataEntry(): Sequence(NONE) {}
};


// Separate model-data sequence lists for each collection ID,
// to speed up searching
static vector<ModelDataEntry> MdlList[NUMBER_OF_COLLECTIONS];

// Model-data hash table for extra-fast searching:
static vector<int16> MdlHash[NUMBER_OF_COLLECTIONS];

// Hash-table size and function
const int MdlHashSize = 1 << 8;
const int MdlHashMask = TOHashSize - 1;
inline uint8 MdlHashFunc(short Sequence)
{
	// E-Z
	return (uint8)(Sequence & MdlHashMask);
}


// Deletes a collection's model-data sequences
static void MdlDelete(int c)
{
	MdlList[c].clear();
	MdlHash[c].clear();
}

// Deletes all of them
static void MdlDeleteAll()
{
	for (int c=0; c<NUMBER_OF_COLLECTIONS; c++) MdlDelete(c);
}

OGL_ModelData *OGL_GetModelData(short Collection, short Sequence)
{
	if (!OGL_IsActive()) return NULL;
	
	// Initialize the hash table if necessary
	if (MdlHash[Collection].empty())
	{
		MdlHash[Collection].resize(MdlHashSize);
		objlist_set(&MdlHash[Collection][0],NONE,MdlHashSize);
	}
	
	// Set up a *reference* to the appropriate hashtable entry;
	// this makes setting this entry a bit more convenient
	int16& HashVal = MdlHash[Collection][MdlHashFunc(Sequence)];
	
	// Check to see if the model-data entry is correct;
	// if it is, then we're done.
	if (HashVal != NONE)
	{
		vector<ModelDataEntry>::iterator MdlIter = MdlList[Collection].begin() + HashVal;
		if (MdlIter->Sequence == Sequence)
		{
			return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
		}
	}
	
	// Fallback for the case of a hashtable miss;
	// do a linear search and then update the hash entry appropriately.
	vector<ModelDataEntry>& ML = MdlList[Collection];
	int16 Indx = 0;
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++, Indx++)
	{
		if (MdlIter->Sequence == Sequence)
		{
			HashVal = Indx;
			return MdlIter->ModelData.ModelPresent() ? &MdlIter->ModelData : NULL;
		}
	}
	
	// None found!
	return NULL;
}

#endif // def HAVE_OPENGL


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
			return (strcmp(&Str1[0],&Str2[0]) == 0);
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


void OGL_TextureOptionsBase::Load()
{
	FileSpecifier File;
	
	// Load the normal image with alpha channel
	try
	{
		// Check to see if loading needs to be done;
		// it does not need to be if an image is present.
		if (NormalImg.IsPresent()) throw 0;
		NormalImg.Clear();
		
		// Load the normal image if it has a filename specified for it
		if (!StringPresent(NormalColors)) throw 0;
#ifdef mac
		if (!File.SetToApp()) throw 0;
#endif
		if (!File.SetNameWithPath(&NormalColors[0])) throw 0;
		if (!LoadImageFromFile(NormalImg,File,ImageLoader_Colors)) throw 0;
	}
	catch(...)
	{
		// A texture must have a normal colored part
		return;
	}
	
	try
	{
		// Load the normal mask if it has a filename specified for it
		if (!StringPresent(NormalMask)) throw 0;
#ifdef mac
		if (!File.SetToApp()) throw 0;
#endif
		if (!File.SetNameWithPath(&NormalMask[0])) throw 0;
		if (!LoadImageFromFile(NormalImg,File,ImageLoader_Opacity)) throw 0;
	}
	catch(...)
	{}
	
	// Load the glow image with alpha channel
	try
	{
		// Check to see if loading needs to be done;
		// it does not need to be if an image is present.
		if (GlowImg.IsPresent()) throw 0;
		GlowImg.Clear();
		
		// Load the glow image if it has a filename specified for it
		if (!StringPresent(GlowColors)) throw 0;
#ifdef mac
		if (!File.SetToApp()) throw 0;
#endif
		if (!File.SetNameWithPath(&GlowColors[0])) throw 0;
		if (!LoadImageFromFile(GlowImg,File,ImageLoader_Colors)) throw 0;
		
		// Load the glow mask if it has a filename specified for it;
		// only loaded if an image has been loaded for it
		if (!StringPresent(GlowMask)) throw 0;
#ifdef mac
		if (!File.SetToApp()) throw 0;
#endif
		if (!File.SetNameWithPath(&GlowMask[0])) throw 0;
		if (!LoadImageFromFile(GlowImg,File,ImageLoader_Opacity)) throw 0;
	}
	catch(...)
	{}
	
	// The rest of the code is made simpler by these constraints:
	// that the glow texture only be present if the normal texture is also present,
	// and that the normal and glow textures have the same dimensions
	if (NormalImg.IsPresent())
	{
		int W0 = NormalImg.GetWidth();
		int W1 = GlowImg.GetWidth();
		int H0 = NormalImg.GetHeight();
		int H1 = GlowImg.GetHeight();
		if ((W1 != W0) || (H1 != H0)) GlowImg.Clear();
	}
	else
	{
		GlowImg.Clear();
	}
}

void OGL_TextureOptionsBase::Unload()
{
	NormalImg.Clear();
	GlowImg.Clear();
}


#ifdef HAVE_OPENGL

// Any easy STL ways of doing this mapping of functions onto members of arrays?

void OGL_SkinManager::Load()
{
	for (vector<OGL_SkinData>::iterator SkinIter = SkinData.begin(); SkinIter < SkinData.end(); SkinIter++)
		SkinIter->Load();
}


void OGL_SkinManager::Unload()
{
	for (vector<OGL_SkinData>::iterator SkinIter = SkinData.begin(); SkinIter < SkinData.end(); SkinIter++)
		SkinIter->Unload();
}


void OGL_SkinManager::Reset(bool Clear_OGL_Txtrs)
{
	if (Clear_OGL_Txtrs)
	{
		for (int k=0; k<NUMBER_OF_OPENGL_BITMAP_SETS; k++)
			for (int l=0; l<NUMBER_OF_TEXTURES; l++)
			{
				if (IDsInUse[k][l])
					glDeleteTextures(1,&IDs[k][l]);
			}
	}
	
	// Mass clearing
	objlist_clear(IDsInUse[0],NUMBER_OF_OPENGL_BITMAP_SETS*NUMBER_OF_TEXTURES);
}


OGL_SkinData *OGL_SkinManager::GetSkin(short CLUT)
{
	for (int k=0; k<SkinData.size(); k++)
	{
		OGL_SkinData& Skin = SkinData[k];
		if (Skin.CLUT == CLUT || Skin.CLUT == ALL_CLUTS)
			return &Skin;
	}

	return NULL;
}


bool OGL_SkinManager::Use(short CLUT, short Which)
{
	// References so they can be written into
	GLuint& TxtrID = IDs[CLUT][Which];
	bool& InUse = IDsInUse[CLUT][Which];
	bool LoadSkin = false;
	if (!InUse)
	{
		glGenTextures(1,&TxtrID);
		InUse = true;
		LoadSkin = true;
	}
	glBindTexture(GL_TEXTURE_2D,TxtrID);
	return LoadSkin;
}


// Circle constants
const double TWO_PI = 8*atan(1.0);
const double Degree2Radian = TWO_PI/360;			// A circle is 2*pi radians

// Matrix manipulation (can't trust OpenGL to be active, so won't be using OpenGL matrix stuff)

// Src -> Dest
static void MatCopy(const GLfloat SrcMat[3][3], GLfloat DestMat[3][3])
{
	objlist_copy(DestMat[0],SrcMat[0],9);
}

// Sets the arg to that matrix
static void MatIdentity(GLfloat Mat[3][3])
{
	const GLfloat IMat[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	MatCopy(IMat,Mat);
}

// Src1, Src2 -> Dest (cannot be one of the sources)
static void MatMult(const GLfloat Mat1[3][3], const GLfloat Mat2[3][3], GLfloat DestMat[3][3])
{
	for (int k=0; k<3; k++)
		for (int l=0; l<3; l++)
		{
			GLfloat Sum = 0;
			for (int m=0; m<3; m++)
				Sum += Mat1[k][m]*Mat2[m][l];
			DestMat[k][l] = Sum;
		}
}

// Alters the matrix in place
static void MatScalMult(GLfloat Mat[3][3], const GLfloat Scale)
{
	for (int k=0; k<3; k++)
		for (int l=0; l<3; l++)
			Mat[k][l] *= Scale;
}

// Src -> Dest vector (cannot be the same location)
static void MatVecMult(const GLfloat Mat[3][3], const GLfloat *SrcVec, GLfloat *DestVec)
{
	for (int k=0; k<3; k++)
	{
		GLfloat Sum = 0;
		for (int l=0; l<3; l++)
			Sum += Mat[k][l]*SrcVec[l];
		DestVec[k] = Sum;
	}
}

void OGL_ModelData::Load()
{
	// Already loaded?
	if (ModelPresent()) return;
	
	// Load the model
	FileSpecifier File;
	Model.Clear();
	
	if (!StringPresent(ModelFile)) return;;
#ifdef mac
	if (!File.SetToApp()) return;
#endif
	if (!File.SetNameWithPath(&ModelFile[0])) return;

	bool Success = false;
	
	char *Type = &ModelType[0];
	if (StringsEqual(Type,"wave",4))
	{
		// Alias|Wavefront
		Success = LoadModel_Wavefront(File, Model);
	}
	else if (StringsEqual(Type,"3ds",3))
	{
		// 3D Studio Max
		Success = LoadModel_Studio(File, Model);
	}
#if defined(mac) && !defined(TARGET_API_MAC_CARBON)
	else if (StringsEqual(Type,"qd3d") || StringsEqual(Type,"3dmf") || StringsEqual(Type,"quesa"))
	{
		// QuickDraw 3D / Quesa
		Success = LoadModel_QD3D(File, Model);
	}
#endif
	
	if (!Success)
	{
		Model.Clear();
		return;
	}
	
	// Calculate transformation matrix
	GLfloat Angle, Cosine, Sine;
	GLfloat RotMatrix[3][3], NewRotMatrix[3][3], IndivRotMatrix[3][3];
	MatIdentity(RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = Degree2Radian*XRot;
	Cosine = cos(Angle);
	Sine = sin(Angle);
	IndivRotMatrix[1][1] = Cosine;
	IndivRotMatrix[1][2] = - Sine;
	IndivRotMatrix[2][1] = Sine;
	IndivRotMatrix[2][2] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = Degree2Radian*YRot;
	Cosine = cos(Angle);
	Sine = sin(Angle);
	IndivRotMatrix[2][2] = Cosine;
	IndivRotMatrix[2][0] = - Sine;
	IndivRotMatrix[0][2] = Sine;
	IndivRotMatrix[0][0] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatIdentity(IndivRotMatrix);
	Angle = Degree2Radian*ZRot;
	Cosine = cos(Angle);
	Sine = sin(Angle);
	IndivRotMatrix[0][0] = Cosine;
	IndivRotMatrix[0][1] = - Sine;
	IndivRotMatrix[1][0] = Sine;
	IndivRotMatrix[1][1] = Cosine;
	MatMult(IndivRotMatrix,RotMatrix,NewRotMatrix);
	MatCopy(NewRotMatrix,RotMatrix);
	
	MatScalMult(NewRotMatrix,Scale);			// For the position vertices
	if (Scale < 0) MatScalMult(RotMatrix,-1);	// For the normals
	
	int NumVerts = Model.Positions.size()/3;
	
	for (int k=0; k<NumVerts; k++)
	{
		GLfloat *Pos = Model.PosBase() + 3*k;
		GLfloat NewPos[3];
		MatVecMult(NewRotMatrix,Pos,NewPos);	// Has the scaling
		Pos[0] = NewPos[0] + XShift;
		Pos[1] = NewPos[1] + YShift;
		Pos[2] = NewPos[2] + ZShift;
	}
	
	int NumNorms = Model.Normals.size()/3;
	for (int k=0; k<NumNorms; k++)
	{
		GLfloat *Norms = Model.NormBase() + 3*k;
		GLfloat NewNorms[3];
		MatVecMult(RotMatrix,Norms,NewNorms);	// Not scaled
		objlist_copy(Norms,NewNorms,3);
	}	
	
	// Will need this to find bounding rectangles
	// and to normalize the normals
	Model.FindBoundingBox();
	Model.AdjustNormals(NormalType,NormalSplit);
	
	// Don't forget the skins
	OGL_SkinManager::Load();
}


void OGL_ModelData::Unload()
{
	Model.Clear();
	
	// Don't forget the skins
	OGL_SkinManager::Unload();
}



// for managing the model and image loading and unloading
void OGL_LoadModelsImages(int Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);
	
	// For wall/sprite images
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		// Load the images
		TOIter->OptionsData.Load();
		
		// Find adjusted-frame image-data positioning;
		// this is for doing sprites with textures with sizes different from the originals
		TOIter->OptionsData.FindImagePosition();
	}
	
	// For models
	bool UseModels = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_3D_Models) ? true : false;
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		if (UseModels)
			MdlIter->ModelData.Load();
		else
			MdlIter->ModelData.Unload();
	}
}

void OGL_UnloadModelsImages(int Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);
	
	// For wall/sprite images
	vector<TextureOptionsEntry>& TOL = TOList[Collection];
	for (vector<TextureOptionsEntry>::iterator TOIter = TOL.begin(); TOIter < TOL.end(); TOIter++)
	{
		TOIter->OptionsData.Unload();
	}
	
	// For models
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		MdlIter->ModelData.Unload();
	}
}


// Reset model skins; used in OGL_ResetTextures() in OGL_Textures.cpp
void OGL_ResetModelSkins(bool Clear_OGL_Txtrs)
{
	for (int ic=0; ic<MAXIMUM_COLLECTIONS; ic++)
	{
		vector<ModelDataEntry>& ML = MdlList[ic];
		for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
		{
			MdlIter->ModelData.Reset(Clear_OGL_Txtrs);
		}
	}
}

#else

void OGL_LoadModelsImages(int Collection)
{
}

void OGL_UnloadModelsImages(int Collection)
{
}

#endif // def HAVE_OPENGL


OGL_FogData *OGL_GetFogData(int Type)
{
	return GetMemberWithBounds(FogData,Type,OGL_NUMBER_OF_FOG_TYPES);
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
		int nchars = strlen(Value)+1;
		Data.NormalColors.resize(nchars);
		memcpy(&Data.NormalColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"normal_mask"))
	{
		int nchars = strlen(Value)+1;
		Data.NormalMask.resize(nchars);
		memcpy(&Data.NormalMask[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_image"))
	{
		int nchars = strlen(Value)+1;
		Data.GlowColors.resize(nchars);
		memcpy(&Data.GlowColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_mask"))
	{
		int nchars = strlen(Value)+1;
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

static XML_TextureOptionsParser TextureOptionsParser;


#ifdef HAVE_OPENGL

class XML_SkinDataParser: public XML_ElementParser
{
	short CLUT;
	
	OGL_SkinData Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	vector<OGL_SkinData> *SkinDataPtr;
	
	XML_SkinDataParser(): XML_ElementParser("skin"), SkinDataPtr(NULL) {}
};

bool XML_SkinDataParser::Start()
{
	Data = DefaultSkinData;
	
	return true;
}

bool XML_SkinDataParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"clut"))
	{
		return ReadBoundedInt16Value(Value,Data.CLUT,short(ALL_CLUTS),short(SILHOUETTE_BITMAP_SET));
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
	else if (StringsEqual(Tag,"normal_image"))
	{
		int nchars = strlen(Value)+1;
		Data.NormalColors.resize(nchars);
		memcpy(&Data.NormalColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"normal_mask"))
	{
		int nchars = strlen(Value)+1;
		Data.NormalMask.resize(nchars);
		memcpy(&Data.NormalMask[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_image"))
	{
		int nchars = strlen(Value)+1;
		Data.GlowColors.resize(nchars);
		memcpy(&Data.GlowColors[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"glow_mask"))
	{
		int nchars = strlen(Value)+1;
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
	UnrecognizedTag();
	return false;
}

bool XML_SkinDataParser::AttributesDone()
{
	// Check to see if a frame is already accounted for
	assert(SkinDataPtr);
	vector<OGL_SkinData>& SkinData = *SkinDataPtr;
	for (vector<OGL_SkinData>::iterator SDIter = SkinData.begin(); SDIter < SkinData.end(); SDIter++)
	{
		if (SDIter->CLUT == Data.CLUT)
		{
			// Replace the data
			*SDIter = Data;
			return true;
		}
	}
	
	// If not, then add a new frame entry
	SkinData.push_back(Data);
	
	return true;
}

static XML_SkinDataParser SkinDataParser;


class XML_MdlClearParser: public XML_ElementParser
{
	bool IsPresent;
	short Collection;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MdlClearParser(): XML_ElementParser("model_clear") {}
};

bool XML_MdlClearParser::Start()
{
	IsPresent = false;
	return true;
}

bool XML_MdlClearParser::HandleAttribute(const char *Tag, const char *Value)
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

bool XML_MdlClearParser::AttributesDone()
{
	if (IsPresent)
		MdlDelete(Collection);
	else
		MdlDeleteAll();
	
	return true;
}

static XML_MdlClearParser Mdl_ClearParser;


class XML_ModelDataParser: public XML_ElementParser
{
	bool CollIsPresent, SeqIsPresent;
	short Collection, Sequence;
	
	OGL_ModelData Data;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	bool End();
	
	XML_ModelDataParser(): XML_ElementParser("model") {}
};

bool XML_ModelDataParser::Start()
{
	Data = DefaultModelData;
	CollIsPresent = SeqIsPresent = false;
	
	// For doing the model skins
	SkinDataParser.SkinDataPtr = &Data.SkinData;
		
	return true;
}

bool XML_ModelDataParser::HandleAttribute(const char *Tag, const char *Value)
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
	else if (StringsEqual(Tag,"seq"))
	{
		if (ReadBoundedInt16Value(Value,Sequence,0,MAXIMUM_SHAPES_PER_COLLECTION-1))
		{
			SeqIsPresent = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"scale"))
	{
		return ReadFloatValue(Value,Data.Scale);
	}
	else if (StringsEqual(Tag,"x_rot"))
	{
		return ReadFloatValue(Value,Data.XRot);
	}
	else if (StringsEqual(Tag,"y_rot"))
	{
		return ReadFloatValue(Value,Data.YRot);
	}
	else if (StringsEqual(Tag,"z_rot"))
	{
		return ReadFloatValue(Value,Data.ZRot);
	}
	else if (StringsEqual(Tag,"x_shift"))
	{
		return ReadFloatValue(Value,Data.XShift);
	}
	else if (StringsEqual(Tag,"y_shift"))
	{
		return ReadFloatValue(Value,Data.YShift);
	}
	else if (StringsEqual(Tag,"z_shift"))
	{
		return ReadFloatValue(Value,Data.ZShift);
	}
	else if (StringsEqual(Tag,"side"))
	{
		return ReadInt16Value(Value,Data.Sidedness);
	}
	else if (StringsEqual(Tag,"norm_type"))
	{
		return ReadBoundedInt16Value(Value,Data.NormalType,0,Model3D::NUMBER_OF_NORMAL_TYPES-1);
	}
	else if (StringsEqual(Tag,"norm_split"))
	{
		return ReadFloatValue(Value,Data.NormalSplit);
	}
	else if (StringsEqual(Tag,"light_type"))
	{
		return ReadBoundedInt16Value(Value,Data.LightType,0,NUMBER_OF_MODEL_LIGHT_TYPES-1);
	}
	else if (StringsEqual(Tag,"depth_type"))
	{
		return ReadInt16Value(Value,Data.DepthType);
	}
	else if (StringsEqual(Tag,"file"))
	{
		int nchars = strlen(Value)+1;
		Data.ModelFile.resize(nchars);
		memcpy(&Data.ModelFile[0],Value,nchars);
		return true;
	}
	else if (StringsEqual(Tag,"type"))
	{
		int nchars = strlen(Value)+1;
		Data.ModelType.resize(nchars);
		memcpy(&Data.ModelType[0],Value,nchars);
		return true;
	}
	UnrecognizedTag();
	return false;
}

bool XML_ModelDataParser::AttributesDone()
{
	// Verify...
	if (!CollIsPresent || !SeqIsPresent)
	{
		AttribsMissing();
		return false;
	}
	return true;
}

bool XML_ModelDataParser::End()
{
	// Do this at the end because the model data will then include the skin data
	
	// Check to see if a frame is already accounted for
	vector<ModelDataEntry>& ML = MdlList[Collection];
	for (vector<ModelDataEntry>::iterator MdlIter = ML.begin(); MdlIter < ML.end(); MdlIter++)
	{
		if (MdlIter->Sequence == Sequence)
		{
			// Replace the data
			MdlIter->ModelData = Data;
			return true;
		}
	}
	
	// If not, then add a new frame entry
	ModelDataEntry DataEntry;
	DataEntry.Sequence = Sequence;
	DataEntry.ModelData = Data;
	ML.push_back(DataEntry);
	
	return true;
}

static XML_ModelDataParser ModelDataParser;

#endif // def HAVE_OPENGL

class XML_FogParser: public XML_ElementParser
{
	bool IsPresent[2];
	bool FogPresent;
	float Depth;
	short Type;
	
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
	
	XML_FogParser(): XML_ElementParser("fog") {}
};

bool XML_FogParser::Start()
{
	IsPresent[0] = IsPresent[1] = false;
	Type = 0;
	return true;
}

bool XML_FogParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag,"on"))
	{
		if (ReadBooleanValueAsBool(Value,FogPresent))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"depth"))
	{
		if (ReadFloatValue(Value,Depth))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	else if (StringsEqual(Tag,"type"))
	{
		return ReadBoundedInt16Value(Value,Type,0,OGL_NUMBER_OF_FOG_TYPES-1);
	}
	UnrecognizedTag();
	return false;
}

bool XML_FogParser::AttributesDone()
{
	OGL_FogData& Data = FogData[Type];
	if (IsPresent[0]) Data.IsPresent = FogPresent;
	if (IsPresent[1]) Data.Depth = Depth;
	Color_SetArray(&Data.Color);	
	return true;
}

static XML_FogParser FogParser;

static XML_ElementParser OpenGL_Parser("opengl");


// XML-parser support
XML_ElementParser *OpenGL_GetParser()
{
	OpenGL_Parser.AddChild(&TextureOptionsParser);
	OpenGL_Parser.AddChild(&TO_ClearParser);
	
#ifdef HAVE_OPENGL
	ModelDataParser.AddChild(&SkinDataParser);
	OpenGL_Parser.AddChild(&ModelDataParser);
	OpenGL_Parser.AddChild(&Mdl_ClearParser);
#endif
	
	FogParser.AddChild(Color_GetParser());
	OpenGL_Parser.AddChild(&FogParser);
	
	return &OpenGL_Parser;
}

