
#ifndef LUA_SCRIPT_H
#define LUA_SCRIPT_H

#include "cseries.h"
#include "world.h"
#include "ActionQueues.h"
#include "shape_descriptors.h"

void L_Error(const char *message);
void L_Call_Init(bool fRestoringSaved);
void L_Call_Cleanup();
void L_Call_Idle();
void L_Call_PostIdle();
void L_Call_Start_Refuel(short type, short player_index, short panel_side_index);
void L_Call_End_Refuel(short type, short player_index, short panel_side_index);
void L_Call_Tag_Switch(short tag, short player_index);
void L_Call_Light_Switch(short light, short player_index);
void L_Call_Platform_Switch(short platform, short player_index);
void L_Call_Terminal_Enter(short terminal_id, short player_index);
void L_Call_Terminal_Exit(short terminal_id, short player_index);
void L_Call_Pattern_Buffer(short side_index, short player_index);
void L_Call_Got_Item(short type, short player_index);
void L_Call_Light_Activated(short index);
void L_Call_Platform_Activated(short index);
void L_Call_Player_Revived(short player_index);
void L_Call_Player_Killed(short player_index, short aggressor_player_index, short action, short projectile_index);
void L_Call_Monster_Killed(short monster_index, short aggressor_player_index, short projectile_index);
void L_Call_Player_Damaged(short player_index, short aggressor_player_index, short aggressor_monster_index, int16 damage_type, short damage_amount, short projectile_index);
void L_Call_Projectile_Detonated(short type, short owner_index, short polygon, world_point3d location);
void L_Call_Item_Created(short item_index);

void L_Invalidate_Monster(short monster_index);
void L_Invalidate_Projectile(short projectile_index);
void L_Invalidate_Object(short object_index);

bool LoadLuaScript(const char *buffer, size_t len);
bool RunLuaScript();
void CloseLuaScript();

void ExecuteLuaString(const std::string&);
void LoadSoloLua();

void ToggleLuaMute();
void ResetLuaMute();

bool UseLuaCameras();

ActionQueues* GetLuaActionQueues();

void MarkLuaCollections(bool active);

void LuaTexturePaletteClear();
int LuaTexturePaletteSize();
shape_descriptor LuaTexturePaletteTexture(size_t);
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
    std::vector<timed_point> path_points;
    short current_point_index;
    long last_point_time;
    std::vector<timed_angle> path_angles;
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
