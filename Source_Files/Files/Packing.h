#ifndef _PACKING_
#define _PACKING_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
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
	
	StreamToList(uint8* &Stream, T* List, int Count)
		unpacks a stream into a list of numerical values (T is int16, uint16, int32, uint32)
	
	ListToStream(uint8* &Stream, const T* List, int Count)
		packs a list of numerical values (T is int16, uint16, int32, uint32) into a stream
	
	StreamToBytes(uint8* &Stream, void* Bytes, int Count)
		unpacks a stream into a block of bytes
	
	BytesToStream(uint8* &Stream, const void* Bytes, int Count)
		packs a block of bytes into a stream
*/

#include <string.h>

// Default: packed-data is big-endian.
// May be overridden by some previous definition,
// as is the case for the 3D Studio Max loader code,
// which uses little-endian order
#if !(defined(PACKED_DATA_IS_BIG_ENDIAN)) && !(defined(PACKED_DATA_IS_LITTLE_ENDIAN))
#define PACKED_DATA_IS_BIG_ENDIAN
#undef PACKED_DATA_IS_LITTLE_ENDIAN
#endif

#if !(defined(PACKED_DATA_IS_BIG_ENDIAN)) && !(defined(PACKED_DATA_IS_LITTLE_ENDIAN))
#error "At least one of PACKED_DATA_IS_BIG_ENDIAN and PACKED_DATA_IS_LITTLE_ENDIAN must be defined!"
#elif (defined(PACKED_DATA_IS_BIG_ENDIAN)) && (defined(PACKED_DATA_IS_LITTLE_ENDIAN))
#error "PACKED_DATA_IS_BIG_ENDIAN and PACKED_DATA_IS_LITTLE_ENDIAN cannot both be defined at the same time!"
#endif

#ifdef PACKED_DATA_IS_BIG_ENDIAN
#warning "Big Endian"
#define StreamToValue StreamToValueBE
#define ValueToStream ValueToStreamBE
#endif

#ifdef PACKED_DATA_IS_LITTLE_ENDIAN
#undef StreamToValue
#undef ValueToStream
#warning "Little Endian"
#define StreamToValue StreamToValueLE
#define ValueToStream ValueToStreamLE
#endif

inline static void StreamToValueBE(uint8* &Stream, uint16 &Value)
{
	// Must be unsigned, so they will be zero-extended
	uint16 Byte0 = uint16(*(Stream++));
	uint16 Byte1 = uint16(*(Stream++));

	Value = (Byte0 << 8) | Byte1;
}

inline static void StreamToValueBE(uint8* &Stream, int16 &Value)
{
	uint16 UValue;
	StreamToValueBE(Stream,UValue);
	Value = int16(UValue);
}

inline static void StreamToValueBE(uint8* &Stream, uint32 &Value)
{
	// Must be unsigned, so they will be zero-extended
	uint32 Byte0 = uint32(*(Stream++));
	uint32 Byte1 = uint32(*(Stream++));
	uint32 Byte2 = uint32(*(Stream++));
	uint32 Byte3 = uint32(*(Stream++));

	Value = (Byte0 << 24) | (Byte1 << 16) | (Byte2 << 8) | Byte3;
}

inline static void StreamToValueBE(uint8* &Stream, int32 &Value)
{
	uint32 UValue;
	StreamToValueBE(Stream,UValue);
	Value = int32(UValue);
}

inline static void ValueToStreamBE(uint8* &Stream, uint16 Value)
{
	*(Stream++) = uint8(Value >> 8);
	*(Stream++) = uint8(Value);
}

inline static void ValueToStreamBE(uint8* &Stream, int16 Value)
{
	ValueToStreamBE(Stream,uint16(Value));
}

inline static void ValueToStreamBE(uint8* &Stream, uint32 Value)
{
	*(Stream++) = uint8(Value >> 24);
	*(Stream++) = uint8(Value >> 16);
	*(Stream++) = uint8(Value >> 8);
	*(Stream++) = uint8(Value);
}


inline static void StreamToValueLE(uint8* &Stream, uint16 &Value)
{
    // Must be unsigned, so they will be zero-extended
    uint16 Byte0 = uint16(*(Stream++));
    uint16 Byte1 = uint16(*(Stream++));

    Value = (Byte1 << 8) | Byte0;
}

inline static void StreamToValueLE(uint8* &Stream, int16 &Value)
{
    uint16 UValue;
    StreamToValue(Stream,UValue);
    Value = int16(UValue);
}

inline static void StreamToValueLE(uint8* &Stream, uint32 &Value)
{
    // Must be unsigned, so they will be zero-extended
    uint32 Byte0 = uint32(*(Stream++));
    uint32 Byte1 = uint32(*(Stream++));
    uint32 Byte2 = uint32(*(Stream++));
    uint32 Byte3 = uint32(*(Stream++));

    Value = (Byte3 << 24) | (Byte2 << 16) | (Byte1 << 8) | Byte0;
}

inline static void StreamToValueLE(uint8* &Stream, int32 &Value)
{
    uint32 UValue;
    StreamToValue(Stream,UValue);
    Value = int32(UValue);
}

inline static void ValueToStreamLE(uint8* &Stream, uint16 Value)
{
    *(Stream++) = uint8(Value);
    *(Stream++) = uint8(Value >> 8);
}

inline static void ValueToStreamLE(uint8* &Stream, int16 Value)
{
    ValueToStream(Stream,uint16(Value));
}

inline static void ValueToStreamLE(uint8* &Stream, uint32 Value)
{
    *(Stream++) = uint8(Value);
    *(Stream++) = uint8(Value >> 8);
    *(Stream++) = uint8(Value >> 16);
    *(Stream++) = uint8(Value >> 24);
}

inline static void ValueToStream(uint8* &Stream, int32 Value)
{
	ValueToStream(Stream,uint32(Value));
}

template<class T> inline static void StreamToList(uint8* &Stream, T* List, int Count)
{
	T* ValuePtr = List;
	for (int k=0; k<Count; k++)
		StreamToValue(Stream,*(ValuePtr++));
}


template<class T> inline static void ListToStream(uint8* &Stream, T* List, int Count)
{
	T* ValuePtr = List;
	for (int k=0; k<Count; k++)
		ValueToStream(Stream,*(ValuePtr++));
}

inline static void StreamToBytes(uint8* &Stream, void* Bytes, int Count)
{
	memcpy(Bytes,Stream,Count);
	Stream += Count;
}

inline static void BytesToStream(uint8* &Stream, const void* Bytes, int Count)
{
	memcpy(Stream,Bytes,Count);
	Stream += Count;
}

#endif
