// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
/*
	Jul 1, 2000 (Loren Petrich):
		Added accessor template function

Aug 27, 2000 (Loren Petrich):
	Added object wrappers for memcpy() and memset(); these copy, set, and clear objects
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

template<class T> void objlist_copy(T* destination, const T* source, int num_objects)
	{memcpy(destination, source, num_objects*sizeof(T));}

template<class T> void obj_set(T& object, int value)
	{memset(&object, value, sizeof(T));}

template<class T> void objlist_set(T* object_list, int value, int num_objects)
	{memset(object_list, value, num_objects*sizeof(T));}

template<class T> void obj_clear(T& object)
	{obj_set(object, 0);}

template<class T> void objlist_clear(T* object_list, int num_objects)
	{objlist_set(object_list, 0, num_objects);}


#endif

