/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Renders 3D-model objects;

	Created by Loren Petrich, July 18, 2001
*/

#include <string.h>
#include <stdlib.h>

#include "cseries.h"

#ifdef HAVE_OPENGL

#ifdef __WIN32__
#include <windows.h>
#endif

#include "ModelRenderer.h"


// Index and value data types for the sorter
typedef unsigned short SORT_INDEX_TYPE;
typedef GLfloat SORT_VALUE_TYPE;

// Templates are a bit slow on my system, so expanding...

inline void Swap(SORT_INDEX_TYPE& Indx1, SORT_INDEX_TYPE& Indx2)
{
	SORT_INDEX_TYPE Temp = Indx1;
	Indx1 = Indx2;
	Indx2 = Temp;
}

inline void SwapValues(SORT_VALUE_TYPE& Val1, SORT_VALUE_TYPE& Val2)
{
	SORT_VALUE_TYPE Temp = Val1;
	Val1 = Val2;
	Val2 = Temp;
}


// Be sure to set this to the array to be sorted
static SORT_VALUE_TYPE *Values;

// Returns true if V1 would come after V2 if in sorted order,
// that is, if (V1,V2) is the wrong order
inline bool WrongOrder(SORT_VALUE_TYPE V1, SORT_VALUE_TYPE V2)
{
	return V1 < V2;
}


/* Copyright (C) 1991, 1992, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

// Modified by Loren Petrich on August 30, 2001
// to inline the code for doing an index sort

/*extern void _quicksort __P ((void *const pbase, size_t total_elems,
                             size_t size, __compar_fn_t cmp));*/

// SWAP eliminated since there is an inline function that can do the job

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
struct stack_node
{
	SORT_INDEX_TYPE *lo;
	SORT_INDEX_TYPE *hi;
};

/* The next 4 #defines implement a very fast in-line stack abstraction. */
#define STACK_SIZE      (64)
#define PUSH(low, high) ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high)  ((void) (--top, (low = top->lo), (high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)

/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of MAX_INT is allocated on the
      stack.  Assuming a 32-bit integer, this needs only 32 *
      sizeof(stack_node) == 136 bits.  Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (n)
      stack size is needed (actually O(1) in this case)!  */

