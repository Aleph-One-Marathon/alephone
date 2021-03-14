/*
LUA_SCRIPT.CPP

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

/*
 Created 5-20-03 by Matthew Hielscher
 Controls the loading and execution of Lua scripts.

 Matthew Hielscher, 05-28-03
 Changed the error code to be much more graceful (no more quitting after an error)
 Also incorporated tiennou's functions

 tiennou, 06/23/03
 Added stuff on platforms (speed, heights, movement), terminals (text index), &polygon (heights).

 tiennou, 06/25/03
 Removed the last useless logError around. They prevented some functions to behave properly.
 Added L_Get_Player_Angle, returns player->facing & player->elevation.
 Got rid of all the cast-related warnings in there (Thanks Br'fin !).

 jkvw, 07/03/03
 Added recharge panel triggers, exposed A1's internal random number generators, and item creation.

 jkvw, 07/07/03
 Cleaned up some of the "odd" behaviors.  (e.g., new_monster/new_item would spawn their things at incorrect height.)
 Added triggers for player revival/death.

 tiennou, 07/20/03
 Added mnemonics for sounds, changed L_Start_Fade to L_Screen_Fade, added side_index parameter to L_Call_Start/End_Refuel and updated the docs with the info I had...

 jkvw, 07/21/03
 Lua access to network scoring and network compass, and get_player_name.

 Woody Zenfell, 08/05/03
 Refactored L_Call_* to share common code; reporting runtime Lua script errors via screen_printf
 
 jkvw, 09/16/03
 L_Call_* no longer need guarding with #ifdef HAVE_LUA
 */

// cseries defines HAVE_LUA on A1/SDL
#include "cseries.h"

#include "mouse.h"
#include "interface.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#endif

#include <string>
#include <stdlib.h>
#include <set>
#include <unordered_map>

#include "alephversion.h"
#include "screen.h"
#include "tags.h"
#include "player.h"
#include "render.h"
#include "shell.h"
#include "Logging.h"
#include "lightsource.h"
#include "game_window.h"
#include "items.h"
#include "platforms.h"
#include "media.h"
#include "weapons.h"
#include "monsters.h"
#include "flood_map.h"
#include "vbl.h"
#include "fades.h"
#include "physics_models.h"
#include "Crosshairs.h"
#include "OGL_Setup.h"
#include "SoundManager.h"
#include "world.h"
#include "computer_interface.h"
#include "network.h"
#include "network_games.h"
#include "Random.h"
#include "Console.h"
#include "Music.h"
#include "ViewControl.h"
#include "preferences.h"
#include "BStream.h"
#include "Plugins.h"
#include "shell_options.h"
#include "interpolated_world.h"

#include "lua_script.h"
#include "lua_ephemera.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_projectiles.h"
#include "lua_saved_objects.h"
#include "lua_serialize.h"

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
namespace io = boost::iostreams;

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "monster_definitions.h"


bool use_lua_compass[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
bool can_wield_weapons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
world_point2d lua_compass_beacons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
short lua_compass_states[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

static ActionQueues* sLuaActionQueues = 0;
ActionQueues* GetLuaActionQueues() { return sLuaActionQueues; }

int game_scoring_mode = _game_of_most_points;
int game_end_condition = _game_normal_end_condition;

int GetLuaScoringMode() {
	return game_scoring_mode;
}

#ifndef HAVE_LUA

void L_Call_Init(bool) {}
void L_Call_Cleanup() {}
void L_Call_Idle() {}
void L_Call_PostIdle() {}
void L_Call_Sent_Message(const char* username, const char* message) {}
void L_Call_Start_Refuel(short type, short player_index, short panel_side_index) {}
void L_Call_End_Refuel(short type, short player_index, short panel_side_index) {}
void L_Call_Tag_Switch(short tag, short player_index, short) {}
void L_Call_Light_Switch(short light, short player_index, short) {}
void L_Call_Platform_Switch(short platform, short player_index, short) {}
void L_Call_Terminal_Enter(short terminal_id, short player_index) {}
void L_Call_Terminal_Exit(short terminal_id, short player_index) {}
void L_Call_Pattern_Buffer(short side_index, short player_index) {}
void L_Call_Got_Item(short type, short player_index) {}
void L_Call_Light_Activated(short index) {}
void L_Call_Platform_Activated(short index) {}
void L_Call_Player_Revived(short player_index) {}
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index) {}
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index) {}
void L_Call_Monster_Damaged(short monster_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index) { }
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index) {}
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location, uint16_t flags, int16_t obstruction_index, int16_t line_index) {}
void L_Call_Projectile_Switch(short, short) {}
void L_Call_Projectile_Created(short projectile_index) {}
void L_Call_Item_Created(short item_index) {}

void L_Invalidate_Effect(short) { }
void L_Invalidate_Monster(short) { }
void L_Invalidate_Projectile(short) { }
void L_Invalidate_Object(short) { }
void L_Invalidate_Ephemera(short) { }

bool LoadLuaScript(const char *buffer, size_t len, const char *desc) { /* Should never get here! */ return false; }
bool RunLuaScript() {
	for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
		use_lua_compass [i] = false;
	return false;
}
void CloseLuaScript() {}

void ToggleLuaMute() {}
void ResetLuaMute() {}

bool UseLuaCameras() { return false; }
bool LuaPlayerCanWieldWeapons(short) { return true; }

int GetLuaGameEndCondition() {
	return _game_normal_end_condition;
}

#else /* HAVE_LUA */

bool mute_lua = false;

// Steal all this stuff
extern void ShootForTargetPoint(bool ThroughWalls, world_point3d& StartPosition, world_point3d& EndPosition, short& Polygon);
extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void draw_panels();

extern bool MotionSensorActive;

extern void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);

extern struct view_data *world_view;
extern struct static_data *static_world;

static const luaL_Reg lualibs[] = {
	{"", luaopen_base},
	{LUA_TABLIBNAME, luaopen_table},
	{LUA_STRLIBNAME, luaopen_string},
	{LUA_BITLIBNAME, luaopen_bit32},
	{LUA_MATHLIBNAME, luaopen_math},
	{LUA_DBLIBNAME, luaopen_debug},
	{NULL, NULL}
};

static const luaL_Reg insecurelibs[] = {
	{LUA_IOLIBNAME, luaopen_io},
	{LUA_OSLIBNAME, luaopen_os},
	{NULL, NULL}
};

void* L_Persistent_Table_Key()
{
	static const char *key = "persist";
	return const_cast<char*>(key);
}

std::map<int, std::string> PassedLuaState;
std::map<int, std::string> SavedLuaState;

class LuaState
{
	friend bool CollectLuaStats(std::map<std::string, std::string>&, std::map<std::string, std::string>&);
public:
	LuaState() : running_(false), num_scripts_(0) {
		state_.reset(luaL_newstate(), lua_close);
	}

	virtual ~LuaState() {
	}

public:

	bool Load(const char *buffer, size_t len, const char *desc);
	bool Loaded() { return num_scripts_ > 0; }
	bool Running() { return running_; }
	bool Run();
	void Stop() { running_ = false; }
	bool Matches(lua_State *state) { return state == State(); }
	void MarkCollections(std::set<short>* collections);
	void ExecuteCommand(const std::string& line);
	std::string SavePassed();
	std::string SaveAll();

	virtual void Initialize() {
		const luaL_Reg *lib = lualibs;
		for (; lib->func; lib++)
		{
			luaL_requiref(State(), lib->name, lib->func, 1);
			lua_pop(State(), 1);
		}

		if (shell_options.insecure_lua) 
		{
			const luaL_Reg *lib = insecurelibs;
			for (; lib->func; lib++)
			{
				luaL_requiref(State(), lib->name, lib->func, 1);
				lua_pop(State(), 1);
			}
		}
                else
                {
                    lua_pushnil(State());
                    lua_setglobal(State(), "dofile");

                    lua_pushnil(State());
                    lua_setglobal(State(), "loadfile");
                }

		// set up a persistence table in the registry
		lua_pushlightuserdata(State(), (void *) L_Persistent_Table_Key());
		lua_newtable(State());
		lua_settable(State(), LUA_REGISTRYINDEX);

		RegisterFunctions();
		LoadCompatibility();
	}

