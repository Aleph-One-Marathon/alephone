/*
 *  FSRenderer.cpp
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL_opengl.h>

#include <iostream>

#include "FBO.h"

#include "FSRenderer.h"

#include "lightsource.h"
#include "media.h"
#include "player.h"
#include "weapons.h"
#include "AnimatedTextures.h"
#include "OGL_Faders.h"
#include "OGL_Textures.h"
#include "ChaseCam.h"

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

unsigned gVertexCount;
unsigned gPolyCount;

inline bool FogActive();
void FindShadingColor(GLdouble Depth, _fixed Shading, GLfloat *Color);

/*
 * setup a flat bump texture
 */
void FlatTexture() {

	GLubyte flatTextureData[4] = {0x80, 0x80, 0xFF, 0x80};

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, flatTextureData);
	assert(glGetError() == GL_NO_ERROR);	
}

/*
 * initialize some stuff
 * happens once after opengl, shaders and textures are setup
 */
void FSRenderer::setupGL() {

	Shader* sV = Shader::get("blurV");
	Shader* sH = Shader::get("blurH");

	if(sH && sV) {
		blurLarge = new Blur(320, 160, sH, sV);
		blurSmall = new Blur(640, 320, sH, sV);
	}
	assert(glGetError() == GL_NO_ERROR);

	glEnable(GL_POLYGON_OFFSET_EXT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
}

/*
 * override for RenderRasterizerClass::render_tree()
 *
 * with multiple rendering passes for glow effect
 */
void FSRenderer::render_tree() {

//	std::cout << gPolyCount << std::endl;
//	std::cout << gVertexCount << std::endl;
	gVertexCount = 0;
	gPolyCount = 0;

	FSRenderer::render_tree(kDiffuse);

	if(blurLarge && blurSmall) {

		glDisable(GL_FOG);

		blurLarge->begin();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		FSRenderer::render_tree(kGlow);
		blurLarge->end();

		blurSmall->begin();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		FSRenderer::render_tree(kGlow);
		blurSmall->end();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		blurSmall->draw();
		blurLarge->draw();
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_FOG);
		glEnable(GL_ALPHA_TEST);
	}
}

/*
 * override for RenderRasterizerClass::render_tree()
 * this is basically a copy of RenderRasterizerClass::render_tree()
 * 
 * need to be able to distinguish from floor/ceil for sake of tangent space
 * also rendering pass info is necessary
 */