void GNU_IndexedQuickSort(SORT_INDEX_TYPE *pbase, SORT_INDEX_TYPE total_elems)
{
	register SORT_INDEX_TYPE *base_ptr = pbase;

	/* Allocating SIZE bytes for a pivot buffer facilitates a better
	   algorithm below since we can do comparisons directly on the pivot. */
	// LP: changed that to reading off of a local variable;
	// it can be procedure-global since that value is not "remembered"
	// when going up and down the stack.
	// pivot_buffer eliminated as superfluous; pointer now also superfluous
	SORT_VALUE_TYPE PivotValue;
	SORT_INDEX_TYPE PivotIndex;
	// max_thresh eliminated as superfluous

	if (total_elems <= 0)
		/* Avoid lossage with unsigned arithmetic below.  */
		return;

	if (total_elems > MAX_THRESH)
	{
		SORT_INDEX_TYPE *lo = base_ptr;
		SORT_INDEX_TYPE *hi = lo + (total_elems - 1);
		/* Largest size needed for 32-bit int!!! */
		stack_node stack[STACK_SIZE];
    	stack_node *top = stack + 1;

		while (STACK_NOT_EMPTY)
		{
			SORT_INDEX_TYPE *left_ptr;
			SORT_INDEX_TYPE *right_ptr;

			/* Select median value from among LO, MID, and HI. Rearrange
			   LO and HI so the three values are sorted. This lowers the
			   probability of picking a pathological pivot value and
			   skips a comparison for both the LEFT_PTR and RIGHT_PTR. */

			SORT_INDEX_TYPE *mid = lo + ((hi - lo) >> 1);
			
			// Have the index and the value swapped in parallel
			
			SORT_VALUE_TYPE LoVal = Values[*lo];
			SORT_VALUE_TYPE MidVal = Values[*mid];			
			SORT_VALUE_TYPE HiVal = Values[*hi];			
			if (WrongOrder(LoVal,MidVal))
			{
				Swap(*lo,*mid);
				SwapValues(LoVal,MidVal);
			}
				
			if (WrongOrder(MidVal,HiVal))
			{
				Swap(*mid,*hi);
				SwapValues(MidVal,HiVal);
			
				if (WrongOrder(LoVal,MidVal))
				{
					Swap(*lo,*mid);
					SwapValues(LoVal,MidVal);
				}
			}
			
			PivotIndex = *mid;
			PivotValue = MidVal;

			left_ptr  = lo + 1;
			right_ptr = hi - 1;

			/* Here's the famous "collapse the walls" section of quicksort.
			   Gotta like those tight inner loops!  They are the main reason
			   that this algorithm runs much faster than others. */
			do
			{
				while (WrongOrder(PivotValue,Values[*left_ptr]))
					left_ptr++;

				while (WrongOrder(Values[*right_ptr],PivotValue))
					right_ptr--;

				if (left_ptr < right_ptr)
			 	{
			 		Swap(*left_ptr, *right_ptr);
					left_ptr++;
			        right_ptr--;
				}
				else if (left_ptr == right_ptr)
				{
					left_ptr++;
					right_ptr--;
					break;
				}
			}
			while (left_ptr <= right_ptr);

			/* Set up pointers for next iteration.  First determine whether
			   left and right partitions are below the threshold size.  If so,
			   ignore one or both.  Otherwise, push the larger partition's
			   bounds on the stack and continue sorting the smaller one. */

			if ((right_ptr - lo) <= MAX_THRESH)
			{
				if ((hi - left_ptr) <= MAX_THRESH)
					/* Ignore both small partitions. */
					POP (lo, hi);
				else
					/* Ignore small left partition. */
					lo = left_ptr;
			}
			else if ((hi - left_ptr) <= MAX_THRESH)
				/* Ignore small right partition. */
				hi = right_ptr;
			else if ((right_ptr - lo) > (hi - left_ptr))
			{
				/* Push larger left partition indices. */
				PUSH (lo, right_ptr);
				lo = left_ptr;
			}
			else
			{
				/* Push larger right partition indices. */
				PUSH (left_ptr, hi);
				hi = right_ptr;
			}
        }
    }

	/* Once the BASE_PTR array is partially sorted by quicksort the rest
	   is completely sorted using insertion sort, since this is efficient
	   for partitions below MAX_THRESH size. BASE_PTR points to the beginning
	   of the array to sort, and END_PTR points at the very last element in
	   the array (*not* one beyond it!). */

	// "min" remonved here, since cseries contains "MIN"
	
	SORT_INDEX_TYPE *end_ptr = base_ptr + (total_elems - 1);
    SORT_INDEX_TYPE *tmp_ptr = base_ptr;
    SORT_INDEX_TYPE *base_ptr_with_thresh = base_ptr + MAX_THRESH;
    SORT_INDEX_TYPE *thresh = MIN(end_ptr, base_ptr_with_thresh);
    register SORT_INDEX_TYPE *run_ptr;

	/* Find smallest element in first threshold and place it at the
	   array's beginning.  This is the smallest array element,
	   and the operation speeds up insertion sort's inner loop. */

	SORT_VALUE_TYPE TmpValue = Values[*tmp_ptr];	
	for (run_ptr = tmp_ptr + 1; run_ptr <= thresh; run_ptr++)
	{
		SORT_VALUE_TYPE RunValue = Values[*run_ptr];
		if (WrongOrder(TmpValue,RunValue))
		{
			tmp_ptr = run_ptr;
			TmpValue = RunValue;
		}
	}

	if (tmp_ptr != base_ptr)
		Swap(*tmp_ptr,*base_ptr);
	
	/* Insertion sort, running from left-hand-side up to right-hand-side.  */
	
	run_ptr = base_ptr + 1;
	while ((++run_ptr) <= end_ptr)
	{
		SORT_VALUE_TYPE RunValue = Values[*run_ptr];
		tmp_ptr = run_ptr - 1;
		while (WrongOrder(Values[*tmp_ptr],RunValue))
			tmp_ptr--;
		
		tmp_ptr++;
		if (tmp_ptr != run_ptr)
		{
			// Former loop now executed exactly once;
			// this code pushes down the variable values
			SORT_INDEX_TYPE c = *run_ptr;
			SORT_INDEX_TYPE *hi, *lo;
			
			for (hi = lo = run_ptr; (--lo) >= tmp_ptr; hi = lo)
				*hi = *lo;
			*hi = c;
		}
	}
}

// This code was taken from URL http://www.la.unm.edu/~mbrettin/algorithms/quiksort.html
// Mark Brettin's code -- e-mail breadfan@breadfan.com

// Randomizer -- must be "remembered" across IndexedQuickSort invocations
static unsigned int RandVal = 0;	// Must be unsigned!

