 /*
SCREEN.C

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

Saturday, August 21, 1993 11:55:26 AM

Saturday, December 18, 1993 2:17:05 PM
	removed visible_color_table and cinematic_fade_test, halfway through implementing the new
	startup CLUT munger.
Friday, August 12, 1994 6:59:29 PM
	more changes for 16-bit.
Friday, September 9, 1994 12:13:04 PM (alain)
	modified to get window from a resource. set fore and back color before using copybits.

Jan 30, 2000 (Loren Petrich):
	Did some typecasts
	Suppressed "process_screen_click", because it conflicts with routine of same name
		in shell.c
	Surrounded quadruple_screen with "extern "C"

Feb 1, 2000 (Loren Petrich):
	Added drop shadow to fps display in update_fps_display()
	Changed font size there to 14 (more readable)

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb. 5, 2000 (Loren Petrich):
	Suppressed WHITE_SCREEN_BETWEEN_FRAMES so as to get that smeared look in non-rendered places

Feb 13, 2000 (Loren Petrich):
	Added screendump capability: dump_screen()

Feb 18, 2000 (Loren Petrich):
	Added chase-cam support

Feb 21, 2000 (Loren Petrich):
	Turned fps drop shadow into a black halo
	Added crosshair support

Feb 24, 2000 (Loren Petrich):
	Made crosshairs invisible when using the terminal or the overhead map

Feb 25, 2000 (Loren Petrich):
	Added tunnel-vision-mode support

Feb 26, 2000 (Loren Petrich):
	Shrunk [I forget what I had wanted to write]

Mar 5, 2000 (Loren Petrich):
	Added reset_screen() function,
	for the purpose of resetting its state when starting a game

Mar 12, 2000 (Loren Petrich):
	Added OpenGL stuff

Mar 22, 2000 (Loren Petrich):
	Improved field-of-view management so that the FOV would be set properly
	when one enters a level.

Apr 19, 2000 (Loren Petrich):
	Suppressed Valkyrie code
	
Apr 30, 2000 (Loren Petrich):
	Added reloading of view context when reverting, so that OpenGL won't look funny when one
	changes a level.

May 2, 2000 (Loren Petrich):
	Added piping of overhead map and terminal graphics through OpenGL (optional)

May 21, 2000 (Loren Petrich):
	Disabled sound stopping for the terminal and overhead map

May 22, 2000 (Loren Petrich):
	Added suppression of overhead map if selected with <view map="false">...</view>
	
	Added setting of target field of view; also making initialize_world_view always called

May 28, 2000 (Loren Petrich):
	Added support for rendering the HUD

Jun 15, 2000 (Loren Petrich):
	Added support for Chris Pruett's Pfhortran

Jul 2, 2000 (Loren Petrich):
	The HUD is now always buffered, and the terminal is always at 100%

Jul 3, 2000 (Loren Petrich):
	OpenGL gets switched off if one tries to start in 256-color mode.

Jul 5, 2000 (Loren Petrich):
	Partially completed the task of generalizing the view-control setup so that it can be generalized
	to higher resolutions. This involved putting all the buffer-size and viewport calculation into
	render_screen(), and of getting as much resizing as possible into render_screen().
	The unavoidable exceptions are resized to 640*480, because Bungie's original resizers
	are inadequate for the task ahead.

Jul 6, 2000 (Loren Petrich):
	Got HUD's position generalized
	Fixed dump_screen so that it works properly in higher resolutions
	Added bigger screen-size values and increased the maximum size accordingly

Jul 8, 2000 (Loren Petrich):
	Modified render_screen() so that the two kinds of OpenGL rendering, for the main view
	and for the map view, are distinct, and that OpenGL for map view would be handled properly
	(text displayed in it, etc.)

Jul 17, 2000 (Loren Petrich):
	Added a view-effect reset to enter_screen() because initialize_view_data() no longer has it,
	in order to make teleport view-stretching work correctly.

Aug 3, 2000 (Loren Petrich):
	Removed all OGL_Map routines, because their contents are now handled in OverheadMapRenderer.c
	and subclass routines

Aug 9, 2000 (Loren Petrich):
	Removed OGL_StartMain() and OGL_EndMain() as redundant

Sept 9, 2000 (Loren Petrich):
	Added a kludge to make OpenGL mode well-behaved in 32-bit (millions of colors) screen depth
	-- reset the screen depth to 16-bit and then back to 32-bit.

Nov 29, 2000 (Loren Petrich):
	Added making view-folding effect optional

Dec 2, 2000 (Loren Petrich):
	Added support for hiding and re-showing the app

Dec 23, 2000
	Had a lot of trouble trying to get DrawSprocket to work properly.
	Also, moved stuff shared with screen_sdl.cpp into screen_shared.cpp
	
Dec 29, 2000 (Loren Petrich):
	Added stuff for doing screen messages
	
June 20, 2001 (Loren Petrich):
	Removing the DrawSprocket and replacing it with the Display Manager,
	so as to have less buggy resolution-switching. Something like Moo itself.
	Some of the code was inspired by Dietrich Epp's successful resolution-switching code.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
	Disabled low level fading code under Carbon
	Adjusted init of booleans in render_screen case that was causing compiler errors:
		jump to case label ... crosses initialization of `bool Use_OGL_2D'
	Included Steve Bytnar's OSX QDPort flushing code
	Trusting Suspend/Resume OSX mechanics to do the right thing with mouse and menubar

Feb 24, 2002 (Loren Petrich):
	Modified to use the new refresh_frequency field in the graphics prefs,
	so that a selection will be persistent.

Apr 29, 2002 (Loren Petrich):
	Added automatic setting of screen resolution on request

Apr 22, 2003 (Woody Zenfell):
        Enabling use of OGL_DrawHUD(); reducing unnecessary OGL_SwapBuffers()

  Aug 6, 2003 (Woody Zenfell):
	Fixing OGL HUD placement on widescreen displays; only drawing it in HUD screen modes.
*/

/*
//we expect Entries to leave the device color table untouched, and it doesn’t
*/

#include "macintosh_cseries.h"
#include "my32bqd.h"
// #include "valkyrie.h"

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

// LP addition: crosshairs support
#include "Crosshairs.h"
// LP addition: OpenGL support
#include "OGL_Render.h"

// LP addition: view control
#include "ViewControl.h"
#if !defined(TARGET_API_MAC_CARBON)
#include <Displays.h>
#endif

//CP addition: scripting support
#include "scripting.h"
#include "lua_script.h"

#include "Logging.h"

#ifndef __MACH__
// For loading the OSX fader, if present
#include <CFBundle.h>
#endif

// ZZZ: egads, this stuff just keeps getting hairier
extern bool OGL_HUDActive;

#ifdef env68k
#pragma segment screen
#endif

#ifdef DEBUG
//#define CYBERMAXX
// #define WHITE_SCREEN_BETWEEN_FRAMES
//#define DIRECT_SCREEN_TEST
#endif

#ifdef env68k
#define EXTERNAL
#endif

#ifdef envppc
#define EXTERNAL
#endif

#if defined(TARGET_API_MAC_CARBON)
#define SCREEN_BASED_SCREENSHOT 1
#else
#define SCREEN_BASED_SCREENSHOT 0
#endif

#ifdef CHUD_PROF
#include <CHUD/chudUtil.h>
#endif

/* ---------- constants */

#ifdef DIRECT_SCREEN_TEST
#define WORLD_H 160
#define WORLD_V 80
#else
#define WORLD_H 0
#define WORLD_V 0
#endif

enum
{
	kHUDHeight	= 160,
	kHUDWidth	= 640,
};

// now in screen_shared.cpp
//#define DESIRED_SCREEN_WIDTH 640
//#define DESIRED_SCREEN_HEIGHT 480
//#define MAXIMUM_WORLD_WIDTH 1024
//#define MAXIMUM_WORLD_HEIGHT 768
//#define DEFAULT_WORLD_WIDTH 640
//#define DEFAULT_WORLD_HEIGHT 320

#ifdef USES_NIBS
const CFStringRef Window_Main_Game = CFSTR("Main_Game");
const CFStringRef Window_Main_Backdrop = CFSTR("Main_Backdrop");
#else
#define windGAME_WINDOW 128
#define windBACKDROP_WINDOW 129
#endif

#ifdef envppc
#define EXTRA_MEMORY (100*KILO)
#else
#define EXTRA_MEMORY 0
#endif

#define FREE_MEMORY_FOR_16BIT (4*MEG+EXTRA_MEMORY)
#define FREE_MEMORY_FOR_32BIT (9*MEG+EXTRA_MEMORY)
#define FREE_MEMORY_FOR_FULL_SCREEN (3*MEG+EXTRA_MEMORY)

#define clutMARATHON8_CLUT_ID 5002


#ifdef USES_NIBS

const CFStringRef Window_Prefs_MonitorFreq = CFSTR("Prefs_MonitorFreq");

// Stuff for monitor-frequency dialog box, revised for the nib version
enum {
	MonitorFreq_Set = 1,	// Accept the current setting and quit
	MonitorFreq_Def,		// Revert to the original setting and cancel (iCANCEL)
	MonitorFreq_Accept,		// Accept the current setting without quitting
	MonitorFreq_Popup
};

#else

// Stuff for monitor-frequency dialog box
enum {
	MonitorFreq_Dialog = 300,
	MonitorFreq_Revert = 1,
	MonitorFreq_Def,
	MonitorFreq_Set,
	MonitorFreq_Popup
};

#endif

typedef ReqListRec *ReqListPtr;

/* ---------- structures */

#define _EVERY_OTHER_LINE_BIT 0x8000
#define _SIXTEEN_BIT_BIT 0x4000
#define _THIRTYTWO_BIT_BIT 0x2000

struct copy_screen_data
{
	pixel8 *source, *destination;
	short width, height, bytes_per_row;
	short source_slop, destination_slop;
	short flags;
};

/* ---------- globals */

GDHandle world_device; /* the device we’re running on */
WindowPtr screen_window; /* a window covering our entire device */
WindowPtr backdrop_window; /* a window covering all devices */

// LP note: world_pixels is the rendering buffer
// for the main view, the overhead map, and the terminals.
// The HUD has a separate buffer.
// It is initialized to NULL so as to allow its initing to be lazy.
GWorldPtr world_pixels = NULL;

static bool enough_memory_for_16bit, enough_memory_for_32bit, enough_memory_for_full_screen;

//static bool restore_depth_on_exit; /* otherwise restore CLUT */
//static short restore_depth, restore_flags; /* for SetDepth on exit */
static GDSpec restore_spec; // , resolution_restore_spec;

// For the Display Manager to restore the screen depth when done
static signed long RestoreScreenMode = 0;

// LP addition: a GWorld for the Heads-Up Display
// (if not allocated, then it's the null pointer).
// Also the currently-requested rendering area when doing in piped-through-OpenGL mode
GWorldPtr HUD_Buffer = NULL;
static Rect HUD_SourceRect = {0, 0, 160, 640};

// Flag for making 32-bit-OGL screen resetting done only once.
static bool ScreenFix_OGL32 = false;

// DrawSprocket activity:
static bool DM_Present = false;
static bool DM_Inited = false;

// Moved after the globals because the shared stuff might use those values
#include "screen_shared.h"


/* ---------- private prototypes */

// LP change: the source and destination rects will be very variable, in general.
// Also indicating whether to use high or low resolution (the terminal and the ovhd map are always hi-rez)
static void update_screen(Rect& source, Rect& destination, bool hi_rez);

void calculate_destination_frame(short size, bool high_resolution, Rect *frame);

static void restore_world_device(void);

extern "C" {
void quadruple_screen(struct copy_screen_data *data);
}

static void update_fps_display(GrafPtr port);

// LP addition: display the current position
static void DisplayPosition(GrafPtr port);

// Also display the current messages
static void DisplayMessages(GrafPtr port);

static void calculate_screen_options(void);

static void ClearScreen();

// Draws HUD in software mode
void DrawHUD(Rect& SourceRect, Rect& DestRect);

// Display Manager handlers:

// Checks on the Display Manager's presence
static bool DM_Check();

// For debugging DM errors
static bool DM_HandleError(char *Description, OSErr ErrorCode);

// Changes the monitor's resolution;
// returns the success of doing so
static bool DM_ChangeResolution(GDHandle Device, short BitDepth, short Width, short Height,
	bool ShowFreqDialog);

// Callback for getting display-mode info
static pascal void DM_ModeInfoCallback(void *UserData,
	DMListIndexType Index, DMDisplayModeListEntryPtr ModeInfoPtr);

// Callback for monitor-frequency-selection dialog-box event handling;
// it is for catching keyboard events
static pascal Boolean DM_ModeFreqDialogHandler(DialogPtr Dialog,
	EventRecord *Event, short *ItemHit);

// Parsing of mode name to get frequency:
static float GetFreqFromName(Str255 Name);

// Directly manipulate the video-driver color table.
// Split into two versions, one for OSX and one for Classic,
// because they use different sets of incompatible
// color-manipulation functions.

// Local; uses CGSetDisplayTransferByTable()
// which exists in OSX but not in CarbonLib or Classic
static void animate_screen_clut_osx(
	struct color_table *color_table,
	GDHandle DevHdl);

