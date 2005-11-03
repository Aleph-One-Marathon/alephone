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
#include "mysound.h"
#include "world.h"
#include "computer_interface.h"
#include "network.h"
#include "network_games.h"
#include "Random.h"
#include "Console.h"
#include "music.h"

#include "script_instructions.h"
#include "lua_script.h"

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "monster_definitions.h"
#include "projectile_definitions.h"

bool use_lua_compass[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
world_point2d lua_compass_beacons[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];
short lua_compass_states[MAXIMUM_NUMBER_OF_NETWORK_PLAYERS];

#ifndef HAVE_LUA

void L_Call_Init() {}
void L_Call_Cleanup() {}
void L_Call_Idle() {}
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

bool UseLuaCameras() { return false; }

#else /* HAVE_LUA */

// LP: used by several functions here
const float AngleConvert = 360/float(FULL_CIRCLE);

// Steal all this stuff
extern bool ready_weapon(short player_index, short weapon_index);
extern void DisplayText(short BaseX, short BaseY, char *Text);

extern void advance_monster_path(short monster_index);
extern long monster_pathfinding_cost_function(short source_polygon_index, short line_index,
					      short destination_polygon_index, void *data);
extern void set_monster_action(short monster_index, short action);
extern void set_monster_mode(short monster_index, short new_mode, short target_index);

extern void ShootForTargetPoint(bool ThroughWalls, world_point3d& StartPosition, world_point3d& EndPosition, short& Polygon);
extern void select_next_best_weapon(short player_index);
extern struct physics_constants *get_physics_constants_for_model(short physics_model, uint32 action_flags);
extern void draw_panels();

extern vector<FileSpecifier> Playlist;
extern bool IsLevelMusicActive();

extern bool player_has_valid_weapon(short player_index);
extern player_weapon_data *get_player_weapon_data(const short player_index);

extern bool MotionSensorActive;

extern void destroy_players_ball(short player_index);

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

struct monster_pathfinding_data
{
	struct monster_definition *definition;
	struct monster_data *monster;

	bool cross_zone_boundaries;
};

extern ActionQueues *sPfhortranActionQueues;
extern struct view_data *world_view;
extern struct static_data *static_world;
extern short old_size;

// globals
lua_State *state;
bool lua_loaded = false;
bool lua_running = false;
static vector<short> IntersectedObjects;

vector<lua_camera> lua_cameras;
int number_of_cameras = 0;

uint32 *action_flags;

// For better_random
static GM_Random lua_random_generator;

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
	// {"io", luaopen_io}, jkvw: This is just begging to be a security problem, isn't it?
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

static void
L_Error(const char* inMessage)
{
	screen_printf(inMessage);
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

void L_Call_Init()
{
	// jkvw: Seeding our better random number generator from the lousy one
	// is clearly not ideal, but it should be good enough for our purposes.
	if (lua_running) {
		lua_random_generator.z = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.w = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jsr = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
		lua_random_generator.jcong = (static_cast<uint32>(global_random ()) << 16) + static_cast<uint32>(global_random ());
	}
	L_Call("init");
}

void L_Call_Cleanup ()
{
	L_Call("cleanup");
}

void L_Call_Idle()
{
	L_Call("idle");
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

static int L_Number_of_Polygons (lua_State *L)
{
	lua_pushnumber (L, dynamic_world->polygon_count);
	return 1;
}

static int L_Number_of_Players(lua_State *L)
{
	lua_pushnumber(L, dynamic_world->player_count);
	return 1;
}

static int L_Local_Player_Index(lua_State *L)
{
	lua_pushnumber(L, local_player_index);
	return 1;
}

static int L_Player_To_Monster_Index(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "player_to_monster_index: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "player_to_monster_index: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->monster_index);
	return 1;
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
		screen_printf(lua_tostring(L, 2));
	} else {
		screen_printf(lua_tostring(L, 1));
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
static int L_Inflict_Damage(lua_State *L)
{
	int args = lua_gettop(L);
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "inflict_damage: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "inflict_damage: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
		return 0;

	struct damage_definition damage;
	float temp = static_cast<int>(lua_tonumber(L,2));

	damage.flags= _alien_damage;  // jkvw: do we really want to set this flag?
	damage.type= _damage_crushing;
	damage.base= int16(temp);
	damage.random= 0;
	damage.scale= FIXED_ONE;

	if (args > 2)
	{
		if (!lua_isnumber(L,3))
		{
			lua_pushstring(L, "inflict_damage: incorrect argument type");
			lua_error(L);
		}
		damage.type = static_cast<int>(lua_tonumber(L,3));
	}

	damage_player(player->monster_index, NONE, NONE, &damage, NONE);

	return 0;
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
	if(the_size == GetSizeWithoutHUD(the_size))
	{
		the_mode->size = old_size;
		change_screen_mode(the_mode,true);
		draw_panels();
	}

	return 0;
}

static int L_Set_Tag_State(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_tag_state: incorrect argument type");
		lua_error(L);
	}
	int tag = static_cast<int>(lua_tonumber(L,1));
	bool tag_state = lua_toboolean(L,2);

	set_tagged_light_statuses(int16(tag), tag_state);
	try_and_change_tagged_platform_states(int16(tag), tag_state);
	assume_correct_switch_position(_panel_is_tag_switch, int16(tag), tag_state);

	return 0;
}

static int L_Get_Tag_State(lua_State *L)
{
	size_t light_index;
	struct light_data *light;
	struct platform_data *platform;
	short platform_index;

	bool changed= false;
	int tag;

	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_tag_state: incorrect argument type");
		lua_error(L);
	}
	tag = static_cast<int>(lua_tonumber(L,1));

	for (light_index= 0, light= lights; light_index<MAXIMUM_LIGHTS_PER_MAP; ++light_index, ++light)
	{
		if (light->static_data.tag==tag)
		{
			if (get_light_status(light_index))
			{
				changed= true;
			}
		}
	}


	if (!changed)
	{
		for (platform_index= 0, platform= platforms; platform_index<dynamic_world->platform_count; ++platform_index, ++platform)
		{
			if (platform->tag==tag)
			{
				if (PLATFORM_IS_ACTIVE(platform))
				{
					changed= true;
				}
			}
		}

	}

	lua_pushboolean(L, changed);
	return 1;
}

static int L_Get_Life(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_life: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_life: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->suit_energy);
	return 1;
}

static int L_Set_Life(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_life: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int energy = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_life: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	if (energy > 3*PLAYER_MAXIMUM_SUIT_ENERGY)
		energy = 3*PLAYER_MAXIMUM_SUIT_ENERGY;

	player->suit_energy = energy;
	mark_shield_display_as_dirty();

	return 0;
}

static int L_Get_Oxygen(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_oxygen: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_oxygen: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);

	lua_pushnumber(L, player->suit_oxygen);
	return 1;
}

static int L_Set_Oxygen(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_oxygen: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int oxygen = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_oxygen: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	if (oxygen > PLAYER_MAXIMUM_SUIT_OXYGEN)
		oxygen = PLAYER_MAXIMUM_SUIT_OXYGEN;

	player->suit_oxygen = oxygen;
	mark_shield_display_as_dirty();

	return 0;
}

static int L_Add_Item(lua_State *L)
{
	if (!lua_isnumber(L,1) || !static_cast<int>(lua_tonumber(L,2)))
	{
		lua_pushstring(L, "add_item: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int item = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "add_item: invalid player index");
		lua_error(L);
	}

	try_and_add_player_item(player_index, item);

	return 0;
}

static int L_Remove_Item(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "remove_item: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int item_type = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "remove_item: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);

	struct item_definition *definition= get_item_definition_external(item_type);

	if (definition)
	{
		if(player->items[item_type] >= 1)// && definition->item_kind==_ammunition)
		{   player->items[item_type]--; }		/* Decrement your count.. */
		mark_player_inventory_as_dirty(player_index,item_type);
		if (player->items[item_type] == 0 && definition->item_kind==_weapon)
		{    select_next_best_weapon(player_index); }
    }
	return 0;
}

static int L_Count_Item(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "count_item: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int item_type = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "count_item: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->items[item_type]);
	return 1;
}

static int L_Destroy_Ball(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "destroy_ball: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));

	if (find_player_ball_color(player_index) != NONE)
		destroy_players_ball(player_index);

	return 0;
}

static int L_Select_Weapon(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "select_weapon: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int weapon_index = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "select_weapon: invalid player index");
		lua_error(L);
	}

	ready_weapon(player_index, weapon_index);
	return 0;
}

static int L_Set_Fog_Depth(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_fog_depth: incorrect argument type");
		lua_error(L);
	}
	float depth = static_cast<float>(lua_tonumber(L,1));
	OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth = depth;
	return 0;
}

static int L_Set_Fog_Color(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_fog_color: incorrect argument type");
		lua_error(L);
	}
	float r = static_cast<float>(lua_tonumber(L,1));
	float g = static_cast<float>(lua_tonumber(L,2));
	float b = static_cast<float>(lua_tonumber(L,3));

	rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
	Color.red = PIN(int(65535*r+0.5),0,65535);
	Color.green = PIN(int(65535*g+0.5),0,65535);
	Color.blue = PIN(int(65535*b+0.5),0,65535);
	return 0;
}

static int L_Set_Fog_Present(lua_State *L)
{
	if (!lua_isboolean(L,1))
	 {
		lua_pushstring(L, "set_fog_present: incorrect argument type");
		lua_error(L);
	 }
	OGL_GetFogData(OGL_Fog_AboveLiquid)->IsPresent = static_cast<bool>(lua_toboolean(L,1));
	return 0;
}

static int L_Set_Fog_Affects_Landscapes(lua_State *L)
{
	if (!lua_isboolean(L,1))
	 {
		lua_pushstring(L, "set_fog_affects_landscapes: incorrect argument type");
		lua_error(L);
	 }
	OGL_GetFogData(OGL_Fog_AboveLiquid)->AffectsLandscapes = static_cast<bool>(lua_toboolean(L,1));
	return 0;
}

static int L_Get_Fog_Depth(lua_State *L)
{
	lua_pushnumber(L, OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth);
	return 1;
}

static int L_Get_Fog_Color(lua_State *L)
{
	rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
	lua_pushnumber(L, Color.red/65535.0F);
	lua_pushnumber(L, Color.green/65535.0F);
	lua_pushnumber(L, Color.blue/65535.0F);
	return 3;
}

static int L_Get_Fog_Present(lua_State *L)
{
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_AboveLiquid)->IsPresent);
	return 1;
}

static int L_Get_Fog_Affects_Landscapes(lua_State *L)
{
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_AboveLiquid)->AffectsLandscapes);
	return 1;
}

static int L_Set_Underwater_Fog_Present(lua_State *L)
{
	if (!lua_isboolean(L,1))
	 {
		lua_pushstring(L, "set_underwater_fog_present: incorrect argument type");
		lua_error(L);
	 }
	OGL_GetFogData(OGL_Fog_BelowLiquid)->IsPresent = static_cast<bool>(lua_toboolean(L,1));
	return 0;
}

static int L_Set_Underwater_Fog_Affects_Landscapes(lua_State *L)
{
	if (!lua_isboolean(L,1))
	 {
		lua_pushstring(L, "set_underwater_fog_affects_landscapes: incorrect argument type");
		lua_error(L);
	 }
	OGL_GetFogData(OGL_Fog_BelowLiquid)->AffectsLandscapes = static_cast<bool>(lua_toboolean(L,1));
	return 0;
}

static int L_Set_Underwater_Fog_Depth(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_underwater_fog_depth: incorrect argument type");
		lua_error(L);
	}
	float depth = static_cast<float>(lua_tonumber(L,1));
	OGL_GetFogData(OGL_Fog_BelowLiquid)->Depth = depth;
	return 0;
}

