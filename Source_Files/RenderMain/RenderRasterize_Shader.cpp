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
#include "MatrixStack.hpp"
#include "DrawCache.hpp"

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

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
        
        GLfloat modelProjection[16];
        MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);

        GLfloat mainColorBufferSize[2], blurBufferSize[2];
        mainColorBufferSize[0]=dest.current_contents()._w ;
        mainColorBufferSize[1]=dest.current_contents()._h;
        
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		for (int i = 0; i < passes; i++) {
            
            blurBufferSize[0]=_swapper.current_contents()._w;
            blurBufferSize[1]=_swapper.current_contents()._h;

			_shader_blur->enable();
            _shader_blur->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
            _shader_blur->setVec2(Shader::U_Texture0_Size, blurBufferSize);
			_shader_blur->setFloat(Shader::U_OffsetX, 1);
			_shader_blur->setFloat(Shader::U_OffsetY, 0);
			_shader_blur->setFloat(Shader::U_Pass, i + 1);
			_swapper.filter(false);

            _shader_blur->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
            _shader_blur->setVec2(Shader::U_Texture0_Size, blurBufferSize);
			_shader_blur->setFloat(Shader::U_OffsetX, 0);
			_shader_blur->setFloat(Shader::U_OffsetY, 1);
			_shader_blur->setFloat(Shader::U_Pass, i + 1);
			_swapper.filter(false);
            
            _shader_bloom->enable();
            _shader_bloom->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
            _shader_bloom->setVec2(Shader::U_Texture0_Size, mainColorBufferSize);
            _shader_bloom->setVec2(Shader::U_Texture1_Size, blurBufferSize);
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
	
	float fogMix = 0.0;
	auto fogdata = OGL_GetCurrFogData();
		if (fogdata && fogdata->IsPresent && fogdata->AffectsLandscapes) {
			fogMix = fogdata->LandscapeMix;
	}

	float fogmode = -1.0;
	if (fogdata) {
		fogmode = fogdata->Mode;
	}

	const float virtual_yaw = view->virtual_yaw * FixedAngleToRadians;
	const float virtual_pitch = view->virtual_pitch * FixedAngleToRadians;
	
	Shader* landscape_shaders[] = {
		Shader::get(Shader::S_Landscape),
		Shader::get(Shader::S_LandscapeBloom),
		Shader::get(Shader::S_LandscapeInfravision)
	};

	for (auto s : landscape_shaders) {
		s->enable();
		s->setFloat(Shader::U_FogMix, fogMix);
		s->setFloat(Shader::U_Yaw, virtual_yaw);
		s->setFloat(Shader::U_Pitch, view->mimic_sw_perspective ? 0.0 : virtual_pitch);
	}
	
		//May not be needed after a loadAll
	Shader* fog_mode_shaders[] = {
			Shader::get(Shader::S_Bump),
			Shader::get(Shader::S_BumpBloom),
			Shader::get(Shader::S_Invincible),
			Shader::get(Shader::S_InvincibleBloom),
			Shader::get(Shader::S_Invisible),
			Shader::get(Shader::S_InvisibleBloom),
			Shader::get(Shader::S_Wall),
			Shader::get(Shader::S_WallBloom),
			Shader::get(Shader::S_WallInfravision),
			Shader::get(Shader::S_Sprite),
			Shader::get(Shader::S_SpriteBloom),
			Shader::get(Shader::S_SpriteInfravision)
	};

	for (auto s : fog_mode_shaders) {
		s->enable();
		s->setFloat(Shader::U_FogMode, fogmode);
	}
	
	Shader::disable();

    //Set standard uniforms for all applicable shaders.
    Shader::get(Shader::S_Wall)->enableAndSetStandardUniforms();
    Shader::get(Shader::S_Landscape)->enableAndSetStandardUniforms();
    Shader::get(Shader::S_WallBloom)->enableAndSetStandardUniforms();
    Shader::get(Shader::S_WallInfravision)->enableAndSetStandardUniforms();
    Shader::get(Shader::S_Bump)->enableAndSetStandardUniforms();
    Shader::get(Shader::S_BumpBloom)->enableAndSetStandardUniforms();
    Shader::disable();

        //Buffer test for for ripple, SSAO, SSR. DO NOT USE. Just a demo.
    if(0) { //Should be a prefences check later
        if (colorDepthSansMedia._h == 0 && colorDepthSansMedia._w == 0) {
          GLint viewPort[4];
          glGetIntegerv(GL_VIEWPORT, viewPort);
          colorDepthSansMedia.setup(viewPort[2], viewPort[3], false);
        }
        colorDepthSansMedia.activate();
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glClearColor(0,0,0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderRasterizerClass::render_tree(kDiffuseDepthNoMedia);
        DC()->drawAll(); //Draw and flush buffers
        colorDepthSansMedia.deactivate();
        RasPtr->Begin(); // Needing to call Rasterizer_Shader_Class::Begin() again is wonky.
        glActiveTexture(GL_TEXTURE2); //Bind the colorDepthSansMedia texture to unit 2.
        glBindTexture(GL_TEXTURE_2D, colorDepthSansMedia.texID);
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glPopGroupMarkerEXT();
        glActiveTexture(GL_TEXTURE0);
    }
    
	RenderRasterizerClass::render_tree(kDiffuse);
    //DC()->drawAll(); //Draw and flush buffers
	Shader::disable(); //For some reason, the screen goes a solid color if we don't do this here or in render_viewer_sprite_layer
	
    render_viewer_sprite_layer(kDiffuse);
	DC()->drawAll(); //Draw and flush buffers
    
	if (current_player->infravision_duration == 0 &&
		TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_Blur) &&
		blur.get())
	{
		blur->begin();
            DC()->startGatheringLights();
            
            //Add a random light off the floor if the player has invincibility active.
            if(current_player->invincibility_duration) {
                DC()->addPointLight(current_player->location.x, current_player->location.y, current_player->location.z + 200, 2000, rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX), 0);
            }
            
			//DCW demo from coordinates: -4737, 15180, 0
			//DC()->addSpotLight( -4737,15180,200, 4000, 1,0,-.1, 20,10, 1,.1,.1, 0);
			//DC()->addSpotLight( -4737,15180,200, 4000, -1,0,-.1, 20,10, .1,1,.1, 0);
			//DC()->addPointLight( -4737,15180,100, 4000, 1,0,0, 0);
			//DC()->addPointLight( -15061,5715,200, 1000, 1,1,1, 0);
			//DC()->addSpotLight( -15061,5715,200, 4000, 0,1,-.1, 20,10, 1,.1,.1, 0);
			//DC()->addSpotLight( -15061,5715,200, 4000, 0,-1,-.1, 20,10, .1,1,.1, 0);
		
            RenderRasterizerClass::render_tree(kGlow);
            DC()->finishGatheringLights();
            //DC()->drawAll(); //Draw and flush buffers
            render_viewer_sprite_layer(kGlow);
            DC()->drawAll(); //Draw and flush buffers
		
		blur->end();
		RasPtr->swapper->deactivate();
		blur->draw(*RasPtr->swapper);
		RasPtr->swapper->activate();
	}

	//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
}

