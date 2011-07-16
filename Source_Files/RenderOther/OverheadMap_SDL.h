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

*/

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
