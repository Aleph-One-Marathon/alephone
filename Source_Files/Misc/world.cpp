/*
WORLD.C
Sunday, May 31, 1992 3:57:12 PM

Friday, January 15, 1993 9:59:14 AM
	added arctangent function.
Thursday, January 21, 1993 9:46:24 PM
	fixed arctangent function.  normalize_angle() could stand to be improved.
Saturday, January 23, 1993 9:46:34 PM
	fixed arctangent, hopefully for the last time.  normalize_angle() is a little faster.
Monday, January 25, 1993 3:01:47 PM
	arctangent works (tested at 0.5° increments against SANE’s tan), the only anomoly was
	apparently arctan(0)==180°.
Wednesday, January 27, 1993 3:49:04 PM
	final fix to arctangent, we swear.  recall lim(arctan(x)) as x approaches π/2 or 3π/4 is ±∞,
	depending on which side we come from.  because we didn't realize this, arctan failed in the
	case where x was very close to but slightly below π/2.  i think we’ve seen the last monster
	suddenly ‘panic’ and bolt directly into a wall.
Sunday, July 25, 1993 11:51:42 PM
	the arctan of 0/0 is now (arbitrairly) π/2 because we’re sick of assert(y) failing.
Monday, June 20, 1994 4:15:06 PM
	bug fix in translate_point3d().

Feb 10, 2000 (Loren Petrich):
	Fixed range check in translate_point3d()

Feb 14, 2000 (Loren Petrich):
	Doing some arithmetic as long values instead of short ones, so as to avoid annoying long-distance wraparound.

Feb 17, 2000 (Loren Petrich):
	Fixed arctangent() so that it gets the values into the right octants, and then does a binary search
*/

#include "cseries.h"
#include "world.h"

#include <stdlib.h>
#include <math.h>

#ifdef env68k
#pragma segment render
#endif




/* ---------- globals */

short *cosine_table;
short *sine_table;
static long *tangent_table;

static word random_seed= 0x1;
static word local_random_seed= 0x1;

/* ---------- code */

angle normalize_angle(
	angle theta)
{
	while (theta<0) theta+= NUMBER_OF_ANGLES;
	while (theta>=NUMBER_OF_ANGLES) theta-= NUMBER_OF_ANGLES;
	
	return theta;
}

/* remember this is not wholly accurate, both distance or the sine/cosine values could be
	negative, and the shift can’t make negative numbers zero; this is probably ok because
	we’ll have -1/1024th instead of zero, which is basically our margin for error anyway ... */
world_point2d *translate_point2d(
	world_point2d *point,
	world_distance distance,
	angle theta)
{
	assert(theta>=0&&theta<NUMBER_OF_ANGLES);
	assert(cosine_table[0]==TRIG_MAGNITUDE);
	
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
	
	// LP change: fixed this error check
	assert(theta>=0&&theta<NUMBER_OF_ANGLES);
	assert(phi>=0&&phi<NUMBER_OF_ANGLES);
	
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
	// world_point2d temp;
	
	assert(theta>=0&&theta<NUMBER_OF_ANGLES);
	assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	// LP change: lengthening the values for more precise calculations
	temp.i= long(point->x)-long(origin->x);
	temp.j= long(point->y)-long(origin->y);
	// temp.x= point->x-origin->x;
	// temp.y= point->y-origin->y;
	
	point->x= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT) +
		origin->x;
	point->y= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT) +
		origin->y;
	/*
	point->x= ((temp.x*cosine_table[theta])>>TRIG_SHIFT) + ((temp.y*sine_table[theta])>>TRIG_SHIFT) +
		origin->x;
	point->y= ((temp.y*cosine_table[theta])>>TRIG_SHIFT) - ((temp.x*sine_table[theta])>>TRIG_SHIFT) +
		origin->y;
	*/
	
	return point;
}

