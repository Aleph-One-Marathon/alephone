// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
/*
	Jul 1, 2000 (Loren Petrich):
		Added accessor template function

*/
#ifndef _CSERIES_MACROS_
#define _CSERIES_MACROS_

#define FLOOR(value,floor) ((value)<(floor) ? (floor) : (value))
#define CEILING(value,ceiling) ((value)>(ceiling) ? (ceiling) : (value))

#define PIN(value,floor,ceiling) \
	((value)<(floor) ? (floor) : (value)>(ceiling) ? (ceiling) : (value))

#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MIN(a,b) ((a)<(b) ? (a) : (b))

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SGN(x) ((x)<0 ? -1 : (x)>0 ? 1 : 0)

#define SWAP(a,b) do{long _tmp_=(a);(a)=(b);(b)=_tmp_;}while(0)

#define FLAG(bit) (1L<<(bit))
#define TEST_FLAG32(flags,bit) (((flags)&FLAG(bit))!=0)
#define SET_FLAG32(flags,bit,value) ((value) ? ((flags)|=FLAG(bit)) : ((flags)&=~FLAG(bit)))

#define FLAG16(bit) (1<<(bit))
#define TEST_FLAG16(flags,bit) (((flags)&FLAG16(bit))!=0)
#define SET_FLAG16(flags,bit,value) ((void)((value) ? ((flags)|=FLAG16(bit)) : ((flags)&=~FLAG16(bit))))

// LP addition (Mar 2, 2000): some more generic routines for flags
#define TEST_FLAG(obj,flag) (obj&flag)
#define SET_FLAG(obj,flag,value) ((void)((value)?((obj)|=(flag)):((obj)&=~(flag))))

/*
	LP addition: template class for doing bounds checking when accessing an array;
	it uses an array, an index value, and an intended number of members for that array.
	It will return a pointer to the array member, if that member is in range, or else
	the null pointer. Its caller must check whether a null pointer had been returned,
	and then perform the appropriate action.
*/

template<class T> T* GetMemberWithBounds(T* Array, const int Index, const int Number)
{
	// Bounds checking
	if (!(Index>=0 && Index<Number)) return NULL;
	
	// The appropriate pointer
	return (Array + Index);
}

#endif

