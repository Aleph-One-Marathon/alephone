#ifndef __TEXTURERS_H
#define __TEXTURERS_H

#include "textures.h"

extern short number_of_shading_tables, shading_table_fractional_bits, shading_table_size;
extern uint16 texture_random_seed;
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

/* ---------- texture horizontal polygon */

#define HORIZONTAL_WIDTH_SHIFT 7 /* 128 (8 for 256) */
#define HORIZONTAL_HEIGHT_SHIFT 7 /* 128 */
#define HORIZONTAL_FREE_BITS (32-TRIG_SHIFT-WORLD_FRACTIONAL_BITS)
#define HORIZONTAL_WIDTH_DOWNSHIFT (32-HORIZONTAL_WIDTH_SHIFT)
#define HORIZONTAL_HEIGHT_DOWNSHIFT (32-HORIZONTAL_HEIGHT_SHIFT)
// Ir addition:
#define HORIZONTAL_HEIGHT_COORD_DOWNSHIFT (HORIZONTAL_HEIGHT_DOWNSHIFT-HORIZONTAL_WIDTH_SHIFT)
#define HORIZONTAL_HEIGHT_COORD_MASK (((1<<HORIZONTAL_HEIGHT_SHIFT)-1)<<HORIZONTAL_WIDTH_SHIFT)
#define BIG_HORIZONTAL_WIDTH_SHIFT 8 /* 256 */
#define BIG_HORIZONTAL_HEIGHT_SHIFT 8 /* 256 */
#define BIG_HORIZONTAL_WIDTH_DOWNSHIFT (32-BIG_HORIZONTAL_WIDTH_SHIFT)
#define BIG_HORIZONTAL_HEIGHT_DOWNSHIFT (32-BIG_HORIZONTAL_HEIGHT_SHIFT)
#define BIG_HORIZONTAL_HEIGHT_COORD_DOWNSHIFT (BIG_HORIZONTAL_HEIGHT_DOWNSHIFT-BIG_HORIZONTAL_WIDTH_SHIFT)
#define BIG_HORIZONTAL_HEIGHT_COORD_MASK (((1<<BIG_HORIZONTAL_HEIGHT_SHIFT)-1)<<BIG_HORIZONTAL_WIDTH_SHIFT)

struct _horizontal_polygon_line_header
{
	int32 y_downshift;
};

struct _horizontal_polygon_line_data
{
	uint32 source_x, source_y;
	uint32 source_dx, source_dy;
	
	void *shading_table;
	short x0, x1;
};

/* ---------- texture vertical polygon */

// IR change:
#define VERTICAL_TEXTURE_WIDTH_BITS 7 
#define VERTICAL_TEXTURE_WIDTH (1<<VERTICAL_TEXTURE_WIDTH_BITS)
#define VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS (FIXED_FRACTIONAL_BITS-VERTICAL_TEXTURE_WIDTH_BITS)
#define VERTICAL_TEXTURE_ONE (1<<VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS)
#define VERTICAL_TEXTURE_FREE_BITS FIXED_FRACTIONAL_BITS
#define VERTICAL_TEXTURE_DOWNSHIFT (32-VERTICAL_TEXTURE_WIDTH_BITS)

// IR added:
#define BIG_VERTICAL_TEXTURE_WIDTH_BITS 8
#define BIG_VERTICAL_TEXTURE_WIDTH (1<<BIG_VERTICAL_TEXTURE_WIDTH_BITS)
#define BIG_VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS (FIXED_FRACTIONAL_BITS-BIG_VERTICAL_TEXTURE_WIDTH_BITS)
#define BIG_VERTICAL_TEXTURE_ONE (1<<BIG_VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS)
#define BIG_VERTICAL_TEXTURE_FREE_BITS FIXED_FRACTIONAL_BITS
#define BIG_VERTICAL_TEXTURE_DOWNSHIFT (32-BIG_VERTICAL_TEXTURE_WIDTH_BITS)

// IR removed: these are exact duplicates of whats above:
//#define HORIZONTAL_WIDTH_SHIFT 7 /* 128 (8 for 256) *
//#define HORIZONTAL_HEIGHT_SHIFT 7 /* 128 */
//#define HORIZONTAL_FREE_BITS (32-TRIG_SHIFT-WORLD_FRACTIONAL_BITS)
//#define HORIZONTAL_WIDTH_DOWNSHIFT (32-HORIZONTAL_WIDTH_SHIFT)
//#define HORIZONTAL_HEIGHT_DOWNSHIFT (32-HORIZONTAL_HEIGHT_SHIFT)

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
	short y0, y1;
};


// IR addition: these macros mac the prototypes MUCH nicer and more compact.
#define HORIZONTAL_TEXTURE_PARAMS struct bitmap_definition *texture, \
	struct bitmap_definition *screen, struct _horizontal_polygon_line_data *data, \
	short y0, short line_count, int32 extra

#define VERTICAL_TEXTURE_PARAMS struct bitmap_definition *screen, \
	struct _vertical_polygon_data *data, int32 extra

// IR change: Using macro
void _texture_horizontal_polygon_lines8(HORIZONTAL_TEXTURE_PARAMS);
void _texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);

// IR change: Using macro
void _texture_vertical_polygon_lines8(VERTICAL_TEXTURE_PARAMS);
void _transparent_texture_vertical_polygon_lines8(VERTICAL_TEXTURE_PARAMS);
void _tint_vertical_polygon_lines8(VERTICAL_TEXTURE_PARAMS);
void _randomize_vertical_polygon_lines8(VERTICAL_TEXTURE_PARAMS);

void _texture_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);
void _transparent_texture_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);
void _tint_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);
void _randomize_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);

void _texture_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);
void _transparent_texture_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);
void _tint_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);
void _randomize_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);

void _big_texture_horizontal_polygon_lines8(HORIZONTAL_TEXTURE_PARAMS);
void _big_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _big_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);

// IR addition: prep for B&B
void _transparent_texture_horizontal_polygon_lines8(HORIZONTAL_TEXTURE_PARAMS);
void _transparent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _transparent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);
void _big_transparent_texture_horizontal_polygon_lines8(HORIZONTAL_TEXTURE_PARAMS);
void _big_transparent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _big_transparent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);

// IR addition: software SeeThroughLiquids
// (transpucent = transparent with translucent "opaque" pixels)
void _translucent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _translucent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);
void _transpucent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _transpucent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);
void _translucent_texture_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);
void _translucent_texture_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);
void _transpucent_texture_vertical_polygon_lines16(VERTICAL_TEXTURE_PARAMS);
void _transpucent_texture_vertical_polygon_lines32(VERTICAL_TEXTURE_PARAMS);

void _big_translucent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _big_translucent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);
void _big_transpucent_texture_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _big_transpucent_texture_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);

// IR change: Using macro
void _landscape_horizontal_polygon_lines8(HORIZONTAL_TEXTURE_PARAMS);
void _landscape_horizontal_polygon_lines16(HORIZONTAL_TEXTURE_PARAMS);
void _landscape_horizontal_polygon_lines32(HORIZONTAL_TEXTURE_PARAMS);

// IR addition: function pointers for each type.
typedef void (horizontal_texturer)(HORIZONTAL_TEXTURE_PARAMS);
typedef void (vertical_texturer)(VERTICAL_TEXTURE_PARAMS);

#endif
