/*
SHELL_MACINTOSH.C

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

Tuesday, June 19, 1990 7:20:13 PM

Tuesday, July 17, 1990 12:26:24 AM
	Minotaur modifications.
Tuesday, November 26, 1991 11:29:11 PM
	This shit needs to be totally rewritten.
Tuesday, December 17, 1991 11:19:09 AM
	(strongly favoring tuesdays) ... we now work under single finder, hopefully, and this shit
	is being totally rewritten.
Tuesday, November 17, 1992 3:32:11 PM
	changes for Pathways into Darkness.  this shell will compile into editor_shell.c.o and
	game_shell.c.o based on whether GAME or EDITOR is defined in the preprocessor.
Saturday, December 12, 1992 9:41:53 AM
	we now pass through some command keys, when not handled by the menus.
Friday, April 23, 1993 7:44:35 AM
	added ai timing voodoo for mouse-in-world and closing immediately after an explicit save.
Wednesday, June 2, 1993 9:17:08 AM
	lots of changes for new first-event dialogs.  happy birthday, yesterday.  talked to mara for
	half an hour in front of brek.
Friday, July 30, 1993 6:49:48 PM
	the day before the big build; final modifications to copy-protection stuff.
Saturday, August 21, 1993 4:31:50 PM
	hacks for OZONE.
Monday, August 30, 1993 3:18:37 PM
	menu bar?, what menu bar?
Wednesday, December 8, 1993 1:02:36 PM
	this is now MARATHON.
Wednesday, June 7, 1995 10:50:05 AM
	this is now MARATHON II.
Thursday, September 28, 1995 7:37:12 PM
	reintegrated interface.c/shell.c.  Tried to keep this clean, but I didn't succeed as
	well as I would have liked.

Jan 30, 2000 (Loren Petrich)
	Removed some "static" declarations that conflict with "extern"
	Changed "virtual" to "_virtual" to make data structures more C++-friendly

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 12, 2000 (Loren Petrich):
	Suppressed debug_print_weapon_status() as unnecessary to do.

Feb 13, 2000 (Loren Petrich):
	Added screen-dumping capability: dump_screen()
	Assigned it to both F13 and "escape".
	
Feb 15, 2000 (Loren Petrich):
	Changed the screenshot keys to F13 and F8

Feb 21, 2000 (Loren Petrich):
	Changed the chase-cam key to F6,
	and added crosshair support (F7)

Apr 15, 2000 (Loren Petrich):
	Added TEXT-resource XML-file loading

May 31, 2000 (Loren Petrich):
	Added texture resetting as key F14

Jun 15, 2000 (Loren Petrich):
	Re-enabled screen-resolution changing with OpenGL

Jul 4, 2000 (Loren Petrich):
	Implemented some of the unimplemented cheat codes
	Also allowed more than the cheat amounts to be kept

Jul 5, 2000 (Loren Petrich):
	Changed the resolution-change keys to F1 (decrease) and F2 (increase)
	Also changed F3 to toggle pixel doubling
	Set F4 to reset OpenGL textures, since some people don't have F14 available
	Removed the '~' key from that meaning

Jul 7, 2000 (Loren Petrich):
	Added Ben Thompson's InputSprocket support

Jul 29, 2000 (Loren Petrich):
	Added local-event flag set, for handling quitting, resolution changes, etc.
	with the InputSprocket.

Jul 30, 2000 (Loren Petrich):
	Added posting of MacOS events from the local event queue

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler

Oct 12, 2000 (Loren Petrich):
	Quicktime initialization now here

Dec 1, 2000 (Loren Petrich):
	Added scheme for reading STR# resource 128 to find directories to read MML script files from;
	both 'TEXT' files and 'rsrc' archives can be read.

Dec 2, 2000 (Loren Petrich):
	Added more reasonable suspend-and-resume, so as to hide and show the app properly
	
Dec 31, 2000 (Mike Benonis):
	Changed Dialog Header drawing code so it will not draw the Preferences header in the
	preferences dialog.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Quicktime.h
	Added accessors for datafields now opaque in Carbon
	Added startup checks and initilizations appropriate for Carbon

Jan 29, 2002 (Br'fin (Jeremy Parsons)):
	Added a RunCurrentEventLoop to let our Carbon input timer run

Feb 4, 2002 (Br'fin (Jeremy Parsons)):
	Moved OGL_Initialize to shell_macintosh.cpp from marathon2.cpp

Feb 13, 2002 (Br'fin (Jeremy Parsons)):
	Revised Carbon's use of RunCurrentEventLoop to dispatch Carbon MouseMoved events
	(Which should be caught and handled by an event handler installed by mouse.cpp)

Feb 27, 2002 (Br'fin (Jeremy Parsons)):
	Enabled networking under Carbon

Apr 29, 2002 (Loren Petrich):
	Added test for pressing modifier keys on startup
*/

#if defined(EXPLICIT_CARBON_HEADER)
    #include <quicktime/Quicktime.h>
#else
#include <Movies.h>
#include <exception.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "macintosh_cseries.h"
#include "my32bqd.h"

#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprockets Support */

#if !defined(TARGET_API_MAC_CARBON) && !HAVE_SDL_NET
#include "macintosh_network.h" /* For NetDDPOpen() */
#endif

#if HAVE_SDL_NET
#include "sdl_network.h"
#undef main // Prevents SDL from hijacking main()
#endif

// LP addition: local-event management:
#include "LocalEvents.h"

// LP addition: loading of XML files from resource fork and from loaded text file
#include "XML_ResourceFork.h"
#include "XML_DataBlock.h"

// For doing STL sorting
#include <algorithm>
#include <functional>

#ifdef __MWERKS__
#include <new.h>
#endif

#define kMINIMUM_NETWORK_HEAP (3*MEG)

#ifdef PERFORMANCE
#include <Perf.h>
#endif

#include "Logging.h"

#ifdef env68k
#pragma segment shell
#endif

extern void *level_transition_malloc(size_t size);

/* ---------- constants */
#define alrtQUIT_FOR_SURE 138

#define kMINIMUM_MUSIC_HEAP (4*MEG)
#define kMINIMUM_SOUND_HEAP (3*MEG)

// Marathon-engine dialog boxes:
const short FatalErrorAlert = 128;
const short NonFatalErrorAlert = 129;


#ifndef __MWERKS__
#ifdef envppc
QDGlobals qd;
#endif
#ifdef env68k
QDGlobals qd;
#endif
#endif

#ifdef PERFORMANCE
TP2PerfGlobals perf_globals;
#endif

struct system_information_data *system_information;

extern long first_frame_tick, frame_count; /* for determining frame rate */

// LP addition: the local event flags
unsigned long LocalEventFlags = 0;

static bool HasQuicktime = false;
static bool HasNavServices = false;

// Where the MacOS Toolbox (or some equivalent) has been inited
// Necessary to to indicate whether or not to create a dialog box.
static bool AppServicesInited = false;

#ifdef TARGET_API_MAC_CARBON
#ifdef USES_NIBS

CFBundleRef MainBundle;
IBNibRef GUI_Nib;

#endif
#endif

// The modifier for typing in cheat codes: defined in shell_misc.cpp
extern short CheatCodeModMask;

// Whether the modifier keys had been pressed when starting up the app
static bool ModifierKeysInitiallyPressed = false;