void RenderRasterize_Shader::render_node(sorted_node_data *node, bool SeeThruLiquids, RenderStep renderStep)
{
	// parasitic object detection
    objectCount = 0;
    objectY = 0;

    RenderRasterizerClass::render_node(node, SeeThruLiquids, renderStep);

	// turn off clipping planes
    MSI()->disablePlane(0);
    MSI()->disablePlane(1);
}

void RenderRasterize_Shader::clip_to_window(clipping_window_data *win)
{
    GLfloat clip[] = { 0., 0., 0., 0. };
        
    // recenter to player's orientation temporarily
    MSI()->pushMatrix();
    MSI()->translatef(view->origin.x, view->origin.y, 0.);
    MSI()->rotatef(view->yaw * (360/float(FULL_CIRCLE)) + 90., 0., 0., 1.);
    
    MSI()->rotatef(-0.1, 0., 0., 1.); // leave some excess to avoid artifacts at edges
	if (win->left.i != leftmost_clip.i || win->left.j != leftmost_clip.j) {
		clip[0] = win->left.i;
		clip[1] = win->left.j;
        MSI()->enablePlane(0);
		MSI()->clipPlanef(0, clip);
	} else {
        MSI()->disablePlane(0);
	}
	
    MSI()->rotatef(0.2, 0., 0., 1.); // breathing room for right-hand clip
	if (win->right.i != rightmost_clip.i || win->right.j != rightmost_clip.j) {
		clip[0] = win->right.i;
		clip[1] = win->right.j;
		
        MSI()->enablePlane(1);
		MSI()->clipPlanef(1, clip);
	} else {
        MSI()->disablePlane(1);
	}
    
    MSI()->popMatrix();
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
	GLfloat shade = PIN(static_cast<GLfloat>(rect.ambient_shade)/static_cast<GLfloat>(FIXED_ONE),0,1);
	color[0] = color[1] = color[2] = shade;

	auto TMgr = std::make_unique<TextureManager>();

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
    
	//glEnable(GL_TEXTURE_2D); //NOT SUPPORTED ANGLE ENUM

	// priorities: static, infravision, tinted/solid, shadeless
	if (TMgr->TransferMode == _static_transfer) {
		TMgr->IsShadeless = 1;
		flare = -1;
		if (renderStep == kDiffuse) {
			s = Shader::get(Shader::S_Invincible);
		} else {
			s = Shader::get(Shader::S_InvincibleBloom);
            
             //Add a random light slightly off the floor
            DC()->addPointLight(rect.Position.x, rect.Position.y, rect.Position.z + 100, 2000, rand() / double(RAND_MAX), rand() / double(RAND_MAX), rand() / double(RAND_MAX), 0);
		}
		s->enable();
		s->setFloat(Shader::U_TransferFadeOut,((float)((uint16)rect.transfer_data))/(float)((int)FIXED_ONE));
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
			if (renderStep == kDiffuse || renderStep == kDiffuseDepthNoMedia) {
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
		//s->setFloat(Shader::U_BloomScale, TMgr->BloomScale());
		//s->setFloat(Shader::U_BloomShift, TMgr->BloomShift());
		DC()->cacheBloomScale(TMgr->BloomScale());
		DC()->cacheBloomShift(TMgr->BloomShift());
	}
	/*s->setFloat(Shader::U_Flare, flare);
	s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
	s->setFloat(Shader::U_Pulsate, 0);
	s->setFloat(Shader::U_Wobble, 0);
	s->setFloat(Shader::U_Depth, offset);
	s->setFloat(Shader::U_StrictDepthMode, OGL_ForceSpriteDepth() ? 1 : 0);
	s->setFloat(Shader::U_Glow, 0);*/
	MSI()->color4f(color[0], color[1], color[2], 1);
    //s->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
	DC()->cacheFlare(flare);
	DC()->cacheSelfLuminosity(selfLuminosity);
	DC()->cachePulsate(0);
	DC()->cacheWobble(0);
	DC()->cacheDepth(offset);
	DC()->cacheStrictDepthMode(OGL_ForceSpriteDepth() ? 1 : 0);
	DC()->cacheGlow(0);

	return TMgr;
}

// Circle constants
const double Radian2Circle = 1/TWO_PI;			// A circle is 2*pi radians
const double FullCircleReciprocal = 1/double(FULL_CIRCLE);

std::unique_ptr<TextureManager> RenderRasterize_Shader::setupWallTexture(const shape_descriptor& Texture, short transferMode, float pulsate, float wobble, float intensity, float offset, RenderStep renderStep) {

	Shader *s = NULL;

	auto TMgr = std::make_unique<TextureManager>();
	LandscapeOptions *opts = NULL;
	TMgr->ShapeDesc = Texture;
	if (TMgr->ShapeDesc == UNONE) { return TMgr; }
	get_shape_bitmap_and_shading_table(Texture, &TMgr->Texture, &TMgr->ShadingTables, _shading_normal);

	TMgr->TransferMode = _textured_transfer;
	TMgr->IsShadeless = current_player->infravision_duration ? 1 : 0;
	TMgr->TransferData = 0;

	float flare = weaponFlare;

	//glEnable(GL_TEXTURE_2D); //NOT SUPPORTED ANGLE ENUM
	MSI()->color4f(intensity, intensity, intensity, 1.0);

	switch(transferMode) {
		case _xfer_static:
			TMgr->TextureType = OGL_Txtr_Wall;
			TMgr->TransferMode = _static_transfer;
			TMgr->IsShadeless = 1;
			flare = -1;
			s = Shader::get(renderStep == kGlow ? Shader::S_InvincibleBloom : Shader::S_Invincible);
			s->enable();
			s->setFloat(Shader::U_TransferFadeOut,0);
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
				MSI()->color4f(color[0], color[1], color[2], 1);
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
				if (renderStep == kDiffuse || renderStep == kDiffuseDepthNoMedia) {
					MSI()->color4f(1,1,1,1);
				} else {
					MSI()->color4f(0,0,0,1);
				}
				flare = -1;
			}
	}

	if(s == NULL) {
		if (current_player->infravision_duration) {
			GLfloat color[3] {1, 1, 1};
			FindInfravisionVersionRGBA(GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(Texture)), color);
			MSI()->color4f(color[0], color[1], color[2], 1);
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
			glActiveTexture(GL_TEXTURE1);
			TMgr->RenderBump();
			glActiveTexture(GL_TEXTURE0);
		}
	} else {
		TMgr->ShapeDesc = UNONE;
		return TMgr;
	}

	TMgr->SetupTextureMatrix();
	
	if (TMgr->TextureType == OGL_Txtr_Landscape && opts) {
		double TexScale = std::abs(TMgr->U_Scale);
		double HorizScale = double(1 << opts->HorizExp);
		s->setFloat(Shader::U_ScaleX, HorizScale * (npotTextures ? 1.0 : TexScale) * Radian2Circle);
        DC()->cacheScaleX(HorizScale * (npotTextures ? 1.0 : TexScale) * Radian2Circle);
		s->setFloat(Shader::U_OffsetX, HorizScale * (0.25 + opts->Azimuth * FullCircleReciprocal));
        DC()->cacheOffsetX(HorizScale * (0.25 + opts->Azimuth * FullCircleReciprocal));
        
		short AdjustedVertExp = opts->VertExp + opts->OGL_AspRatExp;
		double VertScale = (AdjustedVertExp >= 0) ? double(1 << AdjustedVertExp)
		                                          : 1/double(1 << (-AdjustedVertExp));
		s->setFloat(Shader::U_ScaleY, VertScale * TexScale * Radian2Circle);
        DC()->cacheScaleY(VertScale * TexScale * Radian2Circle);
		s->setFloat(Shader::U_OffsetY, (0.5 + TMgr->U_Offset) * TexScale);
        DC()->cacheOffsetY((0.5 + TMgr->U_Offset) * TexScale);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW added for landscape. Repeat horizontally
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //DCW added for landscape. Mirror vertically.

      } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //DCW this is probably better for non-landscapes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //DCW this is probably better for non-landscapes
      }

	if (renderStep == kGlow) {
		if (TMgr->TextureType == OGL_Txtr_Landscape) {
			//s->setFloat(Shader::U_BloomScale, TMgr->LandscapeBloom());
            DC()->cacheBloomScale(TMgr->LandscapeBloom());
		} else {
			//s->setFloat(Shader::U_BloomScale, TMgr->BloomScale());
            DC()->cacheBloomScale(TMgr->BloomScale());
			//s->setFloat(Shader::U_BloomShift, TMgr->BloomShift());
            DC()->cacheBloomShift(TMgr->BloomShift());
		}
	}
	//s->setFloat(Shader::U_Flare, flare);
    DC()->cacheFlare(flare);
	//s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
    DC()->cacheSelfLuminosity(selfLuminosity);
	//s->setFloat(Shader::U_Pulsate, pulsate);
    DC()->cachePulsate(pulsate);
	//s->setFloat(Shader::U_Wobble, wobble);
    DC()->cacheWobble(wobble);
	//s->setFloat(Shader::U_Depth, offset);
    DC()->cacheDepth(offset);
	//s->setFloat(Shader::U_Glow, 0);
    DC()->cacheGlow(0);
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
		case _xfer_reverse_horizontal_slide:
		case _xfer_reverse_fast_horizontal_slide:
		case _xfer_reverse_vertical_slide:
		case _xfer_reverse_fast_vertical_slide:
			x0 = y0= 0;
			switch (transfer_mode) {
				case _xfer_fast_horizontal_slide: transfer_phase<<= 1;
				case _xfer_horizontal_slide: x0= (transfer_phase<<2)&(WORLD_ONE-1); break;

				case _xfer_fast_vertical_slide: transfer_phase<<= 1;
				case _xfer_vertical_slide: y0= (transfer_phase<<2)&(WORLD_ONE-1); break;
				case _xfer_reverse_fast_horizontal_slide: transfer_phase<<= 1;
				case _xfer_reverse_horizontal_slide: x0 = WORLD_ONE - (transfer_phase<<2)&(WORLD_ONE-1); break;

				case _xfer_reverse_fast_vertical_slide: transfer_phase<<= 1;
				case _xfer_reverse_vertical_slide: y0 = WORLD_ONE - (transfer_phase<<2)&(WORLD_ONE-1); break;

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
		//glEnable(GL_TEXTURE_2D); //NOT SUPPORTED ANGLE ENUM
		glEnable(GL_BLEND);
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION

		s->enable();
		if (renderStep == kGlow) {
			//s->setFloat(Shader::U_BloomScale, TMgr->GlowBloomScale());
            DC()->cacheBloomScale(TMgr->GlowBloomScale());
			//s->setFloat(Shader::U_BloomShift, TMgr->GlowBloomShift());
            DC()->cacheBloomShift(TMgr->GlowBloomShift());
		}
		//s->setFloat(Shader::U_Flare, flare);
        DC()->cacheFlare(flare);
		//s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
        DC()->cacheSelfLuminosity(selfLuminosity);
		//s->setFloat(Shader::U_Wobble, wobble);
        DC()->cacheWobble(wobble);
		//s->setFloat(Shader::U_Depth, offset - 1.0);
        DC()->cacheDepth(offset - 1.0);
		//s->setFloat(Shader::U_Glow, TMgr->MinGlowIntensity());
        DC()->cacheGlow(TMgr->MinGlowIntensity());
		return true;
	}
	return false;
}

