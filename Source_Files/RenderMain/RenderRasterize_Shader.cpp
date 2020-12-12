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
#include "screen.h"

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

inline bool FogActive();

class Blur {

private:
	FBOSwapper _swapper;
	Shader *_shader_blur;
	Shader *_shader_bloom;
	GLuint _width;
	GLuint _height;

public:

	Blur(GLuint w, GLuint h, Shader* s_blur, Shader* s_bloom)
	: _swapper(w, h, Bloom_sRGB), _shader_blur(s_blur), _shader_bloom(s_bloom), _width(w), _height(h) {}

	GLuint width() { return _width; }
	GLuint height() { return _height; }
	
	void begin() {
		_swapper.activate();
		glDisable(GL_FRAMEBUFFER_SRGB_EXT); // don't blend for initial
	}

	void end() {
		_swapper.swap();
	}

	void draw(FBOSwapper& dest) {
		
		int passes = _shader_bloom->passes();
		if (passes < 0)
			passes = 5;

		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		for (int i = 0; i < passes; i++) {
			_shader_blur->enable();
			_shader_blur->setFloat(Shader::U_OffsetX, 1);
			_shader_blur->setFloat(Shader::U_OffsetY, 0);
			_shader_blur->setFloat(Shader::U_Pass, i + 1);
			_swapper.filter(false);

			_shader_blur->setFloat(Shader::U_OffsetX, 0);
			_shader_blur->setFloat(Shader::U_OffsetY, 1);
			_shader_blur->setFloat(Shader::U_Pass, i + 1);
			_swapper.filter(false);

			_shader_bloom->enable();
			_shader_bloom->setFloat(Shader::U_Pass, i + 1);
//			if (Bloom_sRGB)
//				dest.blend(_swapper.current_contents(), true);
//			else
				dest.blend_multisample(_swapper.current_contents());
			
			Shader::disable();
		}
		
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
};


RenderRasterize_Shader::RenderRasterize_Shader() = default;
RenderRasterize_Shader::~RenderRasterize_Shader() = default;

/*
 * initialize some stuff
 * happens once after opengl, shaders and textures are setup
 */
void RenderRasterize_Shader::setupGL(Rasterizer_Shader_Class& Rasterizer) {

	RasPtr = &Rasterizer;

	Shader::loadAll();

	Shader* s_blur = Shader::get(Shader::S_Blur);
	Shader* s_bloom = Shader::get(Shader::S_Bloom);

	blur.reset();
	if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_Blur)) {
		if(s_blur && s_bloom) {
			blur.reset(new Blur(640., 640. * graphics_preferences->screen_mode.height / graphics_preferences->screen_mode.width, s_blur, s_bloom));
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
const double TWO_PI = 8*atan(1.0);
const float FixedAngleToRadians = TWO_PI/(float(FIXED_ONE)*float(FULL_CIRCLE));
const float FixedAngleToDegrees = 360.0/(float(FIXED_ONE)*float(FULL_CIRCLE));

void RenderRasterize_Shader::render_tree() {

	weaponFlare = PIN(view->maximum_depth_intensity - NATURAL_LIGHT_INTENSITY, 0, FIXED_ONE)/float(FIXED_ONE);
	selfLuminosity = PIN(NATURAL_LIGHT_INTENSITY, 0, FIXED_ONE)/float(FIXED_ONE);

	Shader* s = Shader::get(Shader::S_Invincible);
	s->enable();
	s->setFloat(Shader::U_Time, view->tick_count);
	s->setFloat(Shader::U_LogicalWidth, view->screen_width);
	s->setFloat(Shader::U_LogicalHeight, view->screen_height);
	s->setFloat(Shader::U_PixelWidth, view->screen_width * MainScreenPixelScale());
	s->setFloat(Shader::U_PixelHeight, view->screen_height * MainScreenPixelScale());
	if (blur.get()) {
		s = Shader::get(Shader::S_InvincibleBloom);
		s->enable();
		s->setFloat(Shader::U_Time, view->tick_count);
		s->setFloat(Shader::U_LogicalWidth, view->screen_width);
		s->setFloat(Shader::U_LogicalHeight, view->screen_height);
		s->setFloat(Shader::U_PixelWidth, blur->width());
		s->setFloat(Shader::U_PixelHeight, blur->height());
	}

	short leftmost = INT16_MAX;
	short rightmost = INT16_MIN;
	vector<clipping_window_data>& windows = RSPtr->RVPtr->ClippingWindows;
	for (vector<clipping_window_data>::const_iterator it = windows.begin(); it != windows.end(); ++it) {
		if (it->x0 < leftmost) {
			leftmost = it->x0;
			leftmost_clip = it->left;
		}
		if (it->x1 > rightmost) {
			rightmost = it->x1;
			rightmost_clip = it->right;
		}
	}
	
	bool usefog = false;
	int fogtype;
	OGL_FogData *fogdata;
	if (TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Fog))
	{
		fogtype = (current_player->variables.flags&_HEAD_BELOW_MEDIA_BIT) ?
		OGL_Fog_BelowLiquid : OGL_Fog_AboveLiquid;
		fogdata = OGL_GetFogData(fogtype);
		if (fogdata && fogdata->IsPresent && fogdata->AffectsLandscapes) {
			usefog = true;
		}
	}
	const float virtual_yaw = view->virtual_yaw * FixedAngleToRadians;
	const float virtual_pitch = view->virtual_pitch * FixedAngleToRadians;
	s = Shader::get(Shader::S_Landscape);
	s->enable();
	s->setFloat(Shader::U_UseFog, usefog ? 1.0 : 0.0);
	s->setFloat(Shader::U_Yaw, virtual_yaw);
	s->setFloat(Shader::U_Pitch, view->mimic_sw_perspective ? 0.0 : virtual_pitch);
	s = Shader::get(Shader::S_LandscapeBloom);
	s->enable();
	s->setFloat(Shader::U_UseFog, usefog ? 1.0 : 0.0);
	s->setFloat(Shader::U_Yaw, virtual_yaw);
	s->setFloat(Shader::U_Pitch, view->mimic_sw_perspective ? 0.0 : virtual_pitch);
	s = Shader::get(Shader::S_LandscapeInfravision);
	s->enable();
	s->setFloat(Shader::U_UseFog, usefog ? 1.0 : 0.0);
	s->setFloat(Shader::U_Yaw, virtual_yaw);
	s->setFloat(Shader::U_Pitch, view->mimic_sw_perspective ? 0.0 :virtual_pitch);
	Shader::disable();

	RenderRasterizerClass::render_tree(kDiffuse);
        render_viewer_sprite_layer(kDiffuse);

	if (current_player->infravision_duration == 0 &&
		TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_Blur) &&
		blur.get())
	{
		blur->begin();
		RenderRasterizerClass::render_tree(kGlow);
                render_viewer_sprite_layer(kGlow);
		blur->end();
		RasPtr->swapper->deactivate();
		blur->draw(*RasPtr->swapper);
		RasPtr->swapper->activate();
	}

	glAlphaFunc(GL_GREATER, 0.5);
}

