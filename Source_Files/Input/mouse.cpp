/*
MOUSE.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

Tuesday, January 17, 1995 2:51:59 PM  (Jason')

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Nov 17, 2000 (Loren Petrich):
	Added some handling of absent mice and out-of-range input types;
	this may help avert some crashes on certain PowerBook models

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for ApplicationServices.h
	Disabled all the trap and device based code for the mouse
	Added in Carbon code for mouse location and warping

Jan 29, 2002 (Br'fin (Jeremy Parsons)):
	Rewrote Carbon mouse to report mouse movement based on receiving MouseMoved events

Feb 13, 2002 (Br'fin (Jeremy Parsons)):
	Rewrote Carbon mouse again, to catch mouse movement in the main thread and pass those
	values to the input thread
	
Mar 19, 2002 (Br'fin (Jeremy Parsons)):
	Rewrote Carbon mouse again. Refining based on MacQuakeGL mouse
	Enabling 2nd mouse button as second trigger
	Enabling scrollwheel as weapon selector

May 16, 2002 (Woody Zenfell):
	Configurable mouse sensitivity

May 20, 2003 (Woody Zenfell):
	Reenabling mouse sensitivity; also respecting mouse Y-axis inversion.
*/

/* marathon includes */
#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
#include "player.h"     // for get_absolute_pitch_range()
#include "mouse.h"
#include "shell.h"
#include <math.h>
#if defined(TARGET_API_MAC_CARBON)
#include "interface.h"
#endif
#include "preferences.h"
#include "Logging.h"

/* macintosh includes */
#if defined(EXPLICIT_CARBON_HEADER)
    #include <ApplicationServices/ApplicationServices.h>
#else
/*
#include <CursorDevices.h>
#include <Traps.h>
*/
#endif

#ifdef env68k
#pragma segment input
#endif

/* constants */
#define _CursorADBDispatch 0xaadb
#define CENTER_MOUSE_X      320
#define CENTER_MOUSE_Y      240

static void get_mouse_location(Point *where);
static void set_mouse_location(Point where);
static pascal OSStatus CEvtHandleApplicationMouseEvents (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);

/* ---------- globals */

static _fixed snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity;
static bool snapshot_button_state[MAX_BUTTONS];
static int snapshot_delta_scrollwheel;
static MPCriticalRegionID CE_MouseLock = NULL;
static int _CE_delta_x, _CE_delta_y, _CE_delta_scrollwheel;
static EventHandlerUPP _CEMouseTrackerUPP = NULL;
static EventHandlerRef _CEMouseTracker = NULL;

// For getting mouse deltas while doing Classic-mode fallback:
static Point PrevPosition;

#ifndef __MACH__

// Cribbed from CGDirectDisplay.h

typedef void
(*CGGetLastMouseDelta_Type)(
  CGMouseDelta *  deltaX,
  CGMouseDelta *  deltaY);

typedef CGDisplayErr
(*CGWarpMouseCursorPosition_Type)
(CGPoint Point);

CGGetLastMouseDelta_Type CGGetLastMouseDelta_Ptr = NULL;

CGWarpMouseCursorPosition_Type CGWarpMouseCursorPosition_Ptr = NULL;

static void LoadCGMouseFunctions();

#include <CGDirectDisplay.h>

#endif

/* ---------- code */

