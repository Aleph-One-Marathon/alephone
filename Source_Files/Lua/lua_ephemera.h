#ifndef __LUA_EPHEMERA_H
#define __LUA_EPHEMERA_H

/*
LUA_EPHEMERA.H

	Copyright (C) 2020 by Gregory Smith
 
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

	Implements Lua ephemera classes
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "lua_templates.h"

extern char Lua_Ephemera_Name[]; // "ephemera"
typedef L_Class<Lua_Ephemera_Name> Lua_Ephemera;

extern char Lua_Ephemeras_Name[]; // "Ephemera"
typedef L_Container<Lua_Ephemeras_Name, Lua_Ephemera> Lua_Ephemeras;

int Lua_Ephemera_register(lua_State* L);

#endif

#endif