	virtual void SetSearchPath(const std::string& path) {
		L_Set_Search_Path(State(), path);
	}

protected:
	bool GetTrigger(const char *trigger);
	void CallTrigger(int numArgs = 0);

	virtual void RegisterFunctions();
	virtual void LoadCompatibility();

	boost::shared_ptr<lua_State> state_;
	lua_State* State() { return state_.get(); }

public:
	// triggers
	void Init(bool fRestoringSaved);
	void Idle();
	void Cleanup();
	void PostIdle();
	void StartRefuel(short type, short player_index, short panel_side_index);
	void EndRefuel(short type, short player_index, short panel_side_index);
	void TagSwitch(short tag, short player_index, short side_index);
	void LightSwitch(short tag, short player_index, short side_index);
	void PlatformSwitch(short tag, short player_index, short side_index);
	void ProjectileSwitch(short side_index, short projectile_index);
	void TerminalEnter(short terminal_id, short player_index);
	void TerminalExit(short terminal_id, short player_index);
	void PatternBuffer(short side_index, short player_index);
	void GotItem(short type, short player_index);
	void LightActivated(short index);
	void PlatformActivated(short index);
	void PlayerRevived(short player_index);
	void PlayerKilled(short player_index, short aggressor_player_index, short action, short projectile_index);
	void MonsterKilled(short monster_index, short aggressor_player_index, short projectile_index);
	void MonsterDamaged(short monster_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
	void PlayerDamaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
	void ProjectileDetonated(short type, short owner_index, short polygon, world_point3d location, uint16_t flags, int16_t obstruction_index, int16_t line_index);
	void ProjectileCreated(short projectile_index);
	void ItemCreated(short item_index);

	void InvalidateEffect(short effect_index);
	void InvalidateMonster(short monster_index);
	void InvalidateProjectile(short projectile_index);
	void InvalidateObject(short object_index);
	void InvalidateEphemera(short ephemera_index);

	int RestorePassed(const std::string& s);
	int RestoreAll(const std::string& s);

private:
	bool running_;
	int num_scripts_;
};

typedef LuaState EmbeddedLuaState;
typedef LuaState NetscriptState;
typedef LuaState StatsLuaState;

class SoloScriptState : public LuaState
{
public:
	SoloScriptState() : LuaState() { }

	void Initialize() {
		LuaState::Initialize();
		luaL_requiref(State(), LUA_IOLIBNAME, luaopen_io, 1);
		lua_pop(State(), 1);
	}
};

bool LuaState::GetTrigger(const char* trigger)
{
	if (!running_)
		return false;

	lua_getglobal(State(), "Triggers");
	if (!lua_istable(State(), -1))
	{
		lua_pop(State(), 1);
		return false;
	}

	lua_pushstring(State(), trigger);
	lua_gettable(State(), -2);
	if (!lua_isfunction(State(), -1))
	{
		lua_pop(State(), 2);
		return false;
	}

	lua_remove(State(), -2);
	return true;
}

void LuaState::CallTrigger(int numArgs)
{
	if (lua_pcall(State(), numArgs, 0, 0) == LUA_ERRRUN)
		L_Error(lua_tostring(State(), -1));
}

void LuaState::Init(bool fRestoringSaved)
{
	if (GetTrigger("init"))
	{
		lua_pushboolean(State(), fRestoringSaved);
		CallTrigger(1);
	}
}

void LuaState::Idle()
{
	if (GetTrigger("idle"))
		CallTrigger();
}

void LuaState::Cleanup()
{
	if (GetTrigger("cleanup"))
		CallTrigger();
}

void LuaState::PostIdle()
{
	if (GetTrigger("postidle"))
		CallTrigger();
}

void LuaState::StartRefuel(short type, short player_index, short panel_side_index)
{
	if (GetTrigger("start_refuel"))
	{
		Lua_ControlPanelClass::Push(State(), type);
		Lua_Player::Push(State(), player_index);
		Lua_Side::Push(State(), panel_side_index);
		CallTrigger(3);
	}
}

void LuaState::EndRefuel(short type, short player_index, short panel_side_index)
{
	if (GetTrigger("end_refuel"))
	{
		Lua_ControlPanelClass::Push(State(), type);
		Lua_Player::Push(State(), player_index);
		Lua_Side::Push(State(), panel_side_index);
		CallTrigger(3);
	}
}

void LuaState::TagSwitch(short tag, short player_index, short side_index)
{
	if (GetTrigger("tag_switch"))
	{
		Lua_Tag::Push(State(), tag);
		Lua_Player::Push(State(), player_index);
		Lua_Side::Push(State(), side_index);
		CallTrigger(3);
	}
}

void LuaState::LightSwitch(short light, short player_index, short side_index)
{
	if (GetTrigger("light_switch"))
	{
		Lua_Light::Push(State(), light);
		Lua_Player::Push(State(), player_index);
		Lua_Side::Push(State(), side_index);
		CallTrigger(3);
	}
}

void LuaState::PlatformSwitch(short platform, short player_index, short side_index)
{
	if (GetTrigger("platform_switch"))
	{
		Lua_Polygon::Push(State(), platform);
		Lua_Player::Push(State(), player_index);
		Lua_Side::Push(State(), side_index);
		CallTrigger(3);
	}
}

void LuaState::ProjectileSwitch(short side_index, short projectile_index)
{
	if (GetTrigger("projectile_switch"))
	{
		Lua_Projectile::Push(State(), projectile_index);
		Lua_Side::Push(State(), side_index);
		CallTrigger(2);
	}
}

void LuaState::TerminalEnter(short terminal_id, short player_index)
{
	if (GetTrigger("terminal_enter"))
	{
		Lua_Terminal::Push(State(), terminal_id);
		Lua_Player::Push(State(), player_index);
		CallTrigger(2);
	}
}

void LuaState::TerminalExit(short terminal_id, short player_index)
{
	if (GetTrigger("terminal_exit"))
	{
		Lua_Terminal::Push(State(), terminal_id);
		Lua_Player::Push(State(), player_index);
		CallTrigger(2);
	}
}

void LuaState::PatternBuffer(short side_index, short player_index)
{
	if (GetTrigger("pattern_buffer"))
	{
		Lua_Side::Push(State(), side_index);
		Lua_Player::Push(State(), player_index);
		CallTrigger(2);
	}
}

void LuaState::GotItem(short type, short player_index)
{
	if (GetTrigger("got_item"))
	{
		Lua_ItemType::Push(State(), type);
		Lua_Player::Push(State(), player_index);
		CallTrigger(2);
	}
}

void LuaState::LightActivated(short index)
{
	if (GetTrigger("light_activated"))
	{
		Lua_Light::Push(State(), index);
		CallTrigger(1);
	}
}

void LuaState::PlatformActivated(short index)
{
	if (GetTrigger("platform_activated"))
	{
		Lua_Polygon::Push(State(), index);
		CallTrigger(1);
	}
}

void LuaState::PlayerRevived (short player_index)
{
	if (GetTrigger("player_revived"))
	{
		Lua_Player::Push(State(), player_index);
		CallTrigger(1);
	}
}

void LuaState::PlayerKilled (short player_index, short aggressor_player_index, short action, short projectile_index)
{
	if (GetTrigger("player_killed"))
	{
		Lua_Player::Push(State(), player_index);

		if (aggressor_player_index != -1)
			Lua_Player::Push(State(), aggressor_player_index);
		else
			lua_pushnil(State());

		Lua_MonsterAction::Push(State(), action);
		if (projectile_index != -1)
			Lua_Projectile::Push(State(), projectile_index);
		else
			lua_pushnil(State());

		CallTrigger(4);
	}
}

void LuaState::MonsterKilled (short monster_index, short aggressor_player_index, short projectile_index)
{
	if (GetTrigger("monster_killed"))
	{
		Lua_Monster::Push(State(), monster_index);
		if (aggressor_player_index != -1)
			Lua_Player::Push(State(), aggressor_player_index);
		else
			lua_pushnil(State());

		if (projectile_index != -1)
			Lua_Projectile::Push(State(), projectile_index);
		else
			lua_pushnil(State());

		CallTrigger(3);
	}
}

void LuaState::MonsterDamaged(short monster_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	if (GetTrigger("monster_damaged"))
	{
		Lua_Monster::Push(State(), monster_index);
		if (aggressor_monster_index != -1) 
			Lua_Monster::Push(State(), aggressor_monster_index);
		else
			lua_pushnil(State());
		
		Lua_DamageType::Push(State(), damage_type);
		lua_pushnumber(State(), damage_amount);
		
		if (projectile_index != -1) 
			Lua_Projectile::Push(State(), projectile_index);
		else
			lua_pushnil(State());

		CallTrigger(5);
	}
		
}

void LuaState::PlayerDamaged (short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	if (GetTrigger("player_damaged"))
	{
		Lua_Player::Push(State(), player_index);

		if (aggressor_player_index != -1)
			Lua_Player::Push(State(), aggressor_player_index);
		else
			lua_pushnil(State());
		
		if (aggressor_monster_index != -1)
			Lua_Monster::Push(State(), aggressor_monster_index);
		else
			lua_pushnil(State());
		
		Lua_DamageType::Push(State(), damage_type);
		lua_pushnumber(State(), damage_amount);
		
		if (projectile_index != -1)
			Lua_Projectile::Push(State(), projectile_index);
		else
			lua_pushnil(State());

		CallTrigger(6);
	}
}

void LuaState::ProjectileDetonated(short type, short owner_index, short polygon, world_point3d location, uint16_t flags, int16_t obstruction_index, int16_t line_index) 
{
	if (GetTrigger("projectile_detonated"))
	{
		Lua_ProjectileType::Push(State(), type);
		if (owner_index != -1)
			Lua_Monster::Push(State(), owner_index);
		else
			lua_pushnil(State());
		Lua_Polygon::Push(State(), polygon);
		lua_pushnumber(State(), location.x / (double)WORLD_ONE);
		lua_pushnumber(State(), location.y / (double)WORLD_ONE);
		lua_pushnumber(State(), location.z / (double)WORLD_ONE);

		if (flags & _projectile_hit_monster)
		{
			auto object = get_object_data(obstruction_index);
			Lua_Monster::Push(State(), object->permutation);
		}
		else if (flags & _projectile_hit_floor)
		{
			Lua_Polygon_Floor::Push(State(), polygon);
		}
		else if (flags & _projectile_hit_media)
		{
			Lua_Polygon::Push(State(), polygon);
		}
		else if (flags & _projectile_hit_scenery)
		{
			Lua_Scenery::Push(State(), obstruction_index);
		}
		else if (obstruction_index != NONE)
		{
			Lua_Polygon_Ceiling::Push(State(), polygon);
		}
		else if (flags & _projectile_hit)
		{
			auto side_index = find_adjacent_side(polygon, line_index);
			Lua_Side::Push(State(), side_index);
		}
		else
		{
			lua_pushnil(State());
		}

		CallTrigger(7);
	}
}

void LuaState::ProjectileCreated (short projectile_index)
{
	if (GetTrigger("projectile_created"))
	{
		Lua_Projectile::Push(State(), projectile_index);
		CallTrigger(1);
	}
}

void LuaState::ItemCreated (short item_index)
{
	if (GetTrigger("item_created"))
	{
		Lua_Item::Push(State(), item_index);
		CallTrigger(1);
	}
}

void LuaState::InvalidateEffect(short effect_index)
{
	if (!running_) return;

	Lua_Effect::Invalidate(State(), effect_index);
}

void LuaState::InvalidateMonster(short monster_index)
{
	if (!running_) return;

	Lua_Monster::Invalidate(State(), monster_index);
}

void LuaState::InvalidateProjectile(short projectile_index)
{
	if (!running_) return;

	Lua_Projectile::Invalidate(State(), projectile_index);
}

void LuaState::InvalidateObject(short object_index)
{
	if (!running_) return;

	object_data *object = GetMemberWithBounds(objects, object_index, MAXIMUM_OBJECTS_PER_MAP);
	if (GET_OBJECT_OWNER(object) == _object_is_item)
	{
		Lua_Item::Invalidate(State(), object_index);
	}
	else if (Lua_Scenery::Valid(object_index))
	{
		Lua_Scenery::Invalidate(State(), object_index);
	}
}

void LuaState::InvalidateEphemera(short ephemera_index)
{
	if (!running_) return;

	Lua_Ephemera::Invalidate(State(), ephemera_index);
}

static char L_SEARCH_PATH_KEY[] = "search_path";

void L_Set_Search_Path(lua_State* L, const std::string& path)
{
	lua_pushlightuserdata(L, reinterpret_cast<void*>(L_SEARCH_PATH_KEY));
	lua_pushstring(L, path.c_str());
	lua_settable(L, LUA_REGISTRYINDEX);	
}

std::string L_Get_Search_Path(lua_State* L)
{
	lua_pushlightuserdata(L, reinterpret_cast<void*>(L_SEARCH_PATH_KEY));
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isstring(L, -1))
	{
		std::string path = lua_tostring(L, -1);
		lua_pop(L, 1);
		return path;
	}
	else
	{
		lua_pop(L, 1);
		return std::string();
	}

}

