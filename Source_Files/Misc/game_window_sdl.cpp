/*
 *  game_window_sdl.cpp - HUD display, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include "map.h"
#include "shell.h"
#include "preferences.h"
#include "screen_drawing.h"
#include "interface.h"
#include "screen.h"
#include "mysound.h" // for screen_definitions.h
#include "screen_definitions.h"
#include "images.h"


// From game_window.cpp
extern void update_everything(short time_elapsed);

// From sreen_sdl.cpp
extern SDL_Surface *HUD_Buffer;
extern void build_sdl_color_table(const color_table *color_table, SDL_Color *colors);

// From images_sdl.cpp
extern SDL_Surface *picture_to_surface(void *picture, uint32 size);


/*
 *  Draw HUD (to HUD surface)
 */

void draw_panels(void)
{
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
	uint32 size;
	void *picture = get_picture_resource_from_images(INTERFACE_PANEL_BASE, size);
	if (picture) {
		SDL_Surface *s = picture_to_surface(picture, size);
		free(picture);
		if (s) {
			SDL_Rect dst_rect = {0, 320, 640, 160};
			SDL_BlitSurface(s, NULL, HUD_Buffer, &dst_rect);
			free(s);

			// Add dynamic elements
			_set_port_to_HUD();
			update_everything(NONE);
			_restore_port();
		}
	}

	// Tell main loop to render the HUD in the next run
	RequestDrawingHUD();
}