void RenderRasterize_Shader::render_node(sorted_node_data *node, bool SeeThruLiquids, RenderStep renderStep)
{
	// parasitic object detection
    objectCount = 0;
    objectY = 0;

    RenderRasterizerClass::render_node(node, SeeThruLiquids, renderStep);

	// turn off clipping planes
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
}

void RenderRasterize_Shader::clip_to_window(clipping_window_data *win)
{
    GLdouble clip[] = { 0., 0., 0., 0. };
        
    // recenter to player's orientation temporarily
    glPushMatrix();
    glTranslatef(view->origin.x, view->origin.y, 0.);
    glRotatef(view->yaw * (360/float(FULL_CIRCLE)) + 90., 0., 0., 1.);
    
    glRotatef(-0.1, 0., 0., 1.); // leave some excess to avoid artifacts at edges
	if (win->left.i != leftmost_clip.i || win->left.j != leftmost_clip.j) {
		clip[0] = win->left.i;
		clip[1] = win->left.j;
		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, clip);
	} else {
		glDisable(GL_CLIP_PLANE0);
	}
	
    glRotatef(0.2, 0., 0., 1.); // breathing room for right-hand clip
	if (win->right.i != rightmost_clip.i || win->right.j != rightmost_clip.j) {
		clip[0] = win->right.i;
		clip[1] = win->right.j;
		glEnable(GL_CLIP_PLANE1);
		glClipPlane(GL_CLIP_PLANE1, clip);
	} else {
		glDisable(GL_CLIP_PLANE1);
	}
    
    glPopMatrix();
}

void RenderRasterize_Shader::store_endpoint(
	endpoint_data *endpoint,
	long_vector2d& p)
{
	p.i = endpoint->vertex.x;
	p.j = endpoint->vertex.y;
}

std::unique_ptr<TextureManager> RenderRasterize_Shader::setupSpriteTexture(const rectangle_definition& rect, short type, float offset, RenderStep renderStep) {

	Shader *s = NULL;
	GLfloat color[3];
	GLdouble shade = PIN(static_cast<GLfloat>(rect.ambient_shade)/static_cast<GLfloat>(FIXED_ONE),0,1);
	color[0] = color[1] = color[2] = shade;

	auto TMgr = std::unique_ptr<TextureManager>(new TextureManager());

	TMgr->ShapeDesc = rect.ShapeDesc;
	TMgr->LowLevelShape = rect.LowLevelShape;
	TMgr->ShadingTables = rect.shading_tables;
	TMgr->Texture = rect.texture;
	TMgr->TransferMode = rect.transfer_mode;
	TMgr->TransferData = rect.transfer_data;
	TMgr->IsShadeless = (rect.flags&_SHADELESS_BIT) != 0;
	TMgr->TextureType = type;

	if (current_player->infravision_duration) {
		struct bitmap_definition* dummy;
		// grab the normal shading tables, since the shader does the tinting
		extended_get_shape_bitmap_and_shading_table(GET_DESCRIPTOR_COLLECTION(TMgr->ShapeDesc), TMgr->LowLevelShape, &dummy, &TMgr->ShadingTables, _shading_normal);
	}

	float flare = weaponFlare;

	glEnable(GL_TEXTURE_2D);

	// priorities: static, infravision, tinted/solid, shadeless
	if (TMgr->TransferMode == _static_transfer) {
		TMgr->IsShadeless = 1;
		flare = -1;
		if (renderStep == kDiffuse) {
			s = Shader::get(Shader::S_Invincible);
		} else {
			s = Shader::get(Shader::S_InvincibleBloom);
		}
		s->enable();
	} else if (current_player->infravision_duration) {
		color[0] = color[1] = color[2] = 1;
		FindInfravisionVersionRGBA(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(rect.ShapeDesc)), color);
		s = Shader::get(Shader::S_SpriteInfravision);
		s->enable();
	} else if (TMgr->TransferMode == _tinted_transfer) {
		flare = -1;
		if (renderStep == kDiffuse) {
			s = Shader::get(Shader::S_Invisible);
		} else {
			s = Shader::get(Shader::S_InvisibleBloom);
		}
		s->enable();
		s->setFloat(Shader::U_Visibility, 1.0 - rect.transfer_data/32.0f);
	} else if (TMgr->TransferMode == _solid_transfer) {
		// is this ever used?
		color[0] = 0;
		color[1] = 1;
		color[2] = 0;
	} else if (TMgr->TransferMode == _textured_transfer) {
		if (TMgr->IsShadeless) {
			if (renderStep == kDiffuse) {
				color[0] = color[1] = color[2] = 1;
			} else {
				color[0] = color[1] = color[2] = 0;
			}
			flare = -1;
		}
	} else {
		// I've never seen this happen
		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
	}

	if(s == NULL) {
		if (renderStep == kDiffuse) {
			s = Shader::get(Shader::S_Sprite);
		} else {
			s = Shader::get(Shader::S_SpriteBloom);
		}
		s->enable();
	}

	if(TMgr->Setup()) {
		TMgr->RenderNormal();
	} else {
		TMgr->ShapeDesc = UNONE;
		return TMgr;
	}

	TMgr->SetupTextureMatrix();

	if (renderStep == kGlow) {
		s->setFloat(Shader::U_BloomScale, TMgr->BloomScale());
		s->setFloat(Shader::U_BloomShift, TMgr->BloomShift());
	}
	s->setFloat(Shader::U_Flare, flare);
	s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
	s->setFloat(Shader::U_Pulsate, 0);
	s->setFloat(Shader::U_Wobble, 0);
	s->setFloat(Shader::U_Depth, offset);
	s->setFloat(Shader::U_StrictDepthMode, OGL_ForceSpriteDepth() ? 1 : 0);
	s->setFloat(Shader::U_Glow, 0);
	glColor4f(color[0], color[1], color[2], 1);
	return TMgr;
}