/* ---------- externs that I couldn't fit into the #include heirarchy nicely */
extern bool load_and_start_game(FileSpecifier& File);
extern bool handle_open_replay(FileSpecifier& File);

/* ---------- private code */

static void verify_environment(void);
static void initialize_application_heap(void);

static void initialize_system_information(void);
static void initialize_core_events(void);
static void initialize_marathon_music_handler(void);
static pascal OSErr handle_open_document(const AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_quit_application(const AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_print_document(const AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_open_application(const AppleEvent *event, AppleEvent *reply, long myRefCon);
static OSErr required_appleevent_check(const AppleEvent *event);

static void marathon_dialog_header_proc(DialogPtr dialog, Rect *frame);

static void process_event(EventRecord *event);
// LP: "static" removed
void process_screen_click(EventRecord *event);
static void process_key(EventRecord *event, short key);
static void wait_for_highlevel_event(void);
static void handle_high_level_event(EventRecord *event);

void update_any_window(WindowPtr window, EventRecord *event);
void activate_any_window(WindowPtr window, EventRecord *event, bool active);

static bool is_keypad(short keycode);

// LP addition: compose a MacOS event from a local-event-queue event;
// returns whether one was composed
static bool ComposeOSEventFromLocal(EventRecord& Event);

// Reads in the filespec of the root directory
static void ReadRootDirectory();

// Looks for MML files and resources in that directory and parses them
static void FindAndParseFiles(DirectorySpecifier& DirSpec);

// For detecting whether modifiers had initially been pressed
static bool KeyIsPressed(KeyMap key_map, short key_code);
static bool ModifierKeysPressed();

// A function for loading a framework bundle
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);

/* ---------- code */

//#if defined(TARGET_API_MAC_CARBON)
int main(
	void)
/*
#else
void main(
	void)
#endif
*/
{	
	// LP: on Christian Bauer's suggestion, I've enclosed the code in a try-catch block
	try
	{
		initialize_application_heap();
		main_event_loop();
	}
	catch(exception& e)
	{
		if (AppServicesInited)
		{
#ifdef TARGET_API_MAC_CARBON
			csprintf(temporary,
				"Unhandled exception: %s",e.what());
			InitCursor();
			SimpleAlert(kAlertStopAlert,temporary);
#else
			psprintf(ptemporary,"Unhandled exception: %s",e.what());
			ParamText(ptemporary,"\p0",NULL,NULL);
			InitCursor();
			Alert(128,NULL);
#endif
			ExitToShell();
		}
	}
	catch(...)
	{
		if (AppServicesInited)
		{
#ifdef TARGET_API_MAC_CARBON
			csprintf(temporary,
				"Unknown exception");
			InitCursor();
			SimpleAlert(kAlertStopAlert,"Unknown exception");
#else
			psprintf(ptemporary,"Unknown exception");
			ParamText(ptemporary,"\p0",NULL,NULL);
			InitCursor();
			Alert(128,NULL);
#endif
			ExitToShell();
		}
	}
	ExitToShell();
}

/* ---------- private code */

// MML parsers
static XML_DataBlock XML_DataBlockLoader;
static XML_ResourceFork XML_ResourceForkLoader;

// This is so that one will not quit if one presses "escape"
// after using a terminal
static int EscapeKeyRefractoryTime = 0;


static void initialize_application_heap(
	void)
{
	// For telling the screen-frequency dialog not to show
	ModifierKeysInitiallyPressed = ModifierKeysPressed();

/*
#if !defined(TARGET_API_MAC_CARBON)
	MaxApplZone();
#endif
*/
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();

/*
#if !defined(TARGET_API_MAC_CARBON)
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0); *//* resume procedure ignored for multifinder and >=system 7.0 *//*
#endif
*/
	InitCursor();
#if defined(TARGET_API_MAC_CARBON) && defined(__MACH__) && defined(CARBON_USE_QUARTZ_RENDERING)
#if MAC_OS_X_VERSION_10_2 <= MAC_OS_X_VERSION_MAX_ALLOWED
	QDSwapTextFlags(1 | 2 | 4);
#endif
#endif
	long response;
	OSErr error;

#if defined(TARGET_API_MAC_CARBON) && defined(__MACH__)
	const char *home = getenv("HOME");
	if (home) chdir(home);
#endif
	
#if  defined(__MWERKS__) && !defined(COMPLAIN_BAD_ALLOCS)
std::throws_bad_alloc = false; //AS: can't test this code, if it fails, try throws_bad_allocs instead
#endif

	error= Gestalt(gestaltQuickTime, &response);
	if(!error) 
	{
		/* No error finding QuickTime.  Check for ICM so we can use Standard Preview */
		error= Gestalt(gestaltCompressionMgr, &response);
		if(!error) 
		{
			// Now initialize Quicktime's Movie Toolbox,
			// since the soundtrack player will need it
			error = EnterMovies();
			if (!error) {
				HasQuicktime = true;
			}
		}
	}
	
	if ((void*)NavLoad != (void*)kUnresolvedCFragSymbolAddress)
	{
		NavLoad();
		HasNavServices = true;
	}

	// The MacOS Toolbox has now been started up!
	AppServicesInited = true;
	
#ifdef DEBUG
	initialize_debugger(true);
#endif

	SetCursor(*GetCursor(watchCursor));

//#if defined(USE_CARBON_ACCESSORS)
	SetQDGlobalsRandomSeed(TickCount());
/*
#else
	qd.randSeed = TickCount();
#endif
*/

#ifdef PERFORMANCE
	{
		bool succeeded;
		
		succeeded = InitPerf(&perf_globals, 4, 8, true, true, "\pCODE", 0, "\p", 0, 0, 0, 0);
		assert(succeeded);
	}
#endif

	/* Determine what type of system we are working on.. */
	initialize_system_information();
	initialize_core_events();
	
	// LP addition:
	// Try to get this one first.
	initialize_typecodes();
	
	// Safest place to load the configuration files
	SetupParseTree();
	XML_DataBlockLoader.CurrentElement = &RootParser;
	XML_ResourceForkLoader.CurrentElement = &RootParser;
	XML_ResourceForkLoader.SourceName = "[Application]";
	XML_ResourceForkLoader.ParseResourceSet('TEXT');
	
#ifdef TARGET_API_MAC_CARBON
#ifdef USES_NIBS

	// The asserts need ALRT resources 128 and 129 -- and nothing else that needs to be loaded

	MainBundle = CFBundleGetMainBundle();
	assert (MainBundle);
	
	const CFStringRef GUI_Nib_Filename = CFSTR("GUI");
	error = CreateNibReferenceWithCFBundle(MainBundle, GUI_Nib_Filename, &GUI_Nib);
	assert(error == noErr);

#endif
#endif
	
	// Need to set the root directory before doing reading any other files
	ReadRootDirectory();
	
	// Look for such files in subdirectories specified in a STR# resource:
	const int PathID = 128;
	Handle PathStrings = Get1Resource('STR#',PathID);
	if (PathStrings)
	{
		//fdprintf("Parsing external files");
		// Count how many there are
		HLock(PathStrings);
		short *NumPtr = (short *)(*PathStrings);
		short NumStrings = NumPtr[0];
		HUnlock(PathStrings);
		ReleaseResource(PathStrings);
		for (int i=0; i<NumStrings; i++)
		{
			// Get next path specification; quit when one has found an empty one

			unsigned char PathSpec[256];
//#if defined(USE_CARBON_ACCESSORS)
			Str255 pascalPathSpec;
			GetIndString(pascalPathSpec,PathID,i+1);	// Zero-based to one-based indexing
			
			// Make a C string from this Pascal string
			CopyPascalStringToC(pascalPathSpec, (char *)PathSpec);
/*
#else
			GetIndString(PathSpec,PathID,i+1);	// Zero-based to one-based indexing
			
			// Make a C string from this Pascal string
			p2cstr(PathSpec);
#endif
*/
			//fdprintf("Dir Path = %s",PathSpec);
			
			DirectorySpecifier DirSpec;
			if (!DirSpec.SetToSubdirectory((char *)PathSpec)) continue;
			FindAndParseFiles(DirSpec);
		}
	}
	
	// JTP: OpenGL Needs to be initialized *before* we handle preferences
	// otherwise OGL_IsPresent() hasn't been decided yet
	// This is ok to call if HAVE_OPENGL is false.
	OGL_Initialize();
	initialize_preferences();
	GetDateTime(&player_preferences->last_time_ran);
	write_preferences();

	initialize_my_32bqd();
	verify_environment();

	if (FreeMem()>kMINIMUM_SOUND_HEAP) initialize_sound_manager(sound_preferences);

	initialize_marathon_music_handler();
	initialize_ISp(); /* BT: Added April 16, 2000 ISp: Initialize Input Sprockets */
	
	mytm_initialize();
	initialize_keyboard_controller();
	initialize_screen(&graphics_preferences->screen_mode, ModifierKeysInitiallyPressed);
	initialize_marathon();
	initialize_screen_drawing();
	initialize_terminal_manager();
	initialize_shape_handler();
	initialize_fades();

	kill_screen_saver();
	
	set_dialog_header_proc(marathon_dialog_header_proc);
	initialize_images_manager();
	
	/* Load the environment.. */
	load_environment_from_preferences();
	
	initialize_game_state();
//#if defined(USE_CARBON_ACCESSORS)
	Cursor arrow;
	GetQDGlobalsArrow(&arrow);
	SetCursor(&arrow);
/*
#else
	SetCursor(&qd.arrow);
#endif
*/
}

void handle_game_key(
	EventRecord *event,
	short key)
{
	short _virtual;
	bool changed_screen_mode= false;
	bool changed_prefs= false;
	
	_virtual = (event->message >> 8) & charCodeMask;
	
// LP change: implementing Benad's "cheats always on"
// #ifndef FINAL
	if (!game_is_networked && (event->modifiers & CheatCodeModMask))
	{
		short type_of_cheat;
		
		// LP change: this is now conditional
		if (CheatsActive)
		{
			unsigned char cheat_key = key;
			if (cheat_key < 0x20) cheat_key += 0x40;	// Map control+key onto proper key
			type_of_cheat = process_keyword_key(cheat_key);
			if (type_of_cheat != NONE) handle_keyword(type_of_cheat);
		}
	}
// #endif

	if (!is_keypad(_virtual))
	{
		switch(key)
		{
			case '.': case '>': // sound volume up
				changed_prefs= adjust_sound_volume_up(sound_preferences, Sound_AdjustVolume());
				break;
			case ',': case '<': // sound volume down.
				changed_prefs= adjust_sound_volume_down(sound_preferences, Sound_AdjustVolume());
				break;
			case kDELETE: // switch player view
				walk_player_list();
				render_screen(NONE);
				break;
			case '+': case '=':
				if (zoom_overhead_map_in())
					PlayInterfaceButtonSound(Sound_ButtonSuccess());
				else
					PlayInterfaceButtonSound(Sound_ButtonFailure());
				break;
			case '-': case '_':
				if (zoom_overhead_map_out())
					PlayInterfaceButtonSound(Sound_ButtonSuccess());
				else
					PlayInterfaceButtonSound(Sound_ButtonFailure());
				break;
			case '[': case '{':
				if(player_controlling_game())
				{
					PlayInterfaceButtonSound(Sound_ButtonSuccess());
					scroll_inventory(-1);
				}
				else
				{
					decrement_replay_speed();
				}
				break;
			case ']': case '}':
				if(player_controlling_game())
				{
					PlayInterfaceButtonSound(Sound_ButtonSuccess());
					scroll_inventory(1);
				}
				else
				{
					increment_replay_speed();
				}
				break;

			case '%':
				PlayInterfaceButtonSound(Sound_ButtonSuccess());
				toggle_suppression_of_background_tasks();
				break;
/*
#ifndef FINAL
#ifdef DEBUG
			case '!':
				debug_print_weapon_status();
				break;
#endif
#endif
*/

			case '?':
				{
					PlayInterfaceButtonSound(Sound_ButtonSuccess());
					extern bool displaying_fps;
					displaying_fps= !displaying_fps;
				}
				break;
			
			case 0x1b:
				// "Escape" posts a "quit" event
				// ZZZ: changing to make a little friendlier, like the SDL version
				if(!player_controlling_game())
					do_menu_item_command(mGame, iQuitGame, false);
				else {
					if(get_ticks_since_local_player_in_terminal() > 1 * TICKS_PER_SECOND) {
						if(!game_is_networked) {
							do_menu_item_command(mGame, iQuitGame, false);
						}
						else {
							screen_printf("If you wish to quit, press Command+Q.");
						}
					}
				}
				break;
						
			default: // well, let's check the function keys then, using the keycodes.
				switch(_virtual)
				{
					// LP change: disabled these if OpenGL is active;
					// may want to either consolidate or eliminate these
					case kcF1:
						// LP change: turned this into screen-size decrement
						if (graphics_preferences->screen_mode.size > 0)
						{
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
							graphics_preferences->screen_mode.size--;
							changed_screen_mode = changed_prefs = true;
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonFailure());
						break;

					case kcF2:
						// LP change: turned this into screen-size increment
						if (graphics_preferences->screen_mode.size < NUMBER_OF_VIEW_SIZES-1)
						{
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
							graphics_preferences->screen_mode.size++;
							changed_screen_mode = changed_prefs = true;
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonFailure());
						break;

					case kcF3:
						if (!OGL_IsActive())
						{
							// Changed this to resolution toggle; no sense doing this in OpenGL mode
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
							graphics_preferences->screen_mode.high_resolution = !graphics_preferences->screen_mode.high_resolution;
							changed_screen_mode = changed_prefs = true;
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonInoperative());
						break;

					case kcF4:
					case kcF14:
						if (OGL_IsActive())
						{
							// Reset OpenGL textures;
							// play the button sound in advance to get the full effect of the sound
							PlayInterfaceButtonSound(Sound_OGL_Reset());
							OGL_ResetTextures();
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonInoperative());
						break;

						// One can check on the shift key with event->modifiers & shiftKey
						// and likewise for the option and control keys
						// Deleted old code contains setting of "draw_every_other_line" option,
						// which is now gone
						
					case kcF11:
						if (graphics_preferences->screen_mode.gamma_level)
						{
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
							graphics_preferences->screen_mode.gamma_level--;
							change_gamma_level(graphics_preferences->screen_mode.gamma_level);
							changed_prefs= true;
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonFailure());
						break;
						
					case kcF12:
						if (graphics_preferences->screen_mode.gamma_level<NUMBER_OF_GAMMA_LEVELS-1)
						{
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
							graphics_preferences->screen_mode.gamma_level++;
							change_gamma_level(graphics_preferences->screen_mode.gamma_level);
							changed_prefs= true;
						}
						else
							PlayInterfaceButtonSound(Sound_ButtonFailure());
						break;
					
					// LP addition: screendump facility
					// Added "escape" key so as to have a key available for PowerBooks and iMacs
					// Changed "escape" key to F8 key out of protests over misleading significance
					case kcF13:
					case kcF9:
						dump_screen();
						break;
			
					case kcF5:
						// Make the chase cam switch sides
						if (ChaseCam_IsActive())
							PlayInterfaceButtonSound(Sound_ButtonSuccess());
						else
							PlayInterfaceButtonSound(Sound_ButtonInoperative());
						ChaseCam_SwitchSides();
						break;
					
					case kcF6:
						// Toggle the chase cam
						PlayInterfaceButtonSound(Sound_ButtonSuccess());
						ChaseCam_SetActive(!ChaseCam_IsActive());
						break;
					
					case kcF7:
						// Toggle tunnel vision
						PlayInterfaceButtonSound(Sound_ButtonSuccess());
						SetTunnelVision(!GetTunnelVision());
						break;
					
					case kcF8:
						// Toggle the crosshairs
						PlayInterfaceButtonSound(Sound_ButtonSuccess());
						Crosshairs_SetActive(!Crosshairs_IsActive());
						break;
					
					case kcF10:
						// Toggle the position display
						PlayInterfaceButtonSound(Sound_ButtonSuccess());
						{
							extern bool ShowPosition;
							ShowPosition = !ShowPosition;
						}
						break;
					
					default:
						if(get_game_controller()==_demo)
						{
							set_game_state(_close_game);
						}
						break;
				}
				break;
		}
	}
	if (changed_screen_mode)
	{
		change_screen_mode(&graphics_preferences->screen_mode, true);
		render_screen(0);
	}
	if (changed_prefs) write_preferences();
}

static void verify_environment(
	void)
{
#if defined(TARGET_API_MAC_CARBON)
	// Carbon systems always have:
	// >= System 7
	// Color QuickDraw
	// PowerPC CPU's
	/*
	OSErr result;
	long response;
	long getCPUtype;
	
	result = Gestalt(gestaltSystemVersion, &response);
	if (result!=noErr||response<0x1001)
	{
		alert_user(fatalError, strERRORS, badSystem, response);
	}

	result = Gestalt(gestaltQuickdrawVersion, &response);
	if (result!=noErr||response<gestalt32BitQD13)
	{
		alert_user(fatalError, strERRORS, badQuickDraw, 0);
	}
	
	result = Gestalt(gestaltNativeCPUtype, &getCPUtype);
	if(result == noErr)
	{
		if (0x100 & getCPUtype) {
			// We're on a PowerPC, rock on
		}
		else
		{
			result = Gestalt ( gestaltProcessorType, &getCPUtype );
			if(getCPUtype != gestalt68040)
			alert_user(fatalError, strERRORS, badProcessor, response);
		}
	}
	else
	{
		alert_user(fatalError, strERRORS, badProcessor, response);
	}
	*/
	
#else
	SysEnvRec environment;
	OSErr result;

	result= SysEnvirons(curSysEnvVers, &environment);
	if (result!=noErr||environment.systemVersion<0x0700)
	{
		alert_user(fatalError, strERRORS, badSystem, environment.systemVersion);
	}
	
	if (!environment.hasColorQD)
	{
		alert_user(fatalError, strERRORS, badQuickDraw, 0);
	}
	
	if (!environment.processor&&environment.processor<env68040)
	{
		alert_user(fatalError, strERRORS, badProcessor, environment.processor);
	}
#endif
	
	if (FreeMem()<2900000)
	{
		alert_user(fatalError, strERRORS, badMemory, FreeMem());
	}
}

static void handle_high_level_event(
	EventRecord *event)
{
	if(system_information->has_apple_events)
	{
		AEProcessAppleEvent(event);
	}
}

static void initialize_core_events(
	void)
{
	if(system_information->has_seven && system_information->has_apple_events)
	{
		AEEventHandlerUPP open_document_proc;
		AEEventHandlerUPP quit_application_proc;
		AEEventHandlerUPP print_document_proc;
		AEEventHandlerUPP open_application_proc;
		OSErr err;
		
		// Changed ending from "Proc" to "UPP"
		open_document_proc= NewAEEventHandlerUPP(handle_open_document);
		quit_application_proc= NewAEEventHandlerUPP(handle_quit_application);
		print_document_proc= NewAEEventHandlerUPP(handle_print_document);
		open_application_proc= NewAEEventHandlerUPP(handle_open_application);
		assert(open_document_proc && quit_application_proc 
			&& print_document_proc && open_application_proc);
	
		err= AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, open_document_proc, 0,
			false);
		assert(!err);
		
		err= AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, quit_application_proc, 0,
			false);
		assert(!err);
	
		err= AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, print_document_proc, 0,
			false);
		assert(!err);
	
		err= AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, open_application_proc, 0,
			false);
		assert(!err);
	}
}