void    RenderRasterize_Shader::render_node_floor_or_ceiling(clipping_window_data *window, polygon_data *polygon, horizontal_surface_data *surface, bool void_present, bool ceil, RenderStep renderStep) {

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
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION
	} else {
		glDisable(GL_BLEND);
        
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
	}

	if (void_present && TMgr->IsBlended()) {
		glDisable(GL_BLEND);
        
		//glDisable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
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
		MSI()->normal3f(N[0], N[1], N[2]);
        
        GLfloat tex4[4] = {T[0], T[1], T[2], sign};
        
		//glMultiTexCoord4f(GL_TEXTURE1, T[0], T[1], T[2], sign); //NOT SUPPORTED ANGLE FUNCTION

		GLfloat vertex_array[MAXIMUM_VERTICES_PER_POLYGON * 3];
		GLfloat texcoord_array[MAXIMUM_VERTICES_PER_POLYGON * 2];

		GLfloat* vp = vertex_array;
		GLfloat* tp = texcoord_array;
		float scale;

		switch (surface->transfer_mode)
		{
			case _xfer_2x:
				scale = 2 * WORLD_ONE * TMgr->TileRatio();
				break;
			case _xfer_4x:
				scale = 4 * WORLD_ONE * TMgr->TileRatio();
				break;
			default:
				scale = WORLD_ONE * TMgr->TileRatio();
				break;
		}

		if (ceil)
		{
			for(short i = 0; i < vertex_count; ++i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[vertex_count - 1 - i])->vertex;
				*vp++ = vertex.x;
				*vp++ = vertex.y;
				*vp++ = surface->height;
				*tp++ = (vertex.x + surface->origin.x + x) / scale;
				*tp++ = (vertex.y + surface->origin.y + y) / scale;
			}
		}
		else
		{
			for(short i=0; i<vertex_count; ++i) {
				world_point2d vertex = get_endpoint_data(polygon->endpoint_indexes[i])->vertex;
				*vp++ = vertex.x;
				*vp++ = vertex.y;
				*vp++ = surface->height;
				*tp++ = (vertex.x + surface->origin.x + x) / scale;
				*tp++ = (vertex.y + surface->origin.y + y) / scale;
			}
		}
        
		//glVertexPointer(3, GL_FLOAT, 0, vertex_array);
		//glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);
        
        /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texcoord_array);
        glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
        
        glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertex_array);
        glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
        
        glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, MSI()->normals());
        glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);


        Shader* lastShader = lastEnabledShader();
        if (lastShader) {
          lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
          lastShader->setVec4(Shader::U_TexCoords4, tex4);
        }
        
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);*/

        DC()->addTriangleFan(vertex_count, vertex_array, texcoord_array, tex4);
        
		// see note 2 above; pulsate uniform should stay set from setupWall call
		if (setupGlow(view, TMgr, 0, intensity, weaponFlare, selfLuminosity, offset, renderStep)) {
			//glDrawArrays(GL_TRIANGLE_FAN, 0, vertex_count);
            DC()->addTriangleFan(vertex_count, vertex_array, texcoord_array, tex4);
		}

		Shader::disable();
		MSI()->matrixMode(MS_TEXTURE);
		MSI()->loadIdentity();
		MSI()->matrixMode(MS_MODELVIEW);
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
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION
	} else {
		//glDisable(GL_BLEND);
        
        glEnable(GL_BLEND); //DCW seriously, why would we ever want to not blend?
        
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
	}

	if (void_present && TMgr->IsBlended()) {
		//glDisable(GL_BLEND);
        glEnable(GL_BLEND); //DCW seriously, why would we ever want to not blend?
        
		//glDisable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
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
            vertices[0].x= vertices[3].x= vertex[0].x; vertices[0].y= vertices[3].y= vertex[0].y; //DCW changed , to ;
            vertices[1].x= vertices[2].x= vertex[1].x; vertices[1].y= vertices[2].y= vertex[1].y;//DCW changed , to ;
			vertices[0].flags = vertices[3].flags = 0;
			vertices[1].flags = vertices[2].flags = 0;

			uint16 div;
			switch (surface->transfer_mode)
			{
				case _xfer_2x:
					div = 2 * WORLD_ONE * TMgr->TileRatio();
					break;
				case _xfer_4x:
					div = 4 * WORLD_ONE * TMgr->TileRatio();
					break;
				default:
					div = WORLD_ONE * TMgr->TileRatio();;
					break;
			}

			double dx = (surface->p1.i - surface->p0.i) / double(surface->length);
			double dy = (surface->p1.j - surface->p0.j) / double(surface->length);

			world_distance x0 = surface->texture_definition->x0 % div;
			world_distance y0 = surface->texture_definition->y0 % div;

			double tOffset = surface->h1 + view->origin.z + y0;

			vec3 N(-dy, dx, 0);
			vec3 T(dx, dy, 0);
			float sign = 1;
			MSI()->normal3f(N[0], N[1], N[2]);
            GLfloat tex4[4] = {T[0], T[1], T[2], sign};
			//glMultiTexCoord4f(GL_TEXTURE1, tex4[0], tex4[1], tex4[2], tex4[3]); //NOT SUPPORTED ANGLE FUNCTION

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
				*tp++ = (tOffset - vertices[i].z) / static_cast<float>(div);
				*tp++ = (x0+p2) / static_cast<float>(div);
			}
			//glVertexPointer(3, GL_FLOAT, 0, vertex_array);
			//glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);
            /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texcoord_array);
            glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
            
            glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertex_array);
            glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
            
            glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, MatrixStack::Instance()->normals());
            glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);

            
            Shader* lastShader = lastEnabledShader();
            if (lastShader) {
              lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
              lastShader->setVec4(Shader::U_TexCoords4, tex4);
            }*/
            
            //These originally drew GL_QUADS with a vertex_count. If that's ever not 4, the triangle fan substitution will be bad.
            assert(vertex_count < 5); //If we ever hit this, I will need to rewrite the below for GL_QUADS (or ignore the visial glitch?);
            
			//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            DC()->addTriangleFan(vertex_count, vertex_array, texcoord_array, tex4);

			if (setupGlow(view, TMgr, wobble, intensity, weaponFlare, selfLuminosity, offset, renderStep)) {
				//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                DC()->addTriangleFan(vertex_count, vertex_array, texcoord_array, tex4);
			}

			Shader::disable();
			MSI()->matrixMode(MS_TEXTURE);
			MSI()->loadIdentity();
			MSI()->matrixMode(MS_MODELVIEW);
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

	//glEnable(GL_TEXTURE_2D); //NOT SUPPORTED ANGLE ENUM
	if (SkinPtr->OpacityType != OGL_OpacType_Crisp || RenderRectangle.transfer_mode == _tinted_transfer) {
		glEnable(GL_BLEND);
		setupBlendFunc(SkinPtr->NormalBlend);
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION
	} else {
		glDisable(GL_BLEND);
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
	}

	GLfloat color[3];
	GLfloat shade = PIN(static_cast<GLfloat>(RenderRectangle.ambient_shade)/static_cast<GLfloat>(FIXED_ONE),0,1);
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
		s->enable();
		s->setFloat(Shader::U_TransferFadeOut,((float)((uint16)RenderRectangle.transfer_data))/(float)((int)FIXED_ONE));
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
			if (renderStep == kDiffuse || renderStep == kDiffuseDepthNoMedia) {
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
		/*s->setFloat(Shader::U_BloomScale, SkinPtr->BloomScale);
		s->setFloat(Shader::U_BloomShift, SkinPtr->BloomShift);*/
		DC()->cacheBloomScale(SkinPtr->BloomScale);
		DC()->cacheBloomShift(SkinPtr->BloomShift);
	}
	/*s->setFloat(Shader::U_Flare, flare);
	s->setFloat(Shader::U_SelfLuminosity, selfLuminosity);
	s->setFloat(Shader::U_Wobble, 0);
	s->setFloat(Shader::U_Depth, 0);
	s->setFloat(Shader::U_Glow, 0);*/
	
	DC()->cacheFlare(flare);
	DC()->cacheSelfLuminosity(selfLuminosity);
	DC()->cacheWobble(0);
	DC()->cacheDepth(0);
	DC()->cacheGlow(0);
	
	MSI()->color4f(color[0], color[1], color[2], 1);
	
	// Find an animated model's vertex positions and normals:
	short ModelSequence = RenderRectangle.ModelSequence;
	if (ModelSequence >= 0)
	{
		int NumFrames = ModelPtr->Model.NumSeqFrames(ModelSequence);
		if (NumFrames > 0)
		{
			short ModelFrame = PIN(RenderRectangle.ModelFrame, 0, NumFrames - 1);
			short NextModelFrame = PIN(RenderRectangle.NextModelFrame, 0, NumFrames - 1);
			float MixFrac = RenderRectangle.MixFrac;
			ModelPtr->Model.FindPositions_Sequence(true,
				ModelSequence, ModelFrame, MixFrac, NextModelFrame);
		}
		else
			ModelPtr->Model.FindPositions_Neutral(true);	// Fallback: neutral
	}
	else
		ModelPtr->Model.FindPositions_Neutral(true);	// Fallback: neutral (will do nothing for static models)
    
	//glVertexPointer(3,GL_FLOAT,0,ModelPtr->Model.PosBase());
    /*glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, ModelPtr->Model.PosBase());
    glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);*/
    
	//glClientActiveTexture(GL_TEXTURE0);
	if (ModelPtr->Model.TxtrCoords.empty()) {
		//glDisableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	} else {
        /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, ModelPtr->Model.TCBase());
        glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);*/
	}

	//glEnableClientState(GL_NORMAL_ARRAY);
	//glNormalPointer(GL_FLOAT,0,ModelPtr->Model.NormBase());
    /*glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, ModelPtr->Model.NormBase());
    glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);*/
    
	//glClientActiveTexture(GL_TEXTURE1);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	//glTexCoordPointer(4,GL_FLOAT,sizeof(vec4),ModelPtr->Model.TangentBase());
    /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS4, 4, GL_FLOAT, GL_FALSE, 0, ModelPtr->Model.TangentBase());
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS4);*/

	if(ModelPtr->Use(CLUT,OGL_SkinManager::Normal)) {
		LoadModelSkin(SkinPtr->NormalImg, Collection, CLUT);
	}

	if(TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_BumpMap)) {
		glActiveTexture(GL_TEXTURE1);
		if(ModelPtr->Use(CLUT,OGL_SkinManager::Bump)) {
			LoadModelSkin(SkinPtr->OffsetImg, Collection, CLUT);
		}
		if (!SkinPtr->OffsetImg.IsPresent()) {
			FlatBumpTexture();
		}
		glActiveTexture(GL_TEXTURE0);
	}

    /*Shader* lastShader = lastEnabledShader();
    if (lastShader) {
		GLfloat modelMatrix[16], projectionMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];

		MatrixStack::Instance()->getFloatv(MS_MODELVIEW, modelMatrix);
		MatrixStack::Instance()->getFloatv(MS_PROJECTION, projectionMatrix);
		MatrixStack::Instance()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
		MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);
		MatrixStack::Instance()->getFloatv(MS_TEXTURE, textureMatrix);

		lastShader->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
		lastShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
		lastShader->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
		lastShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
		lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
		lastShader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
		lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
		lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());
        
        GLfloat plane0[4], plane1[4], plane5[4];
        MatrixStack::Instance()->getPlanev(0, plane0);
        MatrixStack::Instance()->getPlanev(1, plane1);
        MatrixStack::Instance()->getPlanev(5, plane5);
        lastShader->setVec4(Shader::U_ClipPlane0, plane0);
        lastShader->setVec4(Shader::U_ClipPlane1, plane1);
        lastShader->setVec4(Shader::U_ClipPlane5, plane5);

    }*/
    
	//glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());
	
	DC()->addTriangles(ModelPtr->Model.NumVI(), ModelPtr->Model.VIBase(), ModelPtr->Model.PosBase(), ModelPtr->Model.TCBase(), ModelPtr->Model.NormBase(), ModelPtr->Model.TangentBase());
	
	if (canGlow && SkinPtr->GlowImg.IsPresent()) {
		glEnable(GL_BLEND);
		setupBlendFunc(SkinPtr->GlowBlend);
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION

		s->enable();
		s->setFloat(Shader::U_Glow, SkinPtr->MinGlowIntensity);
		if (renderStep == kGlow) {
			s->setFloat(Shader::U_BloomScale, SkinPtr->GlowBloomScale);
			s->setFloat(Shader::U_BloomShift, SkinPtr->GlowBloomShift);
		}

		if(ModelPtr->Use(CLUT,OGL_SkinManager::Glowing)) {
			LoadModelSkin(SkinPtr->GlowImg, Collection, CLUT);
		}
        
        /*Shader* lastShader = lastEnabledShader();
        if (lastShader) {
			GLfloat modelMatrix[16], projectionMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];

			MatrixStack::Instance()->getFloatv(MS_MODELVIEW, modelMatrix);
			MatrixStack::Instance()->getFloatv(MS_PROJECTION, projectionMatrix);
			MatrixStack::Instance()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
			MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);
			MatrixStack::Instance()->getFloatv(MS_TEXTURE, textureMatrix);

			lastShader->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
			lastShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
			lastShader->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
			lastShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
			lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
			lastShader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
			lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
			lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());
        }*/
        
		//glDrawElements(GL_TRIANGLES,(GLsizei)ModelPtr->Model.NumVI(),GL_UNSIGNED_SHORT,ModelPtr->Model.VIBase());
		DC()->addTriangles(ModelPtr->Model.NumVI(), ModelPtr->Model.VIBase(), ModelPtr->Model.PosBase(), ModelPtr->Model.TCBase(), ModelPtr->Model.NormBase(), ModelPtr->Model.TangentBase());
	}

	//glDisableClientState(GL_NORMAL_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
	//glClientActiveTexture(GL_TEXTURE0);
	if (ModelPtr->Model.TxtrCoords.empty()) {
		//glEnableClientState(GL_TEXTURE_COORD_ARRAY); //NOT SUPPORTED ANGLE FUNCTION
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
		GLfloat plane[] = { 0.0, 0.0, 1.0, -h };
		if (view->under_media_boundary ^ other_side_of_media) {
			plane[2] = -1.0;
			plane[3] = h;
		}
		MSI()->clipPlanef(5, plane);
        MSI()->enablePlane(5);
	} else if (other_side_of_media) {
		// When there's no media present, we can skip the second pass.
		return;
	}

    for (win = object->clipping_windows; win; win = win->next_window)
    {
        clip_to_window(win);
        _render_node_object_helper(object, renderStep);
    }
    
    MSI()->disablePlane(5);
}

