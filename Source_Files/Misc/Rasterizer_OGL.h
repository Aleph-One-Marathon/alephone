#ifndef _RASTERIZER_OPENGL_CLASS_
#define _RASTERIZER_OPENGL_CLASS_
/*
	
	Rasterizer OpenGL impementation
	by Loren Petrich,
	August 7, 2000
	
	As it says... will need to rewrite OGL_Render to make it truly object-oriented
*/

#include "Rasterizer.h"


class Rasterizer_OGL_Class: public RasterizerClass
{
public:

	// Sets the rasterizer's view data;
	// be sure to call it before doing any rendering
	void SetView(view_data& View) {OGL_SetView(View);}
	
	// Rendering calls
	void Begin() {OGL_StartMain();}
	void End() {OGL_EndMain();}
	
	void texture_horizontal_polygon(polygon_definition& textured_polygon)
	{
		OGL_RenderWall(textured_polygon,false);
	}
	
	void texture_vertical_polygon(polygon_definition& textured_polygon)
	{
		OGL_RenderWall(textured_polygon,true);
	}
	
	void texture_rectangle(rectangle_definition& textured_rectangle)
	{
		OGL_RenderSprite(textured_rectangle);
	}
};


#endif