static pascal OSErr handle_open_document(
	const AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr err;
	AEDescList docList;

	(void)(reply);
	(void)(myRefCon);
	
	err= AEGetParamDesc(event, keyDirectObject, typeAEList, &docList);
	if(!err)
	{
		err= required_appleevent_check(event);
		if(!err)
		{
			short game_state= get_game_state();
			if(
				game_state == _display_main_menu ||
				game_state == _display_intro_screens
			)
			{
				long itemsInList;
			
				err= AECountItems(&docList, &itemsInList);
	
				if(!err)
				{
					bool done= false;
					long index;
					FSSpec myFSS;
					Size actualSize;
	
					for(index= 1; !done && index<= itemsInList; index++) 
					{
						DescType typeCode;
						AEKeyword theKeyword;
	
						err=AEGetNthPtr(&docList, index, typeFSS, &theKeyword, &typeCode, 
							&myFSS, sizeof(FSSpec), &actualSize);
						if(!err)
						{
							// FInfo theFInfo;
	
							// FSpGetFInfo(&myFSS, &theFInfo);
							// Create a file object to use instead
							FileSpecifier InputFile;
							InputFile.SetSpec(myFSS);
							
							// LP change, since the filetypes are no longer constants
							// OSType Typecode = theFInfo.fdType;
							Typecode type = InputFile.GetType();
							if (type == _typecode_scenario)
							{
								set_map_file(InputFile);
							}
							else if (type == _typecode_savegame)
							{
								if(load_and_start_game(InputFile))
								{
									done= true;
								}
							}
							else if (type == _typecode_film)
							{
								if(handle_open_replay(InputFile))
								{
									done= true;
								}
							}
							else if (type == _typecode_physics)
							{
								set_physics_file(InputFile);
							}
							else if (type == _typecode_shapes)
							{
								open_shapes_file(InputFile);
							}
							else if (type == _typecode_sounds)
							{
								open_sound_file(InputFile);
							}
						}
					}
				}
			}
			else
			{
//				vassert(false, "Attempting to open a file during a game state that doesn't allow it!");
				err= errAEEventNotHandled;
			}
		}
	}
	
	return err;
}