// Created with FaderClassic.mcp and stored in FaderLibClassic
extern void animate_screen_clut_classic(
	struct color_table *color_table,
	GDHandle DevHdl);

#ifndef __MACH__
// Cribbed from CGDirectDisplay.h documentation of CGSetDisplayTransferByTable()
// and Apple's "Quartz Primer
typedef CGDisplayErr 
(*CGSetDisplayTransferByTable_Type)(
  CGDirectDisplayID     display,
  CGTableCount          tableSize,
  const CGGammaValue *  redTable,
  const CGGammaValue *  greenTable,
  const CGGammaValue *  blueTable);

// Pointer to loaded symbol
static CGSetDisplayTransferByTable_Type CGSetDisplayTransferByTable_Ptr = NULL;

// Loads the above symbol
static void LoadCoreGraphicsFader();

#endif

#if USES_NIBS
static pascal OSStatus CEvtWindowUpdateEvents (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
#endif

/* ---------- code */
void initialize_screen(
	struct screen_mode_data *mode, bool ShowFreqDialog)
{
	OSErr error;
	Rect bounds;
	GrafPtr old_port;
	RgnHandle gray_region= GetGrayRgn();
	
	if (!screen_initialized)
	{
		/* Calculate the screen options-> 16, 32, full? */
		calculate_screen_options();
	}
	
	if (mode->bit_depth==32 && !enough_memory_for_32bit) mode->bit_depth= 16;
	if (mode->bit_depth==16 && !enough_memory_for_16bit) mode->bit_depth= 8;
	interface_bit_depth= bit_depth= mode->bit_depth;

	/* beg, borrow or steal an n-bit device */
	graphics_preferences->device_spec.bit_depth= interface_bit_depth;
	world_device= BestDevice(&graphics_preferences->device_spec);
	if (!world_device&&interface_bit_depth!=8)
	{
		graphics_preferences->device_spec.bit_depth= 8;
		world_device= BestDevice(&graphics_preferences->device_spec);
		if (world_device) mode->bit_depth= bit_depth= interface_bit_depth= 8;
	}
	if (!world_device) alert_user(fatalError, strERRORS, badMonitor, -1);

	// Grab the initial screen-depth mode for restoring when done
	if (!screen_initialized)
	{
		VDSwitchInfoRec SwitchInfo;
		OSErr Err = DMGetDisplayMode(world_device,&SwitchInfo);
		if (Err == noErr)
			RestoreScreenMode = SwitchInfo.csData;
	}
		
	// Use the Display Manager to change the display resolution;
	// otherwise, change to the one to restore
	if (graphics_preferences->screen_mode.fullscreen)
	{
		short msize = graphics_preferences->screen_mode.size;
		assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
		const ViewSizeData& VS = ViewSizes[msize];
		DM_ChangeResolution(world_device, graphics_preferences->device_spec.bit_depth,
			VS.OverallWidth, VS.OverallHeight, ShowFreqDialog);
	}
	else if (screen_initialized)
	{
		unsigned long TempBitDepth = restore_spec.bit_depth;
		DMSetDisplayMode(world_device,RestoreScreenMode,&TempBitDepth,NULL,NULL);
	}
	
	if (!screen_initialized)
	{
		graphics_preferences->device_spec.width= DESIRED_SCREEN_WIDTH;
		graphics_preferences->device_spec.height= DESIRED_SCREEN_HEIGHT;
		
		/* get rid of the menu bar */
		myHideMenuBar(GetMainDevice());
		
		uncorrected_color_table= new color_table;
		world_color_table= new color_table;
		visible_color_table= new color_table;
		interface_color_table= new color_table;
		assert(uncorrected_color_table && world_color_table && visible_color_table && interface_color_table);

#if defined(TARGET_API_MAC_CARBON)
#ifdef USES_NIBS
		error = CreateWindowFromNib(GUI_Nib,Window_Main_Backdrop, &backdrop_window);
		assert(error == noErr);
#else
		backdrop_window= GetNewCWindow(windBACKDROP_WINDOW, NULL, (WindowPtr) -1);
#endif
#else
		backdrop_window= (WindowPtr) NewPtr(sizeof(CWindowRecord));
		assert(backdrop_window);
		backdrop_window= GetNewCWindow(windBACKDROP_WINDOW, backdrop_window, (WindowPtr) -1);
#endif
		assert(backdrop_window);
//#if defined(USE_CARBON_ACCESSORS)
		Rect rgnBBox;
		GetRegionBounds(gray_region, &rgnBBox);
		MoveWindow(backdrop_window, rgnBBox.left, rgnBBox.top, false);
		SizeWindow(backdrop_window, RECTANGLE_WIDTH(&rgnBBox), RECTANGLE_HEIGHT(&rgnBBox), true);
/*
#else
		MoveWindow(backdrop_window, (**gray_region).rgnBBox.left, (**gray_region).rgnBBox.top, false);
		SizeWindow(backdrop_window, RECTANGLE_WIDTH(&(**gray_region).rgnBBox), RECTANGLE_HEIGHT(&(**gray_region).rgnBBox), true);
#endif
*/
		ShowWindow(backdrop_window);

#if defined(TARGET_API_MAC_CARBON)
#ifdef USES_NIBS
		error = CreateWindowFromNib(GUI_Nib,Window_Main_Game, &screen_window);
		assert(error == noErr);
#else
		screen_window= GetNewCWindow(windGAME_WINDOW, NULL, (WindowPtr) -1);
#endif
		//AS: slight speedup; disable shadow
		ChangeWindowAttributes(screen_window,kWindowNoShadowAttribute,0);
		// LP: So CodeWarrior can compile it
#ifndef __MWERKS__
		SetWindowAlpha(screen_window,1.0);
#endif
#else
		screen_window= (WindowPtr) NewPtr(sizeof(CWindowRecord));
		assert(screen_window);
		screen_window= GetNewCWindow(windGAME_WINDOW, screen_window, (WindowPtr) -1);
#endif
		assert(screen_window);
		SetWRefCon(screen_window, refSCREEN_WINDOW);
		ShowWindow(screen_window);

#if USES_NIBS
		static EventTypeSpec windowEvents[] = {
			{kEventClassWindow, kEventWindowDrawContent}
		};
		static EventHandlerUPP _CEWindowUpdateUPP = NewEventHandlerUPP(CEvtWindowUpdateEvents);
		InstallWindowEventHandler (screen_window, _CEWindowUpdateUPP, 1, windowEvents, NULL, NULL);
#endif

		/* allocate the bitmap_definition structure for our GWorld (it is reinitialized every frame */
		world_pixels_structure= (struct bitmap_definition *) NewPtr(sizeof(struct bitmap_definition)+sizeof(pixel8 *)*MAXIMUM_WORLD_HEIGHT);
		assert(world_pixels_structure);
		
		/* allocate and initialize our view_data structure; we don’t call initialize_view_data
			anymore (change_screen_mode does that) */
		world_view= (struct view_data *) NewPtr(sizeof(struct view_data));
		assert(world_view);
		world_view->field_of_view= NORMAL_FIELD_OF_VIEW; /* degrees (was 74 for a long, long time) */
		world_view->target_field_of_view= NORMAL_FIELD_OF_VIEW; // LP addition: for no change in FOV
		world_view->overhead_map_scale= DEFAULT_OVERHEAD_MAP_SCALE;
		world_view->overhead_map_active= false;
		world_view->terminal_mode_active= false;
		world_view->horizontal_scale= 1, world_view->vertical_scale= 1;
		// LP addition:
		world_view->tunnel_vision_active = false;
		
		/* make sure everything gets cleaned up after we leave */
		atexit(restore_world_device);
	
		world_pixels= (GWorldPtr) NULL;
	}
	else
	{
		unload_all_collections();
	}
	
	if (!screen_initialized || MatchGDSpec(&restore_spec)!=MatchGDSpec(&graphics_preferences->device_spec))
	{
		if (screen_initialized)
		{
			GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
			SetPort(GetWindowPort(screen_window));
			Rect portRect;
			GetPortBounds(GetWindowPort(screen_window), &portRect);
			PaintRect(&portRect);
/*
#else
			SetPort(screen_window);
			PaintRect(&screen_window->portRect);
#endif
*/
			SetPort(old_port);
			
			SetDepthGDSpec(&restore_spec);
		}
		BuildGDSpec(&restore_spec, world_device);
	}
	SetDepthGDSpec(&graphics_preferences->device_spec);

	MoveWindow(screen_window, (*world_device)->gdRect.left, (*world_device)->gdRect.top, false);
	SizeWindow(screen_window, RECTANGLE_WIDTH(&(*world_device)->gdRect), RECTANGLE_HEIGHT(&(*world_device)->gdRect), true);

	{
		Point origin;
		
		origin.h= - (RECTANGLE_WIDTH(&(*world_device)->gdRect)-DESIRED_SCREEN_WIDTH)/2;
		origin.v= - (RECTANGLE_HEIGHT(&(*world_device)->gdRect)-DESIRED_SCREEN_HEIGHT)/2;
		if (origin.v>0) origin.v= 0;
		
		GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
		SetPort(GetWindowPort(screen_window));
/*
#else
		SetPort(screen_window);
#endif
*/
		SetOrigin(origin.h, origin.v);
		SetPort(old_port);
	}
	
	/* allocate and initialize our GWorld */
	// LP change: doing a 640*480 allocation as a sensible starting point
	calculate_destination_frame(_full_screen, true, &bounds);
	error= world_pixels ? myUpdateGWorld(&world_pixels, 0, &bounds, (CTabHandle) NULL, (GDHandle) NULL, 0) :
		myNewGWorld(&world_pixels, 0, &bounds, (CTabHandle) NULL, (GDHandle) NULL, 0);
	if (error!=noErr) alert_user(fatalError, strERRORS, outOfMemory, error);
	
	change_screen_mode(mode, false);
	
	screen_initialized= true;
}


// LP change: force reload of view context
void ReloadViewContext()
{
	// Cribbed from enter_screen
	switch (screen_mode.acceleration)
	{
		// LP addition: doing OpenGL if present;
		// otherwise, switching OpenGL off (kludge for having it not appear)
		case _opengl_acceleration:
			if (screen_mode.bit_depth > 8)
//#if defined(USE_CARBON_ACCESSORS)
				OGL_StartRun(GetWindowPort(screen_window));
/*
#else
				OGL_StartRun((CGrafPtr)screen_window);
#endif
*/
			else screen_mode.acceleration = _no_acceleration;
			break;
	}
}


void enter_screen(
	void)
{
	GrafPtr old_port;
	
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
	Rect portRect;
	GetPortBounds(GetWindowPort(screen_window), &portRect);
	PaintRect(&portRect);
/*
#else
	SetPort(screen_window);
	PaintRect(&screen_window->portRect);
#endif
*/
	SetPort(old_port);

	if (world_view->overhead_map_active) set_overhead_map_status(false);
	if (world_view->terminal_mode_active) set_terminal_status(false);
	
	// LP change: adding this view-effect resetting here
	// since initialize_world_view() no longer resets it.
	world_view->effect = NONE;
	
	// Kludge to stop annoying 32-bit-mode flickering -- set to 16-bit, then to 32-bit again
	if ((screen_mode.acceleration == _opengl_acceleration) && screen_mode.bit_depth == 32)
	{
		// Be sure to do this only once, and only if one isn't doing 2D through OpenGL...
		if (!ScreenFix_OGL32 && !TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_2DGraphics))
		{
			GDSpec& DevSpec = graphics_preferences->device_spec;
			DevSpec.bit_depth = 16;
			SetDepthGDSpec(&DevSpec);
			DevSpec.bit_depth = 32;
			SetDepthGDSpec(&DevSpec);
			ScreenFix_OGL32 = true;
		}
	}

	// Of course, do this...
	change_screen_mode(&screen_mode, true);
	
	switch (screen_mode.acceleration)
	{
		// LP addition: doing OpenGL if present
		// otherwise, switching OpenGL off (kludge for having it not appear)
		case _opengl_acceleration:
			if (screen_mode.bit_depth > 8)
//#if defined(USE_CARBON_ACCESSORS)
				OGL_StartRun(GetWindowPort(screen_window));
/*
#else
				OGL_StartRun((CGrafPtr)screen_window);
#endif
*/
			else screen_mode.acceleration = _no_acceleration;
			break;
	}
}

void exit_screen(
	void)
{
	// LP addition: doing OpenGL if present
	OGL_StopRun();
}


// LP addition: added stuff to keep the sounds going if desired
/*
static bool stop_sounds = true;

void change_screen_mode_keep_sounds(
	struct screen_mode_data *mode,
	bool redraw)
{
	stop_sounds = false;
	change_screen_mode(mode,redraw);
	stop_sounds = true;
}
*/

void change_screen_mode(
	struct screen_mode_data *mode,
	bool redraw)
{
	// LP: stripped it down to a minimum
	
	// Get the screen mode here
	screen_mode = *mode;
	
	// "Redraw" means clear the screen
	if (redraw) ClearScreen();
	
	frame_count= frame_index= 0;
}

