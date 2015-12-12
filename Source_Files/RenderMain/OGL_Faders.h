#ifndef _OGL_FADERS_
#define _OGL_FADERS_
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
	
	OpenGL Renderer,
	by Loren Petrich,
	May 30, 2000

	This contains code for doing fader stuff.	
*/

#include "cstypes.h"


// Indicates whether OpenGL-rendering faders will be used
bool OGL_FaderActive();

// Which kinds of faders in the fader queue?
enum
{
	FaderQueue_Liquid,
	FaderQueue_Other,
	NUMBER_OF_FADER_QUEUE_ENTRIES
};

// Fader data
struct OGL_Fader
{
	// Which type of fade to do
	short Type;
	// The three color channels and a transparency channel
	float Color[4];
	
	OGL_Fader(): Type(NONE) {}
};

// Fader=queue accessor
OGL_Fader *GetOGL_FaderQueueEntry(int Index);

// Fader renderer; returns whether or not OpenGL faders were active.
bool OGL_DoFades(float Left, float Top, float Right, float Bottom);

#endif
