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
 *  screen_sdl.cpp - Screen management, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 * 
 *  Loren Petrich, Dec 23, 2000; moved shared content into screen_shared.cpp
 */

#include "cseries.h"

#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_OPENGL
#include "SDL_opengl.h"
#include "OGL_Blitter.h"
#endif

#include "world.h"
#include "map.h"
#include "render.h"
#include "shell.h"
#include "interface.h"
#include "player.h"
#include "overhead_map.h"
#include "fades.h"
#include "game_window.h"
#include "screen.h"
#include "preferences.h"
#include "computer_interface.h"
#include "Crosshairs.h"
#include "OGL_Render.h"
#include "ViewControl.h"
#include "screen_drawing.h"
#include "mouse.h"
#include "network.h"
#include "images.h"
#include "motion_sensor.h"

#include "sdl_fonts.h"

#include "lua_script.h"
#include "lua_hud_script.h"
#include "HUDRenderer_Lua.h"

#include <algorithm>

#if defined(__WIN32__) || (defined(__MACH__) && defined(__APPLE__)) || defined(__MACOS__)
#define MUST_RELOAD_VIEW_CONTEXT
#endif

// Global variables
static SDL_Surface *main_surface;	// Main (display) surface

// Rendering buffer for the main view, the overhead map, and the terminals.
// The HUD has a separate buffer.
// It is initialized to NULL so as to allow its initing to be lazy.
SDL_Surface *world_pixels = NULL;
SDL_Surface *HUD_Buffer = NULL;
SDL_Surface *Term_Buffer = NULL;

#ifdef HAVE_OPENGL
static OGL_Blitter Term_Blitter;
#endif

// Initial gamma table
bool default_gamma_inited = false;
uint16 default_gamma_r[256];
uint16 default_gamma_g[256];
uint16 default_gamma_b[256];

static bool PrevFullscreen = false;
static bool in_game = false;	// Flag: menu (fixed 640x480) or in-game (variable size) display

static int desktop_width;
static int desktop_height;

static int prev_width;
static int prev_height;

// From shell_sdl.cpp
extern bool option_nogamma;

#include "screen_shared.h"

using namespace alephone;

Screen Screen::m_instance;


// Prototypes
static void change_screen_mode(int width, int height, int depth, bool nogl);
static void build_sdl_color_table(const color_table *color_table, SDL_Color *colors);
static void reallocate_world_pixels(int width, int height);
static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez);
static void update_fps_display(SDL_Surface *s);
static void DisplayPosition(SDL_Surface *s);
static void DisplayMessages(SDL_Surface *s);
static void DisplayNetMicStatus(SDL_Surface *s);
static void DrawSurface(SDL_Surface *s, SDL_Rect &dest_rect, SDL_Rect &src_rect);
static void clear_screen_margin();

// LP addition:
void start_tunnel_vision_effect(
	bool out)
{
	// LP change: doing this by setting targets
  world_view->target_field_of_view = (out && NetAllowTunnelVision()) ? TUNNEL_VISION_FIELD_OF_VIEW : 
		((current_player->extravision_duration) ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW);
}

/*
 *  Initialize screen management
 */

