/*

	interface_macintosh.c
	Thursday, September 28, 1995 7:32:09 PM- rdm created.

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 19, 2000 (Loren Petrich):
	Won't do screen reinit if OpenGL has been selected
*/

#include "macintosh_cseries.h"
#include <Movies.h>
#include <FixMath.h>

#include <string.h>

#include "map.h"
#include "shell.h"
#include "interface.h"
#include "player.h"

#include "macintosh_network.h"
#include "network_sound.h"

#include "screen_drawing.h"
#include "mysound.h"
#include "preferences.h"
#include "fades.h"
#include "game_window.h"
#include "game_errors.h"
#include "screen.h"

// #include "portable_files.h"
#include "images.h"

#include "interface_menus.h"

#ifdef env68k
	#pragma segment macintosh_
#endif

enum { /* Cheat level dialog */
	dlogLEVEL_NUMBER= 137,
	iLEVEL_NUMBER= 3
};

#define TICKS_BETWEEN_XNEXTEVENT_CALLS 10
#define MAXIMUM_CONSECUTIVE_GETOSEVENT_CALLS 12 // 2 seconds

/* -------- local prototypes */
static void network_speaker_proc(void *buffer, short size, short player_index);
static boolean machine_has_quicktime(void);
static void draw_picture_into_gworld(GWorldPtr gworld, PicHandle picture);

/* -------- code */
void do_preferences(
	void)
{
	struct screen_mode_data mode= graphics_preferences->screen_mode;
	GDSpec old_spec= graphics_preferences->device_spec;

	handle_preferences();
	if (!EqualGDSpec(&graphics_preferences->device_spec, &old_spec) ||
		mode.bit_depth != graphics_preferences->screen_mode.bit_depth ||
		// LP change: no need for reinit
		false)
		// mode.acceleration != graphics_preferences->screen_mode.acceleration)
	{
		paint_window_black();
		initialize_screen(&graphics_preferences->screen_mode);

		/* Re fade in, so that we get the proper colortable loaded.. */
		display_main_menu();
	}
	else if (memcmp(&mode, &graphics_preferences->screen_mode, sizeof(struct screen_mode_data)))
	{
		change_screen_mode(&graphics_preferences->screen_mode, FALSE);
	}
	
	return;
}

short get_level_number_from_user(
	void)
{
	short index, item_hit, level_number, maximum_level_number;
	DialogPtr dialog;
	struct entry_point entry;
	boolean done= FALSE;
	
	index = 0; maximum_level_number= 0;
	while (get_indexed_entry_point(&entry, &index, _single_player_entry_point | _multiplayer_carnage_entry_point | _multiplayer_cooperative_entry_point)) maximum_level_number++;

	dialog = myGetNewDialog(dlogLEVEL_NUMBER, NULL, (WindowPtr) -1, 0);
	assert(dialog);

	psprintf(ptemporary, "%d", maximum_level_number); 
	ParamText(ptemporary, "\p", "\p", "\p");
	SelectDialogItemText(dialog, iLEVEL_NUMBER, 0, SHORT_MAX);

	while(!done)
	{
		do
		{
			ModalDialog(get_general_filter_upp(), &item_hit);
		} while (item_hit>iCANCEL);

		level_number= extract_number_from_text_item(dialog, iLEVEL_NUMBER);

		switch(item_hit)
		{
			case iOK:
				if(level_number<=0 || level_number>maximum_level_number)
				{
					SelectDialogItemText(dialog, iLEVEL_NUMBER, 0, SHORT_MAX);
					SysBeep(-1);
				} else {
					level_number-= 1; /* Make it zero based */
					done= TRUE;
				}
				break;
				
			case iCANCEL:
				done= TRUE;
				level_number= NONE;
				break;
				
			default:
				// LP change:
				assert(false);
				// halt();
				break;
		}
	}
	DisposeDialog(dialog);

	return level_number;
}