static pascal OSErr handle_quit_application(
	const AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr err;
	
	(void)(reply);
	(void)(myRefCon);
	err= required_appleevent_check(event);
	if(err) return err;
	
	set_game_state(_quit_game);
	return noErr;
}

static pascal OSErr handle_print_document(
	const AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	(void)(event);
	(void)(reply);
	(void)(myRefCon);

	return (errAEEventNotHandled);
}

static pascal OSErr handle_open_application(
	const AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr	err;

	(void)(reply);
	(void)(myRefCon);
	err=required_appleevent_check(event);
	if(err) return err;

	return err;
}

static OSErr required_appleevent_check(
	const AppleEvent *event)
{
	OSErr err;
	DescType typeCode;
	Size actualSize;
	
	err=AEGetAttributePtr(event, keyMissedKeywordAttr, typeWildCard, &typeCode,
		0l, 0, &actualSize);
		
	if(err==errAEDescNotFound) return noErr;
	if(err==noErr) return (errAEEventNotHandled);

	return err;
}

static void initialize_system_information(
	void)
{
	long system_version, apple_events_present, processor_type, classic_mode;
	OSErr err;
	
	/* Allocate the system information structure.. */	
	system_information= (struct system_information_data *) 
		NewPtr(sizeof(struct system_information_data));
	assert(system_information);

	/* System Version */
	system_information->has_seven= false;
	system_information->has_ten= false;
	err= Gestalt(gestaltSystemVersion, &system_version);
	if (!err)
	{
		if(system_version>=0x0700) system_information->has_seven= true;
		if(system_version>=0x1000) system_information->has_ten= true;
	}

	system_information->machine_has_network_memory= (FreeMem()>5000000) ? true : false;

	/* Appleevents? */
	err= Gestalt(gestaltAppleEventsAttr, &apple_events_present);
	if(!err && (apple_events_present & 1<<gestaltAppleEventsPresent))
	{
		system_information->has_apple_events= true;
	} else {
		system_information->has_apple_events= false;
	}

	/* Is appletalk available? */
#if defined(TARGET_API_MAC_CARBON) || HAVE_SDL_NET
	// Networking will not be appletalk
	system_information->appletalk_is_available= false;
#else
	if(system_information->has_seven && NetDDPOpen()==noErr && FreeMem()>kMINIMUM_NETWORK_HEAP)
	{
		system_information->appletalk_is_available= true;
	} else {
		system_information->appletalk_is_available= false;
	}
#endif

	system_information->sdl_networking_is_available= false;
#if HAVE_SDL_NET
	if(SDL_Init != NULL && SDL_Init(0) == 0)
	{
		atexit(SDL_Quit);
		if (SDLNet_Init() == 0)
		{
			system_information->sdl_networking_is_available= true;
			atexit(SDLNet_Quit);
		}
	}
#endif

	/* Type of machine? */
	err= Gestalt(gestaltNativeCPUtype, &processor_type);
	if(!err)
	{
		switch (processor_type)
		{
			case gestaltCPU68000:
			case gestaltCPU68010: // highly unlikely, wouldn't you say?
			case gestaltCPU68020:
			case gestaltCPU68030:
				system_information->machine_is_68k= true; 
				system_information->machine_is_68040= false; 
				system_information->machine_is_ppc= false;
				break;
				
			case gestaltCPU68040:
				system_information->machine_is_68k= true; 
				system_information->machine_is_68040= true; 
				system_information->machine_is_ppc= false;
				break;
				
			case gestaltCPU601:
			case gestaltCPU603:
			case gestaltCPU604:
			default: // assume the best
				system_information->machine_is_68k= false; 
				system_information->machine_is_68040= false; 
				system_information->machine_is_ppc= true;
				break;
		}
	}
	else // handle sys 6 machines, which are certainly not ppcs. (can they even be '040s?)
	{
		system_information->machine_is_68k= true; 
		system_information->machine_is_ppc= false;

		err= Gestalt(gestaltProcessorType, &processor_type);
		if(!err)
		{
			if(processor_type==gestalt68040)
			{
				system_information->machine_is_68040= true; 
			} else {
				system_information->machine_is_68040= false; 
			}
		}
	}
		
	//Are we in Classic?
	err= Gestalt(gestaltMacOSCompatibilityBoxAttr, &classic_mode);
	if(!err && (classic_mode & 1<<gestaltMacOSCompatibilityBoxPresent))
	{
		system_information->machine_is_bluebox= true;
	} else {
		system_information->machine_is_bluebox= false;
	}
}

