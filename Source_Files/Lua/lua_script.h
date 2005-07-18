
#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include "world.h"

void L_Call_Init();
void L_Call_Cleanup();
void L_Call_Idle();
void L_Call_Start_Refuel(short type, short player_index, short panel_side_index);
void L_Call_End_Refuel(short type, short player_index, short panel_side_index);
void L_Call_Tag_Switch(short tag, short player_index);
void L_Call_Light_Switch(short light, short player_index);
void L_Call_Platform_Switch(short platform, short player_index);
void L_Call_Terminal_Enter(short terminal_id, short player_index);
void L_Call_Terminal_Exit(short terminal_id, short player_index);
void L_Call_Pattern_Buffer(short buffer_id, short player_index);
void L_Call_Got_Item(short type, short player_index);
void L_Call_Light_Activated(short index);
void L_Call_Platform_Activated(short index);
void L_Call_Player_Revived(short player_index);
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index);
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index);
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location);
void L_Call_Item_Created(short item_index);

bool LoadLuaScript(const char *buffer, size_t len);
bool RunLuaScript();
void CloseLuaScript();

bool UseLuaCameras();


// camera data structures
struct timed_point
{
    int polygon;
    world_point3d point;
    long delta_time; //for REALLY long cutscenes
};

struct timed_angle
{
    short yaw, pitch;
    long delta_time;
};

struct lua_path
{
    short index;
    vector<timed_point> path_points;
    short current_point_index;
    long last_point_time;
    vector<timed_angle> path_angles;
    short current_angle_index;
    long last_angle_time;
};

struct lua_camera //an expanded version of script_camera; uses Lua's path scheme
{
    short index;
    lua_path path;
    long time_elapsed;
    int player_active;
};

#endif