void FSRenderer::render_tree(const RenderStep& renderStep) {

	std::vector<sorted_node_data>::iterator node;
	std::vector<sorted_node_data>& SortedNodes = RSPtr->SortedNodes;
	
	bool SeeThruLiquids = true;
	
	for(node = SortedNodes.begin(); node != SortedNodes.end(); ++node) {

		polygon_data *polygon= get_polygon_data(node->polygon_index);
		clipping_window_data *window;
		render_object_data *object;
		
		horizontal_surface_data floor_surface, ceiling_surface;
		short i;
		
		ceiling_surface.origin= polygon->ceiling_origin;
		ceiling_surface.height= polygon->ceiling_height;
		ceiling_surface.texture= polygon->ceiling_texture;
		ceiling_surface.lightsource_index= polygon->ceiling_lightsource_index;
		ceiling_surface.transfer_mode= polygon->ceiling_transfer_mode;
		ceiling_surface.transfer_mode_data= 0;
		
		floor_surface.origin= polygon->floor_origin;
		floor_surface.height= polygon->floor_height;
		floor_surface.texture= polygon->floor_texture;
		floor_surface.lightsource_index= polygon->floor_lightsource_index;
		floor_surface.transfer_mode= polygon->floor_transfer_mode;
		floor_surface.transfer_mode_data= 0;
		
		media_data *media = NULL;
		if (polygon->media_index!=NONE)
			media = get_media_data(polygon->media_index);
		
		if (media && !SeeThruLiquids)
		{
			horizontal_surface_data *media_surface= NULL;
			
			if (view->under_media_boundary)
			{
				if (media->height <= polygon->floor_height)
					continue;
				
				if (media->height<polygon->ceiling_height) media_surface= &ceiling_surface;
			}
			else
			{
				if (media->height >= polygon->ceiling_height)
					continue;
				
				if (media->height>polygon->floor_height) media_surface= &floor_surface;
			}
			
			if (media_surface)
			{
				media_surface->origin= media->origin;
				media_surface->height= media->height;
				media_surface->texture= media->texture;
				media_surface->lightsource_index= polygon->media_lightsource_index;
				media_surface->transfer_mode= media->transfer_mode;
				media_surface->transfer_mode_data= 0;
			}
		}
		else if (!SeeThruLiquids)
		{
			if (view->under_media_boundary) continue;
		}

        // Set up clipping planes for this node
        clipping_window_data *left_win = node->clipping_windows;
        clipping_window_data *right_win = left_win;
        if (left_win)
        {
            while (right_win->next_window)
                right_win = right_win->next_window;
            
            GLdouble clip[] = { 0., 0., 0., 0. };
            
			world_point3d cam_pos = local_player->camera_location;
			short cam_poly;
			angle cam_yaw = FIXED_INTEGERAL_PART(local_player->variables.direction);
			angle cam_pitch;
			ChaseCam_GetPosition(cam_pos, cam_poly, cam_yaw, cam_pitch);
			
            // recenter to player's orientation temporarily
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(cam_pos.x, cam_pos.y, 0.);
            glRotatef(cam_yaw * (360/float(FULL_CIRCLE)) + 90., 0., 0., 1.);
            
            glRotatef(-1., 0., 0., 1.); // give an extra degree to avoid artifacts at edges
            clip[0] = left_win->left.i;
            clip[1] = left_win->left.j;
            glEnable(GL_CLIP_PLANE0);
            glClipPlane(GL_CLIP_PLANE0, clip);
            
            glRotatef(2., 0., 0., 1.); // breathing room for right-hand clip
            clip[0] = right_win->right.i;
            clip[1] = right_win->right.j;
            glEnable(GL_CLIP_PLANE1);
            glClipPlane(GL_CLIP_PLANE1, clip);
            
            glPopMatrix();
        }
        
		for (window= node->clipping_windows; window; window= window->next_window)
		{
			if (ceiling_surface.height>floor_surface.height)
			{
				if (ceiling_surface.height>view->origin.z) {
					render_node_floor_or_ceiling(window, polygon, &ceiling_surface, true, true, renderStep);
				}
				
				for (i= 0; i<polygon->vertex_count; ++i) {
					short side_index= polygon->side_indexes[i];
					
					if (side_index!=NONE && TEST_RENDER_FLAG(side_index, _side_is_visible)) {

						line_data *line= get_line_data(polygon->line_indexes[i]);
						side_data *side= get_side_data(side_index);
						vertical_surface_data surface;
						
						surface.length= line->length;

						const world_point2d& endpoint0 = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
						surface.p0.i = endpoint0.x; surface.p0.j = endpoint0.y;
						const world_point2d& endpoint1 = get_endpoint_data(polygon->endpoint_indexes[(i+1)%polygon->vertex_count])->vertex;
						surface.p1.i = endpoint1.x; surface.p1.j = endpoint1.y;
						surface.ambient_delta= side->ambient_delta;
						
						bool void_present;
						
						switch (side->type) {
							case _full_side:
								void_present = true;
								if (polygon->adjacent_polygon_indexes[i] != NONE) void_present = false;
								
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= floor_surface.height;
								surface.hmax= ceiling_surface.height;
								surface.h1= polygon->ceiling_height;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(window, &surface, void_present, renderStep);
								break;
							case _split_side: /* render _low_side first */
								surface.lightsource_index= side->secondary_lightsource_index;
								surface.h0= floor_surface.height;
								surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height);
								surface.hmax= ceiling_surface.height;
								surface.texture_definition= &side->secondary_texture;
								surface.transfer_mode= side->secondary_transfer_mode;
								render_node_side(window, &surface, true, renderStep);
								/* fall through and render high side */
							case _high_side:
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= MIN(line->lowest_adjacent_ceiling, ceiling_surface.height);
								surface.hmax= ceiling_surface.height;
								surface.h1= polygon->ceiling_height;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(window, &surface, true, renderStep);
								break;
							case _low_side:
								surface.lightsource_index= side->primary_lightsource_index;
								surface.h0= floor_surface.height;
								surface.h1= MAX(line->highest_adjacent_floor, floor_surface.height);
								surface.hmax= ceiling_surface.height;
								surface.texture_definition= &side->primary_texture;
								surface.transfer_mode= side->primary_transfer_mode;
								render_node_side(window, &surface, true, renderStep);
								break;
								
							default:
								assert(false);
								break;
						}
						
						if (side->transparent_texture.texture!=UNONE) {

							surface.lightsource_index= side->transparent_lightsource_index;
							surface.h0= MAX(line->highest_adjacent_floor, floor_surface.height);
							surface.h1= line->lowest_adjacent_ceiling;
							surface.hmax= ceiling_surface.height;
							surface.texture_definition= &side->transparent_texture;
							surface.transfer_mode= side->transparent_transfer_mode;
							render_node_side(window, &surface, false, renderStep);
						}
					}
				}
				
				if (floor_surface.height<view->origin.z) {
					render_node_floor_or_ceiling(window, polygon, &floor_surface, true, false, renderStep);
				}
			}
            
		}
		
		if (SeeThruLiquids)
		{
			for (object= node->exterior_objects; object; object= object->next_object)
			{
				render_node_object(object, true, *view, renderStep);
			}
		}
		
		if (media && SeeThruLiquids)
		{
			if (media->height > polygon->floor_height && media->height < polygon->ceiling_height)
			{				
				horizontal_surface_data LiquidSurface;
				
				LiquidSurface.origin= media->origin;
				LiquidSurface.height= media->height;
				LiquidSurface.texture= media->texture;
				LiquidSurface.lightsource_index= polygon->media_lightsource_index;
				LiquidSurface.transfer_mode= media->transfer_mode;
				LiquidSurface.transfer_mode_data= 0;
				
				for (window= node->clipping_windows; window; window= window->next_window) {
					render_node_floor_or_ceiling(window, polygon, &LiquidSurface, false, false, renderStep);
				}
			}
		}
		
		for (object= node->exterior_objects; object; object= object->next_object)
		{
			render_node_object(object, false, *view, renderStep);
		}

        // turn off clipping planes
        glDisable(GL_CLIP_PLANE0);
        glDisable(GL_CLIP_PLANE1);
	}
}