void render_screen(
	short ticks_elapsed)
{
	PixMapHandle pixels;
#ifdef CHUD_PROF
    chudStartStopAmber();
#endif
	/* make whatever changes are necessary to the world_view structure based on whichever player
		is frontmost */
	world_view->ticks_elapsed= ticks_elapsed;
	world_view->tick_count= dynamic_world->tick_count;
	world_view->yaw= current_player->facing;
	world_view->pitch= current_player->elevation;
	world_view->maximum_depth_intensity= current_player->weapon_intensity;
	world_view->shading_mode= current_player->infravision_duration ? _shading_infravision : _shading_normal;

	// LP change: suppress the overhead map if desired
	if (PLAYER_HAS_MAP_OPEN(current_player) && View_MapActive())
	{
		if (!world_view->overhead_map_active) set_overhead_map_status(true);
	}
	else
	{
		if (world_view->overhead_map_active) set_overhead_map_status(false);
	}

	if(player_in_terminal_mode(current_player_index))
	{
		if (!world_view->terminal_mode_active) set_terminal_status(true);
	} else {
		if (world_view->terminal_mode_active) set_terminal_status(false);
	}

	// LP change: set rendering-window bounds for the current sort of display to render
	// A Rect is, in order, top, left, bottom, right
	screen_mode_data *mode = &screen_mode;
//#if defined(USE_CARBON_ACCESSORS)
	Rect ScreenRect;
	GetPortBounds(GetWindowPort(screen_window), &ScreenRect);
/*
#else
	Rect ScreenRect = screen_window->portRect;
#endif
*/
	Rect BufferRect, ViewRect;
	bool HighResolution;
	
	short msize = mode->size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	const ViewSizeData& VS = ViewSizes[msize];
	
	// Rectangle where the view is to go (must not overlap the HUD)
	short OverallWidth = VS.OverallWidth;
	short OverallHeight = VS.OverallHeight - (VS.ShowHUD ? kHUDHeight : 0);
	short BufferWidth, BufferHeight;
	
	// Offsets for placement in the screen
	short ScreenOffsetWidth = ((RECTANGLE_WIDTH(&ScreenRect) - VS.OverallWidth) >> 1) + ScreenRect.left;
	short ScreenOffsetHeight = ((RECTANGLE_HEIGHT(&ScreenRect) - VS.OverallHeight) >> 1) + ScreenRect.top;
	
	// HUD location
	Rect HUD_DestRect;
	short HUD_Offset = (OverallWidth - kHUDWidth) / 2;
	SetRect(&HUD_DestRect, HUD_Offset, OverallHeight, HUD_Offset + kHUDWidth, VS.OverallHeight);
	OffsetRect(&HUD_DestRect, ScreenOffsetWidth, ScreenOffsetHeight);
	
	bool ChangedSize = false;
	
	// Each kind of display needs its own size
	if (world_view->terminal_mode_active)
	{
		// Standard terminal size
		BufferWidth = 640;
		BufferHeight = 320;
		HighResolution = true;		
	}
	else if (world_view->overhead_map_active)
	{
		// Fill the available space
		BufferWidth = OverallWidth;
		BufferHeight = OverallHeight;
		HighResolution = true;		
	}
	else
	{
		BufferWidth = VS.MainWidth;
		BufferHeight = VS.MainHeight;
		HighResolution = OGL_IsActive() ? true : mode->high_resolution;
	}
	
	if (BufferWidth != PrevBufferWidth)
	{
		ChangedSize = true;
		PrevBufferWidth = BufferWidth;
	}
	if (BufferHeight != PrevBufferHeight)
	{
		ChangedSize = true;
		PrevBufferHeight = BufferHeight;
	}
	
	// Do the buffer/viewport rectangle setup:
	
	// First, the destination rectangle (viewport to be drawn in)
	SetRect(&ViewRect,0,0,BufferWidth,BufferHeight);
	short OffsetWidth = (OverallWidth - BufferWidth) >> 1;
	short OffsetHeight = (OverallHeight - BufferHeight) >> 1;
	OffsetRect(&ViewRect, OffsetWidth+ScreenOffsetWidth, OffsetHeight+ScreenOffsetHeight);
	
	if (OffsetWidth != PrevOffsetWidth)
	{
		ChangedSize = true;
		PrevOffsetWidth = OffsetWidth;
	}
	if (OffsetHeight != PrevOffsetHeight)
	{
		ChangedSize = true;
		PrevOffsetHeight = OffsetHeight;
	}
	
	// Now the buffer rectangle; be sure to shrink it as appropriate
	if (!HighResolution)
	{
		BufferWidth >>= 1;
		BufferHeight >>= 1;
	}
	SetRect(&BufferRect,0,0,BufferWidth,BufferHeight);
	
	// Set up view data appropriately (cribbed from change_screen_mode)
	world_view->screen_width= BufferWidth;
	world_view->screen_height= BufferHeight;
	world_view->standard_screen_width= 2*BufferHeight;	
	initialize_view_data(world_view);
	
	if (world_pixels)
	{
		// Check on the drawing buffer's size
//#if defined(USE_CARBON_ACCESSORS)
		Rect portRect;
		GetPortBounds(world_pixels, &portRect);
		if (RECTANGLE_WIDTH(&portRect) != BufferWidth) ChangedSize = true;
		if (RECTANGLE_HEIGHT(&portRect) != BufferHeight) ChangedSize = true;
/*
#else
		if (RECTANGLE_WIDTH(&world_pixels->portRect) != BufferWidth) ChangedSize = true;
		if (RECTANGLE_HEIGHT(&world_pixels->portRect) != BufferHeight) ChangedSize = true;
#endif
*/
	}
	else ChangedSize = true;
	
	if (ChangedSize)
	{
		ClearScreen();
		if (VS.ShowHUD) draw_interface();
		
		// Reallocate the drawing buffer
		short error = world_pixels ?
			myUpdateGWorld(&world_pixels, 0, &BufferRect, (CTabHandle) NULL, (GDHandle) NULL, 0) :
				myNewGWorld(&world_pixels, 0, &BufferRect, (CTabHandle) NULL, (GDHandle) NULL, 0);
		if (error!=noErr) alert_user(fatalError, strERRORS, outOfMemory, error);
	}
	
	// Be sure that the main view is buffered...
	OGL_SetWindow(ScreenRect,ViewRect, true);
	
	// LP: Resizing must come *before* locking the GWorld's pixels
	myLockPixels(world_pixels);
	pixels= myGetGWorldPixMap(world_pixels);
	
	switch (screen_mode.acceleration)
	{
		/* if we’re using the overhead map, fall through to no acceleration */
		// LP change: might want to init OpenGL rendering here
		case _opengl_acceleration:
			if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
			{
				break;
			}
		case _no_acceleration:
			world_pixels_structure->width= world_view->screen_width;
			world_pixels_structure->height= world_view->screen_height;
			world_pixels_structure->flags= 0;
/* #ifdef DIRECT_SCREEN_TEST
			pixels= (*world_device)->gdPMap; //((CGrafPtr)screen_window)->portPixMap;
			world_pixels_structure->bytes_per_row= (*pixels)->rowBytes&0x3fff;
			world_pixels_structure->row_addresses[0]= ((pixel8 *)(*pixels)->baseAddr) + WORLD_H +
				WORLD_V*world_pixels_structure->bytes_per_row;
			*//* blanking the screen is not supported in direct-to-screen mode */
// #else /* !DIRECT_SCREEN_TEST */
			world_pixels_structure->bytes_per_row= (*pixels)->rowBytes&0x3fff;
			world_pixels_structure->row_addresses[0]= (pixel8 *) myGetPixBaseAddr(world_pixels);
// #ifdef WHITE_SCREEN_BETWEEN_FRAMES
// LP change: will use OpenGL option for this
			if (TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_VoidColor))
			{
				/* The terminal stuff only draws when necessary */
				if(!world_view->terminal_mode_active)
				{
					GDHandle old_device;
					CGrafPtr old_port;
					
					myGetGWorld(&old_port, &old_device);
					mySetGWorld(world_pixels, (GDHandle) NULL);
					RGBColor SavedColor;
					GetBackColor(&SavedColor);
					RGBBackColor(&Get_OGL_ConfigureData().VoidColor);
//#if defined(USE_CARBON_ACCESSORS)
					Rect portRect;
					GetPortBounds(world_pixels, &portRect);
					EraseRect(&portRect);
/*
#else
					EraseRect(&world_pixels->portRect);
#endif
*/
					RGBBackColor(&SavedColor);
					mySetGWorld(old_port, old_device);
				}
			}
// #endif /* WHITE_SCREEN_BETWEEN_FRAMES */
// #endif /* else DIRECT_SCREEN_TEST */
			precalculate_bitmap_row_addresses(world_pixels_structure);
			break;
		
		default:
			assert(false);
			break;
	}

	// assert(world_view->screen_height<=MAXIMUM_WORLD_HEIGHT);

// LP change: Cybermaxx support -- this indicates how to do binocular vision
	/* render left and right images on alternating scan lines */
/*
	{
		world_point3d left_origin, right_origin;
		short left_polygon_index, right_polygon_index;
		angle left_angle, right_angle;

		get_binocular_vision_origins(current_player_index, &left_origin, &left_polygon_index, &left_angle,
			&right_origin, &right_polygon_index, &right_angle);
		
		world_pixels_structure->bytes_per_row*= 2;
		precalculate_bitmap_row_addresses(world_pixels_structure);
		world_view->origin= left_origin;
		world_view->origin_polygon_index= left_polygon_index;
		world_view->yaw= left_angle;
		render_view(world_view, world_pixels_structure);
		
		world_pixels_structure->row_addresses[0]+= world_pixels_structure->bytes_per_row/2;
		precalculate_bitmap_row_addresses(world_pixels_structure);
		world_view->origin= right_origin;
		world_view->origin_polygon_index= right_polygon_index;
		world_view->yaw= right_angle;
		render_view(world_view, world_pixels_structure);
	}
*/
	world_view->origin= current_player->camera_location;
	world_view->origin_polygon_index= current_player->camera_polygon_index;
	// LP addition: chase-cam support
	//CP Change: add script-based camera control
	if (!UseLuaCameras() && !script_Camera_Active())
	{
		// LP addition: chase-cam support
		world_view->show_weapons_in_hand =
			!ChaseCam_GetPosition(world_view->origin,world_view->origin_polygon_index,world_view->yaw,world_view->pitch);
	}
	
	// Is map to be drawn with OpenGL?
	if (OGL_IsActive() && world_view->overhead_map_active)
		OGL_MapActive = TEST_FLAG(Get_OGL_ConfigureData().Flags,OGL_Flag_Map);
	else
		OGL_MapActive = false;

        // Is HUD to be drawn with OpenGL?
        if (OGL_IsActive())
                OGL_HUDActive = TEST_FLAG(Get_OGL_ConfigureData().Flags, OGL_Flag_HUD);
        else
                OGL_HUDActive = false;

        render_view(world_view, world_pixels_structure);
	
	switch (screen_mode.acceleration)
	{
		// LP change: OpenGL acceleration included with no acceleration here
		case _opengl_acceleration:
		case _no_acceleration:
			update_fps_display((GrafPtr)world_pixels);
			// LP additions: display position and messages and show crosshairs
			if (!world_view->terminal_mode_active)
				DisplayPosition((GrafPtr)world_pixels);
			DisplayMessages((GrafPtr)world_pixels);
			
			// Don't show the crosshairs when either the overhead map or the terminal is active
			if (!world_view->overhead_map_active && !world_view->terminal_mode_active)
			  if (NetAllowCrosshair())
			    if (Crosshairs_IsActive())
			      if (!OGL_RenderCrosshairs())
				Crosshairs_Render((GrafPtr)world_pixels);
// #ifndef DIRECT_SCREEN_TEST
			// LP change: put in OpenGL buffer swapping when the main view or the overhead map
			// are being rendered in OpenGL.
			// Otherwise, if OpenGL is active, then blit the software rendering to the screen.
			bool Use_OGL_2D;
			Use_OGL_2D = OGL_Get2D();
			if ((OGL_MapActive || !world_view->overhead_map_active) && !world_view->terminal_mode_active)
			{
				// Main or map view already rendered
			}
			else
			{
				// Copy 2D rendering to screen
				// Paint onto the back buffer, so that it will be copied frontward properly
//#if defined(USE_CARBON_ACCESSORS)
				Rect portRect;
				GetPortBounds(world_pixels, &portRect);
				OGL_Copy2D(world_pixels, portRect, portRect,true,false);
/*
#else
				OGL_Copy2D(world_pixels,world_pixels->portRect,world_pixels->portRect,true,false);
#endif
*/
			}
			if (!OGL_IsActive())
                                update_screen(BufferRect,ViewRect,HighResolution);

                        if(OGL_HUDActive)
                        {
				if(VS.ShowHUD)
				{
					OGL_SetWindow(ScreenRect,ScreenRect,true);
					Rect theRealHUDDestRect;
					if(msize < 4)
						SetRect(&theRealHUDDestRect, 0, 320, 640, 480);
					else
					{
						short theHUDAreaHeight = VS.OverallHeight - VS.MainHeight;
						theRealHUDDestRect.left = (VS.OverallWidth - kHUDWidth)/2;
						theRealHUDDestRect.top = VS.MainHeight + (theHUDAreaHeight - kHUDHeight) / 2;
						theRealHUDDestRect.right = theRealHUDDestRect.left + kHUDWidth;
						theRealHUDDestRect.bottom = theRealHUDDestRect.top + kHUDHeight;
					}

					if(!graphics_preferences->screen_mode.fullscreen)
					{
						int theXOffset = (RECTANGLE_WIDTH(&ScreenRect) - VS.OverallWidth)/2;
						int theYOffset = (RECTANGLE_HEIGHT(&ScreenRect) - VS.OverallHeight)/2;
						OffsetRect(&theRealHUDDestRect, theXOffset, theYOffset);
					}

					// we try to log these once per game here.  log level is dump, so
     // only those wanting lots of detail will ever see it.
					static bool logged = false; // for debugging

					if(dynamic_world->tick_count > 150 && dynamic_world->tick_count < 180)
					{
						if(!logged)
						{
							logDump4("ScreenRect: {%d, %d, %d, %d}", ScreenRect.left, ScreenRect.top, ScreenRect.right, ScreenRect.bottom);
							logDump4("HUD_DestRect: {%d, %d, %d, %d}", HUD_DestRect.left, HUD_DestRect.top, HUD_DestRect.right, HUD_DestRect.bottom);
							logDump4("theRealHUDDestRect: {%d, %d, %d, %d}", theRealHUDDestRect.left, theRealHUDDestRect.top, theRealHUDDestRect.right, theRealHUDDestRect.bottom);
							logged = true;
						}
					}
					else
						logged = false;

					OGL_DrawHUD(theRealHUDDestRect, ticks_elapsed);

				} // current mode should show HUD

			} // rendering HUD with OpenGL
			else {
                                if (HUD_RenderRequest)
                                {
                                        if (Use_OGL_2D)
                                        {
                                                // This horrid-looking resizing does manage to get the HUD to work properly...
                                                struct DownwardOffsetSet
                                                {
                                                        short Top, Bottom;
                                                };
                                                // Formula: 1st value = 40 + (3/16)*(true 1st value)
                                                const DownwardOffsetSet OGL_DownwardOffsets[NUMBER_OF_VIEW_SIZES] =
                                                {
                                                        {160, 480},		//  _320_160_HUD
                                                        {160, 480},		//  _480_240_HUD
                                                        {160, 480},		//  _640_320_HUD
                                                        {160, 480},		//  _640_480
                                                        {190, 600},		//  _800_400_HUD
                                                        {190, 600},		//  _800_600
                                                        {232, 768},		// _1024_512_HUD
                                                        {232, 768},		// _1024_768
                                                        {280, 1024},	// _1280_640_HUD
                                                        {280, 1024},	// _1280_1024
                                                        {340, 1200},	// _1600_800_HUD
                                                        {340, 1200},	// _1600_1200
                                                        {204, 640},	// _1024_440_HUD
                                                        {204, 640},	// _1024_640
                                                        {244, 800},	// _1280_600_HUD
                                                        {244, 800},	// _1280_800
                                                        {244, 854},	// _1280_640WS_HUD
                                                        {244, 854},	// _1280_854
                                                        {260, 854},	// _1440_700_HUD
                                                        {260, 854},	// _1440_900
                                                        {284, 1050},	// _1680_840_HUD
                                                        {284, 1050},	// _1680_1050
                                                        {340, 1200},	// _1920_950_HUD
                                                        {340, 1200},	// _1920_1200
                                                };
                                                const DownwardOffsetSet& Set = OGL_DownwardOffsets[msize];
                                                HUD_DestRect.top = Set.Top;
                                                HUD_DestRect.bottom = Set.Bottom;
                                                // Paint on the back buffer and flip it to the front
                                                OGL_SetWindow(ScreenRect,HUD_DestRect,true);
                                                OGL_Copy2D(HUD_Buffer,HUD_SourceRect,HUD_DestRect,true,false);
                                        }
                                        else
                                                DrawHUD(HUD_SourceRect,HUD_DestRect);
                                        HUD_RenderRequest = false;
                                }
                        }

                        if(OGL_IsActive()) OGL_SwapBuffers();
// #endif
			break;
		
		default:
			assert(false);
			break;
	}

	myUnlockPixels(world_pixels);
