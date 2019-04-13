/*
 *  converter.h
 *
 *  Modified by Logue on 12/05/23
 */
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <string>
#include <vector>
#include "SDL_ttf.h"

std::string sjis2utf8(const char* str, size_t len);


void sjisChar(const char* in, int* step, char* dst);
std::vector<std::string> line_wrap(TTF_Font* t, const std::string& str, int size);
const char* line_wrap_term(TTF_Font* t, const char* begin, const char* end,
						   int size);
// Detect 2-byte char. (for Shift_JIS)
inline bool isJChar(unsigned char text) {
	return (((text >= 0x81) && (text <= 0x9f)) ||
			((text >= 0xe0) && (text <= 0xfc))) ? true : false;
}

inline int utf8_len(const char* t) {
	unsigned char c = *t;
	if( c <= 0xc2 )
		return 1;
	if( c <= 0xdf )
		return 2;
	if( c <= 0xef )
		return 3;
	return 4;
}
bool isJChar(unsigned char text);
bool is2ndJChar(unsigned char text);
