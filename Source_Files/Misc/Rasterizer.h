#ifndef _RASTERIZER_CLASS_
#define _RASTERIZER_CLASS_
/*
	
	Rasterizer Implementation Base Class
	by Loren Petrich,
	August 7, 2000
	
	To be subclassed for specific rasterizers (software, OpenGL, etc.)
*/

#include "render.h"
#include "OGL_Render.h"


class RasterizerClass
{
public:

	// Temporary pointers, to be moved into a subclass
	bitmap_definition *destination;
	view_data *view;
	
	// Sets the rasterizer's view data;
	// be sure to call it before doing any rendering
	void SetView(view_data& View) {view = &View; OGL_SetView(View);}
	
	// Rendering calls
	
	void texture_horizontal_polygon(polygon_definition& textured_polygon)
	{
		if (!OGL_RenderWall(textured_polygon,false))
			::texture_horizontal_polygon(&textured_polygon, destination, view);
	}
	
	void texture_vertical_polygon(polygon_definition& textured_polygon)
	{
		if (!OGL_RenderWall(textured_polygon,true))
			::texture_vertical_polygon(&textured_polygon, destination, view);
	}
	
	void texture_rectangle(rectangle_definition& textured_rectangle)
	{
		if (!OGL_RenderSprite(textured_rectangle))
			::texture_rectangle(&textured_rectangle, destination, view);
	}
};


#endif