TextureManager SetupTexture(const rectangle_definition& rect, short type, const RenderStep& renderStep) {

	Shader *s = NULL;
	GLfloat color[3];
	FindShadingColor(rect.depth, rect.ambient_shade, color);

	TextureManager TMgr;	

	TMgr.ShapeDesc = rect.ShapeDesc;
	TMgr.LowLevelShape = rect.LowLevelShape;
	TMgr.ShadingTables = rect.shading_tables;
	TMgr.Texture = rect.texture;
	TMgr.TransferMode = rect.transfer_mode;
	TMgr.TransferData = rect.transfer_data;
	TMgr.TextureType = type;

	switch(TMgr.TransferMode) {
		case _static_transfer:
			s = Shader::get("random");
			break;
		case _tinted_transfer:
			glColor4f(0.0,0.0,0.0, 1.0 - rect.transfer_data/32.0F);
			TMgr.TransferMode = _textured_transfer;
			s = Shader::get("invisible");
			break;
		case _solid_transfer:
			glColor4f(0,1,0,1);
			break;
		case _big_landscaped_transfer:
			s = Shader::get("landscape");
			break;
		case _textured_transfer:
			glColor4f(color[0],color[1],color[2],1);
			break;
		default:
			glColor4f(0,0,1,1);
	}

	if(s == NULL) {
		if(local_player->infravision_duration) {
			s = Shader::get("flat_infravision");
		} else {
			s = Shader::get("flat");
		}
	}

	if(TMgr.Setup()) {
		/* preload textures just to make sure
		 * animated textures seem not to get preloaded correctly (bug somewhere) */
		TMgr.RenderNormal();
		if(TMgr.IsGlowMapped()) {
			TMgr.RenderGlowing();
		}
		if(renderStep == kDiffuse) {
			TMgr.RenderNormal();			
		} else {
			if(TMgr.IsGlowMapped()) {
				TMgr.RenderGlowing();				
			} else {
				glColor4f((color[0] - 0.5) * 0.0125, (color[1] - 0.5) * 0.0125, (color[2] - 0.5) * 0.0125, 1.0);
				TMgr.RenderNormal();
				s = NULL;
			}
		}
	}

	TMgr.SetupTextureMatrix();

	if(s) { s->enable(); }
	return TMgr;
}

