/*
	Created by Loren Petrich,
	Dec. 23, 2000
	Contains everything shared between screen.cpp and screen_sdl.cpp
	
Dec 29, 2000 (Loren Petrich):
	Added stuff for doing screen messages
*/

#include <stdarg.h>

#define DESIRED_SCREEN_WIDTH 640
#define DESIRED_SCREEN_HEIGHT 480

// Biggest possible of those defined
#define MAXIMUM_WORLD_WIDTH 1024
#define MAXIMUM_WORLD_HEIGHT 768

#define DEFAULT_WORLD_WIDTH 640
#define DEFAULT_WORLD_HEIGHT 320


// LP addition: view sizes and display data

struct ViewSizeData
{
	short OverallWidth, OverallHeight;	// Of the display area, so as to properly center everything
	short MainWidth, MainHeight;		// Of the main 3D-rendered view
	short WithHUD, WithoutHUD;			// Corresponding entries that are with the HUD or without it
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

/* ---------- globals */

struct color_table *uncorrected_color_table; /* the pristine color environment of the game (can be 16bit) */
struct color_table *world_color_table; /* the gamma-corrected color environment of the game (can be 16bit) */
struct color_table *interface_color_table; /* always 8bit, for mixed-mode (i.e., valkyrie) fades */
struct color_table *visible_color_table; /* the color environment the player sees (can be 16bit) */

struct view_data *world_view; /* should be static */

// Convenient package for the drawing target (contains dimensions and pixel-row pointers)
struct bitmap_definition *world_pixels_structure;

// LP change: added stuff for keeping track of screen sizes;
// this is for forcing the clearing of the screen when resizing.
// These are initialized to improbable values.
short PrevBufferWidth = INT16_MIN, PrevBufferHeight = INT16_MIN,
	PrevOffsetWidth = INT16_MIN, PrevOffsetHeight = INT16_MIN;

static struct screen_mode_data screen_mode;
static bool overhead_map_status= false;

#define FRAME_SAMPLE_SIZE 20
bool displaying_fps= false;
short frame_count, frame_index;
long frame_ticks[64];

// LP addition:
// whether to show one's position
bool ShowPosition = false;

// Whether rendering of the HUD has been requested
static bool HUD_RenderRequest = false;

static bool screen_initialized= false;

short bit_depth= NONE;
short interface_bit_depth= NONE;

// LP addition: this is defined in overhead_map.c
// It indicates whether to render the overhead map in OpenGL
extern bool OGL_MapActive;


// Current screen messages:
const int NumScreenMessages = 7;
struct ScreenMessage
{
	enum {
		Len = 256
	};

	int TimeRemaining;	// How many more engine ticks until the message expires?
	char Text[Len];		// Text to display
	