#ifdef CHUD_PROF
	chudStartStopAmber();
#endif
}

void change_interface_clut(
	struct color_table *color_table)
{
	obj_copy(*interface_color_table, *color_table);
}

/* builds color tables suitable for SetEntries (in either bit depth) and makes the screen look
	like we expect it to */
void change_screen_clut(
	struct color_table *color_table)
{
	if (interface_bit_depth==8 && bit_depth==8)
	{
		obj_copy(*uncorrected_color_table, *color_table);
		obj_copy(*interface_color_table, *color_table);
	}
	
	if (bit_depth==16 || bit_depth==32)
	{
		build_direct_color_table(uncorrected_color_table, bit_depth);
		if (interface_bit_depth!=8)
		{
			obj_copy(*interface_color_table, *uncorrected_color_table);
		}
	}

	gamma_correct_color_table(uncorrected_color_table, world_color_table, screen_mode.gamma_level);
	obj_copy(*visible_color_table, *world_color_table);

	/* switch to our device, stuff in our corrected color table */
	assert_world_color_table(interface_color_table, world_color_table);

	/* and finally, make sure the ctSeeds of our GWorld and our GDevice are synchronized */
	{
		Rect bounds;
		OSErr error;
	
		// LP change: doing a 640*480 allocation as a sensible starting point
		calculate_destination_frame(_full_screen, true, &bounds);
		error= myUpdateGWorld(&world_pixels, 0, &bounds, (CTabHandle) NULL, (GDHandle) NULL, 0);
		if (error!=noErr) alert_user(fatalError, strERRORS, outOfMemory, error);
	}
}

void build_direct_color_table(
	struct color_table *color_table,
	short bit_depth)
{
	struct rgb_color *color;
	short maximum_component;
	short i;
	
	switch (bit_depth)
	{
		case 16: maximum_component= PIXEL16_MAXIMUM_COMPONENT; break;
		case 32: maximum_component= PIXEL32_MAXIMUM_COMPONENT; break;
		default:
			assert(false);
			break;
	}
	
	color_table->color_count= maximum_component+1;
	for (i= 0, color= color_table->colors; i<=maximum_component; ++i, ++color)
	{
		color->red= color->green= color->blue= (i*0xffff)/maximum_component;
	}
}

void render_computer_interface(
	struct view_data *view)
{
	GWorldPtr old_gworld;
	GDHandle old_device;
	struct view_terminal_data data;

	GetGWorld(&old_gworld, &old_device);
	SetGWorld(world_pixels, (GDHandle) NULL);
	
	// LP change: calculate the view dimensions directly from the GWorld's boundaries
//#if defined(USE_CARBON_ACCESSORS)
	Rect WPRect;
	GetPortBounds(world_pixels, &WPRect);
/*
#else
	Rect& WPRect = ((CGrafPtr)world_pixels)->portRect;
#endif
*/
	data.left = WPRect.left;
	data.right = WPRect.right;
	data.top = WPRect.top;
	data.bottom = WPRect.bottom;
	data.vertical_offset = 0;
	
	/*
	data.left= 0; data.right= view->screen_width;
	data.top= 0; data.bottom= view->screen_height;
	
	switch (screen_mode.size)
	{
			case _50_percent:
			case _75_percent:
			// LP change: moved "full screen" up here, because this is supposed to be like 100%
			// in all cases
			case _full_screen:
				// LP change:
				assert(false);
				// halt();
				break;
				
			case _100_percent:
				data.vertical_offset= 0;
				break;
			case _full_screen:
				data.vertical_offset= (view->screen_height-DEFAULT_WORLD_HEIGHT)/2;
				break;
	}
	*/
	
	_render_computer_interface(&data);

	RGBForeColor(&rgb_black);
	PenSize(1, 1);
	TextFont(0);
	TextFace(normal);
	TextSize(0);
	SetGWorld(old_gworld, old_device);
}

void render_overhead_map(
	struct view_data *view)
{
	GWorldPtr old_gworld;
	GDHandle old_device;
	struct overhead_map_data overhead_data;

	GetGWorld(&old_gworld, &old_device);
	SetGWorld(world_pixels, (GDHandle) NULL);
	
//#if defined(USE_CARBON_ACCESSORS)
	Rect WPRect;
	GetPortBounds(world_pixels, &WPRect);
	PaintRect(&WPRect);
/*
#else
	PaintRect(&world_pixels->portRect);
#endif
*/
	
	// LP change: calculate the view dimensions directly from the GWorld's boundaries
/*
#if !defined(USE_CARBON_ACCESSORS)
	Rect& WPRect = ((CGrafPtr)world_pixels)->portRect;
#endif
*/
	overhead_data.left = WPRect.left;
	overhead_data.top = WPRect.top;
	overhead_data.half_width = (overhead_data.width = RECTANGLE_WIDTH(&WPRect)) >> 1;
	overhead_data.half_height = (overhead_data.height = RECTANGLE_HEIGHT(&WPRect)) >> 1;

	overhead_data.scale= view->overhead_map_scale;
	overhead_data.mode= _rendering_game_map;
	overhead_data.origin.x= view->origin.x;
	overhead_data.origin.y= view->origin.y;
	
	_render_overhead_map(&overhead_data);

	RGBForeColor(&rgb_black);
	PenSize(1, 1);
	TextFont(0);
	TextFace(normal);
	TextSize(0);
	SetGWorld(old_gworld, old_device);
}

// Accessors for the screen's GDevice and GrafPort;
// this abstraction is used for DrawSprocket support
GDHandle GetWorldDevice()
{	
	return world_device;
}

CGrafPtr GetScreenGrafPort() {
//#if defined(USE_CARBON_ACCESSORS)
	return GetWindowPort(screen_window);
/*
#else
	return (CGrafPtr)screen_window;
#endif
*/
}


bool machine_supports_16bit(
	GDSpecPtr spec)
{
	GDSpec test_spec;
	bool found_16bit_device= true;

	/* Make sure that enough_memory_for_16bit is valid */
	calculate_screen_options();
	
	test_spec= *spec;
	test_spec.bit_depth= 16;
	if (!HasDepthGDSpec(&test_spec)) /* automatically searches for grayscale devices */
	{
		found_16bit_device= false;
	}
	
	return found_16bit_device && enough_memory_for_16bit;
}

bool machine_supports_32bit(
	GDSpecPtr spec)
{
	GDSpec test_spec;
	bool found_32bit_device= true;

	/* Make sure that enough_memory_for_16bit is valid */
	calculate_screen_options();

	test_spec= *spec;
	test_spec.bit_depth= 32;
	if (!HasDepthGDSpec(&test_spec)) /* automatically searches for grayscale devices */
	{
		found_32bit_device= false;
	}
	
	return found_32bit_device && enough_memory_for_32bit;
}

short hardware_acceleration_code(
	GDSpecPtr spec)
{
	short acceleration_code= OGL_IsPresent() ? _opengl_acceleration : _no_acceleration;
	
	calculate_screen_options();
	
	return acceleration_code;
}

void update_screen_window(
	WindowPtr window,
	EventRecord *event)
{
	(void) (window);
	(void) (event);
		
	draw_interface();
	change_screen_mode(&screen_mode, true);
	assert_world_color_table(interface_color_table, world_color_table);
}

void activate_screen_window(
	WindowPtr window,
	EventRecord *event,
	bool active)
{
	(void) (window);
	(void) (event);
	(void) (active);
}


extern int BeepTwice( void );


/* LowLevelSetEntries bypasses the Color Manager and goes directly to the hardware.  this means
	QuickDraw doesn’t fuck up during RGBForeColor and RGBBackColor. */
void animate_screen_clut(
	struct color_table *color_table,
	bool full_screen)
{
#ifdef __MACH__

	animate_screen_clut_osx(color_table, world_device);
	
#else

	LoadCoreGraphicsFader();
	
	// Try the OSX version, and then the Classic version
	if (CGSetDisplayTransferByTable_Ptr != NULL)
	{
		animate_screen_clut_osx(color_table, world_device);
	}
	else
	{
		// Check to see if this function is actually present before trying to call it;
		// it is in a shared library
		if (animate_screen_clut_classic != NULL)
			animate_screen_clut_classic(color_table, world_device);
	}

#endif
/*
#if !defined(TARGET_API_MAC_CARBON)
	// Use special direct animation if the bit depth > 8;
	// this is for the benefit of MacOS X Classic
	// AS turned off when not in Classic
	if ((bit_depth > 8) && (system_information->machine_is_bluebox == true))
	{
		direct_animate_screen_clut(color_table, world_device);
		return;
	}
	
	CTabHandle macintosh_color_table= build_macintosh_color_table(color_table);
	
	if (macintosh_color_table)
	{
		GDHandle old_device;
		HLock((Handle)macintosh_color_table);
		old_device= GetGDevice();
		SetGDevice(world_device);
		
		switch (screen_mode.acceleration)
		{
			// LP change: added OpenGL behavior
			case _opengl_acceleration:
			case _no_acceleration:
				LowLevelSetEntries(0, (*macintosh_color_table)->ctSize, (*macintosh_color_table)->ctTable);
				break;
		}
		
		DisposeHandle((Handle)macintosh_color_table);
		SetGDevice(old_device);
	}
#else

	// Uses Carbon enabled version
	direct_animate_screen_clut(color_table, world_device);	
//#endif
*/
}