void Screen::Initialize(screen_mode_data* mode)
//void initialize_screen(struct screen_mode_data *mode, bool ShowFreqDialog)
{
	interface_bit_depth = bit_depth = mode->bit_depth;

	if (!screen_initialized) {

		uncorrected_color_table = (struct color_table *)malloc(sizeof(struct color_table));
		world_color_table = (struct color_table *)malloc(sizeof(struct color_table));
		visible_color_table = (struct color_table *)malloc(sizeof(struct color_table));
		interface_color_table = (struct color_table *)malloc(sizeof(struct color_table));
		assert(uncorrected_color_table && world_color_table && visible_color_table && interface_color_table);
		memset(uncorrected_color_table, 0, sizeof(struct color_table));
		memset(world_color_table, 0, sizeof(struct color_table));
		memset(visible_color_table, 0, sizeof(struct color_table));
		memset(interface_color_table, 0, sizeof(struct color_table));

		// Allocate the bitmap_definition structure for our GWorld (it is reinitialized every frame)
		world_pixels_structure = (struct bitmap_definition *)malloc(sizeof(struct bitmap_definition) + sizeof(pixel8 *) * MAXIMUM_WORLD_HEIGHT);
		assert(world_pixels_structure);

		// Allocate and initialize our view_data structure
		world_view = (struct view_data *)malloc(sizeof(struct view_data));
		assert(world_view);
		world_view->field_of_view = NORMAL_FIELD_OF_VIEW; // degrees (was 74 for a long, long time)
		world_view->target_field_of_view = NORMAL_FIELD_OF_VIEW; // for no change in FOV
		world_view->overhead_map_scale = DEFAULT_OVERHEAD_MAP_SCALE;
		world_view->overhead_map_active = false;
		world_view->terminal_mode_active = false;
		world_view->horizontal_scale = 1;
		world_view->vertical_scale = 1;
		world_view->tunnel_vision_active = false;
#if SDL_VERSION_ATLEAST(1, 2, 10)
		desktop_height = SDL_GetVideoInfo()->current_h;
		desktop_width = SDL_GetVideoInfo()->current_w;
#endif

		// build a list of fullscreen modes
		// list some modes
		SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
		if (modes)
		{
			for (int i = 0; modes[i]; ++i)
			{
				if (modes[i]->w >= 640 && modes[i]->h >= 480)
				{
					m_modes.push_back(std::pair<int, int>(modes[i]->w, modes[i]->h));
					if (modes[i]->w == 640 && modes[i]->h == 480)
					{
						m_modes.push_back(std::pair<int, int>(480, 240));
						m_modes.push_back(std::pair<int, int>(320, 160));
					}
				}
			}
		}

		if (m_modes.empty())
		{
			m_modes.push_back(std::pair<int, int>(640, 480));
			m_modes.push_back(std::pair<int, int>(480, 240));
			m_modes.push_back(std::pair<int, int>(320, 160));
		} 
		else if (m_modes.size() == 1)
		{
			// insert some common window sizes
			std::vector<std::pair<int, int> > common_modes;
			common_modes.push_back(std::pair<int, int>(1600, 1200));
			common_modes.push_back(std::pair<int, int>(1280, 1024));
			common_modes.push_back(std::pair<int, int>(1280, 960));
			common_modes.push_back(std::pair<int, int>(1024, 768));
			common_modes.push_back(std::pair<int, int>(800, 600));
			
			for (std::vector<std::pair<int, int> >::const_iterator it = common_modes.begin(); it != common_modes.end(); ++it)
			{
				if (it->first <= m_modes[0].first && it->second <= m_modes[0].second && !(it->first == m_modes[0].first && it->second == m_modes[0].second))
				{
					m_modes.push_back(*it);
				}
			}

			m_modes.push_back(std::pair<int, int>(640, 480));
			m_modes.push_back(std::pair<int, int>(480, 240));
			m_modes.push_back(std::pair<int, int>(320, 160));
		}

		// insert custom mode if it's in the prefs
		if (graphics_preferences->screen_mode.width <= m_modes[0].first && graphics_preferences->screen_mode.height <= m_modes[0].second)
		{
			// sort it into the list
			for (std::vector<std::pair<int, int> >::iterator it = m_modes.begin(); it != m_modes.end(); ++it)
			{
				if (graphics_preferences->screen_mode.width >= it->first && graphics_preferences->screen_mode.height >= it->second)
				{
					if (graphics_preferences->screen_mode.width != it->first || graphics_preferences->screen_mode.height != it->second)
					{
						m_modes.insert(it, std::pair<int, int>(graphics_preferences->screen_mode.width, graphics_preferences->screen_mode.height));
					}
					break;
				}
			}
		}

		// these are not validated in graphics prefs because
		// SDL is not initialized yet when prefs load, so
		// validate them here
		if (Screen::instance()->FindMode(graphics_preferences->screen_mode.width, graphics_preferences->screen_mode.height) < 0)
		{
			graphics_preferences->screen_mode.width = 640;
			graphics_preferences->screen_mode.height = 480;
			write_preferences();
		}
	} else {

		unload_all_collections();
		if (world_pixels)
			SDL_FreeSurface(world_pixels);
	}
	world_pixels = NULL;

	// Set screen to 640x480 without OpenGL for menu
	screen_mode = *mode;
	change_screen_mode(640, 480, bit_depth, true);
	screen_initialized = true;

}

int Screen::height()
{
	return SDL_GetVideoSurface()->h;
}

int Screen::width()
{
	return SDL_GetVideoSurface()->w;
}

int Screen::window_height()
{
	return std::max(static_cast<short>(480), screen_mode.height);
}

int Screen::window_width()
{
	return std::max(static_cast<short>(640), screen_mode.width);
}

bool Screen::hud()
{
	return screen_mode.hud;
}

bool Screen::lua_hud()
{
	return screen_mode.hud && environment_preferences->use_hud_lua;
}

bool Screen::openGL()
{
	return screen_mode.acceleration == _opengl_acceleration;
}

bool Screen::fifty_percent()
{
	return screen_mode.height == 160;
}

bool Screen::seventyfive_percent()
{
	return screen_mode.height == 240;;
}

SDL_Rect Screen::window_rect()
{
	SDL_Rect r;
	r.w = window_width();
	r.h = window_height();
	r.x = (width() - r.w) / 2;
	r.y = (height() - r.h) / 2;
	return r;
}

SDL_Rect Screen::view_rect()
{
	SDL_Rect r;
	if (lua_hud())
	{
		r.x = lua_view_rect.x + (width() - window_width()) / 2;
		r.y = lua_view_rect.y + (height() - window_height()) / 2;
		r.w = MIN(lua_view_rect.w, window_width() - lua_view_rect.x);
		r.h = MIN(lua_view_rect.h, window_height() - lua_view_rect.y);
	}
	else if (!hud())
	{
		r.x = (width() - window_width()) / 2;
		r.y = (height() - window_height()) / 2;
		r.w = window_width();
		r.h = window_height();
	}
	else
	{
		int available_height = window_height() - hud_rect().h;
		if (window_width() > available_height * 2)
		{
			r.w = available_height * 2;
			r.h = available_height;
		}
		else
		{
			r.w = window_width();
			r.h = window_width() / 2;
		}
		r.x = (width() - r.w) / 2;
		r.y = (height() - window_height()) / 2 + (available_height - r.h) / 2;
	}

	if (fifty_percent())
	{
		r.y += r.h / 4;
		r.x += r.w / 4;
		r.w /= 2;
		r.h /= 2;
	}
	else if (seventyfive_percent())
	{
		r.y += r.h / 8;
		r.x += r.w / 8;
		r.w = r.w * 3 / 4;
		r.h = r.h * 3 / 4;
	}

	return r;
}

