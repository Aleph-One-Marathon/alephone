/*
 *  game_window_sdl.cpp - HUD display, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"

#include "HUDRenderer_SW.h"

#include "shell.h"
#include "preferences.h"
#include "screen.h"
#include "screen_definitions.h"
#include "images.h"


// From sreen_sdl.cpp
extern SDL_Surface *HUD_Buffer;
extern void build_sdl_color_table(const color_table *color_table, SDL_Color *colors);

// From game_window.cpp
extern HUD_SW_Class HUD_SW;
extern bool OGL_HUDActive;

extern void draw_panels(void);


/*
 *  Draw HUD (to HUD surface)
 */

void draw_panels(void)
{
	if (OGL_HUDActive)
		return;

	// Allocate surface for HUD if not present
	if (HUD_Buffer == NULL) {
		SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 8, 0xff, 0xff, 0xff, 0xff);
		if (s == NULL)
			alert_user(fatalError, strERRORS, outOfMemory, -1);
		HUD_Buffer = SDL_DisplayFormat(s);
		if (HUD_Buffer == NULL)
			alert_user(fatalError, strERRORS, outOfMemory, -1);
		SDL_FreeSurface(s);
	}

	// Draw static HUD picture
	static SDL_Surface *static_hud_pict = NULL;
	static bool hud_pict_not_found = false;
	if (static_hud_pict == NULL && !hud_pict_not_found) {
		LoadedResource rsrc;
		if (get_picture_resource_from_images(INTERFACE_PANEL_BASE, rsrc))
			static_hud_pict = picture_to_surface(rsrc);
		else
			hud_pict_not_found = true;
	} else {
		SDL_Rect dst_rect = {0, 320, 640, 160};
		SDL_BlitSurface(static_hud_pict, NULL, HUD_Buffer, &dst_rect);
	}

	// Add dynamic elements
	_set_port_to_HUD();
	HUD_SW.update_everything(NONE);
	_restore_port();

	// Tell main loop to render the HUD in the next run
	RequestDrawingHUD();
}
