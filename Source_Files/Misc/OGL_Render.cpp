/*
	
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
*/

#include <GL/gl.h>
#include <GL/glu.h>
#include <agl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "cseries.h"
#include "interface.h"
#include "render.h"
#include "map.h"
#include "OGL_Render.h"
#include "OGL_Textures.h"
#include "Crosshairs.h"
#include "VectorOps.h"
#include "GrowableList.h"
#include "my32bqd.h"
#include "Random.h"
#include "ViewControl.h"
#include "OGL_Faders.h"


// Whether or not OpenGL is active for rendering
static bool _OGL_IsActive = false;

// Render context; expose for OGL_Map.c
AGLContext RenderContext;

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
static GLdouble Screen_2_Clip[16];
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

// Whether Z-buffering is being used
bool Z_Buffering = false;

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

// Yaw angle (full circle = 1)
static double Yaw;

// Landscape rescaling to get closer to software-rendering scale
static double LandscapeRescale;


// Self-luminosity (the "miner's light" effect and weapons flare)
static fixed SelfLuminosity;


// Self-explanatory :-)
static bool FogPresent = false;
static GLfloat FogColor[4] = {0,0,0,0};


// Stipple patterns for that static look
// 3 color channels * 32*32 array of bits
const int StatPatLen = 32;
static GLuint StaticPatterns[3][StatPatLen];

// Alternative: flat static
static bool UseFlatStatic;
static word FlatStaticColor[3];

// The randomizer for the static-effect pixels
static GM_Random StaticRandom;


// Whether to do one-pass multitexturing
static bool OnePassMultitexturing = false;


// Display-list ID used for text fonts;
// this is actually the display list ID for ASCII '\0' (0);
// the others are in appropriate numerical order.
static GLuint FontDisplayList;

// Function for resetting map fonts when starting up an OpenGL rendering context;
// defined in OGL_Map.c
extern void OGL_ResetMapFonts();


// This is a strip buffer for piping 2D graphics through OpenGL.
static GLuint *Buffer2D = NULL;
static int Buffer2D_Width = 0;
// Making the buffer several lines makes it more efficient
const int Buffer2D_Height = 16;


// This function returns whether OpenGL is active;
// if OpenGL is not present, it will never be active.

// Test for activity;
bool OGL_IsActive() {if (OGL_IsPresent()) return _OGL_IsActive; else return false;}


// Start an OpenGL run (creates a rendering context)
bool OGL_StartRun(CGrafPtr WindowPtr)
{
	if (!OGL_IsPresent()) return false;
	
	// Will stop previous run if it had been active
	if (OGL_IsActive()) OGL_StopRun();
	
	// If bit depth is too small, then don't start
	if (bit_depth <= 8) return false;
	
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	Z_Buffering = TEST_FLAG(ConfigureData.Flags,OGL_Flag_ZBuffer) != 0;
	
	// Plain and simple
	GrowableList<GLint> PixelFormatSetupList(16);
	PixelFormatSetupList.Add(GLint(AGL_RGBA));
	PixelFormatSetupList.Add(GLint(AGL_DOUBLEBUFFER));
	if (Z_Buffering)
	{
		PixelFormatSetupList.Add(GLint(AGL_DEPTH_SIZE));
		PixelFormatSetupList.Add(16);
	}
	PixelFormatSetupList.Add(GLint(AGL_NONE));
	
	// Request that pixel format
	AGLPixelFormat PixelFormat = aglChoosePixelFormat(NULL, 0, PixelFormatSetupList.Begin());
	
	// Was it found?
	if (!PixelFormat) return false;
	
	// Create the rendering context
	RenderContext = aglCreateContext(PixelFormat, NULL);
	
	// Clean up
	aglDestroyPixelFormat(PixelFormat);
	
	// Was it successful?
	if (!RenderContext) return false;
	
	// Attach the window; if possible
	bool AttachWindow = aglSetDrawable(RenderContext, WindowPtr);
	if (!AttachWindow)
	{
		aglDestroyContext(RenderContext);
		return false;
	}
	
	if (!aglSetCurrentContext(RenderContext))
	{
		aglDestroyContext(RenderContext);
		return false;
	}
	
	// Set up some OpenGL stuff: these will be the defaults for this rendering context
	
	// Set up for Z-buffering
	if (Z_Buffering)
	{
		glEnable(GL_DEPTH_TEST);
		// This Z-buffering setup does seem to work correctly;
		// one may want to make the world geometry write-only (glDepthFunc(GL_ALWAYS))
		// if coincident textures are not rendered very well.
		// [DEFAULT] -- anything marked out as this ought to be reverted to if changed
		glDepthFunc(GL_LEQUAL);
		glDepthRange(1,0);
	}
	
	// Prevent wrong-side polygons from being rendered;
	// this works because the engine's visibility routines make all world-geometry
	// polygons have the same sidedness when they are viewed from inside.
	// [DEFAULT]
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	
	// Note: GL_BLEND and GL_ALPHA_FUNC do not have defaults; these are to be set
	// if some new pixels cannot be assumed to be always 100% opaque.
	
	// [DEFAULT]
	// Set standard alpha-test function; cut off at halfway point (for sharp edges)
	glAlphaFunc(GL_GREATER,0.5);
	
	// [DEFAULT]
	// Set standard blending function (for smooth transitions)
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	// Switch on use of vertex and texture-coordinate arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	OnePassMultitexturing = TEST_FLAG(ConfigureData.Flags,OGL_Flag_SnglPass) != 0;
	// Texture 0 is the default one; texture 1 is the additional texture
	if (OnePassMultitexturing)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}
	
	// Initialize the texture accounting
	OGL_StartTextures();
	
	// Fog status:
	FogPresent = TEST_FLAG(ConfigureData.Flags,OGL_Flag_Fog);
		
	// Convenient function for setting up fonts;
	// set aside some display lists for them
	FontDisplayList = glGenLists(256);
	aglUseFont(RenderContext, kFontIDMonaco, normal, 12, 0, 256, FontDisplayList);
	
	// Reset the font into for overhead-map fonts done in OpenGL fashion
	OGL_ResetMapFonts();
	
	// Success!
	JustInited = true;
	return (_OGL_IsActive = true);
}

