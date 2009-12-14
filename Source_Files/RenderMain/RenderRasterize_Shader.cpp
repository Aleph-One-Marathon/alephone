/*
 *  RenderRasterize_Shader.cpp
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */

#include "OGL_Headers.h"

#include <iostream>

#include "RenderRasterize_Shader.h"

#include "lightsource.h"
#include "media.h"
#include "player.h"
#include "weapons.h"
#include "AnimatedTextures.h"
#include "OGL_Faders.h"
#include "OGL_Textures.h"
#include "OGL_Shader.h"
#include "ChaseCam.h"
#include "preferences.h"

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

inline bool FogActive();
void FindShadingColor(GLdouble Depth, _fixed Shading, GLfloat *Color);

class FBO {
	
	friend class Blur;
private:
	GLuint _fbo;
	GLuint _depthBuffer;
public:
	GLuint _w;
	GLuint _h;
	GLuint texID;
	
	FBO(GLuint w, GLuint h) : _h(h), _w(w) {
		glGenFramebuffersEXT(1, &_fbo);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);		
		
		glGenRenderbuffersEXT(1, &_depthBuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthBuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, _w, _h);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, _depthBuffer);
		
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8, _w, _h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);		
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, texID, 0);
		assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);		
	}
	
	void activate() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);		
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, _w, _h);
	}
	
	static void deactivate() {
		glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);		
	}
	
	void draw() {
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texID);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		
		glBegin(GL_QUADS);
		glTexCoord2i(0., 0.); glVertex2i(0., 0.);
		glTexCoord2i(0., _h); glVertex2i(0., _h);	
		glTexCoord2i(_w, _h); glVertex2i(_w, _h);
		glTexCoord2i(_w, 0.); glVertex2i(_w, 0.);
		glEnd();
		
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	
	~FBO() {
		glDeleteFramebuffersEXT(1, &_fbo);
		glDeleteRenderbuffersEXT(1, &_depthBuffer);
	}	
};

class Blur {
	
private:
	FBO _horizontal;
	FBO _vertical;
	
	Shader *_shader;
	Shader *_shader2;
	
public:
	
	Blur(GLuint w, GLuint h, Shader* shaderV, Shader* shaderH)
	: _horizontal(w, h), _vertical(w, h), _shader(shaderV), _shader2(shaderH) {}
	
	void resize(GLuint w, GLuint h) {
		_horizontal = FBO(w, h);
		_vertical = FBO(w, h);
	}
	
	void begin() {
		/* draw in first buffer */
		_horizontal.activate();
	}
	
	void end() {
		FBO::deactivate();
	}
	
	void draw() {
		assert(_shader);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();	
		
		glDisable(GL_BLEND);
		glOrtho(0, _vertical._w, 0, _vertical._h, 0.0, 1.0);
		glColor4f(1., 1., 1., 1.0);
		_vertical.activate();
		_shader->enable();
		_horizontal.draw();
		_shader->disable();
		FBO::deactivate();
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
		glColor4f(1., 1., 1., 1.);
		_shader2->enable();
		_vertical.draw();
		_shader2->disable();
	}
};


/*
 * initialize some stuff
 * happens once after opengl, shaders and textures are setup
 */
void RenderRasterize_Shader::setupGL() {

	Shader* sV = Shader::get("blurV");
	Shader* sH = Shader::get("blurH");

	blur = NULL;
	if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_Blur)) {
		if(sH && sV) {
			blur = new Blur(graphics_preferences->screen_mode.width / 2, graphics_preferences->screen_mode.width / 4, sH, sV);
		}
	}

//	glDisable(GL_CULL_FACE);
//	glDisable(GL_LIGHTING);
}

/*
 * override for RenderRasterizerClass::render_tree()
 *
 * with multiple rendering passes for glow effect
 */
