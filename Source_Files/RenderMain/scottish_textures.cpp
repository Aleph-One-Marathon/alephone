/*
SCOTTISH_TEXTURES.C

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

Wednesday, April 20, 1994 9:35:36 AM

this is not your father’s texture mapping library.
(in fact it isn’t yours either, dillweed)

Wednesday, April 20, 1994 3:39:21 PM
	vertical repeats would be difficult because it would require testing repeats in the
	innermost loop of the pixel mapper (a compare and branch we can do without).
Saturday, April 23, 1994 10:42:41 AM
	(on the plane to santa clara) finished the slower version of the trapezoid mapper (we
	need to handle stretching with a degenerate switch statement like marathon used to) but
	the whole sampling process is now mathematically correct except for the squared function
	we use to calculate the x texture position and the shading table (but this is accurate to
	within 1/64k and doesn't accumulate error so who cares).
Sunday, April 24, 1994 10:12:47 AM
	(waiting for the CGDC to start at 9:00 PST) added all polygon stuff.  it struck me this
	morning that clipping against the view cone must be deterministic (that is, line segments
	of polygons and line segments of walls must be clipped in the same manner) or our
	edges won't meet up.  ordered dither darkening will look really cool but will be slow in c.
Sunday, April 24, 1994 11:21:47 PM
	still need transparent trapezoids, dither darkening, faster DDA for trapezoid mapping.
Wednesday, April 27, 1994 9:49:55 AM
	i'm just looking for one divine hammer (to bang it all day).  solid polygons are currently
	unaffected by darkening.  i'm not entirely certain we'll even use them.
Sunday, May 8, 1994 8:32:11 AM
	LISP’s lexical contours kick C firmly and painfully in the ass. everything is fast now
	except the landscape mapper which has just been routed and is in full retreat.
Friday, May 13, 1994 10:05:08 AM
	low-level unification of trapezoids and rectangles, transparent runs in shapes are run-length
	encoded now.  maintaining run tables was slower than generating d, delta_d and delta_d_prime
	and using them on the fly.
Wednesday, May 18, 1994 2:16:26 PM
	scope matters (at WWDC).
Sunday, May 22, 1994 12:32:02 PM
	drawing things in column order to cached (i.e., non-screen) memory is like crapping in the
	data cache, right?  maybe drawing rectangles in column-order wasn't such a great idea after all.
	it also occurs to me that i know nothing about how to order instructions for the ’040 pipelines.
Thursday, June 16, 1994 9:56:14 PM
	modified _render_textured_polygon_line to handle elevation.
Thursday, July 7, 1994 1:23:09 PM
	changed MAXIMUM_SCRATCH_TABLE_ENTRIES from 4k to 1200.  Modified render code to work as well,
	now the problem is floor/ceiling matching with trapezoids, which should fall out with the 
	rewrite...
Tuesday, July 26, 1994 3:42:16 PM
	OBSOLETE’ed nearly the entire file (fixed_pixels are no more).  rewriting texture_rectangle.
	will do 16bit mapping, soon.  a while ago i rewrote everything in 68k.
Friday, September 16, 1994 6:03:11 PM  (Jason')
	texture_rectangle() now respects top and bottom clips
Tuesday, September 20, 1994 9:58:30 PM  (Jason')
	if we’re so close to a rectangle that n>LARGEST_N then we don’t draw anything
Wednesday, October 26, 1994 3:18:59 PM (Jason)
	for non-convex or otherwise weird lines (dx<=0, dy<=0) we don’t draw anything (somebody’ll
	notice that for sure).
Friday, November 4, 1994 7:35:48 PM  (Jason')
	pretexture_horizontal_polygon_lines() now respects the (x,y) polygon origin and uses z as height.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 24, 2000 (Loren Petrich):
	Using a special "landscape yaw" for the landscape texturing, so that the landscape center
	will stay put.

May 23, 2000 (Loren Petrich):
	Adding support for different size scales for landscapes

Jul 6, 2000 (Loren Petrich):
	Added some slop to MAXIMUM_SCRATCH_TABLE_ENTRIES, because displays are now bigger;
	its size got upped by 2

Aug 9, 2000 (Loren Petrich):
	Rasterizer_SW object introduced (software subclass of rasterizer object)

Aug 12, 2001 (Ian Rickard):
	Added prototypes for new horizontal surface rasterizers

Aug 19, 2001 (Ian Rickard):
	some changes for changed render depth global

Aug 22, 2001 (Ian Rickard):
	Implemented translucent surface drawing.

*/

/*
rectangle shrinking has vertical error and appears to randomly shear the bitmap
pretexture_horizontal_polygon_lines() has integer error in large height cases

_static_transfer doesn’t work for ceilings and floors (because they call the wall mapper)
build_y_table and build_x_table could both be sped up in nearly-horizontal and nearly-vertical cases (respectively)
_pretexture_vertical_polygon_lines() takes up to half the time _texture_vertical_polygon_lines() does
not only that, but texture_horizontal_polygon() is actually faster than texture_vertical_polygon()

//calculate_shading_table() needs to be inlined in a macro
*/

#include "cseries.h"
#include "render.h"
#include "Rasterizer_SW.h"
// IR addition:
#include "upscaled_textures.h"
#include "shape_descriptors.h"
#include "interface.h"

#include <stdlib.h>
#include <limits.h>

#include "readout_data.h"

#ifdef env68k
#pragma segment texture
#endif

#ifdef env68k
#define EXTERNAL
#endif

/* ---------- constants */

// LP change: boosted to cope with big displays
#define MAXIMUM_SCRATCH_TABLE_ENTRIES 2048
#define MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE (MAX(sizeof(_vertical_polygon_data), sizeof(_horizontal_polygon_line_data)))

#define SHADE_TO_SHADING_TABLE_INDEX(shade) ((shade)>>(FIXED_FRACTIONAL_BITS-shading_table_fractional_bits))
#define DEPTH_TO_SHADE(d) (((_fixed)(d))<<(FIXED_FRACTIONAL_BITS-WORLD_FRACTIONAL_BITS-3))

#define LARGEST_N 24

<<<<<<< scottish_textures.cpp
// LP: LANDSCAPE_REPEAT_BITS is defined later;
// (1 << LANDSCAPE_REPEAT_BITS) is the number of landscape repeats

// IR change: translucent surfaces demands way too many textureres to stick in this file.
#include "texturers.h"
=======
/* ---------- texture horizontal polygon */

#define HORIZONTAL_WIDTH_SHIFT 7 /* 128 (8 for 256) */
#define HORIZONTAL_HEIGHT_SHIFT 7 /* 128 */
#define HORIZONTAL_FREE_BITS (32-TRIG_SHIFT-WORLD_FRACTIONAL_BITS)
#define HORIZONTAL_WIDTH_DOWNSHIFT (32-HORIZONTAL_WIDTH_SHIFT)
#define HORIZONTAL_HEIGHT_DOWNSHIFT (32-HORIZONTAL_HEIGHT_SHIFT)

struct _horizontal_polygon_line_header
{
	int32 y_downshift;
};

struct _horizontal_polygon_line_data
{
	uint32 source_x, source_y;
	uint32 source_dx, source_dy;
	
	void *shading_table;
};

/* ---------- texture vertical polygon */

#define VERTICAL_TEXTURE_WIDTH 128
#define VERTICAL_TEXTURE_WIDTH_BITS 7
#define VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS (FIXED_FRACTIONAL_BITS-VERTICAL_TEXTURE_WIDTH_BITS)
#define VERTICAL_TEXTURE_ONE (1<<VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS)
#define VERTICAL_TEXTURE_FREE_BITS FIXED_FRACTIONAL_BITS
#define VERTICAL_TEXTURE_DOWNSHIFT (32-VERTICAL_TEXTURE_WIDTH_BITS)

#define HORIZONTAL_WIDTH_SHIFT 7 /* 128 (8 for 256) */
#define HORIZONTAL_HEIGHT_SHIFT 7 /* 128 */
#define HORIZONTAL_FREE_BITS (32-TRIG_SHIFT-WORLD_FRACTIONAL_BITS)
#define HORIZONTAL_WIDTH_DOWNSHIFT (32-HORIZONTAL_WIDTH_SHIFT)
#define HORIZONTAL_HEIGHT_DOWNSHIFT (32-HORIZONTAL_HEIGHT_SHIFT)

struct _vertical_polygon_data
{
	int16 downshift;
	int16 x0;
	int16 width;
	
	int16 pad;
};

struct _vertical_polygon_line_data
{
	void *shading_table;
	pixel8 *texture;
	int32 texture_y, texture_dy;
};
>>>>>>> 1.16

/* ---------- macros */

// i0 + i1 == MAX(i0, i1) + MIN(i0, i1)/2
#define CALCULATE_SHADING_TABLE(result, view, shading_tables, depth, ambient_shade) \
{ \
	short table_index; \
	_fixed shade; \
	 \
	if ((ambient_shade)<0) \
	{ \
		table_index= SHADE_TO_SHADING_TABLE_INDEX(-(ambient_shade)); \
	} \
	else \
	{ \
		shade= (view)->maximum_depth_intensity - DEPTH_TO_SHADE(depth); \
		shade= PIN(shade, 0, FIXED_ONE); \
		table_index= SHADE_TO_SHADING_TABLE_INDEX((ambient_shade>shade) ? (ambient_shade + (shade>>1)) : (shade + (ambient_shade>>1))); \
	} \
	 \
	switch (g_render_depth) \
	{ \
		case 8: result= ((byte*)(shading_tables)) + MAXIMUM_SHADING_TABLE_INDEXES*sizeof(pixel8)* \
			CEILING(table_index, number_of_shading_tables-1); break; \
		case 16: result= ((byte*)(shading_tables)) + MAXIMUM_SHADING_TABLE_INDEXES*sizeof(pixel16)* \
			CEILING(table_index, number_of_shading_tables-1); break; \
		case 32: result= ((byte*)(shading_tables)) + MAXIMUM_SHADING_TABLE_INDEXES*sizeof(pixel32)* \
			CEILING(table_index, number_of_shading_tables-1); break; \
	} \
}

//		table_index= SHADE_TO_SHADING_TABLE_INDEX((view)->maximum_depth_intensity - DEPTH_TO_SHADE(depth));
//		table_index= PIN(table_index, 0, number_of_shading_tables);
//		table_index= MAX(SHADE_TO_SHADING_TABLE_INDEX(ambient_shade), table_index);

/* ---------- globals */

/* these tables are used by the polygon rasterizer (to store the x-coordinates of the left and
	right lines of the current polygon), the trapezoid rasterizer (to store the y-coordinates
	of the top and bottom of the current trapezoid) and the rectangle mapper (for it’s
	vertical and if necessary horizontal distortion tables).  these are not necessary as
	globals, just as global storage. */
//static short *scratch_table0, *scratch_table1;
static void *precalculation_table;
static void *clip_table;

// IR addition: upscaled textures for software mode!  Yes!
UpscaledTextureManager upscaledTextures(3000000);

// IR change: need to access this from the texturers, so its not static any more.
uint16 texture_random_seed= 1;

/* ---------- private prototypes */

static void CheckTransparency(bitmap_definition *bitmap);

static void _pretexture_horizontal_polygon_lines(struct polygon_definition *polygon,
	struct bitmap_definition *screen, struct view_data *view, struct _horizontal_polygon_line_data *data,
	short y0, short line_count);

// IR removed: moved the protytpes for texturers into texturers.h

static void _pretexture_vertical_polygon_lines(struct polygon_definition *polygon,
	struct bitmap_definition *screen, struct view_data *view, struct _vertical_polygon_data *data,
	short x0, short line_count);

// IR removed: moved the protytpes for texturers into texturers.h

// IR addition: uprezed textures
static void _big_pretexture_horizontal_polygon_lines(struct polygon_definition *polygon,
	struct bitmap_definition *screen, struct view_data *view, struct _horizontal_polygon_line_data *data,
	short y0, short line_count);

