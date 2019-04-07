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

	This contains functions intended to interface OpenGL 3D-rendering code
	with the rest of the Marathon source code.
	
	Much of the setup code is cribbed from the Apple GLUT code, or at least inspired by it.
	
	Late April, 2000:
	
	Moved texture stuff out to OGL_Textures.c/h
	
	Added wall-texture glow mapping.
	
	May 14, 2000:
	
	Added George Marsaglia's random-number generator
	
	May 24, 2000:
	
	Added view-control landscape-options support;
	also fixed a bug in the landscape scaling -- it is now close to the software-rendering
	scaling.
	
	May 27, 2000:
	
	Added vertical-wall-texture idiot-proofing for texture vectors -- don't render
	if horizontal or vertical texture vectors have zero length.
	
	Added support for flat static effect
	
	Added partial transparency of textures (IsBlended)
	
June 11, 2000:
	
	Added support of IsSeeThrough flag for polygons
	a texture overlaid on other visible textures is see-through,
	while one overlaid on the void is not
	
	Removed TRANSPARENT_BIT test as irrelevant
	
	Made semitransparency optional if the void is on one side of the texture

July 7, 2000:
	
	Calculated center correctly in OGL_RenderCrosshairs()
	
Jul 8, 2000:

	Modified OGL_SetView() so that one can control whether to allocate a back buffer to draw in
	Modified OGL_Copy2D() so that one can control which buffer (front or back)

Jul 9, 2000:

	Turned BeginFrame() and EndFrame() into OGL_StartMain() and OGL_EndMain()
	
	Also, grabbed some display list ID's for the fonts with glGenLists();
	this makes it unnecessary to hardcode their ID's. Also, grabbed a display list ID
	for a text string; this makes it easier to repeat its rendering.
	Calling OGL_ResetMapFonts() near there -- it resets the font-info cache for the overhead map
	
Jul 17, 2000:
	Reorganized the fog setting a bit; now it's set at the beginning of ecah frame.
	That ought to make it easier for stuff like Pfhortran to change it.

Aug 10, 2000:
	Changed the fog handling so that the fog preferences get consulted only once,
	when an OpenGL context is created. This will make it easier to change
	the fog color and depth on the fly. Also, the presence flag, the depth, and the color
	were made nonstatic so that Pfhortran can see them.

Sep 21, 2000:
	Added partial transparency to static mode

Oct 13, 2000 (Loren Petrich)
	Converted the animated-texture accounting into Standard Template Library vectors

Nov 18, 2000 (Loren Petrich):
	Added support for landscape vertical repeats

Dec 17, 2000 (Loren Petrich):
	Moved fog parameters into OGL_Setup.cpp;
	changed "fog is on" in preferences to "fog is allowed"
	Added "current fog color" so that landscapes will be correctly colored
	in infravision mode.

Jan 31, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for AGL.h
	Added accessors for datafields now opaque in Carbon
	Added a check to make sure AGL_SWAP_RECT is enabled before we try to disable it, trying to squash a bug that occasionally pops up

Feb 3, 2002 (Br'fin (Jeremy Parsons) and Loren Petrich):
	Centered OpenGL displays under Carbon OS X
	Fixed AGL_SWAP_RECT spamming of OS X console
	
Dec 13, 2002 (Loren Petrich):
	Added initial preloading of textures to avoid lazy loading of wall textures
	on start/restore of level
        
Feb 1, 2003 (Woody Zenfell):
        Trying to reduce texture-preloading time by eliminating redundant processing

April 22, 2003 (Woody Zenfell):
        Macs can try using aglSetFullScreen() rather than aglSetDrawable() (experimental_rendering)

May 3, 2003 (Br'fin (Jeremy Parsons))
	Added LowLevelShape workaround for passing LowLevelShape info of sprites
	instead of abusing/overflowing shape_descriptors
*/

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <set>
#include <algorithm>	// pair<>, for_each()

#include "cseries.h"
#include "world.h"
#include "shell.h"
#include "preferences.h"

#ifdef HAVE_OPENGL

#include "OGL_Headers.h"

#include "interface.h"
#include "render.h"
#include "map.h"
#include "player.h"
#include "OGL_Render.h"
#include "OGL_Textures.h"
#include "OGL_Blitter.h"
#include "AnimatedTextures.h"
#include "Crosshairs.h"
#include "VecOps.h"
#include "Random.h"
#include "ViewControl.h"
#include "OGL_Faders.h"
#include "ModelRenderer.h"
#include "Logging.h"
#include "screen.h"
#include "OGL_Shader.h"

#include <cmath>

extern bool use_lua_hud_crosshairs;

// Whether or not OpenGL is active for rendering
static bool _OGL_IsActive = false;


// Reads off of the current map;
// call it to avoid lazy loading of textures
typedef std::pair<shape_descriptor,int16> TextureWithTransferMode;
static void PreloadTextures();
static void PreloadWallTexture(const TextureWithTransferMode& inTexture);


// Was OpenGL just inited? If so, then some state may need changing
static bool JustInited = false;

// The various boundary rectangles (all of the screen, and the view)
static Rect SavedScreenBounds = {0,0,0,0};
static Rect SavedViewBounds = {0,0,0,0};

// For fixing some of the vertices
short ViewWidth, ViewHeight;

// Adjust the coordinates because those exactly on the right and bottom edges
// are sometimes troublesome; they can cause surface polygons to drop out
inline short Adjust_X(short x) {return PIN(x,1,ViewWidth-1);}
inline short Adjust_Y(short y) {return PIN(y,1,ViewHeight-1);}

/*
	Coordinate systems: there are several that we must deal with here.
	
	Marathon world coordinates: x and y are horizontal; z is vertical
	Origin is map origin; orientation is world;
	extent is biggest short
	
	Marathon leveled-world coordinates: x and y are horizontal; z is vertical
	Origin is intended map origin (horizontal) and viewpoint location(vertical);
	due to an engine bug, the origin's horizontal location is in error;
	orientation is world;
	extent is biggest short
	
	Marathon centered-world coordinates: x and y are horizontal; z is vertical
	Origin is viewpoint location;
	orientation is world;
	extent is biggest short
		
	Marathon-style eye coordinates: x is outward, y is rightward, and z is upward
	Origin is viewpoint location;
	orientation is viewpoint;
	extent is biggest short
	
	OpenGL-style eye coordinates: x is rightward, y is upward, and z is inward
	Origin is viewpoint location; extent is biggest short
	
	Screen coordinates: x is rightward, y is downward, and z is inward
	Origin is top left corner of the screen; extent is screen, z between -1 and 1
	
	OpenGL fundamental (clip) coordinates: x is rightward, y is upward, and z is inward
	Everything is clipped to a cube that is -1 to +1 in all the coordinates.
*/

// Marathon centered world -> Marathon eye
static GLdouble CenteredWorld_2_MaraEye[16];
// Marathon world -> Marathon eye
static GLdouble World_2_MaraEye[16];
// Marathon eye -> OpenGL eye (good for handling vertical-surface data)
static const GLdouble MaraEye_2_OGLEye[16] =
{	// Correct OpenGL arrangement: transpose to get usual arrangement
	0,	0,	-1,	0,
	1,	0,	0,	0,
	0,	1,	0,	0,
	0,	0,	0,	1
};

// World -> OpenGL eye (good modelview matrix for 3D-model inhabitants)
static GLdouble World_2_OGLEye[16];
// Centered world -> OpenGL eye (good modelview matrix for 3D-model skyboxes)
// (also good for handling horizontal-surface data)
static GLdouble CenteredWorld_2_OGLEye[16];

// Screen -> clip (good starter matrix; assumes distance is already projected)
GLdouble Screen_2_Clip[16];
// OpenGL eye -> clip (good projection matrix for 3D models)
static GLdouble OGLEye_2_Clip[16];
// OpenGL eye -> screen
static GLdouble OGLEye_2_Screen[16];


// Projection-matrix management: select the appropriate one for what to render
enum {
	Projection_NONE,
	Projection_OpenGL_Eye,	// Appropriate for anything with depth
	Projection_Screen		// Appropriate for anything that's flat on the screen
};
static int ProjectionType = Projection_NONE;

static void SetProjectionType(int NewProjectionType)
{
	if (NewProjectionType == ProjectionType) return;

	switch(NewProjectionType)
	{
	case Projection_OpenGL_Eye:
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(OGLEye_2_Clip);
		ProjectionType = NewProjectionType;
		break;
	
	case Projection_Screen:
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(Screen_2_Clip);
		ProjectionType = NewProjectionType;
		break;
	}
}


// Rendering depth extent: minimum and maximum z
const GLdouble Z_Near = 50;
const GLdouble Z_Far = 1.5*64*WORLD_ONE;

// Projection coefficients for depth
const GLdouble Z_Proj0 = (Z_Far + Z_Near)/(Z_Far - Z_Near);
const GLdouble Z_Proj1 = 2*Z_Far*Z_Near/(Z_Far - Z_Near);

// Screen <-> world conversion factors and functions
GLdouble XScale, YScale, XScaleRecip, YScaleRecip, XOffset, YOffset;

// This adjusts a point position in place, using Adjust_X and Adjust_Y
// (intended to correct for exactly-on-edge bug)
inline void AdjustPoint(point2d& Pt)
{
	Pt.x = Adjust_X(Pt.x);
	Pt.y = Adjust_Y(Pt.y);
}

// This produces a ray in OpenGL eye coordinates (z increasing inward);
// it sets the point position to its adjusted value
inline void Screen2Ray(point2d& Pt, GLdouble* Ray)
{
	AdjustPoint(Pt);
	Ray[0] = XScaleRecip*(Pt.x - XOffset);
	Ray[1] = YScaleRecip*(Pt.y - YOffset);
	Ray[2] = -1;
}


// Surface-coordinate management;
// does all necessary setup tasks for finding where a ray hits a surface
struct SurfaceCoords
{
	// Vectors for increase in a texture coordinate by 1:
	// these have a 4th coordinate, for the convenience of the OpenGL-matrix-multiply routines
	// that are used to create them. It is, however, ignored here.
	// U (along scanlines):
	GLdouble U_Vec[4];
	// V (scanline-to-scanline):
	GLdouble V_Vec[4];
	
	// Complement vectors: (vector).(complement vector) = 1 if for the same quantity, 0 otherwise
	// U (along scanlines):
	GLdouble U_CmplVec[3];
	// V (scanline-to-scanline):
	GLdouble V_CmplVec[3];
	// W (perpendicular to both)
	GLdouble W_CmplVec[3];
	
	// Find complement vectors; return whether the two input vectors were noncollinear
	bool FindComplements();
};

static SurfaceCoords HorizCoords, VertCoords;


// Circle constants
const double TWO_PI = 8*atan(1.0);
const double Radian2Circle = 1/TWO_PI;			// A circle is 2*pi radians
const double FullCircleReciprocal = 1/double(FULL_CIRCLE);


// Number of static-effect rendering passes
#define USE_STIPPLE_STATIC_EFFECT
#ifdef USE_STIPPLE_STATIC_EFFECT
// For stippling
const int StaticEffectPasses = 4;
const int SeparableStaticEffectPasses = 4;	// Because of all-or-nothing for all passes
#else
// for stenciling
const int StaticEffectPasses = 3;
const int SeparableStaticEffectPasses = 0;	// Do every model triangle separately for safety 
#endif

// Yaw angle (full circle = 1)
static double Yaw;

// Landscape rescaling to get closer to software-rendering scale
static double LandscapeRescale;


// Self-luminosity (the "miner's light" effect and weapons flare)
static _fixed SelfLuminosity;

// Pointer to current fog data:
OGL_FogData *CurrFog = NULL;

inline bool FogActive()
{
	if (!CurrFog) return false;
	bool FogAllowed = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fog);
	return CurrFog->IsPresent && FogAllowed;
}

// Current fog color; may be different from the fog color above because of infravision being on
static GLfloat CurrFogColor[4] = {0,0,0,0};

#ifndef USE_STIPPLE_STATIC_EFFECT
// For doing static effects with stenciling
static float StencilTxtrOpacity;
#endif

// Stipple patterns for that static look
// (3 color channels + 1 alpha channel) * 32*32 array of bits
const int StatPatLen = 32;
static GLuint StaticPatterns[4][StatPatLen];

// Alternative: partially-transparent flat static
static bool UseFlatStatic;
static uint16 FlatStaticColor[4];

// The randomizer for the static-effect pixels
static GM_Random StaticRandom;


// Function for setting up the rendering of a 3D model: scaling, clipping, etc.;
// returns whether or not the model could be rendered
static bool RenderModelSetup(rectangle_definition& RenderRectangle);

// Function for rendering a 3D model
// Returns whether or not the model could be rendered
// (lack of a skin appropriate for the CLUT, for example)
static bool RenderModel(rectangle_definition& RenderRectangle, short Collection, short CLUT);

// Does the lighting and blending setup;
// returns whether or not the texture can be glowmapped
// (not the case for infravision, invisible, static)
// it gets "IsBlended" off of the texture definition
// It returns in args
// the "true" blending (invisibility is blended)
// the color to use,
// and whether the object will be externally lit
static bool DoLightingAndBlending(rectangle_definition& RenderRectangle, bool& IsBlended,
	GLfloat *Color, bool& ExternallyLit);

// Setup and teardown for the static-effect mode
static void SetupStaticMode(int16 transfer_data);
static void TeardownStaticMode();

// Renderer object and its "base" view direction
static ModelRenderer ModelRenderObject;
GLfloat ViewDir[2];

// Shader lists for the object renderer
static ModelRenderShader StandardShaders[2];
static ModelRenderShader StaticModeShaders[4];

// Data for static-mode shader callback: which one in sequence
static int SequenceNumbers[4] = {0, 1, 2, 3};

// Contains everything that the shader callbacks will need
struct ShaderDataStruct
{
	OGL_ModelData *ModelPtr;
	OGL_SkinData *SkinPtr;
	GLfloat Color[4];
	short Collection, CLUT;
};
static ShaderDataStruct ShaderData;

// Shader callbacks for textures
void NormalShader(void *Data);
void GlowingShader(void *Data);
void StaticModeIndivSetup(int SeqNo);
void StaticModeShader(void *Data);

// External-lighting data (given to callback)
struct LightingDataStruct
{
	short Type;
	short ProjDistance;
	GLfloat *Dir;		// Direction to the "light point"
	GLfloat AvgLight;	// Average from all directions
	GLfloat LightDiff;	// Top-to-bottom difference
	GLfloat Opacity;	// For overall-semitransparent models
	
	// This is in 3 sets of 4 values:
	// R0, R1, R2, R3, G0, G1, G2, G3, B0, B1, B2, B3;
	// the color components are calculated with
	// R0*N0 + R1*N1 + R2*N2 + R3
	// G0*N0 + G1*N1 + G2*N2 + G3
	// B0*N0 + B1*N1 + B2*N2 + B3
	GLfloat Colors[3][4];
};
static LightingDataStruct LightingData;

// Shader callback for lighting
static void LightingCallback(void *Data, size_t NumVerts, GLfloat *Normals, GLfloat *Positions, GLfloat *Colors);

// Set up the shader data
static void SetupShaders();


// Remember the last blend set so as to avoid redundant blend resettings
static short BlendType = OGL_BlendType_Crossfade;

// Set the blend, being sure to remember the blend type set to
static void SetBlend(short _BlendType);


// This function returns whether OpenGL is active;
// if OpenGL is not present, it will never be active.

// Test for activity;
bool OGL_IsActive() { return MainScreenIsOpenGL(); }


// It will be black; whether OpenGL is active will be returned
bool OGL_ClearScreen()
{
	if (OGL_IsActive())
	{
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return true;
	}
	else return false;
}

void OGL_Rasterizer_Init();

// Start an OpenGL run (creates a rendering context)
bool OGL_StartRun()
{
	logContext("starting up OpenGL rendering");

	if (!OGL_IsPresent()) return false;

	// Will stop previous run if it had been active
	if (OGL_IsActive()) OGL_StopRun();

#ifdef __WIN32__
	glewInit();
#endif

	Wanting_sRGB = false;
	if(graphics_preferences->OGL_Configure.Use_sRGB) {
	  if(!OGL_CheckExtension("GL_EXT_framebuffer_sRGB") || !OGL_CheckExtension("GL_EXT_texture_sRGB"))
	  {
	    graphics_preferences->OGL_Configure.Use_sRGB = false;
	    logWarning("Gamma corrected blending is not available");
	  }
	  else
	    Wanting_sRGB = true;
	}

	npotTextures = false;
	if (graphics_preferences->OGL_Configure.Use_NPOT)
	{
	  if (!OGL_CheckExtension("GL_ARB_texture_non_power_of_two"))
	  {
	    graphics_preferences->OGL_Configure.Use_NPOT = false;
	    logWarning("Non-power-of-two textures are not available");
	  }
	  else
	    npotTextures = true;
	}

	FBO_Allowed = false;
	if (!OGL_CheckExtension("GL_EXT_framebuffer_object"))
	{
		logWarning("Framebuffer Objects not available");
		return false;
	}
	else
	{
		FBO_Allowed = true;
	}

	Bloom_sRGB = false;
	if (TEST_FLAG(graphics_preferences->OGL_Configure.Flags, OGL_Flag_Blur))
	{
	  if (!FBO_Allowed)
	  {
	    SET_FLAG(graphics_preferences->OGL_Configure.Flags, OGL_Flag_Blur, false);
	    logWarning("Bloom effects are not available");
	  }
	  else if(!OGL_CheckExtension("GL_EXT_framebuffer_sRGB") || !OGL_CheckExtension("GL_EXT_texture_sRGB"))
	  {
	    logWarning("sRGB framebuffer is not available for bloom effects");
	  }
	  else
	    Bloom_sRGB = true;
	}

	_OGL_IsActive = true;
	OGL_StartProgress(count_replacement_collections() + 2);

	// Set up some OpenGL stuff: these will be the defaults for this rendering context
	
	// Set up for Z-buffering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);
	
	// Prevent wrong-side polygons from being rendered;
	// this works because the engine's visibility routines make all world-geometry
	// polygons have the same sidedness when they are viewed from inside.
	// [DEFAULT]
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	
	// Note: GL_BLEND and GL_ALPHA_TEST do not have defaults; these are to be set
	// if some new pixels cannot be assumed to be always 100% opaque.
	
	// [DEFAULT]
	// Set standard alpha-test function; cut off at halfway point (for sharp edges)
	glAlphaFunc(GL_GREATER,0.5);
	
	// [DEFAULT]
	// Set standard crossfade blending function (for smooth transitions)
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	// Switch on use of vertex and texture-coordinate arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	OGL_Rasterizer_Init();
	
	OGL_ResetForceSpriteDepth();
	load_replacement_collections();	

	// Initialize the texture accounting
	OGL_StartTextures();

	// Reset the font info for OpenGL rendering
	FontSpecifier::OGL_ResetFonts(true);
	
	// Since an OpenGL context has just been created, don't try to clear any OpenGL textures
	OGL_ResetModelSkins(false);

	// Setup for 3D-model rendering
	ModelRenderObject.Clear();
	SetupShaders();
	OGL_ProgressCallback(1);

	// Avoid lazy initial texture loading
	PreloadTextures();
	OGL_ProgressCallback(1);
	OGL_StopProgress();

	// Success!
	JustInited = true;
	return (_OGL_IsActive = true);
}

// Stop an OpenGL run (destroys a rendering context)
bool OGL_StopRun()
{
	if (!OGL_IsActive() || !_OGL_IsActive) return false;
	
	OGL_StopTextures();
	Shader::unloadAll();
	
	Wanting_sRGB = false;
	
	_OGL_IsActive = false;
	return true;
}


// Reads off of the current map;
// call it to avoid lazy loading of textures
// ZZZ: changes to try to do less redundant work (using a set of pairs etc.)
void PreloadTextures()
{
	typedef std::set<TextureWithTransferMode> TextureWithTransferModeSet;

	TextureWithTransferModeSet theSetOfTexturesUsed;

	// Loop through the map polygons
	for (int n=0; n<dynamic_world->polygon_count; n++)
	{
		polygon_data *polygon = map_polygons + n;
		
		theSetOfTexturesUsed.insert(TextureWithTransferMode(polygon->floor_texture,polygon->floor_transfer_mode));
		theSetOfTexturesUsed.insert(TextureWithTransferMode(polygon->ceiling_texture,polygon->ceiling_transfer_mode));
		
		for (int i=0; i<polygon->vertex_count; i++)
		{
			short side_index= polygon->side_indexes[i];
			if (side_index == NONE) continue;
			side_data *side= get_side_data(side_index);
			switch (side->type)
			{
			case _full_side:
				theSetOfTexturesUsed.insert(TextureWithTransferMode(side->primary_texture.texture,side->primary_transfer_mode));
				break;
			case _split_side:
				theSetOfTexturesUsed.insert(TextureWithTransferMode(side->secondary_texture.texture,side->secondary_transfer_mode));
				// Fall through to the high-side case
			case _high_side:
				theSetOfTexturesUsed.insert(TextureWithTransferMode(side->primary_texture.texture,side->primary_transfer_mode));
				break;
			case _low_side:
				theSetOfTexturesUsed.insert(TextureWithTransferMode(side->primary_texture.texture,side->primary_transfer_mode));
				break;
			}
			
			theSetOfTexturesUsed.insert(TextureWithTransferMode(side->transparent_texture.texture,side->transparent_transfer_mode));
		}
	}

	// May want to preload the liquid texture also
	// Sprites will have the problem of guessing which ones to preload

	// ZZZ: now we have a fairly (we hope) minimal set of texture stuffs, let's load them in.
	for_each(theSetOfTexturesUsed.begin(), theSetOfTexturesUsed.end(), PreloadWallTexture);
}

void PreloadWallTexture(const TextureWithTransferMode& inTexture)
{
	shape_descriptor texture = inTexture.first;
	int16 transfer_mode = inTexture.second;

	// In case of an empty side
	if (texture == UNONE) return;

	// Infravision is usually inactive when entering or restoring a level.
	bool IsInfravision = false;

	TextureManager TMgr;
	TMgr.ShapeDesc = AnimTxtr_Translate(texture);
	if(TMgr.ShapeDesc == UNONE) return;
	
	get_shape_bitmap_and_shading_table(
		TMgr.ShapeDesc,
		&TMgr.Texture,
		&TMgr.ShadingTables,
		IsInfravision ? _shading_infravision : _shading_normal);
	if (!TMgr.Texture) return;

	TMgr.IsShadeless = IsInfravision;
	
	// Cribbed from instantiate_polygon_transfer_mode() in render.cpp;
	// translate the transfer mode
	int16 TMgr_TransferMode = _textured_transfer;
	switch (transfer_mode)
	{
	case _xfer_smear:
		TMgr_TransferMode = _solid_transfer;
		break;
		
	case _xfer_static:
		TMgr_TransferMode = _static_transfer;
		break;
	
	case _xfer_landscape:
		TMgr_TransferMode = _big_landscaped_transfer;
		break;
	}
	
	TMgr.TransferMode = TMgr_TransferMode;
	TMgr.TransferData = 0;
	
	// As in the code below, landscapes get special treatment
	bool IsLandscape = TMgr_TransferMode == _big_landscaped_transfer;
	TMgr.TextureType = IsLandscape ? OGL_Txtr_Landscape : OGL_Txtr_Wall;
	
	if (IsLandscape)
	{
		// Get the landscape-texturing options
		LandscapeOptions *LandOpts = View_GetLandscapeOptions(TMgr.ShapeDesc);	
		TMgr.LandscapeVertRepeat = LandOpts->VertRepeat;
		TMgr.Landscape_AspRatExp = LandOpts->OGL_AspRatExp;
	}
	
	// After all this setting up, now use it!
	if (TMgr.Setup()) {
		TMgr.RenderNormal();
		if (TMgr.IsGlowMapped()) TMgr.RenderGlowing();
		if (TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap))
			TMgr.RenderBump();
	}
}