static int L_Set_Underwater_Fog_Color(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_underwater_fog_color: incorrect argument type");
		lua_error(L);
	}
	float r = static_cast<float>(lua_tonumber(L,1));
	float g = static_cast<float>(lua_tonumber(L,2));
	float b = static_cast<float>(lua_tonumber(L,3));

	rgb_color& Color = OGL_GetFogData(OGL_Fog_BelowLiquid)->Color;
	Color.red = PIN(int(65535*r+0.5),0,65535);
	Color.green = PIN(int(65535*g+0.5),0,65535);
	Color.blue = PIN(int(65535*b+0.5),0,65535);
	return 0;
}

static int L_Get_Underwater_Fog_Depth(lua_State *L)
{
	lua_pushnumber(L, OGL_GetFogData(OGL_Fog_BelowLiquid)->Depth);
	return 1;
}

static int L_Get_Underwater_Fog_Color(lua_State *L)
{
	rgb_color& Color = OGL_GetFogData(OGL_Fog_BelowLiquid)->Color;
	lua_pushnumber(L, Color.red/65535.0F);
	lua_pushnumber(L, Color.green/65535.0F);
	lua_pushnumber(L, Color.blue/65535.0F);
	return 3;
}

static int L_Get_Underwater_Fog_Present(lua_State *L)
{
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_BelowLiquid)->IsPresent);
	return 1;
}

static int L_Get_Underwater_Fog_Affects_Landscapes(lua_State *L)
{
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_BelowLiquid)->AffectsLandscapes);
	return 1;
}

/* This code would be better if we were using pointers instead of references. */
static int L_Get_All_Fog_Attributes(lua_State *L)
{
	lua_newtable(L);
	lua_pushstring(L,"OGL_Fog_AboveLiquid");
	lua_newtable(L);
	lua_pushstring(L, "Depth");
	lua_pushnumber(L, OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth);
	lua_settable(L, -3);
	lua_pushstring(L, "Color");
	lua_newtable(L);
	{
		rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
		lua_pushstring(L, "red");
		lua_pushnumber(L, Color.red/65535.0F);
		lua_settable(L, -3);
		lua_pushstring(L, "green");
		lua_pushnumber(L, Color.green/65535.0F);
		lua_settable(L, -3);
		lua_pushstring(L, "blue");
		lua_pushnumber(L, Color.blue/65535.0F);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);
	lua_pushstring(L, "IsPresent");
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_AboveLiquid)->IsPresent);
	lua_settable(L, -3);
	lua_pushstring(L, "AffectsLandscapes");
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_AboveLiquid)->AffectsLandscapes);
	lua_settable(L, -3);
	lua_settable(L, -3);
	lua_pushstring(L,"OGL_Fog_BelowLiquid");
	lua_newtable(L);
	lua_pushstring(L, "Depth");
	lua_pushnumber(L, OGL_GetFogData(OGL_Fog_BelowLiquid)->Depth);
	lua_settable(L, -3);
	lua_pushstring(L, "Color");
	lua_newtable(L);
	{
		rgb_color& Color = OGL_GetFogData(OGL_Fog_BelowLiquid)->Color;
		lua_pushstring(L, "red");
		lua_pushnumber(L, Color.red/65535.0F);
		lua_settable(L, -3);
		lua_pushstring(L, "green");
		lua_pushnumber(L, Color.green/65535.0F);
		lua_settable(L, -3);
		lua_pushstring(L, "blue");
		lua_pushnumber(L, Color.blue/65535.0F);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);
	lua_pushstring(L, "IsPresent");
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_BelowLiquid)->IsPresent);
	lua_settable(L, -3);
	lua_pushstring(L, "AffectsLandscapes");
	lua_pushboolean(L,OGL_GetFogData(OGL_Fog_BelowLiquid)->AffectsLandscapes);
	lua_settable(L, -3);
	lua_settable(L, -3);
	return 1;
}

static int L_Set_All_Fog_Attributes(lua_State *L)
{
	if(!lua_istable(L,1)) {
		lua_pushstring(L, "set_all_fog_attributes: incorrect argument type");
		lua_error(L);
	}
	lua_pushstring(L,"OGL_Fog_AboveLiquid");
	lua_gettable(L, -2);
	lua_pushstring(L, "Depth");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "Color");
	lua_gettable(L, -2);
	{
		rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
		lua_pushstring(L, "red");
		lua_gettable(L, -2);
		Color.red = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 1);
		lua_pushstring(L, "green");
		lua_gettable(L, -2);
		Color.green = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 1);
		lua_pushstring(L, "blue");
		lua_gettable(L, -2);
		Color.blue = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 2);
	}
	lua_pushstring(L, "IsPresent");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_AboveLiquid)->IsPresent = lua_toboolean(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "AffectsLandscapes");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_AboveLiquid)->AffectsLandscapes = lua_toboolean(L, -1);
	lua_pop(L, 2);
	lua_pushstring(L,"OGL_Fog_BelowLiquid");
	lua_gettable(L, -2);
	lua_pushstring(L, "Depth");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_BelowLiquid)->Depth = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "Color");
	lua_gettable(L, -2);
	{
		rgb_color& Color = OGL_GetFogData(OGL_Fog_BelowLiquid)->Color;
		lua_pushstring(L, "red");
		lua_gettable(L, -2);
		Color.red = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 1);
		lua_pushstring(L, "green");
		lua_gettable(L, -2);
		Color.green = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 1);
		lua_pushstring(L, "blue");
		lua_gettable(L, -2);
		Color.blue = PIN(int(65535*lua_tonumber(L, -1)+0.5),0,65535);
		lua_pop(L, 2);
	}
	lua_pushstring(L, "IsPresent");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_BelowLiquid)->IsPresent = lua_toboolean(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "AffectsLandscapes");
	lua_gettable(L, -2);
	OGL_GetFogData(OGL_Fog_BelowLiquid)->AffectsLandscapes = lua_toboolean(L, -1);
	lua_pop(L, 3);
	return 0;
}

static int L_Teleport_Player(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "teleport_player: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int dest = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "teleport_player: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	monster_data *monster= get_monster_data(player->monster_index);

	SET_PLAYER_TELEPORTING_STATUS(player, true);
	monster->action= _monster_is_teleporting;
	player->teleporting_phase= 0;
	player->delay_before_teleport= 0;

	player->teleporting_destination= dest;
	if (local_player_index == player_index)
		start_teleporting_effect(true);
	play_object_sound(player->object_index, Sound_TeleportOut());
	return 0;
}

// Note: a monster of the type created must already exist in the map.
static int L_New_Monster(lua_State *L)
{
	int args = lua_gettop(L);
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "new_monster: incorrect argument type");
		lua_error(L);
	}
	short monster_type = static_cast<int>(lua_tonumber(L,1));
	short polygon = static_cast<int>(lua_tonumber(L,2));
	/* SB: validate the monster type */
	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES) {
		lua_pushstring(L, "new_monster: invalid monster type");
		lua_error(L);
	}
	
	short facing = 0;
	if (args > 2)
	{
		if (!lua_isnumber(L,3))
		{
			lua_pushstring(L, "new_monster: incorrect argument type");
			lua_error(L);
		}
		facing = static_cast<int>(lua_tonumber(L,3));
	}

	short height = 0;
	if (args > 3)
	{
		if (!lua_isnumber(L,4))
		{
			lua_pushstring(L, "new_monster: incorrect argument type");
			lua_error(L);
		}
		height = static_cast<int>(lua_tonumber(L,4));
	}

	object_location theLocation;
	struct polygon_data *destination;
	world_point3d theDestination;
	world_point2d theCenter;
	short index;

	theLocation.polygon_index = (int16)polygon;
	destination = NULL;
	destination= get_polygon_data(theLocation.polygon_index);
	if(destination==NULL)
		return 0;
	// *((world_point2d *)&theDestination)= destination->center;
 //stolen, assuming it works
	if (args > 5)
	{
		if (!lua_isnumber(L,5) || !lua_isnumber(L,6))
		{
			lua_pushstring(L, "new_monster: incorrect argument type");
			lua_error(L);
		}
		theDestination.x = static_cast<int>(lua_tonumber(L,5)*WORLD_ONE);
		theDestination.y = static_cast<int>(lua_tonumber(L,6)*WORLD_ONE);
	}
	else {
	  find_center_of_polygon(polygon, &theCenter);
	  theDestination.x = theCenter.x;
	  theDestination.y = theCenter.y;
	}
	theDestination.z= height;
	theLocation.p = theDestination;
	theLocation.yaw = 0;
	theLocation.pitch = 0;
	theLocation.flags = 0;

	index = new_monster(&theLocation, (short)monster_type);
	if (index == NONE)
		return 0;
	lua_pushnumber(L, index);

	monster_data *monster = get_monster_data(index);
	object_data *object = get_object_data(monster->object_index);
	object->facing = normalize_angle(static_cast<int>((double)facing/AngleConvert));

	return 1;
}

static int L_Activate_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "activate_monster: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	if (monster_index == -1)
		return 0;
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if (!SLOT_IS_USED(monster))
	{
		lua_pushstring(L, "activate_monster: invalid monster index");
		lua_error(L);
	}
	return 0;
	if(!MONSTER_IS_ACTIVE(monster))
		activate_monster(monster_index);
}

static int L_Deactivate_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "deactivate_monster: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	if (monster_index == -1)
		return 0;
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if (!SLOT_IS_USED(monster))
	{
		lua_pushstring(L, "deactivate_monster: invalid monster index");
		lua_error(L);
	}
	return 0;
	if(MONSTER_IS_ACTIVE(monster))
		deactivate_monster(monster_index);
}

static int L_Damage_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "damage_monster: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	int damage_amount = static_cast<int>(lua_tonumber(L,2));
	int damage_type = -1;
	if (lua_gettop(L) == 3 && lua_isnumber(L,3))
		damage_type = static_cast<int>(lua_tonumber(L,3));
	if (monster_index == -1)
	{
		lua_pushstring(L, "damage_monster: invalid monster index");
		lua_error(L);
	}

	struct damage_definition theDamage;
	struct monster_data *theMonster;
	if (damage_type != NONE)
		theDamage.type = damage_type;
	else
		theDamage.type = _damage_fist;

	theDamage.base = damage_amount;
	theDamage.random = 0;
	theDamage.flags = 0;
	theDamage.scale = 65536;

	theMonster= GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if (!SLOT_IS_USED(theMonster))
	{
		lua_pushstring(L, "damage_monster: invalid monster index");
		lua_error(L);
	}
	damage_monster(monster_index, NONE, NONE, &(get_monster_data(monster_index)->sound_location), &theDamage, NONE);
	return 0;
}

static int L_Attack_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "attack_monster: incorrect argument type");
		lua_error(L);
	}
	int attacker_index = static_cast<int>(lua_tonumber(L,1));
	int target_index = static_cast<int>(lua_tonumber(L,2));

	struct monster_data *bully, *poorHelplessVictim;
	if(attacker_index == NONE || target_index == NONE) return 0;
	bully = GetMemberWithBounds(monsters,attacker_index,MAXIMUM_MONSTERS_PER_MAP);
	poorHelplessVictim = GetMemberWithBounds(monsters,target_index,MAXIMUM_MONSTERS_PER_MAP);
	if (!SLOT_IS_USED(bully) || !SLOT_IS_USED(poorHelplessVictim))
	{
		lua_pushstring(L, "attack_monster: invalid monster index");
		lua_error(L);
	}
	change_monster_target(attacker_index, target_index);

	return 0;
}

