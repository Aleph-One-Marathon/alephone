/*
 *  csdialogs_sdl.cpp - Dialog management, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"

#include <stdio.h>


/*
 *  Center rect in frame
 */

void AdjustRect(Rect const *frame, Rect const *in, Rect *out, short how)
{
	int dim;

	switch (how) {
		case centerRect:
			dim = in->right - in->left;
			out->left = (frame->left + frame->right - dim) / 2;
			out->right = out->left + dim;
			dim = in->bottom - in->top;
			out->top = (frame->top + frame->bottom - dim) / 2;
			out->bottom = out->top + dim;
			break;
	}
}
