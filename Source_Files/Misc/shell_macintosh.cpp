/*
SHELL_MACINTOSH.C
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
*/

#include <exception.h>

#include "macintosh_cseries.h"
#include "my32bqd.h"

#include "ISp_Support.h" /* BT: Added April 16, 2000 for Input Sprockets Support */

#include "macintosh_network.h" /* For NetDDPOpen() */

// LP addition: local-event management:
#include "LocalEvents.h"

// LP addition: loading of XML files from resource fork
#include "XML_ResourceFork.h"


#define kMINIMUM_NETWORK_HEAP (3*MEG)

#ifdef PERFORMANCE
#include <Perf.h>
#endif

#ifdef env68k
#pragma segment shell
#endif

extern void *level_transition_malloc(size_t size);

/* ---------- constants */
#define alrtQUIT_FOR_SURE 138

#define kMINIMUM_MUSIC_HEAP (4*MEG)
#define kMINIMUM_SOUND_HEAP (3*MEG)

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

// Where the MacOS Toolbox (or some equivalent) has been inited
// Necessary to to indicate whether or not to create a dialog box.
static bool AppServicesInited = false;

/* ---------- externs that I couldn't fit into the #include heirarchy nicely */
extern bool load_and_start_game(FileSpecifier& File);
extern bool handle_open_replay(FileSpecifier& File);

/* ---------- private code */

static void verify_environment(void);
static void initialize_application_heap(void);

static void initialize_system_information(void);
static void initialize_core_events(void);
static void initialize_marathon_music_handler(void);
static pascal OSErr handle_open_document(AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_quit_application(AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_print_document(AppleEvent *event, AppleEvent *reply, long myRefCon);
static pascal OSErr handle_open_application(AppleEvent *event, AppleEvent *reply, long myRefCon);
static OSErr required_appleevent_check(AppleEvent *event);

static void marathon_dialog_header_proc(DialogPtr dialog, Rect *frame);

static void process_event(EventRecord *event);
// LP: "static" removed
void process_screen_click(EventRecord *event);
static void process_key(EventRecord *event, short key);
static void wait_for_highlevel_event(void);
static void handle_high_level_event(EventRecord *event);

void update_any_window(WindowPtr window, EventRecord *event);
void activate_any_window(WindowPtr window, EventRecord *event, bool active);

bool is_keypad(short keycode);

// LP addition: post an action from the local event queue in the app's MacOS event queue
static void PostOSEventFromLocal();


/* ---------- code */

void main(
	void)
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
			psprintf(ptemporary,"Unhandled exception: %s",e.what());
			ParamText(ptemporary,"\p0",NULL,NULL);
			InitCursor();
			Alert(128,NULL);
			exit(0);
		}
	}
	catch(...)
	{
		if (AppServicesInited)
		{
			psprintf(ptemporary,"Unknown exception");
			ParamText(ptemporary,"\p0",NULL,NULL);
			InitCursor();
			Alert(128,NULL);
			exit(0);
		}
	}
	exit(0);
}

/* ---------- private code */

static void initialize_application_heap(
	void)
{
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0); /* resume procedure ignored for multifinder and >=system 7.0 */
	InitCursor();

	long response;
	OSErr error;

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
	
	// The MacOS Toolbox has now been started up!
	AppServicesInited = true;
	
#ifdef DEBUG
	initialize_debugger(true);
#endif

	SetCursor(*GetCursor(watchCursor));

	qd.randSeed = TickCount();

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
	XML_ResourceFork XML_Loader;
	XML_Loader.CurrentElement = &RootParser;
	XML_Loader.ParseResourceSet('TEXT');
	
	initialize_preferences();
	GetDateTime(&player_preferences->last_time_ran);
	write_preferences();

	initialize_my_32bqd();
	verify_environment();

	if (FreeMem()>kMINIMUM_SOUND_HEAP) initialize_sound_manager(sound_preferences);

	initialize_marathon_music_handler();
	initialize_ISp(); /* BT: Added April 16, 2000 ISp: Initialize Input Sprockets */

	initialize_keyboard_controller();
	initialize_screen(&graphics_preferences->screen_mode);
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
	SetCursor(&qd.arrow);

	return;
}

