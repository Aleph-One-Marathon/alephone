/*
WORLD.H
Wednesday, June 17, 1992 6:40:10 PM

Friday, June 26, 1992 10:47:47 AM
	to maintain precision when squaring world coordinates, they must be in [0,32*WORLD_ONE).
Wednesday, August 18, 1993 2:59:47 PM
	added 3d world points, world_distance is now a short (because we care).
Sunday, May 22, 1994 10:48:26 PM
	added fixed_point3d.  GUESS_HYPOTENUSE() is no longer completely broken.

Feb 15, 2000 (Loren Petrich):
	Moved definition of "long" versions of vectors into here
	
Feb 17, 2000 (Loren Petrich):
	Made the arctangent function long-distance friendly
*/

#ifndef _WORLD_H
#define _WORLD_H

// LP addition:
#include "cseries.h"

/* ---------- constants */

#define TRIG_SHIFT 10
#define TRIG_MAGNITUDE (1<<TRIG_SHIFT)

#define ANGULAR_BITS 9
#define NUMBER_OF_ANGLES ((short)(1<<ANGULAR_BITS))
#define FULL_CIRCLE NUMBER_OF_ANGLES
#define QUARTER_CIRCLE ((short)(NUMBER_OF_ANGLES/4))
#define HALF_CIRCLE ((short)(NUMBER_OF_ANGLES/2))
#define THREE_QUARTER_CIRCLE ((short)((NUMBER_OF_ANGLES*3)/4))
#define EIGHTH_CIRCLE ((short)(NUMBER_OF_ANGLES/8))
#define SIXTEENTH_CIRCLE ((short)(NUMBER_OF_ANGLES/16))

#define WORLD_FRACTIONAL_BITS 10
#define WORLD_ONE ((world_distance)(1<<WORLD_FRACTIONAL_BITS))
#define WORLD_ONE_HALF ((world_distance)(WORLD_ONE/2))
#define WORLD_ONE_FOURTH ((world_distance)(WORLD_ONE/4))
#define WORLD_THREE_FOURTHS ((world_distance)((WORLD_ONE*3)/4))

#define DEFAULT_RANDOM_SEED ((word)0xfded)

/* ---------- types */

typedef short angle;
typedef short world_distance;

/* ---------- macros */

#define INTEGER_TO_WORLD(s) (((world_distance)(s))<<WORLD_FRACTIONAL_BITS)
#define WORLD_FRACTIONAL_PART(d) ((d)&((world_distance)(WORLD_ONE-1)))
#define WORLD_INTEGERAL_PART(d) ((d)>>WORLD_FRACTIONAL_BITS)

#define WORLD_TO_FIXED(w) (((fixed)(w))<<(FIXED_FRACTIONAL_BITS-WORLD_FRACTIONAL_BITS))
#define FIXED_TO_WORLD(f) ((world_distance)((f)>>(FIXED_FRACTIONAL_BITS-WORLD_FRACTIONAL_BITS)))

#define FACING4(a) (NORMALIZE_ANGLE((a)-EIGHTH_CIRCLE)>>(ANGULAR_BITS-2))
#define FACING5(a) ((NORMALIZE_ANGLE((a)-FULL_CIRCLE/10))/((NUMBER_OF_ANGLES/5)+1))
#define FACING8(a) (NORMALIZE_ANGLE((a)-SIXTEENTH_CIRCLE)>>(ANGULAR_BITS-3))

/* arguments must be positive (!) or use guess_hypotenuse() */
#define GUESS_HYPOTENUSE(x, y) ((x)>(y) ? ((x)+((y)>>1)) : ((y)+((x)>>1)))

/* -360¡<t<720¡ (!) or use normalize_angle() */
//#define NORMALIZE_ANGLE(t) ((t)<(angle)0?(t)+NUMBER_OF_ANGLES:((t)>=NUMBER_OF_ANGLES?(t)-NUMBER_OF_ANGLES:(t)))
#define NORMALIZE_ANGLE(t) ((t)&(angle)(NUMBER_OF_ANGLES-1))

