#ifndef __LUA_OBJECTS_H
#define __LUA_OBJECTS_H

/*
LUA_OBJECTS.H

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

	Implements Lua map objects classes
*/

#include "cseries.h"

#ifdef HAVE_LUA
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "items.h"
#include "map.h"
#include "lua_templates.h"

extern char Lua_Item_Name[]; // "item"
typedef L_Class<Lua_Item_Name> Lua_Item;

extern char Lua_Items_Name[]; // "Items"
typedef L_Container<Lua_Items_Name, Lua_Item> Lua_Items;

extern char Lua_ItemType_Name[]; // "item_type"
typedef L_Enum<Lua_ItemType_Name> Lua_ItemType;

extern char Lua_ItemTypes_Name[]; // "ItemTypes"
typedef L_EnumContainer<Lua_ItemTypes_Name, Lua_ItemType> Lua_ItemTypes;

extern char Lua_Scenery_Name[]; // "scenery"
typedef L_Class<Lua_Scenery_Name> Lua_Scenery;

extern char Lua_Sceneries_Name[]; // "Scenery"
typedef L_Container<Lua_Sceneries_Name, Lua_Scenery> Lua_Sceneries;

int Lua_Objects_register(lua_State *L);

#endif

#endif
