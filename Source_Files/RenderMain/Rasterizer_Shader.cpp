/*
 *  Rasterizer_Shader.cpp
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */

#include "OGL_Headers.h"

#include <iostream>

#include "Rasterizer_Shader.h"

#include "lightsource.h"
#include "media.h"
#include "player.h"
#include "weapons.h"
#include "AnimatedTextures.h"
#include "OGL_Faders.h"
#include "OGL_FBO.h"
#include "vr_openxr.h"
#include "OGL_Textures.h"
#include "OGL_Shader.h"
#include "ChaseCam.h"
#include "preferences.h"
#include "fades.h"
#include "screen.h"

#ifdef HAVE_OPENGL

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

const float FixedAngleToDegrees = 360.0/(float(FIXED_ONE)*float(FULL_CIRCLE));

const GLdouble kViewBaseMatrix[16] = {
	0,	0,	-1,	0,
	1,	0,	0,	0,
	0,	1,	0,	0,
	0,	0,	0,	1
};

const GLdouble kViewBaseMatrixInverse[16] = {
	0,	1,	0,	0,
	0,	0,	1,	0,
	-1,	0,	0,	0,
	0,	0,	0,	1
};

Rasterizer_Shader_Class::Rasterizer_Shader_Class() = default;
Rasterizer_Shader_Class::~Rasterizer_Shader_Class() = default;

void Rasterizer_Shader_Class::SetView(view_data& view) {
	OGL_SetView(view);
	
	if (view.screen_width != view_width || view.screen_height != view_height) {
		view_width = view.screen_width;
		view_height = view.screen_height;
		swapper.reset();
		swapper.reset(new FBOSwapper(view_width * MainScreenPixelScale(), view_height * MainScreenPixelScale(), false));
	}
	
	float aspect = view.screen_width / float(view.screen_height);
	float deg2rad = 8.0 * atan(1.0) / 360.0;
	float xtan, ytan;
	if (View_FOV_FixHorizontalNotVertical()) {
		xtan = tan(view.field_of_view * deg2rad / 2.0);
		ytan = xtan / aspect;
	} else {
		ytan = tan(view.field_of_view * deg2rad / 2.0) / 2.0;
		xtan = ytan * aspect;
	}
	
	// Adjust for view distortion during teleport effect
	ytan *= view.real_world_to_screen_y / double(view.world_to_screen_y);
	xtan *= view.real_world_to_screen_x / double(view.world_to_screen_x);

	double yaw = view.virtual_yaw * FixedAngleToDegrees;
	double pitch = view.virtual_pitch * FixedAngleToDegrees;
	pitch = (pitch > 180.0 ? pitch - 360.0 : pitch);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float nearVal = 64.0;
	float farVal = 128.0 * 1024.0;
	float x = xtan * nearVal;
	float y = ytan * nearVal;
	float yoff = view.mimic_sw_perspective ? tan(pitch * deg2rad) * nearVal : 0;
	glFrustum(-x, x, -y + yoff, y + yoff, nearVal, farVal);

	glMatrixMode(GL_MODELVIEW);

	// Build the classic view matrix (kViewBaseMatrix * Ry(pitch) * Rz(-yaw) * T(-origin)).
	// Used for both the non-VR modelview AND as the landscape UV matrix, which the landscape
	// shaders use (via landscapeInverseMatrix uniform) to compute relDir in a yaw-aligned
	// classic eye space so landscape textures stay anchored to world directions.
	glLoadMatrixd(kViewBaseMatrix);
	if (!view.mimic_sw_perspective)
		glRotated(pitch, 0.0, 1.0, 0.0);
	glRotated(-yaw, 0.0, 0.0, 1.0);
	glTranslated(-view.origin.x, -view.origin.y, -view.origin.z);

	// Capture and set as the landscape UV matrix for all 6 landscape shader variants.
	// In VR this is overridden below with the eye-height-adjusted version.
	{
		GLfloat landscapeMatrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, landscapeMatrix);
		Shader *ls[] = {
			Shader::get(Shader::S_Landscape),
			Shader::get(Shader::S_LandscapeBloom),
			Shader::get(Shader::S_LandscapeInfravision),
			Shader::get(Shader::S_LandscapeSphere),
			Shader::get(Shader::S_LandscapeSphereBloom),
			Shader::get(Shader::S_LandscapeSphereInfravision),
		};
		for (auto s : ls) { s->enable(); s->setMatrix4(Shader::U_LandscapeInverseMatrix, landscapeMatrix); }
		Shader::disable();
	}

