#ifndef _OGL_RENDER_
#define _OGL_RENDER_
/*
	
	OpenGL Interface File,
	by Loren Petrich,
	March 12, 2000

	This contains functions intended to interface OpenGL 3D-rendering code
	with the rest of the Marathon source code. It was separated from the
	presence-accessing and parameter-accessing code in OGL_Control because
	this include file contains some stuff used by the rendering code.

July 8, 2000:
	Modified OGL_SetView() and OGL_Copy2D() to control whether or not to use a back buffer,
	and whether or not to write to the back buffer, respectively.
*/


#include "OGL_Setup.h"

// These functions return whether OpenGL is active;
// if OpenGL is not present, it will never be active.

// Test for activity
bool OGL_IsActive();

// Start an OpenGL run (creates a rendering context)
bool OGL_StartRun(CGrafPtr WindowPtr);

// Stop an OpenGL run (destroys a rendering context)
bool OGL_StopRun();

// Sets the infravision tinting color for a shapes collection, and whether to use such tinting;
// the color values are from 0 to 1.
bool OGL_SetInfravisionTint(short Collection, bool IsTinted, float Red, float Green, float Blue);

// Set OpenGL rendering-window bounds;
// these are calculated using the following boundary Rects:
// The screen (gotten from its portRect)
// The view (here, the main rendering view)
// Whether to allocate a back buffer
bool OGL_SetWindow(Rect &ScreenBounds, Rect &ViewBounds, bool UseBackBuffer);

// Swap buffers (reveal rendered image)
bool OGL_SwapBuffers();

// Set view parameters; this is for proper perspective rendering
bool OGL_SetView(view_data &View);

// Stuff for doing OpenGL rendering of various objects
// The wall renderer takes a flag that indicates whether or not it is vertical
bool OGL_RenderWall(polygon_definition& RenderPolygon, bool IsVertical);
bool OGL_RenderSprite(rectangle_definition& RenderRectangle);

// Rendering crosshairs
bool OGL_RenderCrosshairs();

// Rendering text; this takes it as a Pascal string (byte 0 = number of text bytes)
bool OGL_RenderText(short BaseX, short BaseY, unsigned char *Text);

// Returns whether or not 2D stuff is to be piped through OpenGL
bool OGL_Get2D();

// Copying 2D display: status bar, overhead map, terminal;
// Needs GWorld to copy from, region to copy, whether to copy into a back buffer,
// and whether or not this completes a frame.
bool OGL_Copy2D(GWorldPtr BufferPtr, Rect& SourceBounds, Rect& DestBounds, bool UseBackBuffer, bool FrameEnd);

#endif