bool is_keypad(
	short keycode)
{
	return keycode >= 0x41 && keycode <= 0x5c;
}

bool networking_available(
	void)
{
#if HAVE_SDL_NET
	return system_information->sdl_networking_is_available;
#else
	return system_information->appletalk_is_available;
#endif
}

/* ---------- dialog headers */

#define DIALOG_HEADER_RESOURCE_OFFSET 8000
#define DIALOG_HEADER_VERTICAL_INSET 10
#define DIALOG_HEADER_HORIZONTAL_INSET 13

/* if this dialog has a known refCon, draw the appropriate header graphic */
static void marathon_dialog_header_proc(
	DialogPtr dialog,
	Rect *frame)
{
//#if defined(USE_CARBON_ACCESSORS)
	long refCon= GetWRefCon(GetDialogWindow(dialog));
/*
#else
	long refCon= GetWRefCon(dialog);
#endif
*/
	
	if (refCon>=FIRST_DIALOG_REFCON && refCon<=LAST_DIALOG_REFCON)
	{
		/* Mike Benonis Change */
		if (refCon!=FIRST_DIALOG_REFCON)
		{
			PicHandle picture= GetPicture(DIALOG_HEADER_RESOURCE_OFFSET - FIRST_DIALOG_REFCON + refCon);
			
			if (picture)
			{
				Rect destination= (*picture)->picFrame;
				
				OffsetRect(&destination, frame->left-destination.left+DIALOG_HEADER_HORIZONTAL_INSET,
					frame->top-destination.top+DIALOG_HEADER_VERTICAL_INSET);
				DrawPicture(picture, &destination);
			}
		}
	}
}

static void initialize_marathon_music_handler(
	void)
{
	FileSpecifier SongFile;
	// FSSpec music_file_spec;
	// OSErr error;
	bool initialized= false;
	
	SongFile.SetNameWithPath(getcstr(temporary, strFILENAMES, filenameMUSIC)); // typecode: NONE
	if (SongFile.Exists())
	// error= get_file_spec(&music_file_spec, strFILENAMES, filenameMUSIC, strPATHS);
	// if (!error)
	{
		// bool is_folder, was_aliased;
	
		// ResolveAliasFile(&music_file_spec, true, &is_folder, &was_aliased);
		// initialized= initialize_music_handler((FileDesc *) &music_file_spec);
		initialized= initialize_music_handler(SongFile);
	}
}

