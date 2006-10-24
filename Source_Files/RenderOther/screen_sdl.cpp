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
#include "scripting.h"
#include "screen_drawing.h"
#include "mouse.h"
#include "network.h"

#include "sdl_fonts.h"

#include "lua_script.h"

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

static bool PrevFullscreen = false;
static bool in_game = false;	// Flag: menu (fixed 640x480) or in-game (variable size) display

#ifdef HAVE_OPENGL
// This is defined in overhead_map.c
// It indicates whether to render the overhead map in OpenGL
extern bool OGL_MapActive;
// This is the same for the HUD
extern bool OGL_HUDActive;
// and lastly, for the terminal buffer
bool OGL_TermActive = false;
#endif

static int desktop_width;
static int desktop_height;

// From shell_sdl.cpp
extern bool option_nogamma;

#include "screen_shared.h"


// Prototypes
static void change_screen_mode(int width, int height, int depth, bool nogl);
static void build_sdl_color_table(const color_table *color_table, SDL_Color *colors);
static void reallocate_world_pixels(int width, int height);
static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez);
static void update_fps_display(SDL_Surface *s);
static void DisplayPosition(SDL_Surface *s);
static void DisplayMessages(SDL_Surface *s);
static void DrawHUD(SDL_Rect &dest_rect);

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

void initialize_screen(struct screen_mode_data *mode, bool ShowFreqDialog)
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

	if (OGL_IsActive())
		OGL_HUDActive = true;
	else
		OGL_HUDActive = false;

	// Reset modifier key status
	SDL_SetModState(KMOD_NONE);
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
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif
	} else 
#endif 
	if (nogl) {
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
		printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
		printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
		printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
//		const char *gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
//		printf("GL_EXTENSIONS: %s\n", gl_extensions);
		glScissor(0, 0, width, height);
		glViewport(0, 0, width, height);
#ifdef __WIN32__
		clear_screen();
#endif
	}
#endif
}

