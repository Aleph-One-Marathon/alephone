/*

	Copyright (C) 1991-2001 and beyond by Bo Lindbergh
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/
#ifndef _CSERIES_KEYS_
#define _CSERIES_KEYS_

/*
	[Original author not listed]
Feb 13, 2000 (Loren Petrich):
	Added definitions for F-keys 13, 14, and 15; used Inside Macintosh
	http://developer.apple.com/techpubs/mac/Toolbox/Toolbox-40.html#HEADING40-39

Feb 24, 2000 (Loren Petrich):
	Gave new names to my new key definitions; they begin with "kx"
	instead of "k". Also added kFWD_DELETE and kHELP
*/

// LP changes:
#define kxESCAPE		0x35
#define kxDELETE		0x33
#define kxHELP			0x72
#define kxFWD_DELETE	0x75
#define kxHOME			0x73
#define kxEND			0x77
#define kxPAGE_UP		0x74
#define kxPAGE_DOWN		0x79
#define kxLEFT_ARROW	0x7B
#define kxRIGHT_ARROW	0x7C
#define kxUP_ARROW		0x7E
#define kxDOWN_ARROW	0x7D

#define kDELETE			0x08
#define kHELP			0x05
#define kFWD_DELETE		0x7F
#define kHOME			0x01
#define kEND			0x04
#define kPAGE_UP		0x0B
#define kPAGE_DOWN		0x0C
#define kLEFT_ARROW		0x1C
#define kRIGHT_ARROW	0x1D
#define kUP_ARROW		0x1E
#define kDOWN_ARROW		0x1F


#define kcF1	0x7A
#define kcF2	0x78
#define kcF3	0x63
#define kcF4	0x76
#define kcF5	0x60
#define kcF6	0x61
#define kcF7	0x62
#define kcF8	0x64
#define kcF9	0x65
#define kcF10	0x6D
#define kcF11 	0x67
#define kcF12	0x6F
// LP additions:
#define kcF13	0x69
#define kcF14	0x6B
#define kcF15	0x71

#endif