static void IndexedQuickSort(SORT_INDEX_TYPE *First, SORT_INDEX_TYPE *Last)
{
	// Don't sort if the range is wrong;
	// special-case the smallest values [LP addition]
	int Diff = static_cast<int>(Last - First);
	if (Diff <= 0) return;
	else if (Diff == 1)
	{
		SORT_VALUE_TYPE FirstVal = Values[*First];
		SORT_VALUE_TYPE LastVal = Values[*Last];
		if (WrongOrder(FirstVal,LastVal))
			Swap(*First,*Last);
		return;
	}
	else if (Diff == 2)
	{
		SORT_VALUE_TYPE FirstVal = Values[*First];
		SORT_INDEX_TYPE *Mid = First + 1;
		SORT_VALUE_TYPE MidVal = Values[*Mid];
		SORT_VALUE_TYPE LastVal = Values[*Last];
		
		if (WrongOrder(FirstVal,MidVal))
		{
			Swap(*First,*Mid);
			SwapValues(FirstVal,MidVal);
		}
		
		if (WrongOrder(MidVal,LastVal))
		{
			Swap(*Mid,*Last);
			SwapValues(MidVal,LastVal);
			
			if (WrongOrder(FirstVal,MidVal))
			{
				Swap(*First,*Mid);
				// Value swap is superfluous here
			}
		}
		return;
	}
	
	// Do some sort of fast pseudo-random generation of the next value of RandVal;
	RandVal = (RandVal + Diff) >> 1;
	if (RandVal > unsigned(Diff)) RandVal = Diff >> 1;
	Swap(*First,*(First+RandVal));
	
	// Choose a pivot value: the first one
	SORT_VALUE_TYPE PivotValue = Values[*First];
	
	// Sort the list into three parts:
	// less than, equal to, and greater than the pivot,
	// in that order
	SORT_INDEX_TYPE *Pivot = First;
	
	for (SORT_INDEX_TYPE *FirstUnknown = First+1; FirstUnknown <= Last; FirstUnknown++)
	{
		// Swap a value's index downward if that value is less than the pivot
		// Reversed the order so that the depth sorting can be appropriate
		if (WrongOrder(PivotValue,Values[*FirstUnknown]))
		{
			Pivot++;
			Swap(*Pivot,*FirstUnknown);
		}
	}
	
	// Put the pivot value in its proper place
	Swap(*First,*Pivot);
	
	// Now sort the parts on each side of that pivot
	IndexedQuickSort(First,Pivot-1);
	IndexedQuickSort(Pivot+1,Last);
}


void ModelRenderer::Render(Model3D& Model, ModelRenderShader *Shaders, int NumShaders,
	int NumSeparableShaders, bool Use_Z_Buffer)
{
	if (NumShaders <= 0) return;
	if (!Shaders) return;
	if (Model.Positions.empty()) return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,Model.PosBase());
	
	// Effective number of separable shaders when the Z-buffer is absent: none
	// (the Z-buffer enables separability).
	// Editing this arg is OK since it was called by value.
	if (!Use_Z_Buffer) NumSeparableShaders = 0;
	
	// Optimization: skip depth sorting, render, and quit
	// if all shaders are separable.
	if (NumSeparableShaders >= NumShaders)
	{
		for (int q=0; q<NumShaders; q++)
		{
			SetupRenderPass(Model,Shaders[q]);
			glDrawElements(GL_TRIANGLES,(GLsizei)Model.NumVI(),GL_UNSIGNED_SHORT,Model.VIBase());
		}
		return;			
	}
	
	// If some of the shaders are nonseparable, then the polygons have to be depth-sorted.
	// The separable ones will also use that depth-sorting, out of coding convenience.
	// OpenGL != PowerVR
	// (which can store polygons and depth-sort them in its hardware)
	
	// Find the centroids:
	size_t NumTriangles = Model.NumVI()/3;
	CentroidDepths.resize(NumTriangles);
	Indices.resize(NumTriangles);
	
	GLushort *VIPtr = Model.VIBase();
	for (unsigned short k=0; k<NumTriangles; k++)
	{
		GLfloat Sum[3] = {0, 0, 0};
		for (int v=0; v<3; v++)
		{
			GLfloat *Pos = &Model.Positions[3*(*VIPtr)];
			Sum[0] += Pos[0];
			Sum[1] += Pos[1];
			Sum[2] += Pos[2];
			VIPtr++;
		}
		Indices[k] = k;
		CentroidDepths[k] =
			Sum[0]*ViewDirection[0] + Sum[1]*ViewDirection[1] + Sum[2]*ViewDirection[2];
	}
	
	// Sort!
	Values = &CentroidDepths[0];
	SORT_INDEX_TYPE *IndxPtr = &Indices[0];
	// IndexedQuickSort(IndxPtr,IndxPtr + (NumTriangles - 1));
	GNU_IndexedQuickSort(IndxPtr,(SORT_INDEX_TYPE)NumTriangles);
	
	// Optimization: a single nonseparable shader can be rendered as if it was separable,
	// though it must still be depth-sorted.
	if (NumSeparableShaders == NumShaders - 1)
		NumSeparableShaders++;
	
	for (int q=0; q<NumSeparableShaders; q++)
	{
		// Need to do this only once
		if (q == 0)
		{
			SortedVertIndices.resize(Model.NumVI());
			GLushort *DestTriangle = &SortedVertIndices[0];
			for (size_t k=0; k<NumTriangles; k++)
			{
				GLushort *SourceTriangle = &Model.VertIndices[3*Indices[k]];
				// Copy-over unrolled for speed
				*(DestTriangle++) = *(SourceTriangle++);
				*(DestTriangle++) = *(SourceTriangle++);
				*(DestTriangle++) = *(SourceTriangle++);
			}
		}
		
		// Separable-shader optimization: render in one swell foop
		SetupRenderPass(Model,Shaders[q]);
				
		// Go!
		glDrawElements(GL_TRIANGLES,(GLsizei)Model.NumVI(),GL_UNSIGNED_SHORT,&SortedVertIndices[0]);
	}
	
	if (NumSeparableShaders < NumShaders)
	{
		// Multishader case: each triangle separately
		for (size_t k=0; k<NumTriangles; k++)
		{
			GLushort *Triangle = &Model.VertIndices[3*Indices[k]];
			for (int q=NumSeparableShaders; q<NumShaders; q++)
			{
				SetupRenderPass(Model,Shaders[q]);
				glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_SHORT,Triangle);
			}
		}
	}
}


