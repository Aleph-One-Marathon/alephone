/*
 *  converter.h
 *  AlephOne-OSX10.4
 *
 *  Created by みちあき on 08/06/24.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 *  Modified by Logue on 12/05/23
 */
#include <stdlib.h>
#include <string.h>
#include <iconv.h>


char* sjis2utf8(const char* str, size_t len);
char* utf82sjis(const char* str, size_t len);

unsigned short* sjis2utf16(const char* str, size_t len);
unsigned short* utf82utf16(const char* str, size_t len);
char* utf162utf8(const unsigned short* str, size_t len);

typedef unsigned short uint16;
uint16 sjisChar(char* in, int* step);
int unicodeChar( const char* input, uint16* ret);

bool isJChar(unsigned char text);
bool is2ndJChar(unsigned char text);
