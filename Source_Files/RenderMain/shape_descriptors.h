#ifndef __SHAPE_DESCRIPTORS_H
#define __SHAPE_DESCRIPTORS_H

/*
SHAPE_DESCRIPTORS.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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

Saturday, July 9, 1994 3:24:56 PM

Saturday, July 9, 1994 6:47:04 PM
	this header file is not in the makefile, so changing it won’t result in the nearly-full
	rebuild that it should.  if you change this, which you shouldn’t, you must touch the makefile.
	
Feb 3, 2000 (Loren Petrich):
	Renamed _collection_madd to _collection_vacbob
	Annotated some of the _collection's better
*/

#include "cstypes.h"

/* ---------- types */

using shape_descriptor = uint16; /* [clut.3] [collection.5] [shape.8] */

#define DESCRIPTOR_SHAPE_BITS 8
#define DESCRIPTOR_COLLECTION_BITS 5
#define DESCRIPTOR_CLUT_BITS 3
constexpr shape_descriptor MaximumCollections = 1 << DESCRIPTOR_SHAPE_BITS;
constexpr shape_descriptor MaximumShapesPerCollection = 1 << DESCRIPTOR_COLLECTION_BITS;
constexpr shape_descriptor MaximumClutsPerCollection = 1 << DESCRIPTOR_CLUT_BITS;
#define MAXIMUM_COLLECTIONS (MaximumCollections)
#define MAXIMUM_SHAPES_PER_COLLECTION (MaximumShapesPerCollection)
#define MAXIMUM_CLUTS_PER_COLLECTION (MaximumClutsPerCollection)

/* ---------- collections */

enum /* collection numbers */
{
	_collection_interface, // 0
	_collection_weapons_in_hand, // 1
	_collection_juggernaut, // 2
	_collection_tick, // 3
	_collection_rocket, // 4 -- LP: also known as "Explosion Effects"
	_collection_hunter, // 5
	_collection_player, // 6
	_collection_items, // 7
	_collection_trooper, // 8
	_collection_fighter, // 9
	_collection_defender, // 10
	_collection_yeti, // 11
	_collection_civilian, // 12
	_collection_civilian_fusion, // 13 -- LP: formerly _collection_madd
	_collection_enforcer, // 14
	_collection_hummer, // 15
	_collection_compiler, // 16
	_collection_walls1, // 17 -- LP: Lh'owon water
	_collection_walls2, // 18 -- LP: Lh'owon lava
	_collection_walls3, // 19 -- LP: Lh'owon sewage
	_collection_walls4, // 20 -- LP: Jjaro
	_collection_walls5, // 21 -- LP: Pfhor
	_collection_scenery1, // 22 -- LP: Lh'owon water
	_collection_scenery2, // 23 -- LP: Lh'owon lava
	_collection_scenery3, // 24 -- LP: Lh'owon sewage
	_collection_scenery4, // 25 pathways -- LP: Jjaro
	_collection_scenery5, // 26 alien -- LP: Pfhor
	_collection_landscape1, // 27 day -- LP: Lh'owon day
	_collection_landscape2, // 28 night -- LP: Lh'owon night
	_collection_landscape3, // 29 moon -- LP: Lh'owon moon
	_collection_landscape4, // 30 -- LP: outer space
	_collection_cyborg, // 31
	
	NUMBER_OF_COLLECTIONS
};

/* ---------- macros */
constexpr uint16 ShapeDescriptor_GetDescriptorShape(uint16 d) noexcept {
	return d & (MaximumShapesPerCollection - 1);
}
constexpr uint16 ShapeDescriptor_GetDescriptorCollection(uint16 d) noexcept {
	return ((d) >> DESCRIPTOR_SHAPE_BITS) & uint16(((1<< (DESCRIPTOR_COLLECTION_BITS + DESCRIPTOR_CLUT_BITS)) - 1));
}
constexpr uint16 ShapeDescriptor_BuildDescriptor(uint16 collection, uint16 shape) noexcept {
	return (collection << DESCRIPTOR_SHAPE_BITS) | shape;
}
constexpr uint16 ShapeDescriptor_GetCollectionClut(uint16 collection) noexcept {
	return ((collection) >> DESCRIPTOR_COLLECTION_BITS) & uint16(MAXIMUM_CLUTS_PER_COLLECTION - 1);
}
constexpr uint16 ShapeDescriptor_GetCollection(uint16 collection) noexcept {
	return ((collection) & (MaximumCollections - 1));
}
constexpr uint16 ShapeDescriptor_BuildCollection(uint16 collection, uint16 clut) noexcept {
	return collection | uint16(clut << DESCRIPTOR_COLLECTION_BITS);
}
#define GET_DESCRIPTOR_SHAPE(d) (ShapeDescriptor_GetDescriptorShape((d)))
#define GET_DESCRIPTOR_COLLECTION(d) (ShapeDescriptor_GetDescriptorCollection((d)))
#define BUILD_DESCRIPTOR(collection,shape) (ShapeDescriptor_BuildCollection((collection), (shape)))

#define BUILD_COLLECTION(collection,clut) ((collection)|(uint16)((clut)<<DESCRIPTOR_COLLECTION_BITS))
#define GET_COLLECTION_CLUT(collection) (ShapeDescriptor_GetCollectionClut((collection)))
#define GET_COLLECTION(collection) (ShapeDescriptor_GetCollection((collection)))

#endif
