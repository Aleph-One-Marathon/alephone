/*
 *  Crosshairs_SDL.cpp - Crosshairs display, SDL implementation
 */

#include "cseries.h"
#include "Crosshairs.h"


/*
 *  Crosshairs status
 */

static bool _Crosshairs_IsActive = false;

bool Crosshairs_IsActive(void)
{
	return _Crosshairs_IsActive;
}

bool Crosshairs_SetActive(bool NewState)
{
	return _Crosshairs_IsActive = NewState;
}


/*
 *  Draw crosshairs in center of surface
 */

bool Crosshairs_Render(SDL_Surface *s)
{
	if (!_Crosshairs_IsActive)
		return false;

	// Get the crosshair data
	CrosshairData &Crosshairs = GetCrosshairData();

	// Get color
	uint32 pixel = SDL_MapRGB(s->format, Crosshairs.Color.red >> 8, Crosshairs.Color.green >> 8, Crosshairs.Color.blue >> 8);

	// Get coordinates
	int xcen = s->w / 2 - 1, ycen = s->h / 2 - 1;

	// Left
	SDL_Rect r = {xcen - Crosshairs.FromCenter - Crosshairs.Length, ycen - Crosshairs.Thickness / 2, Crosshairs.Length, Crosshairs.Thickness};
	SDL_FillRect(s, &r, pixel);

	// Right
	r.x = xcen + Crosshairs.FromCenter;
	SDL_FillRect(s, &r, pixel);

	// Top
	r.x = xcen - Crosshairs.Thickness / 2;
	r.y = ycen - Crosshairs.FromCenter - Crosshairs.Length;
	r.w = Crosshairs.Thickness;
	r.h = Crosshairs.Length;
	SDL_FillRect(s, &r, pixel);

	// Bottom
	r.y = ycen + Crosshairs.FromCenter;
	SDL_FillRect(s, &r, pixel);
	return true;
}
