#ifndef __SCOTTISH_TEXTURES_H
#define __SCOTTISH_TEXTURES_H

/*
SCOTTISH_TEXTURES.H

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

Thursday, April 28, 1994 4:54:54 PM

Feb 17, 2000 (Loren Petrich):
	Made the polygons' origin positions and sprite rectangles' depths
	better adapted to long distances

Mar 12, 2000 (Loren Petrich):
	Added shape descriptors to the wall-texture render objects
	for the convenience of OpenGL rendering;
	also added object indices to the sprite.
	Weapons in hand have a special index.

Aug 9, 2000 (Loren Petrich):
	A few obsolete constants deleted;
	rendering functions moved to Rasterizer_SW.h

May 3, 2003 (Br'fin (Jeremy Parsons))
	Added LowLevelShape workaround for passing LowLevelShape info of sprites
	instead of abusing/overflowing shape_descriptors
*/

#include "cseries.h"
#include "OGL_Headers.h"
#include "world.h"
#include "shape_descriptors.h"

/* ---------- constants */

#define MINIMUM_VERTICES_PER_SCREEN_POLYGON ((short)3)
#define MAXIMUM_VERTICES_PER_SCREEN_POLYGON ((short)16)

enum /* transfer modes */
{
	_tinted_transfer, /* pass background through given shading table for non-transparent
		pixels; config word is $mmnn: mm is a mask applied to a random number in [0,32)
		which is then added to nn and used to retrieve a tinting table.  ‘ordinary’ (pathways-style)
		tinting can be accomplished by passing an alternate shading table. */
	_solid_transfer, /* writes (0,0) color of texture for non-transparent pixels; does not
		respect shading */
	_big_landscaped_transfer, /* does not distort texture (texture is anchored in screen-space) */
	_textured_transfer, /* distorts texture to match surface */
	_shadeless_transfer, /* does not respect any lighting information; uses a single shading table */
	_static_transfer /* writes static for non-transparent pixels (config word is the unsigned
		chance in 64k that a non-transparent pixel will become transparent).  does not respect
		shading. */
//	_big_landscaped_transfer
};

/* ---------- shading tables */

#define MAXIMUM_SHADING_TABLE_INDEXES PIXEL8_MAXIMUM_COLORS

#define FIRST_SHADING_TABLE 0

struct tint_table8
{
	pixel8 index[PIXEL8_MAXIMUM_COLORS];
};

struct tint_table16
{
	pixel16 red[PIXEL16_MAXIMUM_COMPONENT+1];
	pixel16 green[PIXEL16_MAXIMUM_COMPONENT+1];
	pixel16 blue[PIXEL16_MAXIMUM_COMPONENT+1];
};

struct tint_table32
{
	pixel32 red[PIXEL32_MAXIMUM_COMPONENT+1];
	pixel32 green[PIXEL32_MAXIMUM_COMPONENT+1];
	pixel32 blue[PIXEL32_MAXIMUM_COMPONENT+1];
};

/* ---------- structures */

struct point2d
{
	short x, y;
};

/* ignore multiple shading tables if set */
#define _SHADELESS_BIT 0x8000

class OGL_ModelData;

struct rectangle_definition
{
	uint16 flags;
	
	struct bitmap_definition *texture;
	
	/* screen coordinates; x0<x1, y0<y1 */
	int16 x0, y0;
	int16 x1, y1;

	/* screen coordinates */
	int16 clip_left, clip_right;
	int16 clip_top, clip_bottom;

	/* depth at logical center (used to calculate light due to viewer) */
	// LP change: made this long-distance friendly
	int32 depth;
	
	/* ambient shading table index; many objects will be self-luminescent, so this may have
		nothing to do with the polygon the object is sitting in */
	_fixed ambient_shade;
	
	/* here are all the shading tables, crammed together in memory */	
	void *shading_tables;

	/* _tinted, _textured and _static are supported; _solid would be silly and _landscape
		would be hard (but might be cool) */
	int16 transfer_mode, transfer_data;
	
	/* mirrored horizontally and vertically if true */
	bool flip_vertical, flip_horizontal;
	
	// LP addition: shape-descriptor value for the convenience of OpenGL;
	// the lower byte is the frame
	// Note that for the convenience of 3D-model rendering, more shape information may
	// eventually have to be transmitted.
	shape_descriptor ShapeDesc;
	uint16 LowLevelShape;
	
	// For the convenience of the teleporter: shrink to the median axis,
	// with this position on the screen
	int16 xc;
	
	// Set overall, so as to make semitransparent chase-cam mode easy (0 to 1)
	float Opacity;
	
	// For the convenience of the OpenGL 3D-model renderer
	_fixed ceiling_light;		// The ambient_shade is the floor light
	OGL_ModelData *ModelPtr;	// For models
	short ModelSequence, ModelFrame, NextModelFrame;	// For model animation
	float MixFrac;				// Mixture between current and next frame
	world_point3d Position;		// In overall world coordinates
	short ProjDistance;			// Of centroid, from view plane
	short Azimuth;				// Facing angle
	float Scale;				// how big
	float HorizScale;			// For teleporting
	short LiquidRelHeight;		// Relative to object location
	bool BelowLiquid;			// Where the rendering is relative to the liquid
	int LightDepth;				// For finding the amount of "miner's light"
	GLfloat LightDirection[3];		// To the "light point"
	short WorldLeft, WorldBottom, WorldRight, WorldTop;

	// Constructor required defaults
	rectangle_definition() :
		LowLevelShape(0) {}
};

struct polygon_definition
{
	uint16 flags;
	
	struct bitmap_definition *texture; /* ignored for _tinted and _solid polygons */

	/* ambient shading table index */
	_fixed ambient_shade;

	/* here are all the shading tables, crammed together in memory (unless this is a tinted
		polygon in which case it is a single 256-byte tinting table) */
	void *shading_tables;	

	/* all modes legal */	
	int16 transfer_mode, transfer_data;
	
	// LP change: made this long-distance friendly
	long_point3d origin;
	world_vector3d vector; /* used only for vertically textured polygons */

	/* clockwise vertices for this convex polygon */
	int16 vertex_count;
	point2d vertices[MAXIMUM_VERTICES_PER_SCREEN_POLYGON];
	
	// LP addition: shape-descriptor value for the convenience of OpenGL;
	// the lower byte is the frame
	shape_descriptor ShapeDesc;
	
	// Whether the void is present on one side; useful for suppressing semitransparency
	bool VoidPresent;
};

/* ---------- globals */

extern short bit_depth;
extern short interface_bit_depth;

extern short number_of_shading_tables, shading_table_fractional_bits, shading_table_size;

/* ---------- prototypes/SCOTTISH_TEXTURES.C */

void allocate_texture_tables(void);

#endif