// Circle constants
const double Radian2Circle = 1/TWO_PI;			// A circle is 2*pi radians
const double FullCircleReciprocal = 1/double(FULL_CIRCLE);

std::unique_ptr<TextureManager> RenderRasterize_Shader::setupWallTexture(const shape_descriptor& Texture, short transferMode, float pulsate, float wobble, float intensity, float offset, RenderStep renderStep) {

	Shader *s = NULL;

	auto TMgr = std::unique_ptr<TextureManager>(new TextureManager());
	LandscapeOptions *opts = NULL;
	TMgr->ShapeDesc = Texture;
	if (TMgr->ShapeDesc == UNONE) { return TMgr; }
	get_shape_bitmap_and_shading_table(Texture, &TMgr->Texture, &TMgr->ShadingTables, _shading_normal);

	TMgr->TransferMode = _textured_transfer;
	TMgr->IsShadeless = current_player->infravision_duration ? 1 : 0;
	TMgr->TransferData = 0;

	float flare = weaponFlare;

	glEnable(GL_TEXTURE_2D);
	glColor4f(intensity, intensity, intensity, 1.0);

	switch(transferMode) {
		case _xfer_static:
			TMgr->TextureType = OGL_Txtr_Wall;
			TMgr->TransferMode = _static_transfer;
			TMgr->IsShadeless = 1;
			flare = -1;
			s = Shader::get(renderStep == kGlow ? Shader::S_InvincibleBloom : Shader::S_Invincible);
			s->enable();
			break;
		case _xfer_landscape:
		case _xfer_big_landscape:
			TMgr->TextureType = OGL_Txtr_Landscape;
			TMgr->TransferMode = _big_landscaped_transfer;
			opts = View_GetLandscapeOptions(Texture);
			TMgr->LandscapeVertRepeat = opts->VertRepeat;
			TMgr->Landscape_AspRatExp = opts->OGL_AspRatExp;
			if (current_player->infravision_duration) {
				GLfloat color[3] {1, 1, 1};
				FindInfravisionVersionRGBA(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(Texture)), color);
				glColor4f(color[0], color[1], color[2], 1);
				s = Shader::get(Shader::S_LandscapeInfravision);
			} else {
				if (renderStep == kDiffuse) {
					s = Shader::get(Shader::S_Landscape);
				} else {
					s = Shader::get(Shader::S_LandscapeBloom);
				}
			}
			s->enable();
			break;
		default:
			TMgr->TextureType = OGL_Txtr_Wall;
			if(TMgr->IsShadeless) {
				if (renderStep == kDiffuse) {
					glColor4f(1,1,1,1);
				} else {
					glColor4f(0,0,0,1);
				}
				flare = -1;
			}
	}

	if(s == NULL) {
		if (current_player->infravision_duration) {
			GLfloat color[3] {1, 1, 1};
			FindInfravisionVersionRGBA(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(Texture)), color);
			glColor4f(color[0], color[1], color[2], 1);
			s = Shader::get(Shader::S_WallInfravision);
		} else if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
			s = Shader::get(renderStep == kGlow ? Shader::S_BumpBloom : Shader::S_Bump);
		} else {
			s = Shader::get(renderStep == kGlow ? Shader::S_WallBloom : Shader::S_Wall);
		}
		s->enable();
	}

	if(TMgr->Setup()) {
		TMgr->RenderNormal(); // must allocate first
		if (TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
			glActiveTextureARB(GL_TEXTURE1_ARB);
			TMgr->RenderBump();
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
	} else {
		TMgr->ShapeDesc = UNONE;
		return TMgr;
	}

	TMgr->SetupTextureMatrix();
	
	if (TMgr->TextureType == OGL_Txtr_Landscape && opts) {
		double TexScale = ABS(TMgr->U_Scale);
		double HorizScale = double(1 << opts->HorizExp);
		s->setFloat(Shader::U_ScaleX, HorizScale * (npotTextures ? 1.0 : TexScale) * Radian2Circle);
		s->setFloat(Shader::U_OffsetX, HorizScale * (0.25 + opts->Azimuth * FullCircleReciprocal));
		
		short AdjustedVertExp = opts->VertExp + opts->OGL_AspRatExp;
		double VertScale = (AdjustedVertExp >= 0) ? double(1 << AdjustedVertExp)
		                                          : 1/double(1 << (-AdjustedVertExp));
		s->setFloat(Shader::U_ScaleY, VertScale * TexScale * Radian2Circle);
		s->setFloat(Shader::U_OffsetY, (0.5 + TMgr->U_Offset) * TexScale);
	}

	if (renderStep == kGlow) {
		if (TMgr->TextureType == OGL_Txtr_Landscape) {
			s->setFloat(Shader::U_BloomScale, TMgr->LandscapeBloom());
		} else {
			s->setFloat(Shader::U_BloomScale, TMgr->BloomScale());
			s->setFloat(Shader::U_BloomShift, TMgr->BloomShift());
		}
	}
	s->setFloat(Shader::U_Flare, flare);
	s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
	s->setFloat(Shader::U_Pulsate, pulsate);
	s->setFloat(Shader::U_Wobble, wobble);
	s->setFloat(Shader::U_Depth, offset);
	s->setFloat(Shader::U_Glow, 0);
	return TMgr;
}