void ModelRenderer::SetupRenderPass(Model3D& Model, ModelRenderShader& Shader)
{
	assert(Shader.TextureCallback);
	
	// Do textured rendering
	if (!Model.TxtrCoords.empty() && TEST_FLAG(Shader.Flags,Textured))
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,0,Model.TCBase());
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	
	// Check whether to use external lighting
	if (Shader.LightingCallback && !Model.Normals.empty() && TEST_FLAG(Shader.Flags,ExtLight))
	{
		size_t NumVerts = Model.Positions.size()/3;
		size_t NumCPlanes = TEST_FLAG(Shader.Flags,EL_SemiTpt) ? 4 : 3;
		size_t NumCValues = NumCPlanes*NumVerts;
		ExtLightColors.resize(NumCValues);
		
		Shader.LightingCallback(Shader.LightingCallbackData,
			NumVerts, Model.NormBase(),Model.PosBase(),&ExtLightColors[0]);
		
		if (!Model.Colors.empty() && TEST_FLAG(Shader.Flags,Colored))
		{
			GLfloat *ExtColorPtr = &ExtLightColors[0];
			GLfloat *ColorPtr = Model.ColBase();
			if (NumCPlanes == 3)
			{
				for (size_t k=0; k<NumCValues; k++, ExtColorPtr++, ColorPtr++)
					(*ExtColorPtr) *= (*ColorPtr);
			}
			else if (NumCPlanes == 4)
			{
				for (size_t k=0; k<NumVerts; k++)
				{
					for (int chn=0; chn<3; chn++)
					{
						(*ExtColorPtr) *= (*ColorPtr);
						ExtColorPtr++, ColorPtr++;
					}
					// Nothing happens to the alpha channel
					ExtColorPtr++;
				}
			}
		}
		
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer((GLint)NumCPlanes,GL_FLOAT,0,&ExtLightColors[0]);
	}
	else
	{
		// Do colored rendering
		if (!Model.Colors.empty() && TEST_FLAG(Shader.Flags,Colored))
		{
			// May want to recover the currently-set color and do the same kind
			// of treatment as above
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_FLOAT,0,Model.ColBase());
		}
		else
			glDisableClientState(GL_COLOR_ARRAY);
	}
	
	// Do whatever texture management is necessary
	Shader.TextureCallback(Shader.TextureCallbackData);
}


void ModelRenderer::Clear()
{
	CentroidDepths.clear();
	Indices.clear();
	SortedVertIndices.clear();
	ExtLightColors.clear();
}

#endif // def HAVE_OPENGL