/* ---------- point structures */

struct world_point2d
{
	world_distance x, y;
};
typedef struct world_point2d world_point2d;

struct world_point3d
{
	world_distance x, y, z;
};
typedef struct world_point3d world_point3d;

struct fixed_point3d
{
	fixed x, y, z;
};
typedef struct fixed_point3d fixed_point3d;

/* ---------- vector structures */

struct world_vector2d
{
	world_distance i, j;
};
typedef struct world_vector2d world_vector2d;

struct world_vector3d
{
	world_distance i, j, k;
};
typedef struct world_vector3d world_vector3d;

struct fixed_vector3d
{
	fixed i, j, k;
};
typedef struct fixed_vector3d fixed_vector3d;

// LP addition: long-integer intermediate values for better long-distance calculation

struct long_point2d
{
	long x, y;
};
typedef struct long_point2d long_point2d;

struct long_point3d
{
	long x, y, z;
};
typedef struct long_point3d long_point3d;

struct long_vector2d
{
	long i, j;
};
typedef struct long_vector2d long_vector2d;

struct long_vector3d
{
	long i, j, k;
};
typedef struct long_vector3d long_vector3d;

/* ---------- locations */

struct world_location3d
{
	world_point3d point;
	short polygon_index;
	
	angle yaw, pitch;

	world_vector3d velocity;
};
typedef struct world_location3d world_location3d;

/* ---------- globals */

extern short *cosine_table, *sine_table;
/* tangent table is for internal use only (during arctangent calls) */

/* ---------- prototypes: WORLD.C */

void build_trig_tables(void);
angle normalize_angle(angle theta);

world_point2d *rotate_point2d(world_point2d *point, world_point2d *origin, angle theta);
world_point3d *rotate_point3d(world_point3d *point, world_point3d *origin, angle theta, angle phi);

world_point2d *translate_point2d(world_point2d *point, world_distance distance, angle theta);
world_point3d *translate_point3d(world_point3d *point, world_distance distance, angle theta, angle phi);

world_point2d *transform_point2d(world_point2d *point, world_point2d *origin, angle theta);
world_point3d *transform_point3d(world_point3d *point, world_point3d *origin, angle theta, angle phi);

/* angle is in [0,NUMBER_OF_ANGLES), or, [0,2¹) */
// LP change: made this long-distance friendly
angle arctangent(long x, long y);
// angle arctangent(world_distance x, world_distance y);

void set_random_seed(word seed);
word get_random_seed(void);
word random(void);

word local_random(void);

world_distance guess_distance2d(world_point2d *p0, world_point2d *p1);
world_distance distance3d(world_point3d *p0, world_point3d *p1);
world_distance distance2d(world_point2d *p0, world_point2d *p1); /* calls isqrt() */

long isqrt(register unsigned long x);

// LP additions: kludges for doing long-distance calculation
// by storing the upper digits in the upper byte of a "flags" value.
// These digits are the first 4 of X and Y beyond the short-integer digits.

void long_to_overflow_short_2d(long_vector2d& LVec, world_point2d& WVec, word& flags);
void overflow_short_to_long_2d(world_point2d& WVec, word& flags, long_vector2d& LVec);

// Transform that produces a result with this kludge
world_point2d *transform_overflow_point2d(world_point2d *point, world_point2d *origin, angle theta, word *flags);

// Simple copy-overs
inline void long_to_short_2d(long_vector2d& LVec, world_vector2d&WVec)
{
	WVec.i = LVec.i;
	WVec.j = LVec.j;
}
inline void short_to_long_2d(world_vector2d&WVec, long_vector2d& LVec)
{
	LVec.i = WVec.i;
	LVec.j = WVec.j;
}

#endif