void instantiate_transfer_mode(struct view_data *view, short transfer_mode, world_distance &x0, world_distance &y0) {
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
			wobble = transfer_phase / 1024.0;
			break;
	}
	return wobble;
}

void setupBlendFunc(short blendType) {
	switch(blendType)
	{
		case OGL_BlendType_Crossfade:
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case OGL_BlendType_Add:
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			break;
		case OGL_BlendType_Crossfade_Premult:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case OGL_BlendType_Add_Premult:
			glBlendFunc(GL_ONE, GL_ONE);
			break;
	}
}

bool setupGlow(struct view_data *view, std::unique_ptr<TextureManager>& TMgr, float wobble, float intensity, float flare, float selfLuminosity, float offset, RenderStep renderStep) {
	if (TMgr->TransferMode == _textured_transfer && TMgr->IsGlowMapped()) {
		Shader *s = NULL;
		if (TMgr->TextureType == OGL_Txtr_Wall) {
			if (TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
				s = Shader::get(renderStep == kGlow ? Shader::S_BumpBloom : Shader::S_Bump);
			} else {
				s = Shader::get(renderStep == kGlow ? Shader::S_WallBloom : Shader::S_Wall);
			}
		} else {
			s = Shader::get(renderStep == kGlow ? Shader::S_SpriteBloom : Shader::S_Sprite);
		}

		TMgr->RenderGlowing();
		setupBlendFunc(TMgr->GlowBlend());
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);

		s->enable();
		if (renderStep == kGlow) {
			s->setFloat(Shader::U_BloomScale, TMgr->GlowBloomScale());
			s->setFloat(Shader::U_BloomShift, TMgr->GlowBloomShift());
		}
		s->setFloat(Shader::U_Flare, flare);
		s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
		s->setFloat(Shader::U_Wobble, wobble);
		s->setFloat(Shader::U_Depth, offset - 1.0);
		s->setFloat(Shader::U_Glow, TMgr->MinGlowIntensity());
		return true;
	}
	return false;
}

void RenderRasterize_Shader::render_node_floor_or_ceiling(clipping_window_data *window,
	polygon_data *polygon, horizontal_surface_data *surface, bool void_present, bool ceil, RenderStep renderStep) {

	float offset = 0;

	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture);
	float intensity = get_light_intensity(surface->lightsource_index) / float(FIXED_ONE - 1);
	float wobble = calcWobble(surface->transfer_mode, view->tick_count);
	// note: wobble and pulsate behave the same way on floors and ceilings
	// note 2: stronger wobble looks more like classic with default shaders
	auto TMgr = setupWallTexture(texture, surface->transfer_mode, wobble * 4.0, 0, intensity, offset, renderStep);
	if(TMgr->ShapeDesc == UNONE) { return; }

	if (TMgr->IsBlended()) {
		glEnable(GL_BLEND);
		setupBlendFunc(TMgr->NormalBlend());
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);
	} else {
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5);
	}

	if (void_present && TMgr->IsBlended()) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}

	short vertex_count = polygon->vertex_count;

	if (vertex_count) {
        clip_to_window(window);

		world_distance x = 0.0, y = 0.0;
		instantiate_transfer_mode(view, surface->transfer_mode, x, y);

		vec3 N;
		vec3 T;
		float sign;
		if(ceil) {
			N = vec3(0,0,-1);
			T = vec3(0,1,0);
			sign = 1;
		} else {
			N = vec3(0,0,1);
			T = vec3(0,1,0);
			sign = -1;
		}
		glNormal3f(N[0], N[1], N[2]);
		glMultiTexCoord4fARB(GL_TEXTURE1_ARB, T[0], T[1], T[2], sign);

		GLfloat vertex_array[MAXIMUM_VERTICES_PER_POLYGON * 3];
		GLfloat texcoord_array[MAXIMUM_VERTICES_PER_POLYGON * 2];

		GLfloat* vp = vertex_array;
		GLfloat* tp = texcoord_array;
		if (ceil)
		{
			for(short i = 0; i < vertex_count; ++i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[vertex_count - 1 - i])->vertex;
				*vp++ = vertex.x;
				*vp++ = vertex.y;
				*vp++ = surface->height;
				*tp++ = (vertex.x + surface->origin.x + x) / float(WORLD_ONE);
				*tp++ = (vertex.y + surface->origin.y + y) / float(WORLD_ONE);
			}
		}
		else
		{
			for(short i=0; i<vertex_count; ++i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				*vp++ = vertex.x;
				*vp++ = vertex.y;
				*vp++ = surface->height;
				*tp++ = (vertex.x + surface->origin.x + x) / float(WORLD_ONE);
				*tp++ = (vertex.y + surface->origin.y + y) / float(WORLD_ONE);
			}
		}
		glVertexPointer(3, GL_FLOAT, 0, vertex_array);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);

		glDrawArrays(GL_POLYGON, 0, vertex_count);

		// see note 2 above; pulsate uniform should stay set from setupWall call
		if (setupGlow(view, TMgr, 0, intensity, weaponFlare, selfLuminosity, offset, renderStep)) {
			glDrawArrays(GL_POLYGON, 0, vertex_count);
		}

		Shader::disable();
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
}

