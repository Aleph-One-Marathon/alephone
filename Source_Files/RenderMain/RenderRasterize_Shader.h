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
#include "Rasterizer_Shader.h"

#include <memory>

class Blur;
class RenderRasterize_Shader : public RenderRasterizerClass {

	std::unique_ptr<Blur> blur;
	Rasterizer_Shader_Class *RasPtr;
	
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

    void render_viewer_sprite_layer(RenderStep renderStep);
    void render_viewer_sprite(rectangle_definition& RenderRectangle, RenderStep renderStep);
	
public:

	RenderRasterize_Shader();
	~RenderRasterize_Shader();

	virtual void setupGL(Rasterizer_Shader_Class& Rasterizer);

	virtual void render_tree(void);
        bool renders_viewer_sprites_in_tree() { return true; }

	std::unique_ptr<TextureManager> setupWallTexture(const shape_descriptor& Texture, short transferMode, float pulsate, float wobble, float intensity, float offset, RenderStep renderStep);
	std::unique_ptr<TextureManager> setupSpriteTexture(const rectangle_definition& rect, short type, float offset, RenderStep renderStep);
};

#endif