SDL_Rect Screen::map_rect()
{
	SDL_Rect r;
	if (lua_hud())
    {
		r.x = lua_map_rect.x + (width() - window_width()) / 2;
		r.y = lua_map_rect.y + (height() - window_height()) / 2;
		r.w = MIN(lua_map_rect.w, window_width() - lua_map_rect.x);
		r.h = MIN(lua_map_rect.h, window_height() - lua_map_rect.y);
        return r;
    }
	
	r.w = window_width();
	r.h = window_height();
	if (hud()) 
		r.h -= hud_rect().h;

	r.x = (width() - window_width()) / 2;
	r.y = (height() - window_height()) / 2;

	return r;
}

SDL_Rect Screen::term_rect()
{
	int wh = window_height();
	int ww = window_width();
	int wx = (width() - ww)/2;
	int wy = (height() - wh)/2;
	
	if (lua_hud())
	{
		wx += lua_term_rect.x;
		wy += lua_term_rect.y;
		ww = MIN(lua_term_rect.w, ww - lua_term_rect.x);
		wh = MIN(lua_term_rect.h, wh - lua_term_rect.y);
	}
	
	SDL_Rect r;
	
	int available_height = wh;
	if (hud() && !lua_hud())
		available_height -= hud_rect().h;
	
	r.w = 640;
	switch (screen_mode.term_scale_level)
	{
		case 1:
			if (available_height >= 640 && ww >= 1280)
				r.w *= 2;
			break;
		case 2:
			r.w = std::min(ww, std::max(640, 2 * available_height));
			break;
	}
	r.h = r.w / 2;
	r.x = wx + (ww - r.w) / 2;
	r.y = wy + (available_height - r.h) / 2;

	return r;
}

SDL_Rect Screen::hud_rect()
{
	SDL_Rect r;
	r.w = 640;
	switch (screen_mode.hud_scale_level)
	{
		case 1:
			if (window_height() >= 960 && window_width() >= 1280)
				r.w *= 2;
			break;
		case 2:
			r.w = std::min(window_width(), std::max(640, 4 * window_height() / 3));
			break;
	}
	r.h = r.w / 4;
	r.x = (width() - r.w) / 2;
	r.y = window_height() - r.h + (height() - window_height()) / 2;

	return r;
}

/*
 *  (Re)allocate off-screen buffer
 */

static void reallocate_world_pixels(int width, int height)
{
	if (world_pixels) {
		SDL_FreeSurface(world_pixels);
		world_pixels = NULL;
	}
	SDL_PixelFormat *f = main_surface->format;
	world_pixels = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, f->BitsPerPixel, f->Rmask, f->Gmask, f->Bmask, f->Amask);
	if (world_pixels == NULL)
		alert_user(fatalError, strERRORS, outOfMemory, -1);
	else if (bit_depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(world_color_table, colors);
		SDL_SetColors(world_pixels, colors, 0, 256);
	}
}


/*
 *  Force reload of view context
 */

void ReloadViewContext(void)
{
#ifdef HAVE_OPENGL
	if (screen_mode.acceleration == _opengl_acceleration)
		OGL_StartRun();
#endif
}


/*
 *  Enter game screen
 */

void enter_screen(void)
{
	if (world_view->overhead_map_active)
		set_overhead_map_status(false);
	if (world_view->terminal_mode_active)
		set_terminal_status(false);

	// Adding this view-effect resetting here since initialize_world_view() no longer resets it
	world_view->effect = NONE;
	
	// Set screen to selected size
	in_game = true;
	change_screen_mode(&screen_mode, true);
	PrevFullscreen = screen_mode.fullscreen;

#if defined(HAVE_OPENGL) && !defined(MUST_RELOAD_VIEW_CONTEXT)
	// if MUST_RELOAD_VIEW_CONTEXT, we know this just happened in
	// change_screen_mode
	if (screen_mode.acceleration == _opengl_acceleration)
		OGL_StartRun();
#endif

	// Reset modifier key status
	SDL_SetModState(KMOD_NONE);
	
	Screen *scr = Screen::instance();
	int w = scr->width();
	int h = scr->height();
	int ww = scr->window_width();
	int wh = scr->window_height();
	
	scr->lua_clip_rect.x = 0;
	scr->lua_clip_rect.y = 0;
	scr->lua_clip_rect.w = w;
	scr->lua_clip_rect.h = h;
	
	scr->lua_view_rect.x = scr->lua_map_rect.x = (w - ww) / 2;
	scr->lua_view_rect.y = scr->lua_map_rect.y = (h - wh) / 2;
	scr->lua_view_rect.w = scr->lua_map_rect.w = ww;
	scr->lua_view_rect.h = scr->lua_map_rect.h = wh;
	
	scr->lua_term_rect.x = (w - 640) / 2;
	scr->lua_term_rect.y = (h - 320) / 2;
	scr->lua_term_rect.w = 640;
	scr->lua_term_rect.h = 320;

	L_Call_HUDResize();
}


/*
 *  Exit game screen
 */

void exit_screen(void)
{
	// Return to 640x480 without OpenGL
	in_game = false;
	change_screen_mode(640, 480, bit_depth, true);
#ifdef HAVE_OPENGL
	OGL_StopRun();
#endif
}


/*
 *  Change screen mode
 */

static void change_screen_mode(int width, int height, int depth, bool nogl)
{

	int vmode_height = (screen_mode.fullscreen && !screen_mode.fill_the_screen) ? desktop_height : height;
	int vmode_width = (screen_mode.fullscreen && !screen_mode.fill_the_screen) ? desktop_width : width;
	uint32 flags = (screen_mode.fullscreen ? SDL_FULLSCREEN : 0);
#ifdef HAVE_OPENGL
	if (!nogl && screen_mode.acceleration == _opengl_acceleration) {
		flags |= SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if SDL_VERSION_ATLEAST(1,2,6)
		if (Get_OGL_ConfigureData().Multisamples > 0) {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Get_OGL_ConfigureData().Multisamples);
		} else {
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		}
#endif
#if SDL_VERSION_ATLEAST(1,2,10)
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, Get_OGL_ConfigureData().WaitForVSync ? 1 : 0);
#endif
	} else 
