/*
LUA_EPHEMERA.CPP

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

	Implements Lua ephemera classes
*/

#include <functional>

#include "ephemera.h"
#include "lua_ephemera.h"
#include "lua_map.h"
#include "preferences.h"
#include "render.h"

static uint16_t set_shape(uint16_t descriptor, uint16_t shape)
{
	descriptor &= ~(MAXIMUM_SHAPES_PER_COLLECTION - 1);
	descriptor |= shape;

	return descriptor;
}

static uint16_t set_clut(uint16_t descriptor, uint16_t clut)
{
	descriptor &= ~((MAXIMUM_CLUTS_PER_COLLECTION - 1) << (DESCRIPTOR_SHAPE_BITS + DESCRIPTOR_COLLECTION_BITS));
	descriptor |= clut << (DESCRIPTOR_SHAPE_BITS + DESCRIPTOR_COLLECTION_BITS);
	return descriptor;
}

static uint16_t set_collection(uint16_t descriptor, uint16_t collection)
{
	descriptor &= ~((MAXIMUM_COLLECTIONS - 1) << DESCRIPTOR_SHAPE_BITS);
	descriptor |= collection << DESCRIPTOR_SHAPE_BITS;
	return descriptor;
}

const float AngleConvert = 360/float(FULL_CIRCLE);

static int Lua_Ephemera_Delete(lua_State* L)
{
	remove_ephemera(Lua_Ephemera::Index(L, 1));
	return 0;
}

static int Lua_Ephemera_Get_Clut_Index(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, object->shape >> (DESCRIPTOR_SHAPE_BITS + DESCRIPTOR_COLLECTION_BITS));
	return 1;
}

static int Lua_Ephemera_Get_Collection(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	Lua_Collection::Push(L, GET_DESCRIPTOR_COLLECTION(object->shape));
	return 1;
}

static int Lua_Ephemera_Get_End_When_Animation_Loops(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushboolean(L, object->flags & _ephemera_end_when_animation_loops);
	return 1;
}

static int Lua_Ephemera_Get_Enlarged(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushboolean(L, GET_OBJECT_SCALE_FLAGS(object) & _object_is_enlarged);
	return 1;
}

static int Lua_Ephemera_Get_Facing(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, (double) object->facing * AngleConvert);
	return 1;
}

static int Lua_Ephemera_Get_Polygon(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	Lua_Polygon::Push(L, object->polygon);
	return 1;
}

static int Lua_Ephemera_Get_Rendered(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushboolean(L, TEST_RENDER_FLAG(object->polygon, _polygon_is_visible));
	return 1;
}

static int Lua_Ephemera_Get_Shape_Index(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, GET_DESCRIPTOR_SHAPE(object->shape));
	return 1;
}

static int Lua_Ephemera_Get_Tiny(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushboolean(L, GET_OBJECT_SCALE_FLAGS(object) & _object_is_tiny);
	return 1;
}

static int Lua_Ephemera_Get_Valid(lua_State* L)
{
	lua_pushboolean(L, Lua_Ephemera::Valid(Lua_Ephemera::Index(L, 1)));
	return 1;
}

static int Lua_Ephemera_Get_X(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(object->location.x) / WORLD_ONE);
	return 1;
}

static int Lua_Ephemera_Get_Y(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(object->location.y) / WORLD_ONE);
	return 1;
}

static int Lua_Ephemera_Get_Z(lua_State* L)
{
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	lua_pushnumber(L, static_cast<double>(object->location.z) / WORLD_ONE);
	return 1;
}

static int Lua_Ephemera_Position(lua_State* L)
{
	if (!lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
		return luaL_error(L, "position: incorrect argument type");

	short polygon_index = 0;
	if (lua_isnumber(L, 5))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 5));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "position: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 5))
	{
		polygon_index = Lua_Polygon::Index(L, 5);
	}
	else
		return luaL_error(L, "position: incorrect argument type");

	auto object_index = Lua_Ephemera::Index(L, 1);
	auto object = get_ephemera_data(object_index);
	object->location.x = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	object->location.y = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);
	object->location.z = static_cast<int>(lua_tonumber(L, 4) * WORLD_ONE);

	if (polygon_index != object->polygon)
	{
		remove_ephemera_from_polygon(object_index);
		add_ephemera_to_polygon(object_index, polygon_index);
	}

	return 0;
}