static int L_Move_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "move_monster: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	int polygon = static_cast<int>(lua_tonumber(L,2));

	world_point2d *theEnd;
	struct monster_pathfinding_data thePath;
	struct monster_data *theRealMonster;
	struct monster_definition *theDef;
	struct object_data *theObject;

	if(monster_index == NONE)
	{
		lua_pushstring(L, "move_monster: invalid monster index");
		lua_error(L);
	}
	theRealMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theRealMonster)) return 0;		//we must have a legal monster
	theDef = get_monster_definition_external(theRealMonster->type);
	theObject = get_object_data(theRealMonster->object_index);
	//some checks stolen from generate_new_path_for_monster
	if(!MONSTER_IS_ACTIVE(theRealMonster))
		activate_monster(monster_index);
	if (theRealMonster->path!=NONE)
        {
		delete_path(theRealMonster->path);
		theRealMonster->path= NONE;
        }
	SET_MONSTER_NEEDS_PATH_STATUS(theRealMonster, false);

	thePath.definition = get_monster_definition_external(theRealMonster->type);
	thePath.monster = theRealMonster;
	thePath.cross_zone_boundaries = true;

	theEnd = &(get_polygon_data(polygon)->center);

	theRealMonster->path = new_path((world_point2d *)&theObject->location, theObject->polygon, theEnd, polygon, 3*get_monster_definition_external(theRealMonster->type)->radius, monster_pathfinding_cost_function, &thePath);
	if (theRealMonster->path==NONE)
	{
		if(theRealMonster->action!=_monster_is_being_hit || MONSTER_IS_DYING(theRealMonster))
		{	set_monster_action(monster_index, _monster_is_stationary); }
		set_monster_mode(monster_index, _monster_unlocked, NONE);
		return 0;
	}
	advance_monster_path(monster_index);

	return 0;
}

static int L_Monster_Index_Valid(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "monster_index_valid: incorrect argument type");
		lua_error(L);
	}
	short monster_index = static_cast<int>(lua_tonumber(L,1));
	if(monster_index < 0 || monster_index >= MAXIMUM_MONSTERS_PER_MAP) {
		lua_pushnil(L);
	}
	else {
		struct monster_data* monster;
		monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
		lua_pushboolean(L, SLOT_IS_USED(monster));
	}
	return 1;
}

static int L_Get_Monster_Type(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "get_monster_type: incorrect argument type");
		lua_error(L);
	}
	short monster_index = static_cast<int>(lua_tonumber(L,1));
	if(monster_index < 0 || monster_index >= MAXIMUM_MONSTERS_PER_MAP) {
		lua_pushstring(L, "get_monster_type: invalid monster index");
		lua_error(L);
	}
	struct monster_data* monster;
	monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) {
		lua_pushstring(L, "get_monster_type: invalid monster index");
		lua_error(L);
	}
	lua_pushnumber(L, monster->type);
	return 1;
}

static int L_Get_Monster_Type_Class(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "get_monster_type_class: incorrect argument type");
		lua_error(L);
	}
	short type = static_cast<int>(lua_tonumber(L,1));
	if(type < 0 || type >= NUMBER_OF_MONSTER_TYPES) {
		lua_pushstring(L, "get_monster_type_class: invalid monster type");
		lua_error(L);
	}
	struct monster_definition* def;
	def = get_monster_definition_external(type);
	lua_pushnumber(L, def->_class);
	return 1;
}

static int L_Set_Monster_Type_Class(lua_State *L) {
	if(!lua_isnumber(L,1)||!lua_isnumber(L,2)) {
		lua_pushstring(L, "set_monster_type_class: incorrect argument type");
		lua_error(L);
	}
	short type = static_cast<int>(lua_tonumber(L,1));
	short _class = static_cast<int>(lua_tonumber(L,2));
	if(type < 0 || type >= NUMBER_OF_MONSTER_TYPES) {
		lua_pushstring(L, "set_monster_type_class: invalid monster type");
		lua_error(L);
	}
	if(_class < _class_player || _class > _class_yeti_bit) {
		lua_pushstring(L, "set_monster_type_class: invalid monster class");
		lua_error(L);
	}
	struct monster_definition* def;
	def = get_monster_definition_external(type);
	def->_class = _class;
	return 0;
}

static int L_Select_Monster(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "select_monster: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int polygon = static_cast<int>(lua_tonumber(L,2));
	/* SB: validate here too */
	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES) {
		lua_pushstring(L, "select_monster: invalid monster type");
		lua_error(L);
	}
	
	//we are using IntersectedObjects
	int found;
	size_t i, objectCount;
	struct monster_data * theMonster;

	found = possible_intersecting_monsters(&IntersectedObjects, LOCAL_INTERSECTING_MONSTER_BUFFER_SIZE, (short)polygon, false);
	//now we have a list of stuff
	objectCount = IntersectedObjects.size();
	for(i=0;i<objectCount;i++)
	{
		theMonster = GetMemberWithBounds(monsters,get_object_data(IntersectedObjects[i])->permutation,MAXIMUM_MONSTERS_PER_MAP);
		if((theMonster->type == monster_type) && SLOT_IS_USED(theMonster) && theMonster->unused[0] != polygon)
		{
			if(get_object_data(theMonster->object_index)->polygon == polygon)
			{
				theMonster->unused[0]=polygon;
				lua_pushnumber(L, get_object_data(IntersectedObjects[i])->permutation);
				return 1;
			}
		}
	}
	return 0;
}

static int L_Get_Monster_Polygon(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_polygon: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));

	struct monster_data *theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theMonster))
	{
		lua_pushstring(L, "get_monster_polygon: invalid monster index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(theMonster->object_index);

	lua_pushnumber(L, object->polygon);
	return 1;
}

static int L_Get_Monster_Immunity(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_monster_immunity: incorrect argument type");
		lua_error(L);
	}

	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int damage_type = static_cast<int>(lua_tonumber(L,2));

	struct monster_definition *theDef;
	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "get_monster_immunity: invalid monster type");
		lua_error(L);
	}
	theDef = get_monster_definition_external(monster_type);
	lua_pushboolean(L, theDef->immunities & 1<<damage_type);
	return 1;
}

static int L_Get_Monster_Position(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_position: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));

	struct monster_data *theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theMonster))
	{
		lua_pushstring(L, "get_monster_position: invalid monster index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(theMonster->object_index);

	lua_pushnumber(L, object->location.x);
	lua_pushnumber(L, object->location.y);
	lua_pushnumber(L, object->location.z);
	return 3;
}

static int L_Get_Monster_Facing(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_facing: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));

	struct monster_data *theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(theMonster))
	{
		lua_pushstring(L, "get_monster_facing: invalid monster index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(theMonster->object_index);

	lua_pushnumber(L, object->facing);
	return 1;
}

static int L_Set_Monster_Immunity(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isboolean(L,3))
	{
		lua_pushstring(L, "set_monster_immunity: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int damage_type = static_cast<int>(lua_tonumber(L,2));
	bool immune = lua_toboolean(L,3);

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "set_monster_immunity: invalid monster type");
		lua_error(L);
	}
	if(immune)
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->immunities = theDef->immunities | 1<<damage_type;
		return 0;
	}
	else
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->immunities = theDef->immunities & ~(1<<damage_type);
		return 0;
	}
}

static int L_Get_Monster_Weakness(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_monster_weakness: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int damage_type = static_cast<int>(lua_tonumber(L,2));

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "get_monster_weakness: invalid monster type");
		lua_error(L);
	}

	theDef = get_monster_definition_external(monster_type);
	lua_pushboolean(L, theDef->weaknesses & 1<<damage_type);
	return 1;
}

static int L_Set_Monster_Weakness(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isboolean(L,3))
	{
		lua_pushstring(L, "set_monster_weakness: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int damage_type = static_cast<int>(lua_tonumber(L,2));
	bool weak = lua_toboolean(L,3);

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "set_monster_weakness: invalid monster type");
		lua_error(L);
	}
	if(weak)
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->weaknesses = theDef->weaknesses | 1<<damage_type;
		return 0;
	}
	else
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->weaknesses = theDef->weaknesses & ~(1<<damage_type);
		return 0;
	}
}

static int L_Get_Monster_Friend(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_monster_friend: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int friend_class = static_cast<int>(lua_tonumber(L,2));

	struct monster_definition *theDef;
	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "get_monster_friend: invalid monster type");
		lua_error(L);
	}
	theDef = get_monster_definition_external(monster_type);
	lua_pushboolean(L, theDef->friends & 1<<friend_class);
	return 1;
}

static int L_Set_Monster_Friend(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isboolean(L,3))
	{
		lua_pushstring(L, "set_monster_friend: incorrect argument type");
		lua_error(L);
		return 0;
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int friend_class = static_cast<int>(lua_tonumber(L,2));
	bool friendly = lua_toboolean(L,3);

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "set_monster_friend: invalid monster type");
		lua_error(L);
	}
	if(friendly)
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->friends = theDef->friends | 1<<friend_class;
		return 0;
	}
	else
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->friends = theDef->friends & ~(1<<friend_class);
		return 0;
	}
}

static int L_Get_Monster_Enemy(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_monster_enemy: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int enemy_class = static_cast<int>(lua_tonumber(L,2));

	struct monster_definition *theDef;
	
	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "get_monster_enemy: invalid monster type");
		lua_error(L);
	}
	theDef = get_monster_definition_external(monster_type);
	lua_pushboolean(L, theDef->enemies & 1<<enemy_class);
	return 1;
}

static int L_Set_Monster_Enemy(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isboolean(L,3))
	{
		lua_pushstring(L, "set_monster_enemy: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int enemy_class = static_cast<int>(lua_tonumber(L,2));
	bool hostile = lua_toboolean(L,3);

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "set_monster_enemy: invalid monster type");
		lua_error(L);
	}
	if(hostile)
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->enemies = theDef->enemies | 1<<enemy_class;
		return 0;
	}
	else
	{
		theDef = get_monster_definition_external(monster_type);
		theDef->enemies = theDef->enemies & ~(1<<enemy_class);
		return 0;
	}
}

static int L_Get_Monster_Item(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_item: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "get_monster_item: invalid monster type");
		lua_error(L);
	}
	theDef = get_monster_definition_external(monster_type);
	lua_pushnumber(L, theDef->carrying_item_type);
	return 1;
}

static int L_Set_Monster_Item(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_monster_item: incorrect argument type");
		lua_error(L);
	}
	int monster_type = static_cast<int>(lua_tonumber(L,1));
	int item_type = static_cast<int>(lua_tonumber(L,2));

	struct monster_definition *theDef;

	if(monster_type < 0 || monster_type >= NUMBER_OF_MONSTER_TYPES)
	{
		lua_pushstring(L, "set_monster_item: invalid monster type");
		lua_error(L);
	}
	theDef = get_monster_definition_external(monster_type);
	theDef->carrying_item_type = item_type;
	return 0;
}

static int L_Get_Monster_Action(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_action: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	if(monster_index == -1)
	{
		lua_pushstring(L, "get_monster_immunity: invalid monster index");
		lua_error(L);
	}

	struct monster_data *theMonster;

	theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if (!SLOT_IS_USED(theMonster))
	{
		lua_pushstring(L, "get_monster_action: invalid monster index");
		lua_error(L);
	}
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) return 0;
	if (monster)
	{
		lua_pushnumber(L, monster->action);
		return 1;
	}
	else return 0;
}

static int L_Get_Monster_Mode(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_mode: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	if(monster_index == -1)
	{
		lua_pushstring(L, "get_monster_mode: invalid monster index");
		lua_error(L);
	}
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) return 0;
	if (monster)
	{
		lua_pushnumber(L, monster->mode);
		return 1;
	}
	else return 0;
}

static int L_Get_Monster_Vitality(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_monster_vitality: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	if(monster_index == NONE)
	{
		lua_pushstring(L, "get_monster_vitality: invalid monster index");
		lua_error(L);
	}
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) return 0;
	if (monster)
	{
		lua_pushnumber(L, monster->vitality);
		return 1;
	}
	else return 0;
}