static char L_PROPER_ITEM_ACCOUNTING_KEY[] = "proper_item_accounting";

void L_Set_Proper_Item_Accounting(lua_State* L, bool value)
{
	lua_pushlightuserdata(L, reinterpret_cast<void*>(L_PROPER_ITEM_ACCOUNTING_KEY));
	lua_pushboolean(L, value);
	lua_settable(L, LUA_REGISTRYINDEX);
}

bool L_Get_Proper_Item_Accounting(lua_State* L)
{
	bool value = false;
	lua_pushlightuserdata(L, reinterpret_cast<void*>(L_PROPER_ITEM_ACCOUNTING_KEY));
	lua_gettable(L, LUA_REGISTRYINDEX);
	if (lua_isboolean(L, -1))
	{
		value = lua_toboolean(L, -1);
	}
	
	lua_pop(L, 1);
	return value;
}

static char L_NONLOCAL_OVERLAYS_KEY[] = "nonlocal_overlays";

void L_Set_Nonlocal_Overlays(lua_State* L, bool value)
{
  SetScriptHUDNonlocal(value);
}

bool L_Get_Nonlocal_Overlays(lua_State* L)
{
  return IsScriptHUDNonlocal();
}

static int L_Enable_Player(lua_State*);
static int L_Disable_Player(lua_State*);
static int L_Kill_Script(lua_State*);
static int L_Hide_Interface(lua_State*);
static int L_Show_Interface(lua_State*);
static int L_Player_Control(lua_State*);

void LuaState::RegisterFunctions()
{
	lua_register(State(), "enable_player", L_Enable_Player);
	lua_register(State(), "disable_player", L_Disable_Player);
	lua_register(State(), "kill_script", L_Kill_Script);
	lua_register(State(), "hide_interface", L_Hide_Interface);
	lua_register(State(), "show_interface", L_Show_Interface);
	lua_register(State(), "player_control", L_Player_Control);
//	lua_register(state, "prompt", L_Prompt);

	Lua_Ephemera_register(State());
	Lua_Map_register(State());
	Lua_Monsters_register(State());
	Lua_Objects_register(State());
	Lua_Player_register(State());
	Lua_Projectiles_register(State());
	Lua_Saved_Objects_register(State());
}

