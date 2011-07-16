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
	
	Overhead-Map Quickdraw Class Implementation
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering with Classic MacOS Quickdraw
	Code originally from overhead_map_macintosh.c

Aug 6, 2000 (Loren Petrich):
	Added perimeter drawing to drawing commands for the player object;
	this guarantees that this object will always be drawn reasonably correctly

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
*/

#include <string.h>
#include "OverheadMap_QD.h"

static inline void SetColor(rgb_color& Color) {RGBForeColor((RGBColor *)(&Color));}

void OverheadMap_QD_Class::draw_polygon(
	short vertex_count,
	short *vertices,
	rgb_color& color)
{
	PolyHandle polygon= OpenPoly();
	world_point2d *vertex= &GetVertex(vertices[vertex_count-1]);
	MoveTo(vertex->x, vertex->y);
	for (int i=0;i<vertex_count;++i)
	{
		vertex= &GetVertex(vertices[i]);
		LineTo(vertex->x, vertex->y);
	}
	ClosePoly();
	PenSize(1, 1);
	SetColor(color);
	Pattern black;
	GetQDGlobalsBlack(&black);
	FillPoly(polygon, &black);

	KillPoly(polygon);
}

void OverheadMap_QD_Class::draw_line(
	short *vertices,
	rgb_color& color,
	short pen_size)
{
	world_point2d *vertex1= &GetVertex(vertices[0]);
	world_point2d *vertex2= &GetVertex(vertices[1]);
	
	SetColor(color);
	PenSize(pen_size,pen_size);
	MoveTo(vertex1->x, vertex1->y);
	LineTo(vertex2->x, vertex2->y);
}

void OverheadMap_QD_Class::draw_thing(
	world_point2d& center,
	rgb_color& color,
	short shape,
	short radius)
{
	SetColor(color);
	// LP change: adjusted object display so that objects get properly centered;
	// they were inadvertently made 50% too large
	short raddown = short(0.75*radius);
	short radup = short(1.5*radius) - raddown;
	Rect bounds;
	SetRect(&bounds, center.x-raddown, center.y-raddown, center.x+radup, center.y+radup);
	switch (shape)
	{
		case _rectangle_thing:
			PaintRect(&bounds);
			break;
		case _circle_thing:
			PenSize(2, 2);
			FrameOval(&bounds);
			break;
	}
}


void OverheadMap_QD_Class::draw_player(
	world_point2d& center,
	angle facing,
	rgb_color& color,
	short shrink,
	short front,
	short rear,
	short rear_theta)
{
	short i;
	PolyHandle polygon;
	world_point2d triangle[3];

	/* Use our universal clut */
	// changed to transmitted color
	SetColor(color);
	
	triangle[0]= triangle[1]= triangle[2]= center;
	translate_point2d(triangle+0, front>>shrink, facing);
	translate_point2d(triangle+1, rear>>shrink, normalize_angle(facing+rear_theta));
	translate_point2d(triangle+2, rear>>shrink, normalize_angle(facing-rear_theta));
		
	// LP: Modified this to check on whether the sides are too short;
	// if so, then will render a line
	
	// Get the scale back
	polygon= OpenPoly();
	MoveTo(triangle[2].x, triangle[2].y);
	for (i=0;i<3;++i) LineTo(triangle[i].x, triangle[i].y);
	ClosePoly();
	PenSize(1, 1);		// LP: need only 1-pixel thickness of line
	Pattern black;
	GetQDGlobalsBlack(&black);
	FillPoly(polygon, &black);

	FramePoly(polygon);	// LP addition: perimeter drawing makes small version easier to see
	KillPoly(polygon);
}
	
// Text justification: 0=left, 1=center
void OverheadMap_QD_Class::draw_text(
	world_point2d& location,
	rgb_color& color,
	char *text,
	FontSpecifier& FontData,
	short justify)
{
	// Pascalify the name; watch out for buffer overflows
	Str255 pascal_text;
	char cstr[256];
	strncpy(cstr, text, 255);
	CopyCStringToPascal(cstr, pascal_text);

	// Needed up here for finding the width of the text string
	FontData.Use();

	// Find the left-side location
	world_point2d left_location = location;
	switch(justify)
	{
	case _justify_left:
		break;
		
	case _justify_center:
		left_location.x -= (StringWidth(pascal_text)>>1);
		break;
		
	default:
		return;
	}
	
	SetColor(color);
	MoveTo(left_location.x,left_location.y);
	DrawString(pascal_text);	
}

void OverheadMap_QD_Class::set_path_drawing(rgb_color& color)
{
	PenSize(1, 1);
	SetColor(color);
}

void OverheadMap_QD_Class::draw_path(
	short step,	// 0: first point
	world_point2d& location)
{
	step ? LineTo(location.x, location.y) : MoveTo(location.x, location.y);
}