static int L_Set_Monster_Vitality(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_monster_vitality: incorrect argument type");
		lua_error(L);
	}
	int monster_index = static_cast<int>(lua_tonumber(L,1));
	int vitality = static_cast<int>(lua_tonumber(L,2));
	if(monster_index == NONE)
	{
		lua_pushstring(L, "set_monster_vitality: invalid monster index");
		lua_error(L);
	}
	struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) return 0;
	if (monster)
		monster->vitality = vitality;
	return 1;
}
/*
 // should modify all values needed for a Matrix-style slowdown shot ;)
 // this includes monster_data as well and monster_definition entries
 static int L_Set_Monster_Global_Speed(lua_State *L)
 {
	 if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	 {
		 lua_pushstring(L, "set_monster_global_speed: incorrect argument type");
		 lua_error(L);
	 }
	 int monster_index = static_cast<int>(lua_tonumber(L,1));
	 double scale = static_cast<double>(lua_tonumber(L,2));
	 if (scale < 0)
		 return 0;

	 struct monster_data *theMonster;
	 struct monster_definition *theDef;

	 if(monster_index == -1)
	 {
		 lua_pushstring(L, "set_monster_global_speed: invalid monster index");
		 lua_error(L);
	 }

	 theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	 if (!SLOT_IS_USED(theMonster))
	 {
		 lua_pushstring(L, "set_monster_global_speed: invalid monster index");
		 lua_error(L);
	 }

	 if(SLOT_IS_USED(theMonster))
	 {
		 theDef = get_monster_definition_external(theMonster->type);
		 //def: external_velocity_scale, speed, gravity, attack_frequency, sound_pitch
   //    data: external_velocity, vertical_velocity
		 theDef->external_velocity_scale *= scale;
		 theDef->speed *= scale;
		 theDef->gravity *= scale;
		 theDef->attack_frequency /= scale;
		 theDef->sound_pitch *= scale;
		 theMonster->external_velocity *= scale;
		 theMonster->vertical_velocity *= scale;
	 }
	 return 0;
 }
 */
static int L_Play_Sound(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "play_sound: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int sound_index = static_cast<int>(lua_tonumber(L,2));
	float pitch = static_cast<float>(lua_tonumber(L,3));

	if (local_player_index != player_index)
		return 0;

	_play_sound(sound_index, NULL, NONE, _fixed(FIXED_ONE*pitch));
	return 0;
}

static int L_Get_Player_Polygon(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_polygon: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_polygon: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->supporting_polygon_index);
	return 1;
}

static int L_Get_Player_Position(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_position: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_position: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double)player->location.x/WORLD_ONE);
	lua_pushnumber(L, (double)player->location.y/WORLD_ONE);
	lua_pushnumber(L, (double)player->location.z/WORLD_ONE);
	return 3;
}

static int L_Set_Player_Position(lua_State *L)
{
	if (!lua_isnumber(L,1)||!lua_isnumber(L,2)||!lua_isnumber(L,3)||!lua_isnumber(L,4)||!lua_isnumber(L,5))
	{
		lua_pushstring(L, "set_player_position: incorrect argument type");
		lua_error(L);
	}
	
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_position: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	struct monster_data *monster= get_monster_data(player->monster_index);
	struct object_data *object= get_object_data(monster->object_index);
	world_point3d loc;
	loc.x = static_cast<int>(lua_tonumber(L,2) * WORLD_ONE);
	loc.y = static_cast<int>(lua_tonumber(L,3) * WORLD_ONE);
	loc.z = static_cast<int>(lua_tonumber(L,4) * WORLD_ONE);
	translate_map_object(player->object_index, &loc,static_cast<int>(lua_tonumber(L,5)));
	player->variables.position.x= WORLD_TO_FIXED(object->location.x);
	player->variables.position.y= WORLD_TO_FIXED(object->location.y);
	player->variables.position.z= WORLD_TO_FIXED(object->location.z);
	return 0;
}

static int L_Get_Player_Angle(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_angle: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_angle: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, (double)FIXED_INTEGERAL_PART(player->variables.direction)*AngleConvert);
	lua_pushnumber(L, (double)FIXED_INTEGERAL_PART(player->variables.elevation)*AngleConvert);
	return 2;
}

static int L_Set_Player_Angle(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_player_angle: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	double facing = static_cast<double>(lua_tonumber(L,2));
	double elevation = static_cast<double>(lua_tonumber(L,3));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_angle: invalid player index");
		lua_error(L);
	}
	
	player_data *player = get_player_data(player_index);
	player->variables.direction = INTEGER_TO_FIXED((int)(facing/AngleConvert));
	player->variables.elevation = INTEGER_TO_FIXED((int)(elevation/AngleConvert));
	return 0;
}

static int L_Get_Player_Color(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_color: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_color: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->color);
	return 1;
}

static int L_Set_Player_Color(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_player_color: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	int color = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_color: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	player->color = color;
	return 0;
}

static int L_Get_Player_Team(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_team: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_team: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->team);
	return 1;
}

static int L_Set_Player_Team(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_player_team: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	int team = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_team: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	player->team = team;
	return 0;
}

static int L_Get_Player_Name(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_name: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_name: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushstring(L, player->name);
	return 1;
}

static int L_Get_Player_Powerup_Duration(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_player_powerup_duration: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_powerup_duration: invalid player index");
		lua_error(L);
	}
	
	int powerup = static_cast<int>(lua_tonumber(L,2));
	if (powerup < 0 || powerup > 3){
		lua_pushstring(L, "get_player_powerup_duration: invalid powerup type");
		lua_error(L);
	}
	
	player_data *player = get_player_data(player_index);
	
	switch(powerup){
		case 0:
			lua_pushnumber(L, player->invisibility_duration);
			break;
		
		case 1:
			lua_pushnumber(L, player->invincibility_duration);
			break;
		
		case 2:
			lua_pushnumber(L, player->infravision_duration);
			break;
		
		case 3:
			lua_pushnumber(L, player->extravision_duration);
			break;
	}
	
	return 1;
}

static int L_Set_Player_Powerup_Duration(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_player_powerup_duration: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	int powerup = static_cast<int>(lua_tonumber(L,2));
	int duration = static_cast<int>(lua_tonumber(L,3));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_powerup_duration: invalid player index");
		lua_error(L);
	}
	if (powerup < 0 || powerup > 3){
		lua_pushstring(L, "set_player_powerup_duration: invalid powerup type");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	switch(powerup){
		case 0:
			player->invisibility_duration = duration;
			break;
		
		case 1:
			player->invincibility_duration = duration;
			break;
		
		case 2:
			player->infravision_duration = duration;
			break;
		
		case 3:
			player->extravision_duration = duration;
			break;
	}
	return 0;
}

static int L_Get_Player_Internal_Velocity(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_internal_velocity: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_internal_velocity: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->variables.velocity);
	lua_pushnumber(L, player->variables.perpendicular_velocity);
	return 2;
}

static int L_Get_Player_External_Velocity(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_player_external_velocity: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "get_player_external_velocity: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	lua_pushnumber(L, player->variables.external_velocity.i);
        lua_pushnumber(L, player->variables.external_velocity.j);
        lua_pushnumber(L, player->variables.external_velocity.k);
	return 3;
}

static int L_Set_Player_External_Velocity(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,4))
	{
		lua_pushstring(L, "set_player_external_velocity: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
        int raw_velocity_i = static_cast<int>(lua_tonumber(L,2));
        int raw_velocity_j = static_cast<int>(lua_tonumber(L,3));
        int raw_velocity_k = static_cast<int>(lua_tonumber(L,4));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "set_player_external_velocity: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	player->variables.external_velocity.i = raw_velocity_i;
        player->variables.external_velocity.j = raw_velocity_j;
        player->variables.external_velocity.k = raw_velocity_k;
	return 0;
}

static int L_Add_To_Player_External_Velocity(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,4))
	{
		lua_pushstring(L, "add_to_player_external_velocity: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
        int raw_velocity_i = static_cast<int>(lua_tonumber(L,2));
        int raw_velocity_j = static_cast<int>(lua_tonumber(L,3));
        int raw_velocity_k = static_cast<int>(lua_tonumber(L,4));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "add_to_player_external_velocity: invalid player index");
		lua_error(L);
	}

	player_data *player = get_player_data(player_index);
	player->variables.external_velocity.i += raw_velocity_i * WORLD_ONE;
        player->variables.external_velocity.j += raw_velocity_j * WORLD_ONE;
        player->variables.external_velocity.k += raw_velocity_k * WORLD_ONE;
	return 0;
}

static int L_Accelerate_Player(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,4))
	{
		lua_pushstring(L, "accelerate_player: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
        double vertical_velocity = static_cast<double>(lua_tonumber(L,2));
        double direction = static_cast<double>(lua_tonumber(L,3));
        double velocity = static_cast<double>(lua_tonumber(L,4));
	
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "accelerate_player: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	
	accelerate_player(player->monster_index, static_cast<int>(vertical_velocity*WORLD_ONE), static_cast<int>(direction/AngleConvert), static_cast<int>(velocity*WORLD_ONE));

	return 0;
}

static int L_Player_Is_Dead(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "player_is_dead: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "player_is_dead: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);

	if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);
	return 1;
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

	// uses the Pfhortran action queue
	if (sPfhortranActionQueues == NULL)
	{
		sPfhortranActionQueues = new ActionQueues(MAXIMUM_NUMBER_OF_PLAYERS, ACTION_QUEUE_BUFFER_DIAMETER, true);
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

		case 13: // reset pfhortran_action_queue
			GetPfhortranActionQueues()->reset();
			break;
#endif
		default:
			break;
	}

	if (DoAction)
	{
		for (int i=1; i<value; i++)
			action_flags[i] = action_flags[0];

		GetPfhortranActionQueues()->enqueueActionFlags(player_index, action_flags, value);
	}
	return 0;
}

static int L_Teleport_Player_To_Level(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "teleport_player_to_level: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	int dest = static_cast<int>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "teleport_player_to_level: invalid player index");
		lua_error(L);
	}
	player_data *player = get_player_data(player_index);
	monster_data *monster= get_monster_data(player->monster_index);

	SET_PLAYER_TELEPORTING_STATUS(player, true);
	monster->action= _monster_is_teleporting;
	player->teleporting_phase= 0;
	player->delay_before_teleport= 0;

	player->teleporting_destination= - dest -1;
	start_teleporting_effect(true);
	play_object_sound(player->object_index, Sound_TeleportOut());
	return 0;
}

#if 0
 static int L_Set_Player_Global_Speed(lua_State *L)
 {
	 if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	 {
		 lua_pushstring(L, "set_player_global_speed: incorrect argument type");
		 lua_error(L);
	 }
	 int player_index = static_cast<int>(lua_tonumber(L,1));
	 if (player_index < 0 || player_index >= dynamic_world->player_count)
	 {
		 lua_pushstring(L, "set_player_global_speed: invalid player index");
		 lua_error(L);
	 }
	 double scale = static_cast<double>(lua_tonumber(L,2));
	 player_data *player = get_player_data(player_index);
	 struct physics_constants *constants = get_physics_constants_for_model(static_world->physics_model, _run_dont_walk);

	 /* angular_velocity, vertical_angular_velocity, velocity, perpendicular_velocity,
		 external_velocity, external_angular_velocity */
			 player->variables.angular_velocity *= scale;
		  player->variables.vertical_angular_velocity *= scale;
		  player->variables.velocity *= scale;
		  player->variables.perpendicular_velocity *= scale;
		  player->variables.external_velocity.i *= scale;
		  player->variables.external_velocity.j *= scale;
		  player->variables.external_velocity.k *= scale;
		  player->variables.external_angular_velocity *= scale;
		  constants->gravitational_acceleration *= scale;
		  constants->maximum_forward_velocity *= scale;
		  constants->maximum_backward_velocity *= scale;
		  constants->maximum_perpendicular_velocity *= scale;
		  constants->step_delta *= scale;
		  return 0;
 }
