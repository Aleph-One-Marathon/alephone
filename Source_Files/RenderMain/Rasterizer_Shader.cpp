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
#include "OGL_Textures.h"
#include "OGL_Shader.h"
#include "ChaseCam.h"

#define MAXIMUM_VERTICES_PER_WORLD_POLYGON (MAXIMUM_VERTICES_PER_POLYGON+4)

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

void Rasterizer_Shader_Class::SetView(view_data& view) {
	OGL_SetView(view);
	float aspect = view.screen_width / float(view.screen_height);
	float yfov = view.field_of_view / (View_FOV_FixHorizontalNotVertical() ? aspect : 2.0);
	float xfov = yfov * aspect;
	
	// Adjust for view distortion during teleport effect
	yfov *= view.real_world_to_screen_y / float(view.world_to_screen_y);
	xfov *= view.real_world_to_screen_x / float(view.world_to_screen_x);
	
	// The view flips or shrinks when the FOV goes too far out
	// of normal bounds, so this limit keeps the teleporting
	// effect from getting distorted in annoying ways.
	if (yfov > 119.9) {
		yfov = 119.9;
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(yfov, xfov / yfov, 64, 128*1024);
	glMatrixMode(GL_MODELVIEW);
	double yaw = view.yaw * 360.0 / float(NUMBER_OF_ANGLES);
	double pitch = view.pitch * 360.0 / float(NUMBER_OF_ANGLES);
	pitch = (pitch > 180.0 ? pitch -360.0 : pitch);

	// setup a rotation matrix for the landscape texture shader
	// this aligns the landscapes to the center of the screen for standard
	// pitch ranges, so that they don't need to be stretched

	glLoadIdentity();
	glTranslated(view.origin.x, view.origin.y, view.origin.z);
	glRotated(yaw, 0.0, 0.0, 1.0);
	glRotated(-pitch, 0.0, 1.0, 0.0);
	glMultMatrixd(kViewBaseMatrixInverse);

	GLfloat landscapeInverseMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, landscapeInverseMatrix);

	Shader *s;

	s = Shader::get(Shader::S_Landscape);
	s->enable();
	s->setMatrix4(Shader::U_LandscapeInverseMatrix, landscapeInverseMatrix);

	s = Shader::get(Shader::S_LandscapeBloom);
	s->enable();
	s->setMatrix4(Shader::U_LandscapeInverseMatrix, landscapeInverseMatrix);

	Shader::disable();

	// setup the normal view matrix

	glLoadMatrixd(kViewBaseMatrix);
	glRotated(pitch, 0.0, 1.0, 0.0);
//	apperently 'roll' is not what i think it is
//	rubicon sets it to some strange value
//	double roll = view.roll * 360.0 / float(NUMBER_OF_ANGLES);
//	glRotated(roll, 1.0, 0.0, 0.0);
	glRotated(-yaw, 0.0, 0.0, 1.0);
	glTranslated(-view.origin.x, -view.origin.y, -view.origin.z);
}