void assert_world_color_table(
	struct color_table *interface_color_table,
	struct color_table *world_color_table)
{
	if (interface_bit_depth==8)
	{
		CTabHandle macintosh_color_table= build_macintosh_color_table(interface_color_table);
		
		if (macintosh_color_table)
		{
			GDHandle old_device;
			
			HLock((Handle)macintosh_color_table);
			old_device= GetGDevice();
			SetGDevice(world_device);
			SetEntries(0, (*macintosh_color_table)->ctSize, (*macintosh_color_table)->ctTable);
			SetGDevice(old_device);
			
			DisposeHandle((Handle)macintosh_color_table);
		}
	}
	
	if (world_color_table) animate_screen_clut(world_color_table, false);
}

void darken_world_window(
	void)
{
	GrafPtr old_port;
	Rect bounds;
	
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
	Pattern pattern;
	GetQDGlobalsGray(&pattern);
	PenPat(&pattern);
/*
#else
	SetPort(screen_window);
	PenPat(&qd.gray);
#endif
*/
	PenMode(srcOr);
	calculate_destination_frame(screen_mode.size, screen_mode.high_resolution, &bounds);
	PaintRect(&bounds);
	PenMode(srcCopy);
//#if defined(USE_CARBON_ACCESSORS)
	GetQDGlobalsBlack(&pattern);
	PenPat(&pattern);
/*
#else        
	PenPat(&qd.black);
#endif
*/
	SetPort(old_port);
}

void validate_world_window(
	void)
{
	GrafPtr old_port;
	Rect bounds;
	
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
/*
#else
	SetPort(screen_window);
#endif
*/
	calculate_destination_frame(screen_mode.size, screen_mode.high_resolution, &bounds);
//#if defined(USE_CARBON_ACCESSORS)
	ValidWindowRect(screen_window, &bounds);
/*
#else
	ValidRect(&bounds);
#endif
*/
	SetPort(old_port);
}


/* ---------- private code */

#ifdef MOVED_OUT
// LP addition: routine for displaying text
static void DisplayText(short BaseX, short BaseY, const char *Text)
{
	// OpenGL version:
	// activate only in the main view, and also if OpenGL is being used for the overhead map
	if((OGL_MapActive || !world_view->overhead_map_active) && !world_view->terminal_mode_active)
		if (OGL_RenderText(BaseX, BaseY, Text)) return;
	
	// C to Pascal
	Str255 PasText;
	
	int Len = MIN(strlen(Text),255);
	PasText[0] = Len;
	memcpy(PasText+1,Text,Len);
	
	// LP change: added black halo
	RGBForeColor(&rgb_black);
	MoveTo(BaseX+1,BaseY+1);
	DrawString(PasText);

	RGBForeColor(&rgb_white);
	MoveTo(BaseX,BaseY);
	DrawString(PasText);
}


static void update_fps_display(
	GrafPtr port)
{
	if (displaying_fps && !player_in_terminal_mode(current_player_index))
	{
		long ticks= TickCount();
		GrafPtr old_port;
		char fps[sizeof("120.00fps")];
		
		frame_ticks[frame_index]= ticks;
		frame_index= (frame_index+1)%FRAME_SAMPLE_SIZE;
		if (frame_count<FRAME_SAMPLE_SIZE)
		{
			frame_count+= 1;
			strcpy(fps, "--");
		}
		else
		{
                    float framecount = (FRAME_SAMPLE_SIZE*60)/(float)(ticks-frame_ticks[frame_index]);
                    if (framecount >= TICKS_PER_SECOND)
			sprintf(fps,"%d%s",(int)TICKS_PER_SECOND,".00fps");
		    else
			sprintf(fps, "%3.2ffps", framecount);
		}

		GetPort(&old_port);
		SetPort(port);
		// Now using font-definition abstraction
		FontSpecifier& Font = GetOnScreenFont();
		Font.Use();
		// The line spacing is a generalization of "5" for larger fonts
		short Offset = Font.LineSpacing / 3;
		DisplayText(port->portRect.left+Offset,port->portRect.bottom-Offset,fps);
		RGBForeColor(&rgb_black);
		SetPort(old_port);
	}
	else
	{
		frame_count= frame_index= 0;
	}
}


static void DisplayPosition(GrafPtr port)
{
	if (!ShowPosition) return;
	
	// Push
	GrafPtr old_port;
	GetPort(&old_port);
	SetPort(port);
	
	FontSpecifier& Font = GetOnScreenFont();
	Font.Use();
	// TextSize(12);
	// TextFont(kFontIDMonaco);
	
	short LineSpacing = Font.LineSpacing;
	short X = port->portRect.left + LineSpacing/3;
	short Y = port->portRect.top + LineSpacing;
	const float FLOAT_WORLD_ONE = float(WORLD_ONE);
	const float AngleConvert = 360/float(FULL_CIRCLE);
	sprintf(temporary, "X       = %8.3f",world_view->origin.x/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Y       = %8.3f",world_view->origin.y/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Z       = %8.3f",world_view->origin.z/FLOAT_WORLD_ONE);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	sprintf(temporary, "Polygon = %8d",world_view->origin_polygon_index);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	short Angle = world_view->yaw;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Yaw     = %8.3f",AngleConvert*Angle);
	DisplayText(X,Y,temporary);
	Y += LineSpacing;
	Angle = world_view->pitch;
	if (Angle > HALF_CIRCLE) Angle -= FULL_CIRCLE;
	sprintf(temporary, "Pitch   = %8.3f",AngleConvert*Angle);
	DisplayText(X,Y,temporary);
	
	RGBForeColor(&rgb_black);
	
	// Pop
	SetPort(old_port);
}

static void DisplayMessages(GrafPtr port)
{	
	// Push
	GrafPtr old_port;
	GetPort(&old_port);
	SetPort(port);
	
	FontSpecifier& Font = GetOnScreenFont();
	Font.Use();
	// TextSize(12);
	// TextFont(kFontIDMonaco);
	
	short LineSpacing = Font.LineSpacing;
	short X = port->portRect.left + LineSpacing/3;
	short Y = port->portRect.top + LineSpacing;
	if (ShowPosition) Y += 6*LineSpacing;	// Make room for the position data
	for (int k=0; k<NumScreenMessages; k++)
	{
		int Which = (MostRecentMessage+NumScreenMessages-k) % NumScreenMessages;
		while (Which < 0)
			Which += NumScreenMessages;
		ScreenMessage& Message = Messages[Which];
		if (Message.TimeRemaining <= 0) continue;
		Message.TimeRemaining--;
		
		DisplayText(X,Y,Message.Text);
		Y += LineSpacing;
	}
	
	RGBForeColor(&rgb_black);
	
	// Pop
	SetPort(old_port);
}
#endif


// LP: has Steven Bytnar's changes to make this routine reentrant and otherwise better behaved.
void restore_world_device(
	void)
{
	GrafPtr old_port;
	
	static int entered = false;
	if (entered) return;
	entered = true;

	switch (screen_mode.acceleration)
	{
	}
	
	if (screen_window)
	{
		GrafPtr screen_port = (GrafPtr)GetWindowPort(screen_window);
		if (screen_port) {
			GetPort(&old_port);
			SetPort(screen_port);
			Rect portRect;
#if TARGET_CARBON
			GetPortBounds(GetWindowPort(screen_window), &portRect);
#else
			portRect = screen_window->portRect;
#endif
			PaintRect(&portRect);
			SetPort(old_port);
		}
	}
	myShowMenuBar();

	/* put our device back the way we found it */
	if (screen_initialized)
	{
		SetDepthGDSpec(&restore_spec);
	}
	
	// Use the Display Manager to set the size back
	
	unsigned long TempBitDepth = restore_spec.bit_depth;
	if (RestoreScreenMode != -1)
	{
		DMSetDisplayMode(world_device,RestoreScreenMode,&TempBitDepth,NULL,NULL);
		RestoreScreenMode = -1;
	}
	
	if (world_pixels)
	{
		myDisposeGWorld(world_pixels);
		world_pixels = NULL;
	}
	
#if !TARGET_CARBON
	if (screen_window)
	{
		CloseWindow(screen_window);
		screen_window = NULL;
	}
	if (backdrop_window)
	{
		CloseWindow(backdrop_window);
		backdrop_window = NULL;
	}
#else
	if (screen_window)
	{
		HideWindow(screen_window);
		DisposeWindow(screen_window);
		screen_window = NULL;
	}
	if (backdrop_window)
	{
		HideWindow(backdrop_window);
		DisposeWindow(backdrop_window);
		backdrop_window = NULL;
	}
#endif
}

// JTP: Bytnar's buffer flushing
void FlushGrafPortRect(const CGrafPtr port, const Rect &destination)
{
#if defined(TARGET_API_MAC_CARBON)
	if (QDIsPortBuffered(port)) {
		RgnHandle theRgn;
		theRgn = NewRgn();
		/* local port coordinates */
		SetRectRgn(theRgn, destination.left, destination.top, destination.right, destination.bottom);
		QDFlushPortBuffer(port, theRgn);
		DisposeRgn(theRgn);
	}
#endif
}

static void memcpy_blit_image(char * __restrict src, char * __restrict dst,const int width, int height,const int srcRowBytes,const int dstRowBytes,const int pelSize)
{
    while (height--) {
	memcpy(dst,src,width*pelSize);
	src += srcRowBytes;
	dst += dstRowBytes;
    }
}

// LP changes: moved sizing and resolution outside of this function,
// because they can be very variable
/* pixels are already locked, etc. */
static void update_screen(Rect& source, Rect& destination, bool hi_rez)
{
	if (hi_rez)
	{
#if defined(__APPLE__) && defined(__MACH__)
	    PixMapHandle screen_pixels = (*world_device)->gdPMap;
	    PixMapHandle world_pixelspm = GetGWorldPixMap(world_pixels);
	    Rect newDst, screenRect;
	    GetPortBounds(GetWindowPort(screen_window), &screenRect);
	    newDst.top = destination.top-screenRect.top;
	    newDst.left = destination.left- screenRect.left;
	    newDst.bottom = newDst.top + RECTANGLE_HEIGHT(&source);
	    newDst.right = newDst.left + RECTANGLE_WIDTH(&source);
	    short pelsize = bit_depth/8;
	    unsigned long sourceRB = GetPixRowBytes(world_pixelspm), dstRB = GetPixRowBytes(screen_pixels);
	    memcpy_blit_image(GetPixBaseAddr(world_pixelspm) + (source.left*pelsize) + (source.top * sourceRB),
					     GetPixBaseAddr(screen_pixels) + (newDst.left*pelsize) + (newDst.top * dstRB),
					     RECTANGLE_WIDTH(&source), RECTANGLE_HEIGHT(&source), sourceRB, dstRB, pelsize);
#else
		GrafPtr old_port;
		RGBColor old_forecolor, old_backcolor;
		
		GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
		SetPort(GetWindowPort(screen_window));
/*
#else
		SetPort(screen_window);
#endif
*/

		GetForeColor(&old_forecolor);
		GetBackColor(&old_backcolor);
		RGBForeColor(&rgb_black);
		RGBBackColor(&rgb_white);
		
//#if defined(USE_CARBON_ACCESSORS)
		Rect screenRect;
		GetPortBounds(GetWindowPort(screen_window), &screenRect);
		Rect adjustedRect;
		adjustedRect.top= destination.top- screenRect.top;
		adjustedRect.left= destination.left- screenRect.left;
		adjustedRect.bottom= adjustedRect.top + RECTANGLE_HEIGHT(&source);
		adjustedRect.right= adjustedRect.left + RECTANGLE_WIDTH(&source);
		
		PixMapHandle screen_pixmap= (*world_device)->gdPMap;
		LockPixels(screen_pixmap);
		CopyBits(GetPortBitMapForCopyBits(world_pixels), reinterpret_cast<BitMap*>(*screen_pixmap),
			&source, &adjustedRect, srcCopy, (RgnHandle) NULL);
		UnlockPixels(screen_pixmap);
/*
#else
		CopyBits((BitMapPtr)*world_pixels->portPixMap, &screen_window->portBits,
			&source, &destination, srcCopy, (RgnHandle) NULL);
#endif
*/
		
		RGBForeColor(&old_forecolor);
		RGBBackColor(&old_backcolor);
		SetPort(old_port);
#endif
	}
	else
	{
		int8 mode;
		short pelsize= bit_depth>>3;
		struct copy_screen_data data;
		short source_rowBytes, destination_rowBytes, source_width, destination_width;
		PixMapHandle screen_pixmap= (*world_device)->gdPMap;
		
		source_rowBytes= (*myGetGWorldPixMap(world_pixels))->rowBytes&0x3fff;
		destination_rowBytes= (*screen_pixmap)->rowBytes&0x3fff;
		source_width= RECTANGLE_WIDTH(&source);
		destination_width= RECTANGLE_WIDTH(&destination);
		
		data.source= (unsigned char *)myGetPixBaseAddr(world_pixels);
		data.source_slop= source_rowBytes-source_width*pelsize;
		
//#if defined(USE_CARBON_ACCESSORS)
		Rect screenRect;
		GetPortBounds(GetWindowPort(screen_window), &screenRect);
		data.destination= (unsigned char *)(*screen_pixmap)->baseAddr + (destination.top-screenRect.top)*destination_rowBytes +
			(destination.left-screenRect.left)*pelsize;
/*
#else
		data.destination= (unsigned char *)(*screen_pixmap)->baseAddr + (destination.top-screen_window->portRect.top)*destination_rowBytes +
			(destination.left-screen_window->portRect.left)*pelsize;
#endif
*/
		data.destination_slop= destination_rowBytes-destination_width*pelsize;

		data.bytes_per_row= destination_rowBytes;
		data.height= RECTANGLE_HEIGHT(&source);

		assert(!((long)data.source&3));
		assert(!((long)data.destination&3));
		assert(!(data.destination_slop&3));
		assert(!(data.source_slop&3));
		assert(!(data.bytes_per_row&3));

		data.flags= 0;
		data.destination_slop+= destination_rowBytes;
#ifdef env68k
		if (screen_mode.draw_every_other_line) data.flags|= _EVERY_OTHER_LINE_BIT;
#endif
		switch (bit_depth)
		{
			case 8:
				data.width= source_width>>2;
				break;
			case 16:
				data.width= source_width>>1;
				data.flags|= _SIXTEEN_BIT_BIT;
				break;
			case 32:
				data.width= source_width;
				data.flags|= _THIRTYTWO_BIT_BIT;
				break;
			default:
				// LP change:
				assert(false);
				// halt();
		}

		mode= true32b;
		SwapMMUMode(&mode);
		quadruple_screen(&data);
		SwapMMUMode(&mode);
	}
}