void RenderRasterize_Shader::render_tree() {

	Shader* s = Shader::get("random");
	if(s) {
		s->setFloat("time", view->tick_count);
	}
	s = Shader::get("random_nostatic");
	if(s) {
		s->setFloat("time", view->tick_count);
	}
	
	RenderRasterizerClass::render_tree(kDiffuse);

	if (!TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_Blur))
		return;

	s = Shader::get("parallax");
	if(s) {
		s->setFloat("flare", 0);
	}
	s = Shader::get("flat");
	if(s) {
		s->setFloat("flare", 0);
	}
	
	if(blur) {

		GLfloat fog[4] = { 0,0,0,0 };
		glFogfv(GL_FOG_COLOR, fog);

		blur->begin();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		RenderRasterizerClass::render_tree(kGlow);
		blur->end();

		glDisable(GL_DEPTH_TEST);
		blur->draw();
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_FOG);
	}
}

void RenderRasterize_Shader::render_node(sorted_node_data *node, bool SeeThruLiquids, RenderStep renderStep)
{
	clipping_window_data *left_win = node->clipping_windows;
	if (!left_win)
		return;
	
	clipping_window_data *right_win = left_win;
	while (right_win && right_win->next_window)
		right_win = right_win->next_window;

	GLdouble clip[] = { 0., 0., 0., 0. };
	
	world_point3d cam_pos = current_player->camera_location;
	short cam_poly;
	angle cam_yaw = FIXED_INTEGERAL_PART(current_player->variables.direction + current_player->variables.head_direction);
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
	
	// parasitic object detection
	objectCount = 0;
	objectY = 0;
	
	RenderRasterizerClass::render_node(node, SeeThruLiquids, renderStep);
	
	// turn off clipping planes
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
}

void RenderRasterize_Shader::store_endpoint(
	endpoint_data *endpoint,
	long_vector2d& p)
{
	p.i = endpoint->vertex.x;
	p.j = endpoint->vertex.y;
}


TextureManager RenderRasterize_Shader::setupSpriteTexture(const rectangle_definition& rect, short type, float offset, RenderStep renderStep) {

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
	TMgr.IsShadeless = (rect.flags&_SHADELESS_BIT) != 0;
	TMgr.TextureType = type;

	float flare = view->maximum_depth_intensity/float(FIXED_ONE_HALF);
	if (renderStep == kGlow) {
		flare = 0;
		for (int i = 0; i < 3; i++) {
			color[i] = color[i] > 0.5 ? (color[i] - 0.5)*0.5 : 0;
		}
	}
	glEnable(GL_TEXTURE_2D);
	glColor4f(color[0], color[1], color[2], 1);
	
	switch(TMgr.TransferMode) {
		case _static_transfer:
			TMgr.IsShadeless = 1;
			flare = 0;
			glColor4f(1,1,1,1);
			if (TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_FlatStatic))
				s = Shader::get("random_nostatic");
			else
				s = Shader::get("random");
			break;
		case _tinted_transfer:
			glColor4f(0.0,0.0,0.0, 1.0 - rect.transfer_data/32.0F);
			flare = 0;
			break;
		case _solid_transfer:
			glColor4f(0,1,0,1);
			break;
		case _big_landscaped_transfer:
			glColor4f(1,1,1,1);
			flare = 0;
			s = Shader::get("landscape");
			break;
		case _textured_transfer:
			if(TMgr.IsShadeless) {
				if (renderStep == kDiffuse) {
					glColor4f(1,1,1,1);
				} else {
					glColor4f(0,0,0,1);
				}
				flare = 0;
			}
			break;
		default:
			glColor4f(0,0,1,1);
	}
	
	if(s == NULL) {
		s = Shader::get("flat");
	}

	if(TMgr.Setup()) {
		TMgr.RenderNormal();			
	} else {
		TMgr.ShapeDesc = UNONE;
		return TMgr;
	}

	TMgr.SetupTextureMatrix();

	if(s) {
		s->setFloat("flare", flare);
		s->setFloat("wobble", 0);
		s->setFloat("offset", offset);
		s->enable();
	}
	return TMgr;
}

