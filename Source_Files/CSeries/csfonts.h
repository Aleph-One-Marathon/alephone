// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_FONTS_
#define _CSERIES_FONTS_

typedef struct TextSpec {
	short font;
	unsigned short style;
	short size;
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