static void _big_pretexture_vertical_polygon_lines(polygon_definition *polygon, bitmap_definition *texture,
	bitmap_definition *screen, view_data *view, _vertical_polygon_data *data,
	short x0, short line_count);

// IR addition: newvis
static bool _clip_horizontal_polygon_lines(_horizontal_polygon_line_data *src, _horizontal_polygon_line_data *dst,
	short *y0, short *height, rasterize_window *window);
static bool _clip_vertical_polygon_lines(_vertical_polygon_data *src, _vertical_polygon_data *dst, rasterize_window *window);


static short *build_x_table(short *table, int stride, short x0, short y0, short x1, short y1);
static short *build_y_table(short *table, int stride, short x0, short y0, short x1, short y1);

static void _prelandscape_horizontal_polygon_lines(struct polygon_definition *polygon,
	struct bitmap_definition *screen, struct view_data *view, struct _horizontal_polygon_line_data *data,
	short y0, short line_count);
// IR removed: moved the protytpes for texturers into texturers.h
/* ---------- code */


// LP addition:
// Find the next lower power of 2, and return the exponent
inline int NextLowerExponent(int n)
{
	int p = n;
	int xp = 0;
	while(p > 1) {p >>= 1; xp++;}
	return xp;
}


/* set aside memory at launch for two line tables (remember, we precalculate all the y-values
	for trapezoids and two lines worth of x-values for polygons before mapping them) */
void allocate_texture_tables(
	void)
{
//	scratch_table0= new short[MAXIMUM_SCRATCH_TABLE_ENTRIES];
//	scratch_table1= new short[MAXIMUM_SCRATCH_TABLE_ENTRIES];
	precalculation_table= malloc(MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE*MAXIMUM_SCRATCH_TABLE_ENTRIES);
<<<<<<< scottish_textures.cpp
	memset(precalculation_table, 0, MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE*MAXIMUM_SCRATCH_TABLE_ENTRIES);
	clip_table= malloc(MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE*MAXIMUM_SCRATCH_TABLE_ENTRIES);
	assert(/*scratch_table0&&scratch_table1&&*/precalculation_table);

	return;
=======
	assert(scratch_table0&&scratch_table1&&precalculation_table);
>>>>>>> 1.16
}

void Rasterizer_SW_Class::texture_horizontal_polygon(polygon_definition& textured_polygon, const rasterize_area_spec& windows)
{
	polygon_definition *polygon = &textured_polygon;	// Reference to pointer
	short vertex, highest_vertex, lowest_vertex;
	point2d *vertices= polygon->vertices;

	assert(polygon->vertex_count>=MINIMUM_VERTICES_PER_SCREEN_POLYGON&&polygon->vertex_count<MAXIMUM_VERTICES_PER_SCREEN_POLYGON);

	gUsageTimers.renderCalc.Start();
	/* if we get static, tinted or landscaped transfer modes punt to the vertical polygon mapper */
	switch (polygon->transfer_mode)
	{
//		case _solid_transfer:
//			if (bit_depth!=8) polygon->transfer_mode= _textured_transfer;
//			break;
		case _static_transfer:
//		case _landscaped_transfer:
			texture_vertical_polygon(textured_polygon, windows);
			// texture_vertical_polygon(polygon, screen, view);
			return;
	}
	
	/* locate the vertically highest (closest to zero) and lowest (farthest from zero) vertices */
	highest_vertex= lowest_vertex= 0;
	for (vertex= 1; vertex<polygon->vertex_count; ++vertex)
	{
		if (vertices[vertex].y<vertices[highest_vertex].y) highest_vertex= vertex;
		if (vertices[vertex].y>vertices[lowest_vertex].y) lowest_vertex= vertex;
	}

	for (vertex=0;vertex<polygon->vertex_count;++vertex)
	{
		if (!(vertices[vertex].x>=0&&vertices[vertex].x<=screen->width&&vertices[vertex].y>=0&&vertices[vertex].y<=screen->height))
		{
//			dprintf("vertex #%d/#%d out of bounds:;dm %x %x;g;", vertex, polygon->vertex_count, polygon->vertices, polygon->vertex_count*sizeof(point2d));
			return;
		}
	}

	/* if this polygon is not a horizontal line, draw it */
	if (highest_vertex==lowest_vertex) return;

	short left_line_count, right_line_count, total_line_count;
	short aggregate_left_line_count, aggregate_right_line_count, aggregate_total_line_count;
	short left_vertex, right_vertex;
	_horizontal_polygon_line_data * lines = (struct _horizontal_polygon_line_data *)precalculation_table;
//	short *left_table= &first_line->x0, *right_table= &first_line->x1;

	left_line_count= right_line_count= 0; /* zero counts so the left and right lines get initialized */
	aggregate_left_line_count= aggregate_right_line_count= 0; /* we’ve precalculated nothing initially */
	left_vertex= right_vertex= highest_vertex; /* both sides start at the highest vertex */
	total_line_count= vertices[lowest_vertex].y-vertices[highest_vertex].y; /* calculate vertical line count */

	assert(total_line_count<MAXIMUM_SCRATCH_TABLE_ENTRIES); /* make sure we have enough scratch space */
	
	/* precalculate high and low y-coordinates for every x-coordinate */			
	aggregate_total_line_count= total_line_count;
	while (total_line_count>0)
	{
		short delta;
		
		/* if we’re out of scan lines on the left side, get a new vertex and build a table
			of x-coordinates so we can walk toward the new vertex */
		if (left_line_count<=0)
		{
			do /* counter-clockwise vertex search */
			{
				vertex= left_vertex ? (left_vertex-1) : (polygon->vertex_count-1);
				left_line_count= vertices[vertex].y-vertices[left_vertex].y;
				if (!build_x_table(&lines[aggregate_left_line_count].x0, sizeof(_horizontal_polygon_line_data),
						vertices[left_vertex].x, vertices[left_vertex].y, vertices[vertex].x, vertices[vertex].y))
					return;
				aggregate_left_line_count+= left_line_count;
				left_vertex= vertex;
//					dprintf("add %d left", left_line_count);
			}
			while (!left_line_count);
		}

		/* if we’re out of scan lines on the right side, get a new vertex and build a table
			of x-coordinates so we can walk toward the new vertex */
		if (right_line_count<=0)
		{
			do /* clockwise vertex search */
			{
				vertex= (right_vertex==polygon->vertex_count-1) ? 0 : (right_vertex+1);
				right_line_count= vertices[vertex].y-vertices[right_vertex].y;
				if (!build_x_table(&lines[aggregate_right_line_count].x1, sizeof(_horizontal_polygon_line_data),
						vertices[right_vertex].x, vertices[right_vertex].y, vertices[vertex].x, vertices[vertex].y))
					return;
				aggregate_right_line_count+= right_line_count;
				right_vertex= vertex;
//					dprintf("add %d right", right_line_count);
			}
			while (!right_line_count);
		}

		/* advance by the minimum of left_line_count and right_line_count */
		delta= MIN(left_line_count, right_line_count);
		assert(delta);
//			dprintf("tc=%d lc=%d rc=%d delta=%d", total_line_count, left_line_count, right_line_count, delta);
		total_line_count-= delta;
		left_line_count-= delta;
		right_line_count-= delta;
		
		assert(delta||!total_line_count); /* if our delta is zero, we’d better be out of lines */
	}
	
	/* make sure every coordinate is accounted for in our tables */
	assert(aggregate_right_line_count==aggregate_total_line_count);
	assert(aggregate_left_line_count==aggregate_total_line_count);
	
	bitmap_definition *texture;

	if (polygon->texture->flags && _MAYBE_TRANSPARENT_BIT)
		CheckTransparency(polygon->texture);
	
	texture = polygon->texture;
	
	/* precalculate mode-specific data */
	switch (polygon->transfer_mode)
	{
		case _textured_transfer:
//			case _solid_transfer:
			texture = upscaledTextures.GetUpscale(texture, true);
			if (texture->width == 256)
				_big_pretexture_horizontal_polygon_lines(polygon, screen, view, (struct _horizontal_polygon_line_data *)precalculation_table,
					vertices[highest_vertex].y, aggregate_total_line_count);
			else
				_pretexture_horizontal_polygon_lines(polygon, screen, view, (struct _horizontal_polygon_line_data *)precalculation_table,
					vertices[highest_vertex].y, aggregate_total_line_count);
			break;

		case _big_landscaped_transfer:
			_prelandscape_horizontal_polygon_lines(polygon, screen, view, (struct _horizontal_polygon_line_data *)precalculation_table,
				vertices[highest_vertex].y, aggregate_total_line_count);
			break;
		
		default:
			vhalt(csprintf(temporary, "horizontal_polygons dont support mode #%d", polygon->transfer_mode));
	}
	
	horizontal_texturer *texturer;
	long extra = 0;
	
	gUsageTimers.renderCalc.Stop();
	
<<<<<<< scottish_textures.cpp
	/* render all lines */
	// IR tweak: changed global
	switch (g_render_depth)
	{
		case 8:
			switch (polygon->transfer_mode)
			{

				case _textured_transfer:
					if (texture->width == 256)
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_big_transparent_texture_horizontal_polygon_lines8:
							_big_texture_horizontal_polygon_lines8;
					else
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_transparent_texture_horizontal_polygon_lines8:
							_texture_horizontal_polygon_lines8;
					break;
					
				case _big_landscaped_transfer:
					texturer = _landscape_horizontal_polygon_lines8;
					break;
					
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;
=======
					case _textured_transfer:
						_texture_horizontal_polygon_lines8(polygon->texture, screen, view, (struct _horizontal_polygon_line_data *)precalculation_table,
							vertices[highest_vertex].y, left_table, right_table, aggregate_total_line_count);
						break;
					case _big_landscaped_transfer:
						_landscape_horizontal_polygon_lines8(polygon->texture, screen, view, (struct _horizontal_polygon_line_data *)precalculation_table,
							vertices[highest_vertex].y, left_table, right_table, aggregate_total_line_count);
						break;
						
					default:
						assert(false);
						break;
				}
				break;
>>>>>>> 1.16

		case 16:
			switch (polygon->transfer_mode)
			{
				case _textured_transfer:
					// IR change: try and get the alpha map.  If we get one have some fun.
					uint8* alpha_table = polygon->VoidPresent?NULL:
								get_bitmap_alpha_table(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(polygon->ShapeDesc)), GET_DESCRIPTOR_SHAPE(polygon->ShapeDesc));
					if (alpha_table) {
						extra = (long)alpha_table;
						if (texture->width == 256)
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_big_transpucent_texture_horizontal_polygon_lines16:
								_big_translucent_texture_horizontal_polygon_lines16;
						else
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_transpucent_texture_horizontal_polygon_lines16:
								_translucent_texture_horizontal_polygon_lines16;
					} else {
						if (texture->width == 256)
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_big_transparent_texture_horizontal_polygon_lines16:
								_big_texture_horizontal_polygon_lines16;
						else
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_transparent_texture_horizontal_polygon_lines16:
								_texture_horizontal_polygon_lines16;
					}
					// what a mess...
					break;
					
<<<<<<< scottish_textures.cpp
				case _big_landscaped_transfer:
					texturer = _landscape_horizontal_polygon_lines16;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;
=======
					default:
						assert(false);
						break;
				}
				break;
>>>>>>> 1.16

		case 32:
			switch (polygon->transfer_mode)
			{
				case _textured_transfer:
					// IR change: try and get the alpha map.  If we get one have some fun.
					uint8* alpha_table = polygon->VoidPresent?NULL:
								get_bitmap_alpha_table(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(polygon->ShapeDesc)), GET_DESCRIPTOR_SHAPE(polygon->ShapeDesc));
					if (alpha_table) {
						extra = (long)alpha_table;
						if (texture->width == 256)
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_big_transpucent_texture_horizontal_polygon_lines32:
								_big_translucent_texture_horizontal_polygon_lines32;
						else
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_transpucent_texture_horizontal_polygon_lines32:
								_translucent_texture_horizontal_polygon_lines32;
					} else {
						if (texture->width == 256)
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_big_transparent_texture_horizontal_polygon_lines32:
								_big_texture_horizontal_polygon_lines32;
						else
							texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
								_transparent_texture_horizontal_polygon_lines32:
								            _texture_horizontal_polygon_lines32;
					}
					break;
					
<<<<<<< scottish_textures.cpp
				case _big_landscaped_transfer:
					texturer = _landscape_horizontal_polygon_lines32;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;

		default:
			// LP change:
			assert(false);
			// halt();
	}
	