#endif 
	if (nogl || Screen::instance()->lua_hud()) {
		flags |= SDL_SWSURFACE;
	} else {
		flags |= SDL_HWSURFACE | SDL_HWPALETTE;
	}
	
	main_surface = SDL_SetVideoMode(vmode_width, vmode_height, depth, flags);
#ifdef HAVE_OPENGL
#if SDL_VERSION_ATLEAST(1,2,6)
	if (main_surface == NULL && !nogl && screen_mode.acceleration == _opengl_acceleration && Get_OGL_ConfigureData().Multisamples > 0) {
		// retry with multisampling off
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		main_surface = SDL_SetVideoMode(vmode_width, vmode_height, depth, flags);
	}
#endif
#endif

	if (main_surface == NULL) {
		fprintf(stderr, "Can't open video display (%s)\n", SDL_GetError());
#ifdef HAVE_OPENGL
		fprintf(stderr, "WARNING: Failed to initialize OpenGL with 24 bit colour\n");
		fprintf(stderr, "WARNING: Retrying with 16 bit colour\n");
		
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
 		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

		main_surface = SDL_SetVideoMode(vmode_width, vmode_height, depth, flags);
 		if (main_surface == NULL) {
 			fprintf(stderr, "Can't open video display (%s)\n", SDL_GetError());
 			exit(1);
 		}
#else
	exit(1);
#endif
	}
#ifdef MUST_RELOAD_VIEW_CONTEXT
	if (!nogl && screen_mode.acceleration == _opengl_acceleration) 
		ReloadViewContext();
#endif
	if (depth == 8) {
	        SDL_Color colors[256];
		build_sdl_color_table(interface_color_table, colors);
		SDL_SetColors(main_surface, colors, 0, 256);
	}
	if (HUD_Buffer) {
		SDL_FreeSurface(HUD_Buffer);
		HUD_Buffer = NULL;
	}
	if (Term_Buffer) {
		SDL_FreeSurface(Term_Buffer);
		Term_Buffer = NULL;
	}
#ifdef ALEPHONE_LITTLE_ENDIAN
	Term_Buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 320, 32, 0x000000ff,0x0000ff00, 0x00ff0000, 0xff000000);
#else
	Term_Buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 320, 32, 0xff000000,0x00ff0000, 0x0000ff00, 0x000000ff);
#endif
#ifdef HAVE_OPENGL
	if (main_surface->flags & SDL_OPENGL) {
		static bool gl_info_printed = false;
		if (!gl_info_printed)
		{
			printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
			printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
			printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
//		const char *gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
//		printf("GL_EXTENSIONS: %s\n", gl_extensions);
			gl_info_printed = true;
		}
		glScissor(0, 0, width, height);
		glViewport(0, 0, width, height);
#ifdef __WIN32__
		clear_screen();
#endif
	}
#endif
	
	if (in_game && screen_mode.hud)
	{
		if (prev_width != width || prev_height != height)
		{
			prev_width = width;
			prev_height = height;
			L_Call_HUDResize();
	  }
	}
}

void change_screen_mode(struct screen_mode_data *mode, bool redraw)
{
	// Get the screen mode here
	screen_mode = *mode;

	// "Redraw" change now and clear the screen
	if (redraw) {
		change_screen_mode(std::max(mode->width, static_cast<short>(640)), std::max(mode->height, static_cast<short>(480)), mode->bit_depth, false);
		clear_screen();
		recenter_mouse();
	}

	frame_count = frame_index = 0;
}

void toggle_fullscreen(bool fs)
{
	if (fs != screen_mode.fullscreen) {
		screen_mode.fullscreen = fs;
		if (in_game)
			change_screen_mode(&screen_mode, true);
		else {
		  change_screen_mode(640, 480, bit_depth, true);
		  clear_screen();
		}
	}
}

void toggle_fill_the_screen(bool fill_the_screen)
{
	if (fill_the_screen != screen_mode.fill_the_screen) {
		screen_mode.fill_the_screen = fill_the_screen;
		if (in_game)
			change_screen_mode(&screen_mode, true);
		else {
			change_screen_mode(640, 480, bit_depth, true);
			clear_screen();
		}
	}
}

void toggle_fullscreen()
{
  toggle_fullscreen(! screen_mode.fullscreen);
  if (!in_game) {
    update_game_window();
  } 
}


/*
 *  Render game screen
 */

static bool clear_next_screen = false;

