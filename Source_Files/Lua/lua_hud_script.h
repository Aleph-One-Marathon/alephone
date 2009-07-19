
#ifndef LUA_HUD_SCRIPT_H
#define LUA_HUD_SCRIPT_H

#include "cseries.h"

void L_Call_HUDInit();
void L_Call_HUDCleanup();
void L_Call_HUDDraw();
void L_Call_HUDResize();

bool LoadLuaHUDScript(const char *buffer, size_t len);
bool RunLuaHUDScript();
void LoadHUDLua();
void CloseLuaHUDScript();

#endif
