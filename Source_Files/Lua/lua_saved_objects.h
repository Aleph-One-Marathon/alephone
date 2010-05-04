/*
LUA_SAVED_OBJECTS.H

	Copyright (C) 2010 by Gregory Smith
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Implements Lua map objects classes
*/

#ifndef __LUA_SAVED_OBJECTS_H
#define __LUA_SAVED_OBJECTS_H

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "map.h"
#include "lua_templates.h"

extern char Lua_Goal_Name[]; // "goal"
typedef L_Class<Lua_Goal_Name> Lua_Goal;

extern char Lua_Goals_Name[]; // "Goals"
typedef L_Container<Lua_Goals_Name, Lua_Goal> Lua_Goals;

int Lua_Saved_Objects_register(lua_State* L);

#endif

#endif