static void main_event_loop(
	void)
{
#if TARGET_API_MAC_CARBON
	static EventTypeSpec mouseEvents[] = {
		{kEventClassMouse, kEventMouseDown},
		{kEventClassMouse, kEventMouseUp},
		{kEventClassMouse, kEventMouseWheelMoved},
  		{kEventClassMouse, kEventMouseMoved},
		{kEventClassMouse, kEventMouseDragged}
	};
	EventRef theEvent;
	EventTargetRef theTarget;
	
	theTarget = GetEventDispatcherTarget();
#endif
	wait_for_highlevel_event();
	
	while(get_game_state()!=_quit_game)
	{
		bool use_waitnext;

#if TARGET_API_MAC_CARBON
		// JTP: Give room for catching mouse Events
		RunCurrentEventLoop(kEventDurationNoWait);
		
		// Pass off our mouse events
		while(ReceiveNextEvent(5, mouseEvents, kEventDurationNoWait, true, &theEvent) == noErr)
		{
			SendEventToEventTarget (theEvent, theTarget);
			ReleaseEvent(theEvent);
		}
#endif

		if(try_for_event(&use_waitnext))
		{
			EventRecord event;
			bool got_event= false;
			
			// Set up for ignoring keyboard events while the ISp is reading the keyboard
			uint16 EventMask = everyEvent;
			if (ISp_IsUsingKeyboard()) EventMask &= ~(keyDownMask | autoKeyMask); 
			
			// Get a local event directly; don't bother to put it into the MacOS event queue
			if (ComposeOSEventFromLocal(event))
			{
				got_event = true;
			}
			else if(use_waitnext)
			{
				got_event= WaitNextEvent(EventMask, &event, 2, (RgnHandle) NULL);
			}
			else
			{
#if TARGET_API_MAC_CARBON
				got_event= GetNextEvent(EventMask, &event);
#else
				got_event= GetOSEvent(EventMask, &event);
#endif
			}
			
			if(got_event) process_event(&event);
			
			if(get_game_state()==_game_in_progress) 
			{
				FlushEvents(keyDownMask|keyUpMask|autoKeyMask, 0);
			}
		}

		idle_game_state();
	}

	/* Flush all events on quit.. */
	FlushEvents(everyEvent, 0);
}

/* ------------- private code */
static void wait_for_highlevel_event(
	void)
{
	EventRecord event;
	bool done= false;
	
	while(!done)
	{
		if(WaitNextEvent(highLevelEventMask, &event, 0, NULL))
		{
			process_event(&event);
			done= true;
		}
	}
}

// LP: GetScreenGrafPort() cast to WindowPtr is OK, since it is either screen_window in screen.cpp
// or a DrawSprocket context's GrafPort (no window)

/* Can't be static because the general_filter_proc calls this */
void update_any_window(
	WindowPtr window,
	EventRecord *event)
{
	GrafPtr old_port;

	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
	SetPort(GetWindowPort(window));
/*
#else
	SetPort(window);
#endif
*/
	BeginUpdate(window);

//#if defined(USE_CARBON_ACCESSORS)
	if(window==GetWindowFromPort(GetScreenGrafPort()))
/*
#else
	if(window==(WindowPtr)GetScreenGrafPort())
#endif
*/
	{
		update_game_window(window, event);
	}
	EndUpdate(window);
	SetPort(old_port);
}

/* Can't be static because the general_filter_proc calls this */
void activate_any_window(
	WindowPtr window,
	EventRecord *event,
	bool active)
{
//#if defined(USE_CARBON_ACCESSORS)
	if(window==GetWindowFromPort(GetScreenGrafPort()))
/*
#else
	if(window==(WindowPtr)GetScreenGrafPort())
#endif
*/
	{
		activate_screen_window(window, event, active);
	}
}

static void process_event(
	EventRecord *event)
{
	WindowPtr window;
	short part_code;

	switch (event->what)
	{
		case mouseDown:
			part_code= FindWindow(event->where, &window);
			switch (part_code)
			{
				case inContent:
					process_screen_click(event);
					break;

                                case inSysWindow: /* DAs and the menu bar can blow me */
                                case inMenuBar:
				default:
                                        logAnomaly1("mouseDown in unexpected part_code %d", part_code);
					break;
			}
			break;
		
		case keyDown:
		case autoKey:
			process_key(event, toupper(event->message&charCodeMask));
			break;
			
		case updateEvt:
			update_any_window((WindowPtr)event->message, event);
			break;
			
		case activateEvt:
			activate_any_window((WindowPtr)(event->message), event, event->modifiers&activeFlag);
			break;

		case kHighLevelEvent:
			handle_high_level_event(event);
			break;

		case osEvt:
			switch (event->message>>24)
			{
				case suspendResumeMessage:
					// LP: added some reasonable behavior --
					// the Marathon app will be hidden when switched away from
					if (event->message&resumeFlag)
					{
						/* resume */
						ResumeDisplay(event);
						if (get_game_state()==_game_in_progress)
						{
#if !TARGET_API_MAC_CARBON
							// Lets the MacOS X GUI manager handle the mouse showing/hiding
							hide_cursor();
#endif
							set_keyboard_controller_status(true);
						}
						else
						{
							// whatever is necessary to update it
//#if defined(USE_CARBON_ACCESSORS)
							Rect portRect;
							GetPortBounds(GetScreenGrafPort(), &portRect);
							InvalWindowRect(GetWindowFromPort(GetScreenGrafPort()), &portRect);
/*
#else
							InvalRect(&GetScreenGrafPort()->portRect);
#endif
*/
						}
					}
					else
					{
						/* suspend */
						if (get_game_state()==_game_in_progress)
						{
							set_keyboard_controller_status(false);
						}
						SuspendDisplay(event);
					}
					break;
			}
			break;
			
		default:
			break;
	}
}

// LP: "static" removed
void process_screen_click(
	EventRecord *event)
{
	GrafPtr old_port;
	Point where= event->where;
	bool cheatkeys_down;
	
	GetPort(&old_port);
	SetPort((GrafPtr)GetScreenGrafPort());
	
	GlobalToLocal(&where);
	cheatkeys_down= has_cheat_modifiers(event);
	portable_process_screen_click(where.h, where.v, cheatkeys_down);

	SetPort(old_port);	
}

static void process_key(
	EventRecord *event,
	short key)
{
	process_game_key(event, key);
}


// Index of which local event to look at; the event poster will cycle through
// the possible events, looking for one that is present.
static int LocalEventIndex = 0;

// LP addition: post any events found in the local event queue
bool ComposeOSEventFromLocal(EventRecord& Event)
{
	// Don't quit when in terminal mode!
	if (player_in_terminal_mode(local_player_index))
	{
		EscapeKeyRefractoryTime = 15;
		ClearLocalEvent(LocalEvent_Quit);
	}
	// Or some time after! (half a second of ticks)
	else if (EscapeKeyRefractoryTime > 0)
	{
		EscapeKeyRefractoryTime--;
		ClearLocalEvent(LocalEvent_Quit);
	}

	int SavedLocalEventIndex = LocalEventIndex;
	for (int i=0; i<32; i++)
	{
		// Quick remainder after division by 32 (2^5 or 0x20)
		LocalEventIndex = (SavedLocalEventIndex + i) & 0x1f;
		
		unsigned long LocalEvent = 1 << ((unsigned long)LocalEventIndex);
		if (GetLocalEvent(LocalEvent))
		{
			// Compose the fake event:
			struct EventFeatures
			{
				long Message;
				short Modifiers;
			};
			
			const int NumLocalEventTypes = 23;
			EventFeatures EventFeatureList[NumLocalEventTypes] =
			{
				{'q',cmdKey},
				{'p',cmdKey},
				{'<',0},
				{'>',0},
	
				{'-',0},
				{'+',0},
				{'[',0},
				{']',0},
	
				{kDELETE,0},
				{'%',0},
				{'?',0},
				// These F keys are all virtual, and therefore 1 byte to the left
				{kcF3 << 8,0},
	
				{kcF1 << 8,0},
				{kcF2 << 8,0},
				{kcF11 << 8,0},
				{kcF12 << 8,0},
	
				{kcF5 << 8,0},
				{kcF6 << 8,0},
				{kcF7 << 8,0},
				{kcF8 << 8,0},
	
				{kcF10 << 8,0},
				{kcF13 << 8,0},
				{kcF14 << 8,0}
			};
			
			vassert(LocalEventIndex < NumLocalEventTypes, csprintf(temporary, "Out-of-range local event index: %d", LocalEventIndex));
			
			EventFeatures &Features = EventFeatureList[LocalEventIndex];
			
			Event.what = keyDown;
			Event.message = Features.Message;
			Event.modifiers = Features.Modifiers;
			
			// Set up for next one and quit:
			LocalEventIndex = (LocalEventIndex + 1) & 0x1f;
			return true;
		}
	}
	
	// Reset this value
	LocalEventIndex = SavedLocalEventIndex;
	return false;
}


