/*
	February 21, 2000 (Loren Petrich)
	
	Crosshairs-interface file.

	Feb 25, 2000 (Loren Petrich):
	
	Split the rendering routines up into routines that need different parameters.
	
	Mar 2, 2000 (Loren Petrich):
	
	Moved crosshair data and configuration here from interface.h
*/

#ifndef _CROSSHAIRS
#define _CROSSHAIRS

struct CrosshairData
{
	short Thickness;
	short FromCenter;
	short Length;
	RGBColor Color;
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

// Needs QD graphics context;
// will draw in center of view rectangle in the specified graphics context.
// If there is no view rectangle specified, then it uses the graphics context's rectangle.
// If there is no graphics context specified, then it uses the current one.
#if defined(mac)
bool Crosshairs_Render(Rect &ViewRect);
bool Crosshairs_Render(GrafPtr Context);
bool Crosshairs_Render(GrafPtr Context, Rect &ViewRect);
#elif defined(SDL)
bool Crosshairs_Render(SDL_Surface *s);
#endif

#endif
