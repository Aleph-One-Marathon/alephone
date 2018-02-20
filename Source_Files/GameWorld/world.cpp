/*
WORLD.C

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

Sunday, May 31, 1992 3:57:12 PM

Friday, January 15, 1993 9:59:14 AM
	added arctangent function.
Thursday, January 21, 1993 9:46:24 PM
	fixed arctangent function.  normalize_angle() could stand to be improved.
Saturday, January 23, 1993 9:46:34 PM
	fixed arctangent, hopefully for the last time.  normalize_angle() is a little faster.
Monday, January 25, 1993 3:01:47 PM
	arctangent works (tested at 0.5¡ increments against SANEÕs tan), the only anomoly was
	apparently arctan(0)==180¡.
Wednesday, January 27, 1993 3:49:04 PM
	final fix to arctangent, we swear.  recall lim(arctan(x)) as x approaches ¹/2 or 3¹/4 is ±°,
	depending on which side we come from.  because we didn't realize this, arctan failed in the
	case where x was very close to but slightly below ¹/2.  i think weÕve seen the last monster
	suddenly ÔpanicÕ and bolt directly into a wall.
Sunday, July 25, 1993 11:51:42 PM
	the arctan of 0/0 is now (arbitrairly) ¹/2 because weÕre sick of assert(y) failing.
Monday, June 20, 1994 4:15:06 PM
	bug fix in translate_point3d().

Feb 10, 2000 (Loren Petrich):
	Fixed range check in translate_point3d()

Feb 14, 2000 (Loren Petrich):
	Doing some arithmetic as long values instead of short ones, so as to avoid annoying long-distance wraparound.

Feb 17, 2000 (Loren Petrich):
	Fixed arctangent() so that it gets the values into the right octants, and then does a binary search

Jul 1, 2000 (Loren Petrich):
	Inlined the angle normalization; doing it automatically for all the functions that work with angles
*/

#include "cseries.h"
#include "world.h"
#include "FilmProfile.h"

#include <stdlib.h>
#include <math.h>
#include <limits.h>




/* ---------- globals */

int16 *cosine_table;
int16 *sine_table;
static int32 *tangent_table;

static uint16 random_seed= 0x1;
static uint16 local_random_seed= 0x1;

/* ---------- code */

/*
angle normalize_angle(
	angle theta)
{
	while (theta<0) theta+= NUMBER_OF_ANGLES;
	while (theta>=NUMBER_OF_ANGLES) theta-= NUMBER_OF_ANGLES;
	
	return theta;
}
*/

/* remember this is not wholly accurate, both distance or the sine/cosine values could be
	negative, and the shift canÕt make negative numbers zero; this is probably ok because
	weÕll have -1/1024th instead of zero, which is basically our margin for error anyway ... */
world_point2d *translate_point2d(
	world_point2d *point,
	world_distance distance,
	angle theta)
{
	// LP change: idiot-proofed this
	theta = normalize_angle(theta);
	fc_assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	point->x+= (distance*cosine_table[theta])>>TRIG_SHIFT;
	point->y+= (distance*sine_table[theta])>>TRIG_SHIFT;
	
	return point;
}

/* same comment as above */
world_point3d *translate_point3d(
	world_point3d *point,
	world_distance distance,
	angle theta,
	angle phi)
{
	world_distance transformed_distance;
	
	// LP change: idiot-proofed this
	theta = normalize_angle(theta);
	phi = normalize_angle(phi);
	
	transformed_distance= (distance*cosine_table[phi])>>TRIG_SHIFT;
	point->x+= (transformed_distance*cosine_table[theta])>>TRIG_SHIFT;
	point->y+= (transformed_distance*sine_table[theta])>>TRIG_SHIFT;
	point->z+= (distance*sine_table[phi])>>TRIG_SHIFT;
	
	return point;
}

world_point2d *rotate_point2d(
	world_point2d *point,
	world_point2d *origin,
	angle theta)
{
	// LP change: lengthening the values for more precise calculations
	long_vector2d temp;
	
	theta = normalize_angle(theta);
	fc_assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	temp.i= int32(point->x)-int32(origin->x);
	temp.j= int32(point->y)-int32(origin->y);
	
	point->x= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT) +
		origin->x;
	point->y= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT) +
		origin->y;
	
	return point;
}