bool FSRenderer::setupTexture(const shape_descriptor& Texture, short transferMode, short transfer_phase, const RenderStep& renderStep) {

	bool color = true;
	Shader *s = NULL;

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	TextureManager TMgr;
	get_shape_bitmap_and_shading_table(Texture, &TMgr.Texture, &TMgr.ShadingTables, _shading_normal);
	
	TMgr.ShapeDesc = Texture;
	TMgr.TransferMode = _textured_transfer;
	TMgr.TransferData = 0;

	switch(transferMode) {
		case _xfer_landscape:
		case _xfer_big_landscape:
			TMgr.TextureType = OGL_Txtr_Landscape;
			s = Shader::get("landscape");
			break;
		default:
			TMgr.TextureType = OGL_Txtr_Wall;
			if(renderStep == kDiffuse) {
				if(local_player->infravision_duration) {
					s = Shader::get("infravision");
				} else {
					s = Shader::get("parallax");
				}
			} else {
				s = Shader::get("specular");
			}
	}

	if(TMgr.Setup()) {
		/* preload textures just to make sure
		 * animated textures seem not to get preloaded correctly (bug somewhere) */
		TMgr.RenderNormal();
		if(TMgr.IsGlowMapped()) {
			TMgr.RenderGlowing();
		}
		TMgr.RenderBump();

		if(s) {
			glActiveTextureARB(GL_TEXTURE1_ARB);
			TMgr.RenderBump();
			glActiveTextureARB(GL_TEXTURE0_ARB);			
		}

		if(renderStep == kDiffuse) {
			TMgr.RenderNormal();
		} else {
			if(TMgr.IsGlowMapped()) {
				TMgr.RenderGlowing();
			} else {
				if(TMgr.TextureType != OGL_Txtr_Landscape) {
					/* fallback to draw the same texture as glow */
					color = false;
				}
				TMgr.RenderNormal();
			}
		}
	}

	TMgr.SetupTextureMatrix();

	if(s) {
		switch(transferMode) {
			case _xfer_fast_wobble:
				transfer_phase*= 15;
			case _xfer_pulsate:
			case _xfer_wobble:
				transfer_phase&= WORLD_ONE/16-1;
				transfer_phase= (transfer_phase>=WORLD_ONE/32) ? (WORLD_ONE/32+WORLD_ONE/64 - transfer_phase) : (transfer_phase - WORLD_ONE/64);
				s->setFloat("wobble", transfer_phase / 255.0);
				break;
			default:
				s->setFloat("wobble", 0.0);
		}
		glGetError();
		s->enable();
	}
	return color;
}

inline void setWallColor(GLfloat intensity, const RenderStep& renderStep) {

	if(local_player->infravision_duration) {
		if(renderStep == kGlow) {
			glColor4f(0,0,0,1);
		} else {
			glColor4f(0,0,1,1);					
		}
	} else {
		if(renderStep == kGlow) {
			intensity = intensity > 0.5 ? intensity * 2.0 - 1.0 : 0;
		}
		glColor4f(intensity, intensity, intensity, 1.0);
	}
}

void instantiate_transfer_mode(struct view_data *view, 	short transfer_mode, world_distance &x0, world_distance &y0) {
	world_distance vector_magnitude;
	short alternate_transfer_phase;
	short transfer_phase = view->tick_count;

	switch (transfer_mode) {

		case _xfer_fast_horizontal_slide:
		case _xfer_horizontal_slide:
		case _xfer_vertical_slide:
		case _xfer_fast_vertical_slide:
		case _xfer_wander:
		case _xfer_fast_wander:
			x0 = y0= 0;
			switch (transfer_mode) {
				case _xfer_fast_horizontal_slide: transfer_phase<<= 1;
				case _xfer_horizontal_slide: x0= (transfer_phase<<2)&(WORLD_ONE-1); break;
					
				case _xfer_fast_vertical_slide: transfer_phase<<= 1;
				case _xfer_vertical_slide: y0= (transfer_phase<<2)&(WORLD_ONE-1); break;
					
				case _xfer_fast_wander: transfer_phase<<= 1;
				case _xfer_wander:
					alternate_transfer_phase= transfer_phase%(10*FULL_CIRCLE);
					transfer_phase= transfer_phase%(6*FULL_CIRCLE);
					x0 = (cosine_table[NORMALIZE_ANGLE(alternate_transfer_phase)] +
						 (cosine_table[NORMALIZE_ANGLE(2*alternate_transfer_phase)]>>1) +
						 (cosine_table[NORMALIZE_ANGLE(5*alternate_transfer_phase)]>>1))>>(WORLD_FRACTIONAL_BITS-TRIG_SHIFT+2);
					y0 = (sine_table[NORMALIZE_ANGLE(transfer_phase)] +
						 (sine_table[NORMALIZE_ANGLE(2*transfer_phase)]>>1) +
						 (sine_table[NORMALIZE_ANGLE(3*transfer_phase)]>>1))>>(WORLD_FRACTIONAL_BITS-TRIG_SHIFT+2);
					break;
			}
			break;
		// wobble is done in the shader
		default:
			break;
	}
}