void toggle_menus(
	boolean game_started)
{
	MenuHandle menu_interface, menu_game;
	short item;
	static boolean first_time= TRUE;

	/* First time, we insert the menus... */	
	if(first_time)
	{
		MenuHandle menu;

		/* Insert our fake menu */
		menu= GetMenu(mFakeEmptyMenu);
		assert(menu);
		InsertMenu(menu, 0);

		menu= GetMenu(mInterface);
		assert(menu);
		InsertMenu(menu, -1);

		menu= GetMenu(mGame);
		assert(menu);
		InsertMenu(menu, -1);

		first_time= FALSE;
	}
	
	menu_interface= GetMenuHandle(mInterface);
	menu_game= GetMenuHandle(mGame);
	assert(menu_game && menu_interface);

	if(game_started)
	{
		DisableItem(menu_interface, 0);
		EnableItem(menu_game, 0);
	} else {
		DisableItem(menu_game, 0);
		EnableItem(menu_interface, 0);
	}

	/* Handle enabling/disabling everything.. */
	for(item= iNewGame; item<=iCenterButton; ++item)
	{
		if(enabled_item(item))
		{
			EnableItem(menu_interface, item);
		} else {
			DisableItem(menu_interface, item);
		}
	}

	return;
}

void update_game_window(
	WindowPtr window,
	EventRecord *event)
{
	switch(get_game_state())
	{
		case _game_in_progress:
			assert(event);
			update_screen_window(window, event);
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
			
		case _quit_game:
		case _close_game:
		case _switch_demo:
		case _change_level:
		case _revert_game:
		case _begin_display_of_epilogue:
		case _displaying_network_game_dialogs:
			break;
	
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}
	
	return;
}
	
void process_game_key(
	EventRecord *event,
	short key)
{
	switch(get_game_state())
	{
		case _game_in_progress:
			if(event->modifiers&cmdKey && event->what==keyDown)
			{
				long menu_key= MenuKey(key);
				short menuID, menuItem;

				menuID= menu_key>>16;
				menuItem= menu_key & 0x0000ffff;
				if(menuID)
				{
					assert(menuID==mGame);

					do_menu_item_command(menuID, menuItem, 
						has_cheat_modifiers(event));
				} else {
					handle_game_key(event, key);
				}
			} else {
				handle_game_key(event, key);
			}
			break;
			
		case _display_intro_screens:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _display_credits:
		case _display_quit_screens:
			/* Force the state change next time through.. */
			if(interface_fade_finished())
			{
				force_game_state_change();
			} else {
				stop_interface_fade();
			}
			break;

		case _display_intro_screens_for_demo:
			/* Get out of user mode. */
			stop_interface_fade();
			display_main_menu();
			break;
			
		case _quit_game:
		case _close_game:
		case _revert_game:
		case _switch_demo:	
		case _change_level:
		case _begin_display_of_epilogue:
		case _displaying_network_game_dialogs:
			break;
	
		case _display_main_menu:
			if(event->modifiers&cmdKey && event->what==keyDown)
			{
				long menu_key= MenuKey(key);
				short menuID, menuItem;

				menuID= menu_key>>16;
				menuItem= menu_key & 0x0000ffff;

				/* If it was a menu key. */
				if(menuID) 
				{
					GrafPtr old_port;
				
					assert(menuID==mInterface);

					stop_interface_fade();

					/* Change to this port... */
					GetPort(&old_port);
					SetPort(screen_window);
					draw_menu_button_for_command(menuItem);
					SetPort(old_port);
					
					do_menu_item_command(mInterface, menuItem, 
						has_cheat_modifiers(event));
				}
			} else {
				/* If they are fading in, stop the interface fade */
				if(!interface_fade_finished())
				{
					stop_interface_fade();
				}
			}
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
			break;
	}

	return;
}	

