#ifndef __LUA_MAP_H
#define __LUA_MAP_H

/*
LUA_MAP.H

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

extern char Lua_AmbientSound_Name[]; // "ambient sound"
typedef L_Enum<Lua_AmbientSound_Name> Lua_AmbientSound;

extern char Lua_AmbientSounds_Name[]; // "AmbientSounds"
typedef L_EnumContainer<Lua_AmbientSounds_Name, Lua_AmbientSound> Lua_AmbientSounds;

extern char Lua_Collection_Name[]; // "collection"
typedef L_Enum<Lua_Collection_Name> Lua_Collection;

extern char Lua_Collections_Name[]; // "Collections"
typedef L_EnumContainer<Lua_Collections_Name, Lua_Collection> Lua_Collections;

extern char Lua_ControlPanelClass_Name[]; // "control_panel_class"
typedef L_Enum<Lua_ControlPanelClass_Name> Lua_ControlPanelClass;

extern char Lua_ControlPanelClasses_Name[]; // "ControlPanelClasses"
typedef L_EnumContainer<Lua_ControlPanelClasses_Name, Lua_ControlPanelClass> Lua_ControlPanelClasses;

extern char Lua_ControlPanelType_Name[]; // "control_panel_type"
typedef L_Enum<Lua_ControlPanelType_Name> Lua_ControlPanelType;

extern char Lua_ControlPanelTypes_Name[]; // "ControlPanelTypes"
typedef L_EnumContainer<Lua_ControlPanelTypes_Name, Lua_ControlPanelType> Lua_ControlPanelTypes;

extern char Lua_DamageType_Name[]; // "damage_type"
typedef L_Enum<Lua_DamageType_Name> Lua_DamageType;

extern char Lua_DamageTypes_Name[]; // "DamageTypes"
typedef L_EnumContainer<Lua_DamageTypes_Name, Lua_DamageType> Lua_DamageTypes;

extern char Lua_Line_Name[]; // "line"
typedef L_Class<Lua_Line_Name> Lua_Line;

extern char Lua_Lines_Name[]; // "Lines"
typedef L_Container<Lua_Lines_Name, Lua_Line> Lua_Lines;

extern char Lua_Polygon_Ceiling_Name[]; // "polygon_ceiling"
typedef L_Class<Lua_Polygon_Ceiling_Name> Lua_Polygon_Ceiling;

extern char Lua_Polygon_Floor_Name[]; // "polygon_floor"
typedef L_Class<Lua_Polygon_Floor_Name> Lua_Polygon_Floor;

extern char Lua_Platform_Name[]; // "platform"
typedef L_Class<Lua_Platform_Name> Lua_Platform;

extern char Lua_Platforms_Name[]; // "Platforms";
typedef L_Container<Lua_Platforms_Name, Lua_Platform> Lua_Platforms;

extern char Lua_PlatformType_Name[]; // "platform_type"
typedef L_Enum<Lua_PlatformType_Name> Lua_PlatformType;

extern char Lua_PlatformTypes_Name[]; // "PlatformTypes"
typedef L_EnumContainer<Lua_PlatformTypes_Name, Lua_PlatformType> Lua_PlatformTypes;

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

extern char Lua_Terminal_Name[]; // "terminal"
typedef L_Class<Lua_Terminal_Name> Lua_Terminal;

extern char Lua_Terminals_Name[]; // "Terminals"
typedef L_Container<Lua_Terminals_Name, Lua_Terminal> Lua_Terminals;

extern char Lua_TransferMode_Name[]; // "transfer_mode"
typedef L_Enum<Lua_TransferMode_Name> Lua_TransferMode;

extern char Lua_TransferModes_Name[]; // "TransferModes"
typedef L_EnumContainer<Lua_TransferModes_Name, Lua_TransferMode> Lua_TransferModes;

extern char Lua_Side_Name[]; // "side"
typedef L_Class<Lua_Side_Name> Lua_Side;

extern char Lua_Sides_Name[]; // "Sides"
typedef L_Container<Lua_Sides_Name, Lua_Side> Lua_Sides;

extern char Lua_SideType_Name[]; // "side_type"
typedef L_Enum<Lua_SideType_Name> Lua_SideType;

extern char Lua_SideTypes_Name[]; // "SideTypes"
typedef L_EnumContainer<Lua_SideTypes_Name, Lua_SideType> Lua_SideTypes;

extern char Lua_Media_Name[]; // "media"
typedef L_Class<Lua_Media_Name> Lua_Media;

extern char Lua_Medias_Name[]; // "Media"
typedef L_Container<Lua_Medias_Name, Lua_Media> Lua_Medias;

int Lua_Map_register (lua_State *L);

#endif

#endif
