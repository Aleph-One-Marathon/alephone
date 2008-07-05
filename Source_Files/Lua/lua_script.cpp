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
using namespace std;
#include <stdlib.h>
#include <set>

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

#include "lua_script.h"
#include "lua_map.h"
#include "lua_monsters.h"
#include "lua_objects.h"
#include "lua_player.h"
#include "lua_projectiles.h"

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
void L_Call_Tag_Switch(short tag, short player_index) {}
void L_Call_Light_Switch(short light, short player_index) {}
void L_Call_Platform_Switch(short platform, short player_index) {}
void L_Call_Terminal_Enter(short terminal_id, short player_index) {}
void L_Call_Terminal_Exit(short terminal_id, short player_index) {}
void L_Call_Pattern_Buffer(short side_index, short player_index) {}
void L_Call_Got_Item(short type, short player_index) {}
void L_Call_Light_Activated(short index) {}
void L_Call_Platform_Activated(short index) {}
void L_Call_Player_Revived(short player_index) {}
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index) {}
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index) {}
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index) {}
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location) {}
void L_Call_Item_Created(short item_index) {}

void L_Invalidate_Monster(short) { }
void L_Invalidate_Projectile(short) { }
void L_Invalidate_Object(short) { }

bool LoadLuaScript(const char *buffer, size_t len) { /* Should never get here! */ return false; }
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

// LP: used by several functions here
const float AngleConvert = 360/float(FULL_CIRCLE);

bool mute_lua = false;

// Steal all this stuff
extern void ShootForTargetPoint(bool ThroughWalls, world_point3d& StartPosition, world_point3d& EndPosition, short& Polygon);
extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void draw_panels();

extern bool MotionSensorActive;

extern void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);

extern struct view_data *world_view;
extern struct static_data *static_world;
static short old_size;

// globals
lua_State *state;
bool lua_loaded = false;
bool lua_running = false;

vector<lua_camera> lua_cameras;
int number_of_cameras = 0;

uint32 *action_flags;

// For better_random
GM_Random lua_random_generator;

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

static const luaL_reg lualibs[] =
{
	{"base", luaopen_base},
	{"table", luaopen_table},
//	{"io", luaopen_io}, //jkvw: This is just begging to be a security problem, isn't it?
	{"string", luaopen_string},
	{"math", luaopen_math},
	{"debug", luaopen_debug},
	// {"loadlib", luaopen_loadlib}, jkvw: Do you really want evil scripts calling loadlib?
	// jkvw: And don't even think about adding the operating system facilities library, burrito.
	/* add your libraries here */
	{NULL, NULL}
};

static void OpenStdLibs(lua_State* l)
{
	const luaL_reg *lib = lualibs;
	for (; lib->func; lib++)
	{
		lib->func(l);
		lua_settop(l,0);
	}
}

void
L_Error(const char* inMessage)
{
	if (!mute_lua) screen_printf("%s", inMessage);
	logError(inMessage);
}

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

static bool L_Get_Trigger(const char* inLuaTriggerName)
{
	if (!lua_running)
		return false;

	lua_pushstring(state, "Triggers");
	lua_gettable(state, LUA_GLOBALSINDEX);
	if (!lua_istable(state, -1))
	{
		lua_pop(state, 1);
		return false;
	}

	lua_pushstring(state, inLuaTriggerName);
	lua_gettable(state, -2);
	if (!lua_isfunction(state, -1))
	{
		lua_pop(state, 2);
		return false;
	}

	lua_remove(state, -2);
	return true;
}

static void L_Call_Trigger(int numArgs = 0)
{
	if (lua_pcall(state, numArgs, 0, 0)==LUA_ERRRUN)
		L_Error(lua_tostring(state,-1));
}

void L_Call_Init(bool fRestoringSaved)
{
	// jkvw: Seeding our better random number generator from the lousy one
	// is clearly not ideal, but it should be good enough for our purposes.
	if (lua_running) {
		lua_random_generator.z = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.w = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jsr = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jcong = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
	}
	if (L_Get_Trigger("init"))
	{
		lua_pushboolean(state, fRestoringSaved);
		L_Call_Trigger(1);
	}
}

void L_Call_Cleanup ()
{
	if (L_Get_Trigger("cleanup"))
	{
		L_Call_Trigger();
	}
}

void L_Call_Idle()
{
	if (L_Get_Trigger("idle"))
	{
		L_Call_Trigger();
	}
}

void L_Call_PostIdle()
{
	if (L_Get_Trigger("postidle"))
	{
		L_Call_Trigger();
	}
}

