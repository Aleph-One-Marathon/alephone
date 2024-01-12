/*
WORLD.H

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

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

Jul 1, 2000 (Loren Petrich):
	Inlined the angle normalization; using tricky code for that
*/

#ifndef _WORLD_H
#define _WORLD_H

#include "cstypes.h"
#include <tuple>

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

#define DEFAULT_RANDOM_SEED ((uint16)0xfded)

/* ---------- types */

typedef int16 angle;
typedef _fixed fixed_angle; // angle with _fixed precision
typedef int16 world_distance;

/* ---------- macros */

#define INTEGER_TO_WORLD(s) (((world_distance)(s))<<WORLD_FRACTIONAL_BITS)
#define WORLD_FRACTIONAL_PART(d) ((d)&((world_distance)(WORLD_ONE-1)))
#define WORLD_INTEGERAL_PART(d) ((d)>>WORLD_FRACTIONAL_BITS)

#define WORLD_TO_FIXED(w) (((_fixed)(w))<<(FIXED_FRACTIONAL_BITS-WORLD_FRACTIONAL_BITS))
#define FIXED_TO_WORLD(f) ((world_distance)((f)>>(FIXED_FRACTIONAL_BITS-WORLD_FRACTIONAL_BITS)))

#define FACING4(a) (NORMALIZE_ANGLE((a)-EIGHTH_CIRCLE)>>(ANGULAR_BITS-2))
#define FACING5(a) ((NORMALIZE_ANGLE((a)-FULL_CIRCLE/10))/((NUMBER_OF_ANGLES/5)+1))
#define FACING8(a) (NORMALIZE_ANGLE((a)-SIXTEENTH_CIRCLE)>>(ANGULAR_BITS-3))

/* arguments must be positive (!) or use guess_hypotenuse() */
#define GUESS_HYPOTENUSE(x, y) ((x)>(y) ? ((x)+((y)>>1)) : ((y)+((x)>>1)))

/* -360°<t<720° (!) or use normalize_angle() */
//#define NORMALIZE_ANGLE(t) ((t)<(angle)0?(t)+NUMBER_OF_ANGLES:((t)>=NUMBER_OF_ANGLES?(t)-NUMBER_OF_ANGLES:(t)))
#define NORMALIZE_ANGLE(t) ((t)&(angle)(NUMBER_OF_ANGLES-1))

/* ---------- int32 (long_...) and int16 (world_...) vectors and points */

// Conversions:
//     world_ to long_:  implicit
//     long_ to world_:  use to_world() (truncates)
//     3D to 2D:         use .ij() or .xy()
//     2D to 3D:         no shortcut currently
//     vector to point:  write long_pointNd{} + vec
//     point to vector:  write pt - long_pointNd{}
// Math ops:
//                 -vector  ->  long_vector
//     vector {+,-} vector  ->  long_vector
//         scalar * vector  ->  long_vector
//           point - point  ->  long_vector
//      point {+,-} vector  ->  long_point
//          vector + point  ->  unsupported (other way around is clearer)
//     long_ types support compound assignment
//     no guards against int32 overflow

struct long_vector2d
{
	int32 i, j;
	constexpr auto& operator+=(long_vector2d b) { i += b.i; j += b.j; return *this; }
	constexpr auto& operator-=(long_vector2d b) { i -= b.i; j -= b.j; return *this; }
	template <class S> constexpr auto& operator*=(S s) { return (*this = {int32(s*i), int32(s*j)}); }
};

struct long_vector3d
{
	int32 i, j, k;
	constexpr auto& operator+=(long_vector3d b) { i += b.i; j += b.j; k += b.k; return *this; }
	constexpr auto& operator-=(long_vector3d b) { i -= b.i; j -= b.j; k -= b.k; return *this; }
	template <class S> constexpr auto& operator*=(S s) { return (*this = {int32(s*i), int32(s*j), int32(s*k)}); }
	constexpr auto ij() const { return long_vector2d{i, j}; }
};

struct long_point2d
{
	int32 x, y;
	constexpr auto& operator+=(long_vector2d v) { x += v.i; y += v.j; return *this; }
	constexpr auto& operator-=(long_vector2d v) { x -= v.i; y -= v.j; return *this; }
};

struct long_point3d
{
	int32 x, y, z;
	constexpr auto& operator+=(long_vector3d v) { x += v.i; y += v.j; z += v.k; return *this; }
	constexpr auto& operator-=(long_vector3d v) { x -= v.i; y -= v.j; z -= v.k; return *this; }
	constexpr auto xy() const { return long_point2d{x, y}; }
};

struct world_vector2d
{
	world_distance i, j;
	/*implicit*/ constexpr operator long_vector2d() const { return {i, j}; }
};

struct world_vector3d
{
	world_distance i, j, k;
	/*implicit*/ constexpr operator long_vector3d() const { return {i, j, k}; }
	constexpr auto ij() const { return world_vector2d{i, j}; }
};

struct world_point2d
{
	world_distance x, y;
	/*implicit*/ constexpr operator long_point2d() const { return {x, y}; }
};

struct world_point3d
{
	world_distance x, y, z;
	/*implicit*/ constexpr operator long_point3d() const { return {x, y, z}; }
	constexpr auto xy() const { return world_point2d{x, y}; }
};