//	texturer(texture, screen, (struct _horizontal_polygon_line_data *)precalculation_table, vertices[highest_vertex].y,
//		aggregate_total_line_count, extra);
=======
					default:
						assert(false);
						break;
				}
				break;
>>>>>>> 1.16

<<<<<<< scottish_textures.cpp
	gUsageTimers.renderClip.Start();
	for (int i=0 ; i<windows.window_count ; i++) {
		_horizontal_polygon_line_data *clipped_lines = (struct _horizontal_polygon_line_data *)clip_table;
		short y0 = vertices[highest_vertex].y;
		short height = aggregate_total_line_count;
		
		if (_clip_horizontal_polygon_lines(lines, clipped_lines, &y0, &height, windows.windows+i))
		{
			assert(y0 < view->screen_height);
			assert(height >= 0 && y0 + height <= view->screen_height);
			
			
			_horizontal_polygon_line_data *line= clipped_lines;
			
			for (int debugI=0 ; debugI < height  ; debugI++, line++) {
				assert( line->x0 >= 0 && line->x1 <= view->screen_width);
			}
			
			gUsageTimers.renderClip.Stop();
			gUsageTimers.renderDraw.Start();
			texturer(texture, screen, clipped_lines, y0, height, extra);
			gUsageTimers.renderDraw.Stop();
			gUsageTimers.renderClip.Start();
=======
			default:
				assert(false);
				break;
>>>>>>> 1.16
		}
	}
<<<<<<< scottish_textures.cpp
	gUsageTimers.renderClip.Stop();
	
	
	return;
=======
>>>>>>> 1.16
}

