#ifndef _OVERHEAD_MAP_OPENGL_CLASS_
#define _OVERHEAD_MAP_OPENGL_CLASS_
/*
	
	Overhead-Map OpenGL Class
	by Loren Petrich,
	August 3, 2000
	
	Subclass of OverheadMapClass for doing rendering in OpenGL
*/

#include "GrowableList.h"
#include "OverheadMapRenderer.h"


// Font-cache data, so fonts don't have to be repeatedly re-uploaded
struct FontCacheData
{
	FontDataStruct FontData;	// Font details
	unsigned long DispList;		// OpenGL display list for fonts
	bool WasInited;				// Was this dataset inited?
	
	// Update font display list
	void Update();
	
	// Clear font display list
	void Clear();
	
	FontCacheData() {WasInited = false;}
};


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
		FontDataStruct& FontData,
		short which,
		short justify);
	
	void set_path_drawing(rgb_color& color);
	void draw_path(
		short step,	// 0: first point
		world_point2d &location);
	
	void finish_path();
	
	// Cached polygons and their color
	GrowableList<unsigned short> PolygonCache;
	rgb_color SavedColor;

	// Cached polygon lines and their width
	GrowableList<unsigned short> LineCache;
	short SavedPenSize;
	
	// Cached lines For drawing monster paths
	GrowableList<world_point2d> PathPoints;
	
	// Font caching: the cache has space for all the different-sized annotation fonts
	// and also for the map-name font
	enum {FONT_CACHE_SIZE = NUMBER_OF_ANNOTATION_SIZES+1};
	FontCacheData FontCache[FONT_CACHE_SIZE];

public:
	void ResetFonts();
};

#endif