void L_Call_Start_Refuel (short type, short player_index, short panel_side_index)
{
	if (L_Get_Trigger("start_refuel"))
	{
		Lua_ControlPanelClass::Push(state, type);
		Lua_Player::Push(state, player_index);
		Lua_Side::Push(state, panel_side_index);
		L_Call_Trigger(3);
	}
}

void L_Call_End_Refuel (short type, short player_index, short panel_side_index)
{
	if (L_Get_Trigger("end_refuel"))
	{
		Lua_ControlPanelClass::Push(state, type);
		Lua_Player::Push(state, player_index);
		Lua_Side::Push(state, panel_side_index);
		L_Call_Trigger(3);
	}
}

void L_Call_Tag_Switch(short tag, short player_index)
{
	if (L_Get_Trigger("tag_switch"))
	{
		Lua_Tag::Push(state, tag);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Light_Switch(short light, short player_index)
{
	if (L_Get_Trigger("light_switch"))
	{
		Lua_Light::Push(state, light);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Platform_Switch(short platform, short player_index)
{
	if (L_Get_Trigger("platform_switch"))
	{
		Lua_Polygon::Push(state, platform);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Terminal_Enter(short terminal_id, short player_index)
{
	if (L_Get_Trigger("terminal_enter"))
	{
		Lua_Terminal::Push(state, terminal_id);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Terminal_Exit(short terminal_id, short player_index)
{
	if (L_Get_Trigger("terminal_exit"))
	{
		Lua_Terminal::Push(state, terminal_id);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Pattern_Buffer(short side_index, short player_index)
{
	if (L_Get_Trigger("pattern_buffer"))
	{
		Lua_Side::Push(state, side_index);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Got_Item(short type, short player_index)
{
	if (L_Get_Trigger("got_item"))
	{
		Lua_ItemType::Push(state, type);
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(2);
	}
}

void L_Call_Light_Activated(short index)
{
	if (L_Get_Trigger("light_activated"))
	{
		Lua_Light::Push(state, index);
		L_Call_Trigger(1);
	}
}

void L_Call_Platform_Activated(short index)
{
	if (L_Get_Trigger("platform_activated"))
	{
		Lua_Polygon::Push(state, index);
		L_Call_Trigger(1);
	}
}

void L_Call_Player_Revived (short player_index)
{
	if (L_Get_Trigger("player_revived"))
	{
		Lua_Player::Push(state, player_index);
		L_Call_Trigger(1);
	}
}

void L_Call_Player_Killed (short player_index, short aggressor_player_index, short action, short projectile_index)
{
	if (L_Get_Trigger("player_killed"))
	{
		Lua_Player::Push(state, player_index);

		if (aggressor_player_index != -1)
			Lua_Player::Push(state, aggressor_player_index);
		else
			lua_pushnil(state);

		Lua_MonsterAction::Push(state, action);
		if (projectile_index != -1)
			Lua_Projectile::Push(state, projectile_index);
		else
			lua_pushnil(state);

		L_Call_Trigger(4);
	}
}

void L_Call_Monster_Killed (short monster_index, short aggressor_player_index, short projectile_index)
{
	if (L_Get_Trigger("monster_killed"))
	{
		Lua_Monster::Push(state, monster_index);
		if (aggressor_player_index != -1)
			Lua_Player::Push(state, aggressor_player_index);
		else
			lua_pushnil(state);

		if (projectile_index != -1)
			Lua_Projectile::Push(state, projectile_index);
		else
			lua_pushnil(state);

		L_Call_Trigger(3);
	}
}

void L_Call_Player_Damaged (short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	if (L_Get_Trigger("player_damaged"))
	{
		Lua_Player::Push(state, player_index);

		if (aggressor_player_index != -1)
			Lua_Player::Push(state, aggressor_player_index);
		else
			lua_pushnil(state);
		
		if (aggressor_monster_index != -1)
			Lua_Monster::Push(state, aggressor_monster_index);
		else
			lua_pushnil(state);
		
		Lua_DamageType::Push(state, damage_type);
		lua_pushnumber(state, damage_amount);
		
		if (projectile_index != -1)
			Lua_Projectile::Push(state, projectile_index);
		else
			lua_pushnil(state);

		L_Call_Trigger(6);
	}
}

void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location) 
{
	if (L_Get_Trigger("projectile_detonated"))
	{
		Lua_ProjectileType::Push(state, type);
		if (owner_index != -1)
			Lua_Monster::Push(state, owner_index);
		else
			lua_pushnil(state);
		Lua_Polygon::Push(state, polygon);
		lua_pushnumber(state, location.x / (double)WORLD_ONE);
		lua_pushnumber(state, location.y / (double)WORLD_ONE);
		lua_pushnumber(state, location.z / (double)WORLD_ONE);

		L_Call_Trigger(6);
	}
}

void L_Call_Item_Created (short item_index)
{
	if (L_Get_Trigger("item_created"))
	{
		Lua_Item::Push(state, item_index);
		L_Call_Trigger(1);
	}
}

void L_Invalidate_Monster(short monster_index)
{
	if (!lua_running) return;

	Lua_Monster::Invalidate(state, monster_index);
}

void L_Invalidate_Projectile(short projectile_index)
{
	if (!lua_running) return;

	Lua_Projectile::Invalidate(state, projectile_index);
}

void L_Invalidate_Object(short object_index)
{
	if (!lua_running) return;

	object_data *object = GetMemberWithBounds(objects, object_index, MAXIMUM_OBJECTS_PER_MAP);
	if (GET_OBJECT_OWNER(object) == _object_is_item)
	{
		Lua_Item::Invalidate(state, object_index);
	}
	else if (GET_OBJECT_OWNER(object) == _object_is_effect)
	{
		Lua_Effect::Invalidate(state, object_index);
	}
	else if (Lua_Scenery::Valid(object_index))
	{
		Lua_Scenery::Invalidate(state, object_index);
	}
}

static int L_Enable_Player(lua_State *L)
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

static int L_Disable_Player(lua_State *L)
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

static int L_Kill_Script(lua_State *L)
{
	lua_running = false;
	return 0;
}

static int L_Hide_Interface(lua_State *L)
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
	old_size = the_mode->size;
	short new_size = GetSizeWithoutHUD(old_size);
	if(the_mode->size != new_size)
	{
		the_mode->size = new_size;
		change_screen_mode(the_mode,true);
	}

	return 0;
}

static int L_Show_Interface(lua_State *L)
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
	short the_size = the_mode->size;
	if(the_size == GetSizeWithHUD(the_size))
	{
		the_mode->size = old_size;
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

static int L_Player_Control(lua_State *L)
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

void RegisterLuaFunctions()
{
	lua_register(state, "enable_player", L_Enable_Player);
	lua_register(state, "disable_player", L_Disable_Player);
	lua_register(state, "kill_script", L_Kill_Script);
	lua_register(state, "hide_interface", L_Hide_Interface);
	lua_register(state, "show_interface", L_Show_Interface);
	lua_register(state, "player_control", L_Player_Control);
	lua_register(state, "prompt", L_Prompt);

	Lua_Map_register(state);
	Lua_Monsters_register(state);
	Lua_Objects_register(state);
	Lua_Player_register(state);
	Lua_Projectiles_register(state);

	luaL_loadbuffer(state, compatibility_triggers, strlen(compatibility_triggers), "compatibility_triggers");
	lua_pcall(state, 0, 0, 0);
}

void DeclareLuaConstants()
{
	struct lang_def
{
        const char *name;
        int value;
};
struct lang_def constant_list[] =
{
#include "language_definition.h"
};
int constant_list_size = sizeof(constant_list)/sizeof(lang_def);
for (int i=0; i<constant_list_size; i++)
{
        lua_pushnumber(state, constant_list[i].value);
        lua_setglobal(state, constant_list[i].name);
}
/* SB: Don't think this is a constant? */
lua_pushnumber(state, MAXIMUM_MONSTERS_PER_MAP);
lua_setglobal(state, "MAXIMUM_MONSTERS_PER_MAP");
lua_pushnumber(state, MAXIMUM_PROJECTILES_PER_MAP);
lua_setglobal(state, "MAXIMUM_PROJECTILES_PER_MAP");
lua_pushnumber(state, MAXIMUM_OBJECTS_PER_MAP);
lua_setglobal(state, "MAXIMUM_OBJECTS_PER_MAP");
}

// We want to allow MML to load multiple scripts, but we want to show an error if
// we try to load a netscript and another script.
// Netscirpt system will set gLoadingLuaNetscript true when it calls LoadLuaScript.
// Intended as a temporary hack, this should change once we get a system for
// simultaneous independant scripts.
bool gLoadingLuaNetscript = false;
static bool sLuaNetscriptLoaded = false;

static int numScriptsLoaded;

bool LoadLuaScript(const char *buffer, size_t len)
{
	int status;

	if (gLoadingLuaNetscript)
		sLuaNetscriptLoaded = true;

	if (lua_loaded == true && sLuaNetscriptLoaded)
	{
		// Probably the mml and netscript systems are fighting over control of lua
		show_cursor ();
		alert_user(infoError, strERRORS, luascriptconflict, 0);
		hide_cursor (); // this is bad bad badtz-maru if LoadLuaScript gets called when the pointer isn't supposed to be hidden
	} else if(!lua_loaded) {
		numScriptsLoaded = 0;
		state = lua_open();

		OpenStdLibs(state);

		RegisterLuaFunctions();
		DeclareLuaConstants();
	}

	status = luaL_loadbuffer(state, buffer, len, "level_script");
	if (status == LUA_ERRRUN)
		logWarning("Lua loading failed: error running script.");
	if (status == LUA_ERRFILE)
		logWarning("Lua loading failed: error loading file.");
	if (status == LUA_ERRSYNTAX) {
		logWarning("Lua loading failed: syntax error.");
		logWarning(lua_tostring(state, -1));
	}
	if (status == LUA_ERRMEM)
		logWarning("Lua loading failed: error allocating memory.");
	if (status == LUA_ERRERR)
		logWarning("Lua loading failed: unknown error.");

	lua_loaded = (status==0);
	numScriptsLoaded += lua_loaded;
	
	return lua_loaded;
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
	for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
	{
		use_lua_compass [i] = false;
		can_wield_weapons[i] = true;
	}
	if (!lua_loaded)
		return false;

	PreservePreLuaSettings();

	int result = 0;
	// Reverse the functions we're calling
	for(int i = 0; i < numScriptsLoaded - 1; ++i)
		lua_insert(state, -(numScriptsLoaded - i));
	// Call 'em
	for(int i = 0; i < numScriptsLoaded; ++i)
		result = result || lua_pcall(state, 0, LUA_MULTRET, 0);
	lua_running = (result==0);

	return lua_running;
}

void ExecuteLuaString(const std::string& line)
{
	if (!lua_loaded)
	{
		numScriptsLoaded = 0;
		state = lua_open();

		OpenStdLibs(state);
		RegisterLuaFunctions();
		DeclareLuaConstants();
		PreservePreLuaSettings();

		lua_loaded = true;
	}

	string buffer;
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


	if (luaL_loadbuffer(state, buffer.c_str(), buffer.size(), "console") != 0)
	{
		L_Error(lua_tostring(state, -1));
	}
	else 
	{
		if (!lua_running) lua_running = true;
		if (lua_pcall(state, 0, (print_result) ? 1 : 0, 0) != 0)
			L_Error(lua_tostring(state, -1));
		else if (print_result)
		{
			lua_getglobal(state, "tostring");
			lua_insert(state, 1);
			lua_pcall(state, 1, 1, 0);
			if (lua_tostring(state, -1))
			{
				screen_printf("%s", lua_tostring(state, -1));
			}
		}
	}
	
	lua_settop(state, 0);
}

void LoadSoloLua()
{
	if (environment_preferences->use_solo_lua)
	{
		FileSpecifier fs (environment_preferences->solo_lua_file);
		OpenedFile script_file;
		if (fs.Open(script_file))
		{
			long script_length;
			script_file.GetLength(script_length);

			std::vector<char> script_buffer(script_length);
			if (script_file.Read(script_length, &script_buffer[0]))
			{
				LoadLuaScript(&script_buffer[0], script_length);
			}
		}
	}
}

void CloseLuaScript()
{
	if (lua_loaded)
	{
		lua_close(state);
		
		RestorePreLuaSettings();
	}

	lua_loaded = false;
	lua_running = false;
	lua_cameras.resize(0);
	number_of_cameras = 0;

	LuaTexturePaletteClear();

	game_end_condition = _game_normal_end_condition;
	game_scoring_mode = _game_of_most_points;
	
	sLuaNetscriptLoaded = false;
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
	static set<short> lua_collections;

	if (loading)
	{
		lua_collections.clear();

		if (!lua_running)
			return;
		
		lua_pushstring(state, "CollectionsUsed");
		lua_gettable(state, LUA_GLOBALSINDEX);
		
		if (lua_istable(state, -1))
		{
			int i = 1;
			lua_pushnumber(state, i++);
			lua_gettable(state, -2);
			while (lua_isnumber(state, -1))
			{
				short collection_index = static_cast<short>(lua_tonumber(state, -1));
				if (collection_index >= 0 && collection_index < NUMBER_OF_COLLECTIONS)
				{
					mark_collection_for_loading(collection_index);
					lua_collections.insert(collection_index);
				}
				lua_pop(state, 1);
				lua_pushnumber(state, i++);
				lua_gettable(state, -2);
			}
			
			lua_pop(state, 2);
		}
		else if (lua_isnumber(state, -1))
		{
			short collection_index = static_cast<short>(lua_tonumber(state, -1));
			if (collection_index >= 0 && collection_index < NUMBER_OF_COLLECTIONS)
			{
				mark_collection_for_loading(collection_index);
				lua_collections.insert(collection_index);
			}

			lua_pop(state, 1);
		}
		else
		{
			lua_pop(state, 1);
		}
	}
	else
	{
		for (set<short>::iterator it = lua_collections.begin(); it != lua_collections.end(); it++)
		{
			mark_collection_for_unloading(*it);
		}
	}
}

bool UseLuaCameras()
{
	if (!lua_running)
		return false;

	bool using_lua_cameras = false;
	if (lua_cameras.size()>0)
	{
		for (int i=0; i<number_of_cameras; i++)
		{
			if (lua_cameras[i].player_active == local_player_index)
			{
				world_view->show_weapons_in_hand = false;
				using_lua_cameras = true;
				short point_index = lua_cameras[i].path.current_point_index;
				short angle_index = lua_cameras[i].path.current_angle_index;
				if (angle_index != -1 && static_cast<size_t>(angle_index) != lua_cameras[i].path.path_angles.size()-1)
				{
					world_view->yaw = normalize_angle(static_cast<short>(FindLinearValue(lua_cameras[i].path.path_angles[angle_index].yaw, lua_cameras[i].path.path_angles[angle_index+1].yaw, lua_cameras[i].path.path_angles[angle_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time)));
					world_view->pitch = normalize_angle(static_cast<short>(FindLinearValue(lua_cameras[i].path.path_angles[angle_index].pitch, lua_cameras[i].path.path_angles[angle_index+1].pitch, lua_cameras[i].path.path_angles[angle_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time)));
				}
				else if (static_cast<size_t>(angle_index) == lua_cameras[i].path.path_angles.size()-1)
				{
					world_view->yaw = normalize_angle(lua_cameras[i].path.path_angles[angle_index].yaw);
					world_view->pitch = normalize_angle(lua_cameras[i].path.path_angles[angle_index].pitch);
				}
				if (point_index != -1 && static_cast<size_t>(point_index) != lua_cameras[i].path.path_points.size()-1)
				{
					world_point3d oldPoint = lua_cameras[i].path.path_points[point_index].point;
					world_view->origin = FindLinearValue(lua_cameras[i].path.path_points[point_index].point, lua_cameras[i].path.path_points[point_index+1].point, lua_cameras[i].path.path_points[point_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_point_time);
					world_point3d newPoint = world_view->origin;
					short polygon = lua_cameras[i].path.path_points[point_index].polygon;
					ShootForTargetPoint(true, oldPoint, newPoint, polygon);
					world_view->origin_polygon_index = polygon;
				}
				else if (static_cast<size_t>(point_index) == lua_cameras[i].path.path_points.size()-1)
				{
					world_view->origin = lua_cameras[i].path.path_points[point_index].point;
					world_view->origin_polygon_index = lua_cameras[i].path.path_points[point_index].polygon;
				}

				lua_cameras[i].time_elapsed++;

				if (lua_cameras[i].time_elapsed - lua_cameras[i].path.last_point_time >= lua_cameras[i].path.path_points[point_index].delta_time)
				{
					lua_cameras[i].path.current_point_index++;
					lua_cameras[i].path.last_point_time = lua_cameras[i].time_elapsed;
					if (static_cast<size_t>(lua_cameras[i].path.current_point_index) >= lua_cameras[i].path.path_points.size())
						lua_cameras[i].path.current_point_index = -1;
				}
				if (lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time >= lua_cameras[i].path.path_angles[angle_index].delta_time)
				{
					lua_cameras[i].path.current_angle_index++;
					lua_cameras[i].path.last_angle_time = lua_cameras[i].time_elapsed;
					if (static_cast<size_t>(lua_cameras[i].path.current_angle_index) >= lua_cameras[i].path.path_angles.size())
						lua_cameras[i].path.current_angle_index = -1;
				}
			}
		}
	}
	return using_lua_cameras;
}

bool LuaPlayerCanWieldWeapons(short player_index)
{
	return (!lua_running || can_wield_weapons[player_index]);
}		

int GetLuaGameEndCondition() {
	return game_end_condition;
}

#endif /* HAVE_LUA */
