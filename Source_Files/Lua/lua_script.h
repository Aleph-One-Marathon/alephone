#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

/*
LUA_SCRIPT.H

	Copyright (C) 2003 and beyond by Matthew Hielscher
	and the "Aleph One" developers
 
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

	Controls the loading and execution of Lua scripts.
*/

#include "cseries.h"
#include "world.h"
#include "ActionQueues.h"
#include "shape_descriptors.h"

#include <map>
#include <string>

void L_Error(const char *message);
void L_Call_Init(bool fRestoringSaved);
void L_Call_Cleanup();
void L_Call_Idle();
void L_Call_PostIdle();
void L_Call_Start_Refuel(short type, short player_index, short panel_side_index);
void L_Call_End_Refuel(short type, short player_index, short panel_side_index);
void L_Call_Tag_Switch(short tag, short player_index, short side_index);
void L_Call_Light_Switch(short light, short player_index, short side_index);
void L_Call_Platform_Switch(short platform, short player_index, short side_index);
void L_Call_Terminal_Enter(short terminal_id, short player_index);
void L_Call_Terminal_Exit(short terminal_id, short player_index);
void L_Call_Pattern_Buffer(short side_index, short player_index);
void L_Call_Projectile_Switch(short side_index, short projectile_index);
void L_Call_Got_Item(short type, short player_index);
void L_Call_Light_Activated(short index);
void L_Call_Platform_Activated(short index);
void L_Call_Player_Revived(short player_index);
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index);
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index);
void L_Call_Monster_Damaged(short monster_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location, uint16_t flags, int16_t obstruction_index, int16_t line_index);
void L_Call_Projectile_Created(short projectile_index);
void L_Call_Item_Created(short item_index);

void L_Invalidate_Effect(short effect_index);
void L_Invalidate_Monster(short monster_index);
void L_Invalidate_Projectile(short projectile_index);
void L_Invalidate_Object(short object_index);
void L_Invalidate_Ephemera(short ephemera_index);

enum ScriptType {
	_embedded_lua_script,
	_lua_netscript,
	_solo_lua_script,
	_stats_lua_script
};

void *L_Persistent_Table_Key();

bool LoadLuaScript(const char *buffer, size_t len, ScriptType type);
bool RunLuaScript();
void CloseLuaScript();
void ResetPassedLua();

void ExecuteLuaString(const std::string&);
void LoadSoloLua();
void LoadReplayNetLua();

void LoadStatsLua();
bool CollectLuaStats(std::map<std::string, std::string>& table, std::map<std::string, std::string>& parameters);

void ToggleLuaMute();
void ResetLuaMute();

bool UseLuaCameras();

void unpack_lua_states(uint8* data, size_t length);
size_t save_lua_states();
void pack_lua_states(uint8* data, size_t length);

ActionQueues* GetLuaActionQueues();

void MarkLuaCollections(bool active);

void LuaTexturePaletteClear();
int LuaTexturePaletteSize();
shape_descriptor LuaTexturePaletteTexture(size_t);
short LuaTexturePaletteTextureType(size_t);
int LuaTexturePaletteSelected();

bool LuaPlayerCanWieldWeapons(short player_index);

/* Custom game scoring modes */
enum {
  _game_of_most_points,
  _game_of_most_time,
  _game_of_least_points,
  _game_of_least_time,
  NUMBER_OF_GAME_SCORING_MODES
};

/* Game end conditions */
enum {
  _game_normal_end_condition,
  _game_no_end_condition,
  _game_end_now_condition,
  NUMBER_OF_GAME_END_CONDITIONS
};

int GetLuaScoringMode();
int GetLuaGameEndCondition();

// camera data structures
struct timed_point
{
    int polygon;
    world_point3d point;
    int32 delta_time; //for REALLY long cutscenes
};

struct timed_angle
{
    short yaw, pitch;
    int32 delta_time;
};

struct lua_path
{
    short index;
    std::vector<timed_point> path_points;
    short current_point_index;
    int32 last_point_time;
    std::vector<timed_angle> path_angles;
    short current_angle_index;
    int32 last_angle_time;
};

struct lua_camera //an expanded version of script_camera; uses Lua's path scheme
{
    short index;
    lua_path path;
    int32 time_elapsed;
    int player_active;
};

#endif