/* ----------- PRIVATE CODE */
/* Should be in shell.h and shell.c */
bool machine_has_quicktime() 
{
	return HasQuicktime;
}

bool machine_has_nav_services()
{
	return HasNavServices;
}

// returns -1 (first string greater), 0 (strings equal), or +1 (second string greater)
static int PascalCompare(const Str255 Str1, const Str255 Str2)
{
	int Len1 = int(Str1[0]);
	int Len2 = int(Str2[0]);
	int MinLen = MIN(Len1,Len2);
	
	// Do the shared-extent subsets match?
	for (int k=1; k<=MinLen; k++)
	{
		unsigned char c1 = Str1[k];
		unsigned char c2 = Str2[k];
		if (c1 > c2) return -1;
		else if (c1 < c2) return 1;
	}
	
	// If matching contents are equal, then the longer string is the greater one
	if (Len1 > Len2) return -1;
	else if (Len1 < Len2) return 1;
	
	return 0;
}


// For adding is-directory and perhaps other flags to a FSSpec
struct TypedSpec
{
	enum {
		Directory,
		TextFile,
		ResourceFile
	};
	
	FSSpec Spec;
	short Type;
	
	// For comparison
	bool operator==(const TypedSpec& Spec);
};


bool TypedSpec::operator==(const TypedSpec& TSpec)
{
	if (Type != TSpec.Type) return false;
	if (Spec.vRefNum != TSpec.Spec.vRefNum) return false;
	if (Spec.parID != TSpec.Spec.parID) return false;
	if (PascalCompare(Spec.name,TSpec.Spec.name) != 0) return false;
		
	return true;
}


// Needed for an STL index sort
struct AlphabeticalCompareTypedSpecs: public binary_function<int, int, bool> {

	vector<TypedSpec>::iterator SpecListIter;
	
	bool operator()(int i1, int i2)
		{return (PascalCompare(SpecListIter[i1].Spec.name,SpecListIter[i2].Spec.name) < 0);}
};


// Put this stuff out here, to avoid eating up stack space
static CInfoPBRec PB;
static TypedSpec DummySpec;	// push_back() food
static long ParentDir;
static short VolRefNum;
static AlphabeticalCompareTypedSpecs CompareTS;

void ReadRootDirectory()
{
	// Set up its name
	FileSpecifier F;
	F.SetToApp();
	F.SetName("RootDirectory.txt",NONE);
	
	// No need to work with a nonexistent file
	if (!F.Exists()) return;
	
	OpenedFile OFile;
	if (!F.Open(OFile)) return;
	
	long Len = 0;
	OFile.GetLength(Len);
	if (Len <= 0) return;
	
	vector<char> FileContents(Len+1);
	if (!OFile.Read(Len,&FileContents[0])) return;
	FileContents[Len] = 0;
	
	// Strip Unix-style comments, blank lines
	int StartIndex = NONE;
	enum {
		S_Normal,
		S_LineEnded,
		S_InComment
	};
	int State = S_LineEnded;
	for (unsigned int k=0; k<FileContents.size(); k++)
	{
		char c = FileContents[k];
		if (c == '\r' || c == '\n')
		{
			if (State == S_Normal)
			{
				// Found end of desired string;
				// tack on C end-of-string
				FileContents[k] = 0;
				break;
			}
			else
				State = S_LineEnded;
		}
		else if (State == S_LineEnded)
		{
			if (c == '#')
			{
				// A comment begins here
				State = S_InComment;
			}
			else if (c == ' ' || c == '\t')
			{
				// Initial whitespace -- treat as the end of the previous line
			}
			else
			{
				// The desired line begins here
				StartIndex = k;
				State = S_Normal;
			}
		}
	}
	
	// No filespec found?
	if (StartIndex == NONE) return;
		
	Files_SetRootDirectory(&FileContents[StartIndex]);
}


