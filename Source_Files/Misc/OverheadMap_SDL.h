/*
 *  OverheadMap_SDL.h -- Subclass of OverheadMapClass for rendering with SDL
 */

#ifndef _OVERHEAD_MAP_SDL_CLASS_
#define _OVERHEAD_MAP_SDL_CLASS_

#include "OverheadMapRenderer.h"


class OverheadMap_SDL_Class : public OverheadMapClass {
protected:
	void draw_polygon(
		short vertex_count,
		short *vertices,
		rgb_color &color);

	void draw_line(
		short *vertices,
		rgb_color &color,
		short pen_size);

	void draw_thing(
		world_point2d &center,
		rgb_color &color,
		short shape,
		short radius);

	void draw_player(
		world_point2d &center,
		angle facing,
		rgb_color &color,
		short shrink,
		short front,
		short rear,
		short rear_theta);

	void draw_text(
		world_point2d &location,
		rgb_color &color,
		char *text,
		FontSpecifier& FontData,
		// FontDataStruct &FontData,
		short justify);

	void set_path_drawing(rgb_color &color);

	void draw_path(
		short step,	// 0: first point
		world_point2d &location);

private:
	uint32 path_pixel;
	world_point2d path_point;
};

#endif
