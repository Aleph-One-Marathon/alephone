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
world_point2d lua_compass_beacons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
short lua_compass_states[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

//static ActionQueues* sLuaActionQueues = 0;
static ActionQueues* sLuaActionQueues = 0;
ActionQueues* GetLuaActionQueues() { return sLuaActionQueues; }

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
void L_Call_Pattern_Buffer(short buffer_id, short player_index) {}
void L_Call_Got_Item(short type, short player_index) {}
void L_Call_Light_Activated(short index) {}
void L_Call_Platform_Activated(short index) {}
void L_Call_Player_Revived(short player_index) {}
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index) {}
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index) {}
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index) {}
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location) {}
void L_Call_Item_Created(short item_index) {}

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

#else /* HAVE_LUA */

// LP: used by several functions here
const float AngleConvert = 360/float(FULL_CIRCLE);

static bool mute_lua = false;

// Steal all this stuff
extern bool ready_weapon(short player_index, short weapon_index);
extern void DisplayText(short BaseX, short BaseY, char *Text, unsigned char r = 0xff, unsigned char g = 0xff, unsigned char b = 0xff);


extern void ShootForTargetPoint(bool ThroughWalls, world_point3d& StartPosition, world_point3d& EndPosition, short& Polygon);
extern void select_next_best_weapon(short player_index);
extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void draw_panels();

extern bool MotionSensorActive;

extern void destroy_players_ball(short player_index);

extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void instantiate_physics_variables(struct physics_constants *constants, struct physics_variables *variables, short player_index, bool first_time, bool take_action);

extern void add_object_to_polygon_object_list(short object_index, short polygon_index);

enum // control panel sounds
{
	_activating_sound,
	_deactivating_sound,
	_unusuable_sound,
	
	NUMBER_OF_CONTROL_PANEL_SOUNDS
};

struct control_panel_definition
{
	int16 _class;
	uint16 flags;
	
	int16 collection;
	int16 active_shape, inactive_shape;

	int16 sounds[NUMBER_OF_CONTROL_PANEL_SOUNDS];
	_fixed sound_frequency;
	
	int16 item;
};

extern control_panel_definition *get_control_panel_definition(const short control_panel_type);

extern ActionQueues *sPfhortranActionQueues;
extern struct view_data *world_view;
extern struct static_data *static_world;
static short old_size;

// globals
lua_State *state;
bool lua_loaded = false;
bool lua_running = false;
static vector<short> IntersectedObjects;

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

static void
L_Call(const char* inLuaFunctionName)
{
	if(L_Should_Call(inLuaFunctionName))
		L_Do_Call(inLuaFunctionName);
}

static void
L_Call_N(const char* inLuaFunctionName, lua_Number inArg1)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		L_Do_Call(inLuaFunctionName, 1);
	}
}

static void
L_Call_NN(const char* inLuaFunctionName, lua_Number inArg1, lua_Number inArg2)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		lua_pushnumber(state, inArg2);
		L_Do_Call(inLuaFunctionName, 2);
	}
}

static void
L_Call_NNN(const char* inLuaFunctionName, lua_Number inArg1, lua_Number inArg2, lua_Number inArg3)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		lua_pushnumber(state, inArg2);
		lua_pushnumber(state, inArg3);
		L_Do_Call(inLuaFunctionName, 3);
	}
}

static void
L_Call_NNNN(const char* inLuaFunctionName, lua_Number inArg1, lua_Number inArg2, lua_Number inArg3, lua_Number inArg4)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		lua_pushnumber(state, inArg2);
		lua_pushnumber(state, inArg3);
		lua_pushnumber(state, inArg4);
		L_Do_Call(inLuaFunctionName, 4);
	}
}

#if 0
static void
L_Call_NNNNN(const char* inLuaFunctionName, lua_Number inArg1, lua_Number inArg2, lua_Number inArg3, lua_Number inArg4, lua_Number inArg5)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		lua_pushnumber(state, inArg2);
		lua_pushnumber(state, inArg3);
		lua_pushnumber(state, inArg4);
		lua_pushnumber(state, inArg5);
		L_Do_Call(inLuaFunctionName, 5);
	}
}
#endif

