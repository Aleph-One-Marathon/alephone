#ifndef _PACKING_
#define _PACKING_
/*

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

	Created by Loren Petrich, August 28, 2000

	Basic packing and unpacking routines. These are used because the Marathon series has used
	packed big-endian data formats, while internally, data is most efficiently accessed
	if it has native alignments, which often involve compiler-generated padding.
	Furthermore, the data may internally be little-endian instead of big-endian.

	The packed form of the data is a simple stream of bytes, and as the routines run,
	they advance the stream pointer appropriately. So be sure to keep a copy of an object's
	base pointer when using these routines.
	
	The syntax was chosen to make it easy to create an unpacking version of a packing routine,
	by simply changing the routine names.
	
	StreamToValue(uint8* &Stream, T& Number)
		unpacks a stream into a numerical value (T is int16, uint16, int32, uint32)
	
	ValueToStream(uint8* &Stream, T Number)
		packs a numerical value (T is int16, uint16, int32, uint32) into a stream
	
	StreamToList(uint8* &Stream, T* List, size_t Count)
		unpacks a stream into a list of numerical values (T is int16, uint16, int32, uint32)
	
	ListToStream(uint8* &Stream, const T* List, size_t Count)
		packs a list of numerical values (T is int16, uint16, int32, uint32) into a stream
	
	StreamToBytes(uint8* &Stream, void* Bytes, size_t Count)
		unpacks a stream into a block of bytes
	
	BytesToStream(uint8* &Stream, const void* Bytes, size_t Count)
		packs a block of bytes into a stream

Aug 27, 2002 (Alexander Strange):
	Moved functions to Packing.cpp to get around inlining issues.
*/

#include "cstypes.h"

// Default: packed-data is big-endian.
// May be overridden by some previous definition,
// as is the case for the 3D Studio Max loader code,
// which uses little-endian order
#if !(defined(PACKED_DATA_IS_BIG_ENDIAN)) && !(defined(PACKED_DATA_IS_LITTLE_ENDIAN)) && (!defined(PACKING_INTERNAL))
#define PACKED_DATA_IS_BIG_ENDIAN
#undef PACKED_DATA_IS_LITTLE_ENDIAN
#endif

#if (defined(PACKED_DATA_IS_BIG_ENDIAN)) && (defined(PACKED_DATA_IS_LITTLE_ENDIAN))
#error "PACKED_DATA_IS_BIG_ENDIAN and PACKED_DATA_IS_LITTLE_ENDIAN cannot both be defined at the same time!"
#endif

#ifdef PACKED_DATA_IS_BIG_ENDIAN
#define StreamToValue StreamToValueBE
#define ValueToStream ValueToStreamBE
#endif

#ifdef PACKED_DATA_IS_LITTLE_ENDIAN
#define StreamToValue StreamToValueLE
#define ValueToStream ValueToStreamLE
#endif

extern void StreamToValue(uint8* &Stream, uint16 &Value);
extern void StreamToValue(uint8* &Stream, int16 &Value);
extern void StreamToValue(uint8* &Stream, uint32 &Value);
extern void StreamToValue(uint8* &Stream, int32 &Value);
extern void ValueToStream(uint8* &Stream, uint16 Value);
extern void ValueToStream(uint8* &Stream, int16 Value);
extern void ValueToStream(uint8* &Stream, uint32 Value);
extern void ValueToStream(uint8* &Stream, int32 Value);

#ifndef PACKING_INTERNAL
template<class T> inline static void StreamToList(uint8* &Stream, T* List, size_t Count)
{
    T* ValuePtr = List;
    for (size_t k=0; k<Count; k++)
        StreamToValue(Stream,*(ValuePtr++));
}


template<class T> inline static void ListToStream(uint8* &Stream, T* List, size_t Count)
{
    T* ValuePtr = List;
    for (size_t k=0; k<Count; k++)
        ValueToStream(Stream,*(ValuePtr++));
}

inline static void StreamToBytes(uint8* &Stream, void* Bytes, size_t Count)
{
    memcpy(Bytes,Stream,Count);
    Stream += Count;
}

inline static void BytesToStream(uint8* &Stream, const void* Bytes, size_t Count)
{
    memcpy(Stream,Bytes,Count);
    Stream += Count;
}
#endif
#endif
