#ifndef _VIEW_CONTROL_
#define _VIEW_CONTROL_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	May 22, 2000 (Loren Petrich)
	
	View controller. This controls various parameters of the viewing.
	
	The parameters are, in turn, controllable with XML.
	
	May 23, 2000 (Loren Petrich):
	
	Added field-of-view control
	
	May 24, 2000 (Loren Petrich):
	
	Added landscape control

Nov 29, 2000 (Loren Petrich):
	Added making view-folding effect optional
	Added making teleport static/fold effect optional

Dec 17, 2000 (Loren Petrich:
	Added teleport-sound control for Marathon 1 compatibility
*/

#include "world.h"
#include "FontHandler.h"
#include "shape_descriptors.h"

// Returns whether or not the overhead map can possibly be active
bool View_MapActive();

// Accessors for field-of-view values (normal, extravision, tunnel vision):
float View_FOV_Normal();
float View_FOV_ExtraVision();
float View_FOV_TunnelVision();

// Move field-of-view value closer to some target value;
// returns whether or not the FOV had been changed.
bool View_AdjustFOV(float& FOV, float FOV_Target);

// Indicates whether to fix the horizontal or the vertical field-of-view angle
// (default: fix vertical FOV angle)
bool View_FOV_FixHorizontalNotVertical();

// Indicates whether to do fold-in/fold-out effect when one is teleporting
bool View_DoFoldEffect();

// Indicates whether to do the "static" effect when one is teleporting
bool View_DoStaticEffect();

// Indicates whether to skip all teleport effects teleporting into a level
bool View_DoInterlevelTeleportInEffects();

// Indicates whether to skip all teleport effects teleporting out of a level
bool View_DoInterlevelTeleportOutEffects();

// Gets the on-screen-display font
FontSpecifier& GetOnScreenFont();

// Landscape stuff

struct LandscapeOptions
{
	// 2^(HorizExp) is the number of texture repeats when going in a circle;
	// it is a horizontal scaling factor
	short HorizExp;
	// 2^(VertExp) is a vertical scaling factor, which creates an amount of scaling
	// equal to the corresponding horizontal scaling factor.
	short VertExp;
	// Aspect-ratio exponent to use in OpenGL rendering;
	// (height) = 2^(-OGL_AspRatExp)*(width).
	// Necessary because OpenGL prefers powers of 2, and Bungie's landscapes have heights
	// that are not powers of 2.
	short OGL_AspRatExp;
	// Whether the texture repeats in the vertical direction (true: like Marathon 1)
	// or gets clamped in the vertical direction (false: like Marathon 2/oo)
	bool VertRepeat;
	// This is the azimuth or yaw (full circle = 512);
	// the texture is shifted leftward, relative to view direction, by this amount.
	angle Azimuth;
	
	// Constructor: sets everything to defaults appropriate for standard textures
	// Same scale for horizontal and vertical, 2^1 = 2 repeats,
	// OpenGL hight is half width, and the azimuth is zero
	LandscapeOptions(): HorizExp(1), VertExp(1), OGL_AspRatExp(0), VertRepeat(false), Azimuth(0) {}
};

LandscapeOptions *View_GetLandscapeOptions(shape_descriptor Desc);


class InfoTree;
void parse_mml_view(const InfoTree& root);
void reset_mml_view();
void parse_mml_landscapes(const InfoTree& root);
void reset_mml_landscapes();

#endif
