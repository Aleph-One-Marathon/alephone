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

	Chase-Cam implementation; this makes Marathon something like Halo.
	
	Moved out of player.c
*/

#include "cseries.h"

#include "map.h"
#include "player.h"
#include "ChaseCam.h"
#include "network.h"

#include <limits.h>

// Chase-cam state globals
static bool _ChaseCam_IsActive = false;
static bool _ChaseCam_IsReset = true;

// Chase-cam position globals;
// the extra positions are the chase cam's previous positions
static world_point3d CC_Position, CC_Position_1, CC_Position_2;
static short CC_Polygon, CC_Yaw, CC_Pitch;


// LP addition: chase-cam functions
// This function returns whether the chase cam can possibly activate;
// this is done to avoid loading the player sprites if it cannot be.
bool ChaseCam_CanExist()
{
  return !TEST_FLAG(GetChaseCamData().Flags,_ChaseCam_NeverActive);
}


// All these functions return the chase cam's state (true: active; false: inactive)
bool ChaseCam_IsActive()
{
  if (!NetAllowBehindview()) return false;
  if (!ChaseCam_CanExist()) return false;
  return _ChaseCam_IsActive;
}
bool ChaseCam_SetActive(bool NewState)
{
  if (!NetAllowBehindview()) return false;
  if (!ChaseCam_CanExist()) return false;
  if (!_ChaseCam_IsActive && NewState != 0)
  {
    _ChaseCam_IsActive = true;
    ChaseCam_Reset();
    ChaseCam_Update();
  }
  return (_ChaseCam_IsActive = (NewState != 0));
}

// This function initializes the chase cam for a game
bool ChaseCam_Initialize()
{
	if (!ChaseCam_CanExist()) return false;
	
	// Of course...
	ChaseCam_Reset();

	return ChaseCam_SetActive(TEST_FLAG(GetChaseCamData().Flags,_ChaseCam_OnWhenEntering));
}

// This function resets the chase cam, in case one has entered a level,
// is reviving, or is teleporting
bool ChaseCam_Reset()
{
	if (!ChaseCam_CanExist()) return false;
	if (!ChaseCam_IsActive()) return false;
	
	_ChaseCam_IsReset = true;
	
	return true;
}


// Switches which side if horizontally offset
bool ChaseCam_SwitchSides()
{
	if (!ChaseCam_CanExist()) return false;
	if (!ChaseCam_IsActive()) return false;

	ChaseCamData &ChaseCam = GetChaseCamData();
	ChaseCam.Rightward *= -1;

	return true;
}

// This function calls everything as references; it does not change the outputs
// if the chase-cam is inactive. It will return everything necessary to set the chase-cam's view.
// Any persistent data, such as previous position, ought to be stored in player_data.
// Won't alter camera_position and camera_polygon; these are needed for various
// other stuff.
bool ChaseCam_GetPosition(world_point3d &position,
	short &polygon_index, angle &yaw, angle &pitch)
{
	if (!ChaseCam_CanExist()) return false;
	if (!ChaseCam_IsActive()) return false;
	
	position = CC_Position;
	polygon_index = CC_Polygon;
	yaw = CC_Yaw;
	pitch = CC_Pitch;
	return true;
}


// For updating the position with the help of the cam's springiness
static int CC_PosUpdate(float Damping, float Spring, short x0, short x1, short x2)
{
	float x = x0 + (2*Damping)*(x1 - x0) - (Damping*Damping + Spring)*(x2 - x0);
	return ((x > 0) ? int(x + 0.5) : int(x - 0.5));
}