/* This function is NOT void because both the computer interface editor and vulcan use it */
/*  to determine the size of their gworlds.. */
void calculate_destination_frame(
	short size,
	bool high_resolution,
	Rect *frame)
{
	(void) (high_resolution);
	
	/* calculate destination frame */
	switch (size)
	{
		case _full_screen: /* ow */
			SetRect(frame, 0, 0, DESIRED_SCREEN_WIDTH, DESIRED_SCREEN_HEIGHT);
			break;
		case _100_percent:
			SetRect(frame, 0, 0, DEFAULT_WORLD_WIDTH, DEFAULT_WORLD_HEIGHT);
			break;
		case _75_percent:
			SetRect(frame, 0, 0, 3*DEFAULT_WORLD_WIDTH/4, 3*DEFAULT_WORLD_HEIGHT/4);
			break;
		case _50_percent:
			SetRect(frame, 0, 0, DEFAULT_WORLD_WIDTH/2, DEFAULT_WORLD_HEIGHT/2);
			break;
	}
	
	if (size!=_full_screen)
	{
		OffsetRect(frame, WORLD_H+((DEFAULT_WORLD_WIDTH-frame->right)>>1), 
			WORLD_V+((DEFAULT_WORLD_HEIGHT-frame->bottom)>>1));
	}
}

enum
{
	BITS_PER_CACHE_LINE= 6,
	CACHE_LINE_SIZE= (1<<BITS_PER_CACHE_LINE) // ppc601
};

static void calculate_screen_options(
	void)
{
	static bool screen_options_initialized= false;
	
	if(!screen_options_initialized)
	{
		enough_memory_for_16bit= (FreeMem()>FREE_MEMORY_FOR_16BIT) ? true : false;
		enough_memory_for_32bit= (FreeMem()>FREE_MEMORY_FOR_32BIT) ? true : false;
		enough_memory_for_full_screen= (FreeMem()>FREE_MEMORY_FOR_FULL_SCREEN) ? true : false;
		screen_options_initialized= true;
	}
}


#ifdef __MACH__
// Bytnar: On Mach-O, it's not worth it to hack quadruple.s to work.
#undef EXTERNAL
#endif


#ifndef EXTERNAL
typedef unsigned short word;

void quadruple_screen(
	struct copy_screen_data *data)
{
	register short bytes_per_row= data->bytes_per_row;
	register long *read= (long *) data->source;
	register long *write1= (long *) data->destination;
	register long *write2= (long *) ((byte *)data->destination + bytes_per_row);
	register unsigned long in, out1, out2;
	register short count;
	short rows_left;

	for (rows_left=data->height;rows_left>0;--rows_left)
	{
		if (data->flags&_THIRTYTWO_BIT_BIT)
		{
			for (count= data->width; count>0; --count)
			{
				in= *read++;
				*write1++= *write2++= in;
				*write1++= *write2++= in;
			}
		}
		else
		{
			if (data->flags&_SIXTEEN_BIT_BIT)
			{
				for (count=data->width;count>0;--count)
				{
					in= *read++;
					*write1++= *write2++= (in&0xffff0000) | (in>>16);
					*write1++= *write2++= (in&0x0000ffff) | (in<<16);
				}
			}
			else
			{
				if (data->flags&_EVERY_OTHER_LINE_BIT)
				{
					for (count=data->width;count>0;--count)
					{
						in= *read++;
						*write1++= ((unsigned long)(((word)(in&0xff)) | ((word)(in<<8)))) | (unsigned long)((((in>>8)&0xff) | (in&0xff00))<<16);
						in>>= 16;
						*write1++= ((unsigned long)(((word)(in&0xff)) | ((word)(in<<8)))) | (unsigned long)((((in>>8)&0xff) | (in&0xff00))<<16);
					}
				}
				else
				{
					for (count=data->width;count>0;--count)
					{
						in= *read++;
						out2= ((unsigned long)(((word)(in&0xff)) | ((word)(in<<8)))) | (unsigned long)((((in>>8)&0xff) | (in&0xff00))<<16);
						in>>= 16;
						out1= ((unsigned long)(((word)(in&0xff)) | ((word)(in<<8)))) | (unsigned long)((((in>>8)&0xff) | (in&0xff00))<<16);
						
						*write1++= out1, *write2++= out1;
						*write1++= out2, *write2++= out2;
					}
				}
			}
		}
		
		(byte*)read+= data->source_slop;
		(byte*)write1+= data->destination_slop;
		(byte*)write2+= data->destination_slop;
	}
}
#endif

#if SCREEN_BASED_SCREENSHOT
// JTP: createScreenPixMap taken from Apple's Sample code: SuperSnapshot
PixMapHandle createScreenPixMap()
{
	GDHandle	mainDevice;
	CTabHandle	cTable;
	short		depth;	
	Ptr		offBaseAddr;	/* Pointer to the off-screen pixel image */
	short		bytesPerRow;
	PixMapHandle	pixHandle = nil;
	BitMap		bitMap;	// temp store for screenbits
	Rect		gBounds;
	
	pixHandle = NewPixMap();
	
	/* Get a handle to the main device. */
	mainDevice = GetMainDevice();

	/* Store its current pixel depth. */
	depth = (**(**mainDevice).gdPMap).pixelSize;

	/* Make an identical copy of its pixmap's colortable. */
	cTable = (**(**mainDevice).gdPMap).pmTable;
	(void) HandToHand( (Handle*)&cTable );
	
	// Get Resolution of screen
//#if defined(USE_CARBON_ACCESSORS)
	GetQDGlobalsScreenBits(&bitMap);
/*
#else
	bitMap = qd.screenBits;
#endif
*/
	SetRect( &gBounds, 0, 0, bitMap.bounds.right, bitMap.bounds.bottom);
	
	// Fill in a few of the PixMap's fields...
	// NewPixMap() is good for default initialization, simply modify
	// the new PixMap
	(*pixHandle)->pmTable = cTable;
	(*pixHandle)->bounds = gBounds;
	(*pixHandle)->pixelSize = depth;
	bytesPerRow = ((gBounds.right - gBounds.left) * depth) / 8;
	offBaseAddr = NewPtr((unsigned long) bytesPerRow * (gBounds.bottom - gBounds.top));
	(*pixHandle)->baseAddr = offBaseAddr;  			// Point to image
	(*pixHandle)->rowBytes = bytesPerRow | 0x8000;	// MSB set for PixMap
	
	LockPixels(pixHandle);

	CopyBits( (BitMap *)*(**mainDevice).gdPMap, (BitMap *) *pixHandle,
				&(**(**mainDevice).gdPMap).bounds, &(*pixHandle)->bounds, srcCopy, 0l );
	
	UnlockPixels(pixHandle);
	
	return pixHandle;
}
#endif

// LP addition: routine for doing screendumps.
// Ought to return some error code.
void dump_screen()
{
#if SCREEN_BASED_SCREENSHOT
	PixMapHandle screenshotHandle = createScreenPixMap();
#endif

	// Push the old graphics context
	GrafPtr old_port;
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
/*
#else
	SetPort(screen_window);
#endif
*/
	
	RGBColor old_forecolor, old_backcolor;
	GetForeColor(&old_forecolor);
	GetBackColor(&old_backcolor);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);

	// Find out how the area that gets copied;
	// this is intended to be done as the game runs (including terminal mode)
	Rect DumpRect;
#if SCREEN_BASED_SCREENSHOT
	Rect SourceRect;
#endif

	short msize = screen_mode.size;
	assert(msize >= 0 && msize < NUMBER_OF_VIEW_SIZES);
	const ViewSizeData& VS = ViewSizes[msize];
	
	// Rectangle where the view is to go (must not overlap the HUD)
	short OverallWidth = VS.OverallWidth;
	short OverallHeight = VS.OverallHeight;
	
	// Offsets for placement in the screen
//#if defined(USE_CARBON_ACCESSORS)
	Rect ScreenRect;
	GetPortBounds(GetWindowPort(screen_window), &ScreenRect);
/*
#else
	Rect ScreenRect = screen_window->portRect;
#endif
*/
	short ScreenOffsetWidth = ((RECTANGLE_WIDTH(&ScreenRect) - VS.OverallWidth) >> 1) + ScreenRect.left;
	short ScreenOffsetHeight = ((RECTANGLE_HEIGHT(&ScreenRect) - VS.OverallHeight) >> 1) + ScreenRect.top;
	
	SetRect(&DumpRect, 0, 0, OverallWidth, OverallHeight);
	OffsetRect(&DumpRect, ScreenOffsetWidth, ScreenOffsetHeight);
	
	// Just in case it's too big...
	DumpRect.left = MAX(DumpRect.left,ScreenRect.left);
	DumpRect.top = MAX(DumpRect.top,ScreenRect.top);
	DumpRect.right = MIN(DumpRect.right,ScreenRect.right);
	DumpRect.bottom = MIN(DumpRect.bottom,ScreenRect.bottom);
	
#if SCREEN_BASED_SCREENSHOT
	// Make sure the SourceRect is appropriate
	SourceRect = DumpRect;
	OffsetRect(&SourceRect, 0 - ScreenRect.left, 0 - ScreenRect.top);
	// OffsetRect(&SourceRect, 0 - SourceRect.left, 0 - SourceRect.top);
#endif

	// Now create a picture
	OpenCPicParams PicParams;
	PicParams.srcRect = DumpRect;
	PicParams.hRes = 0x00480000; // 72 dpi
	PicParams.vRes = 0x00480000; // 72 dpi
	PicParams.version = -2;
	PicParams.reserved1 = 0;
	PicParams.reserved2 = 0;
	PicHandle PicObject = OpenCPicture(&PicParams);

#if SCREEN_BASED_SCREENSHOT
	LockPixels(screenshotHandle);
	
//#if defined(USE_CARBON_ACCESSORS)
	CopyBits((BitMap *) *screenshotHandle, GetPortBitMapForCopyBits(GetWindowPort(screen_window)),
		&SourceRect, &DumpRect, srcCopy, (RgnHandle) NULL);
/*
#else
	CopyBits((BitMap *) *screenshotHandle, &screen_window->portBits,
		&SourceRect, &DumpRect, srcCopy, (RgnHandle) NULL);
#endif
*/

	UnlockPixels(screenshotHandle);
	DisposePixMap(screenshotHandle);
#else	
	ClipRect(&DumpRect);
//#if defined(USE_CARBON_ACCESSORS)
	CopyBits(GetPortBitMapForCopyBits(GetWindowPort(screen_window)), GetPortBitMapForCopyBits(GetWindowPort(screen_window)),
		&DumpRect, &DumpRect, srcCopy, (RgnHandle) NULL);