const luaL_Reg Lua_Ephemera_Get[] = {
	{"clut_index", Lua_Ephemera_Get_Clut_Index},
	{"collection", Lua_Ephemera_Get_Collection},
	{"delete", L_TableFunction<Lua_Ephemera_Delete>},
	{"end_when_animation_loops", Lua_Ephemera_Get_End_When_Animation_Loops},
	{"enlarged", Lua_Ephemera_Get_Enlarged},
	{"facing", Lua_Ephemera_Get_Facing},
	{"position", L_TableFunction<Lua_Ephemera_Position>},
	{"polygon", Lua_Ephemera_Get_Polygon},
	{"rendered", Lua_Ephemera_Get_Rendered},
	{"shape_index", Lua_Ephemera_Get_Shape_Index},
	{"tiny", Lua_Ephemera_Get_Tiny},
	{"valid", Lua_Ephemera_Get_Valid},
	{"x", Lua_Ephemera_Get_X},
	{"y", Lua_Ephemera_Get_Y},
	{"z", Lua_Ephemera_Get_Z},
	{0, 0}
};

static int Lua_Ephemera_Set_Clut_Index(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "clut_index: incorrect argument type");
	
	auto ephemera_index = Lua_Ephemera::Index(L, 1);
	auto object = get_ephemera_data(ephemera_index);
	uint16_t clut_index = lua_tonumber(L, 2);

	set_ephemera_shape(ephemera_index, set_clut(object->shape, clut_index));

	return 0;
}

static int Lua_Ephemera_Set_Collection(lua_State* L)
{
	auto ephemera_index = Lua_Ephemera::Index(L, 1);
	auto object = get_ephemera_data(ephemera_index);
	int16_t collection = Lua_Collection::ToIndex(L, 2);

	set_ephemera_shape(ephemera_index, set_collection(object->shape, collection));

	return 0;
}

static int Lua_Ephemera_Set_End_When_Animation_Loops(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "end_when_animation_loops: incorrect argument type");

	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));

	if (lua_toboolean(L, 2)) {
		object->flags |= _ephemera_end_when_animation_loops;
	} else {
		object->flags &= ~_ephemera_end_when_animation_loops;
	}

	return 0;
}

static int Lua_Ephemera_Set_Enlarged(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "enlarged: incorrect argument type");

	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));

	if (lua_toboolean(L, 2)) {
		object->flags |= _object_is_enlarged;
	} else {
		object->flags &= ~_object_is_enlarged;
	}

	return 0;
}

static int Lua_Ephemera_Set_Facing(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "facing: incorrect argument type");

	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	object->facing = static_cast<int>(lua_tonumber(L, 2) / AngleConvert);
	return 0;
}

static int Lua_Ephemera_Set_Shape_Index(lua_State* L)
{
	if (!lua_isnumber(L, 2))
		return luaL_error(L, "shape_index: incorrect argument type");

	auto ephemera_index = Lua_Ephemera::Index(L, 1);
	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));
	int16_t shape_index = lua_tonumber(L, 2);

	set_ephemera_shape(ephemera_index, set_shape(object->shape, shape_index));

	return 0;
}

static int Lua_Ephemera_Set_Tiny(lua_State* L)
{
	if (!lua_isboolean(L, 2))
		return luaL_error(L, "tiny: incorrect argument type");

	auto object = get_ephemera_data(Lua_Ephemera::Index(L, 1));

	if (lua_toboolean(L, 2)) {
		object->flags |= _object_is_tiny;
	} else {
		object->flags &= ~_object_is_tiny;
	}

	return 0;
}