world_point2d *transform_point2d(
	world_point2d *point,
	world_point2d *origin,
	angle theta)
{
	// LP change: lengthening the values for more precise calculations
	long_vector2d temp;
	
	theta = normalize_angle(theta);
	fc_assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	temp.i= int32(point->x)-int32(origin->x);
	temp.j= int32(point->y)-int32(origin->y);
	
	point->x= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT);
	
	return point;
}

world_point3d *transform_point3d(
	world_point3d *point,
	world_point3d *origin,
	angle theta,
	angle phi)
{
	// LP change: lengthening the values for more precise calculations
	long_vector3d temporary;
	
	temporary.i= int32(point->x)-int32(origin->x);
	temporary.j= int32(point->y)-int32(origin->y);
	temporary.k= int32(point->z)-int32(origin->z);
	
	/* do theta rotation (in x-y plane) */
	point->x= ((temporary.i*cosine_table[theta])>>TRIG_SHIFT) + ((temporary.j*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temporary.j*cosine_table[theta])>>TRIG_SHIFT) - ((temporary.i*sine_table[theta])>>TRIG_SHIFT);
	
	/* do phi rotation (in x-z plane) */
	if (phi)
	{
		temporary.i= point->x;
		/* temporary.z is already set */
		
		point->x= ((temporary.i*cosine_table[phi])>>TRIG_SHIFT) + ((temporary.k*sine_table[phi])>>TRIG_SHIFT);
		point->z= ((temporary.k*cosine_table[phi])>>TRIG_SHIFT) - ((temporary.i*sine_table[phi])>>TRIG_SHIFT);
		/* y-coordinate is unchanged */
	}
	else
	{
		point->z= temporary.k;
	}
	
	return point;
}

void build_trig_tables(
	void)
{
	short i;
	double two_pi= 8.0*atan(1.0);
	double theta;

	sine_table= (int16 *) malloc(sizeof(int16)*NUMBER_OF_ANGLES);
	cosine_table= (int16 *) malloc(sizeof(int16)*NUMBER_OF_ANGLES);
	tangent_table= (int32 *) malloc(sizeof(int32)*NUMBER_OF_ANGLES);
	fc_assert(sine_table&&cosine_table&&tangent_table);
	
	for (i=0;i<NUMBER_OF_ANGLES;++i)
	{
		theta= two_pi*(double)i/(double)NUMBER_OF_ANGLES;
		
		cosine_table[i]= (short) ((double)TRIG_MAGNITUDE*cos(theta)+0.5);
		sine_table[i]= (short) ((double)TRIG_MAGNITUDE*sin(theta)+0.5);
		
		if (i==0) sine_table[i]= 0, cosine_table[i]= TRIG_MAGNITUDE;
		if (i==QUARTER_CIRCLE) sine_table[i]= TRIG_MAGNITUDE, cosine_table[i]= 0;
		if (i==HALF_CIRCLE) sine_table[i]= 0, cosine_table[i]= -TRIG_MAGNITUDE;
		if (i==THREE_QUARTER_CIRCLE) sine_table[i]= -TRIG_MAGNITUDE, cosine_table[i]= 0;
		
		/* what we care about here is NOT accuracy, rather weÕre concerned with matching the
			ratio of the existing sine and cosine tables as exactly as possible */
		if (cosine_table[i])
		{
			tangent_table[i]= ((TRIG_MAGNITUDE*sine_table[i])/cosine_table[i]);
		}
		else
		{
			/* we always take -°, even though the limit is ±°, depending on which side you
				approach it from.  this is because of the direction we traverse the circle
				looking for matches during arctan. */
			tangent_table[i]= INT32_MIN;
		}
	}
}