void RenderRasterize_Shader::render_node_side(clipping_window_data *window, vertical_surface_data *surface, bool void_present, RenderStep renderStep) {

	float offset = 0;
	if (!void_present) {
		offset = -2.0;
	}

	const shape_descriptor& texture = AnimTxtr_Translate(surface->texture_definition->texture);
	float intensity = (get_light_intensity(surface->lightsource_index) + surface->ambient_delta) / float(FIXED_ONE - 1);
	float wobble = calcWobble(surface->transfer_mode, view->tick_count);
	float pulsate = 0;
	if (surface->transfer_mode == _xfer_pulsate) {
		pulsate = wobble;
		wobble = 0;
	}
	auto TMgr = setupWallTexture(texture, surface->transfer_mode, pulsate, wobble, intensity, offset, renderStep);
	if(TMgr->ShapeDesc == UNONE) { return; }

	if (TMgr->IsBlended()) {
		glEnable(GL_BLEND);
		setupBlendFunc(TMgr->NormalBlend());
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);
	} else {
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5);
	}

	if (void_present && TMgr->IsBlended()) {
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
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

		if (vertex_count) {
            clip_to_window(window);

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

			vec3 N(-dy, dx, 0);
			vec3 T(dx, dy, 0);
			float sign = 1;
			glNormal3f(N[0], N[1], N[2]);
			glMultiTexCoord4fARB(GL_TEXTURE1_ARB, T[0], T[1], T[2], sign);

			world_distance x = 0.0, y = 0.0;
			instantiate_transfer_mode(view, surface->transfer_mode, x, y);

			x0 -= x;
			tOffset -= y;

			GLfloat vertex_array[12];
			GLfloat texcoord_array[8];

			GLfloat* vp = vertex_array;
			GLfloat* tp = texcoord_array;

			for(int i = 0; i < vertex_count; ++i) {
				float p2 = 0;
				if(i == 1 || i == 2) { p2 = surface->length; }

				*vp++ = vertices[i].x;
				*vp++ = vertices[i].y;
				*vp++ = vertices[i].z;
				*tp++ = (tOffset - vertices[i].z) / div;
				*tp++ = (x0+p2) / div;
			}
			glVertexPointer(3, GL_FLOAT, 0, vertex_array);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);
			
			glDrawArrays(GL_QUADS, 0, vertex_count);

			if (setupGlow(view, TMgr, wobble, intensity, weaponFlare, selfLuminosity, offset, renderStep)) {
				glDrawArrays(GL_QUADS, 0, vertex_count);
			}

			Shader::disable();
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}
}

extern void FlatBumpTexture(); // from OGL_Textures.cpp