void enter_mouse(
	short type)
{
	(void) (type);

	snapshot_delta_yaw= snapshot_delta_pitch= snapshot_delta_velocity= false;
	for(int i = 0; i < MAX_BUTTONS; i++)
		snapshot_button_state[i] = false;
	snapshot_delta_scrollwheel = 0;
	
	// JTP: Install our Carbon Event mouse handler and create the critical region for safe value sharing
	static EventTypeSpec mouseEvents[] = {
		{kEventClassMouse, kEventMouseDown},
		{kEventClassMouse, kEventMouseUp},
		{kEventClassMouse, kEventMouseWheelMoved},
		{kEventClassMouse, kEventMouseMoved},
		{kEventClassMouse, kEventMouseDragged}
	};
	_CEMouseTrackerUPP = NewEventHandlerUPP(CEvtHandleApplicationMouseEvents);
	InstallApplicationEventHandler (_CEMouseTrackerUPP,
		5, mouseEvents, NULL, &_CEMouseTracker);
	MPCreateCriticalRegion(&CE_MouseLock);
	
	// Fallback in case the lock cannot be allocated (possible with Classic):
	if (!CE_MouseLock)
		GetGlobalMouse(&PrevPosition);
	
#ifndef __MACH__
	LoadCGMouseFunctions();
#endif
}

void test_mouse(
	short type,
	uint32 *action_flags,
	_fixed *delta_yaw,
	_fixed *delta_pitch,
	_fixed *delta_velocity)
{
	(void) (type);

	for (int i = 0; i < MAX_BUTTONS; i++)
		if (snapshot_button_state[i])
			switch (input_preferences->mouse_button_actions[i])
			{
				case _mouse_button_fires_left_trigger:
					*action_flags|= _left_trigger_state;
					break;
				case _mouse_button_fires_right_trigger:
					*action_flags|= _right_trigger_state;
					break;
				case _mouse_button_does_nothing:
				default:
					break; // nothing
			}

	if (snapshot_delta_scrollwheel > 0) *action_flags|= _cycle_weapons_forward;
	else if (snapshot_delta_scrollwheel < 0) *action_flags|= _cycle_weapons_backward;
	snapshot_delta_scrollwheel = 0;

	*delta_yaw= snapshot_delta_yaw;
	*delta_pitch= snapshot_delta_pitch;
	*delta_velocity= snapshot_delta_velocity;
}

void exit_mouse(
	short type)
{
	(void) (type);

	RemoveEventHandler(_CEMouseTracker);
	_CEMouseTracker = NULL;
	DisposeEventHandlerUPP(_CEMouseTrackerUPP);
	_CEMouseTrackerUPP = NULL;
	MPDeleteCriticalRegion(CE_MouseLock);
	CE_MouseLock = NULL;
}

/* 1200 pixels per second is the highest possible mouse velocity */
#define MAXIMUM_MOUSE_VELOCITY (1200/MACINTOSH_TICKS_PER_SECOND)

//#define MAXIMUM_MOUSE_VELOCITY ((float)1500/MACINTOSH_TICKS_PER_SECOND)

/* take a snapshot of the current mouse state */
void mouse_idle(
	short type)
{
	Point where;
	Point center;
	static bool first_run = true;
	static int32 last_tick_count;
	int32 tick_count= TickCount();
	int32 ticks_elapsed= tick_count-last_tick_count;
	
	if(first_run)
	{
		ticks_elapsed = 0;
		first_run = false;
	}
	
	get_mouse_location(&where);

	center.h= CENTER_MOUSE_X, center.v= CENTER_MOUSE_Y;
	set_mouse_location(center);
	
	if (ticks_elapsed)
	{
		/* calculate axis deltas */
		_fixed vx= INTEGER_TO_FIXED(where.h-center.h)/(ticks_elapsed*MAXIMUM_MOUSE_VELOCITY);
		_fixed vy= - INTEGER_TO_FIXED(where.v-center.v)/(ticks_elapsed*MAXIMUM_MOUSE_VELOCITY);

		// ZZZ: mouse inversion
		if (TEST_FLAG(input_preferences->modifiers, _inputmod_invert_mouse))
			vy *= -1;

		// LP: modified for doing each axis separately;
		// ZZZ: scale input by sensitivity
		if (input_preferences->sens_horizontal != FIXED_ONE)
			vx = _fixed((float(input_preferences->sens_horizontal)*vx)/float(FIXED_ONE));
		if (input_preferences->sens_vertical != FIXED_ONE)
			vy = _fixed((float(input_preferences->sens_vertical)*vy)/float(FIXED_ONE));

		if(input_preferences->mouse_acceleration) {
			/* pin and do nonlinearity */
			vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1, vx*= (vx<0) ? -vx : vx, vx>>= 14;
			vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1, vy*= (vy<0) ? -vy : vy, vy>>= 14;
		}
		else {
			/* pin and do NOT do nonlinearity */
			vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1;
			vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1;
		}

		snapshot_delta_yaw= vx;
		
		switch (type)
		{
			case _mouse_yaw_pitch:
				snapshot_delta_pitch= vy, snapshot_delta_velocity= 0;
				break;
			case _mouse_yaw_velocity:
				snapshot_delta_velocity= vy, snapshot_delta_pitch= 0;
				break;
			
			default:
				// LP change: put in some (presumably) reasonable behavior
				// for some unrecognized type.
				snapshot_delta_pitch= 0, snapshot_delta_velocity= 0;
				break;
		}
		
		// It works for the primary...
		snapshot_button_state[0] = Button();
		
		if(MPEnterCriticalRegion(CE_MouseLock, kDurationImmediate) == noErr)
		{
			snapshot_delta_scrollwheel = _CE_delta_scrollwheel;
			_CE_delta_scrollwheel = 0;
			MPExitCriticalRegion(CE_MouseLock);
		}

		last_tick_count= tick_count;
		
//		dprintf("%08x %08x %08x;g;", snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity);
	}
}