TextureManager RenderRasterize_Shader::setupWallTexture(const shape_descriptor& Texture, short transferMode, float wobble, float intensity, float offset, RenderStep renderStep) {

	Shader *s = NULL;

	TextureManager TMgr;
	TMgr.ShapeDesc = Texture;
	if (TMgr.ShapeDesc == UNONE) { return TMgr; }
	get_shape_bitmap_and_shading_table(Texture, &TMgr.Texture, &TMgr.ShadingTables,
		current_player->infravision_duration ? _shading_infravision : _shading_normal);
	
	TMgr.TransferMode = _textured_transfer;
	TMgr.IsShadeless = current_player->infravision_duration ? 1 : 0;
	TMgr.TransferData = 0;

	float flare = view->maximum_depth_intensity/float(FIXED_ONE_HALF);
	if (renderStep == kGlow) {
		flare = 0;
		intensity = intensity > 0.5 ? (intensity - 0.5) : 0;
	}
	glEnable(GL_TEXTURE_2D);
	glColor4f(intensity, intensity, intensity, 1.0);

	switch(transferMode) {
		case _xfer_landscape:
		case _xfer_big_landscape:
		{
			TMgr.TextureType = OGL_Txtr_Landscape;
			TMgr.Landscape_AspRatExp = 0;
			LandscapeOptions *opts = View_GetLandscapeOptions(Texture);
			glColor4f(1,1,1,1);
			flare = 0;
			s = Shader::get("landscape");
			s->setFloat("repeat", 1.0 + opts->HorizExp);
		}
			break;
		default:
			TMgr.TextureType = OGL_Txtr_Wall;
			if(TMgr.IsShadeless) {
				if (renderStep == kDiffuse) {
					glColor4f(1,1,1,1);
				} else {
					glColor4f(0,0,0,1);
				}
				flare = 0;
			}
			if(!TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
				s = Shader::get("flat");
			} else if(renderStep == kDiffuse) {
				s = Shader::get("parallax");
			} else {
				s = Shader::get("specular");
			}
	}
	
	if(TMgr.Setup()) {
		if(s) {
			if (TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
				glActiveTextureARB(GL_TEXTURE1_ARB);
				TMgr.RenderBump();
				glActiveTextureARB(GL_TEXTURE0_ARB);
			}
		}
		TMgr.RenderNormal();
	} else {
		TMgr.ShapeDesc = UNONE;
		return TMgr;
	}

	TMgr.SetupTextureMatrix();

	if(s) {
		s->setFloat("wobble", wobble);
		s->setFloat("flare", flare);
		s->setFloat("offset", offset);
		s->enable();
	}
	return TMgr;
}