void RenderRasterize_Shader::_render_node_object_helper(render_object_data *object, RenderStep renderStep) {	
	rectangle_definition& rect = object->rectangle;
	const world_point3d& pos = rect.Position;
    
    DC()->addDefaultLight(pos.x, pos.y, pos.z, object->object_owner_type, object->object_owner_permutation_type);
    
	if(rect.ModelPtr) {
		MSI()->pushMatrix();
		MSI()->translatef(pos.x, pos.y, pos.z);
		MSI()->rotatef((360.0/FULL_CIRCLE)*rect.Azimuth,0,0,1);
		GLfloat HorizScale = rect.Scale*rect.HorizScale;
		MSI()->scalef(HorizScale,HorizScale,rect.Scale);

		short descriptor = GET_DESCRIPTOR_COLLECTION(rect.ShapeDesc);
		short collection = GET_COLLECTION(descriptor);
		short clut = ModifyCLUT(rect.transfer_mode,GET_COLLECTION_CLUT(descriptor));

		RenderModel(rect, collection, clut, weaponFlare, selfLuminosity, renderStep);
		MSI()->popMatrix();
		return;
	}

	MSI()->pushMatrix();
	MSI()->translatef(pos.x, pos.y, pos.z);

	double yaw = view->virtual_yaw * FixedAngleToDegrees;
	MSI()->rotatef(yaw, 0.0, 0.0, 1.0);

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
	if (TMgr->ShapeDesc == UNONE) { MSI()->popMatrix(); return; }

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
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION
	} else {
		//glDisable(GL_BLEND);
        glEnable(GL_BLEND); //DCW seriously, why would we ever want to not blend?
        
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
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

	//glVertexPointer(3, GL_FLOAT, 0, vertex_array);
	//glTexCoordPointer(2, GL_FLOAT, 0, texcoord_array);
    
    /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texcoord_array);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
    
    glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertex_array);
    glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
    
    glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, MatrixStack::Instance()->normals());
    glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);*/

    Shader* lastShader = lastEnabledShader();
	/*
    if (lastShader) {
		GLfloat modelMatrix[16], projectionMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];
		MatrixStack::Instance()->getFloatv(MS_MODELVIEW, modelMatrix);
		MatrixStack::Instance()->getFloatv(MS_PROJECTION, projectionMatrix);
		MatrixStack::Instance()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
		MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);
		MatrixStack::Instance()->getFloatv(MS_TEXTURE, textureMatrix);

		lastShader->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
		lastShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
		lastShader->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
		lastShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
		lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
		lastShader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
		lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
		lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());


		GLfloat plane0[4], plane1[4], plane5[4], media6[4];
		MatrixStack::Instance()->getPlanev(0, plane0);
		MatrixStack::Instance()->getPlanev(1, plane1);
		MatrixStack::Instance()->getPlanev(5, plane5);
		MatrixStack::Instance()->getPlanev(6, media6);
		lastShader->setVec4(Shader::U_ClipPlane0, plane0);
		lastShader->setVec4(Shader::U_ClipPlane1, plane1);
		lastShader->setVec4(Shader::U_ClipPlane5, plane5);
    }*/
    
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	DC()->addTriangleFan(4, vertex_array, texcoord_array, NULL);
	
	if (setupGlow(view, TMgr, 0, 1, weaponFlare, selfLuminosity, offset, renderStep)) {
        
        /*glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, 0, texcoord_array);
        glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
        
        glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, vertex_array);
        glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
        
        glVertexAttribPointer(Shader::ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, MatrixStack::Instance()->normals());
        glEnableVertexAttribArray(Shader::ATTRIB_NORMAL);
        
        lastShader = lastEnabledShader();
        if (lastShader) {
			GLfloat modelMatrix[16], projectionMatrix[16], modelProjection[16], modelMatrixInverse[16], textureMatrix[16];
			MatrixStack::Instance()->getFloatv(MS_MODELVIEW, modelMatrix);
			MatrixStack::Instance()->getFloatv(MS_PROJECTION, projectionMatrix);
			MatrixStack::Instance()->getFloatvInverse(MS_MODELVIEW, modelMatrixInverse);
			MatrixStack::Instance()->getFloatvModelviewProjection(modelProjection);
			MatrixStack::Instance()->getFloatv(MS_TEXTURE, textureMatrix);

			lastShader->setMatrix4(Shader::U_ModelViewMatrix, modelMatrix);
			lastShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjection);
			lastShader->setMatrix4(Shader::U_ModelViewMatrixInverse, modelMatrixInverse);
			lastShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
			lastShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
			lastShader->setVec4(Shader::U_FogColor, MatrixStack::Instance()->fog());
			lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogStart());
			lastShader->setVec4(Shader::U_FogStart, MatrixStack::Instance()->fogEnd());


			GLfloat plane0[4], plane1[4], plane5[4], media6[4];
			MatrixStack::Instance()->getPlanev(0, plane0);
			MatrixStack::Instance()->getPlanev(1, plane1);
			MatrixStack::Instance()->getPlanev(5, plane5);
			MatrixStack::Instance()->getPlanev(6, media6);
			lastShader->setVec4(Shader::U_ClipPlane0, plane0);
			lastShader->setVec4(Shader::U_ClipPlane1, plane1);
			lastShader->setVec4(Shader::U_ClipPlane5, plane5);
        }
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);*/
		
		DC()->addTriangleFan(4, vertex_array, texcoord_array, NULL);
	}
        
	glEnable(GL_DEPTH_TEST);
	MSI()->popMatrix();
	Shader::disable();
	TMgr->RestoreTextureMatrix();
}