/* ---------- private code */

// unused 
static void get_mouse_location(
	Point *where)
{

	if(MPEnterCriticalRegion(CE_MouseLock, kDurationImmediate) == noErr)
	{
		where->h = CENTER_MOUSE_X + _CE_delta_x;
		where->v = CENTER_MOUSE_Y + _CE_delta_y;
		_CE_delta_x = 0;
		_CE_delta_y = 0;
		MPExitCriticalRegion(CE_MouseLock);
	}
	else
	{
		// where->h = CENTER_MOUSE_X;
		// where->v = CENTER_MOUSE_Y;
		// Don't use GetMouse() here -- it does global-to-local,
		// which makes the mouse position unstable
		// Fallback in case the lock cannot be allocated (possible with Classic):
		Point Position;
		GetGlobalMouse(&Position);
		
		// OSX-native code tracks deltas; this code will also
		short dh = Position.h - PrevPosition.h;
		short dv = Position.v - PrevPosition.v;
				
		where->h = CENTER_MOUSE_X + dh;
		where->v = CENTER_MOUSE_Y + dv;
		
		// Get ready for next round
		PrevPosition = Position;
	}
}

static void set_mouse_location(
	Point where)
{
	#ifdef __MACH__
	CGWarpMouseCursorPosition(CGPointMake(where.h, where.v));
	#else
	if (CGWarpMouseCursorPosition_Ptr)
		CGWarpMouseCursorPosition_Ptr(CGPointMake(where.h, where.v));
	#endif
}

