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

class Rasterizer_Shader_Class : public Rasterizer_OGL_Class {

public:

	Rasterizer_Shader_Class() : Rasterizer_OGL_Class() {}

	virtual void SetView(view_data& View);

};

#endif