void instantiate_transfer_mode(struct view_data *view, 	short transfer_mode, world_distance &x0, world_distance &y0) {
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

float calcWobble(short transferMode, short transfer_phase) {
	float wobble = 0;
	switch(transferMode) {
		case _xfer_fast_wobble:
			transfer_phase*= 15;
		case _xfer_pulsate:
		case _xfer_wobble:
			transfer_phase&= WORLD_ONE/16-1;
			transfer_phase= (transfer_phase>=WORLD_ONE/32) ? (WORLD_ONE/32+WORLD_ONE/64 - transfer_phase) : (transfer_phase - WORLD_ONE/64);
			wobble = transfer_phase / 255.0;
			break;
	}
	return wobble;
}

bool setupGlow(TextureManager &TMgr, float wobble, float intensity, float offset, RenderStep renderStep) {
	if (TMgr.IsGlowMapped() && TMgr.TransferMode == _textured_transfer) {
		Shader *s = NULL;
		if (TMgr.TextureType == OGL_Txtr_Wall && TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
			s = Shader::get(renderStep == kGlow ? "specular" : "parallax");
		} else {
			s = Shader::get("flat");
		}
		s->setFloat("flare", 0);
		s->setFloat("wobble", wobble);
		s->setFloat("offset", offset - 1.0);

		if (renderStep == kDiffuse) {
			glColor4f(1,1,1,1);
		} else {
			float in = intensity > 0.5 ? (intensity - 0.5) * 2.0 : 1;
			glColor4f(in,in,in,1);
		}
		TMgr.RenderGlowing();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		return true;
	}
	return false;
}	

void RenderRasterize_Shader::render_node_floor_or_ceiling(clipping_window_data *window,
	polygon_data *polygon, horizontal_surface_data *surface, bool void_present, bool ceil, RenderStep renderStep) {

	float offset = 0;
	if (void_present) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	} else {
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}
	
	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture);
	float intensity = get_light_data(surface->lightsource_index)->intensity / float(FIXED_ONE - 1);
	float wobble = calcWobble(surface->transfer_mode, view->tick_count);
	TextureManager TMgr = setupWallTexture(texture, surface->transfer_mode, wobble, intensity, offset, renderStep);
	if(TMgr.ShapeDesc == UNONE) { return; }
	
	short vertex_count = polygon->vertex_count;

	if (vertex_count) {

		world_distance x = 0.0, y = 0.0;
		instantiate_transfer_mode(view, surface->transfer_mode, x, y);

		if(ceil) {
			glNormal3f(0,0,-1);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, 0,1,0, -1);
		} else {
			glNormal3f(0,0,1);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, 0,1,0,1);
		}

		glBegin(GL_POLYGON);
		if (ceil)
		{
			for(short i=vertex_count-1; i>=0; --i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				glTexCoord2f((vertex.x + surface->origin.x + x) / float(WORLD_ONE), (vertex.y + surface->origin.y + y) / float(WORLD_ONE));
				glVertex3f(vertex.x, vertex.y, surface->height);
			}
		}
		else
		{
			for(short i=0; i<vertex_count; ++i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				glTexCoord2f((vertex.x + surface->origin.x + x) / float(WORLD_ONE), (vertex.y + surface->origin.y + y) / float(WORLD_ONE));
				glVertex3f(vertex.x, vertex.y, surface->height);
			}
		}
		glEnd();

		if (setupGlow(TMgr, wobble, intensity, offset, renderStep)) {
			glBegin(GL_POLYGON);
			if (ceil)
			{
				for(short i=vertex_count-1; i>=0; --i) {
					world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
					glTexCoord2f((vertex.x + surface->origin.x + x) / float(WORLD_ONE), (vertex.y + surface->origin.y + y) / float(WORLD_ONE));
					glVertex3f(vertex.x, vertex.y, surface->height);
				}
			}
			else
			{
				for(short i=0; i<vertex_count; ++i) {
					world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
					glTexCoord2f((vertex.x + surface->origin.x + x) / float(WORLD_ONE), (vertex.y + surface->origin.y + y) / float(WORLD_ONE));
					glVertex3f(vertex.x, vertex.y, surface->height);
				}
			}
			glEnd();
		}
		
		Shader::disable();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
}

