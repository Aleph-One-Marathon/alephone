#ifndef _OVERHEAD_MAP_OPENGL_CLASS_
#define _OVERHEAD_MAP_OPENGL_CLASS_
/*
	
	Overhead-Map OpenGL Class
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering in OpenGL
	
Oct 13, 2000 (Loren Petrich)
	Converted the various lists into Standard Template Library vectors
*/

#include <vector.h>
#include "OverheadMapRenderer.h"


class OverheadMap_OGL_Class: public OverheadMapClass
{
	void begin_overall();
	void end_overall();
	
	void begin_polygons();
	
	void draw_polygon(
		short vertex_count,
		short *vertices,
		rgb_color& color);
	
	void end_polygons();

	void DrawCachedPolygons();
	
	void begin_lines();

	void draw_line(
		short *vertices,
		rgb_color& color,
		short pen_size);

	void end_lines();	// Needed for flushing cached lines
	
	void DrawCachedLines();
	
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
	
	// Text justification: 0=left, 1=center
	void draw_text(
		world_point2d& location,
		rgb_color& colorr,
		char *text,
		FontSpecifier& FontData,
		short justify);
	
	void set_path_drawing(rgb_color& color);
	void draw_path(
		short step,	// 0: first point
		world_point2d &location);
	
	void finish_path();
	
	// Cached polygons and their color
	vector<unsigned short> PolygonCache;
	rgb_color SavedColor;

	// Cached polygon lines and their width
	vector<unsigned short> LineCache;
	short SavedPenSize;
	
	// Cached lines For drawing monster paths
	vector<world_point2d> PathPoints;

public:
};

#endif
