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
static const TextSpec monaco_spec = {kFontIDMonaco, normal, 12};


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
int PrevBufferWidth = SHORT_MIN, PrevBufferHeight = SHORT_MIN,
    PrevOffsetWidth = SHORT_MIN, PrevOffsetHeight = SHORT_MIN;

#define FRAME_SAMPLE_SIZE 20
bool displaying_fps = true;
int frame_count, frame_index;
uint32 frame_ticks[64];

bool ShowPosition = false;	// Whether to show one's position

SDL_Surface *HUD_Buffer = NULL;
static bool HUD_RenderRequest = false;

static bool screen_initialized = false;

short bit_depth = NONE;
short interface_bit_depth = NONE;

static struct screen_mode_data screen_mode;


// From shell_sdl.cpp
extern bool option_fullscreen;

// Prototypes
static void reallocate_world_pixels(int width, int height);
static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez);
static void update_fps_display(SDL_Surface *s);
static void DisplayPosition(SDL_Surface *s);
static void set_overhead_map_status(boolean status);
static void set_terminal_status(boolean status);
static void DrawHUD(SDL_Rect &dest_rect);
static void ClearScreen(void);


/*
 *  Initialize screen management
 */

void initialize_screen(struct screen_mode_data *mode)
{
	interface_bit_depth = bit_depth = mode->bit_depth;

	if (!screen_initialized) {

		graphics_preferences->device_spec.width = DESIRED_SCREEN_WIDTH;
		graphics_preferences->device_spec.height = DESIRED_SCREEN_HEIGHT;

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
		world_view->overhead_map_active = FALSE;
		world_view->terminal_mode_active = FALSE;
		world_view->horizontal_scale = 1;
		world_view->vertical_scale = 1;
		world_view->tunnel_vision_active = false;

		world_pixels = NULL;

	} else
		unload_all_collections();

	// Allocate provisionary off-screen buffer
	reallocate_world_pixels(DEFAULT_WORLD_WIDTH, DEFAULT_WORLD_HEIGHT);

	// Set screen mode
	change_screen_mode(mode, FALSE);

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
	uint32 Rmask, Gmask, Bmask;
	switch (bit_depth) {
		case 8:
			Rmask = Gmask = Bmask = 0xff;
			break;
		case 16:
			Rmask = 0x7c00;
			Gmask = 0x03e0;
			Bmask = 0x001f;
			break;
		case 32:
			Rmask = 0x00ff0000;
			Gmask = 0x0000ff00;
			Bmask = 0x000000ff;
			break;
	}
	world_pixels = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, bit_depth, Rmask, Gmask, Bmask, 0);
	if (world_pixels == NULL)
		alert_user(fatalError, strERRORS, outOfMemory, -1);
}


/*
 *  This resets the screen; useful when starting a game
 */

void reset_screen()
{
	// Resetting cribbed from initialize_screen()
	world_view->overhead_map_scale = DEFAULT_OVERHEAD_MAP_SCALE;
	world_view->overhead_map_active = FALSE;
	world_view->terminal_mode_active = FALSE;
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
	if (screen_mode.acceleration == _opengl_acceleration) {
#ifdef HAVE_OPENGL
		OGL_StartRun((CGrafPtr)screen_window);
#else
		screen_mode.acceleration = _no_acceleration;
#endif
	}
}


/*
 *  Enter game screen
 */

void enter_screen(void)
{
	ClearScreen();

	if (world_view->overhead_map_active)
		set_overhead_map_status(FALSE);
	if (world_view->terminal_mode_active)
		set_terminal_status(FALSE);

	// Adding this view-effect resetting here since initialize_world_view() no longer resets it
	world_view->effect = NONE;
	
	change_screen_mode(&screen_mode, TRUE);

	if (screen_mode.acceleration == _opengl_acceleration) {
#ifdef HAVE_OPENGL
		OGL_StartRun(screen)window);
#else
		screen_mode.acceleration = _no_acceleration;
#endif
	}
}


/*
 *  Exit game screen
 */

void exit_screen(void)
{
#ifdef HAVE_OPENGL
	OGL_StopRun();
#endif
}


/*
 *  Change screen mode
 */

void change_screen_mode(struct screen_mode_data *mode, boolean redraw)
{
	// Get the screen mode here
	screen_mode = *mode;

	// Open SDL display
	int msize = mode->size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	uint32 flags = SDL_HWSURFACE | SDL_HWPALETTE | (option_fullscreen ? SDL_FULLSCREEN : 0);
	main_surface = SDL_SetVideoMode(ViewSizes[msize].OverallWidth, ViewSizes[msize].OverallHeight, mode->bit_depth, flags);
	if (main_surface == NULL) {
		fprintf(stderr, "Can't open video display (%s)\n", SDL_GetError());
		exit(1);
	}
	
	// "Redraw" means clear the screen
	if (redraw)
		ClearScreen();

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
			set_overhead_map_status(TRUE);
	} else {
		if (world_view->overhead_map_active)
			set_overhead_map_status(FALSE);
	}

	if(player_in_terminal_mode(current_player_index)) {
		if (!world_view->terminal_mode_active)
			set_terminal_status(TRUE);
	} else {
		if (world_view->terminal_mode_active)
			set_terminal_status(FALSE);
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

	// Set up view data appropriately (cribbed from change_screen_mode)
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
		ClearScreen();
		if (VS.ShowHUD)
			draw_interface();

		// Reallocate the drawing buffer
		reallocate_world_pixels(BufferRect.w, BufferRect.h);
	}

#ifdef HAVE_OPENGL
	// Be sure that the main view is buffered...
	OGL_SetWindow(ScreenRect, ViewRect, true);