static const char *compatibility_triggers = ""
	"Triggers = {}\n"
	"Triggers.init = function(restoring_game) if init then init(restoring_game) end end\n"
	"Triggers.cleanup = function() if cleanup then cleanup() end end\n"
	"Triggers.idle = function() if idle then idle() end end\n"
	"Triggers.postidle = function() if postidle then postidle() end end\n"
	"Triggers.start_refuel = function(class, player, side) if start_refuel then start_refuel(class.index, player.index) end end\n"
	"Triggers.end_refuel = function(class, player, side) if end_refuel then end_refuel(class.index, player.index) end end\n"
	"Triggers.tag_switch = function(tag, player) if tag_switch then tag_switch(tag.index, player.index) end end\n"
	"Triggers.light_switch = function(light, player) if light_switch then light_switch(light.index, player.index) end end\n"
	"Triggers.platform_switch = function(platform, player) if platform_switch then platform_switch(platform.index, player.index) end end\n"
	"Triggers.terminal_enter = function(terminal, player) if terminal_enter then terminal_enter(terminal.index, player.index) end end\n"
	"Triggers.terminal_exit = function(terminal, player) if terminal_exit then terminal_exit(terminal.index, player.index) end end\n"
	"Triggers.pattern_buffer = function(side, player) if pattern_buffer then pattern_buffer(side.control_panel.permutation, player.index) end end\n"
	"Triggers.got_item = function(type, player) if got_item then got_item(type.index, player.index) end end\n"
	"Triggers.light_activated = function(light) if light_activated then light_activated(light.index) end end\n"
	"Triggers.platform_activated = function(polygon) if platform_activated then platform_activated(polygon.index) end end\n"
	"Triggers.player_revived = function(player) if player_revived then player_revived(player.index) end end\n"
	"Triggers.player_killed = function(player, aggressor, action, projectile) if player_killed then if aggressor then aggressor_index = aggressor.index else aggressor_index = -1 end if projectile then projectile_index = projectile.index else projectile_index = -1 end player_killed(player.index, aggressor_index, action.index, projectile_index) end end\n"
	"Triggers.monster_killed = function(monster, aggressor, projectile) if monster_killed then if aggressor then aggressor_index = aggressor.index else aggressor_index = -1 end if projectile then projectile_index = projectile.index else projectile_index = -1 end monster_killed(monster.index, aggressor_index, projectile_index) end end\n"
	"Triggers.player_damaged = function(player, aggressor_player, aggressor_monster, type, amount, projectile) if player_damaged then if aggressor_player then aggressor_player_index = aggressor_player.index else aggressor_player_index = -1 end if aggressor_monster then aggressor_monster_index = aggressor_monster.index else aggressor_monster_index = -1 end if projectile then projectile_index = projectile.index else projectile_index = -1 end player_damaged(player.index, aggressor_player_index, aggressor_monster_index, type.index, amount, projectile_index) end end\n"
	"Triggers.projectile_detonated = function(type, owner, polygon, x, y, z) if projectile_detonated then if owner then owner_index = owner.index else owner_index = -1 end projectile_detonated(type.index, owner_index, polygon.index, x, y, z) end end\n"
	"Triggers.item_created = function(item) if item_created then item_created(item.index) end end\n"
	;

void LuaState::LoadCompatibility()
{
	luaL_loadbuffer(State(), compatibility_triggers, strlen(compatibility_triggers), "compatibility_triggers");
	lua_pcall(State(), 0, 0, 0);

	struct lang_def
	{
		const char *name;
		int value;
	};
	struct lang_def constant_list[] = {
#include "language_definition.h"
	};

	int constant_list_size = sizeof(constant_list)/sizeof(lang_def);
	for (int i=0; i<constant_list_size; i++)
	{
		lua_pushnumber(State(), constant_list[i].value);
		lua_setglobal(State(), constant_list[i].name);
	}
/* SB: Don't think this is a constant? */
	lua_pushnumber(State(), MAXIMUM_MONSTERS_PER_MAP);
	lua_setglobal(State(), "MAXIMUM_MONSTERS_PER_MAP");
	lua_pushnumber(State(), MAXIMUM_PROJECTILES_PER_MAP);
	lua_setglobal(State(), "MAXIMUM_PROJECTILES_PER_MAP");
	lua_pushnumber(State(), MAXIMUM_OBJECTS_PER_MAP);
	lua_setglobal(State(), "MAXIMUM_OBJECTS_PER_MAP");
}

bool LuaState::Load(const char *buffer, size_t len, const char *desc)
{
	int status = luaL_loadbufferx(State(), buffer, len, desc, "t");
	if (status == LUA_ERRRUN)
		logWarning("Lua loading failed: error running script.");
	if (status == LUA_ERRFILE)
		logWarning("Lua loading failed: error loading file.");
	if (status == LUA_ERRSYNTAX) {
		logWarning("Lua loading failed: syntax error.");
		logWarning(lua_tostring(State(), -1));
	}
	if (status == LUA_ERRMEM)
		logWarning("Lua loading failed: error allocating memory.");
	if (status == LUA_ERRERR)
		logWarning("Lua loading failed: unknown error.");

	num_scripts_ += ((status == 0) ? 1 : 0);
	return (status == 0);
}


bool LuaState::Run()
{
	if (!Loaded()) return false;

	int result = 0;
	// Reverse the functions we're calling
	for (int i = 0; i < num_scripts_ - 1; ++i)
		lua_insert(State(), -(num_scripts_ - i));

	// Call 'em
	for (int i = 0; i < num_scripts_; ++i)
	{
		int ret = lua_pcall(State(), 0, LUA_MULTRET, 0);
		if (ret != 0)
		{
			L_Error(lua_tostring(State(), -1));
			result = ret;
			break;
		}
	}
	
	if (result == 0) running_ = true;
	return (result == 0);
}

void LuaState::ExecuteCommand(const std::string& line)
{

	std::string buffer;
	bool print_result = false;
	if (line[0] == '=') 
	{
		buffer = "return " + line.substr(1);
		print_result = true;
	}
	else
	{
		buffer = line;
	}


	if (luaL_loadbuffer(State(), buffer.c_str(), buffer.size(), "console") != 0)
	{
		L_Error(lua_tostring(State(), -1));
	}
	else 
	{
		running_ = true;
		if (lua_pcall(State(), 0, (print_result) ? 1 : 0, 0) != 0)
			L_Error(lua_tostring(State(), -1));
		else if (print_result)
		{
			lua_getglobal(State(), "tostring");
			lua_insert(State(), 1);
			lua_pcall(State(), 1, 1, 0);
			if (lua_tostring(State(), -1))
			{
				screen_printf("%s", lua_tostring(State(), -1));
			}
		}
	}
	
	lua_settop(State(), 0);	
}

extern bool can_load_collection(short);

// pass by pointer because boost::bind can't do non-const past 2nd argument
void LuaState::MarkCollections(std::set<short>* collections)
{
	if (!running_)
		return;
		
	lua_getglobal(State(), "CollectionsUsed");
	if (lua_istable(State(), -1))
	{
		int i = 1;
		lua_pushnumber(State(), i++);
		lua_gettable(State(), -2);
		while (lua_isnumber(State(), -1))
		{
			short collection_index = static_cast<short>(lua_tonumber(State(), -1));
			if (can_load_collection(collection_index))
			{
				mark_collection_for_loading(collection_index);
				collections->insert(collection_index);
			}
			lua_pop(State(), 1);
			lua_pushnumber(State(), i++);
			lua_gettable(State(), -2);
		}
			
		lua_pop(State(), 2);
	}
	else if (lua_isnumber(State(), -1))
	{
		short collection_index = static_cast<short>(lua_tonumber(State(), -1));
		if (can_load_collection(collection_index))
		{
			mark_collection_for_loading(collection_index);
			collections->insert(collection_index);
		}

		lua_pop(State(), 1);
	}
	else
	{
		lua_pop(State(), 1);
	}
}

int LuaState::RestoreAll(const std::string& s)
{
	if (s.empty())
	{
		lua_pushboolean(State(), false);
		return 1;
	}

	std::stringbuf sb(s);
	if (lua_restore(State(), &sb))
	{
		lua_pushlightuserdata(State(), L_Persistent_Table_Key());
		lua_insert(State(), -2);
		lua_settable(State(), LUA_REGISTRYINDEX); // muahaha
		lua_pushboolean(State(), true);
	} 
	else
	{
		lua_pop(State(), 2);
		lua_pushboolean(State(), false);
	}

	return 1;
}

