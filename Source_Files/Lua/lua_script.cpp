/*
Created 5-20-03 by Matthew Hielscher
Controls the loading and execution of Lua scripts.
*/
static int dummy;
#ifdef HAVE_LUA
extern "C"
{
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include <string>
using namespace std;

#include "cseries.h"
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
#include "weapons.h"
#include "monsters.h"
#include "flood_map.h"
#include "vbl.h"
#include "OGL_Setup.h"
#include "mysound.h"

#include "script_instructions.h"
#include "lua_script.h"

#define DONT_REPEAT_DEFINITIONS
#include "item_definitions.h"
#include "monster_definitions.h"

// Steal all this stuff
extern bool ready_weapon(short player_index, short weapon_index);
extern void DisplayText(short BaseX, short BaseY, const char *Text);

extern void advance_monster_path(short monster_index);
extern long monster_pathfinding_cost_function(short source_polygon_index, short line_index,
	short destination_polygon_index, void *data);
extern void set_monster_action(short monster_index, short action);
extern void set_monster_mode(short monster_index, short new_mode, short target_index);

extern void ShootForTargetPoint(bool ThroughWalls, world_point3d& StartPosition, world_point3d& EndPosition, short& Polygon);
extern void select_next_best_weapon(short player_index);

struct monster_pathfinding_data
{
	struct monster_definition *definition;
	struct monster_data *monster;
	
	bool cross_zone_boundaries;
};

extern ActionQueues *sPfhortranActionQueues;
extern struct view_data *world_view;

// globals
lua_State *state;
bool lua_loaded = false;
bool lua_running = false;
static vector<short> IntersectedObjects;

vector<lua_camera> lua_cameras;
int number_of_cameras = 0;

double FindLinearValue(double startValue, double endValue, double timeRange, double timeTaken)
{
    return (((endValue-startValue)/timeRange)*timeTaken)+startValue;
}

world_point3d FindLinearValue(world_point3d startPoint, world_point3d endPoint, double timeRange, double timeTaken)
{
    world_point3d realPoint;
    realPoint.x = (((double)(endPoint.x-startPoint.x)/timeRange)*timeTaken)+startPoint.x;
    realPoint.y = (((double)(endPoint.y-startPoint.y)/timeRange)*timeTaken)+startPoint.y;
    realPoint.z = (((double)(endPoint.z-startPoint.z)/timeRange)*timeTaken)+startPoint.z;
    return realPoint;
}

static const luaL_reg lualibs[] =
{
	{"base", luaopen_base},
	{"table", luaopen_table},
	{"io", luaopen_io},
	{"string", luaopen_string},
	{"math", luaopen_math},
	{"debug", luaopen_debug},
	{"loadlib", luaopen_loadlib}, 
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

void L_Call_Init()
{
    if (!lua_running)
        return;
    lua_pushstring(state, "init");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_call(state, 0, 0);
}

void L_Call_Idle()
{
    if (!lua_running)
        return;
    lua_pushstring(state, "idle");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_call(state, 0, 0);
}

void L_Call_Tag_Switch(short tag)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "tag_switch");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, tag);
    lua_call(state, 1, 0);
}

void L_Call_Light_Switch(short light)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "light_switch");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, light);
    lua_call(state, 1, 0);
}

void L_Call_Platform_Switch(short platform)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "platform_switch");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, platform);
    lua_call(state, 1, 0);
}

void L_Call_Terminal_Enter(short terminal_id)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "terminal_enter");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, terminal_id);
    lua_call(state, 1, 0);
}

void L_Call_Terminal_Exit(short terminal_id)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "terminal_exit");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, terminal_id);
    lua_call(state, 1, 0);
}

void L_Call_Pattern_Buffer(short buffer_id)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "pattern_buffer");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, buffer_id);
    lua_call(state, 1, 0);
}

void L_Call_Got_Item(short type)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "got_item");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, type);
    lua_call(state, 1, 0);
}

void L_Call_Light_Activated(short index)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "light_activated");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, index);
    lua_call(state, 1, 0);
}

void L_Call_Platform_Activated(short index)
{
    if (!lua_running)
        return;
    lua_pushstring(state, "platform_activated");
    lua_gettable(state, LUA_GLOBALSINDEX);
    if (!lua_isfunction(state, -1))
    {
        lua_pop(state, 1);
        return;
    }
    lua_pushnumber(state, index);
    lua_call(state, 1, 0);
}

static int L_Number_of_Players(lua_State *L)
{
    lua_pushnumber(L, dynamic_world->player_count);
    return 1;
}

