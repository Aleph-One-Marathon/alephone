/*
MOUSE.C
Tuesday, January 17, 1995 2:51:59 PM  (Jason')

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Nov 17, 2000 (Loren Petrich):
	Added some handling of absent mice and out-of-range input types;
	this may help avert some crashes on certain PowerBook models
*/

/* marathon includes */
#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
#include "player.h"     // for get_absolute_pitch_range()
#include "mouse.h"
#include "shell.h"
#include <math.h>

/* macintosh includes */
#include <CursorDevices.h>
#include <Traps.h>

#ifdef env68k
#pragma segment input
#endif

/* constants */
#define _CursorADBDispatch 0xaadb
#define CENTER_MOUSE_X      320
#define CENTER_MOUSE_Y      240

static void get_mouse_location(Point *where);
static void set_mouse_location(Point where);
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

/* ---------- globals */

static CursorDevicePtr mouse_device = NULL;
static fixed snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity;
static bool snapshot_button_state;

/* ---------- code */

void enter_mouse(
	short type)
{
	(void) (type);
	
	mouse_device= find_mouse_device(); /* will use cursor device manager if non-NULL */

#ifndef env68k
	// vwarn(mouse_device, "no valid mouse/trackball device;g;"); /* must use cursor device manager on non-68k */
#endif
	
	snapshot_delta_yaw= snapshot_delta_pitch= snapshot_delta_velocity= false;
	snapshot_button_state= false;
	
	return;
}

void test_mouse(
	short type,
	uint32 *action_flags,
	fixed *delta_yaw,
	fixed *delta_pitch,
	fixed *delta_velocity)
{
	(void) (type);
	
	// Idiot-proofing in case of an absent mouse
	if (mouse_device == NULL)
	{
		*delta_yaw= 0;
		*delta_pitch= 0;
		*delta_velocity= 0;
		return;
	}
	
	if (snapshot_button_state) *action_flags|= _left_trigger_state;
	
	*delta_yaw= snapshot_delta_yaw;
	*delta_pitch= snapshot_delta_pitch;
	*delta_velocity= snapshot_delta_velocity;
	
	return;
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
	
	return;
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
	static long last_tick_count;
	long tick_count= TickCount();
	long ticks_elapsed= tick_count-last_tick_count;

	get_mouse_location(&where);

	center.h= CENTER_MOUSE_X, center.v= CENTER_MOUSE_Y;
	set_mouse_location(center);
	
	if (ticks_elapsed)
	{
		/* calculate axis deltas */
		fixed vx= INTEGER_TO_FIXED(where.h-center.h)/(ticks_elapsed*MAXIMUM_MOUSE_VELOCITY);
		fixed vy= - INTEGER_TO_FIXED(where.v-center.v)/(ticks_elapsed*MAXIMUM_MOUSE_VELOCITY);

		/* pin and do nonlinearity */
		vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2), vx>>= 1, vx*= (vx<0) ? -vx : vx, vx>>= 14;
		vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2), vy>>= 1, vy*= (vy<0) ? -vy : vy, vy>>= 14;
//		vx= PIN(vx, -FIXED_ONE/2, FIXED_ONE/2);
//		vy= PIN(vy, -FIXED_ONE/2, FIXED_ONE/2);

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
				// assert(false);
				// halt();
		}
		
		snapshot_button_state= Button();
		last_tick_count= tick_count;
		
//		dprintf("%08x %08x %08x;g;", snapshot_delta_yaw, snapshot_delta_pitch, snapshot_delta_velocity);
	}
	
	return;
}

/* ---------- private code */

// unused 
static void get_mouse_location(
	Point *where)
{
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
	
	return;
}

static void set_mouse_location(
	Point where)
{
	if (mouse_device)
	{
		CrsrDevMoveTo(mouse_device, where.h, where.v);
	}
#ifdef env68k
	else
	{
		RawMouse= where;
		MTemp= where;
		CrsrNewCouple= 0xffff;
	}
#endif
	
	return;
}

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