static angle m2_arctangent(
	int32 xx,
	int32 yy)
{
	// the original Marathon 2 function took world_distance parameters
	world_distance x = xx;
	world_distance y = yy;

	long tangent;
	long last_difference, new_difference;
	angle search_arc, theta;
	
	if (x)
	{
		tangent= (TRIG_MAGNITUDE*y)/x;
		
		if (tangent)
		{
			theta= (y>0) ? 1 : HALF_CIRCLE+1;
			if (tangent<0) theta+= QUARTER_CIRCLE;
			
			last_difference= tangent-tangent_table[theta-1];
			for (search_arc=QUARTER_CIRCLE-1;search_arc;search_arc--,theta++)
			{
				new_difference= tangent-tangent_table[theta];
				
				if ((last_difference<=0&&new_difference>=0) || (last_difference>=0&&new_difference<=0))
				{
					if (ABS(last_difference)<ABS(new_difference))
					{
						return theta-1;
					}
					else
					{
						return theta;
					}
				}
				
				last_difference= new_difference;
			}
			
			return theta==NUMBER_OF_ANGLES ? 0 : theta;
		}
		else
		{
			return x<0 ? HALF_CIRCLE : 0;
		}
	}
	else
	{
		/* so arctan(0,0)==¹/2 (bill me) */
		return y<0 ? THREE_QUARTER_CIRCLE : QUARTER_CIRCLE;
	}
}
/* one day weÕll come back here and actually make this run fast */
// LP change: made this long-distance friendly
//
static angle a1_arctangent(
	int32 x, // world_distance x,
	int32 y) // world_distance y)
{
	int32 tangent;
	angle theta;
	
	// LP change: reworked everything in here
	
	// Intermediate transformed values
	int32 xtfm = x, ytfm = y;
	
	// Initial angle:
	theta = 0;
	
	// Reduce 2nd/3rd quadrant to 1st/4th quadrant
	if (xtfm < 0)
	{
		xtfm = -xtfm;
		ytfm = -ytfm;
		theta += HALF_CIRCLE;
	}
	
	// Reduce 4th quadrant to 1st quadrant
	if (ytfm < 0)
	{
		int32 temp = ytfm;
		ytfm = xtfm;
		xtfm = - temp;
		theta += THREE_QUARTER_CIRCLE;
	}
	
	// The 1st quadrant has two octants; which one to choose?
	bool other_octant = false;
	if (ytfm > xtfm)
	{
		// Choosing the second octant instead of the first one..
		other_octant = true;
		int32 temp = ytfm;
		ytfm = xtfm;
		xtfm = temp;
	}
	
	// Find the tangent; exit if both xtfm and ytfm are 0
	if (xtfm == 0) return 0;
	tangent= (TRIG_MAGNITUDE*ytfm)/xtfm;
	
	// Find the search endpoints and test them:
	angle dtheta = 0, dth0 = 0, dth1 = EIGHTH_CIRCLE;
	int32 tan0 = tangent_table[dth0];
	int32 tan1 = tangent_table[dth1];
	if (tangent <= tan0) dtheta = dth0;
	else if (tangent >= tan1) dtheta = dth1;
	else
	{
		// Do binary search
		bool exact_hit = false;
		while(dth1 > dth0+1)
		{
			// Divide the interval in half
			angle dthnew = (dth0 + dth1) >> 1;
			// Where's the point?
			int32 tannew = tangent_table[dthnew];
			if (tangent > tannew)
			{
				dth0 = dthnew;
				tan0 = tannew;
			}
			else if (tangent < tannew)
			{
				dth1 = dthnew;
				tan1 = tannew;
			}
			else
			{
				dtheta = dthnew;
				exact_hit = true;
				break;
			}
		}
		// If didn't hit exactly, find the closest one
		if (!exact_hit)
		{
			if ((tan1 - tangent) < (tangent - tan0))
				dtheta = dth1;
			else
				dtheta = dth0;
		}
	}
	
	// Adjust the octant and add in
	if (other_octant) dtheta = QUARTER_CIRCLE - dtheta;
	theta += dtheta;
	
	// Idiot-proofed exit
	return NORMALIZE_ANGLE(theta);
}

angle arctangent(int32 x, int32 y)
{
	if (film_profile.long_distance_physics)
	{
		return a1_arctangent(x, y);
	}
	else
	{
		return m2_arctangent(x, y);
	}
}

void set_random_seed(
	uint16 seed)
{
	random_seed= seed ? seed : DEFAULT_RANDOM_SEED;
}

uint16 get_random_seed(
	void)
{
	return random_seed;
}

