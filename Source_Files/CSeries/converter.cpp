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
	std::vector<char> text(len*2, '\0');
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
		sz = 1024;
		iconv(j,  &strp, &len, &retp, &sz);
	}
	std::string ret = &text[0];
	if( ret.back() == MAC_LINE_END) { ret.pop_back(); }
	return ret;
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

class utf8_iter {
	std::string s;
	size_t i;
public:
	utf8_iter(const std::string& s) :s(s) ,i(0) {}
	std::string utf8() {
		return s.substr(i, utf8_len(&s[i]));
	}
	char32_t code() {
		switch( utf8_len(&s[i])) {
		case 2 :
			return ((s[i] & 0x1f)<<6) | s[i+1] & 0x3f;
		case 3 :
			return ((s[i] & 0xf)<< 12) |
				((s[i+1] & 0x3f) << 6) |
				((s[i+2] & 0x3f));
		case 4 :
			return ((s[i] & 0x7)<< 18) |
				((s[i+1] & 0x3f) << 12) |
				((s[i+2] & 0x3f) << 6) |
				((s[i+3] & 0x3f));
		default :
			return s[i];
		}
	}
	bool begin() { return i == 0; }
	bool end() { return i >= s.size(); }
	utf8_iter& operator++() {
		const char* c = &s[i];
		i += utf8_len(c);
		return *this;
	}
	utf8_iter& operator--() {
		while( i > 0 &&
			   (unsigned char)s[i-1] >= 0x80 &&
			   (unsigned char)s[i-1] <0xc0 ) { --i; }
		--i;
		return *this;
	}
};
// str is UTF-8
std::vector<std::string> line_wrap(TTF_Font* t, const std::string& str, int size) {
	std::vector<std::string> ret;
	std::string now;
	utf8_iter it(str);
	int w;
	while( ! it.end() ) {
		char32_t c = it.code();
		if( c >= 0x3040 && c <= 0x9fef ||
			c >= 0x20000 && c <= 0x2ebe0 ) {
			// fetch next letter
			std::string tmp = now + it.utf8();
			TTF_SizeUTF8(t, tmp.c_str(), &w, 0);		
			if( w > size ) {
				ret.push_back(now);
				now = it.utf8();
				continue;
			}
			now = tmp;
		} else if( isspace(c) ) {
			// don't care if overflow
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
			std::string test = now + tmp;
			TTF_SizeUTF8(t, test.c_str(), &w, 0);		
			if( w > size ) {
				ret.push_back(now);
				now = tmp;
			} else {
				now = test;
			}
		}
	}
	return ret;	
}