// world_ operands promote
constexpr bool operator==(long_vector2d a, long_vector2d b) { return a.i == b.i && a.j == b.j; }
constexpr bool operator==(long_vector3d a, long_vector3d b) { return a.i == b.i && a.j == b.j && a.k == b.k; }
constexpr bool operator==(long_point2d a, long_point2d b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator==(long_point3d a, long_point3d b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
constexpr bool operator!=(long_vector2d a, long_vector2d b) { return !(a == b); }
constexpr bool operator!=(long_vector3d a, long_vector3d b) { return !(a == b); }
constexpr bool operator!=(long_point2d a, long_point2d b) { return !(a == b); }
constexpr bool operator!=(long_point3d a, long_point3d b) { return !(a == b); }
constexpr auto operator+(long_vector2d a, long_vector2d b) { return a += b; }
constexpr auto operator+(long_vector3d a, long_vector3d b) { return a += b; }
constexpr auto operator-(long_vector2d a, long_vector2d b) { return a -= b; }
constexpr auto operator-(long_vector3d a, long_vector3d b) { return a -= b; }
constexpr auto operator-(long_vector2d v) { return long_vector2d{} - v; }
constexpr auto operator-(long_vector3d v) { return long_vector3d{} - v; }
template <class S> constexpr auto operator*(S s, long_vector2d v) { return v *= s; }
template <class S> constexpr auto operator*(S s, long_vector3d v) { return v *= s; }
constexpr auto operator-(long_point2d a, long_point2d b) { return long_vector2d{a.x - b.x, a.y - b.y}; }
constexpr auto operator-(long_point3d a, long_point3d b) { return long_vector3d{a.x - b.x, a.y - b.y, a.z - b.z}; }
constexpr auto operator+(long_point2d p, long_vector2d v) { return p += v; }
constexpr auto operator+(long_point3d p, long_vector3d v) { return p += v; }
constexpr auto operator-(long_point2d p, long_vector2d v) { return p -= v; }
constexpr auto operator-(long_point3d p, long_vector3d v) { return p -= v; }

constexpr auto to_world(long_vector2d v) { return world_vector2d{int16(v.i), int16(v.j)}; }
constexpr auto to_world(long_vector3d v) { return world_vector3d{int16(v.i), int16(v.j), int16(v.k)}; }
constexpr auto to_world(long_point2d p) { return world_point2d{int16(p.x), int16(p.y)}; }
constexpr auto to_world(long_point3d p) { return world_point3d{int16(p.x), int16(p.y), int16(p.z)}; }

/* ---------- fixed-point vectors and points */

struct fixed_vector3d
{
	_fixed i, j, k;
};

struct fixed_point3d
{
	_fixed x, y, z;
};

/* ---------- angle structures */

// A relative or (possibly non-normalized) absolute direction
struct fixed_yaw_pitch { fixed_angle yaw, pitch; };

/* ---------- locations */

struct world_location3d
{
	world_point3d point;
	short polygon_index;
	
	angle yaw, pitch;

	world_vector3d velocity;

	bool operator==(const world_location3d& other) const {
		return std::tie(pitch, yaw, polygon_index, point, velocity) == std::tie(other.pitch, other.yaw, other.polygon_index, other.point, other.velocity);
	}

	bool operator!=(const world_location3d& other) const {
		return !(*(this) == other);
	}
};
typedef struct world_location3d world_location3d;

/* ---------- globals */

extern short *cosine_table, *sine_table;
/* tangent table is for internal use only (during arctangent calls) */

/* ---------- prototypes: WORLD.C */

void build_trig_tables(void);

// LP change: inlined this for speed, and used the NORMALIZE_ANGLE macro;
// looks as if the code had been worked on by more than one programmer.
static inline angle normalize_angle(angle theta)
{
	return NORMALIZE_ANGLE(theta);
}

// Return (a cross b).k
inline Sint64 cross_product_k(long_vector2d a, long_vector2d b) { return 1LL*a.i*b.j - 1LL*a.j*b.i; }

world_point2d *rotate_point2d(world_point2d *point, world_point2d *origin, angle theta);
world_point3d *rotate_point3d(world_point3d *point, world_point3d *origin, angle theta, angle phi);

world_point2d *translate_point2d(world_point2d *point, world_distance distance, angle theta);
world_point3d *translate_point3d(world_point3d *point, world_distance distance, angle theta, angle phi);

world_point2d *transform_point2d(world_point2d *point, world_point2d *origin, angle theta);
world_point3d *transform_point3d(world_point3d *point, world_point3d *origin, angle theta, angle phi);

/* angle is in [0,NUMBER_OF_ANGLES), or, [0,2π) */
// LP change: made this long-distance friendly
angle arctangent(int32 x, int32 y);

void set_random_seed(uint16 seed);
uint16 get_random_seed(void);
uint16 global_random(void);

uint16 local_random(void);

world_distance guess_distance2d(world_point2d *p0, world_point2d *p1);
world_distance distance3d(world_point3d *p0, world_point3d *p1);
world_distance distance2d(world_point2d *p0, world_point2d *p1); /* calls isqrt() */

int32 isqrt(uint32 x);

// LP additions: kludges for doing long-distance calculation
// by storing the upper digits in the upper byte of a "flags" value.
// These digits are the first 4 of X and Y beyond the short-integer digits.

void long_to_overflow_short_2d(long_vector2d& LVec, world_point2d& WVec, uint16& flags);
void overflow_short_to_long_2d(world_point2d& WVec, uint16& flags, long_vector2d& LVec);

// Transform that produces a result with this kludge
world_point2d *transform_overflow_point2d(world_point2d *point, world_point2d *origin, angle theta, uint16 *flags);

#endif