const luaL_Reg Lua_Ephemera_Set[] = {
	{"clut_index", Lua_Ephemera_Set_Clut_Index},
	{"collection", Lua_Ephemera_Set_Collection},
	{"end_when_animation_loops", Lua_Ephemera_Set_End_When_Animation_Loops},
	{"enlarged", Lua_Ephemera_Set_Enlarged},
	{"facing", Lua_Ephemera_Set_Facing},
	{"shape_index", Lua_Ephemera_Set_Shape_Index},
	{"tiny", Lua_Ephemera_Set_Tiny},
	{0, 0}
};

char Lua_Ephemera_Name[] = "ephemera";
char Lua_Ephemeras_Name[] = "Ephemera";

char Lua_EphemeraQuality_Name[] = "ephemera_quality";
typedef L_Enum<Lua_EphemeraQuality_Name> Lua_EphemeraQuality;

char Lua_EphemeraQualities_Name[] = "EphemeraQualities";
typedef L_EnumContainer<Lua_EphemeraQualities_Name, Lua_EphemeraQuality> Lua_EphemeraQualities;

static int Lua_Ephemeras_Get_Quality(lua_State* L)
{
	Lua_EphemeraQuality::Push(L, graphics_preferences->ephemera_quality);
	return 1;
}

// x, y, z, polygon, collection, sequence, facing
static int Lua_Ephemeras_New(lua_State* L)
{
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) ||
		!lua_isnumber(L, 6) || !lua_isnumber(L, 7))
		luaL_error(L, "new: incorrect argument type");

	int polygon_index = 0;
	if (lua_isnumber(L, 4))
	{
		polygon_index = static_cast<int>(lua_tonumber(L, 4));
		if (!Lua_Polygon::Valid(polygon_index))
			return luaL_error(L, "new: invalid polygon index");
	}
	else if (Lua_Polygon::Is(L, 4))
	{
		polygon_index = Lua_Polygon::Index(L, 4);
	}
	else
		return luaL_error(L, "new: incorrect argument type");

	uint16_t collection = Lua_Collection::ToIndex(L, 5);
	uint16_t shape = static_cast<uint16_t>(lua_tonumber(L, 6));

	world_point3d origin;
	origin.x = static_cast<int>(lua_tonumber(L, 1) * WORLD_ONE);
	origin.y = static_cast<int>(lua_tonumber(L, 2) * WORLD_ONE);
	origin.z = static_cast<int>(lua_tonumber(L, 3) * WORLD_ONE);

	angle facing = static_cast<int>(lua_tonumber(L, 7) / AngleConvert);

	int16_t ephemera_index = ::new_ephemera(origin, polygon_index, BUILD_DESCRIPTOR(collection, shape), facing);

	if (ephemera_index == NONE)
	{
		return 0;
	}

	Lua_Ephemera::Push(L, ephemera_index);
	return 1;
}

const luaL_Reg Lua_Ephemeras_Methods[] = {
	{"new", L_TableFunction<Lua_Ephemeras_New>},
	{"quality", Lua_Ephemeras_Get_Quality},
	{0, 0}
};

static bool Lua_Ephemera_Valid(int32 index)
{
	if (index < 0 || index >= get_dynamic_limit(_dynamic_limit_ephemera))
	{
		return false;
	}
	else
	{
		return SLOT_IS_USED(get_ephemera_data(index));
	}
}

int Lua_Ephemera_register(lua_State* L)
{
	Lua_EphemeraQuality::Register(L, 0, 0, 0, Lua_EphemeraQuality_Mnemonics);
	Lua_EphemeraQuality::Valid = Lua_EphemeraQuality::ValidRange(static_cast<int>(_ephemera_ultra) + 1);

	Lua_EphemeraQualities::Register(L);
	Lua_EphemeraQualities::Length = Lua_EphemeraQualities::ConstantLength(5);
	
	Lua_Ephemera::Register(L, Lua_Ephemera_Get, Lua_Ephemera_Set);
	Lua_Ephemera::Valid = Lua_Ephemera_Valid;

	Lua_Ephemeras::Register(L, Lua_Ephemeras_Methods);
	Lua_Ephemeras::Length = std::bind(get_dynamic_limit, (int) _dynamic_limit_ephemera);

	return 0;
}