void FindAndParseFiles(DirectorySpecifier& DirSpec)
{
	// fdprintf("Entering FAPF; parID = %d",DirSpec.Get_parID());
	// Maintained separately, since updating this ought not to affect the FSSpec,
	// which will be left unchanged in case of failure.
	ParentDir = DirSpec.Get_parID();
	VolRefNum = DirSpec.Get_vRefNum();
	
	// Specific for each level of recursion;
	// prepare for an index sort
	vector<TypedSpec> SpecList;
	vector<int> SpecIndices;
	
	// Walk the directory, searching for a file with a matching name.
	// Have separate indices for all file-system objects
	// and for ones included in the list.
	unsigned short FileIndex = 0, ListedFileIndex = 0;
	while(true)
	{
		// Resetting to look for next file in directory
		obj_clear(PB);
		DummySpec.Spec.vRefNum = VolRefNum;
		PB.hFileInfo.ioVRefNum = VolRefNum;
		PB.hFileInfo.ioNamePtr = DummySpec.Spec.name;
		PB.hFileInfo.ioDirID = ParentDir;
		PB.hFileInfo.ioFDirIndex = FileIndex + 1;	// Zero-based to one-based indexing
		
		// Quit if ran out of files
		OSErr Err = PBGetCatInfo(&PB, false);
		if (Err != noErr) break;
		
		// Skip if the first character is a period
		// (the Unix-land convention for an invisible file)
		// or if the filename is empty;
		// be sure to advance to the next file if so
		if ((DummySpec.Spec.name[0] < 1) || (DummySpec.Spec.name[1] == '.'))
		{
			FileIndex++;
			continue;
		}
		
		bool IsDir = (PB.hFileInfo.ioFlAttrib & 0x10) != 0;
		
		// First order of business: set the directory or parent-directory ID appropriately
		if (IsDir)
		{
			DummySpec.Spec.parID = PB.dirInfo.ioDrDirID;
			
			// Check for invisibility in the Finder;
			// if invisible, advance to the next file
			if (PB.dirInfo.ioDrUsrWds.frFlags & kIsInvisible)
			{
				FileIndex++;
				continue;
			}
			
			DummySpec.Type = TypedSpec::Directory;
			
			// Add!
			SpecList.push_back(DummySpec);
			SpecIndices.push_back(ListedFileIndex++);
		}
		else
		{
			DummySpec.Spec.parID = ParentDir;
			
			// Check for invisibility in the Finder;
			// if invisible, advance to the next file
			if (PB.hFileInfo.ioFlFndrInfo.fdFlags & kIsInvisible)
			{
				FileIndex++;
				continue;
			}
			
			OSType Type = PB.hFileInfo.ioFlFndrInfo.fdType;
			if (Type == 'TEXT' || Type=='MML ')
			{
				DummySpec.Type = TypedSpec::TextFile;
				
				// Add!
				SpecList.push_back(DummySpec);
				SpecIndices.push_back(ListedFileIndex++);
			}
			else if (Type == 'rsrc')
			{
				DummySpec.Type = TypedSpec::ResourceFile;
				
				// Add!
				SpecList.push_back(DummySpec);
				SpecIndices.push_back(ListedFileIndex++);
			}
			else
			{
			    DummySpec.Type = TypedSpec::TextFile;

			    // Add!
			    SpecList.push_back(DummySpec);
			    SpecIndices.push_back(ListedFileIndex++);
			}
		}
		
		// Next one
		FileIndex++;
	}
	
	// DEBUG
	//for (int q=0; q<SpecList.size(); q++)
	//{
	//	TypedSpec& Spec = SpecList[q];
	//	int len = Spec.Spec.name[0];
	//	memcpy(temporary,Spec.Spec.name+1,len);
	//	temporary[len] = 0;
	//	fdprintf("Type = %hd  parID = %d  Name = %s",Spec.Type,Spec.Spec.parID,temporary);
	//}
	
	// Do STL index sort
	CompareTS.SpecListIter = SpecList.begin();
	sort(SpecIndices.begin(),SpecIndices.end(),CompareTS);
	
	// DEBUG
	//for (int q=0; q<SpecIndices.size(); q++)
	//{
	//	TypedSpec& Spec = SpecList[SpecIndices[q]];
	//	int len = Spec.Spec.name[0];
	//	memcpy(temporary,Spec.Spec.name+1,len);
	//	temporary[len] = 0;
	//	fdprintf("Type = %hd  parID = %d  Name = %s",Spec.Type,Spec.Spec.parID,temporary);
	//}
	
	for (FileIndex=0; FileIndex<SpecIndices.size(); FileIndex++)
	{
		// fdprintf("%d - %d",FileIndex,SpecIndices[FileIndex]);
		TypedSpec& SLMember = SpecList[SpecIndices[FileIndex]];
		switch(SLMember.Type)
		{
		case TypedSpec::Directory:
			{
				DirectorySpecifier ChildDirSpec;
				ChildDirSpec.Set_vRefNum(SLMember.Spec.vRefNum);
				ChildDirSpec.Set_parID(SLMember.Spec.parID);
				FindAndParseFiles(ChildDirSpec);
			}
			break;
		
		case TypedSpec::TextFile:
			{
				FileSpecifier FileSpec;
				FileSpec.SetSpec(SLMember.Spec);
				OpenedFile OFile;
				if (FileSpec.Open(OFile))
				{
					long Len = 0;
					OFile.GetLength(Len);
					if (Len <= 0) break;
					
					vector<char> FileContents(Len);
					if (!OFile.Read(Len,&FileContents[0])) break;
					
					char FileName[256];
					FileSpec.GetName(FileName);
					FileName[31] = 0;	// Use only first 31 characters of filename (MacOS Classic)
					// fdprintf("Loading from text file %s",FileName);
					
					XML_DataBlockLoader.SourceName = FileName;
					if (!XML_DataBlockLoader.ParseData(&FileContents[0],Len))
					{
#ifdef TARGET_API_MAC_CARBON
						SimpleAlert(kAlertStopAlert,"There were configuration-file parsing errors");
#else
						ParamText("\pThere were configuration-file parsing errors",0,0,0);
						Alert(FatalErrorAlert,NULL);
						ExitToShell();
#endif
					}
				}
			}
			break;
		
		case TypedSpec::ResourceFile:
			{
				FileSpecifier FileSpec;
				FileSpec.SetSpec(SLMember.Spec);
				
				char FileName[256];
				FileSpec.GetName(FileName);
				FileName[31] = 0;	// Use only first 31 characters of filename (MacOS Classic)
				// fdprintf("Loading from resource file %s",FileName);
				
				XML_ResourceForkLoader.SourceName = FileName;
				XML_LoadFromResourceFork(FileSpec);
			}
			break;
		
		default:
			// Suppressed because for some reason, bad types creep in.
			// However, this suppression seems to be harmless.
			// vassert(false,csprintf(temporary,"Bad type: %d",SLMember.Type));
			break;
		}
	}
	// fdprintf("Exiting FAPF");
}


void XML_LoadFromResourceFork(FileSpecifier& File)
{
	OpenedResourceFile OFile;
	if (File.Open(OFile))
	{
		OFile.Push();
		XML_ResourceForkLoader.ParseResourceSet('TEXT');
		OFile.Pop();
	}
}


// Taken from is_pressed() of keyboard_dialog.cpp (the Macintosh version)
static bool KeyIsPressed(KeyMap key_map, short key_code)
{
	return ((((byte*)key_map)[key_code>>3] >> (key_code & 7)) & 1);
}

// Uses key codes listed in
// http://developer.apple.com/techpubs/mac/Toolbox/Toolbox-40.html#HEADING40-39
static bool ModifierKeysPressed()
{
	KeyMap key_map;
	GetKeys(key_map);
	
	bool WerePressed = false;
	WerePressed |= KeyIsPressed(key_map,0x38);	// Shift key
	WerePressed |= KeyIsPressed(key_map,0x3a);	// Option key
	WerePressed |= KeyIsPressed(key_map,0x37);	// Command key
	
	return WerePressed;
}


// Gets a function pointer for a MacOS-X function
// that may not be explicitly available for CFM Carbon (Carbon/Classic).
// Returns NULL if such a function pointer could not be found
void *GetSystemFunctionPointer(const CFStringRef FunctionName)
{
	static bool AlreadyLoaded = false;
	static CFBundleRef SysBundle = NULL;

	if (!AlreadyLoaded)
	{
		OSStatus err;
		
		// Get the bundle that contains CoreGraphics	
		err = LoadFrameworkBundle(CFSTR("ApplicationServices.framework"), &SysBundle);
		
		if (err != noErr) SysBundle = NULL;
		AlreadyLoaded = true;
	}
	
	return (SysBundle) ? CFBundleGetFunctionPointerForName(SysBundle,FunctionName) : NULL;
}



// Cribbed from some of Apple's sample code -- CallMachOFramework.c
// For locating a Mach-O bundle for some CFM code,
// like this Carbon/Classic code
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
	// This routine finds a the named framework and creates a CFBundle 
	// object for it.  It looks for the framework in the frameworks folder, 
	// as defined by the Folder Manager.  Currently this is 
	// "/System/Library/Frameworks", but we recommend that you avoid hard coded 
	// paths to ensure future compatibility.
	//
	// You might think that you could use CFBundleGetBundleWithIdentifier but 
	// that only finds bundles that are already loaded into your context. 
	// That would work in the case of the System framework but it wouldn't 
	// work if you're using some other, less-obvious, framework.
{
	OSStatus  err;
	FSRef   frameworksFolderRef;
	CFURLRef baseURL;
	CFURLRef bundleURL;
	
	//MoreAssertQ(bundlePtr != nil);
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	// Find the frameworks folder and create a URL for it.
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Append the name of the framework to the URL.
	
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Create a bundle based on that URL and load the bundle into memory.
	// We never unload the bundle, which is reasonable in this case because 
	// the sample assumes that you'll be calling functions from this 
	// framework throughout the life of your application.
	
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
		}
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	} 
	if (baseURL != nil) {
		CFRelease(baseURL);
	} 
	
	return err;
}