boolean try_for_event(
	boolean *use_waitnext)
{
	boolean try_for_event= FALSE;
	static long last_xnextevent_call= 0;
	static short consecutive_getosevent_calls= 0;
	
	switch(get_game_state())
	{
		case _game_in_progress:
		case _change_level:
			if (TickCount()-last_xnextevent_call>=TICKS_BETWEEN_XNEXTEVENT_CALLS)
			{
				try_for_event= TRUE;

				if (suppress_background_events() && 
					EmptyRgn(((WindowPeek)screen_window)->updateRgn)) 
					// && consecutive_getosevent_calls<MAXIMUM_CONSECUTIVE_GETOSEVENT_CALLS)
				{
					*use_waitnext= FALSE;
					consecutive_getosevent_calls+= 1;
				} else {
					*use_waitnext= TRUE;
					consecutive_getosevent_calls= 0;
				}
		
				last_xnextevent_call= TickCount();
			}
			break;

		case _display_intro_screens:
		case _display_main_menu:
		case _display_chapter_heading:
		case _display_prologue:
		case _display_epilogue:
		case _begin_display_of_epilogue:
		case _display_credits:
		case _display_intro_screens_for_demo:
		case _display_quit_screens:
		case _displaying_network_game_dialogs:
			*use_waitnext= interface_fade_finished();
			try_for_event= TRUE;
			break;
			
		case _quit_game:
		case _close_game:
		case _switch_demo:
		case _revert_game:
			*use_waitnext= TRUE;
			try_for_event= TRUE;
			break;
			
		default:
			vhalt(csprintf(temporary, "What the hell is state: %d?", get_game_state()));
			break;
	}

	if(try_for_event)
	{
		global_idle_proc();
	}
	
	return try_for_event;
}

void install_network_microphone(
	void)
{
	short id;

	open_network_speaker(NETWORK_SOUND_CHUNK_BUFFER_SIZE, 2);
	id = NetAddDistributionFunction(network_speaker_proc, TRUE);
	open_network_microphone(id);
}

void remove_network_microphone(
	void)
{
	close_network_speaker();
	close_network_microphone();
}

static void network_speaker_proc(
	void *buffer, 
	short size, 
	short player_index)
{
	(void)(player_index);
	queue_network_speaker_data((byte *) buffer, size);
}

void exit_networking(
	void)
{
	NetExit();
}

boolean has_cheat_modifiers(
	EventRecord *event)
{
	boolean cheat= FALSE;

	if ((event->modifiers&cmdKey) && (event->modifiers&optionKey)
		&& !(event->modifiers&shiftKey) && !(event->modifiers&controlKey))
	{
		cheat= TRUE;
	}
	
	return cheat;
}

#ifdef OBSOLETE
static GWorldPtr main_menu_pressed= NULL;
static GWorldPtr main_menu_unpressed= NULL;

extern short interface_bit_depth;

/* Create the two gworlds */
void load_main_menu_buffers(
	short base_id)
{
	OSErr error;
	Rect bounds;
	PicHandle picture;
	
	assert(main_menu_unpressed==NULL && main_menu_pressed==NULL);

	/* Determine the pictures bounds... */	
	picture= get_picture_resource_from_images(base_id);
	assert(picture);
	bounds= (*picture)->picFrame;
	OffsetRect(&bounds, -bounds.left, -bounds.top);

	error= NewGWorld(&main_menu_unpressed, interface_bit_depth, &bounds, 
		NULL, world_device, noNewDevice);
	assert(!error && main_menu_unpressed);

	/* Draw it and release it.. */
	draw_picture_into_gworld(main_menu_unpressed, picture);

	/* Create the other gworld */	
	error= NewGWorld(&main_menu_pressed, interface_bit_depth, &bounds, 
		(CTabHandle) NULL, world_device, noNewDevice);
	assert(!error && main_menu_pressed);

	picture= get_picture_resource_from_images(base_id+1);
	assert(picture);
	draw_picture_into_gworld(main_menu_pressed, picture);
		
	return;
}

boolean main_menu_buffers_loaded(
	void)
{
	return (main_menu_unpressed!=NULL);
}

