/*
 *  interface_sdl.cpp - Top-level game interface, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "player.h"

#include "network.h"
#include "network_sound.h"

#include "screen_drawing.h"
#include "mysound.h"
#include "preferences.h"
#include "fades.h"
#include "game_window.h"
#include "game_errors.h"
#include "screen.h"

#include "images.h"

#include "interface_menus.h"


// From shell_sdl.cpp
extern int option_level;


/*
 *  Set up and handle preferences menu
 */

void do_preferences(void)
{
	struct screen_mode_data mode = graphics_preferences->screen_mode;

	handle_preferences();

	if (mode.bit_depth != graphics_preferences->screen_mode.bit_depth) {
		paint_window_black();
		initialize_screen(&graphics_preferences->screen_mode);

		/* Re fade in, so that we get the proper colortable loaded.. */
		display_main_menu();

	} else if (memcmp(&mode, &graphics_preferences->screen_mode, sizeof(struct screen_mode_data))) {
		change_screen_mode(&graphics_preferences->screen_mode, false);
	}
}


/*
 *  Level number dialog
 */

short get_level_number_from_user(void)
{
printf("*** get_level_number_from_user()\n");
	//!!
	return option_level;
}


/*
 *  Toggle system hotkeys
 */

void toggle_menus(bool game_started)
{
	// nothing to do
}


/*
 *  Network microphone handling
 */

#if 0
static void network_speaker_proc(void *buffer, short size, short player_index)
{
	queue_network_speaker_data((byte *) buffer, size);
}
#endif

void install_network_microphone(void)
{
#if 0
	//!!
	open_network_speaker(NETWORK_SOUND_CHUNK_BUFFER_SIZE, 2);
	short id = NetAddDistributionFunction(network_speaker_proc, true);
	open_network_microphone(id);
#endif
}

void remove_network_microphone(void)
{
#if 0
	//!!
	close_network_speaker();
	close_network_microphone();
#endif
}


/*
 *  Exit networking
 */

void exit_networking(void)
{
	NetExit();
}


/*
 *  Hide/show mouse pointer
 */

void hide_cursor(void)
{
	SDL_ShowCursor(0);
}

void show_cursor(void)
{
	SDL_ShowCursor(1);
}


/*
 *  Mouse button still down?
 */

bool mouse_still_down(void)
{
	SDL_PumpEvents();
	Uint8 buttons = SDL_GetMouseState(NULL, NULL);
	return buttons & SDL_BUTTON_LMASK;
}


/*
 *  Get current mouse position
 */

void get_mouse_position(short *x, short *y)
{
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	*x = mx;
	*y = my;
}


/*
 *  Show movie
 */

void show_movie(short index)
{
printf("*** show_movie(%d)\n", index);
	//!!
}