static void
L_Call_NNNNNN(const char* inLuaFunctionName, lua_Number inArg1, lua_Number inArg2, lua_Number inArg3, lua_Number inArg4, lua_Number inArg5, lua_Number inArg6)
{
	if(L_Should_Call(inLuaFunctionName))
	{
		lua_pushnumber(state, inArg1);
		lua_pushnumber(state, inArg2);
		lua_pushnumber(state, inArg3);
		lua_pushnumber(state, inArg4);
		lua_pushnumber(state, inArg5);
		lua_pushnumber(state, inArg6);
		L_Do_Call(inLuaFunctionName, 6);
	}
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
	if (L_Should_Call("init")) {
		lua_pushboolean(state, fRestoringSaved);
		L_Do_Call("init", 1);
	}
}

void L_Call_Cleanup ()
{
	L_Call("cleanup");
}

void L_Call_Idle()
{
	L_Call("idle");
}

void L_Call_PostIdle()
{
	L_Call("postidle");
}

void L_Call_Sent_Message(const char* player, const char* message) {
  if(L_Should_Call("sent_message"))
   {
	lua_pushstring(state, player);
	lua_pushstring(state, message);
	L_Do_Call("sent_message", 2);
   }
}

void L_Call_Start_Refuel (short type, short player_index, short panel_side_index)
{
	// ZZZ: Preserving existing behavior which is to only pass along two of these parameters
	L_Call_NN("start_refuel", type, player_index);
}

void L_Call_End_Refuel (short type, short player_index, short panel_side_index)
{
	// ZZZ: Preserving existing behavior which is to only pass along two of these parameters
	L_Call_NN("end_refuel", type, player_index);
}

void L_Call_Tag_Switch(short tag, short player_index)
{
	L_Call_NN("tag_switch", tag, player_index);
}

void L_Call_Light_Switch(short light, short player_index)
{
	L_Call_NN("light_switch", light, player_index);
}

void L_Call_Platform_Switch(short platform, short player_index)
{
	L_Call_NN("platform_switch", platform, player_index);
}

void L_Call_Terminal_Enter(short terminal_id, short player_index)
{
	L_Call_NN("terminal_enter", terminal_id, player_index);
}

void L_Call_Terminal_Exit(short terminal_id, short player_index)
{
	L_Call_NN("terminal_exit", terminal_id, player_index);
}

void L_Call_Pattern_Buffer(short buffer_id, short player_index)
{
	L_Call_NN("pattern_buffer", buffer_id, player_index);
}

void L_Call_Got_Item(short type, short player_index)
{
	L_Call_NN("got_item", type, player_index);
}

void L_Call_Light_Activated(short index)
{
	L_Call_N("light_activated", index);
}

void L_Call_Platform_Activated(short index)
{
	L_Call_N("platform_activated", index);
}

void L_Call_Player_Revived (short player_index)
{
	L_Call_N("player_revived", player_index);
}

void L_Call_Player_Killed (short player_index, short aggressor_player_index, short action, short projectile_index)
{
	L_Call_NNNN("player_killed", player_index, aggressor_player_index, action, projectile_index);
}

void L_Call_Monster_Killed (short monster_index, short aggressor_player_index, short projectile_index)
{
	L_Call_NNN("monster_killed", monster_index, aggressor_player_index, projectile_index);
}

//  Woody Zenfell, 08/03/03
void L_Call_Player_Damaged (short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index)
{
	L_Call_NNNNNN("player_damaged", player_index, aggressor_player_index, aggressor_monster_index, damage_type, damage_amount, projectile_index);
}

/* can't use a L_Call function for this */
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location) {
  if(L_Should_Call("projectile_detonated"))
   {
	lua_pushnumber(state, type);
	lua_pushnumber(state, owner_index);
	lua_pushnumber(state, polygon);
	lua_pushnumber(state, location.x / (double)WORLD_ONE);
	lua_pushnumber(state, location.y / (double)WORLD_ONE);
	lua_pushnumber(state, location.z / (double)WORLD_ONE);
	L_Do_Call("projectile_detonated", 6);
   }
}