// Args of function to find where the chase cam hits the walls:
//   Whether it will go through walls (it it can, it will use the last polygon visited)
//   Start position
//   Input: target position -- Output: final position
//   Input: start-position polygon index -- Output: final-position polygon index
//
//  If using the exclusion zones is desired, then here is the code:
//		world_distance adjusted_floor_height, adjusted_ceiling_height;
//		short supporting_polygon_index;
//		keep_line_segment_out_of_walls(Ref_Polygon, &Ref_Position, &CC_Position,
//			WORLD_ONE, 0, &adjusted_floor_height, &adjusted_ceiling_height, &supporting_polygon_index);
//
/*static*/ void ShootForTargetPoint(bool ThroughWalls,
	world_point3d& StartPosition, world_point3d& EndPosition,
	short& Polygon)
{
	// Prevent short-integer wraparound
	const short HALF_SHORT_MIN = SHRT_MIN >> 1;
	const short HALF_SHORT_MAX = SHRT_MAX >> 1;
	if (StartPosition.x >= HALF_SHORT_MAX && EndPosition.x < HALF_SHORT_MIN) EndPosition.x = SHRT_MAX;
	if (StartPosition.x < HALF_SHORT_MIN && EndPosition.x >= HALF_SHORT_MAX) EndPosition.x = SHRT_MIN;
	if (StartPosition.y >= HALF_SHORT_MAX && EndPosition.y < HALF_SHORT_MIN) EndPosition.y = SHRT_MAX;
	if (StartPosition.y < HALF_SHORT_MIN && EndPosition.y >= HALF_SHORT_MAX) EndPosition.y = SHRT_MIN;

	// Cribbed from translate_map_object() and simplified
	short line_index, Next_Polygon;
	while(true)
	{
		
		// Check for hitting the floors and ceilings
		struct world_point3d intersection;
		struct polygon_data *polygon= get_polygon_data(Polygon);
		world_distance height;
		height = polygon->floor_height;
		if (EndPosition.z <= height)
		{
			// Floor check
			find_floor_or_ceiling_intersection(height, &StartPosition, &EndPosition, &intersection);
			if (find_line_crossed_leaving_polygon(Polygon, (world_point2d *)&StartPosition, (world_point2d *)&intersection) == NONE)
			{
				if (!ThroughWalls) EndPosition = intersection;
				break;
			}
		}
		height = polygon->ceiling_height;
		if (EndPosition.z >= height)
		{
			// Ceiling check
			find_floor_or_ceiling_intersection(height, &StartPosition, &EndPosition, &intersection);
			if (find_line_crossed_leaving_polygon(Polygon, (world_point2d *)&StartPosition, (world_point2d *)&intersection) == NONE)
			{
				if (!ThroughWalls) EndPosition = intersection;
				break;
			}
		}
		
		// Which line crossed?
		line_index= find_line_crossed_leaving_polygon(Polygon, (world_point2d *)&StartPosition, (world_point2d *)&EndPosition);
		if (line_index!=NONE)
		{
			// Where did the crossing happen?
			struct line_data *line= get_line_data(line_index);
			find_line_intersection(&get_endpoint_data(line->endpoint_indexes[0])->vertex,
				&get_endpoint_data(line->endpoint_indexes[1])->vertex, &StartPosition, &EndPosition,
				&intersection);
			
			// Check on the next polygon
			Next_Polygon= find_adjacent_polygon(Polygon, line_index);
			if (Next_Polygon!=NONE)
			{
				// Can the viewpoint enter the next polygon?
				struct polygon_data *next_polygon= get_polygon_data(Next_Polygon);
				if (intersection.z <= next_polygon->floor_height)
				{
					// Hit the lower boundary wall
					if (!ThroughWalls) EndPosition = intersection;
					break;
				}
				else if (intersection.z >= next_polygon->ceiling_height)
				{
					// Hit the upper boundary wall
					if (!ThroughWalls) EndPosition = intersection;
					break;
				}
				else
					// Can continue
					Polygon = Next_Polygon;
			}
			else
			{
				// Fallback in case of invalid next polygon
				if (!ThroughWalls) EndPosition = intersection;
				// {
				//	*((world_point2d *)(&EndPosition)) = get_polygon_data(Polygon)->center;
				//	EndPosition.z = intersection.z;
				// }
				break;
			}
		}
		else
			break;
	}
}


// This function updates the chase cam's position in one game tick
bool ChaseCam_Update()
{
	if (!ChaseCam_CanExist()) return false;
	if (!ChaseCam_IsActive()) return false;
	
	ChaseCamData &ChaseCam = GetChaseCamData();
	
	// Move positions backward in time if the chase cam was not reset
	if (!_ChaseCam_IsReset)
	{
		CC_Position_2 = CC_Position_1;
		CC_Position_1 = CC_Position;
	}
	
	// Crude chase-cam: copy over everything;
	// need to keep old position as a reference for travels
	world_point3d Ref_Position = current_player->camera_location;
	Ref_Position.z -= current_player->step_height;
	short Ref_Polygon = current_player->camera_polygon_index;
	CC_Yaw = current_player->facing;
	CC_Pitch = current_player->elevation;
	
	// Now set its position; don't worry about what polygon it is in
	CC_Position = Ref_Position;
	translate_point3d(&CC_Position,-ChaseCam.Behind,CC_Yaw,CC_Pitch);
	CC_Position.z += ChaseCam.Upward;
	translate_point2d((world_point2d *)&CC_Position,ChaseCam.Rightward,NORMALIZE_ANGLE(CC_Yaw+QUARTER_CIRCLE));
	
	// Can the camera go through walls?
	// If it can, then go back as far as possible through polygons,
	// and use the last one passed through
	bool ThroughWalls = TEST_FLAG(ChaseCam.Flags,_ChaseCam_ThroughWalls);
		
	// Use inertia to update the chase cam's position if it had not been reset.
	if (!_ChaseCam_IsReset)
	{
		// If the chase cam won't go through a wall, then its target position must not also
		if (!ThroughWalls)
		{
			CC_Polygon = Ref_Polygon;
			ShootForTargetPoint(ThroughWalls, Ref_Position, CC_Position, CC_Polygon);
		}
		
		CC_Position.x = CC_PosUpdate(ChaseCam.Damping,ChaseCam.Spring,
			CC_Position.x,CC_Position_1.x,CC_Position_2.x);
		CC_Position.y = CC_PosUpdate(ChaseCam.Damping,ChaseCam.Spring,
			CC_Position.y,CC_Position_1.y,CC_Position_2.y);
		CC_Position.z = CC_PosUpdate(ChaseCam.Damping,ChaseCam.Spring,
			CC_Position.z,CC_Position_1.z,CC_Position_2.z);
	}
	
	CC_Polygon = Ref_Polygon;
	ShootForTargetPoint(ThroughWalls, Ref_Position, CC_Position, CC_Polygon);
	
	// If the chase cam had to be reset, then set the previous positions to the current position
	if (_ChaseCam_IsReset)
	{
		CC_Position_2 = CC_Position_1 = CC_Position;
		_ChaseCam_IsReset = false;
	}
	
	return true;
}
