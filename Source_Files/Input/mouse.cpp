/*
MOUSE.C

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

/* macintosh includes */
#if defined(TARGET_API_MAC_CARBON)
    #include <ApplicationServices/ApplicationServices.h>
#else
#include <CursorDevices.h>
#include <Traps.h>
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
#if defined(TARGET_API_MAC_CARBON)
static pascal OSStatus CEvtHandleApplicationMouseEvents (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
#endif
#if !defined(SUPPRESS_MACOS_CLASSIC)
static CursorDevicePtr find_mouse_device(void);
static bool trap_available(short trap_num);
static TrapType get_trap_type(short trap_num);
static short num_toolbox_traps(void);

extern pascal OSErr CrsrDevNextDevice(CursorDevicePtr *ourDevice)
 TWOWORDINLINE(0x700B, 0xAADB);
extern pascal OSErr CrsrDevMoveTo(CursorDevicePtr ourDevice, long absX, long absY)
 TWOWORDINLINE(0x7001, 0xAADB);

#define MBState *((byte *)0x0172)
#define RawMouse *((Point *)0x082c)
#define MTemp *((Point *)0x0828)
#define CrsrNewCouple *((short *)0x08ce)
#endif

/* ---------- globals */

#if !defined(SUPPRESS_MACOS_CLASSIC)
static CursorDevicePtr mouse_device = NULL;
#endif
static _fixed snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity;
#if !defined(TARGET_API_MAC_CARBON)
static bool snapshot_button_state;
#else
#define MAX_BUTTONS 3
static bool snapshot_button_state[MAX_BUTTONS];
static int snapshot_delta_scrollwheel;
static MPCriticalRegionID CE_MouseLock;
static int _CE_delta_x, _CE_delta_y, _CE_delta_scrollwheel;
static EventHandlerUPP _CEMouseTrackerUPP;
static EventHandlerRef _CEMouseTracker;
#endif
/* ---------- code */

void enter_mouse(
	short type)
{
	(void) (type);
	
#if !defined(SUPPRESS_MACOS_CLASSIC)
	mouse_device= find_mouse_device(); /* will use cursor device manager if non-NULL */
#endif

#ifndef env68k
	// vwarn(mouse_device, "no valid mouse/trackball device;g;"); /* must use cursor device manager on non-68k */
#endif
	
	snapshot_delta_yaw= snapshot_delta_pitch= snapshot_delta_velocity= false;
#if !defined(TARGET_API_MAC_CARBON)	
	snapshot_button_state= false;

#else
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
	
#if !defined(SUPPRESS_MACOS_CLASSIC)
	// Idiot-proofing in case of an absent mouse
	if (mouse_device == NULL)
	{
		*delta_yaw= 0;
		*delta_pitch= 0;
		*delta_velocity= 0;
		return;
	}
#endif
	
#if !defined(TARGET_API_MAC_CARBON)
	if (snapshot_button_state) *action_flags|= _left_trigger_state;
#else
	if (snapshot_button_state[0]) *action_flags|= _left_trigger_state;
	if (snapshot_button_state[1]) *action_flags|= _right_trigger_state;
	if (snapshot_delta_scrollwheel > 0) *action_flags|= _cycle_weapons_forward;
	else if (snapshot_delta_scrollwheel < 0) *action_flags|= _cycle_weapons_backward;
	snapshot_delta_scrollwheel = 0;
#endif	
	*delta_yaw= snapshot_delta_yaw;
	*delta_pitch= snapshot_delta_pitch;
	*delta_velocity= snapshot_delta_velocity;
}

static bool mouse_available(
	short type)
{
	(void) (type);
	
	return true;
}

void exit_mouse(
	short type)
{
	(void) (type);

#if defined(TARGET_API_MAC_CARBON)
	RemoveEventHandler(_CEMouseTracker);
	DisposeEventHandlerUPP(_CEMouseTrackerUPP);
	_CEMouseTrackerUPP = NULL;
	MPDeleteCriticalRegion(CE_MouseLock);
#endif
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
	static long last_tick_count;
	long tick_count= TickCount();
	long ticks_elapsed= tick_count-last_tick_count;

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

		/* pin and do nonlinearity */
		vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1, vx*= (vx<0) ? -vx : vx, vx>>= 14;
		vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1, vy*= (vy<0) ? -vy : vy, vy>>= 14;

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
		
#if !defined(TARGET_API_MAC_CARBON)
		snapshot_button_state= Button();
#else
		// It works for the primary...
		snapshot_button_state[0] = Button();
		
		if(MPEnterCriticalRegion(CE_MouseLock, kDurationImmediate) == noErr)
		{
			snapshot_delta_scrollwheel = _CE_delta_scrollwheel;
			_CE_delta_scrollwheel = 0;
			MPExitCriticalRegion(CE_MouseLock);
		}
#endif
		last_tick_count= tick_count;
		
//		dprintf("%08x %08x %08x;g;", snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity);
	}
}