#if defined(__ANDROID__)
	if (VR_IsActive())
	{
		// Override the engine camera with the OpenXR per-eye projection + head-pose view. The view
		// is eyeFromStage (metres, Y-up) composed with the Marathon world transform (Z-up, world
		// units): modelview = eyeFromStage * S(1/WUperMetre) * (Z-up->Y-up) * Rz(-bodyYaw) * T(-origin).
		// bodyYaw = the engine's locomotion yaw; head yaw/pitch/roll ride in via eyeFromStage.
		const int eye = VR_CurrentEye();
		const float WUperMetre = VR_Settings()->worldScaleWUM;  // ~1 WORLD_ONE per 2 m; tune for comfort
		const float eyeHeightM = VR_Settings()->eyeHeightM;     // align the standing HMD eye with the Marathon eye

		float vrProj[16];
		VR_GetEyeProjection(eye, vrProj, 0.05f, (128.0f * 1024.0f) / WUperMetre);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(vrProj);

		// Override landscape UV matrix with VR eye-height-adjusted classic view.
		// No head pitch: head orientation rides in vrView for world geometry, not landscape UVs.
		{
			const float eyeZAdj = VR_GetEyeZOffset() + eyeHeightM * WUperMetre;
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixd(kViewBaseMatrix);
			glRotated(-yaw, 0.0, 0.0, 1.0);
			glTranslated(-view.origin.x, -view.origin.y, -(view.origin.z - eyeZAdj));
			GLfloat vrLandscapeMatrix[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, vrLandscapeMatrix);
			Shader *ls[] = {
				Shader::get(Shader::S_Landscape),
				Shader::get(Shader::S_LandscapeBloom),
				Shader::get(Shader::S_LandscapeInfravision),
				Shader::get(Shader::S_LandscapeSphere),
				Shader::get(Shader::S_LandscapeSphereBloom),
				Shader::get(Shader::S_LandscapeSphereInfravision),
			};
			for (auto s : ls) { s->enable(); s->setMatrix4(Shader::U_LandscapeInverseMatrix, vrLandscapeMatrix); }
			Shader::disable();
		}

		float vrView[16];   // eyeFromStage, metres
		VR_GetEyeViewMetres(eye, vrView);
		// world(Marathon, Z-up, left-handed yaw) -> stage(Y-up): (x,y,z) -> (-x, z, -y). The negated
		// X makes this det=-1 to match the engine's kViewBaseMatrix handedness (else the world is
		// mirrored). With det=-1 the engine's glFrontFace(GL_CW) culling is correct (no shim flip).
		static const float zUpToYUp[16] = { -1,0,0,0,  0,0,-1,0,  0,1,0,0,  0,0,0,1 };
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(vrView);
		glScalef(1.0f / WUperMetre, 1.0f / WUperMetre, 1.0f / WUperMetre);
		glMultMatrixf(zUpToYUp);
		glRotated(-yaw, 0.0, 0.0, 1.0);
		// Subtract the eye-Z offset that update_world_view_camera added to origin.z for the visibility
		// tree, so the rendered camera height is exactly as before (head height comes via vrView).
		glTranslated(-view.origin.x, -view.origin.y,
			-(view.origin.z - VR_GetEyeZOffset() - eyeHeightM * WUperMetre));
	}
#endif
}

void Rasterizer_Shader_Class::setupGL()
{
	view_width = 0;
	view_height = 0;
	swapper.reset();
	
	smear_the_void = false;
	OGL_ConfigureData& ConfigureData = Get_OGL_ConfigureData();
	if (!TEST_FLAG(ConfigureData.Flags,OGL_Flag_VoidColor))
		smear_the_void = true;
}

void Rasterizer_Shader_Class::Begin()
{
	Rasterizer_OGL_Class::Begin();
#if defined(__ANDROID__)
	if (VR_IsActive()) {
		// Render the world straight into the eye swapchain FBO (bound + cleared by VR_BeginEye) at
		// eye resolution. Skip the FBOSwapper: its size/aspect differ from the eye (causing the
		// stretch/smear) and it blits to the screen, not the eye buffer. (No gamma/bloom in VR yet.)
		glViewport(0, 0, VR_EyeWidth(), VR_EyeHeight());
		return;
	}
#endif
	swapper->activate();
	if (smear_the_void)
		swapper->current_contents().draw_full();
}

void Rasterizer_Shader_Class::End()
{
#if defined(__ANDROID__)
	if (VR_IsActive()) {
		// The world is already in the bound eye FBO; no swapper resolve / gamma / screen blit.
		// (Brightness is applied per-fragment in the shader via a1_Brightness, set by the shim.)
		Rasterizer_OGL_Class::End();
		return;
	}
#endif
	swapper->deactivate();
	swapper->swap();
	
	float gamma_adj = get_actual_gamma_adjust(graphics_preferences->screen_mode.gamma_level);
	if (gamma_adj < 0.99f || gamma_adj > 1.01f) {
		Shader *s = Shader::get(Shader::S_Gamma);
		s->enable();
		s->setFloat(Shader::U_GammaAdjust, gamma_adj);
	}
#if defined(__ANDROID__)
	// VR: the FBOSwapper's deactivate left the pbuffer/default FB bound; the final composite must
	// target the bound eye swapchain framebuffer at eye resolution.
	if (VR_IsActive()) {
		glBindFramebuffer(GL_FRAMEBUFFER, VR_CurrentEyeFramebuffer());
		glViewport(0, 0, VR_EyeWidth(), VR_EyeHeight());
	}
#endif
	swapper->draw();
	Shader::disable();
	
	SetForeground();
	glColor3f(0, 0, 0);
	OGL_RenderFrame(0, 0, view_width, view_height, 1);
	
	Rasterizer_OGL_Class::End();
}

#endif