void render_screen(short ticks_elapsed)
{
	// Make whatever changes are necessary to the world_view structure based on whichever player is frontmost
	world_view->ticks_elapsed = ticks_elapsed;
	world_view->tick_count = dynamic_world->tick_count;
	world_view->yaw = current_player->facing;
	world_view->pitch = current_player->elevation;
	world_view->maximum_depth_intensity = current_player->weapon_intensity;
	world_view->shading_mode = current_player->infravision_duration ? _shading_infravision : _shading_normal;

	// Suppress the overhead map if desired
	if (PLAYER_HAS_MAP_OPEN(current_player) && View_MapActive()) {
		if (!world_view->overhead_map_active)
			set_overhead_map_status(true);
	} else {
		if (world_view->overhead_map_active)
			set_overhead_map_status(false);
	}

	if(player_in_terminal_mode(current_player_index)) {
		if (!world_view->terminal_mode_active)
			set_terminal_status(true);
	} else {
		if (world_view->terminal_mode_active)
			set_terminal_status(false);
	}

	// Set rendering-window bounds for the current sort of display to render
	screen_mode_data *mode = &screen_mode;
	bool HighResolution;

	SDL_Rect HUD_DestRect = Screen::instance()->hud_rect();
	SDL_Rect ViewRect;

	bool ChangedSize = false;

	// Switching fullscreen mode requires much the same reinitialization as switching the screen size
	if (mode->fullscreen != PrevFullscreen) {
		PrevFullscreen = mode->fullscreen;
		ChangedSize = true;
	}

	// Each kind of display needs its own size
	if (world_view->terminal_mode_active) {
		// Standard terminal size
		ViewRect.x = ViewRect.y = 0;
		ViewRect.w = 640;
		ViewRect.h = 320;
		HighResolution = true;
	} else if (world_view->overhead_map_active) {
		// Fill the available space
		ViewRect = Screen::instance()->map_rect();
		HighResolution = true;
	} else {
		ViewRect = Screen::instance()->view_rect();
		HighResolution = mode->high_resolution;
	}
	
	static SDL_Rect PrevViewRect = { 0, 0, 0, 0};
	if (memcmp(&PrevViewRect, &ViewRect, sizeof(SDL_Rect)))
	{
		ChangedSize = true;
		PrevViewRect = ViewRect;
	}

	SDL_Rect BufferRect = {0, 0, ViewRect.w, ViewRect.h};
	// Now the buffer rectangle; be sure to shrink it as appropriate
	if (!HighResolution && screen_mode.acceleration == _no_acceleration) {
		BufferRect.w >>= 1;
		BufferRect.h >>= 1;
	}

	// Set up view data appropriately
	world_view->screen_width = BufferRect.w;
	world_view->screen_height = BufferRect.h;
	world_view->standard_screen_width = 2 * BufferRect.h;
	initialize_view_data(world_view);

	if (world_pixels) {
		// Check on the drawing buffer's size
		if (world_pixels->w != BufferRect.w || world_pixels->h != BufferRect.h)
			ChangedSize = true;
	} else
		ChangedSize = true;

	bool update_full_screen = false;
	if (ChangedSize) {
		clear_screen(false);
		update_full_screen = true;
		if (Screen::instance()->hud() && !Screen::instance()->lua_hud())
			draw_interface();

		// Reallocate the drawing buffer
		reallocate_world_pixels(BufferRect.w, BufferRect.h);

		dirty_terminal_view(current_player_index);
	} 
	else if (screen_mode.acceleration == _opengl_acceleration && clear_next_screen)
	{
		clear_screen(false);
		update_full_screen = true;
		if (Screen::instance()->hud() && !Screen::instance()->lua_hud())
			draw_interface();

		clear_next_screen = false;
	}

	switch (screen_mode.acceleration) {
		case _opengl_acceleration:
			// If we're using the overhead map, fall through to no acceleration
			if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
				break;
		case _no_acceleration:
			world_pixels_structure->width = world_view->screen_width;
			world_pixels_structure->height = world_view->screen_height;
			world_pixels_structure->bytes_per_row = world_pixels->pitch;
			world_pixels_structure->flags = 0;
			world_pixels_structure->bit_depth = bit_depth;
			world_pixels_structure->row_addresses[0] = (pixel8 *)world_pixels->pixels;

			//!! set world_pixels to VoidColor to avoid smearing?

			precalculate_bitmap_row_addresses(world_pixels_structure);
			break;
		default:
			assert(false);
			break;
	}

	world_view->origin = current_player->camera_location;
	world_view->origin_polygon_index = current_player->camera_polygon_index;

	// Script-based camera control
	if (!UseLuaCameras())
		world_view->show_weapons_in_hand = !ChaseCam_GetPosition(world_view->origin, world_view->origin_polygon_index, world_view->yaw, world_view->pitch);

#ifdef HAVE_OPENGL
	// Is map to be drawn with OpenGL?
	if (OGL_IsActive() && world_view->overhead_map_active)
		OGL_MapActive = true;
	else
		OGL_MapActive = false;

	// Set OpenGL viewport to world view
	Rect sr = {0, 0, Screen::instance()->height(), Screen::instance()->width()};
	Rect vr = {ViewRect.y, ViewRect.x, ViewRect.y + ViewRect.h, ViewRect.x + ViewRect.w};
	OGL_SetWindow(sr, vr, true);
	
#endif

    // clear Lua drawing from previous frame
    // (GL must do this before render_view)
    if (screen_mode.acceleration == _opengl_acceleration && Screen::instance()->lua_hud())
        clear_screen_margin();
    
	// Render world view
	render_view(world_view, world_pixels_structure);

    // clear Lua drawing from previous frame
    // (SDL is slower if we do this before render_view)
    if (screen_mode.acceleration != _opengl_acceleration && Screen::instance()->lua_hud())
        clear_screen_margin();
    
	// Render crosshairs
	if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
	  if (NetAllowCrosshair())
	    if (Crosshairs_IsActive())
#ifdef HAVE_OPENGL
			if (!OGL_RenderCrosshairs())
#endif
				Crosshairs_Render(world_pixels);

	// Display FPS and position
	if (!world_view->terminal_mode_active) {
	  extern bool chat_input_mode;
	  if (!chat_input_mode){
		update_fps_display(world_pixels);
	  }
	  DisplayPosition(world_pixels);
	  DisplayNetMicStatus(world_pixels);
	  DisplayScores(world_pixels);
	}
	DisplayMessages(world_pixels);
	DisplayInputLine(world_pixels);
	
#ifdef HAVE_OPENGL
	// Set OpenGL viewport to whole window (so HUD will be in the right position)
	OGL_SetWindow(sr, sr, true);
#endif
	

	// If the main view is not being rendered in software but OpenGL is active,
	// then blit the software rendering to the screen
	if (screen_mode.acceleration == _opengl_acceleration) {
#ifdef HAVE_OPENGL
		if (Screen::instance()->hud()) {
			if (Screen::instance()->lua_hud())
				Lua_DrawHUD(ticks_elapsed);
			else {
				Rect dr = {HUD_DestRect.y, HUD_DestRect.x, HUD_DestRect.y + HUD_DestRect.h, HUD_DestRect.x + HUD_DestRect.w};
				OGL_DrawHUD(dr, ticks_elapsed);
			}
		}
		
		if (world_view->terminal_mode_active) {
			// Copy 2D rendering to screen

			if (Term_RenderRequest) {
				SDL_SetAlpha(Term_Buffer, 0, 0xff);
				Term_Blitter.Load(*Term_Buffer);
				Term_RenderRequest = false;
			}
			Term_Blitter.Draw(Screen::instance()->term_rect());
		}

#endif
	} else {
		// Update world window
		if (!world_view->terminal_mode_active)
			update_screen(BufferRect, ViewRect, HighResolution);
		
		// Update HUD
		if (Screen::instance()->lua_hud())
		{
			Lua_DrawHUD(ticks_elapsed);
		}
		else if (HUD_RenderRequest) {
			SDL_Rect src_rect = { 0, 320, 640, 160 };
			DrawSurface(HUD_Buffer, HUD_DestRect, src_rect);
			HUD_RenderRequest = false;
		}

		// Update terminal
		if (world_view->terminal_mode_active) {
			if (Term_RenderRequest || Screen::instance()->lua_hud()) {
				SDL_Rect dest_rect = Screen::instance()->term_rect();
				DrawSurface(Term_Buffer, dest_rect, ViewRect);
				Term_RenderRequest = false;
			}
		}

		if (update_full_screen || Screen::instance()->lua_hud())
		{
			SDL_UpdateRect(main_surface, 0, 0, 0, 0);
			update_full_screen = false;
		}
		else
		{
			SDL_UpdateRects(main_surface, 1, &ViewRect);
		}
	}

#ifdef HAVE_OPENGL
	// Swap OpenGL double-buffers
	if (screen_mode.acceleration == _opengl_acceleration)
		OGL_SwapBuffers();
#endif
}


