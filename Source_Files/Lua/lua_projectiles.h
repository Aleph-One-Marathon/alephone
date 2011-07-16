#ifndef __LUA_PROJECTILES_H
#define __LUA_PROJECTILES_H
/*
LUA_PROJECTILES.H

	Copyright (C) 2008 by Gregory Smith
 
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

	Implements the Lua projectile class
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

extern char Lua_Projectile_Name[]; // "projectile"
typedef L_Class<Lua_Projectile_Name> Lua_Projectile;

extern char Lua_Projectiles_Name[]; // "Projectiles"
typedef L_Container<Lua_Projectiles_Name, Lua_Projectile> Lua_Projectiles;

extern char Lua_ProjectileType_Name[]; // "projectile_type"
typedef L_Enum<Lua_ProjectileType_Name> Lua_ProjectileType;

extern char Lua_ProjectileTypes_Name[]; // "ProjectileTypes"
typedef L_EnumContainer<Lua_ProjectileTypes_Name, Lua_ProjectileType> Lua_ProjectileTypes;

int Lua_Projectiles_register(lua_State *L);

#endif

#endif
