#ifndef _VIEW_CONTROL_
#define _VIEW_CONTROL_
/*
	May 22, 2000 (Loren Petrich)
	
	View controller. This controls various parameters of the viewing.
	
	The parameters are, in turn, controllable with XML.
	
	May 23, 2000 (Loren Petrich):
	
	Added field-of-view control
	
	May 24, 2000 (Loren Petrich):
	
	Added landscape control
*/

#include "world.h"
#include "shape_descriptors.h"
#include "XML_ElementParser.h"

// Returns whether or not the overhead map can possibly be active
extern bool View_MapActive();

// Accessors for field-of-view values (normal, extravision, tunnel vision):
extern float View_FOV_Normal();
extern float View_FOV_ExtraVision();
extern float View_FOV_TunnelVision();

// Move field-of-view value closer to some target value;
// returns whether or not the FOV had been changed.
extern bool View_AdjustFOV(float& FOV, float FOV_Target);


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
	LandscapeOptions(): HorizExp(1), VertExp(1), OGL_AspRatExp(1), VertRepeat(false), Azimuth(0) {}
};

extern LandscapeOptions *View_GetLandscapeOptions(shape_descriptor Desc);


// XML support:
XML_ElementParser *View_GetParser();
XML_ElementParser *Landscapes_GetParser();

#endif