int LuaState::RestorePassed(const std::string& s)
{
	if (s.empty())
	{
		lua_pushboolean(State(), false);
		return 1;
	}

	std::stringbuf sb(s);
	if (lua_restore(State(), &sb))
	{
		lua_pushlightuserdata(State(), L_Persistent_Table_Key());
		lua_gettable(State(), LUA_REGISTRYINDEX);
		
		lua_getfield(State(), -2, "player");
		lua_setfield(State(), -2, "player");

		lua_getfield(State(), -2, "Game");
		lua_setfield(State(), -2, "Game");

		lua_pop(State(), 2);

		lua_pushboolean(State(), true);
	}
	else
	{
		lua_settop(State(), 0);
		lua_pushboolean(State(), false);
	}

	return 1;
}

std::string LuaState::SaveAll()
{
	std::string retval;
	static const char key = 'k';

	lua_pushlightuserdata(State(), L_Persistent_Table_Key());
	lua_gettable(State(), LUA_REGISTRYINDEX);

	// keep the ephemera fields in a temp location
	lua_pushlightuserdata(State(), const_cast<char*>(&key));
	lua_getfield(State(), -2, Lua_Ephemera_Name);
	lua_settable(State(), LUA_REGISTRYINDEX);

	// remove it from the table while we save
	lua_pushnil(State());
	lua_setfield(State(), -2, Lua_Ephemera_Name);

	std::stringbuf sb;
	if (lua_save(State(), &sb))
	{
		retval = sb.str();
	}

	// restore the ephemera fields
	lua_pushlightuserdata(State(), const_cast<char*>(&key));
	lua_gettable(State(), LUA_REGISTRYINDEX);
	lua_setfield(State(), -2, Lua_Ephemera_Name);

	lua_pop(State(), 1);

	return retval;
}

std::string LuaState::SavePassed()
{
	// copy "player" and "Game" custom fields to a new table
	lua_pushlightuserdata(State(), reinterpret_cast<void*>(L_Persistent_Table_Key()));
	lua_gettable(State(), LUA_REGISTRYINDEX);
	
	lua_newtable(State());
	lua_getfield(State(), -2, "player");
	lua_setfield(State(), -2, "player");

	lua_getfield(State(), -2, "Game");
	lua_setfield(State(), -2, "Game");
	
	lua_remove(State(), -2);

	std::stringbuf sb;
	if (lua_save(State(), &sb))
	{
		return sb.str();
	} 
	else
	{
		return std::string();
	}
}

typedef boost::ptr_map<int, LuaState> state_map;
state_map states;

// globals
std::vector<lua_camera> lua_cameras;
std::unordered_map<std::string, std::string> lua_stash;

uint32 *action_flags;

// For better_random
GM_Random lua_random_generator;
GM_Random lua_random_local_generator;

double FindLinearValue(double startValue, double endValue, double timeRange, double timeTaken)
{
	return (((endValue-startValue)/timeRange)*timeTaken)+startValue;
}

world_point3d FindLinearValue(world_point3d startPoint, world_point3d endPoint, double timeRange, double timeTaken)
{
	world_point3d realPoint;
	realPoint.x = static_cast<int16>(((double)(endPoint.x-startPoint.x)/timeRange)*timeTaken)+startPoint.x;
	realPoint.y = static_cast<int16>(((double)(endPoint.y-startPoint.y)/timeRange)*timeTaken)+startPoint.y;
	realPoint.z = static_cast<int16>(((double)(endPoint.z-startPoint.z)/timeRange)*timeTaken)+startPoint.z;
	return realPoint;
}

void
L_Error(const char* inMessage)
{
	if (!mute_lua) screen_printf("%s", inMessage);
	logError(inMessage);
}

/*
static bool
L_Should_Call(const char* inLuaFunctionName)
{
	if (!lua_running)
		return false;
	
	lua_pushstring(state, inLuaFunctionName);
	lua_gettable(state, LUA_GLOBALSINDEX);

	if (!lua_isfunction(state, -1))
	{
		lua_pop(state, 1);
		return false;
	}

	return true;
}

static void
L_Do_Call(const char* inLuaFunctionName, int inNumArgs = 0, int inNumResults = 0)
{
	if (lua_pcall(state, inNumArgs, inNumResults, 0)==LUA_ERRRUN)
		L_Error(lua_tostring(state,-1));
}
*/

static bool LuaRunning()
{
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		if (it->second->Running())
		{
			return true;
		}
	}	

	return false;
}

// call f on each Lua state
template<class UnaryFunction>
void L_Dispatch(const UnaryFunction& f)
{
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		f(it->second);
	}
}

void L_Call_Init(bool fRestoringSaved)
{
	if (LuaRunning())
	{
		// jkvw: Seeding our better random number
		// generator from the lousy one is clearly not
		// ideal, but it should be good enough for our
		// purposes.
		uint16 current_seed = get_random_seed();
		lua_random_generator.z = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.w = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jsr = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jcong = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		if (!film_profile.lua_increments_rng)
			set_random_seed(current_seed);
		
	}

	L_Dispatch(boost::bind(&LuaState::Init, _1, fRestoringSaved));
}

void L_Call_Cleanup ()
{
	L_Dispatch(boost::bind(&LuaState::Cleanup, _1));
}

void UpdateLuaCameras();

void L_Call_Idle()
{
	UpdateLuaCameras();
	L_Dispatch(boost::bind(&LuaState::Idle, _1));
}

void L_Call_PostIdle()
{
	L_Dispatch(boost::bind(&LuaState::PostIdle, _1));
}

void L_Call_Start_Refuel (short type, short player_index, short panel_side_index)
{
	L_Dispatch(boost::bind(&LuaState::StartRefuel, _1, type, player_index, panel_side_index));
}

void L_Call_End_Refuel (short type, short player_index, short panel_side_index)
{
	L_Dispatch(boost::bind(&LuaState::EndRefuel, _1, type, player_index, panel_side_index));
}

void L_Call_Tag_Switch(short tag, short player_index, short side_index)
{
	L_Dispatch(boost::bind(&LuaState::TagSwitch, _1, tag, player_index, side_index));
}

void L_Call_Light_Switch(short light, short player_index, short side_index)
{
	L_Dispatch(boost::bind(&LuaState::LightSwitch, _1, light, player_index, side_index));
}

void L_Call_Platform_Switch(short platform, short player_index, short side_index)
{
	L_Dispatch(boost::bind(&LuaState::PlatformSwitch, _1, platform, player_index, side_index));
}

void L_Call_Projectile_Switch(short side_index, short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::ProjectileSwitch, _1, side_index, projectile_index));
}

void L_Call_Terminal_Enter(short terminal_id, short player_index)
{
	L_Dispatch(boost::bind(&LuaState::TerminalEnter, _1, terminal_id, player_index));
}

void L_Call_Terminal_Exit(short terminal_id, short player_index)
{
	L_Dispatch(boost::bind(&LuaState::TerminalExit, _1, terminal_id, player_index));
}

void L_Call_Pattern_Buffer(short side_index, short player_index)
{
	L_Dispatch(boost::bind(&LuaState::PatternBuffer, _1, side_index, player_index));
}

void L_Call_Got_Item(short type, short player_index)
{
	L_Dispatch(boost::bind(&LuaState::GotItem, _1, type, player_index));
}

void L_Call_Light_Activated(short index)
{
	L_Dispatch(boost::bind(&LuaState::LightActivated, _1, index));
}

void L_Call_Platform_Activated(short index)
{
	L_Dispatch(boost::bind(&LuaState::PlatformActivated, _1, index));
}

void L_Call_Player_Revived (short player_index)
{
	L_Dispatch(boost::bind(&LuaState::PlayerRevived, _1, player_index));
}

void L_Call_Player_Killed (short player_index, short aggressor_player_index, short action, short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::PlayerKilled, _1, player_index, aggressor_player_index, action, projectile_index));
}

void L_Call_Monster_Killed (short monster_index, short aggressor_player_index, short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::MonsterKilled, _1, monster_index, aggressor_player_index, projectile_index));
}

void L_Call_Monster_Damaged(short monster_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::MonsterDamaged, _1, monster_index, aggressor_monster_index, damage_type, damage_amount, projectile_index));
}