	ScreenMessage(): TimeRemaining(0) {Text[0] = 0;}
};

static int MostRecentMessage = NumScreenMessages-1;
static ScreenMessage Messages[NumScreenMessages];


/* ---------- private prototypes */

// LP change: the source and destination rects will be very variable, in general.
// Also indicating whether to use high or low resolution (the terminal and the ovhd map are always hi-rez)
static void update_screen(Rect& source, Rect& destination, bool hi_rez);
// static void update_screen(void);

static void set_overhead_map_status(bool status);
static void set_terminal_status(bool status);

/* ---------- code */

// LP addition: this resets the screen; useful when starting a game
void reset_screen()
{
	// Resetting cribbed from initialize_screen()
	world_view->overhead_map_scale= DEFAULT_OVERHEAD_MAP_SCALE;
	world_view->overhead_map_active= false;
	world_view->terminal_mode_active= false;
	world_view->horizontal_scale= 1, world_view->vertical_scale= 1;
	
	// LP change:
	ResetFieldOfView();	
}

// LP change: resets field of view to whatever the player had had when reviving
void ResetFieldOfView()
{
	world_view->tunnel_vision_active = false;

	if (current_player->extravision_duration)
	{
		world_view->field_of_view = EXTRAVISION_FIELD_OF_VIEW;
		world_view->target_field_of_view = EXTRAVISION_FIELD_OF_VIEW;
	}
	else
	{
		world_view->field_of_view = NORMAL_FIELD_OF_VIEW;
		world_view->target_field_of_view = NORMAL_FIELD_OF_VIEW;
	}
}


void zoom_overhead_map_out(
	void)
{
	world_view->overhead_map_scale= FLOOR(world_view->overhead_map_scale-1, OVERHEAD_MAP_MINIMUM_SCALE);
	
	return;
}

void zoom_overhead_map_in(
	void)
{
	world_view->overhead_map_scale= CEILING(world_view->overhead_map_scale+1, OVERHEAD_MAP_MAXIMUM_SCALE);
	
	return;
}

void start_teleporting_effect(
	bool out)
{
	if (View_DoFoldEffect())
		start_render_effect(world_view, out ? _render_effect_fold_out : _render_effect_fold_in);
}

void start_extravision_effect(
	bool out)
{
	// LP change: doing this by setting targets
	world_view->target_field_of_view = out ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW;
	// start_render_effect(world_view, out ? _render_effect_going_fisheye : _render_effect_leaving_fisheye);
}

// LP addition:
void start_tunnel_vision_effect(
	bool out)
{
	// LP change: doing this by setting targets
	world_view->target_field_of_view = out ? TUNNEL_VISION_FIELD_OF_VIEW : 
		((current_player->extravision_duration) ? EXTRAVISION_FIELD_OF_VIEW : NORMAL_FIELD_OF_VIEW);
	// start_render_effect(world_view, out ? _render_effect_going_tunnel : _render_effect_leaving_tunnel);
}

//CP addition: returns the screen info
screen_mode_data *get_screen_mode(
	void)
{
	return &screen_mode;
}

// LP: gets a size ID's related size ID's that show or hide the HUD, respectively
short GetSizeWithHUD(short Size)
{
	assert(Size >= 0 && Size < NUMBER_OF_VIEW_SIZES);
	return ViewSizes[Size].WithHUD;
}

short GetSizeWithoutHUD(short Size)
{
	assert(Size >= 0 && Size < NUMBER_OF_VIEW_SIZES);
	return ViewSizes[Size].WithoutHUD;
}

/* These should be replaced with better preferences control functions */
// LP change: generalizing this
bool game_window_is_full_screen(
	void)
{
	short msize = screen_mode.size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	return (!ViewSizes[msize].ShowHUD);
	
	// return screen_mode.size==_full_screen;
}


void change_gamma_level(
	short gamma_level)
{
	screen_mode.gamma_level= gamma_level;
	gamma_correct_color_table(uncorrected_color_table, world_color_table, gamma_level);
	stop_fade();
	obj_copy(*visible_color_table, *world_color_table);
	assert_world_color_table(interface_color_table, world_color_table);
	change_screen_mode(&screen_mode, false);
	set_fade_effect(NONE);
	
	return;
}

/* ---------- private code */

static void set_overhead_map_status( /* it has changed, this is the new status */
	bool status)
{
	world_view->overhead_map_active= status;
	
	return;
}

static void set_terminal_status( /* It has changed, this is the new state.. */
	bool status)
{
	static struct screen_mode_data previous_screen_mode;
	bool restore_effect= false;
	short effect, phase;
	
	if(!status)
	{
		if(world_view->effect==_render_effect_fold_out)
		{
			effect= world_view->effect;
			phase= world_view->effect_phase;
			restore_effect= true;
		}
	}
	world_view->terminal_mode_active= status;
	
	if(restore_effect)
	{
		world_view->effect= effect;
		world_view->effect_phase= phase;
	}

	/* Dirty the view.. */
	dirty_terminal_view(current_player_index);
	
	return;
}
// For getting and setting tunnel-vision mode
bool GetTunnelVision() {return world_view->tunnel_vision_active;}
bool SetTunnelVision(bool TunnelVisionOn)
{
	// LP: simplifying tunnel-vision-activation/deactivation behavior
	world_view->tunnel_vision_active = TunnelVisionOn;
	start_tunnel_vision_effect(TunnelVisionOn);
	return world_view->tunnel_vision_active;
}

// This is for requesting the drawing of the Heads-Up Display;
// this is done because its drawing is now done when the main display is drawn
void RequestDrawingHUD()
{
	HUD_RenderRequest = true;
}


// LP addition: display message on the screen;
// this really puts the current message into a buffer
// Code cribbed from csstrings
void screen_printf(char *format, ...)
{
	MostRecentMessage = (MostRecentMessage + 1) % NumScreenMessages;
	while (MostRecentMessage < 0)
		MostRecentMessage += NumScreenMessages;
	ScreenMessage& Message = Messages[MostRecentMessage];
	
	Message.TimeRemaining = 3*MACHINE_TICKS_PER_SECOND;

	va_list list;

	va_start(list,format);
	vsprintf(Message.Text,format,list);
	va_end(list);
}
