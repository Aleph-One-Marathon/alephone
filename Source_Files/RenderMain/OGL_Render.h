#ifndef _OGL_RENDER_
#define _OGL_RENDER_
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
#include "render.h"


// These functions return whether OpenGL is active;
// if OpenGL is not present, it will never be active.

// Test for activity
bool OGL_IsActive();


// It will be black; whether OpenGL is active will be returned
bool OGL_ClearScreen();


// Start an OpenGL run (creates a rendering context)
bool OGL_StartRun();

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

// Sets the view to what's suitable for rendering foreground objects
// like weapons in hand
bool OGL_SetForeground();

// Sets whether a foreground object is horizontally reflected
bool OGL_SetForegroundView(bool HorizReflect);

// Start and end rendering of main view 
bool OGL_StartMain();
bool OGL_EndMain();

// Stuff for doing OpenGL rendering of various objects
// The wall renderer takes a flag that indicates whether or not it is vertical
bool OGL_RenderWall(polygon_definition& RenderPolygon, bool IsVertical);
bool OGL_RenderSprite(rectangle_definition& RenderRectangle);

// Rendering crosshairs
bool OGL_RenderCrosshairs();

// Rendering text; this takes it as a C string
bool OGL_RenderText(short BaseX, short BaseY, const char *Text, unsigned char r = 0xff, unsigned char g = 0xff, unsigned char b = 0xff);

// Render cursor for Lua/chat console
bool OGL_RenderTextCursor(const SDL_Rect& rect, unsigned char r = 0xff, unsigned char g = 0xff, unsigned char b = 0xff);

// Render rectangles (set color beforehand)
void OGL_RenderRect(float x, float y, float w, float h);
void OGL_RenderRect(const SDL_Rect& rect);

void OGL_RenderTexturedRect(float x, float y, float w, float h, float tleft, float ttop, float tright, float tbottom);
void OGL_RenderTexturedRect(const SDL_Rect& rect, float tleft, float ttop, float tright, float tbottom);

void OGL_RenderFrame(float x, float y, float w, float h, float thickness);

// Render lines (for overhead map)
void OGL_RenderLines(const std::vector<world_point2d>& points, float thickness);

// Returns whether or not 2D stuff is to be piped through OpenGL
bool OGL_Get2D();

#endif