void L_Call_Player_Damaged (short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::PlayerDamaged, _1, player_index, aggressor_player_index, aggressor_monster_index, damage_type, damage_amount, projectile_index));
}

void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location, uint16_t flags, int16_t obstruction_index, int16_t line_index) 
{
	L_Dispatch(boost::bind(&LuaState::ProjectileDetonated, _1, type, owner_index, polygon, location, flags, obstruction_index, line_index));
}

void L_Call_Projectile_Created (short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::ProjectileCreated, _1, projectile_index));
}

void L_Call_Item_Created (short item_index)
{
	L_Dispatch(boost::bind(&LuaState::ItemCreated, _1, item_index));
}

void L_Invalidate_Effect(short effect_index)
{
	L_Dispatch(boost::bind(&LuaState::InvalidateEffect, _1, effect_index));
}

void L_Invalidate_Monster(short monster_index)
{
	L_Dispatch(boost::bind(&LuaState::InvalidateMonster, _1, monster_index));
}

void L_Invalidate_Projectile(short projectile_index)
{
	L_Dispatch(boost::bind(&LuaState::InvalidateProjectile, _1, projectile_index));
}

void L_Invalidate_Object(short object_index)
{
	L_Dispatch(boost::bind(&LuaState::InvalidateObject, _1, object_index));
}

void L_Invalidate_Ephemera(short ephemera_index)
{
	L_Dispatch(boost::bind(&LuaState::InvalidateEphemera, _1, ephemera_index));
}

int L_Enable_Player(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "enable_player: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "enable_player: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);

	if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
		return 0;
	SET_PLAYER_ZOMBIE_STATUS(player, false);
	return 0;
}

int L_Disable_Player(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "disable_player: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "disable_player: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);

	if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
		return 0;
	SET_PLAYER_ZOMBIE_STATUS(player, true);
	return 0;
}

int L_Kill_Script(lua_State *L)
{
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		if (it->second->Matches(L)) {
			it->second->Stop();
		}
	}

	return 0;
}

int L_Hide_Interface(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "hide_interface: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));

	if (local_player_index != player_index)
		return 0;

	screen_mode_data *the_mode;
	the_mode = get_screen_mode();
	if(the_mode->hud)
	{
		the_mode->hud = false;
		change_screen_mode(the_mode,true);
	}

	return 0;
}

int L_Restore_Saved(lua_State *L)
{
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		if (it->second->Matches(L))
		{
			return it->second->RestoreAll(SavedLuaState[it->first]);
		}
	}
	
	return 0;
}

int L_Restore_Passed(lua_State *L)
{
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		if (it->second->Matches(L))
		{
			return it->second->RestorePassed(PassedLuaState[it->first]);
		}
	}
	
	return 0;
}

int L_Show_Interface(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "show_interface: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));

	if (local_player_index != player_index)
		return 0;

	screen_mode_data *the_mode;
	the_mode = get_screen_mode();
	if (!the_mode->hud)
	{
		the_mode->hud = true;
		change_screen_mode(the_mode,true);
		draw_panels();
	}

	return 0;
}

#if TIENNOU_PLAYER_CONTROL
enum
{
	move_player = 1,
	turn_player,
	look_at
};
#endif

int L_Player_Control(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "player_control: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "player_control: invalid player index");
		lua_error(L);
	}
	int move_type = static_cast<int>(lua_tonumber(L,2));
	int value = static_cast<int>(lua_tonumber(L,3));
#if TIENNOU_PLAYER_CONTROL
	player_data *player = get_player_data(player_index);
#endif

	if (sLuaActionQueues == NULL)
	{
		sLuaActionQueues = new ActionQueues(MAXIMUM_NUMBER_OF_PLAYERS, ACTION_QUEUE_BUFFER_DIAMETER, true);
	}

	// Put the enqueuing of the action flags in one place in the code,
 // so it will be easier to change if necessary
 // LP: changed to reallocate-on-expand code;
 // "static" values are remembered from one call to the next
	static uint32 *action_flags = NULL;
	static int prev_value = 0;
	if (value > prev_value)
	{
		if (action_flags) delete []action_flags;
		action_flags = new uint32[value];
	}
	assert(action_flags);
	prev_value = value;

	bool DoAction = false;
#if TIENNOU_PLAYER_CONTROL
	struct physics_variables variables;
	struct physics_variables *variablesptr;
	struct physics_constants *constants= get_physics_constants_for_model(static_world->physics_model, action_flags);
	variables = player->variables;
	*variablesptr = variables;
#endif

	switch(move_type)
	{
#if TIENNOU_PLAYER_CONTROL
		case move_player:
			if (!lua_isnumber(L,3) || !lua_isnumber(L,4) || !lua_isnumber(L,5))
			{
				lua_pushstring(L, "player_control: incorrect argument type for move_player");
				lua_error(L);
			}

			world_point3d goal_point;
			world_point3d current_point;
			current_point = player->location;
			goal_point.x = static_cast<int>(lua_tonumber(L,3)*WORLD_ONE);
			goal_point.y = static_cast<int>(lua_tonumber(L,4)*WORLD_ONE);
			goal_point.z = static_cast<int>(lua_tonumber(L,5)*WORLD_ONE);
			angle heading;
			heading = arctangent((current_point.y - goal_point.y), (current_point.x - goal_point.x));
			angle current_heading;
			current_heading = player->facing;
			_fixed delta;

			if (current_heading < heading)
			{
				screen_printf("Player heading is on the right of the goal_point");
				// turn_left
				while (current_heading <= heading)
				{
					action_flags[0] = _turning_left;
					//action_flag_count++;
     //GetPfhortranActionQueues()->enqueueActionFlags(player_index, action_flags, 1);
					physics_update(constants, variablesptr, player, action_flags);
					instantiate_physics_variables(constants, variablesptr, player_index, false);
				}
			}
				else if (current_heading > heading)
				{
					screen_printf("Player heading is on the left of the goal_point");
					// turn_right
					while (player->facing >= heading)
					{
						action_flags[0] = _turning_right;
						//action_flag_count++;
		    //GetPfhortranActionQueues()->enqueueActionFlags(player_index, action_flags, 1);
						physics_update(constants, variablesptr, player, action_flags);
						instantiate_physics_variables(constants, variablesptr, player_index, false);
					}
				}

				if (current_point.x < goal_point.x)
				{
					screen_printf("goal_point is in front of player");

					/*
					 while (current_point.x > goal_point.x)
					 {
						 action_flags[0] = _moving_forward;
						 //action_flag_count++;
						 GetPfhortranActionQueues()->enqueueActionFlags(player_index, action_flags, 1);
					 }
					 */
				}
				else if (current_point.x > goal_point.x)
				{
					screen_printf("goal_point is behind player");
					/*
					 while (current_point.x < goal_point.x)
					 {
						 action_flags[0] = _moving_backward;
						 //action_flag_count++;
						 GetPfhortranActionQueues()->enqueueActionFlags(player_index, action_flags, 1);
					 }
					 */
				}

				break;

		case turn_player:
			if (!lua_isnumber(L,3) || !lua_isnumber(L,4))
			{
				lua_pushstring(L, "player_control: incorrect argument type for turn_player");
				lua_error(L);
			}

			angle new_facing;
			angle new_elevation;
			new_facing = static_cast<int16>(lua_tonumber(L,3));
			new_elevation = static_cast<int16>(lua_tonumber(L,4));

			if (player->facing < new_facing)
			{
				screen_printf("new_facing is on right of the player heading");
			}
				else if (player->facing > new_facing)
				{
					screen_printf("new_facing is on left of the player heading");
				}

				if (player->elevation < new_elevation)
				{
					screen_printf("new_elevation is above player elevation");
				}
				else if (player->elevation > new_elevation)
				{
					screen_printf("new_elevation is under player elevation");
				}

				break;

		case look_at:
			if (!lua_isnumber(L,3) || !lua_isnumber(L,4) || !lua_isnumber(L,5))
			{
				lua_pushstring(L, "player_control: incorrect argument type for look_at");
				lua_error(L);
			}

			world_point3d look;
			look.x = static_cast<int16>(lua_tonumber(L, 3));
			look.y = static_cast<int16>(lua_tonumber(L, 4));
			look.z = static_cast<int16>(lua_tonumber(L, 5));

			player->facing = arctangent(ABS(look.x-player->location.x), ABS(look.y-player->location.y));
			player->elevation = arctangent(ABS(look.x-player->location.x),ABS(look.z-player->location.z));

			break;
#endif
#if !TIENNOU_PLAYER_CONTROL
		case 0:
			action_flags[0] = _moving_forward;
			DoAction = true;
			break;

		case 1:
			action_flags[0] = _moving_backward;
			DoAction = true;
			break;

		case 2:
			action_flags[0] = _sidestepping_left;
			DoAction = true;
			break;

		case 3:
			action_flags[0] = _sidestepping_right;
			DoAction = true;
			break;

		case 4:
			action_flags[0] = _turning_left;
			DoAction = true;
			break;

		case 5:
			action_flags[0] = _turning_right;
			DoAction = true;
			break;

		case 6:
			action_flags[0] = _looking_up;
			DoAction = true;
			break;

		case 7:
			action_flags[0] = _looking_down;
			DoAction = true;
			break;

		case 8:
			action_flags[0] = _action_trigger_state;
			DoAction = true;
			break;

		case 9:
			action_flags[0] = _left_trigger_state;
			DoAction = true;
			break;

		case 10:
			action_flags[0] = _right_trigger_state;
			DoAction = true;
			break;

		case 13: // reset action queue
			GetLuaActionQueues()->reset();
			break;
#endif
		default:
			break;
	}

	if (DoAction)
	{
		for (int i=1; i<value; i++)
			action_flags[i] = action_flags[0];

		GetLuaActionQueues()->enqueueActionFlags(player_index, action_flags, value);
	}
	return 0;
}
/*
static void L_Prompt_Callback(const std::string& str) {
  if(L_Should_Call("prompt_callback"))
   {
		lua_pushnumber(state, local_player_index);
		lua_pushstring(state, str.c_str());
		L_Do_Call("prompt_callback", 2);
   }
}

static int L_Prompt(lua_State *L)
{
	if(dynamic_world->player_count > 1) {
		lua_pushstring(L, "prompt: Not implemented for network play");
		lua_error(L);
		}
	if(!lua_isnumber(L,1) || !lua_isstring(L,2))
	 {
		lua_pushstring(L, "prompt: incorrect argument type");
		lua_error(L);
	 }
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	 {
		lua_pushstring(L, "prompt: invalid player index");
		lua_error(L);
	 }
	if((Console::instance())->input_active()) {
		lua_pushstring(L, "prompt: prompt already active");
		lua_error(L);
		}
	(Console::instance())->activate_input(L_Prompt_Callback, lua_tostring(L, 2));
	return 0;
}
*/



