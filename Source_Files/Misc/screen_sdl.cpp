/*
 *  screen_sdl.cpp - Screen management, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
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


// Constants
#define DESIRED_SCREEN_WIDTH 640
#define DESIRED_SCREEN_HEIGHT 480

#define MAXIMUM_WORLD_WIDTH 1024
#define MAXIMUM_WORLD_HEIGHT 768

#define DEFAULT_WORLD_WIDTH 640
#define DEFAULT_WORLD_HEIGHT 320


// Supported display sizes
struct ViewSizeData
{
	int OverallWidth, OverallHeight;	// Of the display area, so as to properly center everything
	int MainWidth, MainHeight;			// Of the main 3D-rendered view
	int WithHUD, WithoutHUD;			// Corresponding entries that are with the HUD or without it
	bool ShowHUD;						// Will it be visible?
};

const ViewSizeData ViewSizes[NUMBER_OF_VIEW_SIZES] =
{
	{ 640, 480,	 320, 160,	 _320_160_HUD,  _640_480,	 true},		//  _320_160_HUD
	{ 640, 480,	 480, 240,	 _480_240_HUD,  _640_480,	 true},		//  _480_240_HUD
	{ 640, 480,	 640, 320,	 _640_320_HUD,  _640_480,	 true},		//  _640_320_HUD
	{ 640, 480,	 640, 480,	 _640_320_HUD,  _640_480,	false},		//  _640_480
	{ 800, 600,	 800, 400,	 _800_400_HUD,  _800_600,	 true},		//  _800_400_HUD
	{ 800, 600,	 800, 600,	 _800_400_HUD,  _800_600,	false},		//  _800_600
	{1024, 768,	1024, 512,	_1024_512_HUD, _1024_768,	 true},		// _1024_512_HUD
	{1024, 768,	1024, 768,	_1024_512_HUD, _1024_768,	false},		// _1024_768
};

// Note: the overhead map will always fill all of the screen except for the HUD,
// and the terminal display will always have a size of 640*320.

// Font for FPS/position display
static const TextSpec monaco_spec = {kFontIDMonaco, styleNormal, 12};


// Global variables
static SDL_Surface *main_surface;	// Main (display) surface

struct color_table *uncorrected_color_table; /* the pristine color environment of the game (can be 16bit) */
struct color_table *world_color_table; /* the gamma-corrected color environment of the game (can be 16bit) */
struct color_table *interface_color_table; /* always 8bit, for mixed-mode (i.e., valkyrie) fades */
struct color_table *visible_color_table; /* the color environment the player sees (can be 16bit) */

struct view_data *world_view; /* should be static */

// Rendering buffer for the main view, the overhead map, and the terminals.
// The HUD has a separate buffer.
// It is initialized to NULL so as to allow its initing to be lazy.
SDL_Surface *world_pixels = NULL;
struct bitmap_definition *world_pixels_structure;

// Stuff for keeping track of screen sizes; this is for forcing the clearing of the screen when resizing.
// These are initialized to improbable values.
int PrevBufferWidth = INT16_MIN, PrevBufferHeight = INT16_MIN,
    PrevOffsetWidth = INT16_MIN, PrevOffsetHeight = INT16_MIN;

#define FRAME_SAMPLE_SIZE 20
bool displaying_fps = false;
int frame_count, frame_index;
uint32 frame_ticks[64];

bool ShowPosition = false;	// Whether to show one's position

SDL_Surface *HUD_Buffer = NULL;
static bool HUD_RenderRequest = false;

static bool screen_initialized = false;

short bit_depth = NONE;
short interface_bit_depth = NONE;

#ifdef HAVE_OPENGL
// This is defined in overhead_map.c
// It indicates whether to render the overhead map in OpenGL
extern bool OGL_MapActive;
#endif

static struct screen_mode_data screen_mode;


// From shell_sdl.cpp
extern bool option_fullscreen;

// Prototypes
static void change_screen_mode(int width, int height, int depth, bool nogl);
static void build_sdl_color_table(const color_table *color_table, SDL_Color *colors);
static void reallocate_world_pixels(int width, int height);
static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez);
static void update_fps_display(SDL_Surface *s);
static void DisplayPosition(SDL_Surface *s);
static void set_overhead_map_status(bool status);
static void set_terminal_status(bool status);
static void DrawHUD(SDL_Rect &dest_rect);


