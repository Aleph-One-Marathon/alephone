/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/

/*
 *  interface_sdl.cpp - Top-level game interface, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 *
 *  5 Mar 2002 (Woody Zenfell):
 *      network_speaker and network_microphone routines actually do something now :)
 *
 *  1 Feb 2003 (Woody Zenfell):
 *      network audio stuff now handled in shared interface.cpp instead of per-platform.
 *
 *  12 Feb 2003 (Woody Zenfell):
 *	should_restore_game_networked() implementation
 */

#include "cseries.h"

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "player.h"

#include "network.h"

#include "screen_drawing.h"
#include "mysound.h"
#include "preferences.h"
#include "fades.h"
#include "game_window.h"
#include "game_errors.h"
#include "screen.h"

#include "images.h"

#include "interface_menus.h"
#include "XML_LevelScript.h"

#ifdef HAVE_SMPEG
#include <smpeg/smpeg.h>
#endif

// ZZZ: for should_restore_game_networked()
#include "sdl_dialogs.h"
#include "sdl_widgets.h"
#include "network_dialog_widgets_sdl.h"


/*
 *  Set up and handle preferences menu
 */

void do_preferences(void)
{
	struct screen_mode_data mode = graphics_preferences->screen_mode;

	handle_preferences();

	if (mode.bit_depth != graphics_preferences->screen_mode.bit_depth) {
		paint_window_black();
		initialize_screen(&graphics_preferences->screen_mode, false);

		/* Re fade in, so that we get the proper colortable loaded.. */
		display_main_menu();
	} else if (memcmp(&mode, &graphics_preferences->screen_mode, sizeof(struct screen_mode_data)))
		change_screen_mode(&graphics_preferences->screen_mode, false);
}


/*
 *  Toggle system hotkeys
 */

void toggle_menus(bool game_started)
{
	// nothing to do
}


/*
 *  Update game window
 */

void update_game_window(void)
{
	switch(get_game_state()) {
		case _game_in_progress:
			update_screen_window();
			break;
			
		case _display_quit_screens:
		case _display_intro_screens_for_demo:
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
		case _display_main_menu:
			update_interface_display();
			break;
			
		default:
			break;
	}
}


/*
 *  Exit networking
 */

void exit_networking(void)
{
#if !defined(DISABLE_NETWORKING)
	NetExit();
#endif // !defined(DISABLE_NETWORKING)
}


/*
 *  Show movie
 */

extern bool option_nosound;

void show_movie(short index)
{
#ifdef HAVE_SMPEG
	float PlaybackSize = 2;
	
	FileSpecifier IntroMovie;
	FileSpecifier *File = GetLevelMovie(PlaybackSize);

	if (!File && index == 0)
	{
		if (IntroMovie.SetNameWithPath(getcstr(temporary, strFILENAMES, filenameMOVIE)))
			File = &IntroMovie;
	}

	if (!File) return;

	SDL_Surface *s = SDL_GetVideoSurface();
	
#if defined(__APPLE__) && defined(__MACH__)
	if (!(s->flags & SDL_FULLSCREEN))
		SDL_putenv("SDL_VIDEO_YUV_HWACCEL=0");
	else
		SDL_putenv("SDL_VIDEO_YUV_HWACCEL=");

#endif
	{
		TakeSDLAudioControl sdlAudioControl;
		
		SMPEG_Info info;
		SMPEG *movie;

		movie = SMPEG_new(File->GetPath(), &info, option_nosound ? 0 : 1);
		if (!movie) return;
		if (!info.has_video) {
			SMPEG_delete(movie);
			return;
		}
		int width = (int) (info.width * PlaybackSize);
		int height = (int) (info.height * PlaybackSize);
		
		SMPEG_setdisplay(movie, SDL_GetVideoSurface(), NULL, NULL);
		if (width <= s->w && height <= s->h)
		{
			SMPEG_scaleXY(movie, width, height);
			SMPEG_move(movie, (s->w - width) / 2, (s->h - height) / 2);
		}
		
		bool done = false;
		SMPEG_play(movie);
		while (!done && SMPEG_status(movie) == SMPEG_PLAYING)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event) )
			{
				switch (event.type) {
				case SDL_KEYDOWN:
				case SDL_MOUSEBUTTONDOWN:
					done = true;
					break;
				default:
					break;
				}
			}
			
			SDL_Delay(100);
		}
		SMPEG_delete(movie);
	}
#endif // HAVE_SMPEG
}


size_t should_restore_game_networked()
{
        dialog d;
        
        d.add(new w_static_text("RESUME GAME", TITLE_FONT, TITLE_COLOR));
        d.add(new w_spacer());

        w_toggle* theRestoreAsNetgameToggle = new w_toggle("Resume as", dynamic_world->player_count > 1);
        theRestoreAsNetgameToggle->set_labels_stringset(kSingleOrNetworkStringSetID);
        d.add(theRestoreAsNetgameToggle);
        
        d.add(new w_spacer());
        d.add(new w_spacer());
        
        d.add(new w_left_button("RESUME", dialog_ok, &d));
        d.add(new w_right_button("CANCEL", dialog_cancel, &d));

        // We return -1 (NONE) for "cancel", 0 for "not networked", and 1 for "networked".
        size_t theResult;

        if(d.run() == 0)
        {
                theResult = theRestoreAsNetgameToggle->get_selection();
        }
        else
        {
                theResult = UNONE;
        }

        return theResult;
}