static LuaState* LuaStateFactory(ScriptType script_type)
{
	switch (script_type) {
	case _embedded_lua_script:
		return new EmbeddedLuaState;
	case _lua_netscript:
		return new NetscriptState;
	case _solo_lua_script:
		return new SoloScriptState;
	case _stats_lua_script:
		return new StatsLuaState;
	}
	return NULL;
}

bool LoadLuaScript(const char *buffer, size_t len, ScriptType script_type)
{
	assert(script_type >= _embedded_lua_script && script_type <= _stats_lua_script);
	if (states.find(script_type) == states.end())
	{
		int type = script_type;
		states.insert(type, LuaStateFactory(script_type));
		states[script_type].Initialize();
	}
	const char *desc = "level_script";
	switch (script_type) {
		case _embedded_lua_script:
			desc = "Map Lua";
			break;
		case _lua_netscript:
			desc = "Netscript";
			break;
		case _solo_lua_script:
			desc = "Solo Lua";
			break;
		case _stats_lua_script:
			desc = "Stats Lua";
			break;
	}
	return states[script_type].Load(buffer, len, desc);
}

#ifdef HAVE_OPENGL
static OGL_FogData PreLuaFogState[OGL_NUMBER_OF_FOG_TYPES];
#endif
static bool MotionSensorWasActive;

static void PreservePreLuaSettings()
{
#ifdef HAVE_OPENGL
	for (int i = 0; i < OGL_NUMBER_OF_FOG_TYPES; i++) 
	{
		PreLuaFogState[i] = *OGL_GetFogData(i);
	}
#endif
	MotionSensorWasActive = MotionSensorActive;
}

static void InitializeLuaVariables()
{
	for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; ++i)
	{
		use_lua_compass[i] = false;
		can_wield_weapons[i] = true;
	}
	
	game_end_condition = _game_normal_end_condition;
	game_scoring_mode = _game_of_most_points;
}

static void RestorePreLuaSettings()
{
#ifdef HAVE_OPENGL
	for (int i = 0; i < OGL_NUMBER_OF_FOG_TYPES; i++)
	{
		*OGL_GetFogData(i) = PreLuaFogState[i];
	}
#endif
	MotionSensorActive = MotionSensorWasActive;
}

bool RunLuaScript()
{
	InitializeLuaVariables();
	PreservePreLuaSettings();

	lua_random_local_generator.z = (static_cast<uint32>(local_random()) << 16) + static_cast<uint32>(local_random());
	lua_random_local_generator.w = (static_cast<uint32>(local_random()) << 16) + static_cast<uint32>(local_random());
	lua_random_local_generator.jsr = (static_cast<uint32>(local_random()) << 16) + static_cast<uint32>(local_random());
	lua_random_local_generator.jcong = (static_cast<uint32>(local_random()) << 16) + static_cast<uint32>(local_random());

	bool running = false;
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		running |= it->second->Run();
	}

	return running;
}

void ExecuteLuaString(const std::string& line)
{
	if (states.find(_solo_lua_script) == states.end())
	{
		int type = _solo_lua_script;
		states.insert(type, LuaStateFactory(_solo_lua_script));
		states[_solo_lua_script].Initialize();
	}

	exit_interpolated_world();
	states[_solo_lua_script].ExecuteCommand(line);
	enter_interpolated_world();
}

void LoadSoloLua()
{
	std::string file;
	std::string directory;

	if (environment_preferences->use_solo_lua) 
	{
		file = environment_preferences->solo_lua_file;
	}
	else
	{
		const Plugin* solo_lua_plugin = Plugins::instance()->find_solo_lua();
		if (solo_lua_plugin)
		{
			file = solo_lua_plugin->solo_lua;
			directory = solo_lua_plugin->directory.GetPath();
		}
	}

	if (file.size())
	{
		FileSpecifier fs (file.c_str());
		if (directory.size())
		{
			fs.SetNameWithPath(file.c_str(), directory);
		}

		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			int32 script_length;
			script_file.GetLength(script_length);

			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaScript(&script_buffer[0], script_length, _solo_lua_script);
				if (directory.size())
				{
					states[_solo_lua_script].SetSearchPath(directory);
				}
			}
		}
	}
}

void LoadStatsLua()
{
	std::string file;
	std::string directory;

	const Plugin* stats_lua_plugin = Plugins::instance()->find_stats_lua();
	if (stats_lua_plugin)
	{
		file = stats_lua_plugin->stats_lua;
		directory = stats_lua_plugin->directory.GetPath();
	}

	if (file.size())
	{
		FileSpecifier fs(file.c_str());
		if (directory.size())
		{
			fs.SetNameWithPath(file.c_str(), directory);
		}

		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			int32 script_length;
			script_file.GetLength(script_length);
			
			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaScript(&script_buffer[0], script_length, _stats_lua_script);
				if (directory.size())
				{
					states[_stats_lua_script].SetSearchPath(directory);
				}
			}
			
		}
	}
}

