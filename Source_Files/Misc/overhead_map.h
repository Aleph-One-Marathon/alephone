#ifndef __OVERHEAD_MAP_H
#define __OVERHEAD_MAP_H

/*
	OVERHEAD_MAP.H
	Saturday, July 9, 1994 11:19:49 PM

May 1, 2000 (Loren Petrich): Added XML parser object for the stuff here.
*/

#include "XML_ElementParser.h"

#define OVERHEAD_MAP_MINIMUM_SCALE 1
#define OVERHEAD_MAP_MAXIMUM_SCALE 4
#define DEFAULT_OVERHEAD_MAP_SCALE 3

enum /* modes */
{
	_rendering_saved_game_preview,
	_rendering_checkpoint_map,
	_rendering_game_map
};

struct overhead_map_data
{
	short mode;
	short scale;
	world_point2d origin;
	short origin_polygon_index;
	short half_width, half_height;
	short width, height;
	short top, left;
	
	bool draw_everything;
};

void _render_overhead_map(struct overhead_map_data *data);

// LP addition: get the parser for the overhead-map elements (name "overhead_map")
XML_ElementParser *OverheadMap_GetParser();
#endif
