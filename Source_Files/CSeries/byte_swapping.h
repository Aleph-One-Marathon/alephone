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

#ifndef LITTLE_ENDIAN
#define byte_swap_data(data,elsize,elcount,fields) ((void)0)
#define byte_swap_memory(memory,type,elcount) ((void)0)
#endif

// LP: convenient templates for byte-swapping single objects and object lists;
// they will work out the number of bytes from the object type.

template<class T> void byte_swap_object(T& object, _bs_field *fields)
	{byte_swap_data(&object,sizeof(object),1,fields);}

template<class T> void byte_swap_object_list(T *object_list, int num_objects, _bs_field *fields)
	{byte_swap_data(object_list,sizeof(object),num_objects,fields);}


#if defined(SDL)
#include <SDL/SDL_endian.h>
#elif defined(mac)
#define SDL_SwapBE16(x) (x)
#define SDL_SwapBE32(x) (x)
#endif

#endif