#endif

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

static int L_Crosshairs_Active(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "crosshairs_active: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (local_player_index != player_index)
		return 0;

	lua_pushnumber(L, Crosshairs_IsActive());
	return 1;
}

static int L_Set_Crosshairs_State(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isboolean(L,2))
	{
		lua_pushstring(L, "set_crosshairs_state: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);
	if (local_player_index != player_index)
		return 0;

	Crosshairs_SetActive(state);
	return 0;
}

static int L_Zoom_Active(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "zoom_active: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring(L, "zoom_active: invalid player index");
		lua_error(L);
	}
	if (local_player_index != player_index)
		return 0;

	lua_pushnumber(L, GetTunnelVision());
	return 1;
}

static int L_Set_Zoom_State(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isboolean(L,2))
	{
		lua_pushstring(L, "set_zoom_state: incorrect argument type");
		lua_error(L);
	}
	int player_index = static_cast<int>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);
	if (local_player_index != player_index)
		return 0;

	SetTunnelVision(state);
	return 0;
}

static int L_Set_Motion_Sensor_State(lua_State *L)
{
	if(!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_motion_sensor_state: incorrect argument type");
		lua_error(L);
	}

	short player_index = static_cast<short>(lua_tonumber(L,1));
	/*
	 Useless... There is no way to disable the motion sensor of a given player,
	 it's just a flag in HUD Renderer
	 */
	/* MH: on the contrary, it's better to have it disable a single player's
        motion sensor and do nothing for the others, right?
	*/
	/* tiennou: Changed the code to reflect this behavior, so that the local_player
	motion_sensor wasn't disabled/enabled when this function was passed another player_index
	*/
	bool state = lua_toboolean(L,2);
	if (state && player_index == local_player_index)
		MotionSensorActive = true;
	else
		MotionSensorActive = false;
	return 0;
}

static int L_Get_Motion_Sensor_State(lua_State *L)
{
	/* MH: here we can return the true state if the local player
        is the player referenced, or just return the default value
        (could possibly screw some things up)
	*/
	if(!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_motion_sensor_state: incorrect argument type");
		lua_error(L);
	}
	short player_index = static_cast<short>(lua_tonumber(L,1));
	if (local_player_index != player_index)
		return 0;

	lua_pushboolean(L, MotionSensorActive);
	return 1;
}

static int L_Set_Platform_State(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_platform_state: incorrect argument type");
		lua_error(L);
	}

	short polygon_index = static_cast<short>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);

	struct polygon_data *polygon = get_polygon_data(polygon_index);
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			try_and_change_platform_state(short(polygon->permutation), state);
			assume_correct_switch_position(_panel_is_platform_switch, short(polygon->permutation), state);
		}
	}
	return 0;
}

static int L_Get_Platform_State(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_state: incorrect argument type");
		lua_error(L);
	}

	short polygon_index = static_cast<short>(lua_tonumber(L,1));

	struct polygon_data *polygon = get_polygon_data(polygon_index);
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			lua_pushboolean(L,(PLATFORM_IS_ACTIVE(get_platform_data(short(polygon->permutation)))));
			return 1;
		}
	}
	return 0;
}

static int L_Set_Light_State(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "set_light_state: incorrect argument type");
		lua_error(L);
	}

	size_t light_index = static_cast<size_t>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);

	set_light_status(light_index, state);
	assert(light_index == static_cast<size_t>(static_cast<short>(light_index)));
	assume_correct_switch_position(_panel_is_light_switch, static_cast<short>(light_index), state);
	return 0;
}

static int L_Get_Light_State(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_light_state: incorrect argument type");
		lua_error(L);
	}

	size_t light_index = static_cast<size_t>(lua_tonumber(L, 1));

	lua_pushboolean(L, get_light_status(light_index));
	return 1;

}

static int L_Screen_Fade(lua_State *L)
{
	int args = lua_gettop(L);
	int fade_index;
	
	if (args == 2)
	{
		if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
		{
			lua_pushstring(L, "start_fade: incorrect argument type");
			lua_error(L);
		}
		short player_index = static_cast<short>(lua_tonumber(L,1));
		if (local_player_index != player_index)
			return 0;

		fade_index = static_cast<int>(lua_tonumber(L, 2));
	} else {
		fade_index = static_cast<int>(lua_tonumber(L, 1));
	}
	
	start_fade(fade_index);
	return 0;
}

static int L_Set_Platform_Player_Control(lua_State *L)
{
	if (!lua_isnumber(L,1)) // boolean value can be any type, even nil
	{
		lua_pushstring(L, "set_platform_player_control: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);

	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			struct platform_data *platform = get_platform_data(polygon->permutation);
			if (platform)
			{
				SET_PLATFORM_IS_PLAYER_CONTROLLABLE(platform, state);
			}
		}
	}
	return 0;
}

static int L_Get_Platform_Player_Control(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_player_control: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			struct platform_data *platform = get_platform_data(polygon->permutation);
			if (platform)
			{
				lua_pushboolean(L, PLATFORM_IS_PLAYER_CONTROLLABLE(platform));
				return 1;
			}
		}
	}
	return 0;
}

static int L_Set_Platform_Monster_Control(lua_State *L)
{
	if (!lua_isnumber(L,1)) // boolean value can be any type, even nil
	{
		lua_pushstring(L, "set_platform_monster_control: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	bool state = lua_toboolean(L,2);

	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			struct platform_data *platform = get_platform_data(polygon->permutation);
			if (platform)
			{
				SET_PLATFORM_IS_MONSTER_CONTROLLABLE(platform, state);
			}
		}
	}
	return 0;
}

static int L_Get_Platform_Monster_Control(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_monster_control: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		if (polygon->type == _polygon_is_platform)
		{
			struct platform_data *platform = get_platform_data(polygon->permutation);
			if (platform)
			{
				lua_pushboolean(L, PLATFORM_IS_MONSTER_CONTROLLABLE(platform));
				return 1;
			}
		}
	}
	return 0;
}

static int L_Get_Platform_Speed(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_speed: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			lua_pushnumber(L, platform->speed);
			return 1;
		}
	}
	return 0;
}

static int L_Set_Platform_Speed(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_platform_speed: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	int16 speed = static_cast<int16>(lua_tonumber(L,2));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			platform->speed = speed;
		}
	}
	return 0;
}

static int L_Get_Platform_Floor_Height(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_floor_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			lua_pushnumber(L, (double)platform->floor_height/WORLD_ONE);
			return 1;
		}
	}
	return 0;
}

static int L_Get_Platform_Ceiling_Height(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_ceiling_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			lua_pushnumber(L, (double)platform->ceiling_height/WORLD_ONE);
			return 1;
		}
	}
	return 0;
}

static int L_Set_Platform_Floor_Height(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_platform_floor_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			platform->floor_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
			adjust_platform_endpoint_and_line_heights(polygon->permutation);
			adjust_platform_for_media(polygon->permutation, false);
		}
	}
	return 0;
}

static int L_Set_Platform_Ceiling_Height(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_platform_ceiling_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			platform->ceiling_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
			adjust_platform_endpoint_and_line_heights(polygon->permutation);
			adjust_platform_for_media(polygon->permutation, false);
		}
	}
	return 0;
}

static int L_Get_Platform_Movement(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_platform_movement: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			if PLATFORM_IS_EXTENDING(platform)
				lua_pushboolean(L, true);
			else
				lua_pushboolean(L, false);
			return 1;
		}
	}
	lua_pushboolean(L, false);
	return 1;
}

static int L_Set_Platform_Movement(lua_State *L)
{
	if (!lua_isnumber(L,1)) // bool can be any type
	{
		lua_pushstring(L, "set_platform_movement: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	bool movement = lua_toboolean(L,2);
	if (polygon && polygon->type == _polygon_is_platform)
	{
		struct platform_data *platform = get_platform_data(polygon->permutation);
		if (platform)
		{
			if (movement)
			{	SET_PLATFORM_IS_EXTENDING(platform); }
			else
			{	SET_PLATFORM_IS_CONTRACTING(platform); }
		}
	}
	return 0;
}

static int L_Get_Terminal_Text_Number(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_terminal_text_number: incorrect argument type");
		lua_error(L);
	}

	short polygon_index = static_cast<short>(lua_tonumber(L,1));
	short line_index = static_cast<short>(lua_tonumber(L,2));
	short side_index;
	if (line_side_has_control_panel(line_index, polygon_index, &side_index))
	{
		struct side_data *side_data = get_side_data(side_index);
		if (side_data && SIDE_IS_CONTROL_PANEL(side_data) && side_data->control_panel_type == _panel_is_computer_terminal)
		{
			lua_pushnumber(L, side_data->control_panel_permutation);
			return 1;
		}
	}
	lua_pushnumber(L, -1);
	return 1;
}

static int L_Set_Terminal_Text_Number(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "get_terminal_text_number: incorrect argument type");
		lua_error(L);
	}

	short polygon_index = static_cast<short>(lua_tonumber(L,1));
	short line_index = static_cast<short>(lua_tonumber(L,2));
	int16 terminal_text_id = static_cast<int16>(lua_tonumber(L,3));
	short side_index;
	if (line_side_has_control_panel(line_index, polygon_index, &side_index))
	{
		struct side_data *side_data = get_side_data(side_index);
		if (side_data && SIDE_IS_CONTROL_PANEL(side_data))
		{
			struct control_panel_definition *definition= get_control_panel_definition(side_data->control_panel_type);
			if (definition->_class == _panel_is_computer_terminal)
			{
				side_data->control_panel_permutation = terminal_text_id;
			}
		}
	}
	return 0;
}

static int L_Activate_Terminal(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "activate_terminal: incorrect argument type");
		lua_error(L);
	}

	short player_index = static_cast<short>(lua_tonumber(L,1));
	short text_index = static_cast<short>(lua_tonumber(L,2));
	if (player_index < 0 || player_index >= dynamic_world->player_count){
		lua_pushstring(L, "activate_terminal: invalid player index");
		lua_error(L);
	}
	
	enter_computer_interface(player_index, text_index, calculate_level_completion_state());
	
	return 0;
}

static int L_Get_Polygon_Floor_Height(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_polygon_floor_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		lua_pushnumber(L, (double)polygon->floor_height/WORLD_ONE);
		return 1;
	}
	return 0;
}

static int L_Set_Polygon_Floor_Height(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_polygon_floor_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		polygon->floor_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
		for(short i = 0; i<polygon->vertex_count;++i)
		{
			recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
			recalculate_redundant_line_data(polygon->line_indexes[i]);
		}
	}
	return 0;

}

static int L_Get_Polygon_Ceiling_Height(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_polygon_ceiling_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		lua_pushnumber(L, (double)polygon->ceiling_height/WORLD_ONE);
		return 1;
	}
	return 0;

}

static int L_Set_Polygon_Ceiling_Height(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_polygon_ceiling_height: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		polygon->ceiling_height = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
		for(short i = 0; i<polygon->vertex_count;++i)
		{
			recalculate_redundant_endpoint_data(polygon->endpoint_indexes[i]);
			recalculate_redundant_line_data(polygon->line_indexes[i]);
		}
	}
	return 0;

}

static int L_Get_Polygon_Type(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_polygon_type: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		lua_pushnumber(L, polygon->type);
		return 1;
	}
	return 0;
}

static int L_Set_Polygon_Type(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_polygon_type: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		polygon->type = static_cast<int>(lua_tonumber(L,2));
	}
	return 0;
}

static int L_Get_Polygon_Center(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_polygon_center: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		lua_pushnumber(L, polygon->center.x);
		lua_pushnumber(L, polygon->center.y);
		return 2;
	}
	return 0;
}

static int L_Get_Polygon_Permutation(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_polygon_permutation: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		lua_pushnumber(L, polygon->permutation);
		return 1;
	}
	return 0;
}