void handle_game_key(
	EventRecord *event,
	short key)
{
	short _virtual;
	bool changed_screen_mode= false;
	bool changed_prefs= false;
	bool changed_volume= false;
	bool update_interface= false;
	
	_virtual = (event->message >> 8) & charCodeMask;
	
// LP change: implementing Benad's "cheats always on"
// #ifndef FINAL
	if (!game_is_networked && (event->modifiers & controlKey))
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
				changed_prefs= adjust_sound_volume_up(sound_preferences, _snd_adjust_volume);
				break;
			case ',': case '<': // sound volume down.
				changed_prefs= adjust_sound_volume_down(sound_preferences, _snd_adjust_volume);
				break;
			case kDELETE: // switch player view
				walk_player_list();
				render_screen(0);
				break;
			case '+': case '=':
				zoom_overhead_map_in();
				break;
			case '-': case '_':
				zoom_overhead_map_out();
				break;
			case '[': case '{':
				if(player_controlling_game())
				{
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
					scroll_inventory(1);
				}
				else
				{
					increment_replay_speed();
				}
				break;

			case '%':
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
					extern bool displaying_fps;
					
					displaying_fps= !displaying_fps;
				}
				break;
						
			default: // well, let's check the function keys then, using the keycodes.
				switch(_virtual)
				{
					// LP change: disabled these if OpenGL is active;
					// may want to either consolidate or eliminate these
					case kcF1:
						{
							// LP change: turned this into screen-size decrement
							if (graphics_preferences->screen_mode.size > 0)
							{
								graphics_preferences->screen_mode.size--;
								changed_screen_mode = changed_prefs = true;
							}
							// graphics_preferences->screen_mode.size = _full_screen;
							// changed_screen_mode = changed_prefs = true;
						}
						break;

					case kcF2:
						{
							// LP change: turned this into screen-size increment
							if (graphics_preferences->screen_mode.size < NUMBER_OF_VIEW_SIZES-1)
							{
								graphics_preferences->screen_mode.size++;
								changed_screen_mode = changed_prefs = true;
							}
							// if(graphics_preferences->screen_mode.size==_full_screen) update_interface= true;
							// graphics_preferences->screen_mode.size = _100_percent;
							// changed_screen_mode = changed_prefs = true;
						}
						break;

					case kcF3:
						if (!OGL_IsActive())
						{
							// Changed this to resolution toggle; no sense doing this in OpenGL mode
							graphics_preferences->screen_mode.high_resolution = !graphics_preferences->screen_mode.high_resolution;
							// if(graphics_preferences->screen_mode.size==_full_screen) update_interface= true;
							// graphics_preferences->screen_mode.size = _75_percent;
							changed_screen_mode = changed_prefs = true;
						}
						break;

					case kcF4:
						{
							// Reset OpenGL textures
							OGL_ResetTextures();
							// if(graphics_preferences->screen_mode.size==_full_screen) update_interface= true;
							// graphics_preferences->screen_mode.size = _50_percent;
							// changed_screen_mode = changed_prefs = true;
						}
						break;

/*
					case kcF5:
						if (graphics_preferences->screen_mode.acceleration != _valkyrie_acceleration)
						{
							graphics_preferences->screen_mode.high_resolution = !graphics_preferences->screen_mode.high_resolution;
							if (graphics_preferences->screen_mode.high_resolution) graphics_preferences->screen_mode.draw_every_other_line= false;
							changed_screen_mode = changed_prefs = true;
						}
						break;
#ifdef env68k
					case kcF6:
						if (!graphics_preferences->screen_mode.high_resolution && graphics_preferences->screen_mode.acceleration != _valkyrie_acceleration)
						{
							graphics_preferences->screen_mode.draw_every_other_line = !graphics_preferences->screen_mode.draw_every_other_line;
							changed_screen_mode = changed_prefs = true;
						}
						break;

					case kcF7:
						graphics_preferences->screen_mode.texture_floor = !graphics_preferences->screen_mode.texture_floor;
						changed_screen_mode = changed_prefs = true;
						break;

					case kcF8:
						graphics_preferences->screen_mode.texture_ceiling = !graphics_preferences->screen_mode.texture_ceiling;
						changed_screen_mode = changed_prefs = true;
						break;
#endif
					case kcF9:
						if(event->modifiers & shiftKey)
						{
							short keys[NUMBER_OF_KEYS];
				
							set_default_keys(keys, 0);
							set_keys(keys);
						}
						break;
						
					case kcF10:
						break;
*/
						
					case kcF11:
						if (graphics_preferences->screen_mode.gamma_level)
						{
							graphics_preferences->screen_mode.gamma_level-= 1;
							change_gamma_level(graphics_preferences->screen_mode.gamma_level);
							changed_prefs= true;
						}
						break;
						
					case kcF12:
						if (graphics_preferences->screen_mode.gamma_level<NUMBER_OF_GAMMA_LEVELS-1)
						{
							graphics_preferences->screen_mode.gamma_level+= 1;
							change_gamma_level(graphics_preferences->screen_mode.gamma_level);
							changed_prefs= true;
						}
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
						ChaseCam_SwitchSides();
						break;
					
					case kcF6:
						// Toggle the chase cam
						ChaseCam_SetActive(!ChaseCam_IsActive());
						break;
					
					case kcF7:
						// Toggle tunnel vision
						SetTunnelVision(!GetTunnelVision());
						break;
					
					case kcF8:
						// Toggle the crosshairs
						Crosshairs_SetActive(!Crosshairs_IsActive());
						break;
					
					case kcF10:
						// Toggle the position display
						{
							extern bool ShowPosition;
							ShowPosition = !ShowPosition;
						}
						break;
					
					case kcF14:
						// Reset OpenGL textures
						OGL_ResetTextures();
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
		// if(update_interface) draw_interface();
	}
	if (changed_prefs) write_preferences();
	
	return;
}

