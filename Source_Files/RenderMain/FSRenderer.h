/*
 *  FSRenderer.h
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */


#ifndef _FS_RASTERIZER__H
#define _FS_RASTERIZER__H

#include "cseries.h"
#include "map.h"
#include "FBO.h"
#include "RenderRasterize.h"
#include "Rasterizer_OGL.h"

typedef enum {
	kDiffuse,
	kGlow,
} RenderStep;

class FSRenderer : public RenderRasterizerClass, public Rasterizer_OGL_Class {

	Blur* blurLarge;
	Blur* blurSmall;

public:

	FSRenderer() : blurSmall(NULL), blurLarge(NULL), RenderRasterizerClass(), Rasterizer_OGL_Class() {}

	virtual void setupGL();
	virtual void render_node_floor_or_ceiling(
		  clipping_window_data *window, polygon_data *polygon, horizontal_surface_data *surface,
		  bool void_present, bool ceil, const RenderStep& renderStep);
	virtual void render_node_side(
		  clipping_window_data *window, vertical_surface_data *surface,
		  bool void_present, const RenderStep& renderStep);

	virtual void render_node_object(render_object_data *object, bool other_side_of_media, const view_data& view, const RenderStep& renderStep);

	virtual void render_tree();
	virtual void render_tree(const RenderStep&);

	virtual void render_viewer_sprite_layer(view_data *view);

	virtual void SetView(view_data& View);
	virtual void Begin();
	virtual void End();

	bool setupTexture(const shape_descriptor& texture, short transferMode, const RenderStep& renderStep);
};

void FlatTexture();

#endif