// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_TYPES_
#define _CSERIES_TYPES_

#define NONE (-1)

// LP: moved these types here for convenience
// Integer types with specific bit size
#if defined(mac)
typedef SInt8 int8;
typedef UInt8 uint8;
typedef SInt16 int16;
typedef UInt16 uint16;
typedef SInt32 int32;
typedef UInt32 uint32;
#elif defined(__BEOS__)
#include <support/SupportDefs.h>
#elif defined(SDL)
typedef Uint8 uint8;
typedef Sint8 int8;
typedef Uint16 uint16;
typedef Sint16 int16;
typedef Uint32 uint32;
typedef Sint32 int32;
#endif

typedef int32 fixed;

#define FIXED_FRACTIONAL_BITS 16
#define INTEGER_TO_FIXED(i) ((fixed)(i)<<FIXED_FRACTIONAL_BITS)
#define FIXED_INTEGERAL_PART(f) ((f)>>FIXED_FRACTIONAL_BITS)

#define FIXED_ONE		(1L<<FIXED_FRACTIONAL_BITS)
#define FIXED_ONE_HALF	(1L<<(FIXED_FRACTIONAL_BITS-1))

#define MEG 0x100000
#define KILO 0x400L

#undef SHORT_MAX
#undef SHORT_MIN
#undef LONG_MAX
#undef LONG_MIN
#define SHORT_MAX 32767
#define SHORT_MIN (-SHORT_MAX-1)
#define LONG_MAX 2147483647
#define LONG_MIN (-LONG_MAX-1)

typedef uint16 word;
typedef uint8 byte;
typedef unsigned char boolean;

#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif

// Distinction between Loren Petrich's and Christian Bauer's code;
// define whichever one is appropriate
#define LP
#undef CB

#endif