static void verify_environment(
	void)
{
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
	
	if (FreeMem()<2900000)
	{
		alert_user(fatalError, strERRORS, badMemory, FreeMem());
	}
	
	return;
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

		open_document_proc= NewAEEventHandlerProc(handle_open_document);
		quit_application_proc= NewAEEventHandlerProc(handle_quit_application);
		print_document_proc= NewAEEventHandlerProc(handle_print_document);
		open_application_proc= NewAEEventHandlerProc(handle_open_application);
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
	AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr err;
	AEDescList docList;

	(void) (reply, myRefCon)	;
	err= AEGetParamDesc(event, keyDirectObject, typeAEList, &docList);
	if(!err)
	{
		err= required_appleevent_check(event);
		if(!err)
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
						int Typecode = InputFile.GetType();
						if (Typecode == _typecode_scenario)
						{
							set_map_file(InputFile);
						}
						else if (Typecode == _typecode_savegame)
						{
							if(load_and_start_game(InputFile))
							{
								done= true;
							}
						}
						else if (Typecode == _typecode_film)
						{
							if(handle_open_replay(InputFile))
							{
								done= true;
							}
						}
						else if (Typecode == _typecode_physics)
						{
							set_physics_file(InputFile);
						}
						else if (Typecode == _typecode_shapes)
						{
							open_shapes_file(InputFile);
						}
						else if (Typecode == _typecode_sounds)
						{
					 		open_sound_file(InputFile);
						}
					}
				}
			}
		}
	}
	
	return err;
}

static pascal OSErr handle_quit_application(
	AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr err;
	
	(void)(reply, myRefCon);
	err= required_appleevent_check(event);
	if(err) return err;
	
	set_game_state(_quit_game);
	return noErr;
}

static pascal OSErr handle_print_document(
	AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	(void)(event, reply, myRefCon);

	return (errAEEventNotHandled);
}

static pascal OSErr handle_open_application(
	AppleEvent *event, 
	AppleEvent *reply, 
	long myRefCon)
{
	OSErr	err;

	(void)(reply, myRefCon);
	err=required_appleevent_check(event);
	if(err) return err;

	return err;
}

static OSErr required_appleevent_check(
	AppleEvent *event)
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
	long system_version, apple_events_present, processor_type;
	OSErr err;
	
	/* Allocate the system information structure.. */	
	system_information= (struct system_information_data *) 
		NewPtr(sizeof(struct system_information_data));
	assert(system_information);

	/* System Version */
	system_information->has_seven= false;
	err= Gestalt(gestaltSystemVersion, &system_version);
	if (!err)
	{
		if(system_version>=0x0700) system_information->has_seven= true;;
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
	if(system_information->has_seven && NetDDPOpen()==noErr && FreeMem()>kMINIMUM_NETWORK_HEAP)
	{
		system_information->appletalk_is_available= true;
	} else {
		system_information->appletalk_is_available= false;
	}

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
	
	return;
}

bool is_keypad(
	short keycode)
{
	return keycode >= 0x41 && keycode <= 0x5c;
}

bool networking_available(
	void)
{
	return system_information->appletalk_is_available;
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
	long refCon= GetWRefCon(dialog);
	
	if (refCon>=FIRST_DIALOG_REFCON && refCon<=LAST_DIALOG_REFCON)
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
		
	return;
}