/*
 *  Blit world view to screen
 */

template <class T>
static inline void quadruple_surface(const T *src, int src_pitch, T *dst, int dst_pitch, const SDL_Rect &dst_rect)
{
	int width = dst_rect.w / 2;
	int height = dst_rect.h / 2;
	dst += dst_rect.y * dst_pitch / sizeof(T) + dst_rect.x;
	T *dst2 = dst + dst_pitch / sizeof(T);

	while (height-- > 0) {
		for (int x=0; x<width; x++) {
			T p = src[x];
			dst[x * 2] = dst[x * 2 + 1] = p;
			dst2[x * 2] = dst2[x * 2 + 1] = p;
		}
		src += src_pitch / sizeof(T);
		dst += dst_pitch * 2 / sizeof(T);
		dst2 += dst_pitch * 2 / sizeof(T);
	}
}

static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez)
{
	if (hi_rez) {
		SDL_BlitSurface(world_pixels, NULL, main_surface, &destination);
	} else {
	  if (SDL_MUSTLOCK(main_surface)) {
	    if (SDL_LockSurface(main_surface) < 0) return;
	  }
		switch (world_pixels->format->BytesPerPixel) {
			case 1:
				quadruple_surface((pixel8 *)world_pixels->pixels, world_pixels->pitch, (pixel8 *)main_surface->pixels, main_surface->pitch, destination);
				break;
			case 2:
				quadruple_surface((pixel16 *)world_pixels->pixels, world_pixels->pitch, (pixel16 *)main_surface->pixels, main_surface->pitch, destination);
				break;
			case 4:
				quadruple_surface((pixel32 *)world_pixels->pixels, world_pixels->pitch, (pixel32 *)main_surface->pixels, main_surface->pitch, destination);
				break;
		}
		
		if (SDL_MUSTLOCK(main_surface)) {
		  SDL_UnlockSurface(main_surface);
		}
	}
//	SDL_UpdateRects(main_surface, 1, &destination);
}


/*
 *  Update game display if it was overdrawn
 */

void update_screen_window(void)
{
	draw_interface();
	assert_world_color_table(interface_color_table, world_color_table);
}


/*
 *  Color table handling
 */

static void build_sdl_color_table(const color_table *color_table, SDL_Color *colors)
{
	const rgb_color *src = color_table->colors;
	SDL_Color *dst = colors;
	for (int i=0; i<color_table->color_count; i++) {
		dst->r = src->red >> 8;
		dst->g = src->green >> 8;
		dst->b = src->blue >> 8;
		src++; dst++;
	}
}

void initialize_gamma(void)
{
	if (!default_gamma_inited) {
		SDL_GetGammaRamp(default_gamma_r, default_gamma_g, default_gamma_b);
		default_gamma_inited = true;
	}
}

void restore_gamma(void)
{
    if (!option_nogamma && bit_depth > 8 && default_gamma_inited)
        SDL_SetGammaRamp(default_gamma_r, default_gamma_g, default_gamma_b);
}