void change_screen_mode(struct screen_mode_data *mode, bool redraw)
{
	// Get the screen mode here
	screen_mode = *mode;

	// "Redraw" change now and clear the screen
	if (redraw) {
		int msize = mode->size;
		assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
		change_screen_mode(ViewSizes[msize].OverallWidth, ViewSizes[msize].OverallHeight, mode->bit_depth, false);
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

#ifdef HAVE_OPENGL
GLuint OGL_Term_Texture;
#endif
/*
 *  Render game screen
 */

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
	SDL_Rect ScreenRect = {0, 0, main_surface->w, main_surface->h};

	int msize = mode->size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	const ViewSizeData &VS = ViewSizes[msize];

	// Rectangle where the view is to go (must not overlap the HUD)
	int OverallWidth = VS.OverallWidth;
	int OverallHeight = VS.OverallHeight - (TEST_FLAG(VS.flags, _view_show_HUD) ? 160 : 0);
	int BufferWidth, BufferHeight;

	// Offsets for placement in the screen
	int ScreenOffsetWidth = ((ScreenRect.w - VS.OverallWidth) / 2) + ScreenRect.x;
	int ScreenOffsetHeight = ((ScreenRect.h - VS.OverallHeight) / 2) + ScreenRect.y;

	// HUD location
	int HUD_Offset = (OverallWidth - 640) / 2;
	SDL_Rect HUD_DestRect = {HUD_Offset + ScreenOffsetWidth, OverallHeight + ScreenOffsetHeight, 640, 160};

	bool ChangedSize = false;

	// Switching fullscreen mode requires much the same reinitialization as switching the screen size
	if (mode->fullscreen != PrevFullscreen) {
		PrevFullscreen = mode->fullscreen;
		ChangedSize = true;
	}

	// Each kind of display needs its own size
	if (world_view->terminal_mode_active) {
		// Standard terminal size
		BufferWidth = 640;
		BufferHeight = 320;
		HighResolution = true;
	} else if (world_view->overhead_map_active) {
		// Fill the available space
		BufferWidth = OverallWidth;
		BufferHeight = OverallHeight;
		HighResolution = true;		
	} else {
		BufferWidth = VS.MainWidth;
		BufferHeight = VS.MainHeight;
		HighResolution = mode->high_resolution;
	}

	if (BufferWidth != PrevBufferWidth) {
		ChangedSize = true;
		PrevBufferWidth = BufferWidth;
	}
	if (BufferHeight != PrevBufferHeight) {
		ChangedSize = true;
		PrevBufferHeight = BufferHeight;
	}

	// Do the buffer/viewport rectangle setup:

	// First, the destination rectangle (viewport to be drawn in)
	int OffsetWidth = (OverallWidth - BufferWidth) / 2;
	int OffsetHeight = (OverallHeight - BufferHeight) / 2;
	SDL_Rect ViewRect = {OffsetWidth + ScreenOffsetWidth, OffsetHeight + ScreenOffsetHeight, BufferWidth, BufferHeight};

	if (OffsetWidth != PrevOffsetWidth) {
		ChangedSize = true;
		PrevOffsetWidth = OffsetWidth;
	}
	if (OffsetHeight != PrevOffsetHeight) {
		ChangedSize = true;
		PrevOffsetHeight = OffsetHeight;
	}

	// Now the buffer rectangle; be sure to shrink it as appropriate
	if (!HighResolution && screen_mode.acceleration == _no_acceleration) {
		BufferWidth >>= 1;
		BufferHeight >>= 1;
	}
	SDL_Rect BufferRect = {0, 0, BufferWidth, BufferHeight};

	// Set up view data appropriately
	world_view->screen_width = BufferWidth;
	world_view->screen_height = BufferHeight;
	world_view->standard_screen_width = 2*BufferHeight;	
	initialize_view_data(world_view);

	if (world_pixels) {
		// Check on the drawing buffer's size
		if (world_pixels->w != BufferWidth || world_pixels->h != BufferHeight)
			ChangedSize = true;
	} else
		ChangedSize = true;

	if (ChangedSize) {
		clear_screen();
		if (TEST_FLAG(VS.flags, _view_show_HUD))
			draw_interface();

		// Reallocate the drawing buffer
		reallocate_world_pixels(BufferRect.w, BufferRect.h);

		dirty_terminal_view(current_player_index);
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
	if (!UseLuaCameras() && !script_Camera_Active())
		world_view->show_weapons_in_hand = !ChaseCam_GetPosition(world_view->origin, world_view->origin_polygon_index, world_view->yaw, world_view->pitch);

#ifdef HAVE_OPENGL
	// Is map to be drawn with OpenGL?
	if (OGL_IsActive() && world_view->overhead_map_active)
		OGL_MapActive = true;
	else {
		if (OGL_MapActive) {
			// switching off map
			// clear the remnants of the map out of the back buffer
			glClearColor(0,0,0,0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		OGL_MapActive = false;
	}
	if (OGL_IsActive() && world_view->terminal_mode_active) {
		if (!OGL_TermActive)
		{
			glClearColor(0,0,0,0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
		}
		OGL_TermActive = true;
	} else {
		OGL_TermActive = false;
	}

	// Set OpenGL viewport to world view
	Rect sr = {ScreenRect.y, ScreenRect.x, ScreenRect.y + ScreenRect.h, ScreenRect.x + ScreenRect.w};
	Rect vr = {ViewRect.y, ViewRect.x, ViewRect.y + ViewRect.h, ViewRect.x + ViewRect.w};
	OGL_SetWindow(sr, vr, true);
#endif

	// Render world view
	render_view(world_view, world_pixels_structure);

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
		if (world_view->terminal_mode_active || (world_view->overhead_map_active && !OGL_MapActive)) {

			// Copy 2D rendering to screen

			SDL_Rect term_dst = { OffsetWidth + ScreenOffsetWidth, OffsetHeight + ScreenOffsetHeight, 640, 320};
			SDL_Rect term_ortho = { 0, 0, ScreenRect.w, ScreenRect.h };
			SDL_SetAlpha(Term_Buffer, 0, 0xff);
			OGL_Blitter blitter(*Term_Buffer, term_dst, term_ortho);
			blitter.SetupMatrix();
			blitter.Draw();
			blitter.RestoreMatrix();
		}
		
		if (TEST_FLAG(VS.flags, _view_show_HUD)) {
			if (OGL_HUDActive) {
				Rect dr = {HUD_DestRect.y, HUD_DestRect.x, HUD_DestRect.y + HUD_DestRect.h, HUD_DestRect.x + HUD_DestRect.w};
				OGL_DrawHUD(dr, ticks_elapsed);
			} else {
				if (HUD_RenderRequest) {
					DrawHUD(HUD_DestRect);
					HUD_RenderRequest = false;
				}
			}
		}
#endif
	} else {

		// Update world window
		update_screen(BufferRect, ViewRect, HighResolution);

		// Update HUD
		if (HUD_RenderRequest) {
			DrawHUD(HUD_DestRect);
			HUD_RenderRequest = false;
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
	SDL_UpdateRects(main_surface, 1, &destination);
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

void build_direct_color_table(struct color_table *color_table, short bit_depth)
{
	color_table->color_count = 256;
	rgb_color *color = color_table->colors;
	for (int i=0; i<256; i++, color++)
		color->red = color->green = color->blue = i * 0x0101;
}

void bound_screen()
{
	screen_mode_data *mode = &screen_mode;
	SDL_Rect ScreenRect = { 0, 0, main_surface->w, main_surface->h };

	const ViewSizeData &VS = ViewSizes[mode->size];
	short ScreenOffsetWidth = (ScreenRect.w - VS.OverallWidth) / 2;
	short ScreenOffsetHeight = (ScreenRect.h - VS.OverallHeight) / 2;

	SDL_Rect ViewRect = { ScreenOffsetWidth, ScreenOffsetHeight, VS.OverallWidth, VS.OverallHeight };

	Rect sr = { ScreenRect.y, ScreenRect.x, ScreenRect.y + ScreenRect.h, ScreenRect.x + ScreenRect.w};
	Rect vr = { ViewRect.y, ViewRect.x, ViewRect.y + ViewRect.h, ViewRect.x + ViewRect.w};
	OGL_SetWindow(sr, vr, true);
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

	if (screen_mode.acceleration == _opengl_acceleration) {
		_set_port_to_term();
	} else {
		_set_port_to_gworld();
	}
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
	int size = screen_mode.size;
	SDL_Rect r = {0, 0, ViewSizes[size].OverallWidth, ViewSizes[size].OverallHeight - (TEST_FLAG(ViewSizes[size].flags, _view_show_HUD) ? 160 : 0)};

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
	// nothing to do
}


/*
 *  Draw the HUD (non-OpenGL)
 */

void DrawHUD(SDL_Rect &dest_rect)
{
	if (HUD_Buffer) {
		SDL_Rect src_rect = {0, 320, 640, 160};
		SDL_BlitSurface(HUD_Buffer, &src_rect, main_surface, &dest_rect);
		SDL_UpdateRects(main_surface, 1, &dest_rect);
	}
}


/*
 *  Clear screen
 */

void clear_screen(void)
{
#ifdef HAVE_OPENGL
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL) {
		OGL_ClearScreen();
		SDL_GL_SwapBuffers();
	} else 
#endif
	{
		SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0, 0, 0));
		SDL_UpdateRect(main_surface, 0, 0, 0, 0);
	}
}