/*
#else
	CopyBits(&screen_window->portBits, &screen_window->portBits,
		&DumpRect, &DumpRect, srcCopy, (RgnHandle) NULL);
#endif
*/
#endif
	ClosePicture();

	// Pop back the old graphics context
	RGBForeColor(&old_forecolor);
	RGBBackColor(&old_backcolor);
	SetPort(old_port);

	// Decide on the file's name
	FSSpec Spec;
	// Get the app's FSSpec using cseries function
	get_my_fsspec(&Spec);
	
	// Now write out that picture
	short RefNum;
	OSErr err;
	
	int Indx = 1;
	while(true) {
		// Compose a suitable name
		psprintf(Spec.name,"Screenshot %04d",Indx++);
		// Check for the pre-existence of a file with that name
		// by trying to open it
		err = FSpOpenDF(&Spec, fsRdPerm, &RefNum);
		if (err == noErr)
		{
			FSClose(RefNum);
			continue;
		}
		// Now try to create it
#if defined(TARGET_API_MAC_CARBON)
		err = FSpCreate(&Spec,0l,'PICT',smSystemScript);
#else
		err = FSpCreate(&Spec,'ttxt','PICT',smSystemScript);
#endif
		if (err != noErr)
		{
			KillPicture(PicObject);
			return;
		}
		else break;
	}
	
	err = FSpOpenDF(&Spec, fsRdWrPerm, &RefNum);
	if (err != noErr) {
		KillPicture(PicObject);
		return;
	}
	
	// Write the header:
	long Zero = 0;
	long len;
	for (unsigned int z=0; z<512/sizeof(Zero); z++)
	{
		len = sizeof(Zero);
		err = FSWrite(RefNum,&len,&Zero);
		if (err != noErr) {
			FSClose(RefNum);
			KillPicture(PicObject);
			return;
		}
	}
	
	// Make that "snapshot" sound;
	// the resource number is from the "System" file
	Handle SoundHandle = GetResource('snd ',-16504);
	if (SoundHandle != NULL)
	{
		// Cribbed from Apple's QT Sound Manager docs
		HLock(SoundHandle);
		SndPlay(NULL, (SndListResource **)SoundHandle, true);
		HUnlock(SoundHandle);
		ReleaseResource(SoundHandle);
	}
	
	// Write the body:
	long PicLen = GetHandleSize(Handle(PicObject));
	len = PicLen;
	HLock(Handle(PicObject));
	err = FSWrite(RefNum,&len,*PicObject);
	HUnlock(Handle(PicObject));
		
	// All done!
	FSClose(RefNum);
	KillPicture(PicObject);
}


// Actually do the drawing, in software mode
void DrawHUD(Rect& SourceRect, Rect& DestRect)
{
	// Check if the HUD buffer is there
	assert(HUD_Buffer);
	
	// Code cribbed from draw_panels() in game_window_macintosh.c
	GrafPtr old_port;
	RGBColor old_forecolor, old_backcolor;
	
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
/*
#else
	SetPort(screen_window);
#endif
*/
	GetForeColor(&old_forecolor);
	GetBackColor(&old_backcolor);
	RGBForeColor(&rgb_black);
	RGBBackColor(&rgb_white);
		
	/* Slam it to the screen. */
//#if defined(USE_CARBON_ACCESSORS)
	CopyBits(GetPortBitMapForCopyBits(HUD_Buffer), GetPortBitMapForCopyBits(GetWindowPort(screen_window)),
		&SourceRect, &DestRect, srcCopy, (RgnHandle) NULL);
	/* flush part of the port */
	FlushGrafPortRect(GetWindowPort(screen_window), DestRect);
/*
#else
	CopyBits((BitMapPtr)*HUD_Buffer->portPixMap, &screen_window->portBits, //(BitMapPtr)*screen_pixmap,
		&SourceRect, &DestRect, srcCopy, (RgnHandle) NULL);	
#endif
*/
	RGBForeColor(&old_forecolor);
	RGBBackColor(&old_backcolor);
	SetPort(old_port);
}


// LP: cribbed from change_screen_mode
void ClearScreen()
{
	GrafPtr old_port;
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(screen_window));
	Rect portRect;
	GetPortBounds(GetWindowPort(screen_window), &portRect);
	PaintRect(&portRect);
/*
#else
	SetPort(screen_window);
	PaintRect(&screen_window->portRect);
#endif
*/
	SetPort(old_port);
	
	// To be extra safe, for MacOS X Classic compatibility
	OGL_ClearScreen();
}

// For switching to another process and returning (suspend/resume events)

void SuspendDisplay(EventRecord *EvPtr)
{
	// Switch resolution only if necessary
	if (restore_spec.bit_depth != graphics_preferences->device_spec.bit_depth)
		SetDepthGDSpec(&restore_spec);
	
	HideWindow(screen_window);
	HideWindow(backdrop_window);
#if !defined(TARGET_API_MAC_CARBON)
	SetCursor(&qd.arrow);
	myShowMenuBar();
	show_cursor();
#endif
}

void ResumeDisplay(EventRecord *EvPtr)
{
	// The resolution may have changed when the app was switched out
	BuildGDSpec(&restore_spec, world_device);
	if (restore_spec.bit_depth != graphics_preferences->device_spec.bit_depth)
		SetDepthGDSpec(&graphics_preferences->device_spec);
	
#if !defined(TARGET_API_MAC_CARBON)
	SetCursor(&qd.arrow);
	myHideMenuBar(GetMainDevice());
#endif
	ShowWindow(backdrop_window);
	SelectWindow(screen_window);
	ShowWindow(screen_window);
}

// Handling the resolution with the Display Manager

// Checks on whether the Display Manager is present
bool DM_Check()
{
	// Need to check only once
	if (DM_Inited) return DM_Present;
	DM_Inited = true;
	
	// Check by doing weak linking...
	DM_Present = ((Ptr)DMGetDisplayIDByGDevice != (Ptr)kUnresolvedCFragSymbolAddress);

	return DM_Present;
}


// For debugging convenience;
// returns 1 for success, 0 for failure
static bool DM_HandleError(char *Description, OSErr ErrorCode)
{
	if (ErrorCode != noErr) dprintf("DM: %s Error: %d",Description,ErrorCode);
	return (ErrorCode == noErr);
}


// Classes for auto-dispose of various objects

// When constructed, begins a DM display-configure session;
// when destroyed, ends it.
// LP: I got this trick from a Be Developer online newsletter;
// one issue had discussed an auto-locker C++ class
// that locks when created and unlocks when destroyed
struct DM_Session
{
	Handle State;	// The display state
	OSErr Err;		// Have to define this here, because constructors don't return anything
	
	DM_Session() {Err = DMBeginConfigureDisplays(&State);}
	~DM_Session() {Err = DMEndConfigureDisplays(State);}
};

// When constructed, creates a DM mode list;
// when destroyed, destroys it
struct DM_ModeList
{
	DMListType List;			// The actual list
	DMListIndexType Count;		// How many in it
	DMDisplayModeListIteratorUPP ModeInfoCallbackUPP;	// For obtaining the mode
	OSErr Err;		// Have to define this here, because constructors don't return anything
	
	// Returned pointer to mode-info record (NULL if unset)
	DMDisplayModeListEntryPtr ModeInfoPtr;
	
	// Get a mode with some index value
	void GetMode(DMListIndexType Index)
	{
		// This object will get the mode-info pointer
		Err = DMGetIndexedDisplayModeFromList(List, Index, NULL, ModeInfoCallbackUPP, this);
		if (Err != noErr) ModeInfoPtr = NULL;
	}
	
	// Needs the ID of the display to look at
	DM_ModeList(DisplayIDType DisplayID)
	{
		// Idiot-proofing
		ModeInfoPtr = NULL;
		List = NULL;
		ModeInfoCallbackUPP = NewDMDisplayModeListIteratorUPP(DM_ModeInfoCallback);
		Err = DMNewDisplayModeList(DisplayID, NULL, NULL, &Count, &List);
	}
	
	~DM_ModeList()
	{
		DisposeDMDisplayModeListIteratorUPP(ModeInfoCallbackUPP);
		if (List) DMDisposeList(List);
	}
};


// Mode data:
struct DM_ModeData
{
	Str255 Name;
	int Index;
	unsigned long csData;
};


// List of modes with some dimension
class DM_ModeDimList
{
	vector<DM_ModeData> MDList;	// What indices share these dimensions
	short Width, Height;	// What dimensions?

public:
	bool AddMD(short _Width, short _Height, DM_ModeData& ModeData);
	void SetDims(short _Width, short _Height);
	
	short GetWidth() {return Width;}
	short GetHeight() {return Height;}
	
	int NumMDs() {return MDList.size();}
	DM_ModeData &WhichMD(int I) {return MDList[I];}
	
	DM_ModeDimList() : Width(0), Height(0) {MDList.reserve(16);}
};

bool DM_ModeDimList::AddMD(short _Width, short _Height, DM_ModeData& ModeData)
{
	if (Width != _Width) return false;
	if (Height != _Height) return false;
	
	MDList.push_back(ModeData);
	return true;
}

void DM_ModeDimList::SetDims(short _Width, short _Height)
{
	Width = _Width;
	Height = _Height;
	
	MDList.clear();
}


#ifdef USES_NIBS
static bool BuildFreqMenuItem(int Indx, Str255 ItemName, bool &IsInitial, void *Data)
{
	DM_ModeDimList *MDPtr = (DM_ModeDimList *)(Data);

	if (Indx >= MDPtr->NumMDs()) return false;
	
	pstrcpy(ItemName, MDPtr->WhichMD(Indx).Name);
	
	return true;
}


struct FreqDialogHandlerData
{
	GDHandle Device;
	short BitDepth;
	DM_ModeDimList *MDPtr;
	DM_Session *SessPtr;
	
	EventLoopTimerRef Timer;
	ControlRef FreqPopup;
	bool FreqChanged;
	int PreviousSelection;
};


static pascal void FreqDialogTimer(EventLoopTimerRef Timer, void *Data)
{
	FreqDialogHandlerData *HDPtr = (FreqDialogHandlerData *)(Data);
	
	if (HDPtr->FreqChanged)
	{
		unsigned long TempBitDepth = HDPtr->BitDepth;
		int Selection = HDPtr->PreviousSelection;
		SetControl32BitValue(HDPtr->FreqPopup, Selection+1);
		DMSetDisplayMode(HDPtr->Device,HDPtr->MDPtr->WhichMD(Selection).csData,&TempBitDepth,NULL,HDPtr->SessPtr->State);
		HDPtr->FreqChanged = false;
	}
}


const EventTimerInterval TimeoutTime = 7;

static void FreqDialogHandler(ParsedControl &Ctrl, void *Data)
{
	FreqDialogHandlerData *HDPtr = (FreqDialogHandlerData *)(Data);
	int Selection;
	
	switch(Ctrl.ID.id)
	{
	case MonitorFreq_Accept:
		HDPtr->FreqChanged = false;
		Selection = GetControl32BitValue(HDPtr->FreqPopup) - 1;
		HDPtr->PreviousSelection = Selection;
		break;
		
	case MonitorFreq_Popup:
		unsigned long TempBitDepth = HDPtr->BitDepth;
		Selection = GetControl32BitValue(HDPtr->FreqPopup) - 1;
		DMSetDisplayMode(HDPtr->Device,HDPtr->MDPtr->WhichMD(Selection).csData,&TempBitDepth,NULL,HDPtr->SessPtr->State);
		HDPtr->FreqChanged = true;
		SetEventLoopTimerNextFireTime(HDPtr->Timer, TimeoutTime);
	}
}
#endif // USES_NIBS