extern void position_sprite_axis(short *x0, short *x1, short scale_width, short screen_width, short positioning_mode, _fixed position, bool flip, world_distance world_left, world_distance world_right);

extern GLfloat Screen_2_Clip[16];

void RenderRasterize_Shader::render_viewer_sprite_layer(RenderStep renderStep)
{
        if (!view->show_weapons_in_hand) return;
    
        MSI()->matrixMode(MS_TEXTURE);
        MSI()->pushMatrix();
    
        MSI()->matrixMode(MS_PROJECTION);
        MSI()->pushMatrix();
        MSI()->loadMatrixf(Screen_2_Clip);

        MSI()->matrixMode(MS_MODELVIEW);
        MSI()->pushMatrix();
        MSI()->loadIdentity();

        rectangle_definition rect;
	weapon_display_information display_data;
	shape_information_data *shape_information;
	short count;

        rect.ModelPtr = nullptr;
        rect.Opacity = 1;

        /* get_weapon_display_information() returns true if there is a weapon to be drawn.  it
           should initially be passed a count of zero.  it returns the weapon’s texture and
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
		
		/* lighting: depth of zero in the camera’s polygon index */
		rect.depth= 0;
		rect.ambient_shade= get_light_intensity(get_polygon_data(view->origin_polygon_index)->floor_lightsource_index);
		rect.ambient_shade= MAX(shape_information->minimum_light_intensity, rect.ambient_shade);
		if (view->shading_mode==_shading_infravision) rect.flags|= _SHADELESS_BIT;

		// Calculate the object's horizontal position
		// for the convenience of doing teleport-in/teleport-out
		rect.xc = (rect.x0 + rect.x1) >> 1;

                /* make the weapon reflect the owner’s transfer mode */
		instantiate_rectangle_transfer_mode(view, &rect, display_data.transfer_mode, display_data.transfer_phase);

                render_viewer_sprite(rect, renderStep);
        }

        Shader::disable();
    
        MSI()->popMatrix();

        MSI()->matrixMode(MS_PROJECTION);
        MSI()->popMatrix();

        MSI()->matrixMode(MS_TEXTURE);
        MSI()->popMatrix();
    
        MSI()->matrixMode(MS_MODELVIEW);
}

