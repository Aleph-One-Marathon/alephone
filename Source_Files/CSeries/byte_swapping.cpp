// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include "cseries.h"
#include "byte_swapping.h"

#ifdef ALEPHONE_LITTLE_ENDIAN

void byte_swap_memory(
	void *memory,
	_bs_field type,
	size_t fieldcount)
{
	uint8 *walk;
	int tmp;

	walk=(uint8 *)memory;
	switch (type) {
	case _2byte:
		while (fieldcount>0) {
			tmp=walk[0];
			walk[0]=walk[1];
			walk[1]=tmp;
			walk+=2;
			fieldcount--;
		}
		break;
	case _4byte:
		while (fieldcount>0) {
			tmp=walk[0];
			walk[0]=walk[3];
			walk[3]=tmp;
			tmp=walk[1];
			walk[1]=walk[2];
			walk[2]=tmp;
			walk+=4;
			fieldcount--;
		}
		break;
	}
}

#endif
