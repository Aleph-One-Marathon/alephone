// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _BYTE_SWAPPING_
#define _BYTE_SWAPPING_

typedef short _bs_field;

enum {
	_2byte	= -2,
	_4byte	= -4
};

#include <stddef.h>

extern void byte_swap_memory(
	void *memory,
	_bs_field type,
	size_t fieldcount);

#ifndef ALEPHONE_LITTLE_ENDIAN
#define byte_swap_memory(memory,type,elcount) ((void)0)
#endif

#endif