world_point2d *transform_point2d(
	world_point2d *point,
	world_point2d *origin,
	angle theta)
{
	// LP change: lengthening the values for more precise calculations
	long_vector2d temp;
	// world_point2d temp;
	
	assert(theta>=0&&theta<NUMBER_OF_ANGLES);
	assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	// LP change: lengthening the values for more precise calculations
	temp.i= long(point->x)-long(origin->x);
	temp.j= long(point->y)-long(origin->y);
	// temp.x= point->x-origin->x;
	// temp.y= point->y-origin->y;
	
	point->x= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT);
	/*
	point->x= ((temp.x*cosine_table[theta])>>TRIG_SHIFT) + ((temp.y*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temp.y*cosine_table[theta])>>TRIG_SHIFT) - ((temp.x*sine_table[theta])>>TRIG_SHIFT);
	*/
	
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
	// world_point3d temporary;
	
	temporary.i= long(point->x)-long(origin->x);
	temporary.j= long(point->y)-long(origin->y);
	temporary.k= long(point->z)-long(origin->z);
	// temporary.x= point->x-origin->x;
	// temporary.y= point->y-origin->y;
	// temporary.z= point->z-origin->z;
	
	/* do theta rotation (in x-y plane) */
	point->x= ((temporary.i*cosine_table[theta])>>TRIG_SHIFT) + ((temporary.j*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temporary.j*cosine_table[theta])>>TRIG_SHIFT) - ((temporary.i*sine_table[theta])>>TRIG_SHIFT);
	/*
	point->x= ((temporary.x*cosine_table[theta])>>TRIG_SHIFT) + ((temporary.y*sine_table[theta])>>TRIG_SHIFT);
	point->y= ((temporary.y*cosine_table[theta])>>TRIG_SHIFT) - ((temporary.x*sine_table[theta])>>TRIG_SHIFT);
	*/
	
	/* do phi rotation (in x-z plane) */
	if (phi)
	{
		temporary.i= point->x;
		// temporary.x= point->x;
		/* temporary.z is already set */
		
		point->x= ((temporary.i*cosine_table[phi])>>TRIG_SHIFT) + ((temporary.k*sine_table[phi])>>TRIG_SHIFT);
		point->z= ((temporary.k*cosine_table[phi])>>TRIG_SHIFT) - ((temporary.i*sine_table[phi])>>TRIG_SHIFT);
		/*
		point->x= ((temporary.x*cosine_table[phi])>>TRIG_SHIFT) + ((temporary.z*sine_table[phi])>>TRIG_SHIFT);
		point->z= ((temporary.z*cosine_table[phi])>>TRIG_SHIFT) - ((temporary.x*sine_table[phi])>>TRIG_SHIFT);
		*/
		/* y-coordinate is unchanged */
	}
	else
	{
		point->z= temporary.k;
		// point->z= temporary.z;
	}
	
	return point;
}

void build_trig_tables(
	void)
{
	short i;
	double two_pi= 8.0*atan(1.0);
	double theta;

	sine_table= (short *) malloc(sizeof(short)*NUMBER_OF_ANGLES);
	cosine_table= (short *) malloc(sizeof(short)*NUMBER_OF_ANGLES);
	tangent_table= (long *) malloc(sizeof(long)*NUMBER_OF_ANGLES);
	assert(sine_table&&cosine_table&&tangent_table);
	
	for (i=0;i<NUMBER_OF_ANGLES;++i)
	{
		theta= two_pi*(double)i/(double)NUMBER_OF_ANGLES;
		
		cosine_table[i]= (short) ((double)TRIG_MAGNITUDE*cos(theta)+0.5);
		sine_table[i]= (short) ((double)TRIG_MAGNITUDE*sin(theta)+0.5);
		
		if (i==0) sine_table[i]= 0, cosine_table[i]= TRIG_MAGNITUDE;
		if (i==QUARTER_CIRCLE) sine_table[i]= TRIG_MAGNITUDE, cosine_table[i]= 0;
		if (i==HALF_CIRCLE) sine_table[i]= 0, cosine_table[i]= -TRIG_MAGNITUDE;
		if (i==THREE_QUARTER_CIRCLE) sine_table[i]= -TRIG_MAGNITUDE, cosine_table[i]= 0;
		
		/* what we care about here is NOT accuracy, rather we’re concerned with matching the
			ratio of the existing sine and cosine tables as exactly as possible */
		if (cosine_table[i])
		{
			tangent_table[i]= ((TRIG_MAGNITUDE*sine_table[i])/cosine_table[i]);
		}
		else
		{
			/* we always take -∞, even though the limit is ±∞, depending on which side you
				approach it from.  this is because of the direction we traverse the circle
				looking for matches during arctan. */
			tangent_table[i]= LONG_MIN;
		}
	}

	return;
}

/* one day we’ll come back here and actually make this run fast */
// LP change: made this long-distance friendly
//
angle arctangent(
	long x, // world_distance x,
	long y) // world_distance y)
{
	long tangent;
	register long last_difference, new_difference;
	angle search_arc, theta;
	
	// LP change: reworked everything in here
	
	// Intermediate transformed values
	long xtfm = x, ytfm = y;
	
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
		long temp = ytfm;
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
		long temp = ytfm;
		ytfm = xtfm;
		xtfm = temp;
	}
	
	// Find the tangent; exit if both xtfm and ytfm are 0
	if (xtfm == 0) return 0;
	tangent= (TRIG_MAGNITUDE*ytfm)/xtfm;
	
	// Find the search endpoints and test them:
	angle dtheta, dth0 = 0, dth1 = EIGHTH_CIRCLE;
	long tan0 = tangent_table[dth0];
	long tan1 = tangent_table[dth1];
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
			long tannew = tangent_table[dthnew];
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
	/*
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
		*//* so arctan(0,0)==π/2 (bill me) *//*
		return y<0 ? THREE_QUARTER_CIRCLE : QUARTER_CIRCLE;
	}
	*/
}