void build_direct_color_table(struct color_table *color_table, short bit_depth)
{
	color_table->color_count = 256;
	rgb_color *color = color_table->colors;
	for (int i=0; i<256; i++, color++)
	{
		color->red = default_gamma_r[i];
		color->green = default_gamma_g[i];
		color->blue = default_gamma_b[i];
	}
}

void bound_screen()
{
	Screen *screen = Screen::instance();
	SDL_Rect ScreenRect = {0, 0, screen->width(), screen->height()};

	SDL_Rect ViewRect = { (screen->width() - screen->window_width()) / 2, (screen->height() - screen->window_height()) / 2, screen->window_width(), screen->window_height() };

	Rect sr = { ScreenRect.y, ScreenRect.x, ScreenRect.y + ScreenRect.h, ScreenRect.x + ScreenRect.w};
	Rect vr = { ViewRect.y, ViewRect.x, ViewRect.y + ViewRect.h, ViewRect.x + ViewRect.w};
#ifdef HAVE_OPENGL
	OGL_SetWindow(sr, vr, true);
#endif
}

void change_interface_clut(struct color_table *color_table)
{
	memcpy(interface_color_table, color_table, sizeof(struct color_table));
}

void change_screen_clut(struct color_table *color_table)
{
	if (bit_depth == 8)
		memcpy(uncorrected_color_table, color_table, sizeof(struct color_table));
	else
		build_direct_color_table(uncorrected_color_table, bit_depth);
	memcpy(interface_color_table, uncorrected_color_table, sizeof(struct color_table));

	gamma_correct_color_table(uncorrected_color_table, world_color_table, screen_mode.gamma_level);
	memcpy(visible_color_table, world_color_table, sizeof(struct color_table));

	assert_world_color_table(interface_color_table, world_color_table);
}

void animate_screen_clut(struct color_table *color_table, bool full_screen)
{
	if (bit_depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(color_table, colors);
		SDL_SetPalette(main_surface, SDL_PHYSPAL, colors, 0, 256);
	} else {
		uint16 red[256], green[256], blue[256];
		for (int i=0; i<color_table->color_count; i++) {
			red[i] = color_table->colors[i].red;
			green[i] = color_table->colors[i].green;
			blue[i] = color_table->colors[i].blue;
		}
		if (!option_nogamma)
			SDL_SetGammaRamp(red, green, blue);
	}
}

void assert_world_color_table(struct color_table *interface_color_table, struct color_table *world_color_table)
{
	if (interface_bit_depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(interface_color_table, colors);
		SDL_SetPalette(main_surface, SDL_LOGPAL, colors, 0, 256);
		if (HUD_Buffer)
			SDL_SetColors(HUD_Buffer, colors, 0, 256);
	}
	if (world_color_table)
		animate_screen_clut(world_color_table, false);
}


/*
 *  Render terminal
 */

void render_computer_interface(struct view_data *view)
{
	
	struct view_terminal_data data;

	data.left = data.top = 0;
	data.right = view->screen_width;
	data.bottom = view->screen_height;
	data.vertical_offset = 0;

	_set_port_to_term();
	_render_computer_interface(&data);
	_restore_port();
}


/*
 *  Render overhead map
 */

void render_overhead_map(struct view_data *view)
{
	struct overhead_map_data overhead_data;
	SDL_FillRect(world_pixels, NULL, SDL_MapRGB(world_pixels->format, 0, 0, 0));

	overhead_data.half_width = view->half_screen_width;
	overhead_data.half_height = view->half_screen_height;
	overhead_data.width = view->screen_width;
	overhead_data.height = view->screen_height;
	overhead_data.top = overhead_data.left = 0;

	overhead_data.scale = view->overhead_map_scale;
	overhead_data.mode = _rendering_game_map;
	overhead_data.origin.x = view->origin.x;
	overhead_data.origin.y = view->origin.y;

	_set_port_to_gworld();
	_render_overhead_map(&overhead_data);
	_restore_port();
}


/*
 *  Get world view destination frame for given screen size
 */

void calculate_destination_frame(short size, bool high_resolution, Rect *frame)
{
	frame->left = frame->top = 0;

	// Calculate destination frame
	switch (size) {
		case _full_screen:
			frame->right = DESIRED_SCREEN_WIDTH;
			frame->bottom = DESIRED_SCREEN_HEIGHT;
			break;
		case _100_percent:
			frame->right = DEFAULT_WORLD_WIDTH;
			frame->bottom = DEFAULT_WORLD_HEIGHT;
			break;
		case _75_percent:
			frame->right = 3 * DEFAULT_WORLD_WIDTH / 4;
			frame->bottom = 3 * DEFAULT_WORLD_HEIGHT / 4;
			break;
		case _50_percent:
			frame->right = DEFAULT_WORLD_WIDTH / 2;
			frame->bottom = DEFAULT_WORLD_HEIGHT / 2;
			break;
	}
	
	if (size != _full_screen) {
		int dx = (DEFAULT_WORLD_WIDTH - frame->right) / 2;
		int dy = (DEFAULT_WORLD_HEIGHT - frame->bottom) / 2;
		frame->top += dy;
		frame->left += dx;
		frame->bottom += dy;
		frame->right += dx;
	}
}


/*
 *  Draw dithered black pattern over world window
 */

template <class T>
static inline void draw_pattern_rect(T *p, int pitch, uint32 pixel, const SDL_Rect &r)
{
	p += r.y * pitch / sizeof(T) + r.x;
	for (int y=0; y<r.h; y++) {
		for (int x=y&1; x<r.w; x+=2)
			p[x] = pixel;
		p += pitch / sizeof(T);
	}
}