static int L_Set_Polygon_Permutation(lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_polygon_permutation: incorrect argument type");
		lua_error(L);
	}

	int polygon_index = static_cast<int>(lua_tonumber(L,1));
	struct polygon_data *polygon = get_polygon_data(short(polygon_index));
	if (polygon)
	{
		polygon->permutation = static_cast<int>(lua_tonumber(L,2));
	}
	return 0;
}

static int L_Item_Index_Valid(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "item_index_valid: incorrect argument type");
		lua_error(L);
	}
	short item_index = static_cast<int>(lua_tonumber(L,1));
	if(item_index < 0 || item_index >= MAXIMUM_OBJECTS_PER_MAP) {
		lua_pushnil(L);
	}
	else {
		struct object_data *object = GetMemberWithBounds(objects, item_index, MAXIMUM_OBJECTS_PER_MAP);
		lua_pushboolean(L, SLOT_IS_USED(object) && GET_OBJECT_OWNER(object) == _object_is_item);
	}
	return 1;
}

static int L_Get_Item_Type(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "get_item_type: incorrect argument type");
		lua_error(L);
	}
	short item_index = static_cast<int>(lua_tonumber(L,1));
	if(item_index < 0 || item_index >= MAXIMUM_OBJECTS_PER_MAP) {
		lua_pushstring(L, "get_item_type: invalid item index");
		lua_error(L);
	}
	struct object_data *object = GetMemberWithBounds(objects, item_index, MAXIMUM_OBJECTS_PER_MAP);
	if(!SLOT_IS_USED(object) || GET_OBJECT_OWNER(object) != _object_is_item) {
		lua_pushstring(L, "get_item_type: invalid item index");
		lua_error(L);
	}
	lua_pushnumber(L, object->permutation);
	return 1;
}

static int L_Get_Item_Polygon(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "get_item_polygon: incorrect argument type");
		lua_error(L);
	}
	short item_index = static_cast<int>(lua_tonumber(L,1));
	if(item_index < 0 || item_index >= MAXIMUM_OBJECTS_PER_MAP) {
		lua_pushstring(L, "get_item_polygon: invalid item index");
		lua_error(L);
	}
	struct object_data *object = GetMemberWithBounds(objects, item_index, MAXIMUM_OBJECTS_PER_MAP);
	if(!SLOT_IS_USED(object) || GET_OBJECT_OWNER(object) != _object_is_item) {
		lua_pushstring(L, "get_item_polygon: invalid item index");
		lua_error(L);
	}
	lua_pushnumber(L, object->polygon);
	return 1;
}

static int L_Delete_Item(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "delete_item: incorrect argument type");
		lua_error(L);
	}
	short item_index = static_cast<int>(lua_tonumber(L,1));
	if(item_index < 0 || item_index >= MAXIMUM_OBJECTS_PER_MAP) {
		lua_pushstring(L, "delete_item: invalid item index");
		lua_error(L);
	}
	struct object_data *object = GetMemberWithBounds(objects, item_index, MAXIMUM_OBJECTS_PER_MAP);
	if(!SLOT_IS_USED(object) || GET_OBJECT_OWNER(object) != _object_is_item) {
		lua_pushstring(L, "delete_item: invalid item index");
		lua_error(L);
	}
  remove_map_object(item_index);
	return 1;
}

static int L_New_Item(lua_State *L)
{
	int args = lua_gettop(L);

	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "new_item: incorrect argument type");
		lua_error(L);
	}
	short item_type = static_cast<int>(lua_tonumber(L,1));
	short polygon = static_cast<int>(lua_tonumber(L,2));
	short height = 0;

	if (args > 2)
	{
		if (!lua_isnumber(L,3))
		{
			lua_pushstring(L, "new_item: incorrect argument type");
			lua_error(L);
		}
		height = static_cast<int>(lua_tonumber(L,3));
	}

	object_location theLocation;
	struct polygon_data *destination;
	world_point3d theDestination;
	world_point2d theCenter;
	short index;

	theLocation.polygon_index = (int16)polygon;
	destination = NULL;
	destination= get_polygon_data(theLocation.polygon_index);
	if(destination==NULL)
		return 0;
	find_center_of_polygon(polygon, &theCenter);
	theDestination.x = theCenter.x;
	theDestination.y = theCenter.y;
	theDestination.z= height;
	theLocation.p = theDestination;
	theLocation.yaw = 0;
	theLocation.pitch = 0;
	theLocation.flags = 0;

	index = new_item(&theLocation, (short)item_type);
	if (index == NONE)
		return 0;
	lua_pushnumber(L, index);

	return 1;
}

static int L_Global_Random (lua_State *L)
{
	lua_pushnumber (L, global_random ());
	return 1;
}

static int L_Better_Random (lua_State *L)
{
	lua_pushnumber (L, lua_random_generator.KISS ());
	return 1;
}

static int L_Local_Random (lua_State *L)
{
	lua_pushnumber (L, local_random ());
	return 1;
}

static int L_Award_Points (lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "award_points: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	int points = static_cast<int>(lua_tonumber(L,2));

	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "award_points: invalid player index");
		lua_error (L);
	}
	player_data *player = get_player_data (player_index);

	player -> netgame_parameters[0] += points;
#if !defined(DISABLE_NETWORKING)
	team_netgame_parameters[player->team][0] += points;
#endif // !defined(DISABLE_NETWORKING)

	if(points != 0)
		mark_player_network_stats_as_dirty(current_player_index);

	return 0;
}

static int L_Award_Kills (lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "award_kills: incorrect argument type");
		lua_error(L);
	}

	int aggressor_player_index = static_cast<int>(lua_tonumber(L,1));
	int slain_player_index = static_cast<int>(lua_tonumber(L,2));
	int kills = static_cast<int>(lua_tonumber(L,3));

	if (slain_player_index < 0 || slain_player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "award_kills: invalid player index");
		lua_error (L);
	}
	player_data *slain_player = get_player_data (slain_player_index);

	if (aggressor_player_index == -1) {
		slain_player -> monster_damage_taken.kills += kills;
		team_monster_damage_taken[slain_player->team].kills += kills;
	}
	else
	{
		if (aggressor_player_index < 0 || aggressor_player_index >= dynamic_world->player_count)
		{
			lua_pushstring (L, "award_kills: invalid player index");
			lua_error (L);
		}
		slain_player -> damage_taken [aggressor_player_index].kills += kills;
		struct player_data *aggressor_player = get_player_data(aggressor_player_index);
		team_damage_taken[slain_player->team].kills += kills;
		if (slain_player_index != aggressor_player_index) {
			team_damage_given[slain_player->team].kills += kills;
		}
		if (slain_player->team == aggressor_player->team) {
			team_friendly_fire[slain_player->team].kills += kills;
		}
	}

	if(kills != 0)
		mark_player_network_stats_as_dirty(current_player_index);

	return 0;
}

static int L_Set_Points (lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "set_points: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));
	int points = static_cast<int>(lua_tonumber(L,2));

	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "set_points: invalid player index");
		lua_error (L);
	}
	player_data *player = get_player_data (player_index);

	if(player->netgame_parameters[0] != points)
	{
#if !defined(DISABLE_NETWORKING)
		team_netgame_parameters[player->team][0] += points - player->netgame_parameters[0];
#endif // !defined(DISABLE_NETWORKING)
		player -> netgame_parameters[0] = points;
		mark_player_network_stats_as_dirty(current_player_index);
	}

	return 0;
}

static int L_Set_Kills (lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_kills: incorrect argument type");
		lua_error(L);
	}

	int aggressor_player_index = static_cast<int>(lua_tonumber(L,1));
	int slain_player_index = static_cast<int>(lua_tonumber(L,2));
	int kills = static_cast<int>(lua_tonumber(L,3));

	if (slain_player_index < 0 || slain_player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "set_kills: invalid player index");
		lua_error (L);
	}
	player_data *slain_player = get_player_data (slain_player_index);

	bool score_changed = false;
	
	if (aggressor_player_index == -1)
	{
		if(slain_player->monster_damage_taken.kills != kills)
		{
			team_monster_damage_taken[slain_player->team].kills += 
				(kills - slain_player->monster_damage_taken.kills);
			slain_player -> monster_damage_taken.kills = kills;
			score_changed = true;
		}
	}
	else
	{
		if (aggressor_player_index < 0 || aggressor_player_index >= dynamic_world->player_count)
		{
			lua_pushstring (L, "set_kills: invalid player index");
			lua_error (L);
		}
		if(slain_player->damage_taken[aggressor_player_index].kills != kills)
		{
			struct player_data *aggressor_player = get_player_data(aggressor_player_index);
			team_damage_taken[slain_player->team].kills += kills - slain_player->damage_taken[aggressor_player_index].kills;
			if (slain_player_index != aggressor_player_index) {
				team_damage_given[aggressor_player->team].kills += kills - slain_player->damage_taken[aggressor_player_index].kills;
			}
			if (slain_player->team == aggressor_player->team) {
				team_friendly_fire[slain_player->team].kills += kills - slain_player->damage_taken[aggressor_player_index].kills;
			}
			slain_player -> damage_taken [aggressor_player_index].kills = kills;
			score_changed = true;
		}
	}

	if(score_changed)
		mark_player_network_stats_as_dirty(current_player_index);

	return 0;
}

static int L_Get_Points (lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_points: incorrect argument type");
		lua_error(L);
	}

	int player_index = static_cast<int>(lua_tonumber(L,1));

	if (player_index < 0 || player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "get_points: invalid player index");
		lua_error (L);
	}
	player_data *player = get_player_data (player_index);

	lua_pushnumber (L, player -> netgame_parameters[0]);

	return 1;
}

static int L_Get_Kills (lua_State *L)
{
	if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
	{
		lua_pushstring(L, "get_kills: incorrect argument type");
		lua_error(L);
	}

	int aggressor_player_index = static_cast<int>(lua_tonumber(L,1));
	int slain_player_index = static_cast<int>(lua_tonumber(L,2));

	if (slain_player_index < 0 || slain_player_index >= dynamic_world->player_count)
	{
		lua_pushstring (L, "get_kills: invalid player index");
		lua_error (L);
	}
	player_data *slain_player = get_player_data (slain_player_index);

	if (aggressor_player_index == -1)
		lua_pushnumber (L, slain_player -> monster_damage_taken.kills);
	else
	{
		if (aggressor_player_index < 0 || aggressor_player_index >= dynamic_world->player_count)
		{
			lua_pushstring (L, "get_kills: invalid player index");
			lua_error (L);
		}
		lua_pushnumber (L, slain_player -> damage_taken [aggressor_player_index].kills);
	}

	return 1;
}

static int L_Use_Lua_Compass (lua_State *L)
{

	int args = lua_gettop(L);

	if (args == 1)
	{
		if (!lua_isboolean (L, 1))
		{
			lua_pushstring (L, "use_lua_compass: incorrect argument type");
			lua_error (L);
		}

		for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
		{
			use_lua_compass [i] = lua_toboolean (L, 1);
		}
	}

	if (args == 2)
	{
		if (!lua_isnumber (L, 1) || !lua_isboolean (L, 2))
		{
			lua_pushstring (L, "use_lua_compass: incorrect argument type");
			lua_error (L);
		}

		int player_index = static_cast<int>(lua_tonumber(L,1));

		use_lua_compass [player_index] = lua_toboolean (L, 2);
	}

	return 0;
}

static int L_Set_Lua_Compass_State (lua_State *L)
{
	if (!lua_isnumber (L, 1) || !lua_isnumber (L, 2))
	{
		lua_pushstring (L, "set_lua_compass_state: incorrect argument type");
		lua_error (L);
	}

	int player_index = static_cast<int>(lua_tonumber (L, 1));
	int compass_state = static_cast<int>(lua_tonumber (L, 2));
	lua_compass_states [player_index] = compass_state;
	return 0;
}