inline bool RectsEqual(Rect &R1, Rect &R2)
{
	return (R1.top == R2.top) && (R1.left == R2.left) &&
		(R1.bottom == R2.bottom) && (R1.right == R2.right);
}

inline void DebugRect(Rect &R, char *Label)
{
	dprintf("%s (L,R,T,B): %d %d %d %d",Label,R.left,R.right,R.top,R.bottom);
}

// Set OpenGL rendering-window bounds;
// these are calculated using the following boundary Rects:
// The screen (gotten from its portRect)
// The view (here, the main rendering view)
// Whether to allocate a back buffer
bool OGL_SetWindow(Rect &ScreenBounds, Rect &ViewBounds, bool UseBackBuffer)
{
	if (!OGL_IsActive()) return false;
	
	// Check whether to do update -- only if the bounds had changed
	// or if the view had been inited
	bool DoUpdate = false;
	if (JustInited) {JustInited = false; DoUpdate = true;}
	else if (!RectsEqual(ScreenBounds,SavedScreenBounds)) DoUpdate = true;
	else if (!RectsEqual(ViewBounds,SavedViewBounds)) DoUpdate = true;
	else DoUpdate = true;
	
	if (!DoUpdate) return true;
	
	SavedScreenBounds = ScreenBounds;
	SavedViewBounds = ViewBounds;
	
	// Viewport setup is now done by the caller.
	ViewWidth = ViewBounds.right - ViewBounds.left;
	ViewHeight = ViewBounds.bottom - ViewBounds.top;
	
	// Create the screen -> clip (fundamental) matrix; this will be needed
	// for all the other projections
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX,Screen_2_Clip);
	
	// Set projection type to initially none (force load of first one)
	ProjectionType = Projection_NONE;
	
	return true;
}


