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
#include "OGL_FBO.h"
#include "OGL_Textures.h"

#include <memory>

class Blur;
class RenderRasterize_Shader : public RenderRasterizerClass {

	std::auto_ptr<Blur> blur;
	std::auto_ptr<FBOSwapper> swapper;
	
	int objectCount;
	world_distance objectY;
	float weaponFlare;
	float selfLuminosity;
	
	long_vector2d leftmost_clip, rightmost_clip;

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
	
	virtual void clip_to_window(clipping_window_data *win);
	virtual void _render_node_object_helper(render_object_data *object, RenderStep renderStep);
	
public:

	RenderRasterize_Shader() : blur(NULL), RenderRasterizerClass() {}

	virtual void setupGL();

	virtual void render_tree(void);

	TextureManager setupWallTexture(const shape_descriptor& Texture, short transferMode, float pulsate, float wobble, float intensity, float offset, RenderStep renderStep);
	TextureManager setupSpriteTexture(const rectangle_definition& rect, short type, float offset, RenderStep renderStep);
};

#endif