/*
 *  Initialize screen management
 */

void initialize_screen(struct screen_mode_data *mode)
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
	world_pixels = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bit_depth, f->Rmask, f->Gmask, f->Bmask, f->Amask);
	if (world_pixels == NULL)
		alert_user(fatalError, strERRORS, outOfMemory, -1);
	else if (bit_depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(world_color_table, colors);
		SDL_SetColors(world_pixels, colors, 0, 256);
	}
}


/*
 *  This resets the screen; useful when starting a game
 */

void reset_screen()
{
	// Resetting cribbed from initialize_screen()
	world_view->overhead_map_scale = DEFAULT_OVERHEAD_MAP_SCALE;
	world_view->overhead_map_active = false;
	world_view->terminal_mode_active = false;
	world_view->horizontal_scale = 1;
	world_view->vertical_scale = 1;
	ResetFieldOfView();
}


/*
 *  Resets field of view to whatever the player had had when reviving
 */

void ResetFieldOfView()
{
	world_view->tunnel_vision_active = false;

	if (current_player->extravision_duration) {
		world_view->field_of_view = EXTRAVISION_FIELD_OF_VIEW;
		world_view->target_field_of_view = EXTRAVISION_FIELD_OF_VIEW;
	} else {
		world_view->field_of_view = NORMAL_FIELD_OF_VIEW;
		world_view->target_field_of_view = NORMAL_FIELD_OF_VIEW;
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
	change_screen_mode(&screen_mode, true);

#ifdef HAVE_OPENGL
	if (screen_mode.acceleration == _opengl_acceleration)
		OGL_StartRun();
#endif

	// Reset modifier key status
	SDL_SetModState(KMOD_NONE);
}


/*
 *  Exit game screen
 */

void exit_screen(void)
{
	// Return to 640x480 without OpenGL
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
	uint32 flags = (option_fullscreen ? SDL_FULLSCREEN : 0);
#ifdef HAVE_OPENGL
	// The original idea was to only enable OpenGL for the in-game display, but
	// SDL crashes if OpenGL is turned on later
	if (/*!nogl &&*/ screen_mode.acceleration == _opengl_acceleration) {
		flags |= SDL_OPENGLBLIT;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	} else
		flags |= SDL_HWSURFACE | SDL_HWPALETTE;
#else
	flags |= SDL_HWSURFACE | SDL_HWPALETTE;
#endif
	main_surface = SDL_SetVideoMode(width, height, depth, flags);
	if (main_surface == NULL) {
		fprintf(stderr, "Can't open video display (%s)\n", SDL_GetError());
		exit(1);
	}
	if (depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(interface_color_table, colors);
		SDL_SetColors(main_surface, colors, 0, 256);
	}
	if (HUD_Buffer) {
		SDL_FreeSurface(HUD_Buffer);
		HUD_Buffer = NULL;
	}
#ifdef HAVE_OPENGL
	if (main_surface->flags & SDL_OPENGL) {
		printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
		printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
		printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
		const char *gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
		printf("GL_EXTENSIONS: %s\n", gl_extensions);
		glScissor(0, 0, width, height);
		glViewport(0, 0, width, height);
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
	int OverallHeight = VS.OverallHeight - (VS.ShowHUD ? 160 : 0);
	int BufferWidth, BufferHeight;

	// Offsets for placement in the screen
	int ScreenOffsetWidth = ((ScreenRect.w - VS.OverallWidth) / 2) + ScreenRect.x;
	int ScreenOffsetHeight = ((ScreenRect.h - VS.OverallHeight) / 2) + ScreenRect.y;

	// HUD location
	int HUD_Offset = (OverallWidth - 640) / 2;
	SDL_Rect HUD_DestRect = {HUD_Offset + ScreenOffsetWidth, OverallHeight + ScreenOffsetHeight, 640, 160};

	bool ChangedSize = false;

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
	if (!HighResolution) {
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
		if (VS.ShowHUD)
			draw_interface();

		// Reallocate the drawing buffer
		reallocate_world_pixels(BufferRect.w, BufferRect.h);
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
	if (!script_Camera_Active())
		world_view->show_weapons_in_hand = !ChaseCam_GetPosition(world_view->origin, world_view->origin_polygon_index, world_view->yaw, world_view->pitch);

#ifdef HAVE_OPENGL
	// Is map to be drawn with OpenGL?
	if (OGL_IsActive() && world_view->overhead_map_active)
		OGL_MapActive = (TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Map) != 0);
	else
		OGL_MapActive = false;
#endif

#ifdef HAVE_OPENGL
	// Set OpenGL viewport to world view
	Rect sr = {ScreenRect.y, ScreenRect.x, ScreenRect.y + ScreenRect.h, ScreenRect.x + ScreenRect.w};
	Rect vr = {ViewRect.y, ViewRect.x, ViewRect.y + ViewRect.h, ViewRect.x + ViewRect.w};
	OGL_SetWindow(sr, vr, true);
#endif

	// Render world view
	render_view(world_view, world_pixels_structure);

	// Render crosshairs
	if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
		if (Crosshairs_IsActive())
#ifdef HAVE_OPENGL
			if (!OGL_RenderCrosshairs())
#endif
				Crosshairs_Render(world_pixels);

	// Display FPS and position
	if (!world_view->terminal_mode_active) {
		update_fps_display(world_pixels);
		DisplayPosition(world_pixels);
	}

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
			update_screen(BufferRect, ViewRect, HighResolution);
		}
#endif
	} else {

		// Update world window
		update_screen(BufferRect, ViewRect, HighResolution);
	}

	// Update HUD
	if (HUD_RenderRequest) {
		DrawHUD(HUD_DestRect);
		HUD_RenderRequest = false;
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
static inline void quadruple(const T *src, int src_pitch, T *dst, int dst_pitch, const SDL_Rect &dst_rect)
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
		switch (world_pixels->format->BytesPerPixel) {
			case 1:
				quadruple((pixel8 *)world_pixels->pixels, world_pixels->pitch, (pixel8 *)main_surface->pixels, main_surface->pitch, destination);
				break;
			case 2:
				quadruple((pixel16 *)world_pixels->pixels, world_pixels->pitch, (pixel16 *)main_surface->pixels, main_surface->pitch, destination);
				break;
			case 4:
				quadruple((pixel32 *)world_pixels->pixels, world_pixels->pitch, (pixel32 *)main_surface->pixels, main_surface->pitch, destination);
				break;
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
	//!!
}

void assert_world_color_table(struct color_table *interface_color_table, struct color_table *world_color_table)
{
	SDL_Color colors[256];
	build_sdl_color_table(interface_color_table, colors);
	SDL_SetColors(main_surface, colors, 0, 256);
	if (HUD_Buffer)
		SDL_SetColors(HUD_Buffer, colors, 0, 256);

	if (world_pixels && world_color_table) {
		SDL_Color colors[256];
		build_sdl_color_table(world_color_table, colors);
		SDL_SetColors(world_pixels, colors, 0, 256);
	}
}


/*
 *  Build dummy color table for 16/32 bit modes
 */

void build_direct_color_table(struct color_table *color_table, short bit_depth)
{
	color_table->color_count = 256;
	rgb_color *color = color_table->colors;
	for (int i=0; i<256; i++, color++)
		color->red = color->green = color->blue = i * 0x0101;
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

	_set_port_to_gworld();
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
 *  FPS display
 */

static void update_fps_display(SDL_Surface *s)
{
	char fps[32];
	if (displaying_fps) {

		// Calculate FPS
		uint32 ticks = SDL_GetTicks();
		frame_ticks[frame_index] = ticks;
		frame_index = (frame_index + 1) % FRAME_SAMPLE_SIZE;

		// Format as string
		if (frame_count < FRAME_SAMPLE_SIZE) {
			frame_count++;
			strcpy(fps, "--");
		} else
			sprintf(fps, "%3.2ffps", (FRAME_SAMPLE_SIZE * MACHINE_TICKS_PER_SECOND) / float(ticks - frame_ticks[frame_index]));

		// Print to screen
		draw_text(world_pixels, fps, 5, world_pixels->h - 5, SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff), load_font(monaco_spec), styleNormal);
		//!! OpenGL
	} else
		frame_count = frame_index = 0;
}


/*
 *  Display position
 */

static void DisplayPosition(SDL_Surface *s)
{
	if (!ShowPosition)
		return;

	// Get font and color
	const sdl_font_info *font = load_font(monaco_spec);
	uint32 pixel = SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff);

	// Print position to screen
	int Y = 15, Leading = 16;
	const float FLOAT_WORLD_ONE = float(WORLD_ONE);
	const float AngleConvert = 360.0 / float(FULL_CIRCLE);
	sprintf(temporary, "X       = %8.3f", world_view->origin.x / FLOAT_WORLD_ONE);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	Y += Leading;
	sprintf(temporary, "Y       = %8.3f", world_view->origin.y / FLOAT_WORLD_ONE);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	Y += Leading;
	sprintf(temporary, "Z       = %8.3f", world_view->origin.z / FLOAT_WORLD_ONE);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	Y += Leading;
	sprintf(temporary, "Polygon = %8d", world_view->origin_polygon_index);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	Y += Leading;
	int Angle = world_view->yaw;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Yaw     = %8.3f", AngleConvert * Angle);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	Y += Leading;
	Angle = world_view->pitch;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Pitch   = %8.3f", AngleConvert * Angle);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, styleNormal);
	//!! OpenGL
}


/*
 *  Zoom overhead map
 */

void zoom_overhead_map_out(void)
{
	world_view->overhead_map_scale = FLOOR(world_view->overhead_map_scale-1, OVERHEAD_MAP_MINIMUM_SCALE);
}

void zoom_overhead_map_in(void)
{
	world_view->overhead_map_scale = CEILING(world_view->overhead_map_scale+1, OVERHEAD_MAP_MAXIMUM_SCALE);
}


/*
 *  Special effects
 */

void start_teleporting_effect(bool out)
{
	start_render_effect(world_view, out ? _render_effect_fold_out : _render_effect_fold_in);
}

void start_extravision_effect(bool out)
{
	world_view->target_field_of_view = out ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW;
}

void start_tunnel_vision_effect(bool out)
{
	world_view->target_field_of_view = out ? TUNNEL_VISION_FIELD_OF_VIEW : 
		((current_player->extravision_duration) ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW);
}


/*
 *  Get current screen mode info
 */

screen_mode_data *get_screen_mode(void)
{
	return &screen_mode;
}


/*
 *  Show HUD?
 */

bool game_window_is_full_screen(void)
{
	short msize = screen_mode.size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	return (!ViewSizes[msize].ShowHUD);
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
	if (main_surface->flags & SDL_OPENGL)
		return;	//!!

	// Get black pixel value
	uint32 pixel = SDL_MapRGB(main_surface->format, 0, 0, 0);

	// Get world window bounds
	int size = screen_mode.size;
	SDL_Rect r = {0, 0, ViewSizes[size].OverallWidth, ViewSizes[size].OverallHeight - 160};

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
 *  Change gamma level
 */

void change_gamma_level(short gamma_level)
{
	screen_mode.gamma_level = gamma_level;
	gamma_correct_color_table(uncorrected_color_table, world_color_table, gamma_level);
	stop_fade();
	memcpy(visible_color_table, world_color_table, sizeof(struct color_table));
	assert_world_color_table(interface_color_table, world_color_table);
	change_screen_mode(&screen_mode, false);
	set_fade_effect(NONE);
}


/*
 *  Set map/terminal display status
 */

static void set_overhead_map_status(bool status)
{
	world_view->overhead_map_active = status;
}

static void set_terminal_status(bool status)
{
	world_view->terminal_mode_active = status;
	dirty_terminal_view(current_player_index);
}


/*
 *  Draw the HUD
 */

void DrawHUD(SDL_Rect &dest_rect)
{
	assert(HUD_Buffer);
	SDL_Rect src_rect = {0, 320, 640, 160};
	SDL_BlitSurface(HUD_Buffer, &src_rect, main_surface, &dest_rect);
	SDL_UpdateRects(main_surface, 1, &dest_rect);
}

void RequestDrawingHUD(void)
{
	HUD_RenderRequest = true;
}


/*
 *  For getting and setting tunnel-vision mode
 */

bool GetTunnelVision(void)
{
	return world_view->tunnel_vision_active;
}

bool SetTunnelVision(bool TunnelVisionOn)
{
	world_view->tunnel_vision_active = TunnelVisionOn;
	start_tunnel_vision_effect(TunnelVisionOn);
	return world_view->tunnel_vision_active;
}


/*
 *  Clear screen
 */

void clear_screen(void)
{
	SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0, 0, 0));
	SDL_UpdateRect(main_surface, 0, 0, 0, 0);
#ifdef HAVE_OPENGL
	if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
		SDL_GL_SwapBuffers();
#endif
}
