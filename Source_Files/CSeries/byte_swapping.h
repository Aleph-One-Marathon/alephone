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
#define byte_swap_data(data,elsize,elcount,field) ((void)0)
#define byte_swap_memory(memory,type,elcount) ((void)0)
#endif