#endif

	switch (screen_mode.acceleration) {
		case _opengl_acceleration:
			// If we're using the overhead map, fall through to no acceleration
			if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
				break;
		case _no_acceleration:
			world_pixels_structure->width = world_view->screen_width;
			world_pixels_structure->height = world_view->screen_height;
			world_pixels_structure->flags = 0;
			world_pixels_structure->bytes_per_row = world_pixels->pitch;
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

	render_view(world_view, world_pixels_structure);

	update_fps_display(world_pixels);

	// Display position and show crosshairs
	if (!world_view->terminal_mode_active)
		DisplayPosition(world_pixels);
	if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
		if (Crosshairs_IsActive())
#ifdef HAVE_OPENGL
			if (!OGL_RenderCrosshairs())
#endif
				Crosshairs_Render(world_pixels);

	// Update world window
	update_screen(BufferRect, ViewRect, HighResolution);
	//!! OpenGL

	// Update HUD
	if (HUD_RenderRequest) {
		//!! OpenGL
		DrawHUD(HUD_DestRect);
		HUD_RenderRequest = false;
	}
}


/*
 *  Blit world view to screen
 */

static void update_screen(SDL_Rect &source, SDL_Rect &destination, bool hi_rez)
{
#if 1
	SDL_BlitSurface(world_pixels, NULL, main_surface, &destination);
#else
	uint32 *p = (uint32 *)main_surface->pixels;
	uint32 *q = (uint32 *)world_pixels->pixels;
	for (int y=0; y<destination.h; y++) {
		for (int x=0; x<destination.w/4; x++)
			p[x] = q[x];
		p += main_surface->pitch >> 2;
		q += world_pixels->pitch >> 2;
	}
#endif
	SDL_UpdateRects(main_surface, 1, &destination);
}


/*
 *  Color table handling
 */

void build_sdl_color_table(const color_table *color_table, SDL_Color *colors)
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
	if (interface_bit_depth == 8 && bit_depth == 8) {
		memcpy(uncorrected_color_table, color_table, sizeof(struct color_table));
		memcpy(interface_color_table, color_table, sizeof(struct color_table));
	}

	gamma_correct_color_table(uncorrected_color_table, world_color_table, screen_mode.gamma_level);
	memcpy(visible_color_table, world_color_table, sizeof(struct color_table));

	assert_world_color_table(interface_color_table, world_color_table);
}

void animate_screen_clut(struct color_table *color_table, boolean full_screen)
{
	if (world_pixels && bit_depth == 8) {
		SDL_Color colors[256];
		build_sdl_color_table(color_table, colors);
		SDL_SetColors(world_pixels, colors, 0, color_table->color_count);
	}
}

void assert_world_color_table(struct color_table *interface_color_table, struct color_table *world_color_table)
{
	SDL_Color colors[256];
	build_sdl_color_table(interface_color_table, colors);
	SDL_SetColors(main_surface, colors, 0, interface_color_table->color_count);
	if (HUD_Buffer)
		SDL_SetColors(HUD_Buffer, colors, 0, interface_color_table->color_count);

	if (world_color_table)
		animate_screen_clut(world_color_table, false);
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

	_render_computer_interface(&data);
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

	_render_overhead_map(&overhead_data);
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
		draw_text(world_pixels, fps, 5, world_pixels->h - 5, SDL_MapRGB(world_pixels->format, 0xff, 0xff, 0xff), load_font(monaco_spec), normal);
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
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
	Y += Leading;
	sprintf(temporary, "Y       = %8.3f", world_view->origin.y / FLOAT_WORLD_ONE);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
	Y += Leading;
	sprintf(temporary, "Z       = %8.3f", world_view->origin.z / FLOAT_WORLD_ONE);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
	Y += Leading;
	sprintf(temporary, "Polygon = %8d", world_view->origin_polygon_index);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
	Y += Leading;
	int Angle = world_view->yaw;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Yaw     = %8.3f", AngleConvert * Angle);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
	Y += Leading;
	Angle = world_view->pitch;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Pitch   = %8.3f", AngleConvert * Angle);
	draw_text(world_pixels, temporary, 5, Y, pixel, font, normal);
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

void start_teleporting_effect(boolean out)
{
	start_render_effect(world_view, out ? _render_effect_fold_out : _render_effect_fold_in);
}

void start_extravision_effect(boolean out)
{
	world_view->target_field_of_view = out ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW;
}

void start_tunnel_vision_effect(boolean out)
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

boolean game_window_is_full_screen(void)
{
	short msize = screen_mode.size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	return (!ViewSizes[msize].ShowHUD);
}


/*
 *  16/32 bit supported?
 */

boolean machine_supports_16bit(GDSpecPtr spec)
{
	return true;
}

boolean machine_supports_32bit(GDSpecPtr spec)
{
	return true;
}


/*
 *  Hardware acceleration supported?
 */

short hardware_acceleration_code(GDSpecPtr spec)
{
	return _no_acceleration;
}


/*
 *  Update screen
 */

void update_screen_window(WindowPtr window, EventRecord *event)
{
	draw_interface();
	change_screen_mode(&screen_mode, TRUE);
	assert_world_color_table(interface_color_table, world_color_table);
}


/*
 *  Draw dithered black pattern over world window
 */

void darken_world_window(void)
{
printf("*** darken_world_window()\n");
	//!!
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

static void set_overhead_map_status(boolean status)
{
	world_view->overhead_map_active = status;
}

static void set_terminal_status(boolean status)
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

static void ClearScreen(void)
{
	SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0, 0, 0));
	SDL_UpdateRect(main_surface, 0, 0, 0, 0);
}