static int L_Screen_Print(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isstring(L,2))
    {
        logError("screen_print: incorrect argument type");
        lua_pushstring(L, "screen_print: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    if (local_player_index != player_index)
        return 0;
    screen_printf(lua_tostring(L, 2));
    return 0;
}

static int L_Display_Text(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isstring(L,2) || !lua_isnumber(L,3) || !lua_isnumber(L,3))
    {
        logError("display_text: incorrect argument type");
        lua_pushstring(L, "display_text: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    if (local_player_index != player_index)
        return 0;
    short x = lua_tonumber(L,3);
    short y = lua_tonumber(L,4);
    DisplayText(x, y, lua_tostring(L, 2));
    screen_printf(lua_tostring(L,2));
    return 0;
}

static int L_Inflict_Damage(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("inflict_damage: incorrect argument type");
        lua_pushstring(L, "inflict_damage: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
        return 0;

    struct damage_definition damage;
    float temp = lua_tonumber(L,2);

    damage.flags= _alien_damage;
    damage.type= _damage_crushing;
    damage.base= int16(temp);
    damage.random= 0;
    damage.scale= FIXED_ONE;

    damage_player(player->monster_index, NONE, NONE, &damage);
    
    return 0;
}

static int L_Enable_Player(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("enable_player: incorrect argument type");
        lua_pushstring(L, "enable_player: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
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
        logError("disable_player: incorrect argument type");
        lua_pushstring(L, "disable_player: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
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
/* This is not worth its trouble for now.
static int L_Hide_Interface(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        lua_pushstring(L, "hide_interface: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    
    if (local_player != player)
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
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    
    if (local_player != player)
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
*/
static int L_Set_Tag_State(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("set_tag_state: incorrect argument type");
        lua_pushstring(L, "set_tag_state: incorrect argument type");
        lua_error(L);
    }
    int tag = lua_tonumber(L,1);
    int tag_state = lua_tonumber(L,2);
    
    set_tagged_light_statuses(int16(tag), (tag_state < -0.5) || (tag_state > 0.5));
    try_and_change_tagged_platform_states(int16(tag), (tag_state < -0.5) || (tag_state > 0.5)); 
    assume_correct_switch_position(_panel_is_tag_switch, int16(tag), (tag_state < -0.5) || (tag_state > 0.5));
    
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
        logError("get_tag_state: incorrect argument type");
        lua_pushstring(L, "get_tag_state: incorrect argument type");
        lua_error(L);
    }
    tag = lua_tonumber(L,1);
    
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
    
    if (changed)
        lua_pushnumber(L, 1);
    else
        lua_pushnumber(L, 0);
    
    return 1;
}

static int L_Get_Life(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("get_life: incorrect argument type");
        lua_pushstring(L, "get_life: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    
    lua_pushnumber(L, player->suit_energy);
    return 1;
}

static int L_Set_Life(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("set_life: incorrect argument type");
        lua_pushstring(L, "set_life: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int energy = lua_tonumber(L,2);
    player_data *player = get_player_data(player_index);
    
    player->suit_energy = energy;
    mark_shield_display_as_dirty();
    return 0;
}

static int L_Get_Oxygen(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("get_oxygen: incorrect argument type");
        lua_pushstring(L, "get_oxygen: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    
    lua_pushnumber(L, player->suit_oxygen);
    return 1;
}

static int L_Set_Oxygen(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("set_oxygen: incorrect argument type");
        lua_pushstring(L, "set_oxygen: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int oxygen = lua_tonumber(L,2);
    player_data *player = get_player_data(player_index);
    
    player->suit_oxygen = oxygen;
    mark_shield_display_as_dirty();
    return 0;
}

static int L_Add_Item(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_tonumber(L,2))
    {	
        logError("add_item: incorrect argument type");
        lua_pushstring(L, "add_item: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int item = lua_tonumber(L,2);
    player_data *player = get_player_data(player_index);
    
    if (!try_and_add_player_item(player_identifier_to_player_index(player->identifier), item))
            ; /* this sucks */
    return 0;
}

static int L_Remove_Item(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("remove_item: incorrect argument type");
        lua_pushstring(L, "remove_item: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int item_type = lua_tonumber(L,2);
    player_data *player = get_player_data(player_index);
    
    struct item_definition *definition= get_item_definition_external(item_type);
	
    if (definition)
    {
        if(player->items[item_type] >= 1)// && definition->item_kind==_ammunition)
            player->items[item_type]--;		/* Decrement your count.. */
        mark_player_inventory_as_dirty(player_index,item_type);
        if (player->items[item_type] == 0 && definition->item_kind==_weapon)
            select_next_best_weapon(player_index);
    }
    return 0;
}

static int L_Select_Weapon(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_tonumber(L,2))
    {	
        logError("select_weapon: incorrect argument type");
        lua_pushstring(L, "select_weapon: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int weapon_index = lua_tonumber(L,2);
    
    ready_weapon(player_index, weapon_index);
    return 0;
}

static int L_Set_Fog_Depth(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("set_fog_depth: incorrect argument type");
        lua_pushstring(L, "set_fog_depth: incorrect argument type");
        lua_error(L);
    }
    float depth = lua_tonumber(L,1);
    OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth = depth;
    return 0;
}

static int L_Set_Fog_Color(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {	
        logError("set_fog_color: incorrect argument type");
        lua_pushstring(L, "set_fog_color: incorrect argument type");
        lua_error(L);
    }
    float r = lua_tonumber(L,1);
    float g = lua_tonumber(L,2);
    float b = lua_tonumber(L,3);
    
    rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
    Color.red = PIN(int(65535*r+0.5),0,65535);
    Color.green = PIN(int(65535*g+0.5),0,65535);
    Color.blue = PIN(int(65535*b+0.5),0,65535);
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

static int L_Set_Underwater_Fog_Depth(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("set_underwater_fog_depth: incorrect argument type");
        lua_pushstring(L, "set_underwater_fog_depth: incorrect argument type");
        lua_error(L);
    }
    float depth = lua_tonumber(L,1);
    OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth = depth;
    return 0;
}

static int L_Set_Underwater_Fog_Color(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {	
        logError("set_underwater_fog_color: incorrect argument type");
        lua_pushstring(L, "set_underwater_fog_color: incorrect argument type");
        lua_error(L);
    }
    float r = lua_tonumber(L,1);
    float g = lua_tonumber(L,2);
    float b = lua_tonumber(L,3);
    
    rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
    Color.red = PIN(int(65535*r+0.5),0,65535);
    Color.green = PIN(int(65535*g+0.5),0,65535);
    Color.blue = PIN(int(65535*b+0.5),0,65535);
    return 0;
}

static int L_Get_Underwater_Fog_Depth(lua_State *L)
{
    lua_pushnumber(L, OGL_GetFogData(OGL_Fog_AboveLiquid)->Depth);
    return 1;
}

static int L_Get_Underwater_Fog_Color(lua_State *L)
{
    rgb_color& Color = OGL_GetFogData(OGL_Fog_AboveLiquid)->Color;
    lua_pushnumber(L, Color.red/65535.0F);
    lua_pushnumber(L, Color.green/65535.0F);
    lua_pushnumber(L, Color.blue/65535.0F);
    return 3;
}

static int L_Teleport_Player(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_tonumber(L,2))
    {	
        logError("teleport_player: incorrect argument type");
        lua_pushstring(L, "teleport_player: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int dest = lua_tonumber(L,2);
    player_data *player = get_player_data(player_index);
    monster_data *monster= get_monster_data(player->monster_index);
    
    SET_PLAYER_TELEPORTING_STATUS(player, true);
    monster->action= _monster_is_teleporting;
    local_player->teleporting_phase= 0;
    local_player->delay_before_teleport= 0;
    
    local_player->teleporting_destination= dest;
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
        logError("new_monster: incorrect argument type");
        lua_pushstring(L, "new_monster: incorrect argument type");
        lua_error(L);
    }
    short monster_type = lua_tonumber(L,1);
    short polygon = lua_tonumber(L,2);
    short facing = 0;
    if (args > 2)
    {
        if (!lua_isnumber(L,3))
        {	
            logError("new_monster: incorrect argument type");
            lua_pushstring(L, "new_monster: incorrect argument type");
            lua_error(L);
        }
        short facing = lua_tonumber(L,3);
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
    // *((world_point2d *)&theDestination)= destination->center;
    //stolen, assuming it works
    theDestination.x = theCenter.x;
    theDestination.y = theCenter.y;
    theDestination.z= destination->floor_height;
    theLocation.p = theDestination;
    theLocation.yaw = 0;
    theLocation.pitch = 0;
    theLocation.flags = 0;
    
    index = new_monster(&theLocation, (short)monster_type);
    if (index == NONE)
        return 0;
    lua_pushnumber(L, index);
    
    const float AngleConvert = 360/float(FULL_CIRCLE);
    
    monster_data *monster = get_monster_data(index);
    object_data *object = get_object_data(monster->object_index);
    object->facing = normalize_angle((double)facing/AngleConvert);
    
    return 1;
}

static int L_Activate_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("activate_monster: incorrect argument type");
        lua_pushstring(L, "activate_monster: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    if (monster_index == -1)
        return 0;
    struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(monster))
    {
        if(MONSTER_IS_ACTIVE(monster))
            activate_monster(monster_index);
    }
    return 0;
}

static int L_Deactivate_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {	
        logError("deactivate_monster: incorrect argument type");
        lua_pushstring(L, "deactivate_monster: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    if (monster_index == -1)
        return 0;
    struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(monster))
    {
        if(MONSTER_IS_ACTIVE(monster))
            deactivate_monster(monster_index);
    }
    return 0;
}

static int L_Damage_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("damage_monster: incorrect argument type");
        lua_pushstring(L, "damage_monster: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int damage_amount = lua_tonumber(L,2);
    int damage_type = -1;
    if (lua_gettop(L) == 3 && lua_isnumber(L,3))
        damage_type = lua_tonumber(L,3);
    if (monster_index == -1)
        return 0;
    
    struct damage_definition theDamage;
    struct monster_data *theMonster;
    if (damage_type != -1)
        theDamage.type = damage_type;
    else
        theDamage.type = _damage_fist;
    theDamage.base = damage_amount;
    theDamage.random = 0;
    theDamage.flags = 0;
    theDamage.scale = 65536;
    
    theMonster= GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(!SLOT_IS_USED(theMonster))
        return 0;
    damage_monster(monster_index, NONE, NONE, &(get_monster_data(monster_index)->sound_location), &theDamage);
    return 0;
}

static int L_Attack_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("attack_monster: incorrect argument type");
        lua_pushstring(L, "attack_monster: incorrect argument type");
        lua_error(L);
    }
    int attacker_index = lua_tonumber(L,1);
    int target_index = lua_tonumber(L,2);
    
    struct monster_data *bully, *poorHelplessVictim;
    if(attacker_index == -1 || target_index ==-1) return 0;	
    bully = GetMemberWithBounds(monsters,attacker_index,MAXIMUM_MONSTERS_PER_MAP);
    poorHelplessVictim = GetMemberWithBounds(monsters,target_index,MAXIMUM_MONSTERS_PER_MAP);
    if(!SLOT_IS_USED(bully) || !SLOT_IS_USED(poorHelplessVictim))
        return 0;
    change_monster_target(attacker_index, target_index);
    
    return 0;
}

static int L_Move_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("move_monster: incorrect argument type");
        lua_pushstring(L, "move_monster: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int polygon = lua_tonumber(L,2);
    
    world_point2d *theEnd;
    struct monster_pathfinding_data thePath;
    struct monster_data *theRealMonster;
    struct monster_definition *theDef;
    struct object_data *theObject;
    
    if(monster_index == -1) return 0;
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
            set_monster_action(monster_index, _monster_is_stationary);
        set_monster_mode(monster_index, _monster_unlocked, NONE);
        return 0;
    }
    advance_monster_path(monster_index);
    
    return 0;
}

static int L_Select_Monster(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {	
        logError("select_monster: incorrect argument type");
        lua_pushstring(L, "select_monster: incorrect argument type");
        lua_error(L);
    }
    int monster_type = lua_tonumber(L,1);
    int polygon = lua_tonumber(L,2);
    
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
    lua_pushnumber(L, -1);
    return 1;
}

static int L_Get_Monster_Polygon(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {
        logError("get_monster_polygon: incorrect argument type");
        lua_pushstring(L, "get_monster_polygon: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    
    struct monster_data *theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(!SLOT_IS_USED(theMonster)) return 0;
    struct object_data *object= get_object_data(theMonster->object_index);
    
    lua_pushnumber(L, object->polygon);
    return 1;
}

static int L_Get_Monster_Immunity(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("get_monster_immunity: incorrect argument type");
        lua_pushstring(L, "get_monster_immunity: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int damage_type = lua_tonumber(L,2);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        lua_pushnumber(L, theDef->immunities & 1<<damage_type);
    }
    else
        lua_pushnumber(L, 0);
    return 1;
}

static int L_Set_Monster_Immunity(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("set_monster_immunity: incorrect argument type");
        lua_pushstring(L, "set_monster_immunity: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int damage_type = lua_tonumber(L,2);
    int immune = lua_tonumber(L,3);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        if(immune == 1)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->immunities = theDef->immunities | 1<<damage_type;
            return 0;
        }
        if(immune == 0)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->immunities = theDef->immunities & ~(1<<damage_type);
            return 0;
        }
        dprintf("Invulnerability value must be 0 or 1\n");
    }
    return 0;
}

static int L_Get_Monster_Weakness(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("get_monster_weakness: incorrect argument type");
        lua_pushstring(L, "get_monster_weakness: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int damage_type = lua_tonumber(L,2);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        lua_pushnumber(L, theDef->weaknesses & 1<<damage_type);
    }
    else
        lua_pushnumber(L, 0);
    return 1;
}

static int L_Set_Monster_Weakness(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("set_monster_weakness: incorrect argument type");
        lua_pushstring(L, "set_monster_weakness: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int damage_type = lua_tonumber(L,2);
    int weak = lua_tonumber(L,3);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        if(weak == 1)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->weaknesses = theDef->weaknesses | 1<<damage_type;
            return 0;
        }
        if(weak == 0)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->weaknesses = theDef->weaknesses & ~(1<<damage_type);
            return 0;
        }
        dprintf("Weakness value must be 0 or 1\n");
    }
    return 0;
}

static int L_Get_Monster_Friend(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("get_monster_friend: incorrect argument type");
        lua_pushstring(L, "get_monster_friend: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int friend_type = lua_tonumber(L,2);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        lua_pushnumber(L, theDef->friends & 1<<friend_type);
    }
    else
        lua_pushnumber(L, 0);
    return 1;
}

static int L_Set_Monster_Friend(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("set_monster_friend: incorrect argument type");
        lua_pushstring(L, "set_monster_friend: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int friend_type = lua_tonumber(L,2);
    int friendly = lua_tonumber(L,3);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        if(friendly == 1)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->friends = theDef->friends | 1<<friend_type;
            return 0;
        }
        if(friendly == 0)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->friends = theDef->friends & ~(1<<friend_type);
            return 0;
        }
        dprintf("Friendliness value must be 0 or 1\n");
    }
    return 0;
}

static int L_Get_Monster_Enemy(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("get_monster_enemy: incorrect argument type");
        lua_pushstring(L, "get_monster_enemy: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int enemy_type = lua_tonumber(L,2);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        lua_pushnumber(L, theDef->enemies & 1<<enemy_type);
    }
    else
        lua_pushnumber(L, 0);
    return 1;
}

static int L_Set_Monster_Enemy(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("set_monster_enemy: incorrect argument type");
        lua_pushstring(L, "set_monster_enemy: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int enemy_type = lua_tonumber(L,2);
    int hostile = lua_tonumber(L,3);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        if(hostile == 1)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->enemies = theDef->enemies | 1<<enemy_type;
            return 0;
        }
        if(hostile == 0)
        {
            theDef = get_monster_definition_external(theMonster->type);
            theDef->enemies = theDef->enemies & ~(1<<enemy_type);
            return 0;
        }
        dprintf("Enemy value must be 0 or 1\n");
    }
    return 0;
}

static int L_Get_Monster_Item(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {
        logError("get_monster_item: incorrect argument type");
        lua_pushstring(L, "get_monster_item: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        lua_pushnumber(L, theDef->carrying_item_type);
    }
    else
        lua_pushnumber(L, 0);
    return 1;
}

static int L_Set_Monster_Item(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("set_monster_item: incorrect argument type");
        lua_pushstring(L, "set_monster_item: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int item_type = lua_tonumber(L,2);
    
    struct monster_data *theMonster;
    struct monster_definition *theDef;
    
    if(monster_index == -1) return 0;
    theMonster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
    if(SLOT_IS_USED(theMonster))
    {
        theDef = get_monster_definition_external(theMonster->type);
        theDef->carrying_item_type = item_type;
    }
    return 0;
}

static int L_Get_Monster_Action(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {
        logError("get_monster_action: incorrect argument type");
        lua_pushstring(L, "get_monster_action: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    
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
        logError("get_monster_mode: incorrect argument type");
        lua_pushstring(L, "get_monster_mode: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    
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
        logError("get_monster_vitality: incorrect argument type");
        lua_pushstring(L, "get_monster_vitality: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    
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
        logError("set_monster_vitality: incorrect argument type");
        lua_pushstring(L, "set_monster_vitality: incorrect argument type");
        lua_error(L);
    }
    int monster_index = lua_tonumber(L,1);
    int vitality = lua_tonumber(L,2);
    
    struct monster_data *monster = GetMemberWithBounds(monsters,monster_index,MAXIMUM_MONSTERS_PER_MAP);
	if(!SLOT_IS_USED(monster)) return 0;
	if (monster)
		monster->vitality = vitality;
        return 0;
}

static int L_Play_Sound(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("play_sound: incorrect argument type");
        lua_pushstring(L, "play_sound: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int sound_index = lua_tonumber(L,2);
    float pitch = lua_tonumber(L,3);
    
    if (local_player_index != player_index)
        return 0;
    
    _play_sound(sound_index, NULL, NONE, _fixed(FIXED_ONE*pitch));
    return 0;
}

static int L_Get_Player_Polygon(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {
        logError("get_player_polygon: incorrect argument type");
        lua_pushstring(L, "get_player_polygon: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    lua_pushnumber(L, player->supporting_polygon_index);
    return 1;
}

static int L_Player_Is_Dead(lua_State *L)
{
    if (!lua_isnumber(L,1))
    {
        logError("player_is_dead: incorrect argument type");
        lua_pushstring(L, "player_is_dead: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    player_data *player = get_player_data(player_index);
    
    if (PLAYER_IS_DEAD(player) || PLAYER_IS_TOTALLY_DEAD(player))
        lua_pushboolean(L, true);
    else
        lua_pushboolean(L, false);
    return 1;
}

static int L_Player_Control(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2) || !lua_isnumber(L,3))
    {
        logError("play_control: incorrect argument type");
        lua_pushstring(L, "play_control: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    int move_type = lua_tonumber(L,2);
    int value = lua_tonumber(L,3);
    player_data *player = get_player_data(player_index);
    
    // uses the Pfhortran action queue
    if (sPfhortranActionQueues == NULL)
    {
        sPfhortranActionQueues = new ActionQueues(MAXIMUM_NUMBER_OF_PLAYERS, ACTION_QUEUE_BUFFER_DIAMETER, true);
    }
    
    // Put the enqueuing of the action flags in one place in the code,
    // so it will be easier to change if necessary
    uint32 action_flags = 0;
    bool DoAction = false;
    
    switch(move_type)
    {
        case 0:
            action_flags = _moving_forward;
            DoAction = true;
        break;
        
        case 1:
            action_flags = _moving_backward;
            DoAction = true;
        break;
        
        case 2:
            action_flags = _sidestepping_left;
            DoAction = true;
        break;
        
        case 3:
            action_flags = _sidestepping_right;
            DoAction = true;
        break;
        
        case 4:
            action_flags = _turning_left;
            DoAction = true;
        break;
        
        case 5:
            action_flags = _turning_right;
            DoAction = true;
        break;
        
        case 6:
            action_flags = _looking_up;
            DoAction = true;
        break;
        
        case 7:
            action_flags = _looking_down;
            DoAction = true;
        break;
        
        case 8:
            action_flags = _action_trigger_state;
            DoAction = true;
        break;
        
        case 9:
            action_flags = _left_trigger_state;
            DoAction = true;
        break;
        
        case 10:
            action_flags = _right_trigger_state;
            DoAction = true;
        break;
        
        case 11: // start using pfhortran_action_queue
            SET_PLAYER_IS_PFHORTRAN_CONTROLLED_STATUS(player, true);
        break;
        
        case 12: // stop using pfhortran_action_queue
            SET_PLAYER_IS_PFHORTRAN_CONTROLLED_STATUS(player, false);
        break;
        
        case 13: // reset pfhortran_action_queue
            GetPfhortranActionQueues()->reset();
        break;
        
        default:
        break;
    }
    
    if (DoAction)
    {
        GetPfhortranActionQueues()->enqueueActionFlags(player_index, &action_flags, value);
        if (PLAYER_IS_PFHORTRAN_CONTROLLED(player))
            increment_heartbeat_count(value);
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
        logError("add_path_point: too few arguments");
        lua_pushstring(L, "add_path_point: too few arguments");
        lua_error(L);
    }
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("add_path_point: incorrect argument type");
        lua_pushstring(L, "add_path_point: incorrect argument type");
        lua_error(L);
    }
    int camera_index = lua_tonumber(L,1);
    if (camera_index > number_of_cameras-1)
    {
        logError("add_path_point: bad camera index");
        lua_pushstring(L, "add_path_point: bad camera index");
        lua_error(L);
    }
    int polygon = lua_tonumber(L,2);
    world_point3d point = {lua_tonumber(L,3)*WORLD_ONE, lua_tonumber(L,4)*WORLD_ONE, lua_tonumber(L,5)*WORLD_ONE};
    long time = lua_tonumber(L,6);

    timed_point tp;
    tp.polygon = polygon;
    tp.point = point;
    tp.delta_time = time;

    lua_cameras[camera_index].path.path_points.push_back(tp);
    return 0;
}

static int L_Add_Path_Angle(lua_State *L)
{
    if (lua_gettop(L) < 4)
    {
        logError("add_path_angle: too few arguments");
        lua_pushstring(L, "add_path_angle: too few arguments");
        lua_error(L);
    }
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("add_path_angle: incorrect argument type");
        lua_pushstring(L, "add_path_angle: incorrect argument type");
        lua_error(L);
    }
    int camera_index = lua_tonumber(L,1);
    if (camera_index > number_of_cameras-1)
    {
        logError("add_path_angle: bad camera index");
        lua_pushstring(L, "add_path_angle: bad camera index");
        lua_error(L);
    }
    int yaw = lua_tonumber(L,2);
    int pitch = lua_tonumber(L,3);
    long time = lua_tonumber(L,4);
    int angle_index = lua_cameras[camera_index].path.path_angles.size();
    
    const float AngleConvert = 360/float(FULL_CIRCLE);
    
    timed_angle ta;

    ta.yaw = yaw/AngleConvert;
    ta.pitch = pitch/AngleConvert;
    ta.delta_time = time;
    lua_cameras[camera_index].path.path_angles.push_back(ta);

    return 0;
}

static int L_Activate_Camera(lua_State *L)
{
    if (!lua_isnumber(L,1) || !lua_isnumber(L,2))
    {
        logError("activate_camera: incorrect argument type");
        lua_pushstring(L, "activate_camera: incorrect argument type");
        lua_error(L);
    }
    int player_index = lua_tonumber(L,1);
    if (local_player_index != player_index)
        return 0;
    int camera_index = lua_tonumber(L,2);
    if (camera_index > number_of_cameras-1)
    {
        logError("activate_camera: bad camera index");
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
        logError("deactivate_camera: incorrect argument type");
        lua_pushstring(L, "deactivate_camera: incorrect argument type");
        lua_error(L);
    }
    int camera_index = lua_tonumber(L,1);
    if (camera_index > number_of_cameras-1)
    {
        logError("deactivate_camera: bad camera index");
        lua_pushstring(L, "deactivate_camera: bad camera index");
        lua_error(L);
    }
    lua_cameras[camera_index].time_elapsed = 0;
    lua_cameras[camera_index].player_active = -1;
    lua_cameras[camera_index].path.last_point_time = 0;
    lua_cameras[camera_index].path.last_angle_time = 0;
    return 0;
}

void RegisterLuaFunctions()
{
    lua_register(state, "number_of_players", L_Number_of_Players);
    lua_register(state, "screen_print", L_Screen_Print);
    lua_register(state, "display_text", L_Display_Text);
    lua_register(state, "inflict_damage", L_Inflict_Damage);
    lua_register(state, "enable_player", L_Enable_Player);
    lua_register(state, "disable_player", L_Disable_Player);
    lua_register(state, "kill_script", L_Kill_Script);
    lua_register(state, "get_tag_state", L_Get_Tag_State);
    lua_register(state, "set_tag_state", L_Set_Tag_State);
    lua_register(state, "get_life", L_Get_Life);
    lua_register(state, "set_life", L_Set_Life);
    lua_register(state, "get_oxygen", L_Get_Oxygen);
    lua_register(state, "set_oxygen", L_Set_Oxygen);
    lua_register(state, "add_item", L_Add_Item);
    lua_register(state, "remove_item", L_Remove_Item);
    lua_register(state, "select_weapon", L_Select_Weapon);
    lua_register(state, "set_fog_depth", L_Set_Fog_Depth);
    lua_register(state, "set_fog_color", L_Set_Fog_Color);
    lua_register(state, "get_fog_depth", L_Get_Fog_Depth);
    lua_register(state, "get_fog_color", L_Get_Fog_Color);
    lua_register(state, "set_underwater_fog_depth", L_Set_Underwater_Fog_Depth);
    lua_register(state, "set_underwater_fog_color", L_Set_Underwater_Fog_Color);
    lua_register(state, "get_underwater_fog_depth", L_Get_Underwater_Fog_Depth);
    lua_register(state, "get_underwater_fog_color", L_Get_Underwater_Fog_Color);
    lua_register(state, "teleport_player", L_Teleport_Player);
    lua_register(state, "new_monster", L_New_Monster);
    lua_register(state, "activate_monster", L_Activate_Monster);
    lua_register(state, "deactivate_monster", L_Deactivate_Monster);
    lua_register(state, "damage_monster", L_Damage_Monster);
    lua_register(state, "attack_monster", L_Attack_Monster);
    lua_register(state, "move_monster", L_Move_Monster);
    lua_register(state, "select_monster", L_Select_Monster);
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
    lua_register(state, "play_sound", L_Play_Sound);
    lua_register(state, "get_player_polygon", L_Get_Player_Polygon);
    lua_register(state, "player_is_dead", L_Player_Is_Dead);
    lua_register(state, "player_control", L_Player_Control);
    lua_register(state, "create_camera", L_Create_Camera);
    lua_register(state, "add_path_point", L_Add_Path_Point);
    lua_register(state, "add_path_angle", L_Add_Path_Angle);
    lua_register(state, "activate_camera", L_Activate_Camera);
    lua_register(state, "deactivate_camera", L_Deactivate_Camera);
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
    for (int i=0; i<190; i++)
    {
        lua_pushnumber(state, constant_list[i].value);
        lua_setglobal(state, constant_list[i].name);
    }
}

bool LoadLuaScript(const char *buffer, size_t len)
{
    int status;
    state = lua_open();
    
    OpenStdLibs(state);
    status = luaL_loadbuffer(state, buffer, len, "level_script");
    if (status == LUA_ERRRUN)
        logWarning("Lua loading failed: error running script.");
    if (status == LUA_ERRFILE)
        logWarning("Lua loading failed: error loading file.");
    if (status == LUA_ERRSYNTAX)
        logWarning("Lua loading failed: syntax error.");
    if (status == LUA_ERRMEM)
        logWarning("Lua loading failed: error allocating memory.");
    if (status == LUA_ERRERR)
        logWarning("Lua loading failed: unknown error.");
    
    RegisterLuaFunctions();
    DeclareLuaConstants();
    
    lua_loaded = status==0;
    return lua_loaded;
}

bool RunLuaScript()
{
    if (!lua_loaded)
        return false;
    int result = lua_pcall(state, 0, LUA_MULTRET, 0);
    if (result==0)
        lua_running = true;
    return result==0;
}

void CloseLuaScript()
{
    if (lua_loaded)
        lua_close(state);
    lua_loaded = false;
    lua_running = false;
    lua_cameras.resize(0);
    number_of_cameras = 0;
}

bool UseLuaCameras()
{
    //const float AngleConvert = 360/float(FULL_CIRCLE);	//tiennou: unused !
    
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
                if (angle_index != -1 && angle_index != lua_cameras[i].path.path_angles.size()-1)
                {
                    world_view->yaw = normalize_angle(FindLinearValue(lua_cameras[i].path.path_angles[angle_index].yaw, lua_cameras[i].path.path_angles[angle_index+1].yaw, lua_cameras[i].path.path_angles[angle_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time));
                    world_view->pitch = normalize_angle(FindLinearValue(lua_cameras[i].path.path_angles[angle_index].pitch, lua_cameras[i].path.path_angles[angle_index+1].pitch, lua_cameras[i].path.path_angles[angle_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time));
                }
                else if (angle_index == lua_cameras[i].path.path_angles.size()-1)
                {
                    world_view->yaw = normalize_angle(lua_cameras[i].path.path_angles[angle_index].yaw);
                    world_view->pitch = normalize_angle(lua_cameras[i].path.path_angles[angle_index].pitch);
                }
                if (point_index != -1 && point_index != lua_cameras[i].path.path_points.size()-1)
                {
                    world_point3d oldPoint = lua_cameras[i].path.path_points[point_index].point;
                    world_view->origin = FindLinearValue(lua_cameras[i].path.path_points[point_index].point, lua_cameras[i].path.path_points[point_index+1].point, lua_cameras[i].path.path_points[point_index].delta_time, lua_cameras[i].time_elapsed - lua_cameras[i].path.last_point_time);
                    world_point3d newPoint = world_view->origin;
                    short polygon = lua_cameras[i].path.path_points[point_index].polygon;
                    ShootForTargetPoint(true, oldPoint, newPoint, polygon);
                    world_view->origin_polygon_index = polygon;
                }
                else if (point_index == lua_cameras[i].path.path_points.size()-1)
                {
                    world_view->origin = lua_cameras[i].path.path_points[point_index].point;
                    world_view->origin_polygon_index = lua_cameras[i].path.path_points[point_index].polygon;
                }
                
                lua_cameras[i].time_elapsed++;
                
                if (lua_cameras[i].time_elapsed - lua_cameras[i].path.last_point_time >= lua_cameras[i].path.path_points[point_index].delta_time)
                {
                    lua_cameras[i].path.current_point_index++;
                    lua_cameras[i].path.last_point_time = lua_cameras[i].time_elapsed;
                    if (lua_cameras[i].path.current_point_index >= lua_cameras[i].path.path_points.size())
                        lua_cameras[i].path.current_point_index = -1;
                }
                if (lua_cameras[i].time_elapsed - lua_cameras[i].path.last_angle_time >= lua_cameras[i].path.path_angles[angle_index].delta_time)
                {
                    lua_cameras[i].path.current_angle_index++;
                    lua_cameras[i].path.last_angle_time = lua_cameras[i].time_elapsed;
                    if (lua_cameras[i].path.current_angle_index >= lua_cameras[i].path.path_angles.size())
                        lua_cameras[i].path.current_angle_index = -1;
                }
            }
        }
    }
    return using_lua_cameras;
}
#endif