void main_menu_bit_depth_changed(
	short base_id)
{
	free_main_menu_buffers();
	load_main_menu_buffers(base_id);
	
	return;
}

void free_main_menu_buffers(
	void)
{
	assert(main_menu_unpressed && main_menu_pressed);
	DisposeGWorld(main_menu_unpressed);
	DisposeGWorld(main_menu_pressed);
	
	main_menu_pressed= main_menu_unpressed= NULL;
	
	return;
}

void draw_main_menu(
	void)
{
	PixMapHandle pixmap;
	boolean locked;
	GrafPtr old_port;
	Rect source_bounds, dest_bounds;
	
	assert(main_menu_pressed && main_menu_unpressed);
	
	GetPort(&old_port);
	SetPort(screen_window);

	source_bounds= dest_bounds= main_menu_pressed->portRect;
	AdjustRect(&screen_window->portRect, &dest_bounds, &dest_bounds, centerRect);
	OffsetRect(&dest_bounds, dest_bounds.left<0 ? -dest_bounds.left : 0, dest_bounds.top<0 ? -dest_bounds.top : 0);
	
	pixmap= GetGWorldPixMap(main_menu_unpressed);
	assert(pixmap);
	locked= LockPixels(pixmap);
	assert(locked);
	CopyBits((BitMapPtr)*main_menu_unpressed->portPixMap, &screen_window->portBits,
		&source_bounds, &dest_bounds, srcCopy, (RgnHandle) NULL);
	UnlockPixels(pixmap);

	SetPort(old_port);

	return;	
}

void draw_menu_button(
	short index, 
	boolean pressed)
{
	screen_rectangle *screen_rect= get_interface_rectangle(index);
	PixMapHandle pixmap;
	boolean locked;
	GrafPtr old_port;
	GWorldPtr source_world;

	if(!main_menu_buffers_loaded())
	{
		load_main_menu_buffers(1100); //¥¥¥¥
	}
	
	assert(main_menu_pressed && main_menu_unpressed);
	
	if(pressed)
	{
		source_world= main_menu_pressed;
	} else {
		source_world= main_menu_unpressed;
	}
	
	GetPort(&old_port);
	SetPort(screen_window);
	
	pixmap= GetGWorldPixMap(source_world);
	locked= LockPixels(pixmap);
	assert(locked);
	CopyBits((BitMapPtr)*source_world->portPixMap, &screen_window->portBits,
		(Rect *) screen_rect, (Rect *) screen_rect, srcCopy, (RgnHandle) NULL);
	UnlockPixels(pixmap);

	SetPort(old_port);

	return;
}

static void draw_picture_into_gworld(
	GWorldPtr gworld,
	PicHandle picture)
{
	if (picture)
	{
		Rect bounds;
		GWorldPtr old_world;
		GDHandle old_device;
		PixMapHandle pixmap;
		boolean locked;
	
		GetGWorld(&old_world, &old_device);
		SetGWorld(gworld, NULL);
		pixmap= GetGWorldPixMap(gworld);
		assert(pixmap);
		locked= LockPixels(pixmap);
		assert(locked);
		
		bounds= (*picture)->picFrame;
		AdjustRect(&gworld->portRect, &bounds, &bounds, centerRect);
		OffsetRect(&bounds, bounds.left<0 ? -bounds.left : 0, bounds.top<0 ? -bounds.top : 0);

		HLock((Handle) picture);
		DrawPicture(picture, &bounds);
		HUnlock((Handle) picture);
		
		UnlockPixels(pixmap);
		SetGWorld(old_world, old_device);
				
		ReleaseResource((Handle) picture);
	}

	return;
}
#endif

/* ------------ these are all functions that are simple to write for various */
/* --- machines */
void hide_cursor(
	void)
{
	HideCursor();

	return;
}

void show_cursor(
	void)
{
	InitCursor(); // why worry about balancing?

	return;
}

boolean mouse_still_down(
	void)
{
	return StillDown();
}