// Changes the monitor's resolution and creates a DSp context for the new resolution
// and deletes the old one if necessary; returns the success of doing so
bool DM_ChangeResolution(GDHandle Device, short BitDepth, short Width, short Height,
	bool ShowFreqDialog)
{
	if (!DM_Check()) return false;
	
	// Tell the Display Manager not to update the displays while changing resolution;
	// they won't get updated as long as this object is in scope.
	DM_Session Session;
	
	// Get which display ID
	DisplayIDType DisplayID;
	OSErr Err = DMGetDisplayIDByGDevice(Device, &DisplayID, true);
	if (!DM_HandleError("CR: Find Display ID",Err)) return false;
	
	DM_ModeList ModeList(DisplayID);
	if (!DM_HandleError("CR: Creating mode-info list",ModeList.Err)) return false;
	
	// Search: find biggest mode and best mode with desired bit depth
	DM_ModeDimList Biggest, Best;
	bool PassedBest = false;
	
	for (DMListIndexType Index=0; Index<ModeList.Count; Index++)
	{
		ModeList.GetMode(Index);
		
		// The mode name is a Pascal string pointed to by ModeList.ModeInfoPtr->displayModeName
		// Need the depth info to find which
		DMDepthInfoBlockPtr DepthInfoPtr = ModeList.ModeInfoPtr->displayModeDepthBlockInfo;
		
		// Is there a submode with a bit depth that matches the desired bit depth?
		for (unsigned int id=0; id<DepthInfoPtr->depthBlockCount; id++)
		{
			DMDepthInfoPtr IndivDepthPtr = DepthInfoPtr->depthVPBlock + id;
			VPBlockPtr IDBlockPtr = IndivDepthPtr->depthVPBlock;
			if (IDBlockPtr->vpPixelSize != BitDepth) continue;
			
			// One was found...
			Rect& Bounds = IDBlockPtr->vpBounds;
			short ModeWidth = Bounds.right-Bounds.left;
			short ModeHeight = Bounds.bottom-Bounds.top;
			
			DM_ModeData ModeData;
			memcpy(ModeData.Name,ModeList.ModeInfoPtr->displayModeName,256);
			ModeData.Index = Index;
			ModeData.csData = IndivDepthPtr->depthSwitchInfo->csData;
			
			if (ModeWidth > Biggest.GetWidth() || ModeHeight > Biggest.GetHeight())
				Biggest.SetDims(ModeWidth,ModeHeight);
			
			Biggest.AddMD(ModeWidth,ModeHeight,ModeData);
			
			if (ModeWidth >= Width && ModeHeight >= Height && !PassedBest)
			{
				Best.SetDims(ModeWidth,ModeHeight);
				PassedBest = true;
			}
			
			Best.AddMD(ModeWidth,ModeHeight,ModeData);
			break;
		}
	}
	
	// Find which list of indices to use
	DM_ModeDimList *MDPtr = (Best.NumMDs() > 0) ? &Best :
		((Biggest.NumMDs() > 0) ? &Biggest : NULL);
	if (!MDPtr) return false;
	
	// Find the closest of the monitor frequences to the prefs frequency:
	float PrevFreq = graphics_preferences->refresh_frequency;
	float SmallestDiff = PrevFreq;
	int BestFitIndex = 0;
	
	for (int im=0; im<MDPtr->NumMDs(); im++)
	{
		float Freq = GetFreqFromName(MDPtr->WhichMD(im).Name);
		float FreqDiff = Freq - PrevFreq;
		float AbsFreqDiff = ABS(FreqDiff);
		if (AbsFreqDiff < SmallestDiff)
		{
			BestFitIndex = im;
			SmallestDiff = AbsFreqDiff;
		}
	}
	
	{       // if we're already at the requested resolution, we can exit right away
	  VDSwitchInfoRec current_state;
	  Err= DMGetDisplayMode(Device, &current_state);
	  if (Err == noErr && current_state.csData == MDPtr->WhichMD(BestFitIndex).csData)
	    return TRUE;
	}
	
	// Set monitor resolution and frequency automatically if requested;
	// don't use the dialog box for that.
	if (!ShowFreqDialog)
	{
		unsigned long TempBitDepth = BitDepth;
		Err = DMSetDisplayMode(Device,MDPtr->WhichMD(BestFitIndex).csData,&TempBitDepth,NULL,Session.State);
		bool ChangeSuccess = (Err == noErr);
		if (ChangeSuccess)
			graphics_preferences->refresh_frequency = 
				GetFreqFromName(MDPtr->WhichMD(BestFitIndex).Name);
		return ChangeSuccess;
	}
	
#ifdef USES_NIBS
	OSStatus err;
	
	// Get the window
	AutoNibWindow Window(GUI_Nib,Window_Prefs_MonitorFreq);

	ControlRef FreqPopup = GetCtrlFromWindow(Window(), 0, MonitorFreq_Popup);

	BuildMenu(FreqPopup, BuildFreqMenuItem, MDPtr);
	
	int Selection = BestFitIndex;
	int OriginalSelection = Selection;
	SetControl32BitValue(FreqPopup, Selection+1);
	
	FreqDialogHandlerData HandlerData;
	HandlerData.FreqChanged = false;
	
	// Originally off	
	AutoTimer Timer(0, 0, FreqDialogTimer, &HandlerData);
	
	HandlerData.Device = Device;
	HandlerData.BitDepth = BitDepth;
	HandlerData.MDPtr = MDPtr;
	HandlerData.SessPtr = &Session;

	HandlerData.Timer = Timer();
	HandlerData.FreqPopup = FreqPopup;
	HandlerData.FreqChanged = false;
	HandlerData.PreviousSelection = Selection;
	
	bool ChangeSuccess = RunModalDialog(Window(), false, FreqDialogHandler, &HandlerData);
	
	if (ChangeSuccess)
	{
		// Remember the frequency setting for next time
		Selection = GetControl32BitValue(FreqPopup) - 1;
		graphics_preferences->refresh_frequency = 
			GetFreqFromName(MDPtr->WhichMD(Selection).Name);
	}
	else
	{
		// Revert!
		unsigned long TempBitDepth = BitDepth;
		Selection = OriginalSelection;
		DMSetDisplayMode(Device,MDPtr->WhichMD(Selection).csData,&TempBitDepth,NULL,Session.State);
	}
	
#else
	
	// Build MacOS Classic dialog box
	// and its keyboard-event catcher
	DialogPtr Dialog = GetNewDialog(MonitorFreq_Dialog, nil, WindowPtr(-1));
	assert(Dialog);
//#if defined(TARGET_API_MAC_CARBON)
	ModalFilterUPP DialogEventHandler = NewModalFilterUPP(DM_ModeFreqDialogHandler);
/*
#else
	ModalFilterUPP DialogEventHandler = NewModalFilterProc(DM_ModeFreqDialogHandler);
#endif
*/
	assert(DialogEventHandler);
	
	// Get its popup-menu handle
	ControlHandle PopupHandle;
	short ItemType;
	Rect Bounds;
	GetDialogItem(Dialog, MonitorFreq_Popup, &ItemType, 
		(Handle *) &PopupHandle, &Bounds);
	assert(PopupHandle);
//#if defined(USE_CARBON_ACCESSORS)
	MenuHandle PopupMenuHandle = GetControlPopupMenuHandle(PopupHandle);
/*
#else
	PopupPrivateData **PopupDataHandle = (PopupPrivateData **) ((*PopupHandle)->contrlData);
	MenuHandle PopupMenuHandle = (*PopupDataHandle)->mHandle;
#endif
*/
	
	// Add the mode names
#if defined(TARGET_API_MAC_CARBON)
	int NumMenuItems = CountMenuItems(PopupMenuHandle);
#else
	int NumMenuItems = CountMItems(PopupMenuHandle);
#endif
	for (int im=0; im<NumMenuItems; im++)
		DeleteMenuItem(PopupMenuHandle,1);
	
	for (int im=0; im<MDPtr->NumMDs(); im++)
	{
		InsertMenuItem(PopupMenuHandle,"\pBlam",im);
		SetMenuItemText(PopupMenuHandle, (im+1), MDPtr->WhichMD(im).Name);
	}
	
	// Default: from the previous monitor-frequency value found earlier
	int DefSelected = BestFitIndex;
	int Selected = DefSelected;
	int PrevSelected = Selected;
	SetControlValue(PopupHandle, Selected+1);
	
	// Need to make the change the first time around
	bool ChangeRequested = true;
	bool QuitRequested = false;
	bool ChangeSuccess = false;
	
	// Read that dialog box and set the mode as appropriate
//#if defined(USE_CARBON_ACCESSORS)
	SelectWindow(GetDialogWindow(Dialog));
	ShowWindow(GetDialogWindow(Dialog));
/*
#else
	SelectWindow(Dialog);
	ShowWindow(Dialog);
#endif
*/
	while(true)
	{
		// Putting monitor-resolution changing up here so it only has to be present once
		if (ChangeRequested)
		{
			// Switch!
			unsigned long TempBitDepth = BitDepth;
			Err = DMSetDisplayMode(Device,MDPtr->WhichMD(Selected).csData,&TempBitDepth,NULL,Session.State);
			ChangeSuccess = (Err == noErr);
			
			// No more changes to be done unless explicitly requested
			ChangeRequested = false;
		}
		
		// Quit only after making some requested change, like reversion
		if (QuitRequested) break;
		
		short ItemHit;
		ModalDialog(DialogEventHandler, &ItemHit);
		
		// Here, ChangeRequested and QuitRequested are false unless explicitly made true
		switch(ItemHit)
		{
		case MonitorFreq_Revert:
			Selected = PrevSelected;
			SetControlValue(PopupHandle, Selected+1);
			ChangeRequested = true;
			break;
			
		case MonitorFreq_Def:
			Selected = DefSelected;
			SetControlValue(PopupHandle, Selected+1);
			ChangeRequested = true;
			QuitRequested = true;
			break;
			
		case MonitorFreq_Set:
			QuitRequested = true;
			break;
			
		case MonitorFreq_Popup:
			PrevSelected = Selected;
			Selected = GetControlValue(PopupHandle) - 1;
			ChangeRequested = true;
			break;
		}
	}
	
	// Clean up
//#if defined(USE_CARBON_ACCESSORS)
	HideWindow(GetDialogWindow(Dialog));
/*
#else
	HideWindow(Dialog);
#endif
*/
	DisposeDialog(Dialog);
	DisposeModalFilterUPP(DialogEventHandler);
	
	// Remember the frequency setting for next time
	if (ChangeSuccess)
		graphics_preferences->refresh_frequency = 
			GetFreqFromName(MDPtr->WhichMD(Selected).Name);
#endif
	
	return ChangeSuccess;
}


// Puts record into global
pascal void DM_ModeInfoCallback(void *UserData,
	DMListIndexType Index, DMDisplayModeListEntryPtr ModeInfoPtr)
{
	// Ensures that my mode-list object will get the mode-info pointer
	assert(UserData);
	DM_ModeList *ModeListPtr = (DM_ModeList *)UserData;
	ModeListPtr->ModeInfoPtr = ModeInfoPtr;
}


#ifndef USES_NIBS

// Callback for monitor-frequency-selection dialog-box event handling;
// it is for catching keyboard events
pascal Boolean DM_ModeFreqDialogHandler(DialogPtr Dialog,
	EventRecord *Event, short *ItemHit)
{
	switch(Event->what)
	{
		// Only interested in keyboard events
		case keyDown:
		case autoKey:
		{
			short Key = Event->message & charCodeMask;
			switch(Key)
			{
			// Return is revert to previous resolution
			case 0x0d:
				*ItemHit = MonitorFreq_Revert;
				return true;
				break;
			
			// Escape is quit in default resolution
			case 0x1b:
				*ItemHit = MonitorFreq_Def;
				return true;
				break;
			}
		}
		break;
	}
	
	// Ignore all other kinds of events
	return false;
}

#endif


// Parsing of mode name to get frequency:
static float GetFreqFromName(Str255 Name)
{
	// Failure value: zero frequency
	float Freq = 0;

	// Look for text "Hz" (many not work for some non-Roman-localized text)
	int HZLoc = NONE;
	for (int k=1; k<=Name[0]; k++)
	{
		if (Name[k] == 'H')
		{
			HZLoc = k;
			break;
		}
	}
	// Just in case:
	if (HZLoc == NONE) return Freq;
	
	// Move backwards to first non-whitespace
	for (int k=HZLoc-1; k>1; k--)
	{
		if (Name[k] != ' ')
		{
			HZLoc = k + 1;
			break;
		}
	}
	
	// Look for number beginning
	int NumLoc = NONE;
	for (int k=HZLoc-1; k>1; k--)
	{
		if (Name[k] == ' ')
		{
			NumLoc = k + 1;
			break;
		}
	}
	// Just in case:
	if (NumLoc == NONE) return Freq;
	
	// Extract and make a C string
	int NumLen = HZLoc - NumLoc;
	memcpy(temporary,Name+NumLoc,NumLen);
	temporary[NumLen] = 0;
	
	sscanf(temporary,"%f",&Freq);
	
	return Freq;
}

#ifdef TARGET_API_MAC_CARBON
#ifndef __MACH__

// Loads the symbol for calling CGSetDisplayTransferByTable()
static void LoadCoreGraphicsFader()
{
	// To avoid repeating it unnecessarily
	static bool AlreadyCalled = false;
	
	if (!AlreadyCalled)
	{
		CGSetDisplayTransferByTable_Ptr = (CGSetDisplayTransferByTable_Type)
			GetSystemFunctionPointer(CFSTR("CGSetDisplayTransferByTable"));
	
		AlreadyCalled = true;
	}
}



#endif


// Directly manipulate the video-driver color table.
// Written so that the faders will work in MacOS X Carbon.
void animate_screen_clut_osx(
	struct color_table *color_table,
	GDHandle DevHdl)
	{
		(void) DevHdl; // This might come in handy some day
		
		CGGammaValue redTable[ 256 ];
		CGGammaValue greenTable[ 256 ];
		CGGammaValue blueTable[ 256 ];
		
		short i;
		
		// Break out the colors
		for(i = 0; i < color_table->color_count; i++)
		{
			redTable[i] = ((float)color_table->colors[i].red ) / (65535);
			greenTable[i] = ((float)color_table->colors[i].green ) / (65535);
			blueTable[i] = ((float)color_table->colors[i].blue ) / (65535);
		}

#ifdef __MACH__		
		CGSetDisplayTransferByTable( 0, color_table->color_count, redTable, greenTable, blueTable);
#else
		if (CGSetDisplayTransferByTable_Ptr)
			CGSetDisplayTransferByTable_Ptr(0, color_table->color_count, redTable, greenTable, blueTable);
#endif
	}
#endif

#if USES_NIBS
static pascal OSStatus CEvtWindowUpdateEvents (EventHandlerCallRef nextHandler,
    EventRef theEvent,
    void* userData)
{
	OSStatus err = eventNotHandledErr;

	if (nextHandler)
		err = CallNextEventHandler (nextHandler, theEvent);

	if (err == noErr || err == eventNotHandledErr)
	{
		EventRecord eventRec;
		ConvertEventRefToEventRecord(theEvent, &eventRec);
		update_game_window((WindowPtr)eventRec.message, &eventRec);
		err= noErr;
	}
	return err;
}
#endif