void L_Call_Item_Created (short item_index)
{
	L_Call_N("item_created", item_index);
}

static int L_Screen_Print(lua_State *L)
{
	int args = lua_gettop(L);
	
	if (args == 2)
	{
		if (!lua_isnumber(L,1) || !lua_isstring(L,2))
		{
			lua_pushstring(L, "screen_print: incorrect argument type");
			lua_error(L);
		}
		int player_index = static_cast<int>(lua_tonumber(L,1));
		if (local_player_index != player_index)
			return 0;
		if (!mute_lua) screen_printf("%s", lua_tostring(L, 2));
	} else {
		if (!mute_lua) screen_printf("%s", lua_tostring(L, 1));
	}

	return 0;
}
/*
 static int L_Display_Text(lua_State *L)
 {
	 if (!lua_isnumber(L,1) || !lua_isstring(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,4))
	 {
		 lua_pushstring(L, "display_text: incorrect argument type");
		 lua_error(L);
	 }
	 int player_index = static_cast<int>(lua_tonumber(L,1));
	 if (local_player_index != player_index)
		 return 0;
	 short x = static_cast<int>(lua_tonumber(L,3));
	 short y = static_cast<int>(lua_tonumber(L,4));
	 DisplayText(x, y, lua_tostring(L, 2));
	 screen_printf(lua_tostring(L,2));
	 return 0;
 }
 */

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

static int L_Create_Camera(lua_State *L)
{
	number_of_cameras++;
	lua_cameras.resize(number_of_cameras);
	lua_cameras[number_of_cameras-1].index = number_of_cameras-1;
	lua_cameras[number_of_cameras-1].path.index = number_of_cameras-1;
	lua_cameras[number_of_cameras-1].path.current_point_index = 0;
	lua_cameras[number_of_cameras-1].path.current_angle_index = 0;
	lua_cameras[number_of_cameras-1].path.last_point_time = 0;
	lua_cameras[number_of_cameras-1].path.last_angle_time = 0;
	lua_cameras[number_of_cameras-1].time_elapsed = 0;
	lua_cameras[number_of_cameras-1].player_active = -1;
	lua_pushnumber(L, number_of_cameras-1);
	return 1;
}

static int L_Add_Path_Point(lua_State *L)
{
	if (lua_gettop(L) < 6)
	{
		lua_pushstring(L, "add_path_point: too few arguments");
		lua_error(L);
	}
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "add_path_point: incorrect argument type");
		lua_error(L);
	}
	int camera_index = static_cast<int>(lua_tonumber(L,1));
	if (camera_index > number_of_cameras-1)
	{
		lua_pushstring(L, "add_path_point: bad camera index");
		lua_error(L);
	}
	int polygon = static_cast<int>(lua_tonumber(L,2));
	world_point3d point = {
		static_cast<world_distance>(lua_tonumber(L,3)*WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L,4)*WORLD_ONE),
		static_cast<world_distance>(lua_tonumber(L,5)*WORLD_ONE)
	};
	long time = static_cast<long>(lua_tonumber(L,6));
	int point_index = lua_cameras[camera_index].path.path_points.size();
	lua_cameras[camera_index].path.path_points.resize(point_index+1);
	lua_cameras[camera_index].path.path_points[point_index].polygon = polygon;
	lua_cameras[camera_index].path.path_points[point_index].point = point;
	lua_cameras[camera_index].path.path_points[point_index].delta_time = time;
	return 0;
}