void RenderRasterize_Shader::render_node_side(clipping_window_data *window, vertical_surface_data *surface, bool void_present, RenderStep renderStep) {
		
	float offset = 0;
	if (void_present) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	} else {
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		offset = -1.0;
	}
	
	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture_definition->texture);
	float intensity = (get_light_data(surface->lightsource_index)->intensity + surface->ambient_delta) / float(FIXED_ONE - 1);
	float wobble = calcWobble(surface->transfer_mode, view->tick_count);
	TextureManager TMgr = setupWallTexture(texture, surface->transfer_mode, wobble, intensity, offset, renderStep);
	if(TMgr.ShapeDesc == UNONE) { return; }

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
		
		if (vertex_count) {

			vertex_count= 4;
			vertices[0].z= vertices[1].z= h + view->origin.z;
			vertices[2].z= vertices[3].z= surface->h0 + view->origin.z;
			vertices[0].x= vertices[3].x= vertex[0].x, vertices[0].y= vertices[3].y= vertex[0].y;
			vertices[1].x= vertices[2].x= vertex[1].x, vertices[1].y= vertices[2].y= vertex[1].y;
			vertices[0].flags = vertices[3].flags = 0;
			vertices[1].flags = vertices[2].flags = 0;

			double div = WORLD_ONE;
			double dx = (surface->p1.i - surface->p0.i) / double(surface->length);
			double dy = (surface->p1.j - surface->p0.j) / double(surface->length);

			world_distance x0 = WORLD_FRACTIONAL_PART(surface->texture_definition->x0);
			world_distance y0 = WORLD_FRACTIONAL_PART(surface->texture_definition->y0);

			double tOffset = surface->h1 + view->origin.z + y0;

			glNormal3f(-dy, dx, 0);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, dx, dy, 0, -1);

			world_distance x = 0.0, y = 0.0;
			instantiate_transfer_mode(view, surface->transfer_mode, x, y);

			x0 -= x;
			tOffset -= y;

			glBegin(GL_QUADS);
			for(int i = 0; i < vertex_count; ++i) {
				float p2 = 0;
				if(i == 1 || i == 2) { p2 = surface->length; }
				glTexCoord2f((tOffset - vertices[i].z) / div, (x0+p2) / div);
				glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
			}
			glEnd();
			
			if (setupGlow(TMgr, wobble, intensity, offset, renderStep)) {
				glBegin(GL_QUADS);
				for(int i = 0; i < vertex_count; ++i) {
					float p2 = 0;
					if(i == 1 || i == 2) { p2 = surface->length; }
					glTexCoord2f((tOffset - vertices[i].z) / div, (x0+p2) / div);
					glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
				}
				glEnd();
			}
			
			Shader::disable();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}
}

bool RenderModel(rectangle_definition& RenderRectangle, short Collection, short CLUT, float flare, RenderStep renderStep) {

	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	OGL_SkinData *SkinPtr = ModelPtr->GetSkin(CLUT);
	if(!SkinPtr) { return false; }

	if (ModelPtr->Sidedness < 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	} else if (ModelPtr->Sidedness > 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}

	GLfloat color[3];
	FindShadingColor(RenderRectangle.depth, RenderRectangle.ambient_shade, color);
	if (renderStep == kGlow) {
		flare = 0;
		for (int i = 0; i < 3; i++) {
			color[i] = color[i] > 0.5 ? (color[i] - 0.5)*0.5 : 0;
		}
	}
	glColor4f(color[0], color[1], color[2], 1.0);
	
	Shader *s = NULL;
	if(!TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
		s = Shader::get("flat");
	} else if(renderStep == kDiffuse) {
		s = Shader::get("model_parallax");
	} else {
		s = Shader::get("model_specular");
	}
	if(s) {
		s->setFloat("flare", flare);
		s->setFloat("wobble", 0);
		s->enable();
	}

	glVertexPointer(3,GL_FLOAT,0,ModelPtr->Model.PosBase());
	glTexCoordPointer(2,GL_FLOAT,0,ModelPtr->Model.TCBase());
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT,0,ModelPtr->Model.NormBase());
	
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(4,GL_FLOAT,sizeof(vec4),ModelPtr->Model.TangentBase());

	if(ModelPtr->Use(CLUT,OGL_SkinManager::Normal)) {
		LoadModelSkin(SkinPtr->NormalImg, Collection, CLUT);
	}

	glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_NORMAL_ARRAY);

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
		glVertex3f(ModelPtr->Model.Positions[i*3+0] + -32*ModelPtr->Model.Normals[i*3+0],
				   ModelPtr->Model.Positions[i*3+1] + -32*ModelPtr->Model.Normals[i*3+1],
				   ModelPtr->Model.Positions[i*3+2] + -32*ModelPtr->Model.Normals[i*3+2]);
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