static void initialize_marathon_music_handler(
	void)
{
	FileSpecifier SongFile;
	// FSSpec music_file_spec;
	OSErr error;
	bool initialized= false;
	
	SongFile.SetToApp();
	SongFile.SetName(getcstr(temporary, strFILENAMES, filenameMUSIC),NONE);
	if (SongFile.Exists())
	// error= get_file_spec(&music_file_spec, strFILENAMES, filenameMUSIC, strPATHS);
	// if (!error)
	{
		// bool is_folder, was_aliased;
	
		// ResolveAliasFile(&music_file_spec, true, &is_folder, &was_aliased);
		// initialized= initialize_music_handler((FileDesc *) &music_file_spec);
		initialized= initialize_music_handler(SongFile);
	}
	
	return;
}

static void main_event_loop(
	void)
{
	wait_for_highlevel_event();
	
	while(get_game_state()!=_quit_game)
	{
		bool use_waitnext;
		
		// LP addition: turn a local event into a MacOS event
		PostOSEventFromLocal();
		
		if(try_for_event(&use_waitnext))
		{
			EventRecord event;
			bool got_event= false;
		
			if(use_waitnext)
			{
				got_event= WaitNextEvent(everyEvent, &event, 2, (RgnHandle) NULL);
			}
			else
			{
				got_event= GetOSEvent(everyEvent, &event);
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

	return;
}

/* Can't be static because the general_filter_proc calls this */
void update_any_window(
	WindowPtr window,
	EventRecord *event)
{
	GrafPtr old_port;

	GetPort(&old_port);
	SetPort(window);
	BeginUpdate(window);

	if(window==screen_window)
	{
		update_game_window(window, event);
	}
	EndUpdate(window);
	SetPort(old_port);
	
	return;
}

/* Can't be static because the general_filter_proc calls this */
void activate_any_window(
	WindowPtr window,
	EventRecord *event,
	bool active)
{
	if(window==screen_window)
	{
		activate_screen_window(window, event, active);
	}
	
	return;
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
				case inSysWindow: /* DAs and the menu bar can blow me */
				case inMenuBar:
					// LP change:
					assert(false);
					// halt();
					break;
					
				case inContent:
					process_screen_click(event);
					break;
					
				default:
					// LP change:
					assert(false);
					// halt();
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
					if (event->message&resumeFlag)
					{
						/* resume */
						SetCursor(&qd.arrow);
					}
					else
					{
						/* suspend */
					}
					break;
			}
			break;
			
		default:
			break;
	}
		
	return;
}

// LP: "static" removed
void process_screen_click(
	EventRecord *event)
{
	GrafPtr old_port;
	Point where= event->where;
	bool cheatkeys_down;
	
	GetPort(&old_port);
	SetPort(screen_window);
	
	GlobalToLocal(&where);
	cheatkeys_down= has_cheat_modifiers(event);
	portable_process_screen_click(where.h, where.v, cheatkeys_down);

	SetPort(old_port);	
	
	return;
}

static void process_key(
	EventRecord *event,
	short key)
{
	process_game_key(event, key);
	return;
}


// Index of which local event to look at; the event poster will cycle through
// the possible events, looking for one that is present.
static int LocalEventIndex = 0;

// LP addition: post any events found in the local event queue
void PostOSEventFromLocal()
{
	int SavedLocalEventIndex = LocalEventIndex;
	for (int i=0; i<32; i++)
	{
		// Quick remainder after division by 32 (2^5 or 0x20)
		LocalEventIndex = (SavedLocalEventIndex + i) & 0x1f;
		
		unsigned long LocalEvent = 1 << ((unsigned long)LocalEventIndex);
		if (GetLocalEvent(LocalEvent))
		{
			// Compose an appropriate event message:
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
			
			// Points to the queued event
			EvQElPtr QueueEntry;

			// Post that event in the app's MacOS event queue
			PPostEvent(keyDown,Features.Message,&QueueEntry);
			
			// Edit the queue entry's modifiers:
			QueueEntry->evtQModifiers = Features.Modifiers;
			
			// Set up for next one and quit:
			LocalEventIndex = (LocalEventIndex + 1) & 0x1f;
			return;
		}
	}
	
	// Reset this value
	LocalEventIndex = SavedLocalEventIndex;
}


/* ----------- PRIVATE CODE */
/* Should be in shell.h and shell.c */
bool machine_has_quicktime(void) 
{
	return HasQuicktime;
}