bool CollectLuaStats(std::map<std::string, std::string>& options, std::map<std::string, std::string>& parameters)
{
	if (states.find(_stats_lua_script) == states.end())
	{
		return false;
	}

	LuaState& state = states[_stats_lua_script];
	lua_State* L = state.State();
	
	if (!state.Running()) 
		return false;
	
	options.clear();
	parameters.clear();
	
	lua_getglobal(L, "Statistics");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return false;
	}
	
	// grab all the keys/values
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			options[lua_tostring(L, -2)] = lua_tostring(L, -1);
		}
		lua_pop(L, 1);
	}
	
	// grab the parameters
	lua_pushstring(L, "parameters");
	lua_gettable(L, -2);
	if (lua_istable(L, -1))
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			if (lua_type(L, -2) == LUA_TSTRING)
			{
				if (lua_tostring(L, -1))
				{
					parameters[lua_tostring(L, -2)] = lua_tostring(L, -1);
				}
				// lua_tostring only works with numbers and strings
				else if (lua_isboolean(L, -1))
				{
					parameters[lua_tostring(L, -2)] = (lua_toboolean(L, -1) ? "true" : "false");
				}
			}
			lua_pop(L, 1);
		}
	}
	
	// pop the Parameters table, or the nil
	lua_pop(L, 1);
	
	// pop the Statistics table
	lua_pop(L, 1);
	
	return true;
}

void LoadReplayNetLua()
{
	std::string file;
	std::string directory;
	
	if (environment_preferences->use_replay_net_lua)
	{
		file = network_preferences->netscript_file;
	}
	
	if (file.size())
	{
		FileSpecifier fs (file.c_str());
		if (directory.size())
		{
			fs.SetNameWithPath(file.c_str(), directory);
		}
		
		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			int32 script_length;
			script_file.GetLength(script_length);
			
			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaScript(&script_buffer[0], script_length, _lua_netscript);
				if (directory.size())
				{
					states[_lua_netscript].SetSearchPath(directory);
				}
			}
		}
	}
}

void CloseLuaScript()
{
	// save variables for going into next level
	PassedLuaState.clear();
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		PassedLuaState[it->first] = it->second->SavePassed();
	}
	states.clear();

	SavedLuaState.clear();

	RestorePreLuaSettings();

	lua_cameras.resize(0);
        lua_stash.clear();

	LuaTexturePaletteClear();

	game_end_condition = _game_normal_end_condition;
	
}

void ResetPassedLua()
{
	PassedLuaState.clear();
}

void ToggleLuaMute()
{
	mute_lua = !mute_lua;
	if (mute_lua)
	{
		screen_printf("adding Lua messages to the ignore list");
	} 
	else
	{
		screen_printf("removing Lua messages from the ignore list");
	}
}

void ResetLuaMute()
{
	mute_lua = false;
}

void MarkLuaCollections(bool loading)
{
	static std::set<short> collections;
	if (loading)
	{
		collections.clear();

		L_Dispatch(boost::bind(&LuaState::MarkCollections, _1, &collections));
	}
	else
	{
		for (std::set<short>::iterator it = collections.begin(); it != collections.end(); it++)
		{
			mark_collection_for_unloading(*it);
		}
	}
}

void UpdateLuaCameras()
{
	if (!LuaRunning())
		return;

	for (std::vector<lua_camera>::iterator it = lua_cameras.begin(); it != lua_cameras.end(); ++it)
	{
		if (it->player_active != local_player_index)
		{
			continue;
		}

		short point_index = it->path.current_point_index;
		short angle_index = it->path.current_angle_index;

		it->time_elapsed++;
			
		if (point_index >= 0 && it->time_elapsed - it->path.last_point_time >= it->path.path_points[point_index].delta_time)
		{
			it->path.current_point_index++;
			it->path.last_point_time = it->time_elapsed;
			if (it->path.current_point_index >= static_cast<short>(it->path.path_points.size()))
			{
				it->path.current_point_index = -1;
			}
		}
		
		if (angle_index >= 0 && it->time_elapsed - it->path.last_angle_time >= it->path.path_angles[angle_index].delta_time)
		{
			it->path.current_angle_index++;
			it->path.last_angle_time = it->time_elapsed;
			if (it->path.current_angle_index >= static_cast<short>(it->path.path_angles.size()))
			{
				it->path.current_angle_index = -1;
			}
		}
	}
}

bool UseLuaCameras()
{
	if (!LuaRunning())
		return false;

	bool using_lua_cameras = false;
	for (std::vector<lua_camera>::iterator it = lua_cameras.begin(); it != lua_cameras.end(); ++it)
	{
		if (it->player_active != local_player_index)
		{
			continue;
		}

		world_view->show_weapons_in_hand = false;
		using_lua_cameras = true;
		
		short point_index = it->path.current_point_index;
		short angle_index = it->path.current_angle_index;
		
		if (angle_index >= 0 && angle_index < static_cast<short>(it->path.path_angles.size()))
		{
			if (static_cast<size_t>(angle_index) == it->path.path_angles.size() - 1)
			{
				world_view->yaw = normalize_angle(it->path.path_angles[angle_index].yaw);
				world_view->pitch = normalize_angle(it->path.path_angles[angle_index].pitch);
			}
			else
			{
				world_view->yaw = normalize_angle(static_cast<short>(FindLinearValue(it->path.path_angles[angle_index].yaw, it->path.path_angles[angle_index+1].yaw, it->path.path_angles[angle_index].delta_time, it->time_elapsed - it->path.last_angle_time)));
				world_view->pitch = normalize_angle(static_cast<short>(FindLinearValue(it->path.path_angles[angle_index].pitch, it->path.path_angles[angle_index+1].pitch, it->path.path_angles[angle_index].delta_time, it->time_elapsed - it->path.last_angle_time)));
			}
		}

		if (point_index >= 0 && point_index < static_cast<short>(it->path.path_points.size()))
		{
			if (static_cast<size_t>(point_index) == it->path.path_points.size() - 1)
			{
				world_view->origin = it->path.path_points[point_index].point;
				world_view->origin_polygon_index = it->path.path_points[point_index].polygon;
			}
			else
			{
				world_point3d oldPoint = it->path.path_points[point_index].point;
				world_view->origin = FindLinearValue(it->path.path_points[point_index].point, it->path.path_points[point_index+1].point, it->path.path_points[point_index].delta_time, it->time_elapsed - it->path.last_point_time);
				world_point3d newPoint = world_view->origin;
				short polygon = it->path.path_points[point_index].polygon;
				ShootForTargetPoint(true, oldPoint, newPoint, polygon);
				world_view->origin_polygon_index = polygon;
			}
		}	
	}

	return using_lua_cameras;
}

bool LuaPlayerCanWieldWeapons(short player_index)
{
	return (can_wield_weapons[player_index] || !LuaRunning());
}		

int GetLuaGameEndCondition() {
	return game_end_condition;
}

size_t save_lua_states()
{
	size_t length = 0;

	SavedLuaState.clear();
	for (state_map::iterator it = states.begin(); it != states.end(); ++it)
	{
		SavedLuaState[it->first] = it->second->SaveAll();
		if (SavedLuaState[it->first].size())
		{
			length += 6; // id, length
			length += SavedLuaState[it->first].size();
		}
	}

	return length;
}

void pack_lua_states(uint8* data, size_t length)
{
	io::stream_buffer<io::array_sink> sb(reinterpret_cast<char*>(data), length);
	BOStreamBE s(&sb);
	for (std::map<int, std::string>::iterator it = SavedLuaState.begin(); it != SavedLuaState.end(); ++it)
	{
		if (it->second.size())
		{
			s << static_cast<int16>(it->first);
			s << static_cast<uint32>(it->second.size());
			s.write(&it->second[0], it->second.size());
		}
	}

	SavedLuaState.clear();
}

void unpack_lua_states(uint8* data, size_t length)
{
	io::stream_buffer<io::array_source> sb(reinterpret_cast<char*>(data), length);
	BIStreamBE s(&sb);

	SavedLuaState.clear();
	while (s.tellg() != s.maxg())
	{
		int16 index;
		uint32 length;
		s >> index 
		  >> length;

		SavedLuaState[index].resize(length);
		s.read(&SavedLuaState[index][0], SavedLuaState[index].size());
	}
}
#endif /* HAVE_LUA */