/* ---------- private code */

// unused 
static void get_mouse_location(
	Point *where)
{

#if defined(TARGET_API_MAC_CARBON)
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
		where->h = CENTER_MOUSE_X;
		where->v = CENTER_MOUSE_Y;
	}
#else
	if (mouse_device)
	{
		where->h = mouse_device->whichCursor->where.h;
		where->v = mouse_device->whichCursor->where.v;
	}
	else
	{
		*where= RawMouse;
//		GetMouse(where);
//		LocalToGlobal(where);
	}
#endif
}

static void set_mouse_location(
	Point where)
{
#if defined(TARGET_API_MAC_CARBON)
//	CGWarpMouseCursorPosition(CGPointMake(where.h, where.v));
#else
	if (mouse_device)
	{
		CrsrDevMoveTo(mouse_device, where.h, where.v);
	}
#endif
#ifdef env68k
	else
	{
		RawMouse= where;
		MTemp= where;
		CrsrNewCouple= 0xffff;
	}
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
//	extern qboolean		background;

	event_kind = GetEventKind(theEvent);
	event_class = GetEventClass(theEvent);
	
	if (nextHandler)
		err = CallNextEventHandler (nextHandler, theEvent);

	if (err == noErr || err == eventNotHandledErr)
	{
		if (event_class == kEventClassMouse)
		{
			// If we're not in the game, let something else handle mouse clicks
			if(get_game_state() != _game_in_progress)
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
					CGGetLastMouseDelta(&CGx, &CGy);
					OSStatus err;
					if((err = MPEnterCriticalRegion(CE_MouseLock, kDurationForever)) == noErr)
					{
						_CE_delta_x += CGx;
						_CE_delta_y += CGy;
						MPExitCriticalRegion(CE_MouseLock);
					}
					err = noErr;
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

#if !defined(SUPPRESS_MACOS_CLASSIC)
static CursorDevicePtr find_mouse_device(
	void)
{
	CursorDevicePtr device= (CursorDevicePtr) NULL;
	
	if (trap_available(_CursorADBDispatch))
	{
		do
		{
			CrsrDevNextDevice(&device);
		}
		while (device && device->devClass!=kDeviceClassMouse && device->devClass!=kDeviceClassTrackball);
	}
		
	return device;
}

/* ---------- from IM */

static bool trap_available(short trap_num)
{
	TrapType type;
	
	type = get_trap_type(trap_num);
	if (type == ToolTrap)
		trap_num &= 0x07ff;
	if (trap_num > num_toolbox_traps())
		trap_num = _Unimplemented;
	
	return NGetTrapAddress(trap_num, type) != NGetTrapAddress(_Unimplemented, ToolTrap);
}

#define TRAP_MASK  0x0800

static TrapType get_trap_type(short trap_num)
{
	if ((trap_num & TRAP_MASK) > 0)
		return ToolTrap;
	else
		return OSTrap;
}

static short num_toolbox_traps(void)
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xaa6e, ToolTrap))
		return 0x0200;
	else
		return 0x0400;
}
#endif