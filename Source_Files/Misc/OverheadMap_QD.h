#ifndef _OVERHEAD_MAP_QUICKDRAW_CLASS_
#define _OVERHEAD_MAP_QUICKDRAW_CLASS_
/*
	
	Overhead-Map Quickdraw Class
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering with Classic MacOS Quickdraw
*/

#include "OverheadMapRenderer.h"

class OverheadMap_QD_Class: public OverheadMapClass
{
	void draw_polygon(
		short vertex_count,
		short *vertices,
		rgb_color& color);

	void draw_line(
		short *vertices,
		rgb_color& color,
		short pen_size);

	void draw_thing(
		world_point2d& center,
		rgb_color& color,
		short shape,
		short radius);
	
	void draw_player(
		world_point2d& center,
		angle facing,
		rgb_color& color,
		short shrink,
		short front,
		short rear,
		short rear_theta);
	
	void draw_text(
		world_point2d& location,
		rgb_color& color,
		char *text,
		FontSpecifier& FontData,
		short justify);
	
	void set_path_drawing(rgb_color& color);
	void draw_path(
		short step,	// 0: first point
		world_point2d& location);
};

#endif