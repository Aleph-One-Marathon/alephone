#ifdef HAVE_LUA
#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include "world.h"

void L_Call_Init();
void L_Call_Idle();
void L_Call_Tag_Switch(short tag);
void L_Call_Light_Switch(short light);
void L_Call_Platform_Switch(short platform);
void L_Call_Terminal_Enter(short terminal_id);
void L_Call_Terminal_Exit(short terminal_id);
void L_Call_Pattern_Buffer(short buffer_id);
void L_Call_Got_Item(short type);
void L_Call_Light_Activated(short index);
void L_Call_Platform_Activated(short index);

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
#endif
