/*
 *  csfonts_sdl.cpp - Font handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"


/*
 *  Get TextSpec by resid
 */

void GetNewTextSpec(TextSpec *spec, short resid, short item)
{
	printf("GetNewTextSpec resid %d, item %d\n", resid, item);
	spec->font = 0;
	spec->style = 0;
	spec->size = 0;
}


/*
 *  Get current font
 */

void GetFont(TextSpec *spec)
{
	printf("GetFont\n");
	spec->font = 0;
	spec->style = 0;
	spec->size = 0;
}


/*
 *  Set current font
 */

void SetFont(TextSpec *spec)
{
	printf("GetFont\n");
}
