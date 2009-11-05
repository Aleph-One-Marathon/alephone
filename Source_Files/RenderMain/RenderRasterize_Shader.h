/*
 *  RenderRasterize_Shader.h
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */


#ifndef _RENDERRASTERIZE_SHADER__H
#define _RENDERRASTERIZE_SHADER__H

#include "cseries.h"
#include "map.h"
#include "RenderRasterize.h"

class Blur;
class RenderRasterize_Shader : public RenderRasterizerClass {

	Blur* blurLarge;
	Blur* blurSmall;

protected:
	virtual void render_node(sorted_node_data *node, bool SeeThruLiquids, RenderStep renderStep);	
	virtual void store_endpoint(endpoint_data *endpoint, long_vector2d& p);

	virtual void render_node_floor_or_ceiling(
		  clipping_window_data *window, polygon_data *polygon, horizontal_surface_data *surface,
		  bool void_present, bool ceil, RenderStep renderStep);
	virtual void render_node_side(
		  clipping_window_data *window, vertical_surface_data *surface,
		  bool void_present, RenderStep renderStep);

	virtual void render_node_object(render_object_data *object, bool other_side_of_media, RenderStep renderStep);
	
public:

	RenderRasterize_Shader() : blurSmall(NULL), blurLarge(NULL), RenderRasterizerClass() {}

	virtual void setupGL();

	virtual void render_tree(void);

	bool setupTexture(const shape_descriptor& texture, short transferMode, short transfer_phase, RenderStep renderStep);
};

void FlatTexture();

#endif