void get_mouse_position(
	short *x,
	short *y)
{
	Point p;
	
	GetMouse(&p);
	*x= p.h; *y= p.v;
	
	return;
}

void set_drawing_clip_rectangle(
	short top,
	short left,
	short bottom,
	short right)
{
	Rect rectangle;
	
	SetRect(&rectangle, left, top, right, bottom);
	ClipRect(&rectangle);

	return;
}

void show_movie(
	short index)
{
	if(index==0) /* Only one valid.. */
	{
		if(machine_has_quicktime())
		{
			FSSpec movie_spec; // <===== fill in based on index!
			OSErr err= noErr;
		
			err= get_file_spec(&movie_spec, strFILENAMES, filenameMOVIE, strPATHS);
			if(!err)
			{
				err= EnterMovies();
				if(!err)
				{
					Movie movie;
					short resRefNum;
		
					if(!OpenMovieFile(&movie_spec, &resRefNum, fsRdPerm)) 
					{
						if(!NewMovieFromFile(&movie, resRefNum, nil, nil, newMovieActive, nil )) 
						{
							Rect dispBounds;
							boolean aborted= FALSE;
				
							/* Find movie bounds and zero base it.... */
							GetMovieBox(movie, &dispBounds);
							OffsetRect(&dispBounds, -dispBounds.left, -dispBounds.top);
							dispBounds.right= RECTANGLE_WIDTH(&dispBounds)*2;
							dispBounds.bottom= RECTANGLE_HEIGHT(&dispBounds)*2;
							
							/* ‚enter... */
							AdjustRect(&screen_window->portRect, &dispBounds, &dispBounds, centerRect);
							OffsetRect(&dispBounds, dispBounds.left<0 ? -dispBounds.left : 0, dispBounds.top<0 ? -dispBounds.top : 0);
							SetMovieBox(movie, &dispBounds);
							
							/* Set to the screen_window.. */	
							SetMovieGWorld(movie, (CGrafPtr)  screen_window, nil);
							
							// After setup, play the movie
							GoToBeginningOfMovie(movie); // Rewind
							
							/* Should the preroll movie use FIXED_ONE as second parameter */
							/*  (or whatever the mac equivalent (Fixed) is) */
							PrerollMovie(movie, (TimeValue) 0, (Fixed) fixed1); // Preload portions
//							SetMovieActive(movie, true); // Set movie active for servicing
							
							// Maybe I should delay after prerolling the movie, since the
							// preroll stuff might be asynchrounous?
							StartMovie(movie);
				
							while (!IsMovieDone(movie) && !aborted) 
							{
								EventRecord event;
							
								if(WaitNextEvent(keyDownMask|autoKeyMask|mDownMask, &event, 2, nil))
								{
									switch(event.what)
									{
										case nullEvent:
											break;
											
										case mouseDown:
										case keyDown:
										case autoKey:
											aborted= TRUE;
											StopMovie(movie);
											break;
									}
								}
								global_idle_proc();
					
								/* Give the movie time */
								MoviesTask(movie, DoTheRightThing);
							}

							/* Paint the window black. */
							paint_window_black();
		
							// Dispose of storage, and return
							DisposeMovie(movie); // Movie
						}
		
						CloseMovieFile(resRefNum); // reference to movie file
					}
					
					ExitMovies();
				}
			}
		}
	}

	return;
}


/* ----------- PRIVATE CODE */
/* Should be in shell.h and shell.c */
static boolean machine_has_quicktime(
	void) 
{
	long response;
	boolean has_quicktime= FALSE;
	OSErr error;

	error= Gestalt(gestaltQuickTime, &response);
	if(!error) 
	{
		/* No error finding QuickTime.  Check for ICM so we can use Standard Preview */
		error= Gestalt(gestaltCompressionMgr, &response);
		if(!error) 
		{
			has_quicktime= TRUE;
		}
	}
	
	return has_quicktime;
}