struct ExtendedVertexData
{
	GLfloat Vertex[4];
	GLfloat TexCoord[2];
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
	GLfloat U_Scale = TMgr->U_Scale/(RenderRectangle.y1 - RenderRectangle.y0);
	GLfloat V_Scale = TMgr->V_Scale/(RenderRectangle.x1 - RenderRectangle.x0);
	GLfloat U_Offset = TMgr->U_Offset;
	GLfloat V_Offset = TMgr->V_Offset;
	
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
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.001); //NOT SUPPORTED ANGLE FUNCTION
	} else {
		//glDisable(GL_BLEND);
        glEnable(GL_BLEND); //DCW seriously, why would we ever want to not blend?
        
		//glEnable(GL_ALPHA_TEST); //NOT SUPPORTED ANGLE ENUM
		//glAlphaFunc(GL_GREATER, 0.5); //NOT SUPPORTED ANGLE FUNCTION
	}

    glDisable(GL_DEPTH_TEST);

	// Location of data:
	/*GLfloat modelProjectionMatrix[16], textureMatrix[16];
    Shader *theShader = lastEnabledShader();
    
    glVertexAttribPointer(Shader::ATTRIB_TEXCOORDS, 2, GL_FLOAT, 0, sizeof(ExtendedVertexData), ExtendedVertexList[0].TexCoord);
    glEnableVertexAttribArray(Shader::ATTRIB_TEXCOORDS);
    
    glVertexAttribPointer(Shader::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(ExtendedVertexData), ExtendedVertexList[0].Vertex);
    glEnableVertexAttribArray(Shader::ATTRIB_VERTEX);
	
    MatrixStack::Instance()->getFloatvModelviewProjection(modelProjectionMatrix);
    MatrixStack::Instance()->getFloatv(MS_TEXTURE_MATRIX, textureMatrix);
    
    theShader->setMatrix4(Shader::U_ModelViewProjectionMatrix, modelProjectionMatrix);
    theShader->setMatrix4(Shader::U_TextureMatrix, textureMatrix);
    theShader->setVec4(Shader::U_Color, MatrixStack::Instance()->color());
    
    GLfloat plane0[4], plane1[4], plane5[4];
    MatrixStack::Instance()->getPlanev(0, plane0);
    MatrixStack::Instance()->getPlanev(1, plane1);
    MatrixStack::Instance()->getPlanev(5, plane5);
    theShader->setVec4(Shader::U_ClipPlane0, plane0);
    theShader->setVec4(Shader::U_ClipPlane1, plane1);
    theShader->setVec4(Shader::U_ClipPlane5, plane5);*/
    
    //If invincibility in M1 is not blending, try uncommenting this out:
    //glEnable(GL_BLEND);
    
	//glEnable(GL_TEXTURE_2D); //NOT SUPPORTED ANGLE ENUM
		
	// Go!
        //glDrawArrays(GL_TRIANGLE_FAN,0,4); //This will no longer work since we've changed vertexColor = vColor; in the vert shader.
	
	//Re-pack vertex and texture data because the new draw operation doesn't support a custom stride.
	//This is stupid - maybe sometime we can replace the ExtendedVertexData with somethign different, or maybe support a custom stride in the future.
	GLfloat newVertexData[12], newTexData[8];
	for(int i = 0; i < 4; i++) {
		newVertexData[i*3] = ExtendedVertexList[i].Vertex[0]; newVertexData[i*3 + 1] = ExtendedVertexList[i].Vertex[1]; newVertexData[i*3 + 2] = ExtendedVertexList[i].Vertex[2];
		newTexData[i*2] = ExtendedVertexList[i].TexCoord[0]; newTexData[i*2 + 1] = ExtendedVertexList[i].TexCoord[1];
	}
	
	DC()->addTriangleFan(4, newVertexData, newTexData, NULL);
	
        if (setupGlow(view, TMgr, 0, 1, weaponFlare, selfLuminosity, 0, renderStep)) {
            //DCW I don't know about changing this to a triangle fan...  was originally glDrawArrays(GL_QUADS, 0, 4);
            //glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			DC()->addTriangleFan(4, newVertexData, newTexData, NULL);
	}
	
	glEnable(GL_DEPTH_TEST);
        Shader::disable();
	TMgr->RestoreTextureMatrix();
}