void Rasterizer_SW_Class::texture_vertical_polygon(polygon_definition& textured_polygon, const rasterize_area_spec& windows)
{
	polygon_definition *polygon = &textured_polygon;	// Reference to pointer
	short vertex, highest_vertex, lowest_vertex;
	point2d *vertices= polygon->vertices;

	assert(polygon->vertex_count>=MINIMUM_VERTICES_PER_SCREEN_POLYGON&&polygon->vertex_count<MAXIMUM_VERTICES_PER_SCREEN_POLYGON);

	switch (polygon->transfer_mode)
	{
//		case _landscaped_transfer:
//			preprocess_landscaped_polygon(polygon, view);
<<<<<<< scottish_textures.cpp
//			break;
		case _big_landscaped_transfer:
			texture_horizontal_polygon(textured_polygon, windows);
			// texture_horizontal_polygon(polygon, screen, view);
			return;
	}

	/* locate the horizontally highest (closest to zero) and lowest (farthest from zero) vertices */
	highest_vertex= lowest_vertex= 0;
	for (vertex=1;vertex<polygon->vertex_count;++vertex)
	{
		if (vertices[vertex].x<vertices[highest_vertex].x) highest_vertex= vertex;
		if (vertices[vertex].x>vertices[lowest_vertex].x) lowest_vertex= vertex;
	}

	for (vertex=0;vertex<polygon->vertex_count;++vertex)
	{
		if (!(vertices[vertex].x>=0&&vertices[vertex].x<=screen->width&&vertices[vertex].y>=0&&vertices[vertex].y<=screen->height))
		{
//			dprintf("vertex #%d/#%d out of bounds:;dm %x %x;g;", vertex, polygon->vertex_count, polygon->vertices, polygon->vertex_count*sizeof(point2d));
			return;
		}
	}

	/* if this polygon is not a vertical line, draw it */
	if (highest_vertex==lowest_vertex) return;

	short left_line_count, right_line_count, total_line_count;
	short aggregate_left_line_count, aggregate_right_line_count, aggregate_total_line_count;
	short left_vertex, right_vertex;
	_vertical_polygon_data *polygon_render_data = (struct _vertical_polygon_data *)precalculation_table;
	_vertical_polygon_line_data *render_lines = (_vertical_polygon_line_data*)(polygon_render_data+1);

	left_line_count= right_line_count= 0; /* zero counts so the left and right lines get initialized */
	aggregate_left_line_count= aggregate_right_line_count= 0; /* we’ve precalculated nothing initially */
	left_vertex= right_vertex= highest_vertex; /* both sides start at the highest vertex */
	total_line_count= vertices[lowest_vertex].x-vertices[highest_vertex].x; /* calculate vertical line count */

	assert(total_line_count<MAXIMUM_SCRATCH_TABLE_ENTRIES); /* make sure we have enough scratch space */
	
	gUsageTimers.renderCalc.Start();
	/* precalculate high and low y-coordinates for every x-coordinate */			
	aggregate_total_line_count= total_line_count;
	while (total_line_count>0)
	{
		short delta;
		
		/* if we’re out of scan lines on the left side, get a new vertex and build a table
			of y-coordinates so we can walk toward the new vertex */
		if (left_line_count<=0)
		{
			do /* clockwise vertex search */
			{
				vertex= (left_vertex==polygon->vertex_count-1) ? 0 : (left_vertex+1);
				left_line_count= vertices[vertex].x-vertices[left_vertex].x;
//					dprintf("left line (%d,%d) to (%d,%d) for %d points", vertices[left_vertex].x, vertices[left_vertex].y, vertices[vertex].x, vertices[vertex].y, left_line_count);
				if (!build_y_table(&render_lines[aggregate_left_line_count].y0, sizeof(_vertical_polygon_line_data),
							vertices[left_vertex].x, vertices[left_vertex].y, vertices[vertex].x, vertices[vertex].y))
					return;
				aggregate_left_line_count+= left_line_count;
				left_vertex= vertex;
			}
			while (!left_line_count);
		}

		/* if we’re out of scan lines on the right side, get a new vertex and build a table
			of y-coordinates so we can walk toward the new vertex */
		if (right_line_count<=0)
		{
			do /* counter-clockwise vertex search */
			{
				vertex= right_vertex ? (right_vertex-1) : (polygon->vertex_count-1);
				right_line_count= vertices[vertex].x-vertices[right_vertex].x;
//					dprintf("right line (%d,%d) to (%d,%d) for %d points", vertices[right_vertex].x, vertices[right_vertex].y, vertices[vertex].x, vertices[vertex].y, right_line_count);
				if (!build_y_table(&render_lines[aggregate_right_line_count].y1, sizeof(_vertical_polygon_line_data), 
							vertices[right_vertex].x, vertices[right_vertex].y, vertices[vertex].x, vertices[vertex].y))
					return;
				aggregate_right_line_count+= right_line_count;
				right_vertex= vertex;
			}
			while (!right_line_count);
		}
		
		/* advance by the minimum of left_line_count and right_line_count */
		delta= MIN(left_line_count, right_line_count);
		assert(delta);
		total_line_count-= delta;
		left_line_count-= delta;
		right_line_count-= delta;
		
		assert(delta||!total_line_count); /* if our delta is zero, we’d better be out of lines */
	}
	
	/* make sure every coordinate is accounted for in our tables */
	assert(aggregate_right_line_count==aggregate_total_line_count);
	assert(aggregate_left_line_count==aggregate_total_line_count);

	if (polygon->texture->flags && _MAYBE_TRANSPARENT_BIT)
		CheckTransparency(polygon->texture);
	
	/* precalculate mode-specific data */
	switch (polygon->transfer_mode)
	{
		case _textured_transfer:
//			case _landscaped_transfer:
		case _static_transfer:
			// IR change: try and get a hi-res texture.  If we get one have some fun.
			bitmap_definition *texture = upscaledTextures.GetUpscale(polygon->texture, true);
			if (texture->width == 256)
				_big_pretexture_vertical_polygon_lines(polygon, texture, screen, view, polygon_render_data,
					vertices[highest_vertex].x, aggregate_total_line_count);
			else
				_pretexture_vertical_polygon_lines(polygon, screen, view, polygon_render_data,
					vertices[highest_vertex].x, aggregate_total_line_count);
			break;
		
		default:
			vhalt(csprintf(temporary, "vertical_polygons dont support mode #%d", polygon->transfer_mode));
	}
	gUsageTimers.renderCalc.Stop();
	
	vertical_texturer *texturer;
	long extra = 0;
	uint8* alpha_table = NULL;
	if (!polygon->VoidPresent)
		alpha_table = get_bitmap_alpha_table(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(polygon->ShapeDesc)), GET_DESCRIPTOR_SHAPE(polygon->ShapeDesc));

	/* render all lines */
	// IR tweak: changed global
	switch (g_render_depth)
	{
		case 8:
			switch (polygon->transfer_mode)
			{
				case _textured_transfer:
//					case _landscaped_transfer:
					texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
						_transparent_texture_vertical_polygon_lines8 :
						_texture_vertical_polygon_lines8;
					break;
				
				case _static_transfer:
//						_randomize_vertical_polygon_lines8(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;
			
		case 16:
			switch (polygon->transfer_mode)
			{
				case _textured_transfer:
//					case _landscaped_transfer:
					if (alpha_table)
					{
						extra = (long)alpha_table;
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_transpucent_texture_vertical_polygon_lines16:
							_translucent_texture_vertical_polygon_lines16;
					} else
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_transparent_texture_vertical_polygon_lines16 :
							_texture_vertical_polygon_lines16;
					break;
				
				case _static_transfer:
//						_randomize_vertical_polygon_lines16(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
					break;

				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;
		
		case 32:
			switch (polygon->transfer_mode)
			{
				case _textured_transfer:
//					case _landscaped_transfer:
					// IR change: try and get the alpha map.  If we get one have some fun.
					if (alpha_table)
					{
						extra = (long)alpha_table;
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_transpucent_texture_vertical_polygon_lines32:
							_translucent_texture_vertical_polygon_lines32;
					} else
						texturer = (polygon->texture->flags&_TRANSPARENT_BIT && !polygon->VoidPresent) ?
							_transparent_texture_vertical_polygon_lines32:
							_texture_vertical_polygon_lines32;
					break;
				
				case _static_transfer:
//						_randomize_vertical_polygon_lines32(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
					break;

				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
//	DebugStr("\p;hc");
	
	gUsageTimers.renderClip.Start();
	for (int i=0 ; i<windows.window_count ; i++) {
		_vertical_polygon_data *clipped_data = (struct _vertical_polygon_data *)clip_table;
		if (_clip_vertical_polygon_lines(polygon_render_data, clipped_data, windows.windows+i))
		{
			_vertical_polygon_data *data = clipped_data;
			
			assert(data->x0 >= 0 && data->x0 < view->screen_width);
			assert(data->width >= 0 && data->x0 + data->width <= view->screen_width);
			
			
			_vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
			
			for (int debugI=0 ; debugI < data->width  ; debugI++, line++) {
				assert( line->y0 >= 0 && line->y1 <= view->screen_height);
			}
			gUsageTimers.renderClip.Stop();
			gUsageTimers.renderDraw.Start();
			texturer(screen, clipped_data, extra);
			gUsageTimers.renderDraw.Stop();
			gUsageTimers.renderClip.Start();
		}
	}
	gUsageTimers.renderClip.Stop();
//		texturer(screen, (struct _vertical_polygon_data *)precalculation_table, extra);
	return;
}

void Rasterizer_SW_Class::texture_rectangle(rectangle_definition& textured_rectangle, const rasterize_area_spec& windows)
{
	rectangle_definition *rectangle = &textured_rectangle;	// Reference to pointer
	(void) (view) /* we don’t need no steenkin’ view */;

	if (rectangle->x0 >= rectangle->x1 || rectangle->y0 >= rectangle->y1) return;
	
	/* subsume screen boundaries into clipping parameters */
	if (rectangle->clip_left<0) rectangle->clip_left= 0;
	if (rectangle->clip_right>screen->width) rectangle->clip_right= screen->width;
	if (rectangle->clip_top<0) rectangle->clip_top= 0;
	if (rectangle->clip_bottom>screen->height) rectangle->clip_bottom= screen->height;

	/* subsume left and right sides of the rectangle into clipping parameters */
	if (rectangle->clip_left<rectangle->x0) rectangle->clip_left= rectangle->x0;
	if (rectangle->clip_right>rectangle->x1) rectangle->clip_right= rectangle->x1;
	if (rectangle->clip_top<rectangle->y0) rectangle->clip_top= rectangle->y0;
	if (rectangle->clip_bottom>rectangle->y1) rectangle->clip_bottom= rectangle->y1;

	/* only continue if we have a non-empty rectangle, at least some of which is on the screen */
	if (! (rectangle->clip_left<rectangle->clip_right && rectangle->clip_top<rectangle->clip_bottom &&
		rectangle->clip_right>0 && rectangle->clip_left<screen->width &&
		rectangle->clip_bottom>0 && rectangle->clip_top<screen->height))
		return;
	
	short delta; /* scratch */
	short screen_width= rectangle->x1-rectangle->x0;
	short screen_height= rectangle->y1-rectangle->y0;
	short screen_x= rectangle->x0, screen_y= rectangle->y0;
	struct bitmap_definition *texture= rectangle->texture;
	void *shading_table;

	struct _vertical_polygon_data *header= (struct _vertical_polygon_data *)precalculation_table;
	struct _vertical_polygon_line_data *data= (struct _vertical_polygon_line_data *) (header+1);
	
	_fixed texture_dx= INTEGER_TO_FIXED(texture->width)/screen_width;
	_fixed texture_x= texture_dx>>1;

	_fixed texture_dy= INTEGER_TO_FIXED(texture->height)/screen_height;
	_fixed texture_y0= 0;
	_fixed texture_y1;
	
	if (!(texture_dx&&texture_dy)) return;
	
	/* handle horizontal mirroring */
	if (rectangle->flip_horizontal)
	{
		texture_dx= -texture_dx;
		texture_x= INTEGER_TO_FIXED(texture->width)+(texture_dx>>1);
	}
	
	/* left clipping */		
	if ((delta= rectangle->clip_left-rectangle->x0)>0)
	{
		texture_x+= delta*texture_dx;
		screen_width-= delta;
		screen_x= rectangle->clip_left;
	}				
	/* right clipping */
	if ((delta= rectangle->x1-rectangle->clip_right)>0)
	{
		screen_width-= delta;
	}
	
	/* top clipping */
	if ((delta= rectangle->clip_top-rectangle->y0)>0)
	{
		texture_y0+= delta*texture_dy;
		screen_height-= delta;
	}
	
	/* bottom clipping */
	if ((delta= rectangle->y1-rectangle->clip_bottom)>0)
	{
		screen_height-= delta;
	}

	texture_y1= texture_y0 + screen_height*texture_dy;
	
	header->downshift= FIXED_FRACTIONAL_BITS;
	header->width= screen_width;
	header->x0= screen_x;
	
	if (rectangle->texture->flags && _MAYBE_TRANSPARENT_BIT)
		CheckTransparency(rectangle->texture);

	/* calculate shading table, once */
	switch (rectangle->transfer_mode)
	{
		case _textured_transfer:
			if (!(rectangle->flags&_SHADELESS_BIT))
			{
				// LP change:
				// Made this more long-distance friendly
				CALCULATE_SHADING_TABLE(shading_table, view, rectangle->shading_tables, (short)MIN(rectangle->depth, SHRT_MAX), rectangle->ambient_shade);
				// CALCULATE_SHADING_TABLE(shading_table, view, rectangle->shading_tables, rectangle->depth, rectangle->ambient_shade);
				break;
			}
			/* if shadeless, fall through to a single shading table, ignoring depth */
		case _tinted_transfer:
		case _static_transfer:
			shading_table= rectangle->shading_tables;
			break;
		
		default:
			vhalt(csprintf(temporary, "rectangles dont support mode #%d", rectangle->transfer_mode));
	}

	for (; screen_width; --screen_width)
	{
		byte *read= texture->row_addresses[FIXED_INTEGERAL_PART(texture_x)];
		// CB: first/last are stored in big-endian order
		uint16 first = *read++ << 8;
		first |= *read++;
//					short first= *(int16*)read; read = (int16 *)read + 1;
		uint16 last = *read++ << 8;
		last |= *read++;
//					short last= *(int16*)read; read = (int16 *)read + 1;
		_fixed texture_y= texture_y0;
		short y0= rectangle->clip_top, y1= rectangle->clip_bottom;
		
		if (FIXED_INTEGERAL_PART(texture_y0)<first)
		{
			delta= (INTEGER_TO_FIXED(first) - texture_y0)/texture_dy + 1;
			// IR change: with closer clipping its better just to bail.
			if (delta<0) return;
			vassert(delta>=0, csprintf(temporary, "[%x,%x] ∂=%x (#%d,#%d)", texture_y0, texture_y1, texture_dy, first, last));
			
			y0= MIN(y1, y0+delta);
			texture_y+= delta*texture_dy;
		}
		
		if (FIXED_INTEGERAL_PART(texture_y1)>last)
		{
			delta= (texture_y1 - INTEGER_TO_FIXED(last))/texture_dy + 1;
			// IR change: with closer clipping its better just to bail.
			if (delta<0) return;
			vassert(delta>=0, csprintf(temporary, "[%x,%x] ∂=%x (#%d,#%d)", texture_y0, texture_y1, texture_dy, first, last));
			
			y1= MAX(y0, y1-delta);
		}
		
		data->texture_y= texture_y - INTEGER_TO_FIXED(first);
		data->texture_dy= texture_dy;
		data->shading_table= shading_table;
		data->texture= (unsigned char *)read;
		
		data->y0 = y0;
		data->y1 = y1;
		
		texture_x+= texture_dx;
		data+= 1;
		
		assert(y0<=y1);
		assert(y0>=0 && y1>=0);
		assert(y0<=screen->height);
		assert(y1<=screen->height);
	}
	
	vertical_texturer *texturer;
	long extra = 0;
	uint8* alpha_table = get_bitmap_alpha_table(	GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(textured_rectangle.ShapeDesc)),
													GET_DESCRIPTOR_SHAPE(textured_rectangle.ShapeDesc));

	// IR tweak: changed global
	switch (g_render_depth)
	{
		case 8:
			switch (rectangle->transfer_mode)
			{
				case _textured_transfer:
					texturer = _transparent_texture_vertical_polygon_lines8;
					break;
				
				case _static_transfer:
					extra = rectangle->transfer_data;
					texturer = _randomize_vertical_polygon_lines8;
					break;
				
				case _tinted_transfer:
					extra = rectangle->transfer_data;
					texturer = _tint_vertical_polygon_lines8;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;

		case 16:
			switch (rectangle->transfer_mode)
			{
				case _textured_transfer:
					// IR change: try and get the alpha map.  If we get one have some fun.
					if (alpha_table) {
						extra = (long) alpha_table;
						texturer = _transpucent_texture_vertical_polygon_lines16;
					} else
						texturer = _transparent_texture_vertical_polygon_lines16;
					break;
				
				case _static_transfer:
					extra = rectangle->transfer_data;
					texturer = _randomize_vertical_polygon_lines16;
					break;
				
				case _tinted_transfer:
					extra = rectangle->transfer_data;
					texturer = _tint_vertical_polygon_lines16;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;

		case 32:
			switch (rectangle->transfer_mode)
			{
				case _textured_transfer:
					// IR change: try and get the alpha map.  If we get one have some fun.
					if (alpha_table) {
						extra = (long) alpha_table;
						texturer = _transpucent_texture_vertical_polygon_lines32;
					} else
						texturer = _transparent_texture_vertical_polygon_lines32;
					break;
				
				case _static_transfer:
					extra = rectangle->transfer_data;
					texturer = _randomize_vertical_polygon_lines32;
					break;
				
				case _tinted_transfer:
					extra = rectangle->transfer_data;
					texturer = _tint_vertical_polygon_lines32;
					break;
				
				default:
					// LP change:
					assert(false);
					// halt();
			}
			break;

		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	texturer(screen, (struct _vertical_polygon_data *)precalculation_table, extra);
	
	return;
}

unsigned long gDebugLineLut[] = {
0x01FF0000, 0x0400FF00, 0x030000FF,
0x04FFFF00, 0x0500FFFF, 0x06FF00FF,
0x00FFFFFF};

void Rasterizer_SW_Class::debug_line_v(int x, int y0, int y1) {
	if (x<0 || x>= screen->width) return;
	if (y0 >= screen->height || y1 < 0) return;
	y0 = MAX(y0,0);
	y1 = MIN(y1,screen->height);
	unsigned long rowbytes = screen->bytes_per_row, pixel;
	switch (g_render_depth) {
	case 8: {
		pixel8 *baseAddr = (pixel8*)screen->row_addresses[y0];
		baseAddr += x;
		for (int i=y1-y0 ; i>0 ; i--) {
			*baseAddr = 0;
			baseAddr = (pixel8*)(((char*)baseAddr)+rowbytes);
		}
	} break;
	case 16: {
		pixel16 *baseAddr = (pixel16*)screen->row_addresses[y0];
		baseAddr += x;
		for (int i=y1-y0 ; i>0 ; i--) {
			pixel = *baseAddr;
			pixel >>= 1;
			pixel &= 0x3DEF;
			pixel += 0x3DEF;
			*baseAddr = pixel;
			baseAddr = (pixel16*)(((char*)baseAddr)+rowbytes);
		}
	} break;
	case 32: {
		pixel32 *baseAddr = (pixel32*)screen->row_addresses[y0];
		baseAddr += x;
		for (int i=y1-y0 ; i>0 ; i--) {
			pixel = *baseAddr;
			pixel >>= 24;
			pixel = gDebugLineLut[pixel&0xFF];
			*baseAddr = pixel;
			baseAddr = (pixel32*)(((char*)baseAddr)+rowbytes);
		}
	} break;
	}
}

void Rasterizer_SW_Class::debug_line_h(int y, int x0, int x1) {
	if (y<0 || y>= screen->height) return;
	if (x0 >= screen->width || x1 < 0) return;
	x0 = MAX(x0,0);
	x1 = MIN(x1,screen->width);
	unsigned long pixel;
	switch (g_render_depth) {
	case 8: {
		pixel8 *baseAddr = (pixel8*)screen->row_addresses[y];
		baseAddr += x0;
		for (int i=x1-x0 ; i>0 ; i--) {
			*baseAddr = 0;
			baseAddr++;
		}
	} break;
	case 16: {
		pixel16 *baseAddr = (pixel16*)screen->row_addresses[y];
		baseAddr += x0;
		for (int i=x1-x0 ; i>0 ; i--) {
			pixel = *baseAddr;
			pixel >>= 1;
			pixel &= 0x3DEF;
			pixel += 0x3DEF;
			*baseAddr = pixel;
			baseAddr++;
		}
	} break;
	case 32: {
		pixel32 *baseAddr = (pixel32*)screen->row_addresses[y];
		baseAddr += x0;
		for (int i=x1-x0 ; i>0 ; i--) {
			pixel = *baseAddr;
			pixel >>= 24;
			pixel = gDebugLineLut[pixel&0xFF];
			*baseAddr = pixel;
			baseAddr++;
		}
	} break;
	}
	
}

// IR change: texturer generation moved to textuers.cpp

/* ---------- private code */

#if 0

#define LANDSCAPE_REPEATS 12
static void preprocess_landscaped_polygon(
	struct polygon_definition *polygon,
	struct view_data *view)
{
	polygon->origin.x= (world_distance) ((10000*LANDSCAPE_REPEATS*WORLD_ONE)/(2*31415));
	polygon->origin.y= -(((LANDSCAPE_REPEATS*WORLD_ONE*view->yaw)>>ANGULAR_BITS)&(WORLD_ONE-1));
	polygon->origin.z= 0;
	
	polygon->vector.i= 0;
	polygon->vector.j= WORLD_ONE;
	polygon->vector.k= -WORLD_ONE;

	polygon->ambient_shade= FIXED_ONE;
	
	return;
}

#endif

/* starting at x0 and for line_count vertical lines between *y0 and *y1, precalculate all the
	information _texture_vertical_polygon_lines will need to work */
static void _pretexture_vertical_polygon_lines(
	struct polygon_definition *polygon,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short x0,
	short line_count)
{
	short screen_x= x0-view->half_screen_width;
	int32 dz0= view->world_to_screen_y*polygon->origin.z;
	int32 unadjusted_ty_denominator= view->world_to_screen_y*polygon->vector.k;
	int32 tx_numerator, tx_denominator, tx_numerator_delta, tx_denominator_delta;
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);

	(void) (screen);

	assert(sizeof(struct _vertical_polygon_line_data)<=MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE);

	data->downshift= VERTICAL_TEXTURE_DOWNSHIFT;
	data->x0= x0;
	data->width= line_count;

	/* calculate and rescale tx_numerator, tx_denominator, etc. */
	tx_numerator= view->world_to_screen_x*polygon->origin.y - screen_x*polygon->origin.x;
	tx_denominator= screen_x*polygon->vector.i - view->world_to_screen_x*polygon->vector.j;
	tx_numerator_delta= -polygon->origin.x;
	tx_denominator_delta= polygon->vector.i;

	while (--line_count>=0)
	{
		_fixed tx;
		// LP change: made this quantity more long-distance friendly;
		// have to avoid doing INTEGER_TO_FIXED on this one, however
		int32 world_x;
		// world_distance world_x;
		short x0, y0= line->y0, y1= line->y1;
		short screen_y0= view->half_screen_height-y0+view->dtanpitch;
		int32 ty_numerator, ty_denominator;
		_fixed ty, ty_delta;

		/* would our precision be greater here if we shifted the numerator up to $7FFFFFFF and
			then downshifted only the numerator?  too bad we can’t use BFFFO in 68k */
		{
			int32 adjusted_tx_denominator= tx_denominator;
			int32 adjusted_tx_numerator= tx_numerator;
			
			while (adjusted_tx_numerator>((1<<(31-VERTICAL_TEXTURE_WIDTH_BITS))-1) ||
				adjusted_tx_numerator<((-1)<<(31-VERTICAL_TEXTURE_WIDTH_BITS)))
			{
				adjusted_tx_numerator>>= 1, adjusted_tx_denominator>>= 1;
			}
			if (!adjusted_tx_denominator) adjusted_tx_denominator= 1; /* -1 will still be -1 */
			x0= ((adjusted_tx_numerator<<VERTICAL_TEXTURE_WIDTH_BITS)/adjusted_tx_denominator)&(VERTICAL_TEXTURE_WIDTH-1);

			while (adjusted_tx_numerator>INT16_MAX||adjusted_tx_numerator<INT16_MIN)
			{
				adjusted_tx_numerator>>= 1, adjusted_tx_denominator>>= 1;
			}
			if (!adjusted_tx_denominator) adjusted_tx_denominator= 1; /* -1 will still be -1 */
			tx= INTEGER_TO_FIXED(adjusted_tx_numerator)/adjusted_tx_denominator;
		}
		
		world_x= polygon->origin.x + ((tx*polygon->vector.i)>>FIXED_FRACTIONAL_BITS);
		if (world_x<0) world_x= -world_x; /* it is mostly unclear what we’re supposed to do with negative x values */

		/* calculate and rescale ty_numerator, ty_denominator and calculate ty */
		ty_numerator= world_x*screen_y0 - dz0;
		ty_denominator= unadjusted_ty_denominator;
		while (ty_numerator>INT16_MAX||ty_numerator<INT16_MIN)
		{
			ty_numerator>>= 1, ty_denominator>>= 1;
		}
		if (!ty_denominator) ty_denominator= 1; /* -1 will still be -1 */
		ty= INTEGER_TO_FIXED(ty_numerator)/ty_denominator;
		
		// LP change:
		// Use the same reduction hack used earlier,
		// because otherwise, INTEGER_TO_FIXED would cause world_x to wrap around.
		int32 adjusted_world_x = world_x;
		int32 adjusted_ty_denominator = unadjusted_ty_denominator>>8;
		
		// LP: remember that world_x is always >= 0
		while(adjusted_world_x > INT16_MAX)
		{
			adjusted_world_x >>= 1; adjusted_ty_denominator >>= 1;
		}
		ty_delta= - INTEGER_TO_FIXED(adjusted_world_x)/adjusted_ty_denominator;
		
		// ty_delta= - INTEGER_TO_FIXED(world_x)/(unadjusted_ty_denominator>>8);
		vassert(ty_delta>=0, csprintf(temporary, "ty_delta=W2F(%d)/%d=%d", world_x, unadjusted_ty_denominator, ty_delta));

		/* calculate the shading table for this column */
		if (polygon->flags&_SHADELESS_BIT)
		{
			line->shading_table= polygon->shading_tables;
		}
		else
		{
			// LP change: made this more long-distance friendly
			CALCULATE_SHADING_TABLE(line->shading_table, view, polygon->shading_tables, (short)MIN(world_x, SHRT_MAX), polygon->ambient_shade);
			// CALCULATE_SHADING_TABLE(line->shading_table, view, polygon->shading_tables, world_x, polygon->ambient_shade);
		}

//		if (ty_delta)
		{
			/* calculate texture_y and texture_dy (floor-mapper style) */
//			data->n= VERTICAL_TEXTURE_DOWNSHIFT;
			line->texture_y= ty<<VERTICAL_TEXTURE_FREE_BITS;
			line->texture_dy= ty_delta<<(VERTICAL_TEXTURE_FREE_BITS-8);
			line->texture= polygon->texture->row_addresses[x0];
			
			line+= 1;
		}
		
		tx_numerator+= tx_numerator_delta;
		tx_denominator+= tx_denominator_delta;
		
		screen_x+= 1;
=======
//			break;
		case _big_landscaped_transfer:
			texture_horizontal_polygon(textured_polygon);
			return;
>>>>>>> 1.2
	}
	
	return;
}

static void _pretexture_horizontal_polygon_lines(
	struct polygon_definition *polygon,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short line_count)
{
	int32 hcosine, dhcosine;
	int32 hsine, dhsine;
	int32 hworld_to_screen;
	bool higher_precision= polygon->origin.z>-WORLD_ONE && polygon->origin.z<WORLD_ONE;

	(void) (screen);
	
	/* precalculate a bunch of multiplies */
	hcosine= cosine_table[view->yaw];
	hsine= sine_table[view->yaw];
	if (higher_precision)
	{
		hcosine*= polygon->origin.z;
		hsine*= polygon->origin.z;
	}
	hworld_to_screen= polygon->origin.z*view->world_to_screen_y;
	dhcosine= view->world_to_screen_y*hcosine;
	dhsine= view->world_to_screen_y*hsine;

	while ((line_count-=1)>=0)
	{
		// LP change: made this more long-distance-friendly
		int32 depth;
		// world_distance depth;
		short screen_x, screen_y;
		short x0= data->x0, x1= data->x1;
		
		/* calculate screen_x,screen_y */
		screen_x= x0-view->half_screen_width;
		screen_y= view->half_screen_height-y0+view->dtanpitch;
		if (!screen_y) screen_y= 1; /* this will avoid division by zero and won't change rendering */
		
		/* calculate source_x, source_y, source_dx, source_dy */
//#ifdef env68k
//		if (polygon->transfer_mode!=_solid_transfer)
//#endif
		{
			int32 source_x, source_y, source_dx, source_dy;
			
			/* calculate texture origins and deltas (source_x,source_dx,source_y,source_dy) */
			if (higher_precision)
			{
				source_x= (dhcosine - screen_x*hsine)/screen_y + (polygon->origin.x<<TRIG_SHIFT);
				source_dx= - hsine/screen_y;
				source_y= (screen_x*hcosine + dhsine)/screen_y + (polygon->origin.y<<TRIG_SHIFT);
				source_dy= hcosine/screen_y;
			}
			else
			{
				source_x= ((dhcosine - screen_x*hsine)/screen_y)*polygon->origin.z + (polygon->origin.x<<TRIG_SHIFT);
				source_dx= - (hsine*polygon->origin.z)/screen_y;
				source_y= ((screen_x*hcosine + dhsine)/screen_y)*polygon->origin.z + (polygon->origin.y<<TRIG_SHIFT);
				source_dy= (hcosine*polygon->origin.z)/screen_y;
			}
		
			/* voodoo so x,y texture wrapping is handled automatically by downshifting
				(subtract one from HORIZONTAL_FREE_BITS to double scale) */
			data->source_x= source_x<<HORIZONTAL_FREE_BITS, data->source_dx= source_dx<<HORIZONTAL_FREE_BITS;
			data->source_y= source_y<<HORIZONTAL_FREE_BITS, data->source_dy= source_dy<<HORIZONTAL_FREE_BITS;
		}

		/* get shading table (with absolute value of depth) */
		if ((depth= hworld_to_screen/screen_y)<0) depth= -depth;
		if (polygon->flags&_SHADELESS_BIT)
		{
			data->shading_table= polygon->shading_tables;
		}
		else
		{
<<<<<<< scottish_textures.cpp
			CALCULATE_SHADING_TABLE(data->shading_table, view, polygon->shading_tables, (short)MIN(depth, SHRT_MAX), polygon->ambient_shade);
=======
			case 8:
				switch (polygon->transfer_mode)
				{
					case _textured_transfer:
//					case _landscaped_transfer:
						(polygon->texture->flags&_TRANSPARENT_BIT) ?
							_transparent_texture_vertical_polygon_lines8(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table) :
							_texture_vertical_polygon_lines8(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table);
						break;
					
					case _static_transfer:
//						_randomize_vertical_polygon_lines8(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
						break;
					
					default:
						assert(false);
						break;
				}
				break;
				
			case 16:
				switch (polygon->transfer_mode)
				{
					case _textured_transfer:
//					case _landscaped_transfer:
						(polygon->texture->flags&_TRANSPARENT_BIT) ?
							_transparent_texture_vertical_polygon_lines16(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table) :
							_texture_vertical_polygon_lines16(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table);
						break;
					
					case _static_transfer:
//						_randomize_vertical_polygon_lines16(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
						break;

					default:
						assert(false);
						break;
				}
				break;
			
			case 32:
				switch (polygon->transfer_mode)
				{
					case _textured_transfer:
//					case _landscaped_transfer:
						(polygon->texture->flags&_TRANSPARENT_BIT) ?
							_transparent_texture_vertical_polygon_lines32(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table) :
							_texture_vertical_polygon_lines32(screen, view, (struct _vertical_polygon_data *)precalculation_table, left_table, right_table);
						break;
					
					case _static_transfer:
//						_randomize_vertical_polygon_lines32(screen, view, precalculation_table,
//							left_table, right_table, polygon->transfer_data);
						break;

					default:
						assert(false);
						break;
				}
				break;
			
			default:
				assert(false);
				break;
>>>>>>> 1.16
		}
<<<<<<< scottish_textures.cpp
=======
	}
}

void Rasterizer_SW_Class::texture_rectangle(rectangle_definition& textured_rectangle)
{
	rectangle_definition *rectangle = &textured_rectangle;	// Reference to pointer
	(void) (view) /* we don’t need no steenkin’ view */;

	if (rectangle->x0<rectangle->x1 && rectangle->y0<rectangle->y1)
	{
		/* subsume screen boundaries into clipping parameters */
		if (rectangle->clip_left<0) rectangle->clip_left= 0;
		if (rectangle->clip_right>screen->width) rectangle->clip_right= screen->width;
		if (rectangle->clip_top<0) rectangle->clip_top= 0;
		if (rectangle->clip_bottom>screen->height) rectangle->clip_bottom= screen->height;
	
		/* subsume left and right sides of the rectangle into clipping parameters */
		if (rectangle->clip_left<rectangle->x0) rectangle->clip_left= rectangle->x0;
		if (rectangle->clip_right>rectangle->x1) rectangle->clip_right= rectangle->x1;
		if (rectangle->clip_top<rectangle->y0) rectangle->clip_top= rectangle->y0;
		if (rectangle->clip_bottom>rectangle->y1) rectangle->clip_bottom= rectangle->y1;
	
		/* only continue if we have a non-empty rectangle, at least some of which is on the screen */
		if (rectangle->clip_left<rectangle->clip_right && rectangle->clip_top<rectangle->clip_bottom &&
			rectangle->clip_right>0 && rectangle->clip_left<screen->width &&
			rectangle->clip_bottom>0 && rectangle->clip_top<screen->height)
		{
			short delta; /* scratch */
			short screen_width= rectangle->x1-rectangle->x0;
			short screen_height= rectangle->y1-rectangle->y0;
			short screen_x= rectangle->x0;
			struct bitmap_definition *texture= rectangle->texture;
	
			short *y0_table= scratch_table0, *y1_table= scratch_table1;
			struct _vertical_polygon_data *header= (struct _vertical_polygon_data *)precalculation_table;
			struct _vertical_polygon_line_data *data= (struct _vertical_polygon_line_data *) (header+1);
			
			_fixed texture_dx= INTEGER_TO_FIXED(texture->width)/screen_width;
			_fixed texture_x= texture_dx>>1;
	
			_fixed texture_dy= INTEGER_TO_FIXED(texture->height)/screen_height;
			_fixed texture_y0= 0;
			_fixed texture_y1;
			
			if (texture_dx&&texture_dy)
			{
				/* handle horizontal mirroring */
				if (rectangle->flip_horizontal)
				{
					texture_dx= -texture_dx;
					texture_x= INTEGER_TO_FIXED(texture->width)+(texture_dx>>1);
				}
				
				/* left clipping */		
				if ((delta= rectangle->clip_left-rectangle->x0)>0)
				{
					texture_x+= delta*texture_dx;
					screen_width-= delta;
					screen_x= rectangle->clip_left;
				}				
				/* right clipping */
				if ((delta= rectangle->x1-rectangle->clip_right)>0)
				{
					screen_width-= delta;
				}
				
				/* top clipping */
				if ((delta= rectangle->clip_top-rectangle->y0)>0)
				{
					texture_y0+= delta*texture_dy;
					screen_height-= delta;
				}
				
				/* bottom clipping */
				if ((delta= rectangle->y1-rectangle->clip_bottom)>0)
				{
					screen_height-= delta;
				}
	
				texture_y1= texture_y0 + screen_height*texture_dy;
				
				header->downshift= FIXED_FRACTIONAL_BITS;
				header->width= screen_width;
				header->x0= screen_x;
				
				/* calculate shading table, once */
				void *shading_table = NULL;
				switch (rectangle->transfer_mode)
				{
					case _textured_transfer:
						if (!(rectangle->flags&_SHADELESS_BIT))
						{
							// LP change:
							// Made this more long-distance friendly
							CALCULATE_SHADING_TABLE(shading_table, view, rectangle->shading_tables, (short)MIN(rectangle->depth, SHRT_MAX), rectangle->ambient_shade);
							break;
						}
						/* if shadeless, fall through to a single shading table, ignoring depth */
					case _tinted_transfer:
					case _static_transfer:
						shading_table= rectangle->shading_tables;
						break;
					
					default:
						vhalt(csprintf(temporary, "rectangles dont support mode #%d", rectangle->transfer_mode));
				}
>>>>>>> 1.16
		
<<<<<<< scottish_textures.cpp
		data+= 1;
		y0+= 1;
=======
				for (; screen_width; --screen_width)
				{
					byte *read= texture->row_addresses[FIXED_INTEGERAL_PART(texture_x)];
					// CB: first/last are stored in big-endian order
					uint16 first = *read++ << 8;
					first |= *read++;
					uint16 last = *read++ << 8;
					last |= *read++;
					_fixed texture_y= texture_y0;
					short y0= rectangle->clip_top, y1= rectangle->clip_bottom;
					
					if (FIXED_INTEGERAL_PART(texture_y0)<first)
					{
						delta= (INTEGER_TO_FIXED(first) - texture_y0)/texture_dy + 1;
						vassert(delta>=0, csprintf(temporary, "[%x,%x] ∂=%x (#%d,#%d)", texture_y0, texture_y1, texture_dy, first, last));
						
						y0= MIN(y1, y0+delta);
						texture_y+= delta*texture_dy;
					}
					
					if (FIXED_INTEGERAL_PART(texture_y1)>last)
					{
						delta= (texture_y1 - INTEGER_TO_FIXED(last))/texture_dy + 1;
						vassert(delta>=0, csprintf(temporary, "[%x,%x] ∂=%x (#%d,#%d)", texture_y0, texture_y1, texture_dy, first, last));
						
						y1= MAX(y0, y1-delta);
					}
					
					data->texture_y= texture_y - INTEGER_TO_FIXED(first);
					data->texture_dy= texture_dy;
					data->shading_table= shading_table;
					data->texture= (unsigned char *)read;
					
					texture_x+= texture_dx;
					data+= 1;
					
					*y0_table++= y0;
					*y1_table++= y1;
					
					assert(y0<=y1);
					assert(y0>=0 && y1>=0);
					assert(y0<=screen->height);
					assert(y1<=screen->height);
				}
		
				switch (bit_depth)
				{
					case 8:
						switch (rectangle->transfer_mode)
						{
							case _textured_transfer:
								_transparent_texture_vertical_polygon_lines8(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1);
								break;
							
							case _static_transfer:
								_randomize_vertical_polygon_lines8(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							case _tinted_transfer:
								_tint_vertical_polygon_lines8(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							default:
								assert(false);
								break;
						}
						break;
		
					case 16:
						switch (rectangle->transfer_mode)
						{
							case _textured_transfer:
								_transparent_texture_vertical_polygon_lines16(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1);
								break;
							
							case _static_transfer:
								_randomize_vertical_polygon_lines16(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							case _tinted_transfer:
								_tint_vertical_polygon_lines16(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							default:
								assert(false);
								break;
						}
						break;
		
					case 32:
						switch (rectangle->transfer_mode)
						{
							case _textured_transfer:
								_transparent_texture_vertical_polygon_lines32(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1);
								break;
							
							case _static_transfer:
								_randomize_vertical_polygon_lines32(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							case _tinted_transfer:
								_tint_vertical_polygon_lines32(screen, view, (struct _vertical_polygon_data *)precalculation_table,
									scratch_table0, scratch_table1, rectangle->transfer_data);
								break;
							
							default:
								assert(false);
								break;
						}
						break;
		
					default:
						assert(false);
						break;
				}
			}
		}
>>>>>>> 1.16
	}
<<<<<<< scottish_textures.cpp
	
	return;
=======
}

/* generate code for 8-bit texture-mapping */
#define BIT_DEPTH 8
#include "low_level_textures.h"
#undef BIT_DEPTH

/* generate code for 16-bit texture-mapping */
#define BIT_DEPTH 16
#include "low_level_textures.h"
#undef BIT_DEPTH

/* generate code for 32-bit texture-mapping */
#define BIT_DEPTH 32
#include "low_level_textures.h"
#undef BIT_DEPTH

/* ---------- private code */

#if 0

#define LANDSCAPE_REPEATS 12
static void preprocess_landscaped_polygon(
	struct polygon_definition *polygon,
	struct view_data *view)
{
	polygon->origin.x= (world_distance) ((10000*LANDSCAPE_REPEATS*WORLD_ONE)/(2*31415));
	polygon->origin.y= -(((LANDSCAPE_REPEATS*WORLD_ONE*view->yaw)>>ANGULAR_BITS)&(WORLD_ONE-1));
	polygon->origin.z= 0;
	
	polygon->vector.i= 0;
	polygon->vector.j= WORLD_ONE;
	polygon->vector.k= -WORLD_ONE;

	polygon->ambient_shade= FIXED_ONE;
>>>>>>> 1.16
}


/* starting at x0 and for line_count vertical lines between *y0 and *y1, precalculate all the
	information _texture_vertical_polygon_lines will need to work */
static void _big_pretexture_vertical_polygon_lines(
	polygon_definition *polygon,
	bitmap_definition *texture, 
	bitmap_definition *screen,
	view_data *view,
	_vertical_polygon_data *data,
	short x0,
	short line_count)
{
	short screen_x= x0-view->half_screen_width;
	int32 dz0= view->world_to_screen_y*polygon->origin.z;
	int32 unadjusted_ty_denominator= view->world_to_screen_y*polygon->vector.k;
	int32 tx_numerator, tx_denominator, tx_numerator_delta, tx_denominator_delta;
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);

	(void) (screen);

	assert(sizeof(struct _vertical_polygon_line_data)<=MAXIMUM_PRECALCULATION_TABLE_ENTRY_SIZE);

	data->downshift= BIG_VERTICAL_TEXTURE_DOWNSHIFT;
	data->x0= x0;
	data->width= line_count;

	/* calculate and rescale tx_numerator, tx_denominator, etc. */
	tx_numerator= view->world_to_screen_x*polygon->origin.y - screen_x*polygon->origin.x;
	tx_denominator= screen_x*polygon->vector.i - view->world_to_screen_x*polygon->vector.j;
	tx_numerator_delta= -polygon->origin.x;
	tx_denominator_delta= polygon->vector.i;

	while (--line_count>=0)
	{
		_fixed tx;
		// LP change: made this quantity more long-distance friendly;
		// have to avoid doing INTEGER_TO_FIXED on this one, however
		int32 world_x;
<<<<<<< scottish_textures.cpp
<<<<<<< scottish_textures.cpp
		// world_distance world_x;
		short x0, y0= line->y0, y1= line->y1;
=======
		short x0, y0= *y0_table++, y1= *y1_table++;
>>>>>>> 1.16
=======
		short x0, y0= *y0_table++;
>>>>>>> 1.2
		short screen_y0= view->half_screen_height-y0+view->dtanpitch;
		int32 ty_numerator, ty_denominator;
		_fixed ty, ty_delta;

		/* would our precision be greater here if we shifted the numerator up to $7FFFFFFF and
			then downshifted only the numerator?  too bad we can’t use BFFFO in 68k */
		{
			int32 adjusted_tx_denominator= tx_denominator;
			int32 adjusted_tx_numerator= tx_numerator;
			
			while (adjusted_tx_numerator>((1<<(31-BIG_VERTICAL_TEXTURE_WIDTH_BITS))-1) ||
				adjusted_tx_numerator<((-1)<<(31-BIG_VERTICAL_TEXTURE_WIDTH_BITS)))
			{
				adjusted_tx_numerator>>= 1, adjusted_tx_denominator>>= 1;
			}
			if (!adjusted_tx_denominator) adjusted_tx_denominator= 1; /* -1 will still be -1 */
			x0= ((adjusted_tx_numerator<<BIG_VERTICAL_TEXTURE_WIDTH_BITS)/adjusted_tx_denominator)&(BIG_VERTICAL_TEXTURE_WIDTH-1);

			while (adjusted_tx_numerator>INT16_MAX||adjusted_tx_numerator<INT16_MIN)
			{
				adjusted_tx_numerator>>= 1, adjusted_tx_denominator>>= 1;
			}
			if (!adjusted_tx_denominator) adjusted_tx_denominator= 1; /* -1 will still be -1 */
			tx= INTEGER_TO_FIXED(adjusted_tx_numerator)/adjusted_tx_denominator;
		}
		
		world_x= polygon->origin.x + ((tx*polygon->vector.i)>>FIXED_FRACTIONAL_BITS);
		if (world_x<0) world_x= -world_x; /* it is mostly unclear what we’re supposed to do with negative x values */

		/* calculate and rescale ty_numerator, ty_denominator and calculate ty */
		ty_numerator= world_x*screen_y0 - dz0;
		ty_denominator= unadjusted_ty_denominator;
		while (ty_numerator>INT16_MAX||ty_numerator<INT16_MIN)
		{
			ty_numerator>>= 1, ty_denominator>>= 1;
		}
		if (!ty_denominator) ty_denominator= 1; /* -1 will still be -1 */
		ty= INTEGER_TO_FIXED(ty_numerator)/ty_denominator;
		
		// LP change:
		// Use the same reduction hack used earlier,
		// because otherwise, INTEGER_TO_FIXED would cause world_x to wrap around.
		int32 adjusted_world_x = world_x;
		int32 adjusted_ty_denominator = unadjusted_ty_denominator>>8;
		
		// LP: remember that world_x is always >= 0
		while(adjusted_world_x > INT16_MAX)
		{
			adjusted_world_x >>= 1; adjusted_ty_denominator >>= 1;
		}
		ty_delta= - INTEGER_TO_FIXED(adjusted_world_x)/adjusted_ty_denominator;
		
		vassert(ty_delta>=0, csprintf(temporary, "ty_delta=W2F(%d)/%d=%d", world_x, unadjusted_ty_denominator, ty_delta));

		/* calculate the shading table for this column */
		if (polygon->flags&_SHADELESS_BIT)
		{
			line->shading_table= polygon->shading_tables;
		}
		else
		{
			// LP change: made this more long-distance friendly
			CALCULATE_SHADING_TABLE(line->shading_table, view, polygon->shading_tables, (short)MIN(world_x, SHRT_MAX), polygon->ambient_shade);
			// CALCULATE_SHADING_TABLE(line->shading_table, view, polygon->shading_tables, world_x, polygon->ambient_shade);
		}

//		if (ty_delta)
		{
			/* calculate texture_y and texture_dy (floor-mapper style) */
//			data->n= VERTICAL_TEXTURE_DOWNSHIFT;
			line->texture_y= ty<<BIG_VERTICAL_TEXTURE_FREE_BITS;
			line->texture_dy= ty_delta<<(BIG_VERTICAL_TEXTURE_FREE_BITS-8);
			//if (x0 < 128)
				line->texture= texture->row_addresses[x0];
			//else
			//	line->texture= texture->row_addresses[0];
			
			line+= 1;
		}
		
		tx_numerator+= tx_numerator_delta;
		tx_denominator+= tx_denominator_delta;
		
		screen_x+= 1;
	}
}

static bool _clip_horizontal_polygon_lines(
	_horizontal_polygon_line_data *src,
	_horizontal_polygon_line_data *dst,
	short *y0p,
	short *heightp,
	rasterize_window *window)
{
	*dst = *src;
	
	short y0 = *y0p;
	short height = *heightp;
	
	int offset;
	
	if ((offset = window->y0 - y0) > 0)
	{
		y0 += offset;
		src += offset;
		height -= offset;
		if (height <= 0) return false;
	}
	
	
	if (y0 + height > window->y1)
	{
		height = window->y1 - y0;
		if (height <= 0) return false;
	}
	
	int min = window->x0, max = window->x1;
	
	for (int i=0 ; i<height ; i++) {
	//	*dstline = *srcline;
		dst->shading_table = src->shading_table;
		
		int x0 = src->x0;
		if (x0 < min) {
			dst->x0 = min;
			int dx = src->source_dx;
			dst->source_x = src->source_x + dx*(min-x0);
			dst->source_dx = dx;
			int dy = src->source_dy;
			dst->source_y = src->source_y + dy*(min-x0);
			dst->source_dy = dy;
		} else {
			dst->x0 = x0;
			dst->source_x = src->source_x;
			dst->source_dx = src->source_dx;
			dst->source_y = src->source_y;
			dst->source_dy = src->source_dy;
		}
		
		int x1 = src->x1;
		if (x1 > max) {
			dst->x1 = max;
		} else {
			dst->x1 = x1;
		}
		dst++; src++;
	}
	
	*y0p = y0;
	*heightp = height;
	
	return true;
}

static bool _clip_vertical_polygon_lines(
	_vertical_polygon_data *src,
	_vertical_polygon_data *dst, 
	rasterize_window *window)
{
	*dst = *src;
	
	struct _vertical_polygon_line_data *srcline= (struct _vertical_polygon_line_data *) (src+1);
	
	int offset;
	
	if (window->x0 - dst->x0 > 0)
	{
		offset = window->x0 - dst->x0;
		dst->x0 += offset;
		srcline += offset;
		dst->width -= offset;
		if (dst->width <= 0) return false;
	}
	
	
	if (dst->x0 + dst->width > window->x1)
	{
		offset = window->x1 - (dst->x0 + dst->width);
		dst->width = window->x1 - dst->x0;
		if (dst->width <= 0) return false;
	}
	
	
	
	struct _vertical_polygon_line_data *dstline= (struct _vertical_polygon_line_data *) (dst+1);
	
	int min = window->y0, max = window->y1;
	
	for (int i=0 ; i<dst->width ; i++) {
	//	*dstline = *srcline;
		dstline->texture = srcline->texture;
		dstline->shading_table = srcline->shading_table;
		
		int y0 = srcline->y0;
		if (y0 < min) {
			dstline->y0 = min;
			int dy = srcline->texture_dy;
			dstline->texture_y = srcline->texture_y + dy*(min-y0);
			dstline->texture_dy = dy;
		} else {
			dstline->y0 = y0;
			dstline->texture_y = srcline->texture_y;
			dstline->texture_dy = srcline->texture_dy;
		}
		
		int y1 = srcline->y1;
		if (y1 > max) {
			dstline->y1 = max;
		} else {
			dstline->y1 = y1;
		}
		dstline++; srcline++;
	}
	
	return true;
}


static void _big_pretexture_horizontal_polygon_lines(
	struct polygon_definition *polygon,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short line_count)
{
	int32 hcosine, dhcosine;
	int32 hsine, dhsine;
	int32 hworld_to_screen;
	bool higher_precision= polygon->origin.z>-WORLD_ONE && polygon->origin.z<WORLD_ONE;

	(void) (screen);
	
	/* precalculate a bunch of multiplies */
	hcosine= cosine_table[view->yaw];
	hsine= sine_table[view->yaw];
	if (higher_precision)
	{
		hcosine*= polygon->origin.z;
		hsine*= polygon->origin.z;
	}
	hworld_to_screen= polygon->origin.z*view->world_to_screen_y;
	dhcosine= view->world_to_screen_y*hcosine;
	dhsine= view->world_to_screen_y*hsine;

	while ((line_count-=1)>=0)
	{
		// LP change: made this more long-distance-friendly
		int32 depth;
		// world_distance depth;
		short screen_x, screen_y;
<<<<<<< scottish_textures.cpp
		short x0= data->x0, x1= data->x1;
=======
		short x0= *x0_table++;
>>>>>>> 1.2
		
		/* calculate screen_x,screen_y */
		screen_x= x0-view->half_screen_width;
		screen_y= view->half_screen_height-y0+view->dtanpitch;
		if (!screen_y) screen_y= 1; /* this will avoid division by zero and won't change rendering */
		
		/* calculate source_x, source_y, source_dx, source_dy */
//#ifdef env68k
//		if (polygon->transfer_mode!=_solid_transfer)
//#endif
		{
			int32 source_x, source_y, source_dx, source_dy;
			
			/* calculate texture origins and deltas (source_x,source_dx,source_y,source_dy) */
			if (higher_precision)
			{
				source_x= (dhcosine - screen_x*hsine)/screen_y + (polygon->origin.x<<TRIG_SHIFT);
				source_dx= - hsine/screen_y;
				source_y= (screen_x*hcosine + dhsine)/screen_y + (polygon->origin.y<<TRIG_SHIFT);
				source_dy= hcosine/screen_y;
			}
			else
			{
				source_x= ((dhcosine - screen_x*hsine)/screen_y)*polygon->origin.z + (polygon->origin.x<<TRIG_SHIFT);
				source_dx= - (hsine*polygon->origin.z)/screen_y;
				source_y= ((screen_x*hcosine + dhsine)/screen_y)*polygon->origin.z + (polygon->origin.y<<TRIG_SHIFT);
				source_dy= (hcosine*polygon->origin.z)/screen_y;
			}
		
			/* voodoo so x,y texture wrapping is handled automatically by downshifting
				(subtract one from HORIZONTAL_FREE_BITS to double scale) */
			data->source_x= source_x<<HORIZONTAL_FREE_BITS, data->source_dx= source_dx<<HORIZONTAL_FREE_BITS;
			data->source_y= source_y<<HORIZONTAL_FREE_BITS, data->source_dy= source_dy<<HORIZONTAL_FREE_BITS;
		}

		/* get shading table (with absolute value of depth) */
		if ((depth= hworld_to_screen/screen_y)<0) depth= -depth;
		if (polygon->flags&_SHADELESS_BIT)
		{
			data->shading_table= polygon->shading_tables;
		}
		else
		{
			CALCULATE_SHADING_TABLE(data->shading_table, view, polygon->shading_tables, (short)MIN(depth, SHRT_MAX), polygon->ambient_shade);
		}
		
		data+= 1;
		y0+= 1;
	}
}


// height must be determined emperically (texture is vertically centered at 0°)
// #define LANDSCAPE_REPEAT_BITS 1
static void _prelandscape_horizontal_polygon_lines(
	struct polygon_definition *polygon,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short line_count)
{
	// LP change: made this more general:
	short landscape_width_bits= NextLowerExponent(polygon->texture->height);
	short texture_height= polygon->texture->width;
	_fixed ambient_shade= FIXED_ONE; // MPW C died if we passed the constant directly to the macro

	// Get the landscape-texturing options
	LandscapeOptions *LandOpts = View_GetLandscapeOptions(polygon->ShapeDesc);
	
	// LP change: separate horizontal and vertical pixel deltas:
	// LP change: using a "landscape yaw" that's at the left edge of the screen.
	_fixed first_horizontal_pixel= (view->landscape_yaw + LandOpts->Azimuth)<<(landscape_width_bits+(LandOpts->HorizExp)+FIXED_FRACTIONAL_BITS-ANGULAR_BITS);
	_fixed horizontal_pixel_delta= (view->half_cone<<(1+landscape_width_bits+(LandOpts->HorizExp)+FIXED_FRACTIONAL_BITS-ANGULAR_BITS))/view->standard_screen_width;
	_fixed vertical_pixel_delta= (view->half_cone<<(1+landscape_width_bits+(LandOpts->VertExp)+FIXED_FRACTIONAL_BITS-ANGULAR_BITS))/view->standard_screen_width;
	short landscape_free_bits= 32-FIXED_FRACTIONAL_BITS-landscape_width_bits;
	
	(void) (screen);

	/* calculate the shading table */	
	void *shading_table = NULL;
	if (polygon->flags&_SHADELESS_BIT)
	{
		shading_table= polygon->shading_tables;
	}
	else
	{
		CALCULATE_SHADING_TABLE(shading_table, view, polygon->shading_tables, 0, ambient_shade);
	}
	
	// Find the height to repeat over; use value used for OpenGL texture setup
	short texture_width= polygon->texture->height;
	short repeat_texture_height = texture_width >> LandOpts->OGL_AspRatExp;
	
	short height_reduced = texture_height - 1;
	short height_shift = texture_height >> 1;
	short height_repeat_mask = repeat_texture_height - 1;
	short height_repeat_shift = repeat_texture_height >> 1;
	
	y0-= view->half_screen_height + view->dtanpitch; /* back to virtual screen coordinates */
	while ((line_count-= 1)>=0)
	{
<<<<<<< scottish_textures.cpp
		short x0= data->x0, x1= data->x1;
=======
		short x0= *x0_table++;
>>>>>>> 1.2
		
		data->shading_table= shading_table;
		// LP change: using vertical pixel delta
		// Also using vertical repeat if selected;
		// fold the height into the range (-repeat_height/2, repeat_height)
		short y_txtr_offset= FIXED_INTEGERAL_PART(y0*vertical_pixel_delta);
		if (LandOpts->VertRepeat)
			y_txtr_offset = ((y_txtr_offset + height_repeat_shift) & height_repeat_mask) -
				height_repeat_shift;
		data->source_y= texture_height - PIN(y_txtr_offset + height_shift, 0, height_reduced) - 1;
<<<<<<< scottish_textures.cpp
		data->source_dy= 0;
		// data->source_y= FIXED_INTEGERAL_PART(y0*pixel_delta) + (texture_height>>1);
		// data->source_y= texture_height - PIN(data->source_y, 0, texture_height-1) - 1;
=======
>>>>>>> 1.16
		// LP change: using horizontal pixel delta
		data->source_x= (first_horizontal_pixel + x0*horizontal_pixel_delta)<<landscape_free_bits;
		data->source_dx= horizontal_pixel_delta<<landscape_free_bits;
		
		data+= 1;
		y0+= 1;
	}
}

/* y0<y1; this is for vertical polygons */
static short *build_x_table(
	short *table, int stride,
	short x0,
	short y0,
	short x1,
	short y1)
{
	short dx, dy, adx, ady; /* 'a' prefix means absolute value */
	short x, y; /* x,y screen positions */
	short d, delta_d, d_max; /* descriminator, delta_descriminator, descriminator_maximum */
	short *record;

	/* calculate SGN(dx),SGN(dy) and the absolute values of dx,dy */	
	dx= x1-x0, adx= ABS(dx), dx= SGN(dx);
	dy= y1-y0, ady= ABS(dy), dy= SGN(dy);

	assert(ady<MAXIMUM_SCRATCH_TABLE_ENTRIES); /* can't overflow table */
	if (dy>0)
	{
		/* setup initial (x,y) location and initialize a pointer to our table */
		x= x0, y= y0;
		record= table;
	
		if (adx>=ady)
		{
			/* x-dominant line (we need to record x every time y changes) */
	
			d= adx-ady, delta_d= - 2*ady, d_max= 2*adx;
			while ((adx-=1)>=0)
			{
				if (d<0) { // note to those responsible (you know who you are):
				           // writing five lines of code on a single line does not make it faster.
					y+= 1;
					d+= d_max;
					*record= x;
					record = (short*)(((char*)record) + stride);
					ady-= 1;
				}
				x+= dx, d+= delta_d;
			}
			if (ady==1) *record= x, record = (short*)(((char*)record) + stride);
			else assert(!ady);
		}
		else
		{
			/* y-dominant line (we need to record x every iteration) */
	
			d= ady-adx;
			delta_d= - 2*adx;
			d_max= 2*ady;
			while ((ady-=1)>=0)
			{
				if (d<0) {
					x+= dx;
					d+= d_max;
				}
				*record= x;
				record = (short*)(((char*)record) + stride);
				y+= 1;
				d+= delta_d;
			}
		}
	}
	else
	{
		/* can’t build a table for negative dy */
		if (dy<0) table= (short *) NULL;
	}
	
	return table;
}

/* x0<x1; this is for horizontal polygons */
static short *build_y_table(
	short *table, int stride,
	short x0,
	short y0,
	short x1,
	short y1)
{
	short dx, dy, adx, ady; /* 'a' prefix means absolute value */
	short x, y; /* x,y screen positions */
	short d, delta_d, d_max; /* descriminator, delta_descriminator, descriminator_maximum */
	short *record;

	/* calculate SGN(dx),SGN(dy) and the absolute values of dx,dy */	
	dx= x1-x0, adx= ABS(dx), dx= SGN(dx);
	dy= y1-y0, ady= ABS(dy), dy= SGN(dy);

	assert(adx<MAXIMUM_SCRATCH_TABLE_ENTRIES); /* can't overflow table */
	if (dx>=0) /* vertical lines allowed */
	{
		/* setup initial (x,y) location and initialize a pointer to our table */
		if (dy>=0)
		{
			x= x0, y= y0;
			record= table;
		}
		else
		{
			x= x1, y= y1;
			record= (short*)(((char*)table) + adx*stride);;
		}
	
		if (adx>=ady)
		{
			/* x-dominant line (we need to record y every iteration) */
	
			d= adx-ady, delta_d= - 2*ady, d_max= 2*adx;
			while ((adx-=1)>=0)
			{
				if (d<0) {
					y+= 1;
					d+= d_max;
				}
				
				if (dy>=0) {
					*record= y;
					record = (short*)(((char*)record) + stride);
				} else {
					record = (short*)(((char*)record) - stride);
					*record= y;
				}
				x+= dx;
				d+= delta_d;
			}
		}
		else
		{
			/* y-dominant line (we need to record y every time x changes) */
	
			d= ady-adx, delta_d= - 2*adx, d_max= 2*ady;
			while ((ady-=1)>=0)
			{
				if (d<0) {
					x+= dx;
					d+= d_max;
					adx-= 1;
					if (dy>=0) {
						*record= y;
						record = (short*)(((char*)record) + stride);
					} else {
						record = (short*)(((char*)record) - stride);
						*record= y;
					}
				}
				y+= 1, d+= delta_d;
			}
			if (adx==1) {
				if (dy>=0) {
					*record= y;
					record = (short*)(((char*)record) + stride);
				} else {
					record = (short*)(((char*)record) - stride);
					*record= y;
				}
			} else {
				assert(!adx);
			}
		}
	}
	else
	{
		/* can’t build a table for a negative dx */
		table= (short *) NULL;
	}
	
	return table;
}
<<<<<<< scottish_textures.cpp

#ifdef OBSOLETE /* unnecessary, but do not delete */
/* clip_low<clip_high in destination units; this function builds a table of source coordinates
	for each unclipped destination coordinate (the landscape mapper and the object mapper
	will both use this to build y-coordinate lookup tables) */
static short *build_distortion_table(
	short *table,
	short source,
	short destination,
	short clip_low,
	short clip_high,
	bool mirror)
{
	register short i;
	short count, source_coordinate;
	register fixed d, delta_d, d_max;
	register short *record;
	
	assert(clip_low<=clip_high);
	if (clip_low<0) clip_low= 0;
	if (clip_high>destination) clip_high= destination;
	
	/* initialize d, delta_d, d_max */
	d= 2*destination - source;
	delta_d= 2*source;
	d_max= 2*destination;
	
	/* if unclipped, we'll generation translation entries for every destination pixel */
	count= destination;
	
	/* if unclipped we'll start at source==0 */
	source_coordinate= 0;
	
	/* even marginally clever editing of d_max and delta_d with the addition of a new
		source_coordinate_delta variable would allow us to change the while (d<=0) loop
		into an if (d<=0) below */

	if (mirror)
	{
		short low_clipped_pixels= clip_low, high_clipped_pixels= destination-clip_high;
		clip_low= high_clipped_pixels, clip_high= destination-low_clipped_pixels;
	}

	/* handle high clipping by adjusting count */
	if (clip_high<destination) count-= destination-clip_high;
	
	/* handle low clipping by adjusting d, count, source_coordinate */
	if (clip_low>0)
	{
		d-= clip_low*delta_d;
		if (d<=0)
		{
			short n= 1 - d/d_max;
		
			d+= n*d_max;
			source_coordinate+= n;
		}

		count-= clip_low;
	}

	for (i=count,record=mirror?table+count:table;i>0;--i)
	{
		while (d<=0) source_coordinate+= 1, d+= d_max;
		if (mirror) *--record= source_coordinate; else *record++= source_coordinate;
		d-= delta_d;
	}

	if (count)
	{
		if (table[0]<0 || table[0]>=source || table[count-1]<0 || table[count-1]>=source)
		{
			dprintf("%d==>%d [%d/%d==>%d] at %p", source, destination, clip_low, clip_high, count, table);
		}
	}
	
//	if (destination<20) dprintf("table %d-->%d (cliped to %d);dm #%d #%d;", source, destination, fuck, table, fuck*sizeof(short));
	
	return table;
}
#endif

#ifdef OBSOLETE /* replaced by macro */
/* a negative ambient_shade is interpreted as an absolute shade, and is not modified by depth
	shading */
void *calculate_shading_table(
	struct view_data *view,
	void *shading_tables,
	world_distance depth,
	fixed ambient_shade)
{
	short table_index;
	int32 shading_table_size;
	
	switch (bit_depth)
	{
		case 8: shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel8); break;
		case 16: shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel16); break;
		default:
			// LP change:
			assert(false);
			// halt();
	}

#ifdef env68k
	/* eventually the shading function will depend on something in the view_data structure
		(and this will replace the DEPTH_TO_SHADE macro) */
	(void) (view);
#endif

	if (ambient_shade<0)
	{
		assert(ambient_shade>=-FIXED_ONE);
		table_index= SHADE_TO_SHADING_TABLE_INDEX(-ambient_shade);
	}
	else
	{
		/* calculate shading index due to viewer’s depth, adjust it according to FIRST_ and
			LAST_SHADING_TABLE, then take whichever of this index or the index due to the
			ambient shade is higher */
		table_index= SHADE_TO_SHADING_TABLE_INDEX(DEPTH_TO_SHADE(depth));
		table_index= LAST_SHADING_TABLE-PIN(table_index, FIRST_SHADING_TABLE, LAST_SHADING_TABLE);
		table_index= MAX(SHADE_TO_SHADING_TABLE_INDEX(ambient_shade), table_index);
	}

	/* calculate and return exact shading table */
	return ((byte*)shading_tables) + shading_table_size*CEILING(table_index, number_of_shading_tables-1);
}
#endif

#if OBSOLETE_CLIPPING
		/* fake clipping */
		{
			short i;

			high_point= vertices[highest_vertex].y;
			
			for (i=0;i<aggregate_total_line_count;++i)
			{
				if (left_table[i]<0) left_table[i]= 0;
				if (right_table[i]>screen->width) right_table[i]= screen->width;
				if (left_table[i]<polygon->clip_left) left_table[i]= polygon->clip_left;
				if (right_table[i]>polygon->clip_right) right_table[i]= polygon->clip_right-1;
			}

			if (vertices[highest_vertex].y<0)
			{
				aggregate_total_line_count+= vertices[highest_vertex].y;
				left_table-= vertices[highest_vertex].y;
				right_table-= vertices[highest_vertex].y;
				
				high_point= 0; /* ow */
			}
			
			if (vertices[lowest_vertex].y>screen->height)
			{
				aggregate_total_line_count-= vertices[lowest_vertex].y-screen->height;
			}

			if (aggregate_total_line_count<=0) return;
		}
#endif

static void CheckTransparency(bitmap_definition *bitmap) {
	if (! (bitmap->flags&_MAYBE_TRANSPARENT_BIT)) return;
	
	bitmap->flags &= ~_MAYBE_TRANSPARENT_BIT;
	
	if (bitmap->bytes_per_row == NONE)
	{
		return;
	}
	
	int rows= (bitmap->flags&_COLUMN_ORDER_BIT) ? bitmap->width : bitmap->height;
	int columns= (bitmap->flags&_COLUMN_ORDER_BIT) ? bitmap->height : bitmap->width;

	for (int y=0 ; y<rows ; y++) {
		pixel8 *row = bitmap->row_addresses[y];
		for (int x=0 ; x<columns ; x++)
			if (row[x]==0) {
				bitmap->flags |= _TRANSPARENT_BIT;
				return;
			}
	}
	bitmap->flags &= ~_TRANSPARENT_BIT;
}


















=======
>>>>>>> 1.16
