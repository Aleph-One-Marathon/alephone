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
	
	// Sets the rasterizer's view data;
	// be sure to call it before doing any rendering
	virtual void SetView(view_data& View) {}
	
	// Rendering calls
	virtual void Begin() {}
	virtual void End() {}
	
	virtual void texture_horizontal_polygon(polygon_definition& textured_polygon) {}
	
	virtual void texture_vertical_polygon(polygon_definition& textured_polygon) {}
	
	virtual void texture_rectangle(rectangle_definition& textured_rectangle) {}
};


#endif