bool RenderModel(rectangle_definition& RenderRectangle, short Collection, short CLUT, float flare, float selfLuminosity, RenderStep renderStep) {

	OGL_ModelData *ModelPtr = RenderRectangle.ModelPtr;
	OGL_SkinData *SkinPtr = ModelPtr->GetSkin(CLUT);
	if(!SkinPtr) { return false; }

	if (ModelPtr->Sidedness < 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	} else if (ModelPtr->Sidedness > 0) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	glEnable(GL_TEXTURE_2D);
	if (SkinPtr->OpacityType != OGL_OpacType_Crisp || RenderRectangle.transfer_mode == _tinted_transfer) {
		glEnable(GL_BLEND);
		setupBlendFunc(SkinPtr->NormalBlend);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);
	} else {
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5);
	}

	GLfloat color[3];
	GLdouble shade = PIN(static_cast<GLfloat>(RenderRectangle.ambient_shade)/static_cast<GLfloat>(FIXED_ONE),0,1);
	color[0] = color[1] = color[2] = shade;

	Shader *s = NULL;
	bool canGlow = false;
	if (RenderRectangle.transfer_mode == _static_transfer) {
		flare = -1;
		if (renderStep == kDiffuse) {
			s = Shader::get(Shader::S_Invincible);
		} else {
			s = Shader::get(Shader::S_InvincibleBloom);
		}
	} else if (current_player->infravision_duration) {
		color[0] = color[1] = color[2] = 1;
		FindInfravisionVersionRGBA(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(RenderRectangle.ShapeDesc)), color);
		s = Shader::get(Shader::S_WallInfravision);
	} else if (RenderRectangle.transfer_mode == _tinted_transfer) {
			flare = -1;
			if (renderStep == kDiffuse) {
				s = Shader::get(Shader::S_Invisible);
			} else {
				s = Shader::get(Shader::S_InvisibleBloom);
			}
			s->enable();
			s->setFloat(Shader::U_Visibility, 1.0 - RenderRectangle.transfer_data/32.0f);
	} else if (RenderRectangle.transfer_mode == _solid_transfer) {
		color[0] = 0;
		color[1] = 1;
		color[2] = 0;
	} else if (RenderRectangle.transfer_mode == _textured_transfer) {
		if (RenderRectangle.flags & _SHADELESS_BIT) {
			if (renderStep == kDiffuse) {
				color[0] = color[1] = color[2] = 1;
			} else {
				color[0] = color[1] = color[2] = 0;
			}
			flare = -1;
		} else {
			canGlow = true;
		}
	} else {
		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
	}

	if(s == NULL) {
		if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
			s = Shader::get(renderStep == kGlow ? Shader::S_BumpBloom : Shader::S_Bump);
		} else {
			s = Shader::get(renderStep == kGlow ? Shader::S_WallBloom : Shader::S_Wall);
		}
		s->enable();
	}

	if (renderStep == kGlow) {
		s->setFloat(Shader::U_BloomScale, SkinPtr->BloomScale);
		s->setFloat(Shader::U_BloomShift, SkinPtr->BloomShift);
	}
	s->setFloat(Shader::U_Flare, flare);
	s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
	s->setFloat(Shader::U_Wobble, 0);
	s->setFloat(Shader::U_Depth, 0);
	s->setFloat(Shader::U_Glow, 0);
	glColor4f(color[0], color[1], color[2], 1);

	glVertexPointer(3,GL_FLOAT,0,ModelPtr->Model.PosBase());
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	if (ModelPtr->Model.TxtrCoords.empty()) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glTexCoordPointer(2,GL_FLOAT,0,ModelPtr->Model.TCBase());
	}

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT,0,ModelPtr->Model.NormBase());

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(4,GL_FLOAT,sizeof(vec4),ModelPtr->Model.TangentBase());

	if(ModelPtr->Use(CLUT,OGL_SkinManager::Normal)) {
		LoadModelSkin(SkinPtr->NormalImg, Collection, CLUT);
	}

	if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
		glActiveTextureARB(GL_TEXTURE1_ARB);
		if(ModelPtr->Use(CLUT,OGL_SkinManager::Bump)) {
			LoadModelSkin(SkinPtr->OffsetImg, Collection, CLUT);
		}
		if (!SkinPtr->OffsetImg.IsPresent()) {
			FlatBumpTexture();
		}
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());

	if (canGlow && SkinPtr->GlowImg.IsPresent()) {
		glEnable(GL_BLEND);
		setupBlendFunc(SkinPtr->GlowBlend);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);

		s->enable();
		s->setFloat(Shader::U_Glow, SkinPtr->MinGlowIntensity);
		if (renderStep == kGlow) {
			s->setFloat(Shader::U_BloomScale, SkinPtr->GlowBloomScale);
			s->setFloat(Shader::U_BloomShift, SkinPtr->GlowBloomShift);
		}

		if(ModelPtr->Use(CLUT,OGL_SkinManager::Glowing)) {
			LoadModelSkin(SkinPtr->GlowImg, Collection, CLUT);
		}
		glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());
	}

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	if (ModelPtr->Model.TxtrCoords.empty()) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// Restore the default render sidedness
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	Shader::disable();
	return true;
}

void RenderRasterize_Shader::render_node_object(render_object_data *object, bool other_side_of_media, RenderStep renderStep) {

    if (!object->clipping_windows)
        return;

	clipping_window_data *win;

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

    for (win = object->clipping_windows; win; win = win->next_window)
    {
        clip_to_window(win);
        _render_node_object_helper(object, renderStep);
    }
    
    glDisable(GL_CLIP_PLANE5);
}