void set_random_seed(
	word seed)
{
	random_seed= seed ? seed : DEFAULT_RANDOM_SEED;
	
	return;
}

word get_random_seed(
	void)
{
	return random_seed;
}

word random(
	void)
{
	word seed= random_seed;
	
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

word local_random(
	void)
{
	word seed= local_random_seed;
	
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
	long dx= (long)p0->x - p1->x;
	long dy= (long)p0->y - p1->y;
	long distance;
	
	if (dx<0) dx= -dx;
	if (dy<0) dy= -dy;
	distance= GUESS_HYPOTENUSE(dx, dy);
	
	return distance>SHORT_MAX ? SHORT_MAX : distance;
}

world_distance distance3d(
	world_point3d *p0,
	world_point3d *p1)
{
	long dx= (long)p0->x - p1->x;
	long dy= (long)p0->y - p1->y;
	long dz= (long)p0->z - p1->z;
	long distance= isqrt(dx*dx + dy*dy + dz*dz);
	
	return distance>SHORT_MAX ? SHORT_MAX : distance;
}

world_distance distance2d(
	world_point2d *p0,
	world_point2d *p1)
{
	// LP change: lengthening the values for more precise calculations;
	// code cribbed from the previous function
	long dx= (long)p0->x - p1->x;
	long dy= (long)p0->y - p1->y;
	long distance= isqrt(dx*dx + dy*dy);
	
	return distance>SHORT_MAX ? SHORT_MAX : distance;
	// return isqrt((p0->x-p1->x)*(p0->x-p1->x)+(p0->y-p1->y)*(p0->y-p1->y));
}

/*
 * It requires more space to describe this implementation of the manual
 * square root algorithm than it did to code it.  The basic idea is that
 * the square root is computed one bit at a time from the high end.  Because
 * the original number is 32 bits (unsigned), the root cannot exceed 16 bits
 * in length, so we start with the 0x8000 bit.
 *
 * Let "x" be the value whose root we desire, "t" be the square root
 * that we desire, and "s" be a bitmask.  A simple way to compute
 * the root is to set "s" to 0x8000, and loop doing the following:
 *
 *      t = 0;
 *      s = 0x8000;
 *      do {
 *              if ((t + s) * (t + s) <= x)
 *                      t += s;
 *              s >>= 1;
 *      while (s != 0);
 *
 * The primary disadvantage to this approach is the multiplication.  To
 * eliminate this, we begin simplying.  First, we observe that
 *
 *      (t + s) * (t + s) == (t * t) + (2 * t * s) + (s * s)
 *
 * Therefore, if we redefine "x" to be the original argument minus the
 * current value of (t * t), we can determine if we should add "s" to
 * the root if
 *
 *      (2 * t * s) + (s * s) <= x
 *
 * If we define a new temporary "nr", we can express this as
 *
 *      t = 0;
 *      s = 0x8000;
 *      do {
 *              nr = (2 * t * s) + (s * s);
 *              if (nr <= x) {
 *                      x -= nr;
 *                      t += s;
 *              }
 *              s >>= 1;
 *      while (s != 0);
 *
 * We can improve the performance of this by noting that "s" is always a
 * power of two, so multiplication by "s" is just a shift.  Also, because
 * "s" changes in a predictable manner (shifted right after each iteration)
 * we can precompute (0x8000 * t) and (0x8000 * 0x8000) and then adjust
 * them by shifting after each step.  First, we let "m" hold the value
 * (s * s) and adjust it after each step by shifting right twice.  We
 * also introduce "r" to hold (2 * t * s) and adjust it after each step
 * by shifting right once.  When we update "t" we must also update "r",
 * and we do so by noting that (2 * (old_t + s) * s) is the same as
 * (2 * old_t * s) + (2 * s * s).  Noting that (s * s) is "m" and that
 * (r + 2 * m) == ((r + m) + m) == (nr + m):
 *
 *      t = 0;
 *      s = 0x8000;
 *      m = 0x40000000;
 *      r = 0;
 *      do {
 *              nr = r + m;
 *              if (nr <= x) {
 *                      x -= nr;
 *                      t += s;
 *                      r = nr + m;
 *              }
 *              s >>= 1;
 *              r >>= 1;
 *              m >>= 2;
 *      } while (s != 0);
 *
 * Finally, we note that, if we were using fractional arithmetic, after
 * 16 iterations "s" would be a binary 0.5, so the value of "r" when
 * the loop terminates is (2 * t * 0.5) or "t".  Because the values in
 * "t" and "r" are identical after the loop terminates, and because we
 * do not otherwise use "t"  explicitly within the loop, we can omit it.
 * When we do so, there is no need for "s" except to terminate the loop,
 * but we observe that "m" will become zero at the same time as "s",
 * so we can use it instead.
 *
 * The result we have at this point is the floor of the square root.  If
 * we want to round to the nearest integer, we need to consider whether
 * the remainder in "x" is greater than or equal to the difference
 * between ((r + 0.5) * (r + 0.5)) and (r * r).  Noting that the former
 * quantity is (r * r + r + 0.25), we want to check if the remainder is
 * greater than or equal to (r + 0.25).  Because we are dealing with
 * integers, we can't have equality, so we round up if "x" is strictly
 * greater than "r":
 *
 *      if (x > r)
 *              r++;
 */

long isqrt(
	register unsigned long x)
{
	register unsigned long r, nr, m;

	r= 0;
	m= 0x40000000;
	
	do
	{
		nr= r + m;
		if (nr<=x)
		{
			x-= nr;
			r= nr + m;
		}
		r>>= 1;
		m>>= 2;
	}
	while (m!=0);

	if (x>r) r+= 1;
	return r;
}

#ifdef OBSOLETE
world_distance guess_distance3d(
	world_point3d *p0,
	world_point3d *p1)
{
	world_distance dx= (p0->x<p1->x) ? (p1->x-p0->x) : (p0->x-p1->x);
	world_distance dy= (p0->y<p1->y) ? (p1->y-p0->y) : (p0->y-p1->y);
	world_distance dz= (p0->z<p1->z) ? (p1->z-p0->z) : (p0->z-p1->z);

	/* max + med/4 + min/4 formula from graphics gems; we’re just taking their word for it */
	return (dx>dy) ? ((dx>dz) ? (dx+(dy>>2)+(dz>>2)) : (dz+(dx>>2)+(dy>>2))) :
		((dy>dz) ? (dy+(dx>>2)+(dz>>2)) : (dz+(dx>>2)+(dy>>2)));
}
#endif

// LP additions: stuff for handling long-distance views

void long_to_overflow_short_2d(long_vector2d& LVec, world_point2d& WVec, word& flags)
{
	// Clear upper byte of flags
	flags &= 0x00ff;

	// Extract upper digits and put them into place
	word xupper = word(LVec.i >> 16) & 0x000f;
	word yupper = word(LVec.j >> 16) & 0x000f;
	
	// Put them into place
	flags |= xupper << 12;
	flags |= yupper << 8;
	
	// Move lower values
	WVec.x = LVec.i;
	WVec.y = LVec.j;
}

void overflow_short_to_long_2d(world_point2d& WVec, word& flags, long_vector2d& LVec)
{
	// Move lower values
	LVec.i = long(WVec.x) & 0x0000ffff;
	LVec.j = long(WVec.y) & 0x0000ffff;
	
	// Extract upper digits
	word xupper = (flags >> 12) & 0x000f;
	word yupper = (flags >> 8) & 0x000f;
	
	// Sign-extend them
	if (xupper & 0x0008) xupper |= 0xfff0;
	if (yupper & 0x0008) yupper |= 0xfff0;
	
	// Put them into place
	LVec.i |= long(xupper) << 16;
	LVec.j |= long(yupper) << 16;
}


world_point2d *transform_overflow_point2d(
	world_point2d *point,
	world_point2d *origin,
	angle theta,
	word *flags)
{
	// LP change: lengthening the values for more precise calculations
	long_vector2d temp, tempr;
	// world_point2d temp;
	
	assert(theta>=0&&theta<NUMBER_OF_ANGLES);
	assert(cosine_table[0]==TRIG_MAGNITUDE);
	
	// LP change: lengthening the values for more precise calculations
	temp.i= long(point->x)-long(origin->x);
	temp.j= long(point->y)-long(origin->y);
	// temp.x= point->x-origin->x;
	// temp.y= point->y-origin->y;
		
	tempr.i= ((temp.i*cosine_table[theta])>>TRIG_SHIFT) + ((temp.j*sine_table[theta])>>TRIG_SHIFT);
	tempr.j= ((temp.j*cosine_table[theta])>>TRIG_SHIFT) - ((temp.i*sine_table[theta])>>TRIG_SHIFT);
	
	long_to_overflow_short_2d(tempr,*point,*flags);
	
	return point;
}
