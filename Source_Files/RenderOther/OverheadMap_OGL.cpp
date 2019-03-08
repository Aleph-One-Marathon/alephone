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
	
	Overhead-Map OpenGL Class Implementation
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering with OpenGL
	Code originally from OGL_Map.c; font handling is still MacOS-specific.

[Notes from OGL_Map.c]
	This is for drawing the Marathon overhead map with OpenGL, because at large resolutions,
	the main CPU can be too slow for this.
	
	Much of this is cribbed from overhead_map_macintosh.c and translated into OpenGL form
	
July 9, 2000:

	Complete this OpenGL renderer. I had to add a font-info cache, so as to avoid
	re-generating the fonts for every frame. The font glyphs and offsets are stored
	as display lists, which appears to be very efficient.

July 16, 2000:

	Added begin/end pairs for line and polygon rendering; the purpose of these is to allow
	more efficient caching.

Jul 17, 2000:

	Paths now cached and drawn as a single line strip per path.
	Lines now cached and drawn in groups with the same width and color;
		that has yielded a significant performance improvement.
	Same for the polygons, but with relatively little improvement.
[End notes]

Aug 6, 2000 (Loren Petrich):
	Added perimeter drawing to drawing commands for the player object;
	this guarantees that this object will always be drawn reasonably correctly
	
Oct 13, 2000 (Loren Petrich)
	Converted the various lists into Standard Template Library vectors

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for AGL.h
*/

#include <math.h>
#include <string.h>

#include "cseries.h"
#include "OverheadMap_OGL.h"
#include "map.h"
#include "screen.h"

#ifdef HAVE_OPENGL

#ifdef HAVE_OPENGL
#include "OGL_Headers.h"
#include "OGL_Render.h"
#endif



// rgb_color straight to OpenGL
static inline void SetColor(rgb_color& Color)
{
	if (map_is_translucent())
		glColor4us(Color.red, Color.green, Color.blue, 32767);
	else
		glColor3usv((unsigned short *)(&Color));
}

// Need to test this so as to find out when the color changes
static inline bool ColorsEqual(rgb_color& Color1, rgb_color& Color2)
{
	return
		((Color1.red == Color2.red) &&
			(Color1.green == Color2.green) &&
				(Color1.blue == Color2.blue));
}


// For marking out the area to be blanked out when starting rendering;
// these are defined in OGL_Render.cpp
extern short ViewWidth, ViewHeight;

void OverheadMap_OGL_Class::begin_overall()
{

	// Blank out the screen
	// Do that by painting a black polygon
	if (!map_is_translucent())
	{
		glColor3f(0,0,0);
		OGL_RenderRect(0, 0, ViewWidth, ViewHeight);
	}
	
/*
	glEnable(GL_SCISSOR_TEST);	// Don't erase the HUD
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
*/
	
	// Here's for the overhead map
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	if (map_is_translucent())
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OverheadMap_OGL_Class::end_overall()
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}


void OverheadMap_OGL_Class::begin_polygons()
{
	// Polygons are rendered before lines, and use the endpoint array,
	// so both of them will have it set here. Using the compiled-vertex extension,
	// however, makes everything the same color :-P
	glVertexPointer(2,GL_SHORT,GetVertexStride(),GetFirstVertex());

	// Reset color defaults
	SavedColor.red = SavedColor.green = SavedColor.blue = 0;
	SetColor(SavedColor);
	
	// Reset cache to zero length
	PolygonCache.clear();
}

void OverheadMap_OGL_Class::draw_polygon(
	short vertex_count,
	short *vertices,
	rgb_color& color)
{
	// Test whether the polygon parameters have changed
	bool AreColorsEqual = ColorsEqual(color,SavedColor);
	
	// If any change, then draw the cached lines with the *old* parameters,
	// Set the new parameters
	if (!AreColorsEqual)
	{
		DrawCachedPolygons();
		SavedColor = color;
		SetColor(SavedColor);
	}
	
	// Implement the polygons as triangle fans
	for (int k=2; k<vertex_count; k++)
	{
		PolygonCache.push_back(vertices[0]);
		PolygonCache.push_back(vertices[k-1]);
		PolygonCache.push_back(vertices[k]);
	}
	
	// glDrawElements(GL_POLYGON,vertex_count,GL_UNSIGNED_SHORT,vertices);
}

void OverheadMap_OGL_Class::end_polygons()
{
	DrawCachedPolygons();
}

void OverheadMap_OGL_Class::DrawCachedPolygons()
{
	glDrawElements(GL_TRIANGLES, PolygonCache.size(),
		GL_UNSIGNED_SHORT, PolygonCache.data());
	PolygonCache.clear();
}

void OverheadMap_OGL_Class::begin_lines()
{
	// Reset color and pen size to defaults
	SetColor(SavedColor);
	SavedPenSize = 1;
	
	// Reset cache to zero length
	LineCache.clear();
}


