#ifndef __SCOTTISH_TEXTURES_H
#define __SCOTTISH_TEXTURES_H

/*
SCOTTISH_TEXTURES.H
Thursday, April 28, 1994 4:54:54 PM

Feb 17, 2000 (Loren Petrich):
	Made the polygons' origin positions and sprite rectangles' depths
	better adapted to long distances

Mar 12, 2000 (Loren Petrich):
	Added shape descriptors to the wall-texture render objects
	for the convenience of OpenGL rendering;
	also added object indices to the sprite.
	Weapons in hand have a special index.
*/

#include "shape_descriptors.h"

#define WEAPON_OBJECT_INDEX -32768

/* ---------- constants */

/* maximum width and height of screen for allocating tables, etc. */
#define MAXIMUM_SCREEN_WIDTH 640
#define MAXIMUM_SCREEN_HEIGHT 480

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

#ifdef OBSOLETE
/* number of shading tables and the size of each (in bytes) */
#define SHADING_TABLE_INDEX_FRACTIONAL_BITS (bit_depth==8?5:6)
#define NUMBER_OF_SHADING_TABLES (bit_depth==8?32:64)
#define SHADING_TABLE_SIZE PIXEL8_MAXIMUM_COLORS
#define COMPLETE_SHADING_TABLE_SIZE (SHADING_TABLE_SIZE*NUMBER_OF_SHADING_TABLES)
#define PELSIZE (bit_depth==8?sizeof(pixel8):sizeof(pixel16))
#endif

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

/* ignore multiple shading tables if set */
#define _SHADELESS_BIT 0x8000

struct rectangle_definition
{
	word flags;
	
	struct bitmap_definition *texture;
	
	/* screen coordinates; x0<x1, y0<y1 */
	short x0, y0;
	short x1, y1;

	/* screen coordinates */
	short clip_left, clip_right;
	short clip_top, clip_bottom;

	/* depth at logical center (used to calculate light due to viewer) */
	// LP change: made this long-distance friendly
	long depth;
	// world_distance depth;
	
	/* ambient shading table index; many objects will be self-luminescent, so this may have
		nothing to do with the polygon the object is sitting in */
	fixed ambient_shade;
	
	/* here are all the shading tables, crammed together in memory */	
	void *shading_tables;

	/* _tinted, _textured and _static are supported; _solid would be silly and _landscape
		would be hard (but might be cool) */
	short transfer_mode, transfer_data;
	
	/* mirrored horizontally and vertically if TRUE */
	boolean flip_vertical, flip_horizontal;
	
	// LP addition: shape-descriptor value for the convenience of OpenGL;
	// the lower byte is the frame
	// Note that for the convenience of 3D-model rendering, more shape information may
	// eventually have to be transmitted.
	shape_descriptor ShapeDesc;
};

struct polygon_definition
{
	word flags;
	
	struct bitmap_definition *texture; /* ignored for _tinted and _solid polygons */

	/* ambient shading table index */
	fixed ambient_shade;

	/* here are all the shading tables, crammed together in memory (unless this is a tinted
		polygon in which case it is a single 256-byte tinting table) */
	void *shading_tables;	

	/* all modes legal */	
	short transfer_mode, transfer_data;
	
	// LP change: made this long-distance friendly
	long_point3d origin;
	// world_point3d origin;
	world_vector3d vector; /* used only for vertically textured polygons */

	/* clockwise vertices for this convex polygon */
	short vertex_count;
	point2d vertices[MAXIMUM_VERTICES_PER_SCREEN_POLYGON];
	
	// LP addition: shape-descriptor value for the convenience of OpenGL;
	// the lower byte is the frame
	shape_descriptor ShapeDesc;
};

/* ---------- globals */

extern short bit_depth;
extern short interface_bit_depth;

extern short number_of_shading_tables, shading_table_fractional_bits, shading_table_size;

/* ---------- prototypes/SCOTTISH_TEXTURES.C */

void allocate_texture_tables(void);

void texture_rectangle(struct rectangle_definition *rectangle, struct bitmap_definition *screen, struct view_data *view);
void texture_horizontal_polygon(struct polygon_definition *polygon, struct bitmap_definition *screen, struct view_data *view);
void texture_vertical_polygon(struct polygon_definition *polygon, struct bitmap_definition *screen, struct view_data *view);

/* ---------- prototypes/SPHERE.C */

void allocate_spherical_decision_table(void);
void precalculate_spherical_decision_table(short texture_width, short texture_height, short r);
void texture_sphere(fixed phase, struct bitmap_definition *texture, struct bitmap_definition *destination, void *shading_tables);

#endif
