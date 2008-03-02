#ifndef __LUA_MAP_H
#define __LUA_MAP_H

/*
LUA_MAP.H

	Copyright (C) 2008 by Gregory Smith
 
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

	Implements Lua map classes
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "map.h"
#include "lightsource.h"

#include "lua_templates.h"

extern char Lua_DamageType_Name[]; // "damage_type"
typedef L_Enum<Lua_DamageType_Name> Lua_DamageType;

extern char Lua_DamageTypes_Name[]; // "DamageTypes"
typedef L_EnumContainer<Lua_DamageTypes_Name, Lua_DamageType> Lua_DamageTypes;

extern char Lua_Polygon_Ceiling_Name[]; // "polygon_ceiling"
typedef L_Class<Lua_Polygon_Ceiling_Name> Lua_Polygon_Ceiling;

extern char Lua_Polygon_Floor_Name[]; // "polygon_floor"
typedef L_Class<Lua_Polygon_Floor_Name> Lua_Polygon_Floor;

extern char Lua_Platform_Name[]; // "platform"
typedef L_Class<Lua_Platform_Name> Lua_Platform;

extern char Lua_Platforms_Name[]; // "Platforms";
typedef L_Container<Lua_Platforms_Name, Lua_Platform> Lua_Platforms;

extern char Lua_Polygon_Name[]; // "polygon"
typedef L_Class<Lua_Polygon_Name> Lua_Polygon;

extern char Lua_Polygons_Name[]; // "Polygons"
typedef L_Container<Lua_Polygons_Name, Lua_Polygon> Lua_Polygons;

extern char Lua_Light_Name[]; // "light"
typedef L_Class<Lua_Light_Name> Lua_Light;

extern char Lua_Lights_Name[]; // "Lights"
typedef L_Container<Lua_Lights_Name, Lua_Light> Lua_Lights;

extern char Lua_Tag_Name[]; // "tag"
typedef L_Class<Lua_Tag_Name> Lua_Tag;

extern char Lua_Tags_Name[]; // "Tags"
typedef L_Container<Lua_Tags_Name, Lua_Tag> Lua_Tags;

int Lua_Map_register (lua_State *L);

#endif

#endif