static int L_Set_Lua_Compass_Beacon (lua_State *L)
{
	if (!lua_isnumber (L, 1) || !lua_isnumber (L, 2) || !lua_isnumber (L, 3))
	{
		lua_pushstring (L, "set_lua_compass_beacon: incorrect argument type");
		lua_error (L);
	}

	int player_index = static_cast<int>(lua_tonumber (L, 1));
	int beacon_x = static_cast<world_distance>(lua_tonumber(L,2)*WORLD_ONE);
	int beacon_y = static_cast<world_distance>(lua_tonumber(L,3)*WORLD_ONE);
	lua_compass_beacons [player_index].x = beacon_x;
	lua_compass_beacons [player_index].y = beacon_y;
	return 0;
}

static int L_Get_Projectile_Type (lua_State *L)
{
	if (!lua_isnumber (L, 1))
	{
		lua_pushstring (L, "get_projectile_type: incorrect argument type");
		lua_error (L);
	}
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}
	
	lua_pushnumber (L, projectile->type);
	
	return 1;
}

extern projectile_definition *get_projectile_definition(short type);

static int L_Get_Projectile_Damage_Type (lua_State *L)
{
	if (!lua_isnumber (L, 1))
	{
		lua_pushstring (L, "get_projectile_type: incorrect argument type");
		lua_error (L);
	}
	
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}
	
	struct projectile_definition *definition = get_projectile_definition(projectile->type);
	lua_pushnumber (L, definition->damage.type);

	return 1;
}

static int L_Get_Projectile_Owner (lua_State *L)
{
	if (!lua_isnumber (L, 1))
	{
		lua_pushstring (L, "get_projectile_owner: incorrect argument type");
		lua_error (L);
	}
	
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_owner: invalid projectile index");
		lua_error(L);
	}

	if(projectile->owner_index != NONE)
		lua_pushnumber (L, projectile->owner_index);
	else
		lua_pushnil(L);

	return 1;
}

static int L_Set_Projectile_Owner (lua_State *L)
{
	
	if (!lua_isnumber (L, 1) || (!lua_isnumber (L, 2) && !lua_isnil (L, 2)))
	{
		lua_pushstring (L, "set_projectile_owner: incorrect argument type");
		lua_error (L);
	}
	
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "set_projectile_owner: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "set_projectile_owner: invalid projectile index");
		lua_error(L);
	}
	
	if(lua_isnil(L, 2))
		projectile->owner_index = NONE;
	else {
		int monster_index = static_cast<int>(lua_tonumber (L, 2));
		struct monster_data *monster= get_monster_data(monster_index);
		if(!SLOT_IS_USED(monster))
		{
			lua_pushstring(L, "set_projectile_owner: invalid monster index");
			lua_error(L);
		}
		projectile->owner_index = monster_index;
	}
	
	return 0;
}

static int L_Get_Projectile_Target (lua_State *L)
{
	
	if (!lua_isnumber (L, 1))
	{
		lua_pushstring (L, "get_projectile_target: incorrect argument type");
		lua_error (L);
	}
	
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "get_projectile_target: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_target: invalid projectile index");
		lua_error(L);
	}
	
	if(projectile->target_index != NONE)
		lua_pushnumber (L, projectile->target_index);
	else
		lua_pushnil(L);

	return 1;
}

static int L_Set_Projectile_Target (lua_State *L)
{
	
	if (!lua_isnumber (L, 1) || (!lua_isnumber (L, 2) && !lua_isnil (L, 2)))
	{
		lua_pushstring (L, "set_projectile_target: incorrect argument type");
		lua_error (L);
	}
	
	int projectile_index = static_cast<int>(lua_tonumber (L, 1));
	
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP)
	{
		lua_pushstring(L, "set_projectile_target: invalid projectile index");
		lua_error(L);
	}

	struct projectile_data *projectile= get_projectile_data(projectile_index);
	
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "set_projectile_target: invalid projectile index");
		lua_error(L);
	}
	
	if(lua_isnil(L, 2))
		projectile->target_index = NONE;
	else {
		int monster_index = static_cast<int>(lua_tonumber (L, 2));
		struct monster_data *monster= get_monster_data(monster_index);
		if(!SLOT_IS_USED(monster))
		{
			lua_pushstring(L, "set_projectile_owner: invalid monster index");
			lua_error(L);
		}
		projectile->target_index = monster_index;
	}
	
	return 0;
}

static int L_Projectile_Index_Valid(lua_State *L) {
	if(!lua_isnumber(L,1)) {
		lua_pushstring(L, "projectile_index_valid: incorrect argument type");
		lua_error(L);
	}
	short projectile_index = static_cast<int>(lua_tonumber(L,1));
	if(projectile_index < 0 || projectile_index >= MAXIMUM_PROJECTILES_PER_MAP) {
		lua_pushnil(L);
	}
	else {
		struct projectile_data* projectile;
		projectile = GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
		lua_pushboolean(L, SLOT_IS_USED(projectile));
	}
	return 1;
}

static int L_Get_Projectile_Angle(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_projectile_angle: incorrect argument type");
		lua_error(L);
	}
	int projectile_index = static_cast<int>(lua_tonumber(L,1));

	struct projectile_data *projectile = GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_angle: invalid projectile index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(projectile->object_index);

	lua_pushnumber(L, (double)object->facing*AngleConvert);
	lua_pushnumber(L, (double)projectile->elevation*AngleConvert);
	return 2;
}

static int L_Set_Projectile_Angle(lua_State *L)
{
	if (!lua_isnumber(L,1)||!lua_isnumber(L,2)||!lua_isnumber(L,3))
	{
		lua_pushstring(L, "set_projectile_angle: incorrect argument type");
		lua_error(L);
	}
	int projectile_index = static_cast<int>(lua_tonumber(L,1));

	struct projectile_data *projectile = GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "set_projectile_angle: invalid projectile index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(projectile->object_index);

	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	projectile->elevation = static_cast<int>(lua_tonumber(L, 3) / AngleConvert);

	return 0;
}

static int L_Get_Projectile_Position(lua_State *L)
{
	if (!lua_isnumber(L,1))
	{
		lua_pushstring(L, "get_projectile_position: incorrect argument type");
		lua_error(L);
	}
	int projectile_index = static_cast<int>(lua_tonumber(L,1));

	struct projectile_data *projectile = GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "get_projectile_position: invalid projectile index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(projectile->object_index);

	lua_pushnumber(L, (double)object->polygon);
	lua_pushnumber(L, (double)object->location.x / WORLD_ONE);
	lua_pushnumber(L, (double)object->location.y / WORLD_ONE);
	lua_pushnumber(L, (double)object->location.z / WORLD_ONE);
	return 4;
}

static int L_Set_Projectile_Position(lua_State *L)
{
	if (!lua_isnumber(L,1)||!lua_isnumber(L,2)||!lua_isnumber(L,3)||!lua_isnumber(L,4)||!lua_isnumber(L,5))
	{
		lua_pushstring(L, "set_projectile_position: incorrect argument type");
		lua_error(L);
	}
	int projectile_index = static_cast<int>(lua_tonumber(L,1));

	struct projectile_data *projectile = GetMemberWithBounds(projectiles,projectile_index,MAXIMUM_PROJECTILES_PER_MAP);
	if(!SLOT_IS_USED(projectile))
	{
		lua_pushstring(L, "set_projectile_position: invalid projectile index");
		lua_error(L);
	}
	struct object_data *object= get_object_data(projectile->object_index);

	object->polygon = static_cast<int>(lua_tonumber(L, 2));
	object->location.x = static_cast<int>(lua_tonumber(L, 3)*WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 4)*WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 5)*WORLD_ONE);
	
	return 0;
}

/*
Yet to be implemented:
get/set_projectile_gravity
detonate_projectile
detonate_new_projectile
new_projectile
*/

static int L_Fade_Music(lua_State* L)
{
	short duration;
	if(!lua_isnumber(L, 1))
		duration = 60;
	else
		duration = (short)(lua_tonumber(L, 1) * 60);
	fade_out_music(duration);
	Playlist.clear();
	return 0;
}

static int L_Clear_Music(lua_State* L)
{
	Playlist.clear();
	return 0;
}

static int L_Play_Music(lua_State* L)
{
	bool restart_music;
	int n;
	restart_music = !IsLevelMusicActive() && !music_playing();
	for(n = 1; n <= lua_gettop(L); n++) {
		if(!lua_isstring(L, n)) {
			lua_pushstring(L, "play_music: invalid file specifier");
			lua_error(L);
		}
		FileSpecifier file;
		if(file.SetNameWithPath(lua_tostring(L, n)))
			Playlist.push_back(file);
	}
	if(restart_music)
		PreloadLevelMusic();
	return 0;
}