uint16 global_random(
	void)
{
	uint16 seed= random_seed;
	
	if (seed&1)
	{
		seed= (seed>>1)^0xb400;
	}
	else
	{
		seed>>= 1;
	}

	return (random_seed= seed);
}

uint16 local_random(
	void)
{
	uint16 seed= local_random_seed;
	
	if (seed&1)
	{
		seed= (seed>>1)^0xb400;
	}
	else
	{
		seed>>= 1;
	}

	return (local_random_seed= seed);
}

world_distance guess_distance2d(
	world_point2d *p0,
	world_point2d *p1)
{
	int32 dx= (int32)p0->x - p1->x;
	int32 dy= (int32)p0->y - p1->y;
	int32 distance;
	
	if (dx<0) dx= -dx;
	if (dy<0) dy= -dy;
	distance= GUESS_HYPOTENUSE(dx, dy);
	
	return distance>INT16_MAX ? INT16_MAX : distance;
}

world_distance distance3d(
	world_point3d *p0,
	world_point3d *p1)
{
	int32 dx= (int32)p0->x - p1->x;
	int32 dy= (int32)p0->y - p1->y;
	int32 dz= (int32)p0->z - p1->z;
	int32 distance= isqrt(dx*dx + dy*dy + dz*dz);
	
	return distance>INT16_MAX ? INT16_MAX : distance;
}

static world_distance m2_distance2d(
        world_point2d *p0,
        world_point2d *p1)
{
        return isqrt((p0->x-p1->x)*(p0->x-p1->x)+(p0->y-p1->y)*(p0->y-p1->y));
}

static world_distance a1_distance2d(
	world_point2d *p0,
	world_point2d *p1)
{
	// LP change: lengthening the values for more precise calculations;
	// code cribbed from the previous function
	int32 dx= (int32)p0->x - p1->x;
	int32 dy= (int32)p0->y - p1->y;
	int32 distance= isqrt(dx*dx + dy*dy);
	
	return distance>INT16_MAX ? INT16_MAX : distance;
}

world_distance distance2d(
	world_point2d *p0,
	world_point2d *p1)
{
	if (film_profile.long_distance_physics)
	{
		return a1_distance2d(p0, p1);
	}
	else
	{
		return m2_distance2d(p0, p1);
	}
}

int32 isqrt(uint32 x) {
	return (int32)(sqrt((double)x) + 0.5);
}

// LP additions: stuff for handling long-distance views

void long_to_overflow_short_2d(long_vector2d& LVec, world_point2d& WVec, uint16& flags)
{
	// Clear upper byte of flags
	flags &= 0x00ff;

	// Extract upper digits and put them into place
	uint16 xupper = uint16(LVec.i >> 16) & 0x000f;
	uint16 yupper = uint16(LVec.j >> 16) & 0x000f;
	
	// Put them into place
	flags |= xupper << 12;
	flags |= yupper << 8;
	
	// Move lower values
	WVec.x = LVec.i;
	WVec.y = LVec.j;
}

void overflow_short_to_long_2d(world_point2d& WVec, uint16& flags, long_vector2d& LVec)
{
	// Move lower values
	LVec.i = int32(WVec.x) & 0x0000ffff;
	LVec.j = int32(WVec.y) & 0x0000ffff;
	
	// Extract upper digits
	uint16 xupper = (flags >> 12) & 0x000f;
	uint16 yupper = (flags >> 8) & 0x000f;
	
	// Sign-extend them
	if (xupper & 0x0008) xupper |= 0xfff0;
	if (yupper & 0x0008) yupper |= 0xfff0;
	
	// Put them into place
	LVec.i |= int32(xupper) << 16;
	LVec.j |= int32(yupper) << 16;
}


world_point2d *transform_overflow_point2d(
	world_point2d *point,
	world_point2d *origin,
	angle theta,
	uint16 *flags)
{
	// LP change: lengthening the values for more precise calculations
	long_vector2d temp, tempr;
	
	theta = normalize_angle(theta);
	fc_assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	temp.i= int32(point->x)-int32(origin->x);
	temp.j= int32(point->y)-int32(origin->y);
		
	tempr.i= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT);
	tempr.j= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT);
	
	long_to_overflow_short_2d(tempr,*point,*flags);
	
	return point;
}