#if defined(TARGET_API_MAC_CARBON)
// Catch mouse events in the main thread, store the deltas for the input thread
// JTP: Cribbing from MacQuakeGL
pascal OSStatus CEvtHandleApplicationMouseEvents (EventHandlerCallRef nextHandler,
    EventRef theEvent,
    void* userData)
{
	UInt32 			event_kind;
	UInt32 			event_class;
	EventMouseButton	which_mouse_button;
	SInt32			scroll_wheel_delta;
	OSStatus 		err = eventNotHandledErr;
	CGMouseDelta 		CGx, CGy;
	short			game_state;
//	extern boolean		background;

	event_kind = GetEventKind(theEvent);
	event_class = GetEventClass(theEvent);
	
	if (nextHandler)
		err = CallNextEventHandler (nextHandler, theEvent);

	// Just in case this is still being called when it's supposed to be absent...
	if (!_CEMouseTracker) return err;

	if (err == noErr || err == eventNotHandledErr)
	{
		if (event_class == kEventClassMouse)
		{
			// If we're not in the game, let something else handle mouse clicks
			game_state = get_game_state();
			if(game_state == _display_chapter_heading)
				return eventNotHandledErr;
			if(game_state != _game_in_progress)
			{
				extern void process_screen_click(EventRecord *event);
			
				extern WindowPtr screen_window;
				if(FrontWindow() == screen_window)
				{
					if(event_kind == kEventMouseDown)
					{
						EventRecord eventRec;
						ConvertEventRefToEventRecord(theEvent, &eventRec);
						process_screen_click(&eventRec);
					}
					return noErr;
				}
				
				return err;
			}
			
			switch (event_kind)
			{
				// Carbon tells us when the mouse moves, but CoreGraphics gets the delta
				case kEventMouseMoved:
				case kEventMouseDragged:
					#ifdef __MACH__
					CGGetLastMouseDelta(&CGx, &CGy);
					if((err = MPEnterCriticalRegion(CE_MouseLock, kDurationForever)) == noErr)
					{
						_CE_delta_x += CGx;
						_CE_delta_y += CGy;
						MPExitCriticalRegion(CE_MouseLock);
					}
					err = noErr;
					CGWarpMouseCursorPosition(CGPointMake(CENTER_MOUSE_X, CENTER_MOUSE_Y));
					#else
					// Extract the mouse delta directly from the event record
					if (CGGetLastMouseDelta_Ptr)
					{
						CGGetLastMouseDelta_Ptr(&CGx, &CGy);
						if((err = MPEnterCriticalRegion(CE_MouseLock, kDurationForever)) == noErr)
						{
							_CE_delta_x += CGx;
							_CE_delta_y += CGy;
							MPExitCriticalRegion(CE_MouseLock);
						}
					}
					else
					{
						// Fallback: get it from the event structure itself
						Point Loc;
						err = GetEventParameter(theEvent,
							kEventParamMouseDelta, typeQDPoint,
							NULL, sizeof(Loc), NULL, &Loc);
						if((err = MPEnterCriticalRegion(CE_MouseLock, kDurationForever)) == noErr)
						{
							_CE_delta_x = Loc.h;
							_CE_delta_y = Loc.v;
							MPExitCriticalRegion(CE_MouseLock);
						}
					}
					if (CGWarpMouseCursorPosition_Ptr)
						CGWarpMouseCursorPosition_Ptr(CGPointMake(CENTER_MOUSE_X, CENTER_MOUSE_Y));
					err = noErr;
					#endif
					break;

				case kEventMouseDown:
				case kEventMouseUp:
					
					GetEventParameter(theEvent, kEventParamMouseButton, typeMouseButton, NULL,
						 sizeof(EventMouseButton), NULL, &which_mouse_button);

					if(which_mouse_button <= MAX_BUTTONS)
					{
						snapshot_button_state[which_mouse_button - 1] =
							(event_kind == kEventMouseDown);
					}
					
					err = noErr;
					break;

				case kEventMouseWheelMoved:
					
					GetEventParameter(theEvent, kEventParamMouseWheelDelta, typeLongInteger,
						NULL, sizeof(SInt32), NULL, &scroll_wheel_delta);
					
					if((err = MPEnterCriticalRegion(CE_MouseLock, kDurationForever)) == noErr)
					{
						_CE_delta_scrollwheel += scroll_wheel_delta;
						MPExitCriticalRegion(CE_MouseLock);
					}
					err = noErr;
					break;
				
				default:
					break;
			}
		}
	}
	
	return err;
}
#endif

#ifndef __MACH__
void LoadCGMouseFunctions()
{
	CGWarpMouseCursorPosition_Ptr = (CGWarpMouseCursorPosition_Type)
		GetSystemFunctionPointer(CFSTR("CGWarpMouseCursorPosition"));
	
	CGGetLastMouseDelta_Ptr = (CGGetLastMouseDelta_Type)
		GetSystemFunctionPointer(CFSTR("CGGetLastMouseDelta"));
}
#endif

