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
	February 21, 2000 (Loren Petrich)
	
	Crosshairs-interface file.

Feb 25, 2000 (Loren Petrich):
	Split the rendering routines up into routines that need different parameters.

Mar 2, 2000 (Loren Petrich):	
	Moved crosshair data and configuration here from interface.h
	
Jun 26, 2002 (Loren Petrich):
	Added support for crosshairs being circular and/or partially transparent
*/

#ifndef _CROSSHAIRS
#define _CROSSHAIRS

#include "cseries.h"  // need RGBColor

struct SDL_Surface;

enum {
	CHShape_RealCrosshairs,
	CHShape_Circle
};

struct CrosshairData
{
     RGBColor Color;
	short Thickness;
	short FromCenter;
	short Length;
	short Shape;
	float Opacity;
	float GLColorsPreCalc[4];
	bool PreCalced;
};

// True for OK, false for cancel
// the structure will not be changed if this was canceled
// Implemented in PlayerDialogs.c
bool Configure_Crosshairs(CrosshairData &Data);

// Gotten from preferences
// Implemented in preferences.c
CrosshairData& GetCrosshairData();

// All these functions return the crosshairs' state (true: active; false: inactive)
bool Crosshairs_IsActive();
bool Crosshairs_SetActive(bool NewState);

bool Crosshairs_Render(SDL_Surface *s);

#endif