void OverheadMap_OGL_Class::draw_line(
	short *vertices,
	rgb_color& color,
	short pen_size)
{
	// Test whether the line parameters have changed
	bool AreColorsEqual = ColorsEqual(color,SavedColor);
	bool AreLinesEquallyWide = (pen_size == SavedPenSize);
	
	// If any change, then draw the cached lines with the *old* parameters
	if (!AreColorsEqual || !AreLinesEquallyWide) DrawCachedLines();
	
	// Set the new parameters
	if (!AreColorsEqual)
	{
		SavedColor = color;
		SetColor(SavedColor);
	}
	
	if (!AreLinesEquallyWide)
	{
		SavedPenSize = pen_size;
	}
	
	// Add the line's points to the cached line
	LineCache.push_back(GetVertex(vertices[0]));
	LineCache.push_back(GetVertex(vertices[1]));
}

void OverheadMap_OGL_Class::end_lines()
{
	DrawCachedLines();
}

void OverheadMap_OGL_Class::DrawCachedLines()
{
	OGL_RenderLines(LineCache, SavedPenSize);
	LineCache.clear();
}


void OverheadMap_OGL_Class::draw_thing(
	world_point2d& center,
	rgb_color& color,
	short shape,
	short radius)
{
	SetColor(color);
	
	// Let OpenGL do the transformation work
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x,center.y,0);
	glScalef(radius, radius, 1);

	switch(shape)
	{
	case _rectangle_thing:
		{
			OGL_RenderRect(-0.75f, -0.75f, 1.5f, 1.5f);
		}
		break;
	case _circle_thing:
		{
			GLfloat ft = 0.1f;
			GLfloat ht = ft * 0.5f;
			GLfloat vertices[36] = {
				-0.30f - ht, -0.75f,
				-0.30f + ht, -0.75f + ft,
				+0.30f + ht, -0.75f,
				+0.30f - ht, -0.75f + ft,
				+0.75f - ft, -0.30f + ht,
				+0.75f,      -0.30f - ht,
				+0.75f - ft, +0.30f - ht,
				+0.75f,      +0.30f + ht,
				+0.30f - ht, +0.75f - ft,
				+0.30f + ht, +0.75f,
				-0.30f + ht, +0.75f - ft,
				-0.30f - ht, +0.75f,
				-0.75f + ft, +0.30f - ht,
				-0.75f,      +0.30f + ht,
				-0.75f + ft, -0.30f + ht,
				-0.75f,      -0.30f - ht,
				-0.30f - ht, -0.75f,
				-0.30f + ht, -0.75f + ft
			};
			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
		}
		break;
	default:
		break;
	}
	glPopMatrix();
}

void OverheadMap_OGL_Class::draw_player(
	world_point2d& center,
	angle facing,
	rgb_color& color,
	short shrink,
	short front,
	short rear,
	short rear_theta)
{
	SetColor(color);
	
	// The player is a simple triangle
	GLfloat PlayerShape[3][2];
	
	double rear_theta_rads = rear_theta*(8*atan(1.0)/FULL_CIRCLE);
	float rear_x = (float)(rear*cos(rear_theta_rads));
	float rear_y = (float)(rear*sin(rear_theta_rads));
	PlayerShape[0][0] = front;
	PlayerShape[0][1] = 0;
	PlayerShape[1][0] = rear_x;
	PlayerShape[1][1] = rear_y;
	PlayerShape[2][0] = rear_x;
	PlayerShape[2][1] = - rear_y;
	
	// Let OpenGL do the transformation work
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(center.x,center.y,0);
	glRotatef(facing*(360.0F/FULL_CIRCLE),0,0,1);
	float scale = 1/float(1 << shrink);
	glScalef(scale,scale,1);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2,GL_FLOAT,0,PlayerShape[0]);
	glDrawArrays(GL_POLYGON,0,3);

	glPopMatrix();
}

	
// Text justification: 0=left, 1=center
void OverheadMap_OGL_Class::draw_text(
	world_point2d& location,
	rgb_color& color,
	char *text,
	FontSpecifier& FontData,
	short justify)
{	
	// Find the left-side location
	world_point2d left_location = location;
	switch(justify)
	{
	case _justify_left:
		break;
		
	case _justify_center:
		left_location.x -= (FontData.TextWidth(text)>>1);
		break;
		
	default:
		return;
	}
	
	// Set color and location	
	SetColor(color);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(left_location.x,left_location.y,0);	
	FontData.OGL_Render(text);
	glPopMatrix();
}
	
void OverheadMap_OGL_Class::set_path_drawing(rgb_color& color)
{
	SetColor(color);
}

void OverheadMap_OGL_Class::draw_path(
	short step,	// 0: first point
	world_point2d &location)
{
	// At first step, reset the length
	if (step <= 0) PathPoints.clear();
	
	// Duplicate points to form lines at each step
	if (PathPoints.size() > 1)
		PathPoints.push_back(PathPoints.back());

	// Add the point
	PathPoints.push_back(location);
}

void OverheadMap_OGL_Class::finish_path()
{
	OGL_RenderLines(PathPoints, 1);
	PathPoints.clear();
}
#endif // def HAVE_OPENGL
