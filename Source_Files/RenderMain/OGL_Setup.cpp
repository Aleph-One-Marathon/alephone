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

Feb 5, 2002 (Br'fin (Jeremy Parsons)):
	Refined OGL default preferences for Carbon
*/

#include <vector>
#include <string.h>
#include <math.h>
#include "cseries.h"

#ifdef HAVE_OPENGL

#include "OGL_Headers.h"
#include "OGL_Shader.h"

#endif

#include "shape_descriptors.h"
#include "OGL_Setup.h"
#include "OGL_LoadScreen.h"
#include "progress.h"
#include "InfoTree.h"

// Whether or not OpenGL is present and usable
static bool _OGL_IsPresent = false;

bool Using_sRGB = false;
bool Wanting_sRGB = false;
bool Bloom_sRGB = false;
bool FBO_Allowed = false;
bool npotTextures = false; // non-power-of-two

// Initializer
bool OGL_Initialize()
{
#ifdef HAVE_OPENGL
#if defined(__WIN32__)
//	glewInit();
#endif	

	return _OGL_IsPresent = true;
#else
	return false;
#endif
}

// Test for presence
bool OGL_IsPresent() {return _OGL_IsPresent;}

bool OGL_CheckExtension(const std::string extension) {
#ifdef HAVE_OPENGL
#ifdef __WIN32__
	return glewIsSupported(extension.c_str());
#else
	char *extensions = (char *) glGetString(GL_EXTENSIONS);
	if (!extensions) return false;

	while (*extensions)
	{
		unsigned int length = strcspn(extensions, " ");

		if (length == extension.size() && 
		    strncmp(extension.c_str(), extensions, length) == 0) {
			return true;
		}

		extensions += length + 1;
	}
#endif
#endif
	return false;
}

static int ogl_progress;
static int total_ogl_progress;
static bool show_ogl_progress = false;
static int32 last_update_tick;

extern bool OGL_ClearScreen();

#ifdef HAVE_OPENGL
void OGL_StartProgress(int total_progress)
{
	ogl_progress = 0;
	total_ogl_progress = total_progress;
	if (!OGL_LoadScreen::instance()->Start())
	{
		OGL_ClearScreen();
		open_progress_dialog(_loading, true);
	}
	show_ogl_progress = true;
	last_update_tick = machine_tick_count();
}

void OGL_ProgressCallback(int delta_progress)
{
	if (!show_ogl_progress) return;
	ogl_progress += delta_progress;
	{
		int32 current_ticks = machine_tick_count();
		if (current_ticks > last_update_tick + 33)
		{
			if (OGL_LoadScreen::instance()->Use())
				OGL_LoadScreen::instance()->Progress(100 * ogl_progress / total_ogl_progress);
			else
				draw_progress_bar(ogl_progress, total_ogl_progress);
			last_update_tick = current_ticks;
		}
	}
}

void OGL_StopProgress()
{
	show_ogl_progress = false;
	if (OGL_LoadScreen::instance()->Use())
		OGL_LoadScreen::instance()->Stop();
	else
		close_progress_dialog();
}
#endif

// Sensible defaults for the fog:
static OGL_FogData FogData[OGL_NUMBER_OF_FOG_TYPES] = 
{
	{{0x8000,0x8000,0x8000},8,false,true},
	{{0x8000,0x8000,0x8000},8,false,true}
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
		if (k == OGL_Txtr_Wall || k == OGL_Txtr_Inhabitant)
			TxtrData.FarFilter = 5;		// GL_LINEAR_MIPMAP_LINEAR
		else
			TxtrData.FarFilter = 1;		// GL_LINEAR
		TxtrData.Resolution = 0;		// 1x
		TxtrData.ColorFormat = 0;		// 32-bit color
		TxtrData.MaxSize = 0;                   // Unlimited
	}

	Data.ModelConfig.NearFilter = 1;
	Data.ModelConfig.FarFilter = 5;
	Data.ModelConfig.Resolution = 0;
	Data.ModelConfig.ColorFormat = 0;
	Data.ModelConfig.MaxSize = 0;
	
	// Reasonable default flags
	Data.Flags = OGL_Flag_Fader | OGL_Flag_Map |
		OGL_Flag_HUD | OGL_Flag_LiqSeeThru | OGL_Flag_3D_Models | OGL_Flag_ZBuffer |
		OGL_Flag_Fog;

        Data.AnisotropyLevel = 0.0; // off
	Data.Multisamples = 0; // off
	
	Data.VoidColor = rgb_black;			// Self-explanatory
	for (int il=0; il<4; il++)
		for (int ie=0; ie<2; ie++)
			Data.LscpColors[il][ie] = DefaultLscpColors[il][ie];

	Data.GeForceFix = false;
	Data.WaitForVSync = true;
	Data.Use_sRGB = false;
	Data.Use_NPOT = false;
}


inline bool StringPresent(vector<char>& String)
{
	return (String.size() > 1);
}

#ifdef HAVE_OPENGL

