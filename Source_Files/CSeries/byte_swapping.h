// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _BYTE_SWAPPING_
#define _BYTE_SWAPPING_

typedef short _bs_field;

enum {
	_2byte	= -2,
	_4byte	= -4
};

#include <stddef.h>

extern void byte_swap_data(
	void *data,
	size_t elsize,
	size_t elcount,
	_bs_field *fields);

extern void byte_swap_memory(
	void *memory,
	_bs_field type,
	size_t fieldcount);

#ifndef ALEPHONE_LITTLE_ENDIAN
#define byte_swap_data(data,elsize,elcount,fields) ((void)0)
#define byte_swap_memory(memory,type,elcount) ((void)0)
#endif

// LP: convenient templates for byte-swapping single objects and object lists;
// they will work out the number of bytes from the object type.

// CB: my plan is to eventually get rid of all byte-swapping stuff here;
// the idea was nice, but the current method of reading C structures from
// disk and swapping them in memory has portability problems (and makes
// writing more difficult); instead, functions to read and write data items
// (uint8, uint16, uint32, not more) in big-endian order should be added to
// OpenedFile

template<class T> void byte_swap_object(T &object, _bs_field *fields)
	{byte_swap_data(&object, sizeof(T), 1, fields);}

template<class T> void byte_swap_object_list(T *object_list, int num_objects, _bs_field *fields)
	{byte_swap_data(object_list, sizeof(T), num_objects, fields);}


#if defined(SDL)
#include <SDL/SDL_endian.h>
#elif defined(mac)
#define SDL_SwapBE16(x) (x)
#define SDL_SwapBE32(x) (x)
#endif

#endif
