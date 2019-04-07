/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
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

*/

/*
	Jul 1, 2000 (Loren Petrich):
		Added accessor template function

Aug 27, 2000 (Loren Petrich):
	Added object wrappers for memcpy() and memset(); these copy, set, and clear objects
*/

#ifndef _CSERIES_MACROS_
#define _CSERIES_MACROS_

#include <string.h>
#include "FilmProfile.h" // TERRIBLE

#undef MAX
#define MAX(a,b) ((a)>=(b) ? (a) : (b))
#undef MIN
#define MIN(a,b) ((a)<=(b) ? (a) : (b))

#define FLOOR(value,floor) MAX(value,floor)
#define CEILING(value,ceiling) MIN(value,ceiling)

#define M2_PIN(value,floor,ceiling) \
	((value)<(floor) ? (floor) : (value)>(ceiling) ? (ceiling) : (value))
#define A1_PIN(value,floor,ceiling) (CEILING(FLOOR((value),(floor)),(ceiling)))

#define PIN(value,floor,ceiling) \
	((film_profile.inexplicable_pin_change) ? (A1_PIN(value,floor,ceiling)) : (M2_PIN(value,floor,ceiling)))

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SGN(x) ((x)<0 ? -1 : (x)>0 ? 1 : 0)

template <typename T> void
SWAP(T& a, T& b)
{
	T t = a;
	a = b;
	b = t;
}

#define FLAG(bit) (1L<<(bit))
#define TEST_FLAG32(flags,bit) (((flags)&FLAG(bit))!=0)
#define SET_FLAG32(flags,bit,value) ((value) ? ((flags)|=FLAG(bit)) : ((flags)&=~FLAG(bit)))

#define FLAG16(bit) (1<<(bit))
#define TEST_FLAG16(flags,bit) (((flags)&FLAG16(bit))!=0)
#define SET_FLAG16(flags,bit,value) ((void)((value) ? ((flags)|=FLAG16(bit)) : ((flags)&=~FLAG16(bit))))

// LP addition (Mar 2, 2000): some more generic routines for flags
#define TEST_FLAG(obj,flag) ((obj)&(flag))
#define SET_FLAG(obj,flag,value) ((void)((value)?((obj)|=(flag)):((obj)&=~(flag))))

#define RECTANGLE_WIDTH(rectptr) ((rectptr)->right-(rectptr)->left)
#define RECTANGLE_HEIGHT(rectptr) ((rectptr)->bottom-(rectptr)->top)

static inline int NextPowerOfTwo(int n)
{
	int p = 1;
	while(p < n) {p <<= 1;}
	return p;
}

#ifdef __cplusplus
/*
	LP addition: template class for doing bounds checking when accessing an array;
	it uses an array, an index value, and an intended number of members for that array.
	It will return a pointer to the array member, if that member is in range, or else
	the null pointer. Its caller must check whether a null pointer had been returned,
	and then perform the appropriate action.
*/

template<class T> T* GetMemberWithBounds(T* Array, const size_t Index, const size_t Number)
{
	// Bounds checking
	if (!(Index>=0 && Index<Number)) return NULL;
	
	// The appropriate pointer
	return (Array + Index);
}

/*
	LP addition: convenient type-safe wrappers for memcpy and memset,
		that get rid of annoying sizeof's. obj_ means a single object
		and objlist_ means a list of num_objects of them.
		The _copy set copies "source" to "destination"
		The _set set sets all the bytes to "value"
		The _clear set sets all the bytes to zero (a common operation)
*/

template<class T> void obj_copy(T& destination, const T& source)
	{memcpy(&destination, &source, sizeof(T));}

template<class T> void objlist_copy(T* destination, const T* source, size_t num_objects)
	{if (num_objects > 0) memcpy(destination, source, num_objects*sizeof(T));}

template<class T> void obj_set(T& object, int value)
	{memset(&object, value, sizeof(T));}

template<class T> void objlist_set(T* object_list, int value, size_t num_objects)
	{if (num_objects > 0) memset(object_list, value, num_objects*sizeof(T));}

template<class T> void obj_clear(T& object)
	{obj_set(object, 0);}

template<class T> void objlist_clear(T* object_list, size_t num_objects)
	{objlist_set(object_list, 0, num_objects);}
#endif
#endif