GLint glMaxTextureSize = 0;
bool hasS3TC = false;

void OGL_TextureOptionsBase::Load()
{
	FileSpecifier File;

	GLint maxTextureSize = glMaxTextureSize;
	if (GetMaxSize())
	{
		maxTextureSize = MIN(maxTextureSize, GetMaxSize());
	}
	
	int flags = npotTextures ? 0 : ImageLoader_ResizeToPowersOfTwo;
		
	if (Type >= 0 && Type < OGL_NUMBER_OF_TEXTURE_TYPES && Get_OGL_ConfigureData().TxtrConfigList[Type].FarFilter > 1 /* GL_LINEAR */)
	{
			flags |= ImageLoader_LoadMipMaps;
	}

	if (hasS3TC) 
	{
		flags |= ImageLoader_CanUseDXTC;
	}

	if (Get_OGL_ConfigureData().GeForceFix)
	{
		flags |= ImageLoader_LoadDXTC1AsDXTC3;
	}
	
	// Load the normal image with alpha channel

	// Check to see if loading needs to be done;
	// it does not need to be if an image is present.
	if (NormalImg.IsPresent()) return;

	NormalImg.Clear();
	
	// Load the normal image if it has a filename specified for it
	if (NormalColors != FileSpecifier() && NormalColors.Exists())
	{
		if (!NormalImg.LoadFromFile(NormalColors,ImageLoader_Colors, flags | (NormalIsPremultiplied ? ImageLoader_ImageIsAlreadyPremultiplied : 0), actual_width, actual_height, maxTextureSize))
		{
			// A texture must have a normal colored part
			return;
		}
	}
	else
	{
		return;
	}

	// load a heightmap
	if(OffsetMap != FileSpecifier() && OffsetMap.Exists()) {
		if(!OffsetImg.LoadFromFile(OffsetMap, ImageLoader_Colors, flags | (NormalIsPremultiplied ? ImageLoader_ImageIsAlreadyPremultiplied : 0), actual_width, actual_height, maxTextureSize)) {
			return;
		}
	}

	// Load the normal mask if it has a filename specified for it
	if (NormalMask != FileSpecifier() && NormalMask.Exists())
	{
		NormalImg.LoadFromFile(NormalMask,ImageLoader_Opacity, flags, actual_width, actual_height, maxTextureSize);
	}

	if (maxTextureSize)
	{
		while (NormalImg.GetWidth() > maxTextureSize || NormalImg.GetHeight() > maxTextureSize)
		{
			if (!NormalImg.Minify()) break;
		}
		
		if(OffsetImg.IsPresent()) {
			while (OffsetImg.GetWidth() > maxTextureSize || OffsetImg.GetHeight() > maxTextureSize) {
				if(!OffsetImg.Minify()) { break; }
			}
		}
	}
	
	// Load the glow image with alpha channel
	if (!GlowImg.IsPresent())
	{
		GlowImg.Clear();
		
		// Load the glow image if it has a filename specified for it
		if (GlowColors != FileSpecifier() && GlowColors.Exists())
		{
			if (GlowImg.LoadFromFile(GlowColors,ImageLoader_Colors, flags | (GlowIsPremultiplied ? ImageLoader_ImageIsAlreadyPremultiplied : 0), actual_width, actual_height, maxTextureSize))
			{
		
				// Load the glow mask if it has a
				// filename specified for it; only
				// loaded if an image has been loaded
				// for it
				if (GlowMask != FileSpecifier() && GlowMask.Exists())
				{
					GlowImg.LoadFromFile(GlowMask,ImageLoader_Opacity, flags, actual_width, actual_height, maxTextureSize);
				}
			}
		}
	}
	
	if (GlowImg.IsPresent() && maxTextureSize)
	{
		while (GlowImg.GetWidth() > maxTextureSize || GlowImg.GetHeight() > maxTextureSize) 
		{
			if (!GlowImg.Minify()) break;
		}
	}

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
	OffsetImg.Clear();
}

int OGL_TextureOptionsBase::GetMaxSize()
{
	if (Type >= 0 && Type < OGL_NUMBER_OF_TEXTURE_TYPES)
	{
		return Get_OGL_ConfigureData().TxtrConfigList[Type].MaxSize;
	}
	else
		return 0; // Unlimited
}
#endif

#ifdef HAVE_OPENGL

int OGL_CountModelsImages(short Collection)
{
	return OGL_CountTextures(Collection) + OGL_CountModels(Collection);
}

// for managing the model and image loading and unloading
void OGL_LoadModelsImages(short Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTextureSize);
	hasS3TC = OGL_CheckExtension("GL_ARB_texture_compression") && OGL_CheckExtension("GL_EXT_texture_compression_s3tc");
	
	// For wall/sprite images
	OGL_LoadTextures(Collection);
	
	// For models, skins
	bool UseModels = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_3D_Models) ? true : false;
	if (UseModels)
		OGL_LoadModels(Collection);
	else
		OGL_UnloadModels(Collection);
}