void RenderRasterize_Shader::_render_node_object_helper(render_object_data *object, RenderStep renderStep) {

	rectangle_definition& rect = object->rectangle;
	const world_point3d& pos = rect.Position;
    
	if(rect.ModelPtr) {
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		glRotated((360.0/FULL_CIRCLE)*rect.Azimuth,0,0,1);
		GLfloat HorizScale = rect.Scale*rect.HorizScale;
		glScalef(HorizScale,HorizScale,rect.Scale);

		short descriptor = GET_DESCRIPTOR_COLLECTION(rect.ShapeDesc);
		short collection = GET_COLLECTION(descriptor);
		short clut = ModifyCLUT(rect.transfer_mode,GET_COLLECTION_CLUT(descriptor));

		RenderModel(rect, collection, clut, weaponFlare, selfLuminosity, renderStep);
		glPopMatrix();
		return;
	}

	glPushMatrix();
	glTranslated(pos.x, pos.y, pos.z);

	double yaw = view->virtual_yaw * FixedAngleToDegrees;
	glRotated(yaw, 0.0, 0.0, 1.0);

	float offset = 0;
	if (OGL_ForceSpriteDepth()) {
		// look for parasitic objects based on y position,
		// and offset them to draw in proper depth order
		if(pos.y == objectY) {
			objectCount++;
			offset = objectCount * -1.0;
		} else {
			objectCount = 0;
			objectY = pos.y;
		}
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	auto TMgr = setupSpriteTexture(rect, OGL_Txtr_Inhabitant, offset, renderStep);
	if (TMgr->ShapeDesc == UNONE) { glPopMatrix(); return; }

	float texCoords[2][2];

	if(rect.flip_vertical) {
		texCoords[0][1] = TMgr->U_Offset;
		texCoords[0][0] = TMgr->U_Scale+TMgr->U_Offset;
	} else {
		texCoords[0][0] = TMgr->U_Offset;
		texCoords[0][1] = TMgr->U_Scale+TMgr->U_Offset;
	}

	if(rect.flip_horizontal) {
		texCoords[1][1] = TMgr->V_Offset;
		texCoords[1][0] = TMgr->V_Scale+TMgr->V_Offset;
	} else {
		texCoords[1][0] = TMgr->V_Offset;
		texCoords[1][1] = TMgr->V_Scale+TMgr->V_Offset;
	}

	if(TMgr->IsBlended() || TMgr->TransferMode == _tinted_transfer) {
		glEnable(GL_BLEND);
		setupBlendFunc(TMgr->NormalBlend());
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);
	} else {
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5);
	}

	GLfloat vertex_array[12] = {
		0,
		rect.WorldLeft * rect.HorizScale * rect.Scale,
		rect.WorldTop * rect.Scale,
		0,
		rect.WorldRight * rect.HorizScale * rect.Scale,
		rect.WorldTop * rect.Scale,
		0,
		rect.WorldRight * rect.HorizScale * rect.Scale,
		rect.WorldBottom * rect.Scale,
		0,
		rect.WorldLeft * rect.HorizScale * rect.Scale,
		rect.WorldBottom * rect.Scale
	};

	GLfloat texcoord_array[8] = {
		texCoords[0][0],
		texCoords[1][0],
		texCoords[0][0],
		texCoords[1][1],
		texCoords[0][1],
		texCoords[1][1],
		texCoords[0][1],
		texCoords[1][0]
	};

	glVertexPointer(3, GL_FLOAT, 0, vertex_array);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);

	glDrawArrays(GL_QUADS, 0, 4);

	if (setupGlow(view, TMgr, 0, 1, weaponFlare, selfLuminosity, offset, renderStep)) {
		glDrawArrays(GL_QUADS, 0, 4);
	}
        
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	Shader::disable();
	TMgr->RestoreTextureMatrix();
}

extern void position_sprite_axis(short *x0, short *x1, short scale_width, short screen_width, short positioning_mode, _fixed position, bool flip, world_distance world_left, world_distance world_right);

extern GLdouble Screen_2_Clip[16];

void RenderRasterize_Shader::render_viewer_sprite_layer(RenderStep renderStep)
{
        if (!view->show_weapons_in_hand) return;
    
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
    
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadMatrixd(Screen_2_Clip);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        rectangle_definition rect;
	weapon_display_information display_data;
	shape_information_data *shape_information;
	short count;

        rect.ModelPtr = nullptr;
        rect.Opacity = 1;

        /* get_weapon_display_information() returns true if there is a weapon to be drawn.  it
           should initially be passed a count of zero.  it returns the weaponÕs texture and
           enough information to draw it correctly. */
	count= 0;
	while (get_weapon_display_information(&count, &display_data))
	{
		/* fetch relevant shape data */
                shape_information= extended_get_shape_information(display_data.collection, display_data.low_level_shape_index);

                // Nonexistent frame: skip
		if (!shape_information) continue;
		
		// LP change: for the convenience of the OpenGL renderer
		rect.ShapeDesc = BUILD_DESCRIPTOR(display_data.collection,0);
		rect.LowLevelShape = display_data.low_level_shape_index;

		if (shape_information->flags&_X_MIRRORED_BIT) display_data.flip_horizontal= !display_data.flip_horizontal;
		if (shape_information->flags&_Y_MIRRORED_BIT) display_data.flip_vertical= !display_data.flip_vertical;

		/* calculate shape rectangle */
		position_sprite_axis(&rect.x0, &rect.x1, view->screen_height, view->screen_width, display_data.horizontal_positioning_mode,
			display_data.horizontal_position, display_data.flip_horizontal, shape_information->world_left, shape_information->world_right);
		position_sprite_axis(&rect.y0, &rect.y1, view->screen_height, view->screen_height, display_data.vertical_positioning_mode,
			display_data.vertical_position, display_data.flip_vertical, -shape_information->world_top, -shape_information->world_bottom);
		
		/* set rectangle bitmap and shading table */
		extended_get_shape_bitmap_and_shading_table(display_data.collection, display_data.low_level_shape_index, &rect.texture, &rect.shading_tables, view->shading_mode);
		if (!rect.texture) continue;
		
		rect.flags= 0;

		/* initialize clipping window to full screen */
		rect.clip_left= 0;
		rect.clip_right= view->screen_width;
		rect.clip_top= 0;
		rect.clip_bottom= view->screen_height;

		/* copy mirror flags */
		rect.flip_horizontal= display_data.flip_horizontal;
		rect.flip_vertical= display_data.flip_vertical;
		
		/* lighting: depth of zero in the cameraÕs polygon index */
		rect.depth= 0;
		rect.ambient_shade= get_light_intensity(get_polygon_data(view->origin_polygon_index)->floor_lightsource_index);
		rect.ambient_shade= MAX(shape_information->minimum_light_intensity, rect.ambient_shade);
		if (view->shading_mode==_shading_infravision) rect.flags|= _SHADELESS_BIT;

		// Calculate the object's horizontal position
		// for the convenience of doing teleport-in/teleport-out
		rect.xc = (rect.x0 + rect.x1) >> 1;

                /* make the weapon reflect the ownerÕs transfer mode */
		instantiate_rectangle_transfer_mode(view, &rect, display_data.transfer_mode, display_data.transfer_phase);

                render_viewer_sprite(rect, renderStep);
        }

        Shader::disable();
    
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
    
        glMatrixMode(GL_MODELVIEW);
}