void FSRenderer::render_node_floor_or_ceiling(clipping_window_data*,
	polygon_data *polygon, horizontal_surface_data *surface, bool void_present, bool ceil, const RenderStep& renderStep) {

	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture);

	if(texture == UNONE) { return; }
	bool tex = setupTexture(texture, surface->transfer_mode, view->tick_count, renderStep);
	float intensity = get_light_data(surface->lightsource_index)->intensity / float(FIXED_ONE - 1);
	if(!tex) {
		intensity = 0;
	} else if(renderStep == kGlow) {
		intensity = 1;
	}

	if(void_present) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	} else {
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);		
	}
	
	short vertex_count = polygon->vertex_count;

	if (vertex_count) {

		setWallColor(intensity, renderStep);
		world_distance x = 0.0, y = 0.0;
		instantiate_transfer_mode(view, surface->transfer_mode, x, y);

		if(ceil) {
			glNormal3f(0,0,-1);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, 0,1,0, -1);
		} else {
			glNormal3f(0,0,1);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, 0,1,0,1);
		}

		++gPolyCount;
		gVertexCount += vertex_count;
		glBegin(GL_POLYGON);
		for(short i=0; i<vertex_count; ++i) {
			world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
			glTexCoord2f((vertex.x + surface->origin.x - x) / float(WORLD_ONE), (vertex.y + surface->origin.y - y) / float(WORLD_ONE));
			glVertex3f(vertex.x, vertex.y, surface->height);
		}
		glEnd();

		Shader::disable();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
}

void FSRenderer::render_node_side(clipping_window_data*, vertical_surface_data *surface, bool void_present, const RenderStep& renderStep) {

	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture_definition->texture);
	if(texture == UNONE) { return; }

	bool tex = setupTexture(texture, surface->transfer_mode, view->tick_count, renderStep);

	if(void_present) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	} else {
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glPolygonOffset(-10.0, -0.01);
	}
	world_distance h= MIN(surface->h1, surface->hmax);
	
	if (h>surface->h0) {

		world_point2d vertex[2];
		uint16 flags;
		flagged_world_point3d vertices[MAXIMUM_VERTICES_PER_WORLD_POLYGON];
		short vertex_count;

		/* initialize the two posts of our trapezoid */
		vertex_count= 2;
		long_to_overflow_short_2d(surface->p0, vertex[0], flags);
		long_to_overflow_short_2d(surface->p1, vertex[1], flags);
		float intensity = (get_light_data(surface->lightsource_index)->intensity + surface->ambient_delta) / float(FIXED_ONE - 1);
		if(!tex) {
			intensity = 0;
		}
		
		if (vertex_count) {

			vertex_count= 4;
			vertices[0].z= vertices[1].z= h;
			vertices[2].z= vertices[3].z= surface->h0;
			vertices[0].x= vertices[3].x= vertex[0].x, vertices[0].y= vertices[3].y= vertex[0].y;
			vertices[1].x= vertices[2].x= vertex[1].x, vertices[1].y= vertices[2].y= vertex[1].y;
			vertices[0].flags = vertices[3].flags = 0;
			vertices[1].flags = vertices[2].flags = 0;

			double div = WORLD_ONE;
			double dx = (surface->p1.i - surface->p0.i) / double(surface->length);
			double dy = (surface->p1.j - surface->p0.j) / double(surface->length);

			world_distance x0 = WORLD_FRACTIONAL_PART(surface->texture_definition->x0);
			world_distance y0 = WORLD_FRACTIONAL_PART(surface->texture_definition->y0);

			double tOffset = surface->h1 + y0;

			setWallColor(intensity, renderStep);
			
			glNormal3f(-dy, dx, 0);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, dx, dy, 0, -1);

			short alternate_transfer_phase;
			short transfer_phase = view->tick_count;

			world_distance x = 0.0, y = 0.0;
			instantiate_transfer_mode(view, surface->transfer_mode, x, y);

			x0 -= x;
			tOffset -= y;

			++gPolyCount;
			gVertexCount += vertex_count;
			glBegin(GL_QUADS);
			for(int i = 0; i < vertex_count; ++i) {
				float p2 = 0;
				if(i == 1 || i == 2) { p2 = surface->length; }
				glTexCoord2f((tOffset - vertices[i].z) / div, (x0+p2) / div);
				glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
			}
			glEnd();
			Shader::disable();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}
	glPolygonOffset(0,0);
}