void OGL_UnloadModelsImages(short Collection)
{
	assert(Collection >= 0 && Collection < MAXIMUM_COLLECTIONS);
	
	// For wall/sprite images
	OGL_UnloadTextures(Collection);
	
	// For models, skins
	OGL_UnloadModels(Collection);
}

#else

void OGL_LoadModelsImages(short)
{
}

void OGL_UnloadModelsImages(short)
{
}

#endif // def HAVE_OPENGL


OGL_FogData *OGL_GetFogData(int Type)
{
	return GetMemberWithBounds(FogData,Type,OGL_NUMBER_OF_FOG_TYPES);
}


// XML-parsing stuff
OGL_FogData *OriginalFogData = NULL;

void reset_mml_opengl()
{
	reset_mml_opengl_texture();
	reset_mml_opengl_model();
	reset_mml_opengl_shader();
	
	if (OriginalFogData) {
		for (unsigned i = 0; i < OGL_NUMBER_OF_FOG_TYPES; i++)
			FogData[i] = OriginalFogData[i];
		free(OriginalFogData);
		OriginalFogData = NULL;
	}
}

void parse_mml_opengl(const InfoTree& root)
{
	// back up old values first
	if (!OriginalFogData) {
		OriginalFogData = (OGL_FogData *) malloc(sizeof(OGL_FogData) * OGL_NUMBER_OF_FOG_TYPES);
		assert(OriginalFogData);
		for (unsigned i = 0; i < OGL_NUMBER_OF_FOG_TYPES; i++)
			OriginalFogData[i] = FogData[i];
	}

	// texture options / clear, in order
	BOOST_FOREACH(const InfoTree::value_type &v, root)
	{
		if (v.first == "texture")
			parse_mml_opengl_texture(v.second);
		else if (v.first == "txtr_clear")
			parse_mml_opengl_txtr_clear(v.second);
	}
	
	// model data / clear, in order
	BOOST_FOREACH(const InfoTree::value_type &v, root)
	{
		if (v.first == "model")
			parse_mml_opengl_model(v.second);
		else if (v.first == "model_clear")
			parse_mml_opengl_model_clear(v.second);
	}
	
	BOOST_FOREACH(InfoTree shader, root.children_named("shader"))
	{
		parse_mml_opengl_shader(shader);
	}
	
	BOOST_FOREACH(InfoTree fog, root.children_named("fog"))
	{
		int16 type = 0;
		fog.read_indexed("type", type, OGL_NUMBER_OF_FOG_TYPES);
		OGL_FogData& def = FogData[type];
		
		fog.read_attr("on", def.IsPresent);
		fog.read_attr("depth", def.Depth);
		fog.read_attr("landscapes", def.AffectsLandscapes);
		
		BOOST_FOREACH(InfoTree color, fog.children_named("color"))
		{
			color.read_color(def.Color);
		}
	}
}

#ifdef HAVE_OPENGL
/* These don't belong here */
void SglColor3f(GLfloat r, GLfloat g, GLfloat b) {
  GLfloat ov[3] = {sRGB_frob(r), sRGB_frob(g), sRGB_frob(b)};
  glColor3fv(ov);
}

void SglColor3fv(const GLfloat* iv) {
  GLfloat ov[3] = {sRGB_frob(iv[0]), sRGB_frob(iv[1]), sRGB_frob(iv[2])};
  glColor3fv(ov);
}

void SglColor3ub(GLubyte r, GLubyte g, GLubyte b) {
  GLfloat ov[3] = {sRGB_frob(r*(1.f/255.f)), sRGB_frob(g*(1.f/255.f)), sRGB_frob(b*(1.f/255.f))};
  glColor3fv(ov);
}

void SglColor3us(GLushort r, GLushort g, GLushort b) {
  GLfloat ov[3] = {sRGB_frob(r*(1.f/65535.f)), sRGB_frob(g*(1.f/65535.f)), sRGB_frob(b*(1.f/65535.f))};
  glColor3fv(ov);
}

void SglColor3usv(const GLushort* iv) {
  GLfloat ov[3] = {sRGB_frob(iv[0]*(1.f/65535.f)), sRGB_frob(iv[1]*(1.f/65535.f)), sRGB_frob(iv[2]*(1.f/65535.f))};
  glColor3fv(ov);
}

void SglColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  GLfloat ov[4] = {sRGB_frob(r), sRGB_frob(g), sRGB_frob(b), a};
  glColor4fv(ov);
}

void SglColor4fv(const GLfloat* iv) {
  GLfloat ov[4] = {sRGB_frob(iv[0]), sRGB_frob(iv[1]), sRGB_frob(iv[2]), iv[3]};
  glColor4fv(ov);
}

void SglColor4usv(const GLushort* iv) {
  GLfloat ov[4] = {sRGB_frob(iv[0]*(1.f/65535.f)), sRGB_frob(iv[1]*(1.f/65535.f)), sRGB_frob(iv[2]*(1.f/65535.f)), iv[3]*(1.f/65535.f)};
  glColor4fv(ov);
}
#endif