// Stop an OpenGL run (destroys a rendering context)
bool OGL_StopRun()
{
	if (!OGL_IsActive()) return false;
	
	OGL_StopTextures();
	
	aglDestroyContext(RenderContext);
	
	_OGL_IsActive = false;
	return true;
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
	
	if (!DoUpdate) return true;
	
	SavedScreenBounds = ScreenBounds;
	SavedViewBounds = ViewBounds;
	
	// Note, the screen bounds are adjusted so that the canonical bounds
	// are in the center of the screen.
	
	short Left = ViewBounds.left;
	short Right = ViewBounds.right;
	short Top = ViewBounds.top;
	short Bottom = ViewBounds.bottom;
	ViewWidth = Right - Left;
	ViewHeight = Bottom - Top;
	short TotalHeight = ScreenBounds.bottom - ScreenBounds.top;
	
	// Must be in OpenGL coordinates
	GLint RectBounds[4];
	RectBounds[0] = Left;
	RectBounds[1] = TotalHeight - Bottom;
	RectBounds[2] = ViewWidth;
	RectBounds[3] = ViewHeight;
	
	// Adjustment for the screen
	RectBounds[0] -= ScreenBounds.left;
	RectBounds[1] += ScreenBounds.top;
	
	if (UseBackBuffer)
	{
		// This could not be gotten to work quite right, so a more roundabout way is being done;
		// the screen's portRect gets offset when it is bigger than 640*480.
		/*
		aglEnable(RenderContext,AGL_BUFFER_RECT);
		aglSetInteger(RenderContext,AGL_BUFFER_RECT,RectBounds);
		*/
		
		// Set aside swap area
		aglEnable(RenderContext,AGL_SWAP_RECT);
		aglSetInteger(RenderContext,AGL_SWAP_RECT,RectBounds);
	}
	
	// Do OpenGL bounding
	glScissor(RectBounds[0], RectBounds[1], RectBounds[2], RectBounds[3]);
	glViewport(RectBounds[0], RectBounds[1], RectBounds[2], RectBounds[3]);
	
	// Create the screen -> clip (fundamental) matrix; this will be needed
	// for all the other projections
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ViewWidth, ViewHeight, 0, 1, -1);	// OpenGL-style z
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
	
	// Set the Z-buffering for this go-around
	if (Z_Buffering)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	
	// Moved this test down here for convenience; the overhead map won't have fog,
	// so be sure to turn it on when leaving the overhead map
	if (FogPresent)
	{
		glEnable(GL_FOG);
		// Set the fog color appropriately
		OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
		MakeFloatColor(ConfigureData.FogColor,FogColor);
		if (IsInfravisionActive())
			FindInfravisionVersion(_collection_landscape1+static_world->song_index,FogColor);
		glFogfv(GL_FOG_COLOR,FogColor);
		glFogf(GL_FOG_DENSITY,1.0/ConfigureData.FogDepth);
	}
	else
		glDisable(GL_FOG);
	
	// Set the color of the void
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	if (TEST_FLAG(ConfigureData.Flags,OGL_Flag_VoidColor))
	{
		RGBColor& VoidColor = ConfigureData.VoidColor;
		GLfloat Red = VoidColor.red/65535.0;
		GLfloat Green = VoidColor.green/65535.0;
		GLfloat Blue = VoidColor.blue/65535.0;
		
		// The color of the void will be the color of fog
		if (FogPresent)
		{
			Red = FogColor[0];
			Green = FogColor[1];
			Blue = FogColor[2];
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
	if (UseFlatStatic)
	{
		// Made this per-sprite
		/*
		for (int c=0; c<3; c++)
			FlatStaticColor[c] = StaticRandom.KISS() + StaticRandom.LFIB4();
		*/
	} else {
		for (int c=0; c<3; c++)
			for (int n=0; n<StatPatLen; n++)
				StaticPatterns[c][n] = StaticRandom.KISS() + StaticRandom.LFIB4();
	}
		
	return true;
}


bool OGL_EndMain()
{
	if (!OGL_IsActive()) return false;
	
	// Proper projection
	SetProjectionType(Projection_Screen);
	
	// No texture mapping now
	glDisable(GL_TEXTURE_2D);
	
	GLboolean Z_Buffering = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	
	// Render OpenGL faders, if in use
	OGL_DoFades(0,0,ViewWidth,ViewHeight);
	
	// Paint over 1-pixel boundary
	glColor3f(0,0,0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(0.5,0.5);
	glVertex2f(0.5,ViewHeight-0.5);
	glVertex2f(ViewWidth-0.5,ViewHeight-0.5);
	glVertex2f(ViewWidth-0.5,0.5);
	glEnd();
		
	return true;
}

// Swap buffers (reveal rendered image)
bool OGL_SwapBuffers()
{
	if (!OGL_IsActive()) return false;
	
	aglSwapBuffers(RenderContext);
	
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
	
	assert(HorizCoords.FindComplements());
	
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


// Self-luminosity calculations;
// cribbed from scottish_textures.c (CALCULATE_SHADING_TABLE):

// This finds the intensity-slope crossover depth for splitting polygon lines;
// it takes the shading value from the render object
inline GLdouble FindCrossoverDepth(fixed Shading)
{
	return ((8*GLdouble(WORLD_ONE))/GLdouble(FIXED_ONE))*(SelfLuminosity - Shading);
}


// This finds the color value for lighting from the render object's shading value
void FindShadingColor(GLdouble Depth, fixed Shading, GLfloat *Color)
{
	GLdouble SelfIllumShading =
		PIN(SelfLuminosity - (GLdouble(FIXED_ONE)/(8*GLdouble(WORLD_ONE)))*Depth,0,FIXED_ONE);
	
	GLdouble CombinedShading = (Shading>SelfIllumShading) ? (Shading + 0.5*SelfIllumShading) : (SelfIllumShading + 0.5*Shading);
	
	Color[0] = Color[1] = Color[2] = PIN((1/GLdouble(FIXED_ONE))*CombinedShading,0,1);
}


// For debugging purposes:
static void MakeFalseColor(int c, float Opacity = 1)
{
	int cr = c % 12;
	if (cr < 0) cr += 12;
	
	const float Colors[12][3] =
	{
	{1.0, 0.0, 0.0},
	{1.0, 0.5, 0.0},
	{1.0, 1.0, 0.0},
	{0.5, 1.0, 0.0},
	
	{0.0, 1.0, 0.0},
	{0.0, 1.0, 0.5},
	{0.0, 1.0, 1.0},
	{0.0, 0.5, 1.0},
	
	{0.0, 0.0, 1.0},
	{0.5, 0.0, 1.0},
	{1.0, 0.0, 1.0},
	{1.0, 0.0, 0.5}
	};
	
	glColor4f(Colors[cr][0],Colors[cr][1],Colors[cr][2],Opacity);
}


// Stuff for doing OpenGL rendering of various objects

	
// Storage of intermediate results for mass render with glDrawArrays
struct ExtendedVertexData
{
	GLdouble Vertex[4];
	GLdouble TexCoord[2];
	GLfloat Color[3];
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
	
	// Use that texture
	if (!TMgr.Setup(OGL_Txtr_Wall)) return false;
			
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
	
		assert(VertCoords.FindComplements());
		
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
	
	if (RenderPolygon.flags&_SHADELESS_BIT)
		// The shadeless color is E-Z
		glColor3f(1,1,1);
	else if (RenderPolygon.ambient_shade < 0)
	{
		GLfloat Light = (- RenderPolygon.ambient_shade)/GLfloat(FIXED_ONE);
		glColor3f(Light,Light,Light);
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
				FindShadingColor(-ExtendedVertexList[k].Vertex[2],RenderPolygon.ambient_shade,ExtendedVertexList[k].Color);
		}
		else
			glColor3f(Light,Light,Light);
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
	SetProjectionType(Projection_OpenGL_Eye);
	
	// Location of data:
	glVertexPointer(4,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	
	// Painting a texture...
	glEnable(GL_TEXTURE_2D);

	bool DoTwoSimultaneousTextures = OnePassMultitexturing && TMgr.IsGlowMapped();
	if (DoTwoSimultaneousTextures)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	TMgr.RenderNormal();
	if (DoTwoSimultaneousTextures)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		TMgr.RenderGlowing(true);
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
	
	if (PolygonVariableShade)
	{
		// Do triangulation by hand, creating a sort of ladder;
		// this is to avoid creating a triangle fan
		GLint VertIndices[3*(MAXIMUM_VERTICES_OF_SPLIT_POLYGON - 2)];
		
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
	if (DoTwoSimultaneousTextures)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D,0);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
	else if (TMgr.IsGlowMapped())
	{
		// Do blending here to get the necessary semitransparency;
		// push the cutoff down so 0.5*0.5 (half of half-transparency)
		// The cutoff is irrelevant if the texture is set to one of the blended modes
		// instead of the crisp mode.
		// Added "IsBlended" test, so that alpha-channel selection would work properly
		// on a glowmap texture that is atop a texture that is opaque to the void.
		glColor3f(1,1,1);
		glEnable(GL_BLEND);
		if (IsBlended)
		{
			glDisable(GL_ALPHA_TEST);
		} else {
			glEnable(GL_ALPHA_TEST);
		}
		glAlphaFunc(GL_GREATER,0.25);
		
		TMgr.RenderGlowing(false);
		glDrawArrays(GL_POLYGON,0,NumVertices);
		
		// Reset these values
		glAlphaFunc(GL_GREATER,0.5);
	}
	
	return true;
}


// Render the wall texture as a landscape;
// it returns whether or not the texture is a legitimate landscape texture
// and it does not care about the surface's orientation
static bool RenderAsLandscape(polygon_definition& RenderPolygon)
{
	// Check for fog
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	if (FogPresent)
	{
		// Render as fog at infinity
		glDisable(GL_FOG);
		glDisable(GL_TEXTURE_2D);
		
		// Set up the color
		glColor3fv(FogColor);
		
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
	TMgr.Landscape_AspRatExp = LandOpts->OGL_AspRatExp;
	
	// Use that texture
	if (!TMgr.Setup(OGL_Txtr_Landscape)) return false;
	
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
		
	// Set up blending mode: opaque
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	
	// Proper projection
	SetProjectionType(Projection_Screen);
	
	// Location of data:
	glVertexPointer(3,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	
	// Painting a texture...
	glEnable(GL_TEXTURE_2D);
	TMgr.RenderNormal();
	
	// Go!
	glDrawArrays(GL_POLYGON,0,NumVertices);
	
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
	TMgr.ShadingTables = RenderRectangle.shading_tables;
	TMgr.Texture = RenderRectangle.texture;
	TMgr.TransferMode = RenderRectangle.transfer_mode;
	TMgr.TransferData = RenderRectangle.transfer_data;
	TMgr.IsShadeless = (RenderRectangle.flags&_SHADELESS_BIT) != 0;
	
	// Use that texture
	if (!TMgr.Setup(OGL_Txtr_Inhabitant,OGL_Txtr_WeaponsInHand)) return true;
	
	bool IsInhabitant = TMgr.GetTextureType() == OGL_Txtr_Inhabitant;
	bool IsWeaponsInHand = TMgr.GetTextureType() == OGL_Txtr_WeaponsInHand;
	
	// Find texture coordinates
	ExtendedVertexData ExtendedVertexList[4];
	
	point2d TopLeft, BottomRight;
	// Clipped corners:
	TopLeft.x = MAX(RenderRectangle.x0,RenderRectangle.clip_left);
	TopLeft.y = MAX(RenderRectangle.y0,RenderRectangle.clip_top);
	BottomRight.x = MIN(RenderRectangle.x1,RenderRectangle.clip_right);
	BottomRight.y = MIN(RenderRectangle.y1,RenderRectangle.clip_bottom);
	
	double RayDistance = double(RenderRectangle.depth);
	
	if (IsInhabitant && RayDistance > 0)
	{
		// OpenGL eye coordinates
		GLdouble VertexRay[3];
		Screen2Ray(TopLeft,VertexRay);
		VecScalarMult(VertexRay,RayDistance,ExtendedVertexList[0].Vertex);
		Screen2Ray(BottomRight,VertexRay);
		VecScalarMult(VertexRay,RayDistance,ExtendedVertexList[2].Vertex);
	}
	else if (IsWeaponsInHand && RayDistance == 0)
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
	
	// Apply lighting
	bool IsInvisible = false;
	if (RenderRectangle.transfer_mode == _tinted_transfer)
	{
		// Used for invisibility; the tinting refers to the already-rendered color's fate
		// The opacity is controlled by the transfer data; its value is guessed from
		// the rendering source code (render.c, scottish_textures.c, low_level_textures.c)
		glColor4f(0,0,0,1-RenderRectangle.transfer_data/32.0);
		IsInvisible = true;
	}
	else if (RenderRectangle.flags&_SHADELESS_BIT)
		// Only set when infravision is active
		glColor3f(1,1,1);
	else if (RenderRectangle.ambient_shade < 0)
	{
		GLfloat Light = (- RenderRectangle.ambient_shade)/GLfloat(FIXED_ONE);
		glColor3f(Light,Light,Light);
	}
	else {
		GLfloat Color[3];
		FindShadingColor(RayDistance,RenderRectangle.ambient_shade,Color);
		glColor3fv(Color);
	}
	
	// Make the sprites crisp-edged, except if they are in invisibility mode
	bool IsBlended = TMgr.IsBlended();
	if (IsInvisible || IsBlended)
	{
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	} else { 
		glEnable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
	}
	
	// Proper projection
	if (IsInhabitant)
		SetProjectionType(Projection_OpenGL_Eye);
	else if (IsWeaponsInHand)
		SetProjectionType(Projection_Screen);
	
	// Location of data:
	glVertexPointer(3,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	glEnable(GL_TEXTURE_2D);
	
	bool DoTwoSimultaneousTextures = OnePassMultitexturing && TMgr.IsGlowMapped();
	if (DoTwoSimultaneousTextures)
	{
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}
	
	// Go!
	TMgr.RenderNormal();	// Always do this, of course
	if (RenderRectangle.transfer_mode == _static_transfer)
	{
		if (UseFlatStatic)
		{
			// Per-sprite coloring
			for (int c=0; c<3; c++)
				FlatStaticColor[c] = StaticRandom.KISS() + StaticRandom.LFIB4();
			
			// Do flat-color version of static effect
			glColor3usv(FlatStaticColor);
			glDrawArrays(GL_POLYGON,0,4);
			
		} else {
			// Do multitextured stippling to create the static effect
			
			// First pass: base color (black)
			glColor3f(0,0,0);
			glDrawArrays(GL_POLYGON,0,4);
			
			glEnable(GL_POLYGON_STIPPLE);
			glEnable(GL_COLOR_LOGIC_OP);
			
			// Paint the appropriate color component in
			glLogicOp(GL_OR);
			
			// Each of the primary colors
			const GLfloat StaticBaseColors[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
			
			// Remaining passes, one for each primary color
			for (int c=0; c<3; c++)
			{
				glColor3fv(StaticBaseColors[c]);			
				glPolygonStipple((byte *)StaticPatterns[c]);
				glDrawArrays(GL_POLYGON,0,4);
			}
			glDisable(GL_COLOR_LOGIC_OP);
			glDisable(GL_POLYGON_STIPPLE);
		}
	}
	else
	{
		// Do textured rendering
		if (DoTwoSimultaneousTextures)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			TMgr.RenderGlowing(true);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			
			glDrawArrays(GL_POLYGON,0,4);
			
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBindTexture(GL_TEXTURE_2D,0);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
		else
		{
			glDrawArrays(GL_POLYGON,0,4);
			
			if (TMgr.IsGlowMapped())
			{
				// Do blending here to get the necessary semitransparency;
				// push the cutoff down so 0.5*0.5 (half of half-transparency)
				glColor3f(1,1,1);
				glEnable(GL_BLEND);
				glAlphaFunc(GL_GREATER,0.25);
				
				TMgr.RenderGlowing(false);
				glDrawArrays(GL_POLYGON,0,4);
				
				// Reset these values
				glAlphaFunc(GL_GREATER,0.5);
			}
		}
	}
		
	return true;
}


// Rendering crosshairs
bool OGL_RenderCrosshairs()
{
	if (!OGL_IsActive()) return false;
	
	// Crosshair features
	CrosshairData& Crosshairs = GetCrosshairData();
	
	// Proper projection
	SetProjectionType(Projection_Screen);
	
	// What color
	glColor3usv((GLushort *)&Crosshairs.Color);

	// No textures painted here	
	glDisable(GL_TEXTURE_2D);
	
	// The center:
	short XCen = ViewWidth >> 1;
	short YCen = ViewHeight >> 1;
	/*
	short XCen = ((SavedViewBounds.left + SavedViewBounds.right) >> 1);
	short YCen = ((SavedViewBounds.top + SavedViewBounds.bottom) >> 1);
	*/
	
	// Create a new modelview matrix for the occasion
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	// Move to center of screen and in front of the other stuff
	glTranslated(XCen,YCen,1);
	
	// The line width
	glLineWidth(Crosshairs.Thickness);
	
	// Draw the lines; make them foreground
	glBegin(GL_LINES);
	glVertex2s(- Crosshairs.FromCenter + 1, 0);
	glVertex2s(- Crosshairs.FromCenter - Crosshairs.Length + 1, 0);
	glVertex2s(0, - Crosshairs.FromCenter + 1);
	glVertex2s(0, - Crosshairs.FromCenter - Crosshairs.Length + 1);
	glVertex2s(Crosshairs.FromCenter - 1, 0);
	glVertex2s(Crosshairs.FromCenter + Crosshairs.Length - 1, 0);
	glVertex2s(0, Crosshairs.FromCenter - 1);
	glVertex2s(0, Crosshairs.FromCenter + Crosshairs.Length - 1);
	glEnd();
	
	// Done with that modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	// Reset to default
	glLineWidth(1);
		
	return true;
}

// Rendering text; this takes it as a Pascal string (byte 0 = number of text bytes)
bool OGL_RenderText(short BaseX, short BaseY, unsigned char *Text)
{
	if (!OGL_IsActive()) return false;
	
	// Create display list for the current text string;
	// use the "standard" text-font display list (display lists can be nested)
	GLuint TextDisplayList;
	TextDisplayList = glGenLists(1);
	glNewList(TextDisplayList,GL_COMPILE);
	for (int b=1; b<=Text[0]; b++)
	{
		GLuint ByteDisplayList = FontDisplayList + Text[b];
		glCallLists(1,GL_UNSIGNED_INT,&ByteDisplayList);
	}
	glEndList();
	
	// Place the text in the foreground of the display
	SetProjectionType(Projection_Screen);
	short Depth = 1;
	
	// Background
	glColor3f(0,0,0);
	
	glRasterPos3s(BaseX-1,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX+1,BaseY-1,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX-1,BaseY,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX+1,BaseY,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX-1,BaseY+1,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX,BaseY+1,Depth);
	glCallList(TextDisplayList);
	
	glRasterPos3s(BaseX+1,BaseY+1,Depth);
	glCallList(TextDisplayList);
	
	// Foreground
	glColor3f(1,1,1);
	glRasterPos3s(BaseX,BaseY,Depth);
	glCallList(TextDisplayList);

	glDeleteLists(TextDisplayList,1);
	
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


// Returns whether or not 2D stuff is to be piped through OpenGL
bool OGL_Get2D()
{
	// Can be called when OpenGL is inactive
	if (!OGL_IsActive()) return false;
	
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	return (TEST_FLAG(ConfigureData.Flags,OGL_Flag_2DGraphics) != 0);
}

// Copying 2D display: status bar, overhead map, terminal
bool OGL_Copy2D(GWorldPtr BufferPtr, Rect& SourceBounds, Rect& DestBounds, bool UseBackBuffer, bool FrameEnd)
{
	// Check to see whether to do the copying (returns false when OpenGL is inactive)
	if (!OGL_Get2D()) return false;
	
	// Paint onto the currently-visible area:
	if (!UseBackBuffer) glDrawBuffer(GL_FRONT);
	
	// Where to draw
	Rect& ViewBounds = ((CGrafPtr)BufferPtr)->portRect;
	
	// Reallocate strip buffer if necessary
	short Width = ViewBounds.right - ViewBounds.left;
	if (Width != Buffer2D_Width)
	{
		if (Buffer2D) delete []Buffer2D;
		if (Width > 0)
		{
			Buffer2D_Width = Width;
			Buffer2D = new GLuint[Buffer2D_Width*Buffer2D_Height];
		} else {
			Buffer2D = NULL;
			Buffer2D_Width = 0;
		}
	}
	
	// Number of source bytes (destination bytes = 4)
	short NumSrcBytes = bit_depth/8;
	
	// Set up conversion table
	// MakeConversion_16to32(bit_depth);
	
	// Get pointer to starting buffer	
	PixMapHandle Pxls = GetGWorldPixMap(BufferPtr);
	LockPixels(Pxls);
	
	// Row-start address and row length
	byte *BufferStart = (byte *)GetPixBaseAddr(Pxls);
	long StrideBytes = (**Pxls).rowBytes & 0x7fff;
	
	// How many pixels to draw per line
	short SourceWidth = SourceBounds.right - SourceBounds.left;
	
	for (int h=SourceBounds.top, hstrip=0; h<SourceBounds.bottom; h++)
	{
		// Set where to read from the input buffer
		byte *InPtr = BufferStart + (h-ViewBounds.top)*StrideBytes + NumSrcBytes*(SourceBounds.left-ViewBounds.left);
		
		// Set the strip buffer's current-pixel pointer;
		// be careful to reverse the row order, because most 2D graphics
		// goes top to bottom while OpenGL goes bottom to top.
		// The buffer will be used in a packed manner; in OpenGL, this is not necessary,
		// thanks to glPixelStorei().
		GLuint *OutPtr = Buffer2D + ((Buffer2D_Height - 1) - hstrip)*SourceWidth;
		
		// Fill the buffer
		if (NumSrcBytes == 2)
		{
			for (int w=0; w<SourceWidth; w++)
			{	
				// Big-endian here
				word Intmd = *(InPtr++);
				Intmd <<= 8;
				Intmd |= word(*(InPtr++));
				// Convert from ARGB 5551 to RGBA 8888; make opaque
				*(OutPtr++) = Convert_16to32(Intmd);
				// *(OutPtr++) = ConversionTable_16to32[Intmd & 0x7fff];
			}
		}
		else if (NumSrcBytes == 4)
		{
			for (int w=0; w<SourceWidth; w++)
			{
				// Convert from ARGB 8888 to RGBA 8888; make opaque
				// This makes the (reasonable) assumption of correct alignment of buffer data
				GLuint *InPxl = (GLuint *)InPtr;
				InPtr += 4;
				GLuint Pxl = *InPxl;
				Pxl <<= 8;
				Pxl |= 0x000000ff;
				*(OutPtr++) = Pxl;
			}
		}
		
		// Draw it if the strip buffer has become full;
		// then reset for the next go-around
		if (++hstrip >= Buffer2D_Height || h == (SourceBounds.bottom-1))
		{
			glRasterPos2s(DestBounds.left,h+(DestBounds.top-SourceBounds.top));
			glDrawPixels(SourceWidth,hstrip,GL_RGBA,GL_UNSIGNED_BYTE,
				Buffer2D + (Buffer2D_Height - hstrip)*SourceWidth);
			hstrip = 0;
		}
	}
	
	// All done!
	if (FrameEnd) aglSwapBuffers(RenderContext);		
	UnlockPixels(Pxls);
	if (!UseBackBuffer) glDrawBuffer(GL_BACK);
	
	return true;
}
