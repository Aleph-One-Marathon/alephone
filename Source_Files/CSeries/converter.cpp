/*
 *  converter.cpp
 *
 *  Modified by Logue
 */

#include "converter.h"
#include <string.h>
#include <stdio.h>

#define LIBICONV_PLUG

#include <iconv.h>

// Convert from Shift_JIS to UTF8
#define MAC_LINE_END 13
std::string sjis2utf8(const char* str, size_t len) {
	if( len == 0 ) {
		return "";
	}
	size_t firstsize = len*2;
	std::string text(firstsize, '\0');
	char* strp = (char*)str;
	char* retp = &text[0];
	size_t sz = len*2;
	static iconv_t i = iconv_open("UTF-8", "SHIFT-JIS");
	if( i == iconv_t(-1) ) {
		return str;
	}
	if( iconv(i,  &strp, &len, &retp, &sz) == -1 ) {
		static iconv_t j = iconv_open("UTF-8", "MACROMAN");
		if( j == iconv_t(-1) ) {
			return str;
		}
		strp = (char*)str;
		retp = &text[0];
		sz = firstsize;
		iconv(j,  &strp, &len, &retp, &sz);
	}
	text.resize(firstsize-sz);
	if( text.back() == MAC_LINE_END) { text.resize(text.size()-1); }
	return text;
}

// Convert from Shift_JIS to Unicode

void sjisChar(const char* in, int* step, char* dst) {
	size_t len;
	if( *in == 13 ) { if( step ) *step += 1; *dst = 13; return; }
	if( step ) {
		if ( isJChar(*in) ) {
			*step += 2;
			len = 2;
		} else {
			*step += 1;
			len = 1;
		}
	}
	char* strp = (char*)in;
	char* retp = dst;
	size_t sz = 4;
	iconv_t i = iconv_open("UTF-8", "SHIFT-JIS");
	if( iconv(i,  &strp, &len, &retp, &sz) == -1 ) {
		iconv_t j = iconv_open("UTF-8", "MACROMAN");
		strp = (char*)in;
		retp = dst;
		sz = 4;
		iconv(j,  &strp, &len, &retp, &sz);
		iconv_close(j);
	}
	iconv_close(i);
}


// str is UTF-8
std::vector<std::string> line_wrap(TTF_Font* t, const std::string& str,
								   int size) {
	std::vector<std::string> ret;
	std::string now;
	utf8_iter it(str);
	int w = 0;
	while( ! it.end() ) {
		char32_t c = it.code();
		int w2;
		std::string tmp = it.utf8();
		TTF_SizeUTF8(t, tmp.c_str(), &w2, nullptr);		
		if( c >= 0x3040 && c <= 0x9fef ||
			c >= 0x20000 && c <= 0x2ebe0 ) {
			// fetch next letter
			int w2;
			if( w + w2 < size ) {
				now += tmp;
				w += w2;
			} else {
				ret.push_back(now);
				now = tmp;
				w = 0;
			}
		} else if( isspace(c) ) {
			// don't care overflow
			now += it.utf8();			
		} else {
			// fetch next word
			std::string tmp;
			while( ! it.end() ) {
				char32_t c = it.code();
				if( isspace(c) ||
					c >= 0x3040 && c <= 0x9fef ||
					c >= 0x20000 && c <= 0x2ebe0 ) {
					break;
				} else {
					tmp += it.utf8();
				}
			}
			TTF_SizeUTF8(t, tmp.c_str(), &w2, 0);		
			if( w > size ) {
				ret.push_back(now);
				now = tmp;
			} else {
				now += tmp;
			}
		}
	}
	if( ! now.empty() ) {
		ret.push_back(now);
	}
	return ret;	
}