void darken_world_window(void)
{
	// Get world window bounds
	SDL_Rect r = Screen::instance()->window_rect();

#ifdef HAVE_OPENGL
	if (main_surface->flags & SDL_OPENGL) {

		// Save current state
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// Disable everything but alpha blending
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_FOG);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_STENCIL_TEST);

		// Direct projection
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, GLdouble(main_surface->w), GLdouble(main_surface->h), 0.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw 50% black rectangle
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if(Using_sRGB)
		  glColor4f(0.0, 0.0, 0.0, 0.75);
		else
		  glColor4f(0.0, 0.0, 0.0, 0.5);
		glBegin(GL_QUADS);
			glVertex2i(r.x, r.y);
			glVertex2i(r.x + r.w, r.y);
			glVertex2i(r.x + r.w, r.y + r.h);
			glVertex2i(r.x, r.y + r.h);
		glEnd();

		// Restore projection and state
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();

		SDL_GL_SwapBuffers();
		return;
	}
#endif

	// Get black pixel value
	uint32 pixel = SDL_MapRGB(main_surface->format, 0, 0, 0);

	// Lock surface
	if (SDL_MUSTLOCK(main_surface))
		if (SDL_LockSurface(main_surface) < 0)
			return;

	// Draw pattern
	switch (main_surface->format->BytesPerPixel) {
		case 1:
			draw_pattern_rect((pixel8 *)main_surface->pixels, main_surface->pitch, pixel, r);
			break;
		case 2:
			draw_pattern_rect((pixel16 *)main_surface->pixels, main_surface->pitch, pixel, r);
			break;
		case 4:
			draw_pattern_rect((pixel32 *)main_surface->pixels, main_surface->pitch, pixel, r);
			break;
	}

	// Unlock surface
	if (SDL_MUSTLOCK(main_surface))
		SDL_UnlockSurface(main_surface);

	SDL_UpdateRects(main_surface, 1, &r);
}


/*
 *  Validate world window
 */

void validate_world_window(void)
{
	RequestDrawingTerm();
}


/*
 *  Draw the HUD or terminal (non-OpenGL)
 */

void DrawSurface(SDL_Surface *s, SDL_Rect &dest_rect, SDL_Rect &src_rect)
{
	if (s) {
		SDL_Rect new_src_rect = {src_rect.x, src_rect.y, src_rect.w, src_rect.h};
		SDL_Surface *surface = s;
		if (dest_rect.w != src_rect.w || dest_rect.h != src_rect.h)
		{
			double x_scale = dest_rect.w / (double) src_rect.w;
			double y_scale = dest_rect.h / (double) src_rect.h;
			surface = rescale_surface(s, static_cast<int>(s->w * x_scale), static_cast<int>(s->h * y_scale));
			new_src_rect.x = static_cast<Sint16>(new_src_rect.x * x_scale);
			new_src_rect.y = static_cast<Sint16>(new_src_rect.y * y_scale);
			new_src_rect.w = static_cast<Uint16>(new_src_rect.w * x_scale);
			new_src_rect.h = static_cast<Uint16>(new_src_rect.h * y_scale);
		}
		SDL_BlitSurface(surface, &new_src_rect, main_surface, &dest_rect);
		SDL_UpdateRects(main_surface, 1, &dest_rect);
		
		if (surface != s)
			SDL_FreeSurface(surface);
	}
}


/*
 *  Clear screen
 */

void clear_screen(bool update)
{
#ifdef HAVE_OPENGL
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL) {
		OGL_ClearScreen();
		if (update) SDL_GL_SwapBuffers();
	} else 
#endif
	{
		SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0, 0, 0));
		if (update) SDL_UpdateRect(main_surface, 0, 0, 0, 0);
	}

	clear_next_screen = true;
}

void clear_screen_margin()
{
#ifdef HAVE_OPENGL
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL) {
		OGL_ClearScreen();
        return;
	}
#endif
    SDL_Rect r, wr, dr;
    wr = Screen::instance()->window_rect();
    if (world_view->terminal_mode_active)
        dr = Screen::instance()->term_rect();
    else if (world_view->overhead_map_active)
        dr = Screen::instance()->map_rect();
    else
        dr = Screen::instance()->view_rect();

    if (dr.x > 0)
    {
        r.x = wr.x;
        r.y = wr.y;
        r.w = dr.x;
        r.h = wr.h;
        SDL_FillRect(main_surface, &r, SDL_MapRGB(main_surface->format, 0, 0, 0));
    }
    if ((dr.x + dr.w) < wr.w)
    {
        r.x = wr.x + dr.x + dr.w;
        r.y = wr.y;
        r.w = wr.w - (dr.x + dr.w);
        r.h = wr.h;
        SDL_FillRect(main_surface, &r, SDL_MapRGB(main_surface->format, 0, 0, 0));
    }
    if (dr.y > 0)
    {
        r.x = wr.x + dr.x;
        r.y = wr.y;
        r.w = dr.x + dr.w;
        r.h = dr.y;
        SDL_FillRect(main_surface, &r, SDL_MapRGB(main_surface->format, 0, 0, 0));
    }
    if ((dr.y + dr.h) < wr.h)
    {
        r.x = wr.x + dr.x;
        r.y = wr.y + dr.y + dr.h;
        r.w = dr.x + dr.w;
        r.h = wr.h - (dr.y + dr.h);
        SDL_FillRect(main_surface, &r, SDL_MapRGB(main_surface->format, 0, 0, 0));
    }
}
