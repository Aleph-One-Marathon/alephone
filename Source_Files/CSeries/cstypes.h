#define NONE (-1)

typedef int32 fixed;

#define FIXED_FRACTIONAL_BITS 16
#define INTEGER_TO_FIXED(i) ((fixed)(i)<<FIXED_FRACTIONAL_BITS)
#define FIXED_INTEGERAL_PART(f) ((f)>>FIXED_FRACTIONAL_BITS)

#define FIXED_ONE		(1L<<FIXED_FRACTIONAL_BITS)
#define FIXED_ONE_HALF	(1L<<(FIXED_FRACTIONAL_BITS-1))

#define MEG 0x100000
#define KILO 0x400L

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

