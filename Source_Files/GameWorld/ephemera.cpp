/*
	Copyright (C) 2020 and beyond by Gregory Smith
 
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
*/

#include "ephemera.h"

#include "dynamic_limits.h"
#include "map.h"

class ObjectDataPool {
public:
	ObjectDataPool() :
		pool_{get_dynamic_limit(_dynamic_limit_ephemera)},
		first_unused_{NONE} { }
	
	void init();
	void resize(int size) { pool_.resize(size); }

	object_data& get(int16_t object_index) {
		// TODO: settle on bounds checking
		return pool_.at(object_index);
	}

	int size() { return static_cast<int>(pool_.size()); }
	
	int16_t get_unused(); // marks as unused before returning
	void release(int16_t object_index);

private:
	std::vector<object_data> pool_;
	int16_t first_unused_;
};

std::vector<int16_t> polygon_ephemera;
ObjectDataPool ephemera_pool;

void ObjectDataPool::init()
{
	int size = static_cast<int>(pool_.size());
	if (size)
	{
		for (auto i = 0; i < size - 1; ++i)
		{
			MARK_SLOT_AS_FREE(&pool_[i]);
			pool_[i].next_object = i + 1;
		}
		
		MARK_SLOT_AS_FREE(&pool_[size - 1]);
		pool_[size - 1].next_object = NONE;
		
		first_unused_ = 0;
	}
	else
	{
		first_unused_ = NONE;
	}
}

int16_t ObjectDataPool::get_unused()
{
	int16_t index = NONE;
	if (first_unused_ != NONE)
	{
		index = first_unused_;
		first_unused_ = pool_[index].next_object;
		MARK_SLOT_AS_USED(&pool_[index]);
	}

	return index;
}

void ObjectDataPool::release(int16_t index)
{
	pool_[index].next_object = first_unused_;
	MARK_SLOT_AS_FREE(&pool_[index]);
	first_unused_ = index;
}

void allocate_ephemera_storage(int max_ephemera)
{
	ephemera_pool.resize(max_ephemera);
}

void init_ephemera(int16_t polygon_count)
{
	polygon_ephemera.clear();
	polygon_ephemera.resize(polygon_count, NONE);

	ephemera_pool.init();
}

int16_t new_ephemera(const world_point3d& location, int16_t polygon_index, shape_descriptor shape, angle facing)
{
	int16_t ephemera_index = NONE;
	
	if (polygon_index != NONE)
	{
		ephemera_index = ephemera_pool.get_unused();
		if (ephemera_index != NONE)
		{
			auto& object = ephemera_pool.get(ephemera_index);
			object.polygon = polygon_index;
			object.location = location;
			object.facing = facing;
			object.shape = shape;
			object.sequence = 0;
			object.flags = 0x8000; // SLOT_IS_USED
			object.next_object = NONE;
			// the renderer does not know how to look up parasitic objects for
			// ephemera
			object.parasitic_object = NONE; 
			object.sound_pitch = FIXED_ONE;

			// insert at head of linked list
			object.next_object = polygon_ephemera.at(polygon_index);
			polygon_ephemera.at(polygon_index) = ephemera_index;
		}
	}

	return ephemera_index;
}

void remove_ephemera(int16_t ephemera_index)
{
	// remove from the polygon linked list
	auto& object = ephemera_pool.get(ephemera_index);

	int16_t* p = &polygon_ephemera.at(object.polygon);
	while (*p != ephemera_index) {
		p = &ephemera_pool.get(*p).next_object;
	}

	*p = object.next_object;
	object.polygon = NONE;

	ephemera_pool.release(ephemera_index);
}

int get_max_ephemera()
{
	return ephemera_pool.size();
}

object_data* get_ephemera_data(int16_t ephemera_index)
{
	return &ephemera_pool.get(ephemera_index);
}

int16_t get_polygon_ephemera(int16_t polygon_index)
{
	// TODO: settle on bounds checking
	return polygon_ephemera.at(polygon_index);
}