bool OGL_StartMain()
{
	if (!OGL_IsActive()) return false;
	
	// One-sidedness necessary for correct rendering
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	
	// Set the Z-buffering for this go-around
	glEnable(GL_DEPTH_TEST);
	
	// Moved this test down here for convenience; the overhead map won't have fog,
	// so be sure to turn it on when leaving the overhead map
	// Also, added support for changing fog parameters on the fly,
	// by moving the setting of initial values to where the context gets created.
	int FogType = (local_player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ?
		OGL_Fog_BelowLiquid : OGL_Fog_AboveLiquid;
	CurrFog = OGL_GetFogData(FogType);
	if (FogActive())
	{
		glEnable(GL_FOG);
		Using_sRGB = Wanting_sRGB;
		CurrFogColor[0] = sRGB_frob(CurrFog->Color.red/65535.0F);
		CurrFogColor[1] = sRGB_frob(CurrFog->Color.green/65535.0F);
		CurrFogColor[2] = sRGB_frob(CurrFog->Color.blue/65535.0F);
		CurrFogColor[3] = 0;
		Using_sRGB = false;
		if (IsInfravisionActive())
		{
			if (LandscapesLoaded)
				FindInfravisionVersionRGBA(_collection_landscape1+static_world->song_index,CurrFogColor);
			else
				FindInfravisionVersionRGBA(LoadedWallTexture,CurrFogColor);
		}
		glFogfv(GL_FOG_COLOR,CurrFogColor);
		glFogf(GL_FOG_DENSITY,1.0F/MAX(1,WORLD_ONE*CurrFog->Depth));
	}
	else
	{
		glFogf(GL_FOG_DENSITY,0.0F);
		glDisable(GL_FOG);
	}
	
	// Set the color of the void
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	if (TEST_FLAG(ConfigureData.Flags,OGL_Flag_VoidColor))
	{
		RGBColor& VoidColor = ConfigureData.VoidColor;
		GLfloat Red = VoidColor.red/65535.0F;
		GLfloat Green = VoidColor.green/65535.0F;
		GLfloat Blue = VoidColor.blue/65535.0F;
		
		// The color of the void will be the color of fog
		if (FogActive())
		{
			Red = CurrFogColor[0];
			Green = CurrFogColor[1];
			Blue = CurrFogColor[2];
		}
		
		glClearColor(Red,Green,Blue,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	// Have to clear the Z-buffer before rendering, no matter what
	else glClear(GL_DEPTH_BUFFER_BIT);
	
	// Static patterns; randomize all digits; the various offsets
	// are to ensure that all the bits overlap.
	// Also do flat static if requested;
	// done once per frame to avoid visual inconsistencies
	UseFlatStatic = TEST_FLAG(ConfigureData.Flags,OGL_Flag_FlatStatic);
	
	if (Wanting_sRGB)
	{
		glEnable(GL_FRAMEBUFFER_SRGB_EXT);
		Using_sRGB = true;
	}
	
	return true;
}


bool OGL_EndMain()
{
	if (!OGL_IsActive()) return false;

	if (Wanting_sRGB)
	{
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		Using_sRGB = false;
	}
	
	// Proper projection
	SetProjectionType(Projection_Screen);
	
	// Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// No texture mapping now
	glDisable(GL_TEXTURE_2D);
	
	// And no Z buffer
	glDisable(GL_DEPTH_TEST);
	
	// Render OpenGL faders, if in use
	OGL_DoFades(0,0,ViewWidth,ViewHeight);
	
	return true;
}

// Swap buffers (reveal rendered image)
bool OGL_SwapBuffers()
{
	if (!OGL_IsActive()) return false;
	MainScreenSwap();
	return true;
}


// Find complement vectors; return whether the two input vectors were noncollinear
bool SurfaceCoords::FindComplements()
{
	// Compose the complements of the two texture vectors;
	// this code is designed to be general, and is probably overkill for the Marathon engine,
	// where the texture vectors are always orthogonal.
	GLdouble P_U2 = ScalarProd(U_Vec,U_Vec);
	GLdouble P_UV = ScalarProd(U_Vec,V_Vec);
	GLdouble P_V2 = ScalarProd(V_Vec,V_Vec);
	GLdouble P_Den = P_U2*P_V2 - P_UV*P_UV;
	
	// Will return here if the vectors are collinear
	if (P_Den == 0) return false;
	
	GLdouble Norm = 1/P_Den;
	GLdouble C_UU = Norm*P_V2;
	GLdouble C_UV = - Norm*P_UV;
	GLdouble C_VV = Norm*P_U2;
	
	GLdouble TempU[3], TempV[3];
	
	VecScalarMult(U_Vec,C_UU,TempU);
	VecScalarMult(V_Vec,C_UV,TempV);
	VecAdd(TempU,TempV,U_CmplVec);
	
	VecScalarMult(U_Vec,C_UV,TempU);
	VecScalarMult(V_Vec,C_VV,TempV);
	VecAdd(TempU,TempV,V_CmplVec);
	
	// Compose the third complement; don't bother to normalize it
	VectorProd(U_Vec,V_Vec,W_CmplVec);
	
	// Success!
	return true;
}


// Multiply a vector by an OpenGL matrix
inline void GL_MatrixTimesVector(const GLdouble *Matrix, const GLdouble *Vector, GLdouble *ResVec)
{
	for (int k = 0; k < 4; k++)
		ResVec[k] =
			Matrix[k]*Vector[0] +
			Matrix[4+k]*Vector[1] +
			Matrix[4*2+k]*Vector[2] +
			Matrix[4*3+k]*Vector[3];
}


// Set view parameters; this is for proper perspective rendering
bool OGL_SetView(view_data &View)
{
	if (!OGL_IsActive()) return false;
	
	// Use the modelview matrix as storage; set the matrix back when done
	glMatrixMode(GL_MODELVIEW);

	// World coordinates to Marathon eye coordinates
	glLoadIdentity();
	glGetDoublev(GL_MODELVIEW_MATRIX,CenteredWorld_2_MaraEye);
	
	// Do rotation first:
	const double TrigMagReciprocal = 1/double(TRIG_MAGNITUDE);
	double Cosine = TrigMagReciprocal*double(cosine_table[View.yaw]);
	double Sine = TrigMagReciprocal*double(sine_table[View.yaw]);
	CenteredWorld_2_MaraEye[0] = Cosine;
	CenteredWorld_2_MaraEye[1] = - Sine;
	CenteredWorld_2_MaraEye[4] = Sine;
	CenteredWorld_2_MaraEye[4+1] = Cosine;
	glLoadMatrixd(CenteredWorld_2_MaraEye);
	
	// Set the view direction
	ViewDir[0] = (float)Cosine;
	ViewDir[1] = (float)Sine;
	ModelRenderObject.ViewDirection[2] = 0;	// Always stays the same

	// Do a translation and then save;
	glTranslated(-View.origin.x,-View.origin.y,-View.origin.z);
	glGetDoublev(GL_MODELVIEW_MATRIX,World_2_MaraEye);
	
	// Find the appropriate modelview matrix for 3D-model inhabitant rendering
	glLoadMatrixd(MaraEye_2_OGLEye);
	glMultMatrixd(World_2_MaraEye);
	glGetDoublev(GL_MODELVIEW_MATRIX,World_2_OGLEye);
	
	// Find the appropriate modelview matrix for 3D-model skybox rendering
	glLoadMatrixd(MaraEye_2_OGLEye);
	glMultMatrixd(CenteredWorld_2_MaraEye);
	glGetDoublev(GL_MODELVIEW_MATRIX,CenteredWorld_2_OGLEye);
	
	// Find world-to-screen and screen-to-world conversion factors;
	// be sure to have some fallbacks in case of zero
	XScale = View.world_to_screen_x;
	if (XScale == 0) XScale = 1;
	XScaleRecip = 1/XScale;
	YScale = - View.world_to_screen_y;
	if (YScale == 0) YScale = -1;
	YScaleRecip = 1/YScale;
	XOffset = View.half_screen_width;
	YOffset = View.half_screen_height + View.dtanpitch;
	
	// Find the OGL-eye-to-screen matrix
	// Remember that z is small negative to large negative (OpenGL style)
	glLoadIdentity();
	glGetDoublev(GL_MODELVIEW_MATRIX,OGLEye_2_Screen);
	OGLEye_2_Screen[0] = XScale;
	OGLEye_2_Screen[4+1] = YScale;
	OGLEye_2_Screen[4*2] = - XOffset;
	OGLEye_2_Screen[4*2+1] = - YOffset;
	OGLEye_2_Screen[4*2+2] = Z_Proj0;
	OGLEye_2_Screen[4*2+3] = -1;
	OGLEye_2_Screen[4*3+2] = Z_Proj1;
	OGLEye_2_Screen[4*3+3] = 0;
		
	// Find the OGL-eye-to-clip matrix:
	glLoadMatrixd(Screen_2_Clip);
	glMultMatrixd(OGLEye_2_Screen);
	glGetDoublev(GL_MODELVIEW_MATRIX,OGLEye_2_Clip);
	
	// Restore the default modelview matrix
	glLoadIdentity();
	
	// Calculate the horizontal-surface projected-texture vectors
	GLdouble OrigVec[4];
	
	// Horizontal U
	OrigVec[0] = WORLD_ONE;
	OrigVec[1] = 0;
	OrigVec[2] = 0;
	OrigVec[3] = 0;
	GL_MatrixTimesVector(CenteredWorld_2_OGLEye,OrigVec,HorizCoords.U_Vec);
	
	// Horizontal V
	OrigVec[0] = 0;
	OrigVec[1] = WORLD_ONE;
	OrigVec[2] = 0;
	OrigVec[3] = 0;
	GL_MatrixTimesVector(CenteredWorld_2_OGLEye,OrigVec,HorizCoords.V_Vec);
	
	bool found_complements= HorizCoords.FindComplements();
	if(!found_complements) assert(found_complements);
	
	// Get the yaw angle as a value from 0 to 1
	Yaw = FullCircleReciprocal*View.yaw;
	
	// Set up landscape rescaling:
	// Find view angle in radians, then find the rescaling
	double ViewAngle = (TWO_PI*FullCircleReciprocal)*View.half_cone;
	LandscapeRescale = ViewAngle/tan(ViewAngle);
		
	// Is infravision active?
	IsInfravisionActive() = (View.shading_mode == _shading_infravision);
		
	// Finally...
	SelfLuminosity = View.maximum_depth_intensity;
	
	return true;
}


// Sets the view to what's suitable for rendering foreground objects
// like weapons in hand
bool OGL_SetForeground()
{
	// Foreground objects are to be in front of all the other ones
	glClear(GL_DEPTH_BUFFER_BIT);
	
	// New renderer needs modelview reset
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// Disable sRGB mode
	if (Wanting_sRGB)
	{
		glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		Using_sRGB = false;
	}
	
	return true;
}


// Sets whether a foreground object is horizontally reflected
bool OGL_SetForegroundView(bool HorizReflect)
{
	// x is rightward (OpenGL: x is rightward)
	// y is forward (OpenGL: y is upward)
	// z is upward (OpenGL: z is backward)
	const GLdouble Foreground_2_OGLEye[16] =
	{	// Correct OpenGL arrangement: transpose to get usual arrangement
		1,	0,	0,	0,
		0,	0,	1,	0,
		0,	-1,	0,	0,
		0,	0,	0,	1
	};

	// Find the appropriate modelview matrix for 3D-model inhabitant rendering
	glLoadMatrixd(Foreground_2_OGLEye);
	glGetDoublev(GL_MODELVIEW_MATRIX,World_2_OGLEye);
	
	// Perform the reflection if desired; refer to above definition of Foreground_2_OGLEye
	if (HorizReflect) World_2_OGLEye[0] = -1;
	
	// Restore the default modelview matrix
	glLoadIdentity();
	
	return true;
}


// Self-luminosity calculations;
// cribbed from scottish_textures.c (CALCULATE_SHADING_TABLE):

// This finds the intensity-slope crossover depth for splitting polygon lines;
// it takes the shading value from the render object
inline GLdouble FindCrossoverDepth(_fixed Shading)
{
	return ((8*GLdouble(WORLD_ONE))/GLdouble(FIXED_ONE))*(SelfLuminosity - Shading);
}


// This finds the color value for lighting from the render object's shading value
void FindShadingColor(GLdouble Depth, _fixed Shading, GLfloat *Color)
{
	GLdouble SelfIllumShading =
		PIN(SelfLuminosity - (GLdouble(FIXED_ONE)/(8*GLdouble(WORLD_ONE)))*Depth,0,FIXED_ONE);
	
	GLdouble CombinedShading = (Shading>SelfIllumShading) ? (Shading + 0.5*SelfIllumShading) : (SelfIllumShading + 0.5*Shading);

	Color[0] = Color[1] = Color[2] = sRGB_frob(PIN(static_cast<GLfloat>(CombinedShading/FIXED_ONE),0,1));
}


// Stuff for doing OpenGL rendering of various objects

	
// Storage of intermediate results for mass render with glDrawArrays
struct ExtendedVertexData
{
	GLdouble Vertex[4];
	GLdouble TexCoord[2];
	GLfloat Color[3];
	GLfloat GlowColor[3];
};


// Wraparound increment and decrementfunctions
inline int IncrementAndWrap(int n, int Limit)
	{int m = n + 1; if (m >= Limit) m -= Limit; return m;}
inline int DecrementAndWrap(int n, int Limit)
	{int m = n - 1; if (m < 0) m += Limit; return m;}


// The depth must be in OpenGL form (increasing inward);
// the other arguments are: the two source and one destination extended vertex
static void InterpolateByDepth(GLdouble Depth,
	ExtendedVertexData& EV0,
	ExtendedVertexData& EV1,
	ExtendedVertexData& EVRes)
{
	GLdouble Denom = EV1.Vertex[2] - EV0.Vertex[2];
	assert(Denom != 0);
	
	GLdouble IntFac = (Depth - EV0.Vertex[2])/Denom;
	
	for (int k=0; k<4; k++)
		EVRes.Vertex[k] = EV0.Vertex[k] + IntFac*(EV1.Vertex[k] - EV0.Vertex[k]);
	
	for (int k=0; k<2; k++)
		EVRes.TexCoord[k] = EV0.TexCoord[k] + IntFac*(EV1.TexCoord[k] - EV0.TexCoord[k]);
}


// Render the wall texture as a "real" wall;
// it returns whether or not the texture is a legitimate wall texture
static bool RenderAsRealWall(polygon_definition& RenderPolygon, bool IsVertical)
{
	
	// Set up the texture manager with the input manager
	TextureManager TMgr;
	TMgr.ShapeDesc = RenderPolygon.ShapeDesc;
	TMgr.ShadingTables = RenderPolygon.shading_tables;
	TMgr.Texture = RenderPolygon.texture;
	TMgr.TransferMode = RenderPolygon.transfer_mode;
	TMgr.TransferData = RenderPolygon.transfer_data;
	TMgr.IsShadeless = (RenderPolygon.flags&_SHADELESS_BIT) != 0;
	if (RenderPolygon.transfer_mode == _static_transfer)
		TMgr.IsShadeless = 1;
	TMgr.TextureType = OGL_Txtr_Wall;
	
	// Use that texture
	if (!TMgr.Setup()) return false;
			
	// The currently-used surface-coordinate object
	SurfaceCoords* SCPtr;
	
	// A workspace vector
	GLdouble OrigVec[4];
	
	// Calculate the projected origin and texture coordinates
	if (IsVertical)
	{
		// Set to vertical ones
		SCPtr = &VertCoords;
		
		// Calculate its texture vectors
		world_vector3d& Vec = RenderPolygon.vector;
		
		// Idiot-proofing; don't render a polygon its vertical texture vector
		// has either horizontal or vertical parts being zero.
		
		// Vertical U
		if (Vec.k == 0) return false;
		OrigVec[0] = 0;
		OrigVec[1] = 0;
		OrigVec[2] = Vec.k;
		OrigVec[3] = 0;
		GL_MatrixTimesVector(MaraEye_2_OGLEye,OrigVec,VertCoords.U_Vec);
		
		// Vertical V
		if (Vec.i == 0 && Vec.j == 0) return false;
		OrigVec[0] = Vec.i;
		OrigVec[1] = Vec.j;
		OrigVec[2] = 0;
		OrigVec[3] = 0;
		GL_MatrixTimesVector(MaraEye_2_OGLEye,OrigVec,VertCoords.V_Vec);
	
		bool found_complements= VertCoords.FindComplements();
		if(!found_complements) assert(found_complements);
		
	} else {
		// Set to horizontal ones
		SCPtr = &HorizCoords;
	}
	
	// Find the texture origin in OpenGL eye coordinates
	long_point3d& Origin = RenderPolygon.origin;
	OrigVec[0] = Origin.x;
	OrigVec[1] = Origin.y;
	OrigVec[2] = Origin.z;
	OrigVec[3] = 1;
	GLdouble TexOrigin[4];
	if (IsVertical)
		GL_MatrixTimesVector(MaraEye_2_OGLEye,OrigVec,TexOrigin);
	else
	{
		// The inversion is a kludge for getting around an engine bug
		OrigVec[0] *= -1;
		OrigVec[1] *= -1;
		GL_MatrixTimesVector(CenteredWorld_2_OGLEye,OrigVec,TexOrigin);
	}
	
	// Project it onto the coordinate vectors
	GLdouble TexOrigin_U = ScalarProd(SCPtr->U_CmplVec,TexOrigin);
	GLdouble TexOrigin_V = ScalarProd(SCPtr->V_CmplVec,TexOrigin);
	GLdouble TexOrigin_W = ScalarProd(SCPtr->W_CmplVec,TexOrigin);
	
	// Storage of intermediate results for mass render;
	// take into account the fact that the polygon might get split in both ascending
	// and descending directions along three lines
	const int MAXIMUM_VERTICES_OF_SPLIT_POLYGON = MAXIMUM_VERTICES_PER_SCREEN_POLYGON + 6;
	ExtendedVertexData ExtendedVertexList[MAXIMUM_VERTICES_OF_SPLIT_POLYGON];

	short NumVertices = RenderPolygon.vertex_count;
	for (int k=0; k<NumVertices; k++)
	{
		// Create some convenient references
		point2d& Vertex = RenderPolygon.vertices[k];
		ExtendedVertexData& EVData = ExtendedVertexList[k];
		
		// Emit a ray from the vertex in OpenGL eye coords;
		// it had been specified in screen coordinates
		GLdouble VertexRay[3];
		Screen2Ray(Vertex,VertexRay);
		
		// Project it:
		GLdouble VertexRay_U = ScalarProd(SCPtr->U_CmplVec,VertexRay);
		GLdouble VertexRay_V = ScalarProd(SCPtr->V_CmplVec,VertexRay);
		GLdouble VertexRay_W = ScalarProd(SCPtr->W_CmplVec,VertexRay);
		
		// Find the distance along the ray;
		// watch out for excessively long or negative distances;
		// force them to the maximum Z allowed.
		// This is done because the screen coordinates of the area to be rendered
		// has been rounded off to integers, which may cause off-the-edge errors.
		GLdouble RayDistance = 0;
		bool RayDistanceWasModified = false;
		if (VertexRay_W == 0)
		{
			RayDistanceWasModified = true;
			RayDistance = Z_Far;
		}
		else
		{
			RayDistance = TexOrigin_W/VertexRay_W;
			// Test for possible wraparound
			if (RayDistance < - 16*WORLD_ONE)
			{
				RayDistanceWasModified = true;
				RayDistance = Z_Far;
			}
			// Test for too far
			else if (RayDistance > Z_Far)
			{
				RayDistanceWasModified = true;
				RayDistance = Z_Far;
			}
			// Test for too close
			else if (RayDistance < Z_Near)
			{
				RayDistance = Z_Near;
				RayDistanceWasModified = true;
			}
		}
		
		// Find the texture coordinates
		GLdouble U = VertexRay_U*RayDistance - TexOrigin_U;
		GLdouble V = VertexRay_V*RayDistance - TexOrigin_V;
		
		if (RayDistanceWasModified)
		{
			// Rebuild the vertex here.
			// This is necessary here, since if the ray distance was modified,
			// the vertex will be forced to move on the screen.
			GLdouble TempU[3], TempV[3], TempUV[3];
			VecScalarMult(SCPtr->U_Vec,U,TempU);
			VecScalarMult(SCPtr->V_Vec,V,TempV);
			VecAdd(TempU,TempV,TempUV);
			VecAdd(TexOrigin,TempUV,EVData.Vertex);
			EVData.Vertex[3] = TexOrigin[3];
		
		} else {
			// Project along the ray
			VecScalarMult(VertexRay,RayDistance,EVData.Vertex);
			EVData.Vertex[3] = TexOrigin[3];
		}
				
		// Store the texture coordinates
		EVData.TexCoord[0] = U;
		EVData.TexCoord[1] = V;
	}
	
	// Does the polygon have a variable shading over its extent?
	bool PolygonVariableShade = false;
	GLfloat GlowColor = TMgr.MinGlowIntensity();
	
	if (TMgr.IsShadeless)
	{
		// The shadeless color is E-Z
		glColor3f(1,1,1);
		GlowColor = 1;
	}
	else if (RenderPolygon.ambient_shade < 0)
	{
		GLfloat Light = (- RenderPolygon.ambient_shade)/GLfloat(FIXED_ONE);
		SglColor3f(Light,Light,Light);
		GlowColor = std::max(GlowColor, Light);
	}
	else
	{
		// All this stuff is to do the self-luminosity properly,
		// like how software rendering does it.
	
		// Divide the polygon along these lines;
		// these mark out the self-luminosity boundaries.
		// Be sure to use OpenGL depth conventions
		GLdouble SplitDepths[3];
		// This is where the lighting reaches ambient
		SplitDepths[0] = - FindCrossoverDepth(0);
		// This is where the decline slope changes
		SplitDepths[1] = - FindCrossoverDepth(RenderPolygon.ambient_shade);
		// This is where the lighting gets saturated
		SplitDepths[2] = - FindCrossoverDepth(FIXED_ONE - (RenderPolygon.ambient_shade>>1));
		
		// Check to see if all of the polygon is outside the variable-lighting domain;
		// set the lighting value appropriately if that is the case
		
		// Is the polygon all in the ambient domain?
		GLfloat Light = RenderPolygon.ambient_shade/GLfloat(FIXED_ONE);
		for (int k=0; k<NumVertices; k++)
		{
			ExtendedVertexData& EV = ExtendedVertexList[k];
			if (EV.Vertex[2] > SplitDepths[0])
			{
				PolygonVariableShade = true;
				break;
			}
		}
		
		if (!PolygonVariableShade)
		{
			// Is the polygon all in the saturated domain?
			Light = 1;
			for (int k=0; k<NumVertices; k++)
			{
				ExtendedVertexData& EV = ExtendedVertexList[k];
				if (EV.Vertex[2] < SplitDepths[2])
				{
					PolygonVariableShade = true;
					break;
				}
			}
		}
		
		if (PolygonVariableShade)
		{
			// If the saturation point is outward from the crossover point,
			// make the crossover point the saturation point and the
			// original saturation point out-of-bounds
			if (SplitDepths[2] < SplitDepths[1])
			{
				SplitDepths[1] = SplitDepths[2];
				SplitDepths[2] = 0;
			}
			
			// Find split points:
			// These go in order: ascending split depths (0, 1, 2),
			// then descending split depths (2, 1, 0)
			// Locations of the splits
			int Splits[6];
			for (int m=0; m<6; m++)
				Splits[m] = NONE;
			// Interpolated values
			ExtendedVertexData SplitVertices[6];
			
			// Find ascending splits
			for (int m=0; m<3; m++)
			{
				GLdouble SplitDepth = SplitDepths[m];
				for (int k=0; k<NumVertices; k++)
				{
					ExtendedVertexData& EV0 = ExtendedVertexList[k];
					ExtendedVertexData& EV1 = ExtendedVertexList[IncrementAndWrap(k,NumVertices)];
					if (EV0.Vertex[2] < SplitDepth && EV1.Vertex[2] > SplitDepth)
					{
						Splits[m] = k;
						InterpolateByDepth(SplitDepth,EV0,EV1,SplitVertices[m]);
						break;
					}
				}
			}
			
			// Find descending splits
			for (int m=0; m<3; m++)
			{
				GLdouble SplitDepth = SplitDepths[m];
				for (int k=0; k<NumVertices; k++)
				{
					ExtendedVertexData& EV0 = ExtendedVertexList[k];
					ExtendedVertexData& EV1 = ExtendedVertexList[IncrementAndWrap(k,NumVertices)];
					if (EV0.Vertex[2] > SplitDepth && EV1.Vertex[2] < SplitDepth)
					{
						Splits[5-m] = k;
						InterpolateByDepth(SplitDepth,EV0,EV1,SplitVertices[5-m]);
						break;
					}
				}
			}
			
			// Insert the new vertices into place; first, create a vertex-source list
			int VertexSource[MAXIMUM_VERTICES_OF_SPLIT_POLYGON];
			for (int k=0; k<NumVertices; k++)
				VertexSource[k] = k;
			
			// Now work backward, so that the inserted vertices will be in the proper order;
			// the inserted ones are mapped 0..5 to -1..-6
			bool PolygonSplit = false;
			for (int m=5; m>=0; m--)
			{
				int Split = Splits[m];
				// If no split, then...
				if (Split == NONE) continue;
				// A split occurred; notify the rest of the function
				PolygonSplit = true;
				// Find the split location
				int SplitLoc = NONE;
				for (int k=0; k<NumVertices; k++)
				{
					if (VertexSource[k] == Split)
					{
						SplitLoc = k;
						break;
					}
				}
				assert(SplitLoc != NONE);
				
				// Move up all those past the split;
				// be sure to go backwards so as to move them correctly.
				for (int k=NumVertices-1; k>SplitLoc; k--)
					VertexSource[k+1] = VertexSource[k];
				
				// Put the split one into place
				VertexSource[SplitLoc+1] = -m-1;
				
				// Bump up the number of vertices, since one has been added
				NumVertices++;
			}
			
			// Now, remap
			if (PolygonSplit)
			{
				// Go backwards, since the vertices have been pushed upwards
				for (int k=NumVertices-1; k>=0; k--)
				{
					int Src = VertexSource[k];
					if (Src >= 0)
						ExtendedVertexList[k] = ExtendedVertexList[Src];
					else
						ExtendedVertexList[k] = SplitVertices[-Src-1];
				}
			}
			
			// Set up for vertex lighting
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_FLOAT,sizeof(ExtendedVertexData),ExtendedVertexList[0].Color);
			
			// Calculate the lighting
			for (int k=0; k<NumVertices; k++)
			{
				FindShadingColor(-ExtendedVertexList[k].Vertex[2],RenderPolygon.ambient_shade,ExtendedVertexList[k].Color);
				ExtendedVertexList[k].GlowColor[0] = ExtendedVertexList[k].GlowColor[1] = ExtendedVertexList[k].GlowColor[2] = std::max(GlowColor, ExtendedVertexList[k].Color[0]);
			}
		}
		else
		{
			SglColor3f(Light,Light,Light);
			GlowColor = std::max(GlowColor, Light);
		}
	}
	
	// Set up blending mode: either sharp edges or opaque
	// Added support for partial-opacity blending of wall textures
	// Added support for suppressing semitransparency when the void is on one side;
	// this suppression is optional for those who like weird smearing effects
	bool IsBlended = TMgr.IsBlended();
	if (!RenderPolygon.VoidPresent || TMgr.VoidVisible())
	{
		if (IsBlended)
		{
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
		}
	} else {
		// Completely opaque if can't see through void
		IsBlended = false;
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
	
	// Proper projection
	SetProjectionType(Projection_OpenGL_Eye);
	
	// Location of data:
	glVertexPointer(4,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	
	// Painting a texture...
	glEnable(GL_TEXTURE_2D);

	TMgr.SetupTextureMatrix();
	TMgr.RenderNormal();	
	if (TMgr.TransferMode == _static_transfer)
	{
		SetupStaticMode(IsBlended ? TMgr.TransferData : 0);
		if (!IsBlended)
			glDisable(GL_BLEND);

		if (UseFlatStatic)
		{
			glDrawArrays(GL_POLYGON,0,NumVertices);
		} else {
			// Do multitextured stippling to create the static effect
			for (int k=0; k<StaticEffectPasses; k++)
			{
				StaticModeIndivSetup(k);
				glDrawArrays(GL_POLYGON,0,NumVertices);
			}
		}
		TeardownStaticMode();
	}
	else
	{
	SetBlend(TMgr.NormalBlend());
	
	GLint VertIndices[3*(MAXIMUM_VERTICES_OF_SPLIT_POLYGON - 2)];
	if (PolygonVariableShade)
	{
		// Do triangulation by hand, creating a sort of ladder;
		// this is to avoid creating a triangle fan
		
		// Find minimum and maximum depths:
		GLint MinVertex = 0;
		GLint MaxVertex = 0;
		GLdouble MinDepth = ExtendedVertexList[MinVertex].Vertex[2];
		GLdouble MaxDepth = ExtendedVertexList[MaxVertex].Vertex[2];
		
		for (int k=0; k<NumVertices; k++)
		{
			// Create some convenient references
			ExtendedVertexData& EVData = ExtendedVertexList[k];
			
			GLdouble Depth = EVData.Vertex[2];
			if (Depth < MinDepth)
			{
				MinDepth = Depth;
				MinVertex = k;
			}
			if (Depth > MaxDepth)
			{
				MaxDepth = Depth;
				MaxVertex = k;
			}
		}
		
		// Now create the ladder
		GLint *VIPtr = VertIndices;
		
		// FInd the two neighboring vertices
		GLint LeftVertex = DecrementAndWrap(MinVertex,NumVertices);
		GLint RightVertex = IncrementAndWrap(MinVertex,NumVertices);
		
		// Place in the triangle; be sure to keep the proper order
		*(VIPtr++) = LeftVertex;
		*(VIPtr++) = MinVertex;
		*(VIPtr++) = RightVertex;
		
		// We know how many triangles there will be: NumVertices-2,
		// and we have already found one of them.
		for (int k=0; k<(NumVertices-3); k++)
		{
			if (LeftVertex == MaxVertex)
			{
				// Idiot-proofing; if the left vertex had reached the maximum,
				// the right vertex ought not to be there
				assert(RightVertex != MaxVertex);
				
				// Advance the right vertex
				GLint NewRightVertex = IncrementAndWrap(RightVertex,NumVertices);
				*(VIPtr++) = LeftVertex;
				*(VIPtr++) = RightVertex;
				*(VIPtr++) = NewRightVertex;
				RightVertex = NewRightVertex;
			}
			else if (RightVertex == MaxVertex)
			{	
				// Advance the left vertex
				GLint NewLeftVertex = DecrementAndWrap(LeftVertex,NumVertices);
				*(VIPtr++) = NewLeftVertex;
				*(VIPtr++) = LeftVertex;
				*(VIPtr++) = RightVertex;
				LeftVertex = NewLeftVertex;
			}
			else
			{
				// Right minus left depth
				GLdouble RLDiff = ExtendedVertexList[RightVertex].Vertex[2]
					- ExtendedVertexList[LeftVertex].Vertex[2];
				if (RLDiff < 0)
				{
					// Left vertex ahead; advance the right one
					GLint NewRightVertex = IncrementAndWrap(RightVertex,NumVertices);
					*(VIPtr++) = LeftVertex;
					*(VIPtr++) = RightVertex;
					*(VIPtr++) = NewRightVertex;
					RightVertex = NewRightVertex;
				}
				else if (RLDiff > 0)
				{
					// Right vertex ahead; advance the left one
					GLint NewLeftVertex = DecrementAndWrap(LeftVertex,NumVertices);
					*(VIPtr++) = NewLeftVertex;
					*(VIPtr++) = LeftVertex;
					*(VIPtr++) = RightVertex;
					LeftVertex = NewLeftVertex;
				}
				else
				{
					// Advance to the closest one
					GLint NewLeftVertex = DecrementAndWrap(LeftVertex,NumVertices);
					GLint NewRightVertex = IncrementAndWrap(RightVertex,NumVertices);
					RLDiff = ExtendedVertexList[NewRightVertex].Vertex[2]
						- ExtendedVertexList[NewLeftVertex].Vertex[2];
					if (RLDiff > 0)
					{
						// Left one is closer
						*(VIPtr++) = NewLeftVertex;
						*(VIPtr++) = LeftVertex;
						*(VIPtr++) = RightVertex;
						LeftVertex = NewLeftVertex;
					}
					else
					{
						// Right one is closer
						*(VIPtr++) = LeftVertex;
						*(VIPtr++) = RightVertex;
						*(VIPtr++) = NewRightVertex;
						RightVertex = NewRightVertex;
					}
				}
			}
		}
		
		// Now, go!
		glDrawElements(GL_TRIANGLES,3*(NumVertices-2),GL_UNSIGNED_INT,VertIndices);
		
		// Switch off
		glDisableClientState(GL_COLOR_ARRAY);
	}
	else
		// Go!
		// Don't care about triangulation here, because the polygon never got split
		glDrawArrays(GL_POLYGON,0,NumVertices);
	
	// Do textured rendering
	if (TMgr.IsGlowMapped())
	{
		// Do blending here to get the necessary semitransparency;
		// push the cutoff down so 0.5*0.5 (half of half-transparency)
		// The cutoff is irrelevant if the texture is set to one of the blended modes
		// instead of the crisp mode.
		// Added "IsBlended" test, so that alpha-channel selection would work properly
		// on a glowmap texture that is atop a texture that is opaque to the void.
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		
		TMgr.RenderGlowing();
		SetBlend(TMgr.GlowBlend());
		
		if (PolygonVariableShade)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_FLOAT,sizeof(ExtendedVertexData),ExtendedVertexList[0].GlowColor);
			glDrawElements(GL_TRIANGLES,3*(NumVertices-2),GL_UNSIGNED_INT,VertIndices);
			glDisableClientState(GL_COLOR_ARRAY);
		}
		else
		{
			SglColor3f(GlowColor,GlowColor,GlowColor);
			glDrawArrays(GL_POLYGON,0,NumVertices);
		}
	}
	}
	
	// Revert to default blend
	SetBlend(OGL_BlendType_Crossfade);
	glEnable(GL_DEPTH_TEST);
	TMgr.RestoreTextureMatrix();

	return true;
}


// Render the wall texture as a landscape;
// it returns whether or not the texture is a legitimate landscape texture
// and it does not care about the surface's orientation
static bool RenderAsLandscape(polygon_definition& RenderPolygon)
{
	// Check for fog
	bool IsActive = FogActive();
	bool AffectsLandscapes = IsActive ? CurrFog->AffectsLandscapes : false;
	if (AffectsLandscapes)
	{
		// Render as fog at infinity
		glDisable(GL_FOG);
		glDisable(GL_TEXTURE_2D);
		
		// Set up the color
		glColor3fv(CurrFogColor);
		
		// Set up blending mode: opaque
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		
		// Proper projection
		SetProjectionType(Projection_Screen);
		
		// Load an array of vertices
		struct AltExtendedVertexData {
			short Vertex[3];
		} AltEVList[MAXIMUM_VERTICES_PER_SCREEN_POLYGON];
		
		short NumVertices = RenderPolygon.vertex_count;
		for (int k=0; k<NumVertices; k++)
		{
			// Create convenient references
			point2d& Vertex = RenderPolygon.vertices[k];
			AltExtendedVertexData& AltEV = AltEVList[k];
			
			AdjustPoint(Vertex);
			AltEV.Vertex[0] = Vertex.x;
			AltEV.Vertex[1] = Vertex.y;
			AltEV.Vertex[2] = -1;			// At positive oo
		}
		// Fog is flat-colored
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);		
		glVertexPointer(3,GL_SHORT,sizeof(AltExtendedVertexData),AltEVList[0].Vertex);
		
		// Go!
		glDrawArrays(GL_POLYGON,0,NumVertices);
		
		// Restore
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_FOG);
		
		return true;
	}
	
	// Otherwise, the landscape would get fogged as a function of its world-geometry location
	if (IsActive) glDisable(GL_FOG);

	// Get the landscape-texturing options
	LandscapeOptions *LandOpts = View_GetLandscapeOptions(RenderPolygon.ShapeDesc);
	
	// Adjusted using the texture azimuth (yaw)
	double AdjustedYaw = Yaw + FullCircleReciprocal*(LandOpts->Azimuth);
	
	// Horizontal is straightforward
	double HorizScale = double(1 << LandOpts->HorizExp);
	// Vertical requires adjustment for aspect ratio;
	// the texcoords must be stretched in the vertical direction
	// if the texture is shrunken in the vertical direction
	short AdjustedVertExp = LandOpts->VertExp + LandOpts->OGL_AspRatExp;
	double VertScale = (AdjustedVertExp >= 0) ?
		double(1 << AdjustedVertExp) :
			1/double(1 << (-AdjustedVertExp));
	
	// Do further adjustments,
	// so as to do only two multiplies per texture-coordinate set
	AdjustedYaw *= HorizScale;
	HorizScale *= LandscapeRescale*Radian2Circle;
	VertScale *= LandscapeRescale*Radian2Circle;
		
	// Set up the texture manager with the input manager
	TextureManager TMgr;
	TMgr.ShapeDesc = RenderPolygon.ShapeDesc;
	TMgr.ShadingTables = RenderPolygon.shading_tables;
	TMgr.Texture = RenderPolygon.texture;
	TMgr.TransferMode = RenderPolygon.transfer_mode;
	TMgr.TransferData = RenderPolygon.transfer_data;
	TMgr.IsShadeless = (RenderPolygon.flags&_SHADELESS_BIT) != 0;
	TMgr.LandscapeVertRepeat = LandOpts->VertRepeat;
	TMgr.Landscape_AspRatExp = LandOpts->OGL_AspRatExp;
	TMgr.TextureType = OGL_Txtr_Landscape;
	
	// Use that texture
	if (!TMgr.Setup())
	{
		if (IsActive) glEnable(GL_FOG);
		return false;
	}
	
	// Storage of intermediate results for mass render
	ExtendedVertexData ExtendedVertexList[MAXIMUM_VERTICES_PER_SCREEN_POLYGON];
	
	short NumVertices = RenderPolygon.vertex_count;
	for (int k=0; k<NumVertices; k++)
	{
		// Create some convenient references
		point2d& Vertex = RenderPolygon.vertices[k];
		ExtendedVertexData& EVData = ExtendedVertexList[k];
		
		// Load the vertex position;
		// place it at positive infinity for the benefit of z-buffering
		EVData.Vertex[0] = Vertex.x;
		EVData.Vertex[1] = Vertex.y;
		EVData.Vertex[2] = -1;
		
		// Emit a ray from the vertex in OpenGL eye coords;
		// it had been specified in screen coordinates
		GLdouble VertexRay[3];
		Screen2Ray(Vertex,VertexRay);
	
		// Find the texture coordinates
		GLdouble U = AdjustedYaw + HorizScale*VertexRay[0];
		GLdouble V = 0.5 + VertScale*VertexRay[1];
		
		// Store the texture coordinates
		EVData.TexCoord[0] = U;
		EVData.TexCoord[1] = V;
	}
	
	// Set up lighting:
	glColor3f(1,1,1);
	
	// Cribbed from RenderAsRealWall()
	// Set up blending mode: either sharp edges or opaque
	// Added support for partial-opacity blending of wall textures
	// Added support for suppressing semitransparency when the void is on one side;
	// this suppression is optional for those who like weird smearing effects
	bool IsBlended = TMgr.IsBlended();
	if (!RenderPolygon.VoidPresent || TMgr.VoidVisible())
	{
		if (IsBlended)
		{
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
		} else {
			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
		}
	} else {
		// Completely opaque if can't see through void
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
	
	// Proper projection
	SetProjectionType(Projection_Screen);
	
	// Location of data:
	glVertexPointer(3,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	
	// Painting a texture...
	glEnable(GL_TEXTURE_2D);
	TMgr.SetupTextureMatrix();
	TMgr.RenderNormal();
	
	// Go!
	glDrawArrays(GL_POLYGON,0,NumVertices);
	
	// Cribbed from RenderAsRealWall()
	// Do textured rendering
	if (TMgr.IsGlowMapped())
	{
		// Do blending here to get the necessary semitransparency;
		// push the cutoff down so 0.5*0.5 (half of half-transparency)
		// The cutoff is irrelevant if the texture is set to one of the blended modes
		// instead of the crisp mode.
		// Added "IsBlended" test, so that alpha-channel selection would work properly
		// on a glowmap texture that is atop a texture that is opaque to the void.
		glColor3f(1,1,1);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		
		TMgr.RenderGlowing();
		SetBlend(TMgr.GlowBlend());
		glDrawArrays(GL_POLYGON,0,NumVertices);
	}
	
	// Revert to default blend
	SetBlend(OGL_BlendType_Crossfade);
	TMgr.RestoreTextureMatrix();
	
	if (IsActive) glEnable(GL_FOG);
	return true;
}

// The wall renderer takes a flag that indicates whether or not it is vertical
bool OGL_RenderWall(polygon_definition& RenderPolygon, bool IsVertical)
{
	if (!OGL_IsActive()) return false;
	
	// Make write-only, so as to avoid show-through by big objects behind,
	// and also by walls behind landscapes
	glDepthFunc(GL_ALWAYS);
	switch(RenderPolygon.transfer_mode)
	{
	case _textured_transfer:
	case _static_transfer:
		RenderAsRealWall(RenderPolygon, IsVertical);
		break;
				
	case _big_landscaped_transfer:
		RenderAsLandscape(RenderPolygon);
		break;
	}
	// Standard z-buffering acceptance
	glDepthFunc(GL_LEQUAL);
	return true;
}

// Returns true if OpenGL is active; if not, then false.
bool OGL_RenderSprite(rectangle_definition& RenderRectangle)
{
	if (!OGL_IsActive()) return false;
		
	// Set up the texture manager with the input manager
	TextureManager TMgr;
	TMgr.ShapeDesc = RenderRectangle.ShapeDesc;
	TMgr.LowLevelShape = RenderRectangle.LowLevelShape;
	TMgr.ShadingTables = RenderRectangle.shading_tables;
	TMgr.Texture = RenderRectangle.texture;
	TMgr.TransferMode = RenderRectangle.transfer_mode;
	TMgr.TransferData = RenderRectangle.transfer_data;
	TMgr.IsShadeless = (RenderRectangle.flags&_SHADELESS_BIT) != 0;
	
	// Is this an inhabitant or a weapons-in-hand texture?
	// Test by using the distance away from the viewpoint
	bool IsInhabitant;
	bool IsWeaponsInHand;
	double RayDistance = double(RenderRectangle.depth);
	if (RayDistance > 0)
	{
		IsInhabitant = true;
		IsWeaponsInHand = false;
		TMgr.TextureType = OGL_Txtr_Inhabitant;
	}
	else if (RayDistance == 0)
	{
		IsInhabitant = false;
		IsWeaponsInHand = true;
		TMgr.TextureType = OGL_Txtr_WeaponsInHand;
	}
	else return true;
	
	// Render as a model if one is found
	if (IsInhabitant)
	{
		OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
		if (ModelPtr)
		{
			RenderModelSetup(RenderRectangle);
			return true;
		}
	}
	
	// Find texture coordinates
	ExtendedVertexData ExtendedVertexList[4];
	
	point2d TopLeft, BottomRight;
	// Clipped corners:
	TopLeft.x = MAX(RenderRectangle.x0,RenderRectangle.clip_left);
	TopLeft.y = MAX(RenderRectangle.y0,RenderRectangle.clip_top);
	BottomRight.x = MIN(RenderRectangle.x1,RenderRectangle.clip_right);
	BottomRight.y = MIN(RenderRectangle.y1,RenderRectangle.clip_bottom);
	
	if (IsInhabitant)
	{
		// OpenGL eye coordinates
		GLdouble VertexRay[3];
		Screen2Ray(TopLeft,VertexRay);
		VecScalarMult(VertexRay,RayDistance,ExtendedVertexList[0].Vertex);
		Screen2Ray(BottomRight,VertexRay);
		VecScalarMult(VertexRay,RayDistance,ExtendedVertexList[2].Vertex);
	}
	else if (IsWeaponsInHand)
	{
		// Simple adjustment
		AdjustPoint(TopLeft);
		AdjustPoint(BottomRight);
		
		// Screen coordinates; weapons-in-hand are in the foreground
		ExtendedVertexList[0].Vertex[0] = TopLeft.x;
		ExtendedVertexList[0].Vertex[1] = TopLeft.y;
		ExtendedVertexList[0].Vertex[2] = 1;
		ExtendedVertexList[2].Vertex[0] = BottomRight.x;
		ExtendedVertexList[2].Vertex[1] = BottomRight.y;		
		ExtendedVertexList[2].Vertex[2] = 1;
	}
	else return true;
	
	// Completely clipped away?
	if (BottomRight.x <= TopLeft.x) return true;
	if (BottomRight.y <= TopLeft.y) return true;
	
	// Use that texture
	if (!TMgr.Setup()) return true;
	
	// Calculate the texture coordinates;
	// the scanline direction is downward, (texture coordinate 0)
	// while the line-to-line direction is rightward (texture coordinate 1)
	GLdouble U_Scale = TMgr.U_Scale/(RenderRectangle.y1 - RenderRectangle.y0);
	GLdouble V_Scale = TMgr.V_Scale/(RenderRectangle.x1 - RenderRectangle.x0);
	GLdouble U_Offset = TMgr.U_Offset;
	GLdouble V_Offset = TMgr.V_Offset;
	
	if (RenderRectangle.flip_vertical)
	{
		ExtendedVertexList[0].TexCoord[0] = U_Offset + U_Scale*(RenderRectangle.y1 - TopLeft.y);
		ExtendedVertexList[2].TexCoord[0] = U_Offset + U_Scale*(RenderRectangle.y1 - BottomRight.y);
	} else {
		ExtendedVertexList[0].TexCoord[0] = U_Offset + U_Scale*(TopLeft.y - RenderRectangle.y0);
		ExtendedVertexList[2].TexCoord[0] = U_Offset + U_Scale*(BottomRight.y - RenderRectangle.y0);
	}
	if (RenderRectangle.flip_horizontal)
	{
		ExtendedVertexList[0].TexCoord[1] = V_Offset + V_Scale*(RenderRectangle.x1 - TopLeft.x);
		ExtendedVertexList[2].TexCoord[1] = V_Offset + V_Scale*(RenderRectangle.x1 - BottomRight.x);
	} else {
		ExtendedVertexList[0].TexCoord[1] = V_Offset + V_Scale*(TopLeft.x - RenderRectangle.x0);
		ExtendedVertexList[2].TexCoord[1] = V_Offset + V_Scale*(BottomRight.x - RenderRectangle.x0);
	}
	
	// Fill in remaining points
	// Be sure that the order gives a sidedness the same as
	// that of the world-geometry polygons
	ExtendedVertexList[1].Vertex[0] = ExtendedVertexList[2].Vertex[0];
	ExtendedVertexList[1].Vertex[1] = ExtendedVertexList[0].Vertex[1];
	ExtendedVertexList[1].Vertex[2] = ExtendedVertexList[0].Vertex[2];
	ExtendedVertexList[1].TexCoord[0] = ExtendedVertexList[0].TexCoord[0];
	ExtendedVertexList[1].TexCoord[1] = ExtendedVertexList[2].TexCoord[1];
	ExtendedVertexList[3].Vertex[0] = ExtendedVertexList[0].Vertex[0];
	ExtendedVertexList[3].Vertex[1] = ExtendedVertexList[2].Vertex[1];
	ExtendedVertexList[3].Vertex[2] = ExtendedVertexList[2].Vertex[2];
	ExtendedVertexList[3].TexCoord[0] = ExtendedVertexList[2].TexCoord[0];
	ExtendedVertexList[3].TexCoord[1] = ExtendedVertexList[0].TexCoord[1];
	
	// Proper projection
	if (IsInhabitant)
		SetProjectionType(Projection_OpenGL_Eye);
	else if (IsWeaponsInHand)
		SetProjectionType(Projection_Screen);

	bool IsBlended = TMgr.IsBlended();
	bool ExternallyLit = false;
	GLfloat Color[4];
	DoLightingAndBlending(RenderRectangle, IsBlended,
		Color, ExternallyLit);

	if (IsBlended)
		// alpha test will be disabled, so don't hose z buffer
		glDisable(GL_DEPTH_TEST);

	// Already corrected
	glColor4fv(Color);
	
	// Location of data:
	glVertexPointer(3,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	glEnable(GL_TEXTURE_2D);
		
	// Go!
	TMgr.SetupTextureMatrix();
	TMgr.RenderNormal();	// Always do this, of course
	if (RenderRectangle.transfer_mode == _static_transfer)
	{
		SetupStaticMode(RenderRectangle.transfer_data);
		if (UseFlatStatic)
		{
			glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_POLYGON,0,4);
		} else {
			// Do multitextured stippling to create the static effect
			for (int k=0; k<StaticEffectPasses; k++)
			{
				StaticModeIndivSetup(k);
				glDrawArrays(GL_POLYGON,0,4);
			}
		}
		TeardownStaticMode();
	}
	else
	{
		// Ought not to set this for static mode
		SetBlend(TMgr.NormalBlend());

		// Do textured rendering
		glDrawArrays(GL_POLYGON,0,4);
		
		if (TMgr.IsGlowMapped())
		{
			// Do blending here to get the necessary semitransparency;
			// push the cutoff down so 0.5*0.5 (half of half-transparency)
		  // DON'T sRGB this.
			GLfloat GlowColor = TMgr.MinGlowIntensity();
			glColor4f(std::max(GlowColor,Color[0]),std::max(GlowColor,Color[1]),std::max(GlowColor,Color[2]),Color[3]);
			glEnable(GL_BLEND);
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_DEPTH_TEST);
			
			TMgr.RenderGlowing();
			SetBlend(TMgr.GlowBlend());
			glDrawArrays(GL_POLYGON,0,4);
		}
	}
	
	// Revert to default blend
	SetBlend(OGL_BlendType_Crossfade);
	TMgr.RestoreTextureMatrix();

	glEnable(GL_DEPTH_TEST);
		
	return true;
}

bool RenderModelSetup(rectangle_definition& RenderRectangle)
{
	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	assert(ModelPtr);
	
	// Initial clip check: where relative to the liquid?
	float Scale = RenderRectangle.Scale;
	GLfloat ModelFloor = Scale*ModelPtr->Model.BoundingBox[0][2];
	GLfloat ModelCeiling = Scale*ModelPtr->Model.BoundingBox[1][2];
	short LiquidRelHeight = RenderRectangle.LiquidRelHeight;
	
	if (RenderRectangle.BelowLiquid)
	{
		// Liquid below the bottom?
		if (LiquidRelHeight <= ModelFloor)
			return false;
	}
	else
	{
		// Liquid above the top?
		if (LiquidRelHeight >= ModelCeiling)
			return false;
	}
	if (RenderRectangle.clip_left >= RenderRectangle.x1) return false;
	if (RenderRectangle.clip_right <= RenderRectangle.x0) return false;
	if (RenderRectangle.clip_top >= RenderRectangle.y1) return false;
	if (RenderRectangle.clip_bottom <= RenderRectangle.y0) return false;
	
	// Find an animated model's vertex positions and normals:
	short ModelSequence = RenderRectangle.ModelSequence;
	if (ModelSequence >= 0)
	{
		int NumFrames = ModelPtr->Model.NumSeqFrames(ModelSequence);
		if (NumFrames > 0)
		{
			short ModelFrame = PIN(RenderRectangle.ModelFrame,0,NumFrames-1);
			short NextModelFrame = PIN(RenderRectangle.NextModelFrame,0,NumFrames-1);
			float MixFrac = RenderRectangle.MixFrac;
			ModelPtr->Model.FindPositions_Sequence(true,
				ModelSequence,ModelFrame,MixFrac,NextModelFrame);
		}
		else
			ModelPtr->Model.FindPositions_Neutral(true);	// Fallback: neutral
	}
	else
		ModelPtr->Model.FindPositions_Neutral(true);	// Fallback: neutral (will do nothing for static models)
	
	// For finding the clip planes: 0, 1, 2, 3, and 4
	bool ClipLeft = false, ClipRight = false, ClipTop = false, ClipBottom = false, ClipLiquid = false;
	GLdouble ClipPlane[4] = {0,0,0,0};
	
	if (RenderRectangle.clip_left >= RenderRectangle.x0)
	{
		ClipLeft = true;
		glEnable(GL_CLIP_PLANE0);
		ClipPlane[0] = 1;
		ClipPlane[1] = 0;
		ClipPlane[2] = XScaleRecip*(RenderRectangle.clip_left - XOffset);
		glClipPlane(GL_CLIP_PLANE0,ClipPlane);
	}
	
	if (RenderRectangle.clip_right <= RenderRectangle.x1)
	{
		ClipRight = true;
		glEnable(GL_CLIP_PLANE1);
		ClipPlane[0] = - 1;
		ClipPlane[1] = 0;
		ClipPlane[2] = - XScaleRecip*(RenderRectangle.clip_right - XOffset);
		glClipPlane(GL_CLIP_PLANE1,ClipPlane);
	}
	
	if (RenderRectangle.clip_top >= RenderRectangle.y0)
	{
		ClipTop = true;
		glEnable(GL_CLIP_PLANE2);
		ClipPlane[0] = 0;
		ClipPlane[1] = - 1;
		ClipPlane[2] = - YScaleRecip*(RenderRectangle.clip_top - YOffset);
		glClipPlane(GL_CLIP_PLANE2,ClipPlane);
	}
	
	if (RenderRectangle.clip_bottom <= RenderRectangle.y1)
	{
		ClipBottom = true;
		glEnable(GL_CLIP_PLANE3);
		ClipPlane[0] = 0;
		ClipPlane[1] = 1;
		ClipPlane[2] = YScaleRecip*(RenderRectangle.clip_bottom - YOffset);
		glClipPlane(GL_CLIP_PLANE3,ClipPlane);
	}
	
	// Get from the model coordinates to the screen coordinates.
	SetProjectionType(Projection_OpenGL_Eye);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(World_2_OGLEye);
	world_point3d& Position = RenderRectangle.Position;
	glTranslatef(Position.x,Position.y,Position.z);
	
	// At model's position; now apply the liquid clipping
	if (RenderRectangle.BelowLiquid)
	{
		// Liquid above the bottom? If so, then clip downward
		if (LiquidRelHeight >= ModelFloor)
		{
			ClipLiquid = true;
			glEnable(GL_CLIP_PLANE4);
			ClipPlane[0] = ClipPlane[1] = 0;
			ClipPlane[2] = - 1;
			ClipPlane[3] = LiquidRelHeight;
			glClipPlane(GL_CLIP_PLANE4,ClipPlane);
		}
	}
	else
	{
		// Liquid below the top? If so, then clip upward
		if (LiquidRelHeight <= ModelCeiling)
		{
			ClipLiquid = true;
			glEnable(GL_CLIP_PLANE4);
			ClipPlane[0] = ClipPlane[1] = 0;
			ClipPlane[2] = 1;
			ClipPlane[3] = - LiquidRelHeight;
			glClipPlane(GL_CLIP_PLANE4,ClipPlane);
		}
	}
	
	// Its orientation and size
	glRotated((360.0/FULL_CIRCLE)*RenderRectangle.Azimuth,0,0,1);
	GLfloat HorizScale = Scale*RenderRectangle.HorizScale;
	glScalef(HorizScale,HorizScale,Scale);
	
	// Be sure to include texture-mode effects as appropriate.
	short CollColor = GET_DESCRIPTOR_COLLECTION(RenderRectangle.ShapeDesc);
	short Collection = GET_COLLECTION(CollColor);
	short CLUT = ModifyCLUT(RenderRectangle.transfer_mode,GET_COLLECTION_CLUT(CollColor));
	bool ModelRendered = RenderModel(RenderRectangle,Collection,CLUT);
	
	glPopMatrix();
	
	// No need for the clip planes anymore
	if (ClipLeft) glDisable(GL_CLIP_PLANE0);
	if (ClipRight) glDisable(GL_CLIP_PLANE1);
	if (ClipTop) glDisable(GL_CLIP_PLANE2);
	if (ClipBottom) glDisable(GL_CLIP_PLANE3);
	if (ClipLiquid) glDisable(GL_CLIP_PLANE4);
	
	return ModelRendered;
}


bool RenderModel(rectangle_definition& RenderRectangle, short Collection, short CLUT)
{
	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	assert(ModelPtr);
	
	// Get the skin; test for whether one was actually found
	OGL_SkinData *SkinPtr = ModelPtr->GetSkin(CLUT);
	if (!SkinPtr) return false;
	
	// Parallel to TextureManager::IsBlended() in OGL_Textures.h
	bool IsBlended = SkinPtr->OpacityType != OGL_OpacType_Crisp;
	bool ExternallyLit = false;
	bool IsGlowmappable = DoLightingAndBlending(RenderRectangle, IsBlended,
		ShaderData.Color, ExternallyLit);
	
	ShaderData.ModelPtr = ModelPtr;
	ShaderData.SkinPtr = SkinPtr;
	ShaderData.Collection = Collection;
	ShaderData.CLUT = CLUT;
	
	// Don't care about the magnitude of this vector
	short Azimuth = normalize_angle(RenderRectangle.Azimuth);
	GLfloat Cosine = cosine_table[Azimuth];
	GLfloat Sine = sine_table[Azimuth];
	ModelRenderObject.ViewDirection[0] =   ViewDir[0]*Cosine + ViewDir[1]*Sine;
	ModelRenderObject.ViewDirection[1] = - ViewDir[0]*Sine + ViewDir[1]*Cosine;
	// The z-component is already set -- to 0

	// Set up the render sidedness
	if (ModelPtr->Sidedness < 0)
	{
		// Counterclockwise sides rendered
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	else if (ModelPtr->Sidedness == 0)
	{
		// Both sides rendered
		glDisable(GL_CULL_FACE);
	}
	// Default: clockwise sides rendered
	
	if (RenderRectangle.transfer_mode == _static_transfer)
	{
		SetupStaticMode(RenderRectangle.transfer_data);
		if (UseFlatStatic)
		{
			// Do explicit depth sort because these textures are semitransparent
			StandardShaders[0].Flags = ModelRenderer::Textured;
			ModelRenderObject.Render(ModelPtr->Model, StandardShaders,
				1, 0, true);
		} else {
			// Do multitextured stippling to create the static effect
			ModelRenderObject.Render(ModelPtr->Model, StaticModeShaders,
				StaticEffectPasses, SeparableStaticEffectPasses, true);
		}
		TeardownStaticMode();
	}
	else
	{
		bool IsGlowing = IsGlowmappable && SkinPtr->GlowImg.IsPresent();
		int NumShaders = IsGlowing ? 2 : 1;
		int NumSeparableShaders = IsBlended ? 0 : 1;
		
		SET_FLAG(StandardShaders[0].Flags,ModelRenderer::ExtLight,ExternallyLit);
		SET_FLAG(StandardShaders[0].Flags,ModelRenderer::EL_SemiTpt,false);
		
		if (ExternallyLit)
		{
			// Find the light values to use
			GLfloat AvgLight = (RenderRectangle.ceiling_light + RenderRectangle.ambient_shade)/GLfloat(2*FIXED_ONE);
			GLfloat LightDiff = (RenderRectangle.ceiling_light - RenderRectangle.ambient_shade)/GLfloat(2*FIXED_ONE);
			GLfloat *Dir = RenderRectangle.LightDirection;
			// In the direction of observation; use this to find miner's-light effect
			GLfloat LightInDir = AvgLight - Dir[2]*LightDiff;
			GLfloat Color[3];
			FindShadingColor(RenderRectangle.LightDepth,int(FIXED_ONE*LightInDir + 0.5),Color);
			
			LightingData.Type = ModelPtr->LightType;
			LightingData.ProjDistance = RenderRectangle.ProjDistance;
			LightingData.Dir = Dir;
			LightingData.AvgLight = AvgLight;
			LightingData.LightDiff = LightDiff;
			
			GLfloat Opacity = ShaderData.Color[3];
			LightingData.Opacity = Opacity;
			if (Opacity < 1)
				SET_FLAG(StandardShaders[0].Flags,ModelRenderer::EL_SemiTpt,true);
			
			// Whether to fade miner's light toward model sides
			bool NoFade = false;
			
			switch(LightingData.Type)
			{
			case OGL_MLight_Fast_NoFade:
				NoFade = true;
			case OGL_MLight_Fast:
				for (int c=0; c<3; c++)
				{
					// Note: the miner's-light effect adds to the existing light,
					// thus we subtract the bkgd before adding to the rest of the light
					if (NoFade)
					{
						LightingData.Colors[c][0] = 0;
						LightingData.Colors[c][1] = 0;
						LightingData.Colors[c][2] = LightDiff;
						LightingData.Colors[c][3] = AvgLight + (Color[c] - LightInDir);
					}
					else
					{
						GLfloat HC = (Color[c] - LightInDir)/2;
						LightingData.Colors[c][0] = - HC*Dir[0];
						LightingData.Colors[c][1] = - HC*Dir[1];
						LightingData.Colors[c][2] = LightDiff - HC*Dir[2];
						LightingData.Colors[c][3] = AvgLight + HC;
					}
				}
			}
		}
		
		ModelRenderObject.Render(ModelPtr->Model, StandardShaders, NumShaders,
			NumSeparableShaders, true);
		
		// Revert to default blend
		SetBlend(OGL_BlendType_Crossfade);
		
		// Restore default
		if (ExternallyLit) glDisableClientState(GL_COLOR_ARRAY);
	}
	
	// Restore the default render sidedness
	if (ModelPtr->Sidedness <= 0)
	{
		// Clockwise sides rendered
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	
	return true;
}

bool DoLightingAndBlending(rectangle_definition& RenderRectangle, bool& IsBlended,
	GLfloat *Color, bool& ExternallyLit)
{
	bool IsGlowmappable = true;
	ExternallyLit = true;
	
	// Apply lighting
	bool IsInvisible = false;
	if (RenderRectangle.transfer_mode == _static_transfer)
	{
		// Crisp, no glowmap
		IsBlended = false;
		IsGlowmappable = false;
		glEnable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		return IsGlowmappable;
	}
	else if (RenderRectangle.transfer_mode == _tinted_transfer)
	{
		// Used for invisibility; the tinting refers to the already-rendered color's fate
		// The opacity is controlled by the transfer data; its value is guessed from
		// the rendering source code (render.c, scottish_textures.c, low_level_textures.c)
		Color[0] = Color[1] = Color[2] = 0;
		Color[3] = 1 - RenderRectangle.transfer_data/32.0F;
		if(Using_sRGB) Color[3] = std::sqrt(Color[3]);
		IsInvisible = true;
		IsGlowmappable = false;
	}
	else if (RenderRectangle.flags&_SHADELESS_BIT)
	{
		// Only set when infravision is active
		Color[0] = Color[1] = Color[2] = 1;
		Color[3] = RenderRectangle.Opacity;
		IsGlowmappable = false;
	}
	else if (RenderRectangle.ambient_shade < 0)
	{
		GLfloat Light = (- RenderRectangle.ambient_shade)/GLfloat(FIXED_ONE);
		Color[0] = Color[1] = Color[2] = Light;
		Color[3] = RenderRectangle.Opacity;
	}
	else
	{
		// External lighting includes the player's "miner's light"
		ExternallyLit = true;
		FindShadingColor(RenderRectangle.depth,RenderRectangle.ambient_shade,Color);
		Color[3] = RenderRectangle.Opacity;
	}
	
	// Make the sprites crisp-edged, except if they are in invisibility mode
	// or are otherwise semitransparent
	if (IsInvisible || IsBlended || (RenderRectangle.Opacity < 1))
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		IsBlended = true;
	} else { 
		glEnable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
	}
	
	return IsGlowmappable;
}


void SetupStaticMode(int16 transfer_data)
{
	if (UseFlatStatic)
	{
		// Per-sprite coloring; be sure to add the transparency in appropriate fashion
		for (int c=0; c<3; c++)
			FlatStaticColor[c] = StaticRandom.KISS() + StaticRandom.LFIB4();
		FlatStaticColor[3] = 65535 - transfer_data;
		
		// Do flat-color version of static effect
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		SglColor4usv(FlatStaticColor);
	} else {
#ifdef USE_STIPPLE_STATIC_EFFECT
		// Do multitextured stippling to create the static effect
		
		// The colors:
		// Index from 1 to 3; index 0 is reserve for the initial blackout
		for (int c=1; c<4; c++)
			for (int n=0; n<StatPatLen; n++)
				StaticPatterns[c][n] = StaticRandom.KISS() + StaticRandom.LFIB4();
		
		// The alpha channel:
		for (int n=0; n<StatPatLen; n++)
		{
			uint32 Val = 0;
			// Avoid doing extra random-number evaluations
			// for the case of complete opacity
			if (transfer_data > 0)
			{
				for (int k=0; k<32; k++)
				{
					// Have to do this for every bit to get the proper probability distribution
					Val <<= 1;
					uint16 RandSamp = uint16(StaticRandom.KISS());
					Val |= (RandSamp >= transfer_data) ? 1 : 0;
				}
			}
			else
				Val = 0xffffffff;
			
			// Premultiply the stipple-color values by the alpha
			StaticPatterns[0][n] = Val;
			for (int c=1; c<4; c++)
				StaticPatterns[c][n] &= Val;
		}
		
		// Get ready to use those static patterns
		glEnable(GL_POLYGON_STIPPLE);
#else
		// Use the stencil buffer to create the static effect
		glEnable(GL_STENCIL_TEST);
		
		StencilTxtrOpacity = 1 - transfer_data/65535.0;
#endif
	}
}

void TeardownStaticMode()
{
	if (UseFlatStatic)
	{
		// Nothing
	}
	else
	{
#ifdef USE_STIPPLE_STATIC_EFFECT
		// Restore the default blending
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		// glDisable(GL_COLOR_LOGIC_OP);
		glDisable(GL_POLYGON_STIPPLE);
#else
		// Done with the stencil buffer
		glDisable(GL_STENCIL_TEST);
#endif
	}
}


void NormalShader(void *Data)
{
	// Normal setup: be sure to use the normal color
	SglColor4fv(ShaderData.Color);
	
	if (ShaderData.ModelPtr->Use(ShaderData.CLUT,OGL_SkinManager::Normal))
	{
		LoadModelSkin(ShaderData.SkinPtr->NormalImg,ShaderData.Collection, ShaderData.CLUT);
		SetBlend(ShaderData.SkinPtr->NormalBlend);
	}
}

void GlowingShader(void *Data)
{
	// Glowmapped setup
	GLfloat GlowColor = ShaderData.SkinPtr->MinGlowIntensity;
	SglColor4f(std::max(ShaderData.Color[0],GlowColor),std::max(ShaderData.Color[1],GlowColor),std::max(ShaderData.Color[2],GlowColor),ShaderData.Color[3]*(Using_sRGB ? ShaderData.Color[3] : 1.0));
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	
	if (ShaderData.ModelPtr->Use(ShaderData.CLUT,OGL_SkinManager::Glowing))
	{
		LoadModelSkin(ShaderData.SkinPtr->GlowImg, ShaderData.Collection, ShaderData.CLUT);
		SetBlend(ShaderData.SkinPtr->GlowBlend);
	}
}


void StaticModeIndivSetup(int SeqNo)
{
#ifdef USE_STIPPLE_STATIC_EFFECT
	// Black [backing], red, green, blue
	const GLfloat StaticBaseColors[4][3] = {{0,0,0},{1,0,0},{0,1,0},{0,0,1}};
	
	switch(SeqNo)
	{
	case 0:	// In case of another go-around, as in z-buffer-less rendering
		// Paint on the backing color by making it unblended
		glDisable(GL_BLEND);
		// glDisable(GL_COLOR_LOGIC_OP);
		break;
		
	case 1:	// No need to do this for cases 2 and 3, since they will follow
		// Add the color-channel contributions with appropriate blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		// glEnable(GL_COLOR_LOGIC_OP);
		// glLogicOp(GL_OR);
	}

	// no need to correct
	glColor3fv(StaticBaseColors[SeqNo]);			
	glPolygonStipple((byte *)StaticPatterns[SeqNo]);
#else
	// Stencil buffering
	switch(SeqNo)
	{
	case 0:
		// The stencil buffer will become 0 across the whole rendered object
		glStencilFunc(GL_ALWAYS,1,1);
		glStencilOp(GL_KEEP, GL_ZERO, GL_ZERO);
		glStencilMask(1);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		break;
		
	case 1:
		// The stencil buffer will become 1 everywhere a pixel is to be rendered
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_ALPHA_TEST);
		break;
		
	case 2:
		// Use the stencil buffer -- don't write into it, of course
		glStencilFunc(GL_EQUAL,1,1);
		glStencilMask(0);
		glEnable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		if(Using_sRGB) glDisable(GL_FRAMEBUFFER_sRGB);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		glColor4f(1,1,1,Using_sRGB ? StencilTxtrOpacity*StencilTxtrOpacity : StencilTxtrOpacity);	// Static is fully bright and partially transparent
		break;
	}
#endif
}

void StaticModeShader(void *Data)
{
	int *Which = (int *)Data;
	StaticModeIndivSetup(*Which);
	
#ifndef USE_STIPPLE_STATIC_EFFECT
	if (*Which < 2)
	{
#endif
	// The silhouette texture is a "normal" one
	if (ShaderData.ModelPtr->Use(ShaderData.CLUT,OGL_SkinManager::Normal))
		LoadModelSkin(ShaderData.SkinPtr->NormalImg, ShaderData.Collection, ShaderData.CLUT);
#ifndef USE_STIPPLE_STATIC_EFFECT
	}
	else
	{
		// For the static effect
		glBindTexture(GL_TEXTURE_2D,0);
		
		const int TxSize = 64;
		const int TxPxls = TxSize*TxSize;
		static uint32 Buffer[TxPxls];
		for (int k=0; k<TxPxls; k++)
		{
			uint32 Pxl = 0;
			for (int m=0; m<3; m++)
			{
				Pxl += (StaticRandom.KISS() + StaticRandom.LFIB4()) & 0x000000ff;
				Pxl <<= 8;
			}
			Pxl += 0xff;
			Buffer[k] = Pxl;
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, Using_sRGB ? GL_SRGB_ALPHA : GL_RGBA8, TxSize, TxSize,
			0, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	if(*Which == 3 && Using_sRGB) glEnable(GL_FRAMEBUFFER_sRGB);
#endif
}



void LightingCallback(void *Data, size_t NumVerts, GLfloat *Normals, GLfloat *Positions, GLfloat *Colors)
{
	LightingDataStruct *LPtr = (LightingDataStruct *)Data;

	// In case of a semitransparent object
	GLfloat Opacity = LPtr->Opacity;
	bool Semitransparent = (Opacity < 1);
	int NumCPlanes = Semitransparent ? 4 : 3;
	
	// Whether to fade miner's light toward model sides
	bool NoFade = false;
	
	switch(LPtr->Type)
	{
	case OGL_MLight_Fast_NoFade:
		NoFade = true;
	case OGL_MLight_Fast:
	{
		GLfloat *el0 = LPtr->Colors[0], *el1 = LPtr->Colors[1], *el2 = LPtr->Colors[2];
		for (size_t k=0; k<NumVerts; k++)
		{
			GLfloat N0 = *(Normals++);
			GLfloat N1 = *(Normals++);
			GLfloat N2 = *(Normals++);
			*(Colors++) = el0[0]*N0 + el0[1]*N1 + el0[2]*N2 + el0[3];
			*(Colors++) = el1[0]*N0 + el1[1]*N1 + el1[2]*N2 + el1[3];
			*(Colors++) = el2[0]*N0 + el2[1]*N1 + el2[2]*N2 + el2[3];
			if (Semitransparent)
				*(Colors++) = Opacity;
		}
	}
	break;
	
	case OGL_MLight_Indiv_NoFade:
	NoFade = true;
	case OGL_MLight_Indiv:
	{
		for (size_t k=0; k<NumVerts; k++, Normals += 3, Positions +=3, Colors +=NumCPlanes)
		{
			GLfloat *Dir = LPtr->Dir;
			GLfloat Depth = LPtr->ProjDistance + (Dir[0]*Positions[0] + Dir[1]*Positions[1]);
			GLfloat Light = LPtr->AvgLight + LPtr->LightDiff*Normals[2];
			if (NoFade)
				FindShadingColor(Depth,int(FIXED_ONE*Light + 0.5),Colors);
			else
			{
				GLfloat FrontColor[3], BackColor[3];
				BackColor[0] = BackColor[1] = BackColor[2] = Light;
				FindShadingColor(Depth,int(FIXED_ONE*Light + 0.5),FrontColor);
				GLfloat NProd = -(Normals[0]*Dir[0] + Normals[1]*Dir[1] + Normals[2]*Dir[2]);
				GLfloat BackMult = (1-NProd)/2;
				GLfloat FrontMult = (1+NProd)/2;
				for (int c=0; c<3; c++)
					Colors[c] = BackMult*BackColor[c] + FrontMult*FrontColor[c];
				if (Semitransparent)
					Colors[3] = Opacity;
			}
		}
	}
	break;
	}	
}


void SetupShaders()
{
	StandardShaders[0].Flags = ModelRenderer::Textured;
	StandardShaders[0].TextureCallback = NormalShader;
	StandardShaders[0].LightingCallback = LightingCallback;
	StandardShaders[0].LightingCallbackData = &LightingData;

	StandardShaders[1].Flags = ModelRenderer::Textured;
	StandardShaders[1].TextureCallback = GlowingShader;

	StaticModeShaders[0].Flags = ModelRenderer::Textured;
	StaticModeShaders[0].TextureCallback = StaticModeShader;
	StaticModeShaders[0].TextureCallbackData = SequenceNumbers + 0;

	StaticModeShaders[1].Flags = ModelRenderer::Textured;
	StaticModeShaders[1].TextureCallback = StaticModeShader;
	StaticModeShaders[1].TextureCallbackData = SequenceNumbers + 1;

	StaticModeShaders[2].Flags = ModelRenderer::Textured;
	StaticModeShaders[2].TextureCallback = StaticModeShader;
	StaticModeShaders[2].TextureCallbackData = SequenceNumbers + 2;

	StaticModeShaders[3].Flags = ModelRenderer::Textured;
	StaticModeShaders[3].TextureCallback = StaticModeShader;
	StaticModeShaders[3].TextureCallbackData = SequenceNumbers + 3;
}


// Rendering crosshairs
bool OGL_RenderCrosshairs()
{
	if (!OGL_IsActive()) return false;
	if (use_lua_hud_crosshairs) return false;
	
	// Crosshair features
	CrosshairData& Crosshairs = GetCrosshairData();
	
	// Proper projection
	SetProjectionType(Projection_Screen);

	// No textures painted here, but will blend
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	
	// What color; make 50% transparent (Alexander Strange's idea)
	// Changed it to use the crosshairs data
	if (!Crosshairs.PreCalced)
	{
	    Crosshairs.PreCalced = true;
	    Crosshairs.GLColorsPreCalc[0] = Crosshairs.Color.red/65535.0F;
	    Crosshairs.GLColorsPreCalc[1] = Crosshairs.Color.green/65535.0F;
	    Crosshairs.GLColorsPreCalc[2] = Crosshairs.Color.blue/65535.0F;
	    Crosshairs.GLColorsPreCalc[3] = Crosshairs.Opacity;
	}
	SglColor4fv(Crosshairs.GLColorsPreCalc);
	
	// Create a new modelview matrix for the occasion
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(ViewWidth / 2, ViewHeight / 2, 1);
	
	// To keep pixels aligned, we have to draw on pixel boundaries.
	// The SW renderer always offsets down and to the right when faced
	// with an odd size. We're going to rotate our coordinate system,
	// but we still have to obey the global offset rules.
	//
	// We precalculate the offsets for crosshair thickness below,
	// for each of the four quadrants.
	int halfWidthMin = -Crosshairs.Thickness / 2;
	int halfWidthMax = halfWidthMin - (Crosshairs.Thickness % 2);
	int offsets[4][2] = {   // [quadrant][local x/y]
		{ halfWidthMin, halfWidthMin },
		{ halfWidthMax, halfWidthMin },
		{ halfWidthMax, halfWidthMax },
		{ halfWidthMin, halfWidthMax } };

	for (int quad = 0; quad < 4; quad++)
	{
		int WidthMin = offsets[quad][0];
		int WidthMax = WidthMin + Crosshairs.Thickness;
		int HeightMin = offsets[quad][1];
		int HeightMax = HeightMin + Crosshairs.Thickness;

		switch(Crosshairs.Shape)
		{
		case CHShape_RealCrosshairs:
			{
				// Four simple rectangles
				
				int LenMin = Crosshairs.FromCenter;
				int LenMax = LenMin + Crosshairs.Length;
				
				// at the initial rotation, this is the rectangle at 3:00
				OGL_RenderRect(LenMin, HeightMin, LenMax - LenMin, HeightMax - HeightMin);
			}
			break;
		case CHShape_Circle:
			{
				// This will really be an octagon, for OpenGL-rendering convenience
				//
				// Each of the four sections is drawn with three quads --
				// the middle diagonal section and two straight ends.
				// Depending on the crosshair parameters, some segments
				// have zero length: this happens when LenMid == LenMin.
				
				int LenMax = Crosshairs.Length;
				int LenMid = LenMax / 2;
				int LenMin = std::min(LenMid, static_cast<int>(Crosshairs.FromCenter));
				
				// at the initial rotation, this is the bottom right
				GLint vertices[16] = {
					LenMax + WidthMin, LenMin + HeightMin,
					LenMax + WidthMax, LenMin + HeightMin,
					LenMax + WidthMin, LenMid + HeightMin,
					LenMax + WidthMax, LenMid + HeightMax,
					LenMid + WidthMin, LenMax + HeightMin,
					LenMid + WidthMax, LenMax + HeightMax,
					LenMin + WidthMin, LenMax + HeightMin,
					LenMin + WidthMin, LenMax + HeightMax
				};
				glVertexPointer(2, GL_INT, 0, vertices);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
			}
			break;
		}
		glRotated(-90.0, 0, 0, 1); // turn clockwise		
	}
	
	// Done with that modelview matrix
	glPopMatrix();
			
	return true;
}

// Rendering text
bool OGL_RenderText(short BaseX, short BaseY, const char *Text, unsigned char r, unsigned char g, unsigned char b)
{
	if (!OGL_IsActive()) return false;
	
	// Create display list for the current text string;
	// use the "standard" text-font display list (display lists can be nested)
	GLuint TextDisplayList;
	TextDisplayList = glGenLists(1);
	glNewList(TextDisplayList,GL_COMPILE);
	GetOnScreenFont().OGL_Render(Text);
	glEndList();
	
	// Place the text in the foreground of the display
	SetProjectionType(Projection_Screen);
	GLfloat Depth = 0;
	
	// Using a modelview matrix, of course
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	// Background
	glColor3f(0,0,0);
	
	// Changed to drop shadow only for performance reasons
	/*
	glLoadIdentity();
	glTranslatef(BaseX-1,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX+1,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX-1,BaseY,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX+1,BaseY,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX-1,BaseY+1,Depth);
	glCallList(TextDisplayList);
	
	glLoadIdentity();
	glTranslatef(BaseX,BaseY+1,Depth);
	glCallList(TextDisplayList);
	*/
	
	glLoadIdentity();
	glTranslatef(BaseX+1.0F,BaseY+1.0F,Depth);
	glCallList(TextDisplayList);
	
	// Foreground
	SglColor3f(r/255.0f,g/255.0f,b/255.0f);

	glLoadIdentity();
	glTranslatef(BaseX,BaseY,Depth);
	glCallList(TextDisplayList);
		
	// Clean up
	glDeleteLists(TextDisplayList,1);
	glPopMatrix();
	
	return true;
}

void OGL_RenderRect(float x, float y, float w, float h)
{
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	GLfloat vertices[8] = { x, y, x + w, y, x + w, y + h, x, y + h };
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_POLYGON, 0, 4);

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OGL_RenderRect(const SDL_Rect& rect)
{
	OGL_RenderRect(rect.x, rect.y, rect.w, rect.h);
}

void OGL_RenderTexturedRect(float x, float y, float w, float h, float tleft, float ttop, float tright, float tbottom)
{	
	GLfloat vertices[8] = { x, y, x + w, y, x + w, y + h, x, y + h };
	GLfloat texcoords[8] = { tleft, ttop, tright, ttop, tright, tbottom, tleft, tbottom };
    glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_POLYGON, 0, 4);
}

void OGL_RenderTexturedRect(const SDL_Rect& rect, float tleft, float ttop, float tright, float tbottom)
{
	OGL_RenderTexturedRect(rect.x, rect.y, rect.w, rect.h, tleft, ttop, tright, tbottom);
}

void OGL_RenderFrame(float x, float y, float w, float h, float t)
{
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	GLfloat vertices[20] = {
		x,         y,
		x + t,     y + t,
		x,         y + h,
		x + t,     y + h - t,
		x + w,	   y + h,
		x + w - t, y + h - t,
		x + w,     y,
		x + w - t, y + t,
		x,		   y,
		x + t,	   y + t
	};
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
	
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OGL_RenderLines(const std::vector<world_point2d>& points, float thickness)
{
	if (points.empty())
		return;
	
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	std::vector<GLfloat> coords;
	for (size_t i = 1; i < points.size(); i += 2)
	{
		world_point2d prev = points[i - 1];
		world_point2d cur = points[i];
		
		float rise = cur.y - prev.y;
		float run = cur.x - prev.x;
		float length = sqrtf(rise*rise + run*run);
		
		// Skip degenerate lines
		if (length == 0)
			continue;
		
		float scale = thickness / length;
		float xd = run * scale * 0.5f;
		float yd = rise * scale * 0.5f;
		
		coords.push_back(prev.x - yd);
		coords.push_back(prev.y + xd);
		coords.push_back(prev.x + yd);
		coords.push_back(prev.y - xd);
		coords.push_back(cur.x - yd);
		coords.push_back(cur.y + xd);
		
		coords.push_back(prev.x + yd);
		coords.push_back(prev.y - xd);
		coords.push_back(cur.x + yd);
		coords.push_back(cur.y - xd);
		coords.push_back(cur.x - yd);
		coords.push_back(cur.y + xd);
	}
	
	if (!coords.empty())
	{
		glVertexPointer(2, GL_FLOAT, 0, &coords.front());
		glDrawArrays(GL_TRIANGLES, 0, coords.size() / 2);
	}
	
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Render the console cursor
bool OGL_RenderTextCursor(const SDL_Rect& rect, unsigned char r, unsigned char g, unsigned char b)
{
	if (!OGL_IsActive()) return false;
	
	// Place the cursor in the foreground of the display
	SetProjectionType(Projection_Screen);
	
	SglColor3f(r/255.0f, g/255.0f, b/255.0f);
	OGL_RenderRect(rect);
	
	return true;
}


// Sets the infravision tinting color for a shapes collection, and whether to use such tinting;
// the color values are from 0 to 1.
bool OGL_SetInfravisionTint(short Collection, bool IsTinted, float Red, float Green, float Blue)
{
	// Can be called when OpenGL is inactive
	if (!OGL_IsPresent()) return false;

	// A way of defining some OGL_Textures stuff in OGL_Render.h
	return SetInfravisionTint(Collection, IsTinted, Red, Green, Blue);
}


// Set the blend, being sure to remember the blend type set to
static void SetBlend(short _BlendType)
{
	// Don't need to do anything if no change
	if (_BlendType == BlendType) return;
	
	// Remember what's being set to
	BlendType = _BlendType;
	
	switch(BlendType)
	{
	// Blend-function args are incoming pixel, then background/previous pixel
	case OGL_BlendType_Crossfade:
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		break;
		
	case OGL_BlendType_Add:
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		break;
	case OGL_BlendType_Crossfade_Premult:
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case OGL_BlendType_Add_Premult:
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	}
}

#else

// No OpenGL present
bool OGL_IsActive()
{
	return false;
}

#endif // def HAVE_OPENGL
