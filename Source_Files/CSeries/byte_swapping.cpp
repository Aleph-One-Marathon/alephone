#include "cseries.h"
#include "byte_swapping.h"

#ifdef LITTLE_ENDIAN

void byte_swap_data(
	void *data,
	size_t elsize,
	size_t elcount,
	_bs_field *fields)
{
	uint8 *walk,*lim;
	int tmp;
	_bs_field *field;
	long one;

	walk=(uint8 *)data;
	while (elcount>0) {
		field=fields;
		lim=walk+elsize;
		while (walk<lim) {
			one=*field;
			field++;
			switch (one) {
			case _2byte:
				tmp=walk[0];
				walk[0]=walk[1];
				walk[1]=tmp;
				walk+=2;
				break;
			case _4byte:
				tmp=walk[0];
				walk[0]=walk[3];
				walk[3]=tmp;
				tmp=walk[1];
				walk[1]=walk[2];
				walk[2]=tmp;
				walk+=4;
				break;
			default:
				if (one>0)
					walk+=one;
			}
		}
		walk=lim;
		elcount--;
	}
}

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
