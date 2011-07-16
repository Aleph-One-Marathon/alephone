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
	
	Crosshairs-implementation file; contains Quickdraw crosshairs code.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
	
Jun 26, 2002 (Loren Petrich):
	Added support for crosshairs being circular and/or partially transparent
*/

#include "cseries.h"
#include "Crosshairs.h"


static bool _Crosshairs_IsActive = false;

bool Crosshairs_IsActive() {return _Crosshairs_IsActive;}
bool Crosshairs_SetActive(bool NewState) {return (_Crosshairs_IsActive = (NewState != 0));}


// Will draw in center of view rectangle in the current QD graphics context;
// modeled after fps display.
// It won't try to clip the ends of the lines.
bool Crosshairs_Render(Rect &ViewRect)
{
	if (!_Crosshairs_IsActive) return _Crosshairs_IsActive;
	
	// Get the crosshair data
	CrosshairData &Crosshairs = GetCrosshairData();
	
	// Push previous state
	PenState OldPen;
	RGBColor OldBackColor, OldForeColor;
	
	GetPenState(&OldPen);
	GetBackColor(&OldBackColor);
	GetForeColor(&OldForeColor);
	
	// Get ready to draw the crosshairs
	PenNormal();
	
	// Drawing color
	RGBForeColor(&Crosshairs.Color);

	// Get the center location from the view rectangle	
	// Shift it down one, so that even line widths can come out right.
	short XCen = ((ViewRect.left + ViewRect.right) >> 1) - 1;
	short YCen = ((ViewRect.top + ViewRect.bottom) >> 1) - 1;
	
	// Separate for X and Y; the points are three on top/left and three on top/right:
	// Line-split position
	// Octagon vertex (half-way)
	// Farthest extent
	short OctaPoints[2][6];
	short Len;
	
	switch(Crosshairs.Shape)
	{
	case CHShape_RealCrosshairs:
	
		PenSize(1,Crosshairs.Thickness);
		
		// Left
		MoveTo(XCen-Crosshairs.FromCenter+Crosshairs.Thickness-1,YCen);
		Line(-Crosshairs.Length,0);
		
		// Right
		MoveTo(XCen+Crosshairs.FromCenter,YCen);
		Line(Crosshairs.Length,0);
		
		PenSize(Crosshairs.Thickness,1);
		
		// Top
		MoveTo(XCen,YCen-Crosshairs.FromCenter+Crosshairs.Thickness-1);
		Line(0,-Crosshairs.Length);
		
		// Bottom
		MoveTo(XCen,YCen+Crosshairs.FromCenter);
		Line(0,Crosshairs.Length);
		
		break;
		
	case CHShape_Circle:
		// This will really be an octagon, for OpenGL-rendering convenience
		
		// Precalculate the line endpoints, for convenience
		
		Len = Crosshairs.Length;
		OctaPoints[0][0] = XCen - Len;
		OctaPoints[0][5] = XCen + Len;
		OctaPoints[1][0] = YCen - Len;
		OctaPoints[1][5] = YCen + Len;
		
		Len = Len/2;
		OctaPoints[0][1] = XCen - Len;
		OctaPoints[0][4] = XCen + Len;
		OctaPoints[1][1] = YCen - Len;
		OctaPoints[1][4] = YCen + Len;
		
		Len = MIN(Len,Crosshairs.FromCenter);
		OctaPoints[0][2] = XCen - Len;
		OctaPoints[0][3] = XCen + Len;
		OctaPoints[1][2] = YCen - Len;
		OctaPoints[1][3] = YCen + Len;
		
		// We need to do 12 line segments, so we do them in 2*2*3 fashion
		
		for (int ix=0; ix<2; ix++)
		{
			int ixi = (ix > 0) ? 5 : 0;
			int ixid = (ix > 0) ? -1 : 1;
			for (int iy=0; iy<2; iy++)
			{
				int iyi = (iy > 0) ? 5 : 0;
				int iyid = (iy > 0) ? -1 : 1;
				
				// Vertical
				PenSize(Crosshairs.Thickness,1);
				MoveTo(OctaPoints[0][ixi],OctaPoints[1][iyi+2*iyid]);
				LineTo(OctaPoints[0][ixi],OctaPoints[1][iyi+iyid]);
				
				// Diagonal
				PenSize(Crosshairs.Thickness,Crosshairs.Thickness);
				LineTo(OctaPoints[0][ixi+ixid],OctaPoints[1][iyi]);
				
				// Horizontal
				PenSize(1,Crosshairs.Thickness);
				LineTo(OctaPoints[0][ixi+2*ixid],OctaPoints[1][iyi]);
			}
		}
		
		break;
	}
	
	// Pop previous state
	SetPenState(&OldPen);
	RGBBackColor(&OldBackColor);
	RGBForeColor(&OldForeColor);
		
	return _Crosshairs_IsActive;
}

// If a graphics context is also specified
bool Crosshairs_Render(GrafPtr Context, Rect &ViewRect)
{
	// Push context
	GrafPtr OldContext;
	GetPort(&OldContext);
	SetPort(Context);
	
	bool RetCode = Crosshairs_Render(ViewRect);
	
	// Pop context
	SetPort(OldContext);
	
	return RetCode;
}

// If no view rectangle is explicitly specified
bool Crosshairs_Render(GrafPtr Context)
{
	Rect portRect;
	GetPortBounds(Context, &portRect);
	return Crosshairs_Render(Context, portRect);
}
