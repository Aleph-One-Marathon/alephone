/* script_instructions.h

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

	2/16/00 Created -  Chris Pruett
	
	10/20/00 - Mark Levin
		Added instructions for monster control
		
	01/26/02 - EE
	Added Get_Platform_State, Set_Platform_State, Get_Light_State, Set_Light_State & Get_Player_Poly
	
	06/09/02 - tiennou:
		Added s_Play_Sound, s_Remove_Item, and stuff for Pfhortran Player control
		(GetPfhortranActionQueues & lots of changes in s_Player_Control).
		Added definition for the new Pfhortran ActionQueues.
		Added pfhortran_controls_player flag used in vbl.cpp:process_action_flags
*/


#ifndef _SCRIPT_INSTRUCTION_DEF
#define _SCRIPT_INSTRUCTION_DEF

/* define out our three possible return states in english for greater readability */
#define script_TRUE 1
#define script_FALSE 0
#define script_ERROR -1

#include "cseries.h"
#include "world.h"
#include "ActionQueues.h"

ActionQueues* GetPfhortranActionQueues(void);

struct path_point
{
	world_point3d position;
	short polygon_index;
	angle yaw;
	angle pitch;

};

struct script_camera
{
	short index;
	path_point location;
	short path;
	short point;

};

struct path_list
{
	path_point location;
	short index;
};

struct path_header
{
	path_list *the_path;
	short move_speed;		/* scrolling speed */
	short roll_speed;		/* yaw/pitch speed */
	short index;
	short length;
};

 
enum /* instruction defs */
{
	Camera_Move = 0x01,
	Camera_Look = 0x02,

	Wait_Ticks = 0x03,

	Inflict_Damage = 0x04,

	Jump = 0x05,

	Enable_Player = 0x06,

	Disable_Player = 0x07,

	Script_End = 0x08,

	Hide_Interface = 0x09,

	Show_Interface = 0x0A,

	Set_Tag_State = 0x0B,

	Get_Tag_State = 0x0C,

	Define = 0x0D,

	sAdd = 0x0E,

	sSubtract = 0x0F,

	If_Equal = 0x10,

	Set = 0x11,

	Call = 0x12,

	Return = 0x13,

	If_Greater = 0x14,

	If_Less = 0x15,

	If_Not_Equal = 0x16,

	Get_Life = 0x17,

	Set_Life = 0x18,

	Get_Oxygen = 0x19,

	Set_Oxygen = 0x1A,

	End = 0x1B,

	Get_My_Value = 0x1C,

	Add_Item = 0x1D,

	Select_Weapon = 0x1E,

	Block_Start = 0x1F,

	Block_End = 0x20,
	
	Init_Cameras = 0x21,
	
	Select_Camera = 0x22,
	
	Set_Camera_Poly = 0x23,
	
	Set_Camera_Pos = 0x24,
	
	Set_Camera_YP = 0x25,
	
	Activate_Camera = 0x26,
	
	Use_Camera = 0x27,
	
	Init_Paths = 0x28,
	
	New_Path = 0x29,
	
	Dispose_Path = 0x2A,
	
	Select_Path = 0x2B,
	
	Set_Path_Move_Speed = 0x2C,
	
	Select_Point = 0x2D,
	
	Set_Point_Poly = 0x2E,
	
	Set_Point_Pos = 0x2F,
	
	Set_Point_YP = 0x30,
		
	Start_Camera_On_Path = 0x31,	
	
	Set_Path_Roll_Speed = 0x32,
	
	Wait_For_Path = 0x33,
	
	Change_Soundtrack = 0x34,
	
	Set_Fog_Depth = 0x35,
	
	Set_Fog_Color = 0x36,
	
	Get_Fog_Depth = 0x37,
	
	Get_Fog_Color = 0x38,
	
	Teleport_Player = 0x39,
	
	//Additions for monster control
	// -- Mark Levin
	
	Monster_New = 0x3A,
	
	Monster_Sleep = 0x3B,
		
	Monster_Hurt = 0x3C,
	
	Monster_Attack = 0x3D,
	
	Monster_Move = 0x3E,
	
	Monster_Select = 0x3F,
	
	Monster_Get_Immunity = 0x40,
	
	Monster_Set_Immunity = 0x41,
	
	Monster_Get_Weakness = 0x42,
	
	Monster_Set_Weakness = 0x43,
	
	Monster_Get_Friend = 0x44,
	
	Monster_Set_Friend = 0x45,
	
	Monster_Get_Enemy = 0x46,
	
	Monster_Set_Enemy = 0x47,

	Monster_Get_Item = 0x48,
	
	Monster_Set_Item = 0x49,
	
	Monster_Get_Nuke = 0x4A,
	
	Monster_Set_Nuke = 0x4B,
	
	//Alexander Strange's underwater fog stuff, a straight copy of Matthew Hielscher's code

	Set_UnderFog_Depth = 0x4C,
	
	Set_UnderFog_Color = 0x4D,
	
	Get_UnderFog_Depth = 0x4E,
	
	Get_UnderFog_Color = 0x4F,
	
	Get_Random = 0x50,
	
	Set_Platform_State = 0x51,
	
	Get_Platform_State = 0x52,
	
	Set_Light_State = 0x53,
	
	Get_Light_State = 0x54,
	
	Get_Player_Poly = 0x55,
	
	// LP: adding this stuff for "EE"
	
	Get_Fog_Presence = 0x56,
	
	Set_Fog_Presence = 0x57,
	
	Get_UnderFog_Presence = 0x58,
	
	Set_UnderFog_Presence = 0x59,
	
	Remove_Item = 0x5A,
	
	Player_Control = 0x5B,
	
	Play_Sound = 0x5C,
	
	Debug_Message = 0x5D,
	
	Monster_Get_Action = 0x5E,
	
	Monster_Get_Mode = 0x5F,
	
	Monster_Get_Vitality = 0x60,
	
	Monster_Set_Vitality = 0x61,

	Not = 0x62,
	
	Start_Fade = 0x63,
	
	Get_Platform_Player_Control = 0x64,
	
	Set_Platform_Player_Control = 0x65,
	
	Get_Platform_Monster_Control = 0x66,
	
	Set_Platform_Monster_Control = 0x67,
	
	Get_Platform_Speed = 0x68,
	
	Set_Platform_Speed = 0x69,

	Get_Monster_Poly = 0x6A,
	
	Timer_Start = 0x6B,
	
	Timer_Get = 0x6C,
	
	Timer_Stop = 0x6D,
	
	Display_Message = 0x6E,
	
	Get_Platform_Height = 0x6F,
	
	Set_Motion_Sensor_State = 0x70,
	
	Get_Motion_Sensor_State = 0x71,
	
	NUMBER_OF_INSTRUCTIONS
};

void init_instructions(void);

#endif