void RenderRasterize_Shader::render_node_object(render_object_data *object, bool other_side_of_media, RenderStep renderStep) {

	rectangle_definition& rect = object->rectangle;
	const world_point3d& pos = rect.Position;

	// To properly handle sprites in media, we render above and below
	// the media boundary in separate passes, just like the original
	// software renderer.
	short media_index = get_polygon_data(object->node->polygon_index)->media_index;
	media_data *media = (media_index != NONE) ? get_media_data(media_index) : NULL;
	if (media) {
		float h = media->height;
		GLdouble plane[] = { 0.0, 0.0, 1.0, -h };
		if (view->under_media_boundary ^ other_side_of_media) {
			plane[2] = -1.0;
			plane[3] = h;
		}
		glClipPlane(GL_CLIP_PLANE5, plane);
		glEnable(GL_CLIP_PLANE5);
	} else if (other_side_of_media) {
		// When there's no media present, we can skip the second pass.
		return;
	}
	
	if(rect.ModelPtr) {
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		glRotated((360.0/FULL_CIRCLE)*rect.Azimuth,0,0,1);
		GLfloat HorizScale = rect.Scale*rect.HorizScale;
		glScalef(HorizScale,HorizScale,rect.Scale);

		short descriptor = GET_DESCRIPTOR_COLLECTION(rect.ShapeDesc);
		short collection = GET_COLLECTION(descriptor);
		short clut = ModifyCLUT(rect.transfer_mode,GET_COLLECTION_CLUT(descriptor));

		RenderModel(rect, collection, clut, view->maximum_depth_intensity/float(FIXED_ONE_HALF), renderStep);
		glPopMatrix();
		glDisable(GL_CLIP_PLANE5);
		return;
	}

	glPushMatrix();
	glTranslated(pos.x, pos.y, pos.z);

	double yaw = view->yaw * 360.0 / float(NUMBER_OF_ANGLES);
	glRotated(yaw, 0.0, 0.0, 1.0);

	// look for parasitic objects based on y position,
	// and offset them to draw in proper depth order
	float offset = 0;
	if(pos.y == objectY) {
		objectCount++;
		offset = objectCount * -1.0;
	} else {
		objectCount = 0;
		objectY = pos.y;
	}

	TextureManager TMgr = setupSpriteTexture(rect, OGL_Txtr_Inhabitant, offset, renderStep);
	if (TMgr.ShapeDesc == UNONE) { return; }

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

	glTexCoord2f(texCoords[0][0], texCoords[1][0]);
	glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);

	glTexCoord2f(texCoords[0][0], texCoords[1][1]);
	glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);
		
 	glTexCoord2f(texCoords[0][1], texCoords[1][1]);
	glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);

	glTexCoord2f(texCoords[0][1], texCoords[1][0]);
	glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);

	glEnd();
	
	if (setupGlow(TMgr, 0, 1, offset, renderStep)) {
		glBegin(GL_QUADS);
		
		glTexCoord2f(texCoords[0][0], texCoords[1][0]);
		glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);
		
		glTexCoord2f(texCoords[0][0], texCoords[1][1]);
		glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldTop * rect.Scale);
		
		glTexCoord2f(texCoords[0][1], texCoords[1][1]);
		glVertex3f(0, rect.WorldRight * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);
		
		glTexCoord2f(texCoords[0][1], texCoords[1][0]);
		glVertex3f(0, rect.WorldLeft * rect.HorizScale * rect.Scale, rect.WorldBottom * rect.Scale);
		
		glEnd();
	}
	
	glPopMatrix();
	Shader::disable();
	TMgr.RestoreTextureMatrix();
	glDisable(GL_CLIP_PLANE5);
}

