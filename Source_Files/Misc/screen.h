#ifndef _SCREEN_H_
#define _SCREEN_H_
/*
SCREEN.H
Thursday, August 24, 1995 5:36:27 PM  (Jason)

Feb 13, 2000 (Loren Petrich):
	Added screendump capability: dump_screen()

Mar 5, 2000 (Loren Petrich):
	Added reset_screen() function,
	for the purpose of resetting its state when starting a game

Mar 18, 2000 (Loren Petrich):
	Added OpenGL support, including OpenGL-acceleration mode

Jun 15, 2000 (Loren Petrich):
	Added support for Chris Pruett's Pfhortran

July 2, 2000 (Loren Petrich):
	Reversed the order of the screen-size symbolic constants, in preparation for really big
	screen sizes.
	
	The HUD is now always buffered

Jul 5, 2000 (Loren Petrich):
	Prepared for expanding the number of resolutions available
	by defining a number of view sizes

Dec 2, 2000 (Loren Petrich):
	Added support for hiding and re-showing the app
*/

/* ---------- constants */

// New screen-size definitions
enum /* screen sizes */
{
	_320_160_HUD,
	_480_240_HUD,
	_640_320_HUD,
	_640_480,
	_800_400_HUD,
	_800_600,
	_1024_512_HUD,
	_1024_768,
	NUMBER_OF_VIEW_SIZES
};
// Original screen-size definitions
enum /* screen sizes */
{
	_50_percent,
	_75_percent,
	_100_percent,
	_full_screen,
};

enum /* hardware acceleration codes */
{
	_no_acceleration,
	_opengl_acceleration,	// LP addition: OpenGL support
	_valkyrie_acceleration
};

/* ---------- missing from QUICKDRAW.H */

#define deviceIsGrayscale 0x0000
#define deviceIsColor 0x0001

/* ---------- structures */

// screen_mode_data in SHELL.H for PREFERENCES.H

/* ---------- globals */

extern struct color_table *world_color_table, *visible_color_table, *interface_color_table;

#ifdef mac
extern GDHandle world_device;
extern WindowPtr screen_window;
#endif

//CP Addition: make screen_mode_data usable here too
struct screen_mode_data;

/* ---------- prototypes/SCREEN.C */

void change_screen_clut(struct color_table *color_table);
void change_interface_clut(struct color_table *color_table);
void animate_screen_clut(struct color_table *color_table, bool full_screen);

void build_direct_color_table(struct color_table *color_table, short bit_depth);

void start_teleporting_effect(bool out);
void start_extravision_effect(bool out);

void render_screen(short ticks_elapsed);

void toggle_overhead_map_display_status(void);
void zoom_overhead_map_out(void);
void zoom_overhead_map_in(void);

void enter_screen(void);
void exit_screen(void);

void darken_world_window(void);
void validate_world_window(void);

void change_gamma_level(short gamma_level);

void assert_world_color_table(struct color_table *world_color_table, struct color_table *interface_color_table);

// LP change: added function for resetting the screen state when starting a game
void reset_screen();

// CP addition: added function to return the the game size
screen_mode_data *get_screen_mode(void);

void initialize_screen(struct screen_mode_data *mode);
void change_screen_mode(struct screen_mode_data *mode, bool redraw);

#if defined(mac)
void process_screen_key(EventRecord *event, short key);
void process_screen_click(EventRecord *event);

bool machine_supports_16bit(GDSpecPtr spec);
bool machine_supports_32bit(GDSpecPtr spec);
short hardware_acceleration_code(GDSpecPtr spec);

void activate_screen_window(WindowPtr window, EventRecord *event, bool active);
void update_screen_window(WindowPtr window, EventRecord *event);

// LP: for switching to another process and returning (suspend/resume events)
void SuspendDisplay(EventRecord *EvPtr);
void ResumeDisplay(EventRecord *EvPtr);

#elif defined(SDL)
void toggle_fullscreen(bool fs);
void update_screen_window(void);
void clear_screen(void);
#endif

void calculate_destination_frame(short size, bool high_resolution, Rect *frame);

// LP addition: a routine for dumping the screen contents into a file.
// May need to be modified for pass-through video cards like the older 3dfx ones.
void dump_screen();

// For getting and setting tunnel-vision mode
bool GetTunnelVision();
bool SetTunnelVision(bool TunnelVisionOn);

// Request for drawing the HUD
void RequestDrawingHUD();

// Corresponding with-and-without-HUD sizes for some view-size index,
// for the convenience of Pfhortran scripting;
// the purpose is to get a similar size of display with the HUD status possibly changed
short SizeWithHUD(short _size);
short SizeWithoutHUD(short _size);

#endif