static int L_Stop_Music(lua_State* L)
{
	Playlist.clear();
// jkvw: This is ugly and wrong. Please fix.
#ifndef SDL
	stop_music();
#endif
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

static int L_Player_Media(lua_State *L)
{
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	 {
		lua_pushstring(L, "set_player_position: invalid player index");
		lua_error(L);
	 }
	player_data *player = get_player_data(player_index);
	struct polygon_data* polygon = get_polygon_data(player->supporting_polygon_index);
	if (polygon->media_index!=NONE)
			{
				media_data *media = get_media_data(polygon->media_index);
				if (media)
				 {
					if (player->camera_location.z<media->height)
					 {
						lua_pushnumber(L, media->type);
						return 1;
					 }
				 }
			}
	lua_pushnil(L);
	return 1;
}

static int L_Get_Player_Weapon(lua_State *L)
{
	int player_index = static_cast<int>(lua_tonumber(L,1));
	if (player_index < 0 || player_index >= dynamic_world->player_count)
	 {
		lua_pushstring(L, "get_player_weapon: invalid player index");
		lua_error(L);
	 }
	if(player_has_valid_weapon(player_index))
	 {
		struct player_weapon_data *player_weapons= get_player_weapon_data(player_index);
		lua_pushnumber(L, player_weapons->current_weapon);
	 }
	else {
		lua_pushnil(L);
		}
	return 1;
}

void RegisterLuaFunctions()
{
	lua_register(state, "number_of_polygons", L_Number_of_Polygons);
	lua_register(state, "local_player_index", L_Local_Player_Index);
	lua_register(state, "player_to_monster_index", L_Player_To_Monster_Index);
	lua_register(state, "number_of_players", L_Number_of_Players);
	lua_register(state, "screen_print", L_Screen_Print);
	//lua_register(state, "display_text", L_Display_Text);
	lua_register(state, "inflict_damage", L_Inflict_Damage);
	lua_register(state, "enable_player", L_Enable_Player);
	lua_register(state, "disable_player", L_Disable_Player);
	lua_register(state, "kill_script", L_Kill_Script);
	lua_register(state, "hide_interface", L_Hide_Interface);
	lua_register(state, "show_interface", L_Show_Interface);
	lua_register(state, "get_tag_state", L_Get_Tag_State);
	lua_register(state, "set_tag_state", L_Set_Tag_State);
	lua_register(state, "get_life", L_Get_Life);
	lua_register(state, "set_life", L_Set_Life);
	lua_register(state, "get_oxygen", L_Get_Oxygen);
	lua_register(state, "set_oxygen", L_Set_Oxygen);
	lua_register(state, "add_item", L_Add_Item);
	lua_register(state, "remove_item", L_Remove_Item);
	lua_register(state, "count_item", L_Count_Item);
	lua_register(state, "destroy_ball", L_Destroy_Ball);
	lua_register(state, "select_weapon", L_Select_Weapon);
	lua_register(state, "set_platform_state", L_Set_Platform_State);
	lua_register(state, "get_platform_state", L_Get_Platform_State);
	lua_register(state, "set_light_state", L_Set_Light_State);
	lua_register(state, "get_light_state", L_Get_Light_State);
	lua_register(state, "set_fog_depth", L_Set_Fog_Depth);
	lua_register(state, "set_fog_color", L_Set_Fog_Color);
	lua_register(state, "set_fog_present", L_Set_Fog_Present);
	lua_register(state, "set_fog_affects_landscapes", L_Set_Fog_Affects_Landscapes);
	lua_register(state, "get_fog_depth", L_Get_Fog_Depth);
	lua_register(state, "get_fog_color", L_Get_Fog_Color);
	lua_register(state, "get_fog_present", L_Get_Fog_Present);
	lua_register(state, "get_fog_affects_landscapes", L_Get_Fog_Affects_Landscapes);
	lua_register(state, "set_underwater_fog_depth", L_Set_Underwater_Fog_Depth);
	lua_register(state, "set_underwater_fog_color", L_Set_Underwater_Fog_Color);
	lua_register(state, "set_underwater_fog_present", L_Set_Underwater_Fog_Present);
	lua_register(state, "set_underwater_fog_affects_landscapes", L_Set_Underwater_Fog_Affects_Landscapes);
	lua_register(state, "get_underwater_fog_depth", L_Get_Underwater_Fog_Depth);
	lua_register(state, "get_underwater_fog_color", L_Get_Underwater_Fog_Color);
	lua_register(state, "get_underwater_fog_present", L_Get_Underwater_Fog_Present);
	lua_register(state, "get_underwater_fog_affects_landscapes", L_Get_Underwater_Fog_Affects_Landscapes);
	lua_register(state, "get_all_fog_attributes", L_Get_All_Fog_Attributes);
	lua_register(state, "set_all_fog_attributes", L_Set_All_Fog_Attributes);
	lua_register(state, "new_monster", L_New_Monster);
	lua_register(state, "activate_monster", L_Activate_Monster);
	lua_register(state, "deactivate_monster", L_Deactivate_Monster);
	lua_register(state, "monster_index_valid", L_Monster_Index_Valid);
	lua_register(state, "get_monster_type", L_Get_Monster_Type);
	lua_register(state, "get_monster_type_class", L_Get_Monster_Type_Class);
	lua_register(state, "set_monster_type_class", L_Set_Monster_Type_Class);
	lua_register(state, "damage_monster", L_Damage_Monster);
	lua_register(state, "attack_monster", L_Attack_Monster);
	lua_register(state, "move_monster", L_Move_Monster);
	lua_register(state, "select_monster", L_Select_Monster);
	lua_register(state, "get_monster_position", L_Get_Monster_Position);
	lua_register(state, "get_monster_facing", L_Get_Monster_Facing);
	lua_register(state, "get_monster_polygon", L_Get_Monster_Polygon);
	lua_register(state, "get_monster_immunity", L_Get_Monster_Immunity);
	lua_register(state, "set_monster_immunity", L_Set_Monster_Immunity);
	lua_register(state, "get_monster_weakness", L_Get_Monster_Weakness);
	lua_register(state, "set_monster_weakness", L_Set_Monster_Weakness);
	lua_register(state, "get_monster_friend", L_Get_Monster_Friend);
	lua_register(state, "set_monster_friend", L_Set_Monster_Friend);
	lua_register(state, "get_monster_enemy", L_Get_Monster_Enemy);
	lua_register(state, "set_monster_enemy", L_Set_Monster_Enemy);
	lua_register(state, "get_monster_item", L_Get_Monster_Item);
	lua_register(state, "set_monster_item", L_Set_Monster_Item);
	lua_register(state, "get_monster_action", L_Get_Monster_Action);
	lua_register(state, "get_monster_mode", L_Get_Monster_Mode);
	lua_register(state, "get_monster_vitality", L_Get_Monster_Vitality);
	lua_register(state, "set_monster_vitality", L_Set_Monster_Vitality);
	//lua_register(state, "set_monster_global_speed", L_Set_Monster_Global_Speed);
	lua_register(state, "get_player_position", L_Get_Player_Position);
	lua_register(state, "get_player_polygon", L_Get_Player_Polygon);
	lua_register(state, "set_player_position", L_Set_Player_Position);
	lua_register(state, "get_player_angle", L_Get_Player_Angle);
	lua_register(state, "set_player_angle", L_Set_Player_Angle);
	lua_register(state, "get_player_color", L_Get_Player_Color);
	lua_register(state, "set_player_color", L_Set_Player_Color);
	lua_register(state, "get_player_team", L_Get_Player_Team);
	lua_register(state, "set_player_team", L_Set_Player_Team);
	lua_register(state, "get_player_name", L_Get_Player_Name);
	lua_register(state, "get_player_powerup_duration", L_Get_Player_Powerup_Duration);
	lua_register(state, "set_player_powerup_duration", L_Set_Player_Powerup_Duration);
	lua_register(state, "get_player_internal_velocity", L_Get_Player_Internal_Velocity);
	lua_register(state, "get_player_external_velocity", L_Get_Player_External_Velocity);
	lua_register(state, "set_player_external_velocity", L_Set_Player_External_Velocity);
	lua_register(state, "add_to_player_external_velocity", L_Add_To_Player_External_Velocity);
	lua_register(state, "accelerate_player", L_Accelerate_Player);
	lua_register(state, "player_is_dead", L_Player_Is_Dead);
	lua_register(state, "player_control", L_Player_Control);
	lua_register(state, "teleport_player", L_Teleport_Player);
	lua_register(state, "teleport_player_to_level", L_Teleport_Player_To_Level);
	//lua_register(state, "set_player_global_speed", L_Set_Player_Global_Speed);
	lua_register(state, "set_platform_player_control", L_Set_Platform_Player_Control);
	lua_register(state, "get_platform_player_control", L_Get_Platform_Player_Control);
	lua_register(state, "set_platform_monster_control", L_Set_Platform_Monster_Control);
	lua_register(state, "get_platform_monster_control", L_Get_Platform_Monster_Control);
	lua_register(state, "get_platform_speed", L_Get_Platform_Speed);
	lua_register(state, "set_platform_speed", L_Set_Platform_Speed);
	lua_register(state, "get_platform_floor_height", L_Get_Platform_Floor_Height);
	lua_register(state, "get_platform_ceiling_height", L_Get_Platform_Ceiling_Height);
	lua_register(state, "set_platform_floor_height", L_Set_Platform_Floor_Height);
	lua_register(state, "set_platform_ceiling_height", L_Set_Platform_Ceiling_Height);
	lua_register(state, "set_platform_movement", L_Set_Platform_Movement);
	lua_register(state, "get_platform_movement", L_Get_Platform_Movement);
	lua_register(state, "get_platform_index", L_Get_Polygon_Permutation);
	lua_register(state, "set_motion_sensor_state", L_Set_Motion_Sensor_State);
	lua_register(state, "get_motion_sensor_state", L_Get_Motion_Sensor_State);
	lua_register(state, "create_camera", L_Create_Camera);
	lua_register(state, "add_path_point", L_Add_Path_Point);
	lua_register(state, "add_path_angle", L_Add_Path_Angle);
	lua_register(state, "activate_camera", L_Activate_Camera);
	lua_register(state, "deactivate_camera", L_Deactivate_Camera);
	lua_register(state, "clear_camera", L_Clear_Camera);
	lua_register(state, "crosshairs_active", L_Crosshairs_Active);
	lua_register(state, "set_crosshairs_state", L_Set_Crosshairs_State);
	lua_register(state, "zoom_active", L_Zoom_Active);
	lua_register(state, "set_zoom_state", L_Set_Zoom_State);
	lua_register(state, "play_sound", L_Play_Sound);
	lua_register(state, "screen_fade", L_Screen_Fade);
	lua_register(state, "start_fade", L_Screen_Fade);
	lua_register(state, "get_terminal_text_number", L_Get_Terminal_Text_Number);
	lua_register(state, "set_terminal_text_number", L_Set_Terminal_Text_Number);
	lua_register(state, "activate_terminal", L_Activate_Terminal);
	lua_register(state, "get_polygon_floor_height", L_Get_Polygon_Floor_Height);
	lua_register(state, "get_polygon_ceiling_height", L_Get_Polygon_Ceiling_Height);
	lua_register(state, "set_polygon_floor_height", L_Set_Polygon_Floor_Height);
	lua_register(state, "set_polygon_ceiling_height", L_Set_Polygon_Ceiling_Height);
	lua_register(state, "get_polygon_type", L_Get_Polygon_Type);
	lua_register(state, "set_polygon_type", L_Set_Polygon_Type);
	lua_register(state, "get_polygon_center", L_Get_Polygon_Center);
	lua_register(state, "get_polygon_permutation", L_Get_Polygon_Permutation);
	lua_register(state, "set_polygon_permutation", L_Set_Polygon_Permutation);
	lua_register(state, "get_polygon_target", L_Get_Polygon_Permutation);
	lua_register(state, "set_polygon_target", L_Set_Polygon_Permutation);
	lua_register(state, "item_index_valid", L_Item_Index_Valid);
	lua_register(state, "get_item_type", L_Get_Item_Type);
	lua_register(state, "get_item_polygon", L_Get_Item_Polygon);
	lua_register(state, "delete_item", L_Delete_Item);
	lua_register(state, "new_item", L_New_Item);
	lua_register(state, "global_random", L_Global_Random);
	lua_register(state, "better_random", L_Better_Random);
	lua_register(state, "local_random", L_Local_Random);
	lua_register(state, "award_points", L_Award_Points);
	lua_register(state, "award_kills", L_Award_Kills);
	lua_register(state, "set_points", L_Set_Points);
	lua_register(state, "set_kills", L_Set_Kills);
	lua_register(state, "get_points", L_Get_Points);
	lua_register(state, "get_kills", L_Get_Kills);
	lua_register(state, "use_lua_compass", L_Use_Lua_Compass);
	lua_register(state, "set_lua_compass_state", L_Set_Lua_Compass_State);
	lua_register(state, "set_lua_compass_beacon", L_Set_Lua_Compass_Beacon);
	lua_register(state, "get_projectile_type", L_Get_Projectile_Type);
	lua_register(state, "get_projectile_damage_type", L_Get_Projectile_Damage_Type);
	lua_register(state, "get_projectile_owner", L_Get_Projectile_Owner);
	lua_register(state, "set_projectile_owner", L_Set_Projectile_Owner);
	lua_register(state, "get_projectile_target", L_Get_Projectile_Target);
	lua_register(state, "set_projectile_target", L_Set_Projectile_Target);
	lua_register(state, "projectile_index_valid", L_Projectile_Index_Valid);
	lua_register(state, "get_projectile_angle", L_Get_Projectile_Angle);
	lua_register(state, "set_projectile_angle", L_Set_Projectile_Angle);
	lua_register(state, "get_projectile_position", L_Get_Projectile_Position);
	lua_register(state, "set_projectile_position", L_Set_Projectile_Position);
	lua_register(state, "prompt", L_Prompt);
	lua_register(state, "player_media", L_Player_Media);
	lua_register(state, "get_player_weapon", L_Get_Player_Weapon);
	lua_register(state, "fade_music", L_Fade_Music);
	lua_register(state, "clear_music", L_Clear_Music);
	lua_register(state, "play_music", L_Play_Music);
	lua_register(state, "stop_music", L_Stop_Music);
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
#define LUA_ACCESSING
#include "language_definition.h"
#undef LUA_ACCESSING
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

bool RunLuaScript()
{
	for (int i = 0; i < MAXIMUM_NUMBER_OF_NETWORK_PLAYERS; i++)
		use_lua_compass [i] = false;
	if (!lua_loaded)
		return false;
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

void CloseLuaScript()
{
	if (lua_loaded)
		lua_close(state);
	lua_loaded = false;
	lua_running = false;
	lua_cameras.resize(0);
	number_of_cameras = 0;
	
	sLuaNetscriptLoaded = false;
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