bool RenderModel(rectangle_definition& RenderRectangle, short Collection, short CLUT, const RenderStep& renderStep) {

	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	OGL_SkinData *SkinPtr = ModelPtr->GetSkin(CLUT);
	if(!SkinPtr) { return false; }

	Shader *s = NULL;
	if(renderStep == kGlow) {
		Shader::get("specular");
	} else {
		Shader::get("parallax");
	}
	if(s) { s->enable(); }

	if (ModelPtr->Sidedness < 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	} else if (ModelPtr->Sidedness > 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}

	GLfloat color[3];
	FindShadingColor(RenderRectangle.depth, RenderRectangle.ambient_shade, color);
	glColor3fv(color);
	glColor4f(color[0], color[1], color[2], 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3,GL_FLOAT,0,ModelPtr->Model.PosBase());
	glTexCoordPointer(2,GL_FLOAT,0,ModelPtr->Model.TCBase());
	glNormalPointer(GL_FLOAT,0,ModelPtr->Model.NormBase());

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(4,GL_FLOAT,sizeof(vec4),ModelPtr->Model.TangentBase());

	if(ModelPtr->Use(CLUT,OGL_SkinManager::Normal)) {
		LoadModelSkin(SkinPtr->NormalImg, Collection, CLUT);
	}

	glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Revert to default blend
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	Shader::disable();

#ifdef DEBUG_MODELS
	/* draw normals and tangents as lines */
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glColor4f(1.0,0.0,0.0,1.0);

	for(unsigned i = 0; i < ModelPtr->Model.Positions.size() / 3; ++i) {
		glVertex3f(ModelPtr->Model.Positions[i*3+0],
				   ModelPtr->Model.Positions[i*3+1],
				   ModelPtr->Model.Positions[i*3+2]);
		glVertex3f(ModelPtr->Model.Positions[i*3+0] + 32*ModelPtr->Model.Normals[i*3+0],
				   ModelPtr->Model.Positions[i*3+1] + 32*ModelPtr->Model.Normals[i*3+1],
				   ModelPtr->Model.Positions[i*3+2] + 32*ModelPtr->Model.Normals[i*3+2]);
	}
	glColor4f(0.4,0.7,1.0,1.0);
	for(unsigned i = 0; i < ModelPtr->Model.Positions.size() / 3; ++i) {
		glVertex3f(ModelPtr->Model.Positions[i*3+0],
				   ModelPtr->Model.Positions[i*3+1],
				   ModelPtr->Model.Positions[i*3+2]);
		glVertex3f(ModelPtr->Model.Positions[i*3+0] + 32*ModelPtr->Model.Tangents[i][0]*ModelPtr->Model.Tangents[i][3],
				   ModelPtr->Model.Positions[i*3+1] + 32*ModelPtr->Model.Tangents[i][1]*ModelPtr->Model.Tangents[i][3],
				   ModelPtr->Model.Positions[i*3+2] + 32*ModelPtr->Model.Tangents[i][2]*ModelPtr->Model.Tangents[i][3]);
	}
	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif
	return true;
}

