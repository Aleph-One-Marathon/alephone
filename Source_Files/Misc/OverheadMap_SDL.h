/*
 *  OverheadMap_SDL.h -- Subclass of OverheadMapClass for rendering with SDL
 */

#ifndef _OVERHEAD_MAP_SDL_CLASS_
#define _OVERHEAD_MAP_SDL_CLASS_

#include "OverheadMapRenderer.h"


class OverheadMap_SDL_Class : public OverheadMapClass {
	void draw_polygon(short vertex_count, short *vertices, rgb_color& color);
	void draw_thing(world_point2d &center, rgb_color &color, short shape, short radius);
	void draw_player(world_point2d &center, angle facing, rgb_color &color, short shrink, short front, short rear, short rear_theta);
	void draw_text(world_point2d &location, rgb_color &color, char *text, FontDataStruct &FontData, short which, short justify);
};

#endif
