/*
 *  Rasterizer_Shader.h
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */


#ifndef _RASTERIZER_SHADER__H
#define _RASTERIZER_SHADER__H

#include "cseries.h"
#include "map.h"
#include "Rasterizer_OGL.h"
#include <memory>

class FBOSwapper;
class Rasterizer_Shader_Class : public Rasterizer_OGL_Class {
	friend class RenderRasterize_Shader;
	
protected:
	std::unique_ptr<FBOSwapper> swapper;
	bool smear_the_void;
	short view_width;
	short view_height;

public:

	Rasterizer_Shader_Class();
	~Rasterizer_Shader_Class();

	virtual void SetView(view_data& View);
	virtual void setupGL();
	virtual void Begin();
	virtual void End();

};

#endif