void FSRenderer::render_node_object(render_object_data *object, bool other_side_of_media, const view_data& view, const RenderStep& renderStep) {

	rectangle_definition& rect = object->rectangle;
	const world_point3d& pos = rect.Position;

	if(rect.ModelPtr) {
		// cheap hack to make models work //
		if(renderStep == kGlow) {
			rect.ceiling_light = 0.0f;
			rect.ambient_shade = 0.0f;
		}
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		glRotated((360.0/FULL_CIRCLE)*rect.Azimuth,0,0,1);
		GLfloat HorizScale = rect.Scale*rect.HorizScale;
		glScalef(HorizScale,HorizScale,rect.Scale);

		short descriptor = GET_DESCRIPTOR_COLLECTION(rect.ShapeDesc);
		short collection = GET_COLLECTION(descriptor);
		short clut = ModifyCLUT(rect.transfer_mode,GET_COLLECTION_CLUT(descriptor));

		RenderModel(rect, collection, clut, renderStep);
		glPopMatrix();
		return;
	}

	glPushMatrix();
	glTranslated(pos.x, pos.y, pos.z);

	double yaw = view.yaw * 360.0 / float(NUMBER_OF_ANGLES);
	glRotated(yaw, 0.0, 0.0, 1.0);

	TextureManager TMgr = SetupTexture(rect, OGL_Txtr_Inhabitant, renderStep);

	if(local_player->infravision_duration) {
		glColor4f(1,0,0,1);
	}

	float texCoords[2][2];

	if(rect.flip_vertical) {
		texCoords[0][1] = TMgr.U_Offset;
		texCoords[0][0] = TMgr.U_Scale+TMgr.U_Offset;
	} else {
		texCoords[0][0] = TMgr.U_Offset;
		texCoords[0][1] = TMgr.U_Scale+TMgr.U_Offset;
	}

	if(rect.flip_horizontal) {
		texCoords[1][1] = TMgr.V_Offset;
		texCoords[1][0] = TMgr.V_Scale+TMgr.V_Offset;
	} else {
		texCoords[1][0] = TMgr.V_Offset;
		texCoords[1][1] = TMgr.V_Scale+TMgr.V_Offset;
	}

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBegin(GL_QUADS);

	glTexCoord2f(texCoords[0][1], texCoords[1][0]);
	glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);

	glTexCoord2f(texCoords[0][1], texCoords[1][1]);
	glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);

	glTexCoord2f(texCoords[0][0], texCoords[1][1]);
	glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);

	glTexCoord2f(texCoords[0][0], texCoords[1][0]);
	glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);

	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);

	Shader::disable();
	TMgr.RestoreTextureMatrix();
}

const GLdouble kViewBaseMatrix[16] = {
	0,	0,	-1,	0,
	1,	0,	0,	0,
	0,	1,	0,	0,
	0,	0,	0,	1
};

void FSRenderer::SetView(view_data& view) {
	OGL_SetView(view);
	float fov = std::max(view.field_of_view - 35.0, 1.0);
	float flare = view.maximum_depth_intensity/float(FIXED_ONE_HALF);

	Shader* s = Shader::get("random");
	if(s) {
		s->setFloat("time", view.tick_count);
	}
	s = Shader::get("parallax");
	if(s) {
		s->setFloat("flare", flare);
	}
	s = Shader::get("flat");
	if(s) {
		s->setFloat("flare", flare);
	}
	
	glGetError();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, view.world_to_screen_y * view.screen_width / float(view.screen_height * view.world_to_screen_x), 16, 1024*1024);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(kViewBaseMatrix);
	double pitch = view.pitch * 360.0 / float(NUMBER_OF_ANGLES);
	double yaw = view.yaw * 360.0 / float(NUMBER_OF_ANGLES);
	double roll = view.roll * 360.0 / float(NUMBER_OF_ANGLES);
	glRotated(pitch, 0.0, 1.0, 0.0);
//	apperently 'roll' is not what i think it is
//	rubicon sets it to some strange value
//	glRotated(roll, 1.0, 0.0, 0.0); 
	glRotated(yaw, 0.0, 0.0, -1.0);

	glTranslated(-view.origin.x, -view.origin.y, -view.origin.z);
}

void FSRenderer::Begin() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	int FogType = (local_player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ? OGL_Fog_BelowLiquid : OGL_Fog_AboveLiquid;
	OGL_FogData *CurrFog = OGL_GetFogData(FogType);
	if (CurrFog && CurrFog->IsPresent && TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fog)) {
		GLfloat CurrFogColor[4] = {
			CurrFog->Color.red/65535.0F,
			CurrFog->Color.green/65535.0F,
			CurrFog->Color.blue/65535.0F,
			0.0
		};

		glEnable(GL_FOG);

		glFogfv(GL_FOG_COLOR,CurrFogColor);
		glFogf(GL_FOG_DENSITY, 0.75F/MAX(1,WORLD_ONE*CurrFog->Depth));
	} else {
		glFogf(GL_FOG_DENSITY, 0.0);
		glDisable(GL_FOG);
	}
}

void FSRenderer::End() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
