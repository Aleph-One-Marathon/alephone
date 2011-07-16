/*

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

	March 2, 2000 (Loren Petrich)

	Chase-Cam Interface; this makes Marathon something like Halo.

*/

#ifndef _CHASE_CAM
#define _CHASE_CAM

#include "world.h"

enum // Chase-cam flags

{
	_ChaseCam_OnWhenEntering = 0x0004,
	_ChaseCam_NeverActive = 0x0002,
	_ChaseCam_ThroughWalls = 0x0001
};

struct ChaseCamData
{
	short Behind;
	short Upward;
	short Rightward;
	short Flags;
	float Damping;
	float Spring;
	float Opacity;
};

// True for OK, false for cancel;
// the structure will not be changed if this was canceled
// Implemented in PlayerDialogs.c
bool Configure_ChaseCam(ChaseCamData &Data);

// Gotten from preferences
// Implemented in preferences.c
ChaseCamData& GetChaseCamData();

// LP addition: chase-cam interface.
// This function returns whether the chase cam can possibly activate;
// this is done to avoid loading the player sprites if it cannot be.
bool ChaseCam_CanExist();

// All these functions return the chase cam's state (true: active; false: inactive)
bool ChaseCam_IsActive();
bool ChaseCam_SetActive(bool NewState);

// This function initializes the chase cam for a game
bool ChaseCam_Initialize();

// This function resets the chase cam, in case one has entered a level,
// is reviving, or is teleporting
bool ChaseCam_Reset();

// This function updates the chase cam over a game tick. It's done that way
// so that the chase-cam physics will be correct.
bool ChaseCam_Update();

// This function makes the chase cam switch sides
// when it is offset to one side
bool ChaseCam_SwitchSides();

// This function calls everything as references; it does not change the outputs
// if the chase-cam is inactive. It will return everything necessary to set the chase-cam's view.
bool ChaseCam_GetPosition(world_point3d &position,
	short &polygon_index, angle &yaw, angle &pitch);

#endif

