// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include <stddef.h>

#include <Quickdraw.h>
#include <Memory.h>

#include "cscluts.h"

extern RGBColor rgb_black={0x0000,0x0000,0x0000};
extern RGBColor rgb_white={0xFFFF,0xFFFF,0xFFFF};

extern RGBColor system_colors[NUM_SYSTEM_COLORS] =
{
	{0x2666,0x2666,0x2666},
	{0xD999,0xD999,0xD999}
};

CTabHandle build_macintosh_color_table(
	color_table *table)
{
	CTabHandle clut;
	int i,n;
	rgb_color *src;
	ColorSpec *dst;

	n=table->color_count;
	if (n<0) {
		n=0;
	} else if (n>256) {
		n=256;
	}
	clut=(CTabHandle)NewHandleClear(offsetof(ColorTable,ctTable)+n*sizeof (ColorSpec));
	if (clut) {
		(*clut)->ctSeed=GetCTSeed();
		(*clut)->ctSize=n-1;

		src=table->colors;
		dst=(*clut)->ctTable;
		for (i=0; i<n; i++) {
			dst->value=i;
			dst->rgb.red=src->red;
			dst->rgb.green=src->green;
			dst->rgb.blue=src->blue;
			src++;
			dst++;
		}
	}
	return clut;
}

void build_color_table(
	color_table *table,
	CTabHandle clut)
{
	int i,n;
	ColorSpec *src;
	rgb_color *dst;

	n=(*clut)->ctSize+1;
	if (n<0) {
		n=0;
	} else if (n>256) {
		n=256;
	}
	table->color_count=n;
	src=(*clut)->ctTable;
	dst=table->colors;
	for (i=0; i<n; i++) {
		dst->red=src->rgb.red;
		dst->green=src->rgb.green;
		dst->blue=src->rgb.blue;
		src++;
		dst++;
	}
}