struct ExtendedVertexData
{
	GLdouble Vertex[4];
	GLdouble TexCoord[2];
	GLfloat Color[3];
	GLfloat GlowColor[3];
};

void RenderRasterize_Shader::render_viewer_sprite(rectangle_definition& RenderRectangle, RenderStep renderStep)
{
	// Find texture coordinates
	ExtendedVertexData ExtendedVertexList[4];
	
	point2d TopLeft, BottomRight;
	// Clipped corners:
	TopLeft.x = MAX(RenderRectangle.x0,RenderRectangle.clip_left);
	TopLeft.y = MAX(RenderRectangle.y0,RenderRectangle.clip_top);
	BottomRight.x = MIN(RenderRectangle.x1,RenderRectangle.clip_right);
	BottomRight.y = MIN(RenderRectangle.y1,RenderRectangle.clip_bottom);
	
        // Screen coordinates; weapons-in-hand are in the foreground
        ExtendedVertexList[0].Vertex[0] = TopLeft.x;
        ExtendedVertexList[0].Vertex[1] = TopLeft.y;
        ExtendedVertexList[0].Vertex[2] = 1;
        ExtendedVertexList[2].Vertex[0] = BottomRight.x;
        ExtendedVertexList[2].Vertex[1] = BottomRight.y;
        ExtendedVertexList[2].Vertex[2] = 1;
	
	// Completely clipped away?
	if (BottomRight.x <= TopLeft.x) return;
	if (BottomRight.y <= TopLeft.y) return;
	
	// Use that texture
	auto TMgr = setupSpriteTexture(RenderRectangle, OGL_Txtr_WeaponsInHand, 0, renderStep);
	
	// Calculate the texture coordinates;
	// the scanline direction is downward, (texture coordinate 0)
	// while the line-to-line direction is rightward (texture coordinate 1)
	GLdouble U_Scale = TMgr->U_Scale/(RenderRectangle.y1 - RenderRectangle.y0);
	GLdouble V_Scale = TMgr->V_Scale/(RenderRectangle.x1 - RenderRectangle.x0);
	GLdouble U_Offset = TMgr->U_Offset;
	GLdouble V_Offset = TMgr->V_Offset;
	
	if (RenderRectangle.flip_vertical)
	{
		ExtendedVertexList[0].TexCoord[0] = U_Offset + U_Scale*(RenderRectangle.y1 - TopLeft.y);
		ExtendedVertexList[2].TexCoord[0] = U_Offset + U_Scale*(RenderRectangle.y1 - BottomRight.y);
	} else {
		ExtendedVertexList[0].TexCoord[0] = U_Offset + U_Scale*(TopLeft.y - RenderRectangle.y0);
		ExtendedVertexList[2].TexCoord[0] = U_Offset + U_Scale*(BottomRight.y - RenderRectangle.y0);
	}
	if (RenderRectangle.flip_horizontal)
	{
		ExtendedVertexList[0].TexCoord[1] = V_Offset + V_Scale*(RenderRectangle.x1 - TopLeft.x);
		ExtendedVertexList[2].TexCoord[1] = V_Offset + V_Scale*(RenderRectangle.x1 - BottomRight.x);
	} else {
		ExtendedVertexList[0].TexCoord[1] = V_Offset + V_Scale*(TopLeft.x - RenderRectangle.x0);
		ExtendedVertexList[2].TexCoord[1] = V_Offset + V_Scale*(BottomRight.x - RenderRectangle.x0);
	}
	
	// Fill in remaining points
	// Be sure that the order gives a sidedness the same as
	// that of the world-geometry polygons
	ExtendedVertexList[1].Vertex[0] = ExtendedVertexList[2].Vertex[0];
	ExtendedVertexList[1].Vertex[1] = ExtendedVertexList[0].Vertex[1];
	ExtendedVertexList[1].Vertex[2] = ExtendedVertexList[0].Vertex[2];
	ExtendedVertexList[1].TexCoord[0] = ExtendedVertexList[0].TexCoord[0];
	ExtendedVertexList[1].TexCoord[1] = ExtendedVertexList[2].TexCoord[1];
	ExtendedVertexList[3].Vertex[0] = ExtendedVertexList[0].Vertex[0];
	ExtendedVertexList[3].Vertex[1] = ExtendedVertexList[2].Vertex[1];
	ExtendedVertexList[3].Vertex[2] = ExtendedVertexList[2].Vertex[2];
	ExtendedVertexList[3].TexCoord[0] = ExtendedVertexList[2].TexCoord[0];
	ExtendedVertexList[3].TexCoord[1] = ExtendedVertexList[0].TexCoord[1];

        if(TMgr->IsBlended() || TMgr->TransferMode == _tinted_transfer) {
		glEnable(GL_BLEND);
		setupBlendFunc(TMgr->NormalBlend());
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.001);
	} else {
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.5);
	}

        glDisable(GL_DEPTH_TEST);

	// Location of data:
	glVertexPointer(3,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].Vertex);
	glTexCoordPointer(2,GL_DOUBLE,sizeof(ExtendedVertexData),ExtendedVertexList[0].TexCoord);
	glEnable(GL_TEXTURE_2D);
		
	// Go!
        glDrawArrays(GL_POLYGON,0,4);

        if (setupGlow(view, TMgr, 0, 1, weaponFlare, selfLuminosity, 0, renderStep)) {
            glDrawArrays(GL_QUADS, 0, 4);
	}
	
	glEnable(GL_DEPTH_TEST);
        Shader::disable();
	TMgr->RestoreTextureMatrix();

}
