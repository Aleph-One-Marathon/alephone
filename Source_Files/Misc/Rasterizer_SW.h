#ifndef _RASTERIZER_SOFTWARE_CLASS_
#define _RASTERIZER_SOFTWARE_CLASS_
/*
	
	Rasterizer software impementation (plugs into scottish_textures files)
	by Loren Petrich,
	August 7, 2000
	
*/

#include "Rasterizer.h"


class Rasterizer_SW_Class: public RasterizerClass
{
public:

	// Pointers to stuff used in scottish_textures:
	view_data *view;
	// Calling this one "screen" for scottish_textures convenience:
	bitmap_definition *screen;

	// Sets the rasterizer's view data;
	// be sure to call it before doing any rendering
	void SetView(view_data& View) {view = &View;}
	
	// Rendering calls
	// These are defined in scottish_textures.c (too great a name to change)
	
	void texture_horizontal_polygon(polygon_definition& textured_polygon);
	
	void texture_vertical_polygon(polygon_definition& textured_polygon);
	
	void texture_rectangle(rectangle_definition& textured_rectangle);
};


#endif
