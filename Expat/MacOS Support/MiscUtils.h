// Here are some misc utilities:

#pragma once


// These are from the Standard Template Library
#include <algorithm.h>

// Get the standard namespace
using namespace std;

// Min and max are now part of the Standard Template Library
#include <MacTypes.h>

// Square function (something that I miss from Fortran)
template<class T> inline T sqr(const T& x) {return x*x;}


// Min and max functions (taken from the STL, because they didn't work otherwise)
/*
template <class T>
inline const T& min (const T& a, const T& b)
{ return b < a ? b : a; }
template <class T>
inline const T& max (const T& a, const T& b)
{ return  a < b ? b : a; }
*/

// Round off to integer
// (imitation of sunmath function)
inline int irint(double x) {
	int ix;
	if (x >= 0)
		ix = int(x + 0.5);
	else
		ix = - int((-x) + 0.5);
	return ix;
}

// The first arg is the input number,
// and the second arg is the modulus base

// Positive-range modular arithmetic
inline int pos_mod(int i, int n) {
	int m = i % n;
	if (m < 0) m += n;
	return m;
}

// Quick modular increments, decrements
inline int mod_incr(int i, int n) {
	int j = i + 1;
	if (j >= n) j -= n;
	return j;
}
inline int mod_decr(int i, int n) {
	int j = i - 1;
	if (j < 0) j += n;
	return j;
}

// In these, the iteration directions are adjusted so that
// the routines can be called on the same string pointers.

// Pascal to C Strings:
inline void Pas2C(Str255 InStr, Str255 OutStr) {
	int len = int(InStr[0]);
	for (int i=0; i<len; i++)
		OutStr[i] = InStr[i+1];
	OutStr[len] = 0;
}


// C to Pascal Strings:
inline void C2Pas(Str255 InStr, Str255 OutStr, int MaxLen=255) {
	int len = MaxLen;
	int i;
	for (i=0; i<=MaxLen; i++)
		if (InStr[i] == 0) {
			len = i;
			break;
		}
	for (i=len-1; i>=0; i--)
		OutStr[i+1] = InStr[i];
	OutStr[0] = len;
}


/*
	List of interesting STL functions:
	
	Note, the iterators are pointers to array positions.
	
	From "utility.h": pair (used in "mismatch" and "equal")
	
	From "algobase.h":
	
	min
	max
	mismatch
	equal
	copy
	copy_backward
	swap
	fill
	fill_n
	lexicographical_compare
	
	From "algorithm.h":
	
	for_each
	find
	find_if
	count
	count_if
	transform
	replace
	replace_if
	generate
	generate_n
	reverse
	gcd
	rotate
	sort
	stable_sort
	binary_search
	
	From "numeric.h":
	
	accumulate
	inner_product
	partial_sum
	adjacent_difference
	
	From "functional.h":
	
	plus
	minus
	multiplies
	divides
	modulus
	negate
	equal_to
	not_equal_to
	greater
	less
	greater_equal
	less_equal
	logical_and
	logical_or
	logical_not
	ptr_fun
	
	"complex.h"
*/