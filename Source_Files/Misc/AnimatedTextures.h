/*
	February 21, 2000 (Loren Petrich)
	
	Animated-textures-interface file.
	
	May 14, 2000 (Loren Petrich)
	
	Modified the internal architecture heavily,
	renamed one of the external calls,
	and added XML-configuration support
*/

#ifndef _ANIMATED_TEXTURES
#define _ANIMATED_TEXTURES

#include "shape_descriptors.h"
#include "XML_ElementParser.h"

// Updates the animated textures
void AnimTxtr_Update();

// Does animated-texture translation.
// Note: a shape_descriptor is really a short integer
shape_descriptor AnimTxtr_Translate(shape_descriptor Texture);

// XML-parser support
XML_ElementParser *AnimatedTextures_GetParser();

#endif
