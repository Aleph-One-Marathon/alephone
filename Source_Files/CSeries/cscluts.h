// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_CLUTS_
#define _CSERIES_CLUTS_

// Need this here
#include "cstypes.h"

typedef struct rgb_color {
	uint16 red;
	uint16 green;
	uint16 blue;
} rgb_color;

typedef struct color_table {
	short color_count;
	rgb_color colors[256];
} color_table;

extern CTabHandle build_macintosh_color_table(
	color_table *table);
extern void build_color_table(
	color_table *table,
	CTabHandle clut);

enum {
	gray15Percent,
	windowHighlight,

	NUM_SYSTEM_COLORS
};

extern RGBColor rgb_black;
extern RGBColor rgb_white;
extern RGBColor system_colors[NUM_SYSTEM_COLORS];

#endif