static int L_Add_Path_Angle(lua_State *L)
{
	if (lua_gettop(L) < 4)
	{
		lua_pushstring(L, "add_path_angle: too few arguments");
		lua_error(L);
	}
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "add_path_angle: incorrect argument type");
		lua_error(L);
	}
	int camera_index = static_cast<int>(lua_tonumber(L,1));
	if (camera_index > number_of_cameras-1)
	{
		lua_pushstring(L, "add_path_angle: bad camera index");
		lua_error(L);
	}
	short yaw = static_cast<short>(lua_tonumber(L,2));
	short pitch = static_cast<short>(lua_tonumber(L,3));
	long time = static_cast<long>(lua_tonumber(L,4));
	int angle_index = lua_cameras[camera_index].path.path_angles.size();

	lua_cameras[camera_index].path.path_angles.resize(angle_index+1);
	lua_cameras[camera_index].path.path_angles[angle_index].yaw = static_cast<short>(yaw/AngleConvert);
	lua_cameras[camera_index].path.path_angles[angle_index].pitch = static_cast<short>(pitch/AngleConvert);
	lua_cameras[camera_index].path.path_angles[angle_index].delta_time = time;
	return 0;
}

static int L_Activate_Camera(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "activate_camera: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (local_player_index != player_index)
		return 0;
	int camera_index = static_cast<int>(lua_tonumber(L,2));
	if (camera_index > number_of_cameras-1)
	{
		lua_pushstring(L, "activate_camera: bad camera index");
		lua_error(L);
	}
	lua_cameras[camera_index].time_elapsed = 0;
	lua_cameras[camera_index].player_active = player_index;
	lua_cameras[camera_index].path.current_point_index = 0;
	lua_cameras[camera_index].path.current_angle_index = 0;
	lua_cameras[camera_index].path.last_point_time = 0;
	lua_cameras[camera_index].path.last_angle_time = 0;
	return 0;
}

static int L_Deactivate_Camera(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "deactivate_camera: incorrect argument type");
		lua_error(L);
	}
	int camera_index = static_cast<int>(lua_tonumber(L,1));
	if (camera_index > number_of_cameras-1)
	{
		lua_pushstring(L, "deactivate_camera: bad camera index");
		lua_error(L);
	}
	lua_cameras[camera_index].time_elapsed = 0;
	lua_cameras[camera_index].player_active = -1;
	lua_cameras[camera_index].path.last_point_time = 0;
	lua_cameras[camera_index].path.last_angle_time = 0;
	return 0;
}

static int L_Clear_Camera(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "clear_camera: incorrect argument type");
		lua_error(L);
	}
	int camera_index = static_cast<int>(lua_tonumber(L,1));
	if (camera_index > number_of_cameras-1)
	{
		lua_pushstring(L, "clear_camera: bad camera index");
		lua_error(L);
	}
	lua_cameras[camera_index].path.path_points.resize(0);
	lua_cameras[camera_index].path.path_angles.resize(0);
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

void RegisterLuaFunctions()
{
	lua_register(state, "screen_print", L_Screen_Print);
	//lua_register(state, "display_text", L_Display_Text);
	lua_register(state, "enable_player", L_Enable_Player);
	lua_register(state, "disable_player", L_Disable_Player);
	lua_register(state, "kill_script", L_Kill_Script);
	lua_register(state, "hide_interface", L_Hide_Interface);
	lua_register(state, "show_interface", L_Show_Interface);
	lua_register(state, "player_control", L_Player_Control);
	lua_register(state, "create_camera", L_Create_Camera);
	lua_register(state, "add_path_point", L_Add_Path_Point);
	lua_register(state, "add_path_angle", L_Add_Path_Angle);
	lua_register(state, "activate_camera", L_Activate_Camera);
	lua_register(state, "deactivate_camera", L_Deactivate_Camera);
	lua_register(state, "clear_camera", L_Clear_Camera);
	lua_register(state, "prompt", L_Prompt);

	Lua_Map_register(state);
	Lua_Monsters_register(state);
	Lua_Objects_register(state);
	Lua_Player_register(state);
	Lua_Projectiles_register(state);
}

void DeclareLuaConstants()
{
	struct lang_def
{
        char *name;
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
		use_lua_compass [i] = false;
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

#endif /* HAVE_LUA */
