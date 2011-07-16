/*

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
	
	Vector-operations template functions,
	by Loren Petrich,
	March 16, 2000

	This contains template 
*/

#ifndef VECTOR_OPERATIONS
#define VECTOR_OPERATIONS


// Sets V1 = V0
template <class T0, class T1> inline void VecCopy(const T0* V0, T1* V1)
{
	V1[0] = T1(V0[0]);
	V1[1] = T1(V0[1]);
	V1[2] = T1(V0[2]);
}

// Sets V2 = V0 + V1
template <class T0, class T1, class T2> inline void VecAdd(const T0* V0, const T1* V1, T2* V2)
{
	V2[0] = T2(V0[0] + V1[0]);
	V2[1] = T2(V0[1] + V1[1]);
	V2[2] = T2(V0[2] + V1[2]);
}

// Sets V2 = V0 - V1
template <class T0, class T1, class T2> inline void VecSub(const T0* V0, const T1* V1, T2* V2)
{
	V2[0] = T2(V0[0] - V1[0]);
	V2[1] = T2(V0[1] - V1[1]);
	V2[2] = T2(V0[2] - V1[2]);
}

// Sets V0 += V1
template <class T0, class T1> inline void VecAddTo(T0* V0, const T1* V1)
{
	V0[0] += V1[0];
	V0[1] += V1[1];
	V0[2] += V1[2];
}

// Sets V0 -= V1
template <class T0, class T1> inline void VecSubFrom(T0* V0, const T1* V1)
{
	V0[0] -= V1[0];
	V0[1] -= V1[1];
	V0[2] -= V1[2];
}

// Sets V1 = V0 * S
template <class T0, class TS, class T1> inline void VecScalarMult(const T0* V0, const TS& S, T1* V1)
{
	V1[0] = T1(S*V0[0]);
	V1[1] = T1(S*V0[1]);
	V1[2] = T1(S*V0[2]);
}

// Sets V1 *= S
template <class T, class TS> inline void VecScalarMultTo(T* V, const TS& S)
{
	V[0] *= S;
	V[1] *= S;
	V[2] *= S;
}

// Returns V0.V1
template <class T> inline T ScalarProd(const T* V0, const T* V1)
{return (V0[0]*V1[0] + V0[1]*V1[1] + V0[2]*V1[2]);}

// Sets V2 = V0 x V1
template <class T0, class T1, class T2> inline void VectorProd(const T0* V0, const T1* V1, T2* V2)
{
	V2[0] = T2(V0[1]*V1[2] - V0[2]*V1[1]);
	V2[1] = T2(V0[2]*V1[0] - V0[0]*V1[2]);
	V2[2] = T2(V0[0]*V1[1] - V0[1]*V1[0]);
}


#endif
