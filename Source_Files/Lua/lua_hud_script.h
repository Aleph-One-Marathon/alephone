
#ifndef LUA_HUD_SCRIPT_H
#define LUA_HUD_SCRIPT_H
/*
LUA_HUD_SCRIPT.H
 
    Copyright (C) 2009 by Jeremiah Morris and the Aleph One developers

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

    Implements Lua HUD state and trigger callbacks
*/

#include "cseries.h"
#include <string>

void L_Call_HUDInit();
void L_Call_HUDCleanup();
void L_Call_HUDDraw();
void L_Call_HUDResize();

bool LoadLuaHUDScript(const char *buffer, size_t len);
bool RunLuaHUDScript();
bool LuaHUDRunning();
void LoadHUDLua();
void CloseLuaHUDScript();

void SetLuaHUDScriptPath(const std::string& path);
std::string GetLuaHUDScriptPath();

void MarkLuaHUDCollections(bool loading);


#endif
