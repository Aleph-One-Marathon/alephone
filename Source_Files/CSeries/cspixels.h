typedef unsigned char pixel8;
typedef unsigned short pixel16;
typedef unsigned long pixel32;

#define PIXEL8_MAXIMUM_COLORS 256
#define PIXEL16_MAXIMUM_COMPONENT 31
#define PIXEL32_MAXIMUM_COMPONENT 255
#define NUMBER_OF_COLOR_COMPONENTS 3

/*
	note that the combiner macros expect input values in the range
		0x0000 through 0xFFFF
	while the extractor macros return output values in the ranges
		0x00 through 0x1F (in the 16-bit case)
		0x00 through 0xFF (in the 32-bit case)
 */

#define RGBCOLOR_TO_PIXEL16(r,g,b) ((r)>>1&0x7C00 | (g)>>6&0x03E0 | (b)>>11&0x001F)
#define RED16(p) ((p)>>10&0x1F)
#define GREEN16(p) ((p)>>5&0x1F)
#define BLUE16(p) ((p)&0x1F)

#define RGBCOLOR_TO_PIXEL32(r,g,b) ((r)<<8&0x00FF0000 | (g)&0x00000FF00 | (b)>>8&0x000000FF)
#define RED32(p) ((p)>>16&0xFF)
#define GREEN32(p) ((p)>>8&0xFF)
#define BLUE32(p) ((p)&0xFF)

