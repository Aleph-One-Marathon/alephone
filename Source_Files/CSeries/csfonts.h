// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_FONTS_
#define _CSERIES_FONTS_

#include "cstypes.h"

const int styleNormal = 0;
const int styleBold = 1;
const int styleItalic = 2;
const int styleUnderline = 4;

typedef struct TextSpec {
	int16 font;
	uint16 style;
	int16 size;
} TextSpec;

extern void GetNewTextSpec(
	TextSpec *spec,
	short resid,
	short item);

extern void GetFont(
	TextSpec *spec);
extern void SetFont(
	TextSpec *spec);

#endif

