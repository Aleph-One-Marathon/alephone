/*
	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
Feb 27, 2002 (Br'fin (Jeremy Parsons))
	
	Carbon specific resource changes. This must be added as a resource
	before marathon2.resources in order to supercede them.
	Initial verson tweaks Network dialogs.
*/

#include <Carbon/Carbon.r>

resource 'DLOG' (10000, "AppleTalk Game Gather") {
	{42, 14, 298, 537},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	10000,
	"",
	alertPositionParentWindowScreen
};

resource 'DLOG' (10001, "AppleTalk Game Join") {
	{40, 39, 314, 584},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	10001,
	"",
	alertPositionParentWindowScreen
};

resource 'DLOG' (10002, "Appletalk Distribute Map") {
	{52, 130, 130, 433},
	movableDBoxProc,
	visible,
	noGoAway,
	0x0,
	10002,
	"",
	alertPositionParentWindowScreen
};

resource 'DLOG' (3000, "Net Game Setup") {
	{26, 24, 413, 597},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	3000,
	"",
	centerParentWindowScreen
};

resource 'DLOG' (2100, "OpenGL Texture Options") {
	{100, 100, 287, 400},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x0,
	2100,
	"OpenGL Texture Rendering Preferences",
	centerParentWindowScreen
};

resource 'DITL' (3000, "Net Game Setup") {
	{	/* array DITLarray: 37 elements */
		/* [1] */
		{355, 486, 375, 554},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{355, 407, 375, 475},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{69, 586, 89, 786},
		Control {
			enabled,
			135
		},
		/* [4] */
		{170, 25, 190, 357},
		Control {
			enabled,
			138
		},
		/* [5] */
		{193, 281, 213, 566},
		Control {
			enabled,
			133
		},
		/* [6] */
		{288, 21, 306, 188},
		CheckBox {
			enabled,
			"Disable Motion Sensor"
		},
		/* [7] */
		{306, 21, 324, 225},
		CheckBox {
			enabled,
			"Penalize Dying (10 seconds)"
		},
		/* [8] */
		{270, 21, 288, 203},
		CheckBox {
			enabled,
			"Dead Players Drop Items"
		},
		/* [9] */
		{97, 588, 115, 746},
		CheckBox {
			enabled,
			"Allow Realtime Audio"
		},
		/* [10] */
		{216, 21, 234, 81},
		CheckBox {
			enabled,
			"Aliens"
		},
		/* [11] */
		{252, 21, 270, 123},
		CheckBox {
			enabled,
			"Allow Teams"
		},
		/* [12] */
		{278, 322, 298, 422},
		RadioButton {
			enabled,
			"Untimed"
		},
		/* [13] */
		{298, 322, 318, 434},
		RadioButton {
			enabled,
			"Time Limit:"
		},
		/* [14] */
		{318, 322, 338, 434},
		RadioButton {
			enabled,
			"Kill Limit:"
		},
		/* [15] */
		{298, 438, 314, 474},
		EditText {
			enabled,
			"10"
		},
		/* [16] */
		{322, 438, 338, 474},
		EditText {
			enabled,
			"20"
		},
		/* [17] */
		{235, 21, 251, 194},
		CheckBox {
			enabled,
			"Live Carnage Reporting"
		},
		/* [18] */
		{322, 482, 338, 542},
		StaticText {
			disabled,
			"kills"
		},
		/* [19] */
		{266, 318, 346, 558},
		UserItem {
			disabled
		},
		/* [20] */
		{258, 326, 274, 386},
		StaticText {
			disabled,
			"Duration"
		},
		/* [21] */
		{72, 69, 88, 239},
		EditText {
			enabled,
			""
		},
		/* [22] */
		{121, 20, 141, 305},
		Control {
			enabled,
			136
		},
		/* [23] */
		{72, 21, 88, 63},
		StaticText {
			disabled,
			"Name:"
		},
		/* [24] */
		{324, 21, 342, 236},
		CheckBox {
			enabled,
			"Penalize Suicide (15 seconds)"
		},
		/* [25] */
		{193, 17, 213, 277},
		Control {
			enabled,
			141
		},
		/* [26] */
		{97, 20, 117, 305},
		Control {
			enabled,
			142
		},
		/* [27] */
		{114, 605, 130, 754},
		StaticText {
			disabled,
			"(requires microphone)"
		},
		/* [28] */
		{216, 85, 234, 190},
		CheckBox {
			enabled,
			"Use Script"
		},
		/* [29] */
		{149, 18, 165, 109},
		StaticText {
			disabled,
			"Game Options"
		},
		/* [30] */
		{59, 11, 143, 256},
		UserItem {
			disabled
		},
		/* [31] */
		{49, 19, 65, 99},
		StaticText {
			disabled,
			"Appearance"
		},
		/* [32] */
		{56, 578, 140, 801},
		UserItem {
			disabled
		},
		/* [33] */
		{48, 584, 64, 697},
		StaticText {
			disabled,
			"Network Options"
		},
		/* [34] */
		{419, 11, 469, 425},
		StaticText {
			disabled,
			"Refer to page 17 of your manual for a fu"
			"ll description of the network menu.  Cho"
			"osing an inappropriate network type may "
			"result in an unresponsive or jumpy game."
		},
		/* [35] */
		{298, 482, 314, 550},
		StaticText {
			disabled,
			"minutes"
		},
                /* [36] */
		{0, 0, 1, 1},
		StaticText {
			disabled,
			""
		},
                /* [37] */
		{216, 191, 234, 565},
		StaticText {
			disabled,
			""
		},
                /* [38] */
		{155, 11, 346, 566},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (10000, "AppleTalk Gather Game") {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{228, 453, 248, 511},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{228, 371, 248, 439},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{70, 264, 207, 502},
		UserItem {
			disabled
		},
		/* [4] */
		{228, 184, 248, 242},
		Button {
			enabled,
			"Add"
		},
		/* [5] */
		{68, 13, 218, 241},
		Control {
			enabled,
			450
		},
		/* [6] */
		{57, 256, 216, 511},
		UserItem {
			disabled
		},
		/* [7] */
		{50, 263, 66, 372},
		StaticText {
			disabled,
			"Players In Game"
		},
		/* [8] */
		{44, 9, 64, 211},
		Control {
			enabled,
			137
		},
		/* [9] */
		{49, 11, 65, 144},
		StaticText {
			disabled,
			"Players On Network"
		},
		/* [10] */
		{10, 13, 47, 440},
		Picture {
			disabled,
			8003
		}
	}
};

resource 'DITL' (10002, "AppleTalk Distribute Map") {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{54, 13, 65, 293},
		UserItem {
			disabled
		},
		/* [2] */
		{7, 10, 45, 293},
		StaticText {
			disabled,
			"Now transferring map to remote player."
		}
	}
};

resource 'DITL' (10001, "AppleTalk Game Join") {
	{	/* array DITLarray: 17 elements */
		/* [1] */
		{242, 467, 262, 525},
		Button {
			enabled,
			"Join"
		},
		/* [2] */
		{242, 387, 262, 455},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{68, 19, 205, 257},
		UserItem {
			disabled
		},
		/* [4] */
		{73, 339, 89, 509},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{120, 290, 140, 575},
		Control {
			enabled,
			136
		},
		/* [6] */
		{96, 290, 116, 575},
		Control {
			enabled,
			142
		},
		/* [7] */
		{154, 280, 221, 526},
		StaticText {
			disabled,
			"Bla bla bla.  Bla bla bla bla, bla bla. "
			" Bla bla blaÑ bla bla.  Blah, bla bla.  "
			"Bla bla, bla bla.  Bla bla bla."
		},
		/* [8] */
		{73, 291, 89, 333},
		StaticText {
			disabled,
			"Name:"
		},
		/* [9] */
		{61, 266, 213, 270},
		Control {
			disabled,
			402
		},
		/* [10] */
		{47, 18, 63, 128},
		StaticText {
			disabled,
			"Players In Game"
		},
		/* [11] */
		{48, 290, 64, 370},
		StaticText {
			disabled,
			"Appearance"
		},
		/* [12] */
		{228, 148, 233, 314},
		Control {
			enabled,
			400
		},
		/* [13] */
		{318, 8, 338, 258},
		Control {
			enabled,
			147
		},
		/* [14] */
		{221, 20, 239, 152},
		CheckBox {
			enabled,
			"Join By Address"
		},
		/* [15] */
		{246, 20, 262, 113},
		StaticText {
			disabled,
			"Remote Host:"
		},
		/* [16] */
		{246, 117, 262, 309},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{10, 13, 47, 413},
		Picture {
			disabled,
			8004
		}
	}
};

resource 'DITL' (5000, "Net Damage Stats") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{368, 454, 388, 512},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{48, 24, 69, 250},
		Control {
			enabled,
			1006
		},
		/* [3] */
		{57, 12, 352, 512},
		UserItem {
			disabled
		},
		/* [4] */
		{361, 14, 377, 443},
		StaticText {
			disabled,
			"Total Kills"
		},
		/* [5] */
		{379, 14, 395, 443},
		StaticText {
			disabled,
			"Total Deaths"
		}
	}
};

resource 'MENU' (131, "Game Types") {
	131,
	textMenuProc,
	allEnabled,
	enabled,
	"Game Types",
	{	/* array: 8 elements */
		/* [1] */
		"Every Man For Himself", noIcon, noKey, noMark, plain,
		/* [2] */
		"Co-operative Play", noIcon, noKey, noMark, plain,
		/* [3] */
		"Capture the Flag", noIcon, noKey, noMark, plain,
		/* [4] */
		"King of the Hill", noIcon, noKey, noMark, plain,
		/* [5] */
		"Kill the Man With the Ball", noIcon, noKey, noMark, plain,
		/* [6] */
		"Defense", noIcon, noKey, noMark, plain,
		/* [7] */
		"Rugby", noIcon, noKey, noMark, plain,
		/* [8] */
		"Tag", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (1000, "Team Names") {
	1000,
	textMenuProc,
	allEnabled,
	enabled,
	"Team Colors",
	{	/* array: 8 elements */
		/* [1] */
		"Slate", noIcon, noKey, noMark, plain,
		/* [2] */
		"Red", noIcon, noKey, noMark, plain,
		/* [3] */
		"Violet", noIcon, noKey, noMark, plain,
		/* [4] */
		"Yellow", noIcon, noKey, noMark, plain,
		/* [5] */
		"White", noIcon, noKey, noMark, plain,
		/* [6] */
		"Orange", noIcon, noKey, noMark, plain,
		/* [7] */
		"Blue", noIcon, noKey, noMark, plain,
		/* [8] */
		"Green", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (1001, "Entry Points") {
	1001,
	textMenuProc,
	allEnabled,
	enabled,
	"Gather- Map",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (1002, "Zone Popup") {
	1002,
	textMenuProc,
	allEnabled,
	enabled,
	"Gather- Zones",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (1003, "Network Speeds") {
	1003,
	textMenuProc,
	allEnabled,
	enabled,
	"Net Speed",
	{	/* array: 4 elements */
		/* [1] */
		"AppleTalk Remote Access", noIcon, noKey, noMark, plain,
		/* [2] */
		"LocalTalk", noIcon, noKey, noMark, plain,
		/* [3] */
		"TokenTalk", noIcon, noKey, noMark, plain,
		/* [4] */
		"Ethernet", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (1006, "Carnage View Selector") {
	1006,
	textMenuProc,
	allEnabled,
	enabled,
	"Netgame- Stats",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (1007, "Transport Layer") {
	1007,
	textMenuProc,
	allEnabled,
	enabled,
	"Title",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (2004, "Difficulties") {
	2004,
	textMenuProc,
	allEnabled,
	enabled,
	"Player- Difficulties",
	{	/* array: 5 elements */
		/* [1] */
		"Kindergarten", noIcon, noKey, noMark, plain,
		/* [2] */
		"Easy", noIcon, noKey, noMark, plain,
		/* [3] */
		"Normal", noIcon, noKey, noMark, plain,
		/* [4] */
		"Major Damage", noIcon, noKey, noMark, plain,
		/* [5] */
		"Total Carnage", noIcon, noKey, noMark, plain
	}
};

resource 'CNTL' (133, "Difficulty Level") {
	{0, 0, 20, 285},
	0,
	visible,
	131,
	2004,
	1008,
	0,
	"Difficulty Level:"
};

resource 'CNTL' (135, "Gather- Network Speeds") {
	{0, 0, 20, 200},
	0,
	visible,
	72,
	1003,
	1008,
	0,
	"Network:"
};

resource 'CNTL' (136, "Join- Team") {
	{0, 0, 20, 285},
	0,
	visible,
	131,
	1000,
	1008,
	0,
	"Team:"
};

resource 'CNTL' (137, "Gather- Zones") {
	{217, 9, 237, 211},
	0,
	visible,
	72,
	1002,
	1008,
	0,
	"Zone:"
};

resource 'CNTL' (138, "Gather- Map") {
	{150, 18, 170, 350},
	0,
	visible,
	40,
	1001,
	1008,
	0,
	"Map:"
};

resource 'CNTL' (141, "Netgame- Game Type") {
	{0, 0, 20, 260},
	0,
	visible,
	49,
	131,
	1008,
	0,
	"Game:"
};

resource 'CNTL' (142, "Join- Color") {
	{0, 0, 20, 285},
	0,
	visible,
	131,
	1000,
	1008,
	0,
	"Color:"
};

resource 'CNTL' (147, "Join- Network Game Type") {
	{0, 0, 20, 250},
	0,
	visible,
	120,
	1007,
	1008,
	0,
	"Transport Layer:"
};

resource 'CNTL' (1006, "Netgame- Stats") {
	{0, 0, 21, 226},
	0,
	visible,
	70,
	1006,
	1009,
	0,
	"Graph of:"
};

resource 'CNTL' (400, "Join - Seperator") {
	{228, 139, 233, 305},
	0,
	visible,
	0,
	0,
	144,
	0,
	""
};

resource 'CNTL' (402, "Join - VSeperator") {
	{166, 307, 318, 311},
	0,
	visible,
	0,
	0,
	144,
	0,
	""
};

resource 'CNTL' (450, "Gather - Player List") {
	{66, 13, 216, 241},
	450,
	visible,
	0,
	0,
	353,
	0,
	"CNTL"
};

resource 'ldes' (450, purgeable) {
	versionZero {
		0,
		1,
		0,
		0,
		hasVertScroll,
		noHorizScroll,
		0,
		noGrowSpace
	}
};

resource 'dlgx' (3000) {
	versionZero {
		13
	}
};

resource 'dlgx' (10000) {
	versionZero {
		13
	}
};

resource 'dlgx' (10001) {
	versionZero {
		15
	}
};

resource 'dlgx' (10002) {
	versionZero {
		13
	}
};

resource 'dlgx' (5000) {
	versionZero {
		13
	}
};

data 'PICT' (8004, "join dialog header", purgeable) {
	$"55E8 0000 0000 0023 0170 0011 02FF 0C00"            /* Uè.....#.p...ÿ.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ÿş...H...H...... */
	$"0023 0170 0000 0000 0001 000A 0000 0000"            /* .#.p.......Â.... */
	$"0023 0170 009A 0000 00FF 85C0 0000 0000"            /* .#.p.š...ÿ…À.... */
	$"0023 0170 0000 0004 0000 0000 0048 0000"            /* .#.p.........H.. */
	$"0048 0000 0010 0020 0003 0008 0000 0000"            /* .H..... ........ */
	$"0000 0000 0000 0000 0000 0000 0023 0170"            /* .............#.p */
	$"0000 0000 0023 0170 0040 01C6 E5FF 0EDB"            /* .....#.p.@.Æåÿ.Û */
	$"B9A8 978A 8784 817F 7C7B 90A9 BEDD F0FF"            /* ¹¨—Š‡„.|{©¾İğÿ */
	$"0CD1 C7C2 B8B2 AAA4 9AA2 AEBD CCDF ECFF"            /* .ÑÇÂ¸²ª¤š¢®½Ìßìÿ */
	$"0CDE C7C3 B8B8 AAAA 9E9B ACB9 C9D8 ECFF"            /* .ŞÇÃ¸¸ªª›¬¹ÉØìÿ */
	$"C2FF 0EDC B7A8 998B 8885 8280 7B7C 8EA5"            /* Âÿ.Ü·¨™‹ˆ…‚€{|¥ */
	$"B8D7 F7FF 00DB FED4 00CF FEC5 03BD B5C9"            /* ¸×÷ÿ.ÛşÔ.ÏşÅ.½µÉ */
	$"DEE5 FFEF FF04 DFD4 C7BD BFFE C900 D1FE"            /* Şåÿïÿ.ßÔÇ½¿şÉ.Ñş */
	$"D600 DCF8 FF0E D5B4 A497 8B88 8582 7D7B"            /* Ö.Üøÿ.Õ´¤—‹ˆ…‚}{ */
	$"7F91 A8BA DBF9 FF19 DFBA A99A 8B88 8582"            /* .‘¨ºÛùÿ.ßº©š‹ˆ…‚ */
	$"807C 7A8A 98A2 988D 8A85 8280 7C79 8A9A"            /* €|zŠ˜¢˜Š…‚€|yŠš */
	$"B5D7 DAFF F5FF 0EDB BAA9 9A8D 8A86 8481"            /* µ×Úÿõÿ.Ûº©šŠ†„ */
	$"7F7D 92AA BFDD F0FF 0CD1 C8C2 B9B3 ACA6"            /* .}’ª¿İğÿ.ÑÈÂ¹³¬¦ */
	$"9CA3 B0BE CDDF ECFF 0CDE C8C4 B9B9 ABAC"            /* œ£°¾Íßìÿ.ŞÈÄ¹¹«¬ */
	$"A09E ADBA C9D8 DCFF D2FF 0EDC B8A9 9B8D"            /*  ­ºÉØÜÿÒÿ.Ü¸©› */
	$"8B88 8582 7D7F 90A8 B9D7 F7FF 00DB FED5"            /* ‹ˆ…‚}.¨¹×÷ÿ.ÛşÕ */
	$"00CF FEC5 03BD B6C9 DED5 FFFF FF04 DFD5"            /* .ÏşÅ.½¶ÉŞÕÿÿÿ.ßÕ */
	$"C8BE C0FE CA00 D1FE D600 DCF8 FF0E D6B6"            /* È¾ÀşÊ.ÑşÖ.Üøÿ.Ö¶ */
	$"A699 8D8B 8884 817D 8194 A9BB DBF9 FF19"            /* ¦™‹ˆ„}”©»Ûùÿ. */
	$"DFBB AB9C 8E8B 8885 827F 7C8C 9AA4 9A90"            /* ß»«œ‹ˆ…‚.|Œš¤š */
	$"8C88 8582 7F7B 8C9C B6D7 CEFF 03DC C0B2"            /* Œˆ…‚.{Œœ¶×Îÿ.ÜÀ² */
	$"A50A 9997 9592 908E 8D9F B3C4 DEF0 FF0C"            /* ¥Â™—•’Ÿ³ÄŞğÿ. */
	$"D4CB C8C0 BAB4 AFA8 ADB8 C3D1 E0EC FF0C"            /* ÔËÈÀº´¯¨­¸ÃÑàìÿ. */
	$"DFCB C9C0 C0B4 B4AA A8B6 C1CD DBCC FFE2"            /* ßËÉÀÀ´´ª¨¶ÁÍÛÌÿâ */
	$"FF0E DDBF B2A5 9A98 9694 918E 8E9C B0C0"            /* ÿ.İ¿²¥š˜–”‘œ°À */
	$"D8F7 FF00 DCFE D700 D3FE CA03 C3BD CDDF"            /* Ø÷ÿ.Üş×.ÓşÊ.Ã½Íß */
	$"D3FF 04E0 D7CC C3C5 FECD 00D4 FED8 01DD"            /* Óÿ.à×ÌÃÅşÍ.ÔşØ.İ */
	$"FFF9 FF0E D8BD AFA4 9A98 9692 8F8E 90A0"            /* ÿùÿ.Ø½¯¤š˜–’  */
	$"B2C1 DCF9 FF19 DFC2 B3A8 9A98 9694 918E"            /* ²ÁÜùÿ.ßÂ³¨š˜–”‘ */
	$"8D99 A5AD A59C 9996 9490 8E8C 99A6 BDD8"            /* ™¥­¥œ™–”Œ™¦½Ø */
	$"EAFF 03C0 F6FF 41DB 8775 7375 7677 7778"            /* êÿ.ÀöÿAÛ‡usuvwwx */
	$"7879 4DA0 FFE1 CAA9 8D87 8687 8888 8684"            /* xyM ÿáÊ©‡†‡ˆˆ†„ */
	$"8381 7662 4E47 4A81 C3FF DB84 7373 7576"            /* ƒvbNGJÃÿÛ„ssuv */
	$"7777 7878 723C CAFF 9B78 7575 7271 6F6E"            /* wwxxr<Êÿ›xuurqon */
	$"6861 5D59 5364 84A5 CAF0 FF11 C57D 7573"            /* ha]YSd„¥Êğÿ.Å}us */
	$"7372 706F 6D63 5D59 5558 7596 B8DD FCFF"            /* srpomc]YUXu–¸İüÿ */
	$"01A9 7AFE 73FF 7503 7677 7778 FE78 FE79"            /* .©zşsÿu.vwwxşxşy */
	$"0077 FD73 FF75 0076 FE77 FC78 FE79 056C"            /* .wısÿu.vşwüxşy.l */
	$"7173 7576 77FE 7807 763F 7275 7376 7677"            /* qsuvwşx.v?rusvvw */
	$"FE78 1379 4F68 7373 7575 7777 7878 7647"            /* şx.yOhssuuwwxxvG */
	$"A0FF FFD2 B28F 86FE 870B 8887 8583 8073"            /*  ÿÿÒ²†ş‡.ˆ‡…ƒ€s */
	$"6450 4846 73BF FEFF 02C8 7F67 FE68 1767"            /* dPHFs¿şÿ.È.gşh.g */
	$"6666 6564 6363 615E 5952 6173 A3DD FFFF"            /* ffedcca^YRas£İÿÿ */
	$"A97A 7373 7577 77FE 7807 6743 8273 7375"            /* ©zssuwwşx.gC‚ssu */
	$"7677 FE78 0066 004F F3FF 06DF C09B 7F7C"            /* vwşx.f.Oóÿ.ßÀ›.| */
	$"7871 FE70 FD6F FE6E 186D 663C B4FF E2CE"            /* xqşpıoşn.mf<´ÿâÎ */
	$"A98A 8586 8787 8887 8582 8072 614F 4847"            /* ©Š…†‡‡ˆ‡…‚€raOHG */
	$"87CE FDFF 21D6 B792 8786 8787 8A87 8685"            /* ‡Îıÿ!Ö·’‡†‡‡Š‡†… */
	$"827C 7876 7C83 868A 8785 8381 7A68 584A"            /* ‚|xv|ƒ†Š‡…ƒzhXJ */
	$"4673 B5E1 FF9E 78FE 73FF 7502 7677 77FD"            /* Fsµáÿxşsÿu.vwwı */
	$"78FE 7901 5E85 F6FF 04DB 8A78 7778 0079"            /* xşy.^…öÿ.ÛŠxwx.y */
	$"FE7A FF7B 084F A1FF E1CB AB8F 8A88 FE8B"            /* şzÿ{.O¡ÿáË«Šˆş‹ */
	$"118A 8785 8379 6552 4C4F 84C3 FFDB 8677"            /* .Š‡…ƒyeRLO„ÃÿÛ†w */
	$"7778 78FE 7A15 7B76 3ECB FF9C 7B77 7776"            /* wxxşz.{v>Ëÿœ{wwv */
	$"7572 716C 6461 5C58 6786 A8CB F0FF 11C7"            /* urqlda\Xg†¨Ëğÿ.Ç */
	$"8177 7776 7673 7270 6661 5C59 5C78 98B9"            /* wwvvsrpfa\Y\x˜¹ */
	$"DDFC FF08 AB7D 7676 7777 7879 79FE 7AFD"            /* İüÿ.«}vvwwxyyşzı */
	$"7B03 7C7A 7776 FE77 0378 7979 7AFE 7AFC"            /* {.|zwvşw.xyyzşzü */
	$"7B05 7C6F 7577 7879 FE7A 077B 7843 7678"            /* {.|ouwxyşz.{xCvx */
	$"7778 78FE 7AFF 7B21 526B 7777 7878 797A"            /* wxxşzÿ{!Rkwwxxyz */
	$"7A7B 7849 A1FF FFD2 B391 888A 8A8B 8C8A"            /* z{xI¡ÿÿÒ³‘ˆŠŠ‹ŒŠ */
	$"8786 8377 6654 4D4A 77C0 FEFF 02C9 816B"            /* ‡†ƒwfTMJwÀşÿ.Ék */
	$"FE6C 276B 6969 6867 6666 6462 5C57 6576"            /* şl'kiihgffdb\Wev */
	$"A4DD FFFF AB7C 7677 7879 7A7A 7B7B 6945"            /* ¤İÿÿ«|vwxyzz{{iE */
	$"8577 7778 797A 7A7B 7B69 51F3 FF00 DF05"            /* …wwxyzz{{iQóÿ.ß. */
	$"C09E 8280 7B75 FE73 FD72 FE71 0A70 693F"            /* À‚€{uşsırşqÂpi? */
	$"B5FF E2CF AB8C 878A FE8B 0A8A 8785 8276"            /* µÿâÏ«Œ‡Šş‹ÂŠ‡…‚v */
	$"6453 4D4C 8ACF FDFF 22D7 B895 8A88 8B8B"            /* dSMLŠÏıÿ"×¸•Šˆ‹‹ */
	$"8C8A 8887 8580 7A79 8086 8A8C 8A87 8684"            /* ŒŠˆ‡…€zy€†ŠŒŠ‡†„ */
	$"7D6C 5B4F 4A77 B6E1 FF9F 7A76 FE77 0178"            /* }l[OJw¶áÿŸzvşw.x */
	$"79FD 7AFC 7B01 6286 F6FF 14DC 9787 8788"            /* yızü{.b†öÿ.Ü—‡‡ˆ */
	$"888A 8A8B 8B8C 5DA6 FFE1 CEB3 9C97 9698"            /* ˆŠŠ‹‹Œ]¦ÿáÎ³œ—–˜ */
	$"FF98 0D97 9594 9188 7968 6264 91C8 FFDC"            /* ÿ˜.—•”‘ˆyhbd‘ÈÿÜ */
	$"94FE 8719 888A 8A8B 8B85 4ACE FFA8 8B87"            /* ”ş‡.ˆŠŠ‹‹…JÎÿ¨‹‡ */
	$"8786 8583 827D 7875 716D 7A95 B0CE F0FF"            /* ‡†…ƒ‚}xuqmz•°Îğÿ */
	$"11CB 8F87 8786 8684 8381 7975 716E 7087"            /* .Ë‡‡††„ƒyuqnp‡ */
	$"A3C0 DEFC FF09 B48D 8686 8787 8888 8A8A"            /* £ÀŞüÿÆ´††‡‡ˆˆŠŠ */
	$"FD8B FE8C 028A 8786 FE87 FF88 FE8A FC8B"            /* ı‹şŒ.Š‡†ş‡ÿˆşŠü‹ */
	$"FE8C 0681 8587 8788 8A8B FF8B 0788 4F85"            /* şŒ.…‡‡ˆŠ‹ÿ‹.ˆO… */
	$"8887 8888 8AFE 8B02 8C5F 79FE 870C 888A"            /* ˆ‡ˆˆŠş‹.Œ_yş‡.ˆŠ */
	$"8A8B 8B88 57A5 FFFF D4BA 9EFD 970B 9997"            /* Š‹‹ˆW¥ÿÿÔºı—.™— */
	$"9694 9187 7A6B 645F 86C5 FEFF 01CC 8FFC"            /* –”‘‡zkd_†Åşÿ.Ìü */
	$"7DFF 7C14 7B7A 7979 7776 716C 7885 ADDE"            /* }ÿ|.{zyywvqlx…­Ş */
	$"FFFF B48C 8687 888A 8AFE 8B07 7953 9287"            /* ÿÿ´Œ†‡ˆŠŠş‹.yS’‡ */
	$"8788 888A FE8B 0179 5DF3 FF06 E0C5 A890"            /* ‡ˆˆŠş‹.y]óÿ.àÅ¨ */
	$"8E8B 85FE 84FA 830A 827C 4CBB FFE2 D2B3"            /* ‹…ş„úƒÂ‚|L»ÿâÒ³ */
	$"9996 97FE 980A 9796 9290 8578 6963 6196"            /* ™–—ş˜Â—–’…xica– */
	$"D2FD FF22 D8BF A197 9697 9899 9896 9694"            /* Òıÿ"Ø¿¡—–—˜™˜––” */
	$"8E8B 8A8F 9497 9997 9694 918C 7D70 655F"            /* ‹Š”—™—–”‘Œ}pe_ */
	$"86BD E1FF AA8B 86FE 87FF 88FF 8AFD 8BFE"            /* †½áÿª‹†ş‡ÿˆÿŠı‹ş */
	$"8C01 708C 0278 F6FF 02CF 8E95 FA9C 0867"            /* Œ.pŒ.xöÿ.Ï•úœ.g */
	$"0095 DDAB 9B9A 9A9F F7A2 089E 875D 3935"            /* .•İ«›ššŸ÷¢.‡]95 */
	$"6FD6 8B96 FA9C 0542 00C8 FF9E 8EFA A20A"            /* oÖ‹–úœ.B.Èÿú¢Â */
	$"A098 8F82 6E57 4742 4F94 DBF3 FF01 C58B"            /*  ˜‚nWGBO”Ûóÿ.Å‹ */
	$"FAA2 0AA1 9A91 8878 634C 4446 8AD3 FEFF"            /* ú¢Â¡š‘ˆxcLDFŠÓşÿ */
	$"01A8 8BF8 9CFB 9C00 48EF 9C01 228F FA9C"            /* .¨‹øœûœ.Hïœ."úœ */
	$"0353 0051 95FA 9C03 7700 4D97 FA9C 0866"            /* .S.Q•úœ.w.M—úœ.f */
	$"0096 E2B7 9C9A 9B9F F7A2 0AA0 8F67 3F36"            /* .–â·œš›Ÿ÷¢Â g?6 */
	$"80FF FFC5 8B97 F99C FD9B 098F 826F 5143"            /* €ÿÿÅ‹—ùœı›Æ‚oQC */
	$"519A FFA8 8BF9 9C03 1D00 8F9B FA9C 0025"            /* Qšÿ¨‹ùœ...›úœ.% */
	$"002E F4FF 06C4 9C92 9195 999B F59C 0843"            /* ..ôÿ.Äœ’‘•™›õœ.C */
	$"00AD E0B0 9C9B 9B9F F7A2 0C9F 8C63 3B38"            /* .­à°œ››Ÿ÷¢.ŸŒc;8 */
	$"95FF FFC0 9C9B 9A9E EBA2 0792 693F 3654"            /* •ÿÿÀœ›šë¢.’i?6T */
	$"CC9F 8DF2 9C01 0070 F6FF 04D1 9097 9F9F"            /* ÌŸòœ..pöÿ.Ñ—ŸŸ */
	$"FC9F 0869 0096 DEAD 9E9C 9CA1 F7A4 08A0"            /* üŸ.i.–Ş­œœ¡÷¤.  */
	$"8B61 3D39 71D7 8D98 FA9F 0544 00C9 FFA0"            /* ‹a=9q×˜úŸ.D.Éÿ  */
	$"90FA A40A A29A 9184 725B 4C46 5296 DBF3"            /* ú¤Â¢š‘„r[LFR–Ûó */
	$"FF01 C58D FAA4 0AA3 9C95 8B7B 6650 484A"            /* ÿ.Åú¤Â£œ•‹{fPHJ */
	$"8CD3 FEFF 01A9 8DF2 9F00 4AF8 9FF8 9F01"            /* ŒÓşÿ.©òŸ.JøŸøŸ. */
	$"2592 FA9F 0355 0053 97FA 9F03 7900 4F99"            /* %’úŸ.U.S—úŸ.y.O™ */
	$"FA9F 0868 0097 E2B8 9F9C 9EA1 F7A4 0AA2"            /* úŸ.h.—â¸Ÿœ¡÷¤Â¢ */
	$"916B 433A 82FF FFC7 8D99 F99F FD9E 0992"            /* ‘kC:‚ÿÿÇ™ùŸıÆ’ */
	$"8572 5547 549B FFAA 8DF9 9F03 1F01 919E"            /* …rUGT›ÿªùŸ...‘ */
	$"FA9F 0127 30F4 FF01 C59F 0495 9497 9B9E"            /* úŸ.'0ôÿ.ÅŸ.•”—› */
	$"F59F 0845 00AE E0B3 9F9E 9EA1 F7A4 0CA1"            /* õŸ.E.®à³Ÿ¡÷¤.¡ */
	$"8E66 3F3C 96FF FFC1 9F9E 9CA0 EBA4 0795"            /* f?<–ÿÿÁŸœ ë¤.• */
	$"6D43 3A58 CDA0 8FF2 9F01 0071 F6FF 02D3"            /* mC:XÍ òŸ..qöÿ.Ó */
	$"9CA2 FAA9 0A73 079B DEB5 A9A8 A6AB ADAD"            /* œ¢ú©Âs.›Şµ©¨¦«­­ */
	$"F9AD 08AA 9873 514D 80D8 9AA3 FAA9 0550"            /* ù­.ª˜sQM€Øš£ú©.P */
	$"07CC FFAA 9CFA AD0A ACA5 9F94 836F 625B"            /* .Ìÿªœú­Â¬¥Ÿ”ƒob[ */
	$"65A0 DCF3 FF01 CA9A F9AD 09A8 A198 8B7A"            /* e Üóÿ.Êšù­Æ¨¡˜‹z */
	$"665E 5E97 D5FE FF01 B29A F2A9 0059 EFA9"            /* f^^—Õşÿ.²šò©.Yï© */
	$"0132 9EFC A9FF A903 6307 61A2 FAA9 0386"            /* .2ü©ÿ©.c.a¢ú©.† */
	$"075B A3FA A908 7207 99E2 BEA9 A8A8 ABF7"            /* .[£ú©.r.™â¾©¨¨«÷ */
	$"AD0B AC9E 7C58 4E8E FFFF CB9A A4AA F7A9"            /* ­.¬|XNÿÿËš¤ª÷© */
	$"0AA8 9F94 8469 5C67 A5FF B39A F9A9 022C"            /* Â¨Ÿ”„i\g¥ÿ³šù©., */
	$"0A9E F9A9 0131 3AF4 FF06 CAA9 A1A0 A3A6"            /* Âù©.1:ôÿ.Ê©¡ £¦ */
	$"A8F6 A909 A951 07B5 E0B9 A9A8 A8AB F7AD"            /* ¨ö©Æ©Q.µà¹©¨¨«÷­ */
	$"0CAB 9B78 5350 9FFF FFC5 A9A8 A8AA EBAD"            /* .«›xSPŸÿÿÅ©¨¨ªë­ */
	$"07A1 7F58 4E68 D1AA 9CF2 A901 0776 01F7"            /* .¡.XNhÑªœò©..v.÷ */
	$"F6FF 02CF 8E96 FA9F 0567 0076 85A0 A0F1"            /* öÿ.Ï–úŸ.g.v…  ñ */
	$"9F05 8E2E 1947 8B98 FA9F 0543 00C8 FF9E"            /* Ÿ...G‹˜úŸ.C.Èÿ */
	$"8EF4 9F05 9678 4932 3E97 F4FF 01C5 8BF4"            /* ôŸ.–xI2>—ôÿ.Å‹ô */
	$"9F09 9B88 5734 3069 CEFF A88B F89F FB9F"            /* ŸÆ›ˆW40iÎÿ¨‹øŸûŸ */
	$"0043 EF9F 011F 85FA 9F03 5500 2996 FA9F"            /* .CïŸ..…úŸ.U.)–úŸ */
	$"0379 002C 98FA 9F05 6700 8379 9FA2 F19F"            /* .y.,˜úŸ.g.ƒyŸ¢ñŸ */
	$"0798 3817 57E0 C58B 98F1 9F05 813C 286B"            /* .˜8.WàÅ‹˜ñŸ.<(k */
	$"A48B F99F 021F 0070 F99F 0025 0017 F6FF"            /* ¤‹ùŸ...pùŸ.%..öÿ */
	$"04E1 B69C 9E9E F19F 0543 008F 7CA0 A0F1"            /* .á¶œñŸ.C.|  ñ */
	$"9F06 8424 2076 B49F A2E6 9F05 9842 1F36"            /* Ÿ.„$ v´Ÿ¢æŸ.˜B.6 */
	$"8C8E F29F 0100 70F6 FF04 D190 98A1 A1FC"            /* ŒòŸ..pöÿ.Ñ˜¡¡ü */
	$"A105 6B00 7787 A2A3 F1A1 0591 311C 498D"            /* ¡.k.w‡¢£ñ¡.‘1.I */
	$"9AFA A105 4500 C9FF A090 F4A1 0599 7B4D"            /* šú¡.E.Éÿ ô¡.™{M */
	$"3542 99F4 FF01 C58D F4A1 099F 8C5A 3833"            /* 5B™ôÿ.Åô¡ÆŸŒZ83 */
	$"6CCF FFA9 8DF2 A100 45F8 A1F8 A101 2188"            /* lÏÿ©ò¡.Eø¡ø¡.!ˆ */
	$"FAA1 0358 002B 98FA A103 7B00 309B FAA1"            /* ú¡.X.+˜ú¡.{.0›ú¡ */
	$"056B 0085 7BA2 A4F1 A107 9B3B 1B58 E0C5"            /* .k.…{¢¤ñ¡.›;.XàÅ */
	$"8D9A F1A1 0584 3F2B 6DA6 8DF9 A102 2100"            /* šñ¡.„?+m¦ù¡.!. */
	$"72F9 A101 271B F6FF 03E1 B8A0 A1F0 A105"            /* rù¡.'.öÿ.á¸ ¡ğ¡. */
	$"4500 907F A2A2 F1A1 0688 2724 78B5 A1A4"            /* E..¢¢ñ¡.ˆ'$xµ¡¤ */
	$"E6A1 059B 4522 3A8E 90F2 A101 0071 F6FF"            /* æ¡.›E":ò¡..qöÿ */
	$"02D3 9CA3 FAAB 0573 0780 92AB ACFC ABF6"            /* .Óœ£ú«.s.€’«¬ü«ö */
	$"AB05 9B40 2955 9AA5 FAAB 0551 07CC FFAA"            /* «.›@)Uš¥ú«.Q.Ìÿª */
	$"9CF4 AB05 A38A 6149 51A1 F4FF 01CA 9AF4"            /* œô«.£ŠaIQ¡ôÿ.Êšô */
	$"AB09 A898 6D4A 4577 D2FF B29A F2AB 0051"            /* «Æ¨˜mJEwÒÿ²šò«.Q */
	$"EFAB 012E 96FC ABFF AB03 6407 38A3 FAAB"            /* ï«..–ü«ÿ«.d.8£ú« */
	$"0387 073B A5FA AB05 7307 8A86 ABAD F1AB"            /* .‡.;¥ú«.s.Š†«­ñ« */
	$"07A5 4928 63E0 CA9A A5F1 AB05 9150 3C7A"            /* .¥I(càÊš¥ñ«.‘P<z */
	$"AF9A F9AB 022E 0780 F9AB 0130 27F6 FF04"            /* ¯šù«...€ù«.0'öÿ. */
	$"E1BE A9AA AAF2 AB06 AB51 079A 8BAB ACF1"            /* á¾©ªªò«.«Q.š‹«¬ñ */
	$"AB06 9538 3383 BDAB ADE6 AB05 A553 3248"            /* «.•83ƒ½«­æ«.¥S2H */
	$"9A9C F2AB 0107 7602 70F6 FF02 CF8E 97FA"            /* šœò«..v.pöÿ.Ï—ú */
	$"9B04 6600 1B69 9FFA 9B03 9665 5D82 FA9B"            /* ›.f..iŸú›.–e]‚ú› */
	$"0490 0500 7296 FA9B 0542 00C8 FF8F 8EF1"            /* ...r–ú›.B.Èÿñ */
	$"9B04 8445 225A D1F6 FF01 C48B F19B 068A"            /* ›.„E"ZÑöÿ.Ä‹ñ›.Š */
	$"5925 329C A88B F99B 0070 FB67 002C FD67"            /* Y%2œ¨‹ù›.pûg.,ıg */
	$"006F F99B FC67 0113 85FA 9B03 5C00 1A96"            /* .où›üg..…ú›.\..– */
	$"FA9B 0377 0020 98FA 9B05 6600 1945 9C9E"            /* ú›.w. ˜ú›.f..Eœ */
	$"FA9B 0271 5980 FA9B 0686 0500 ADC5 8B96"            /* ú›.qY€ú›.†..­Å‹– */
	$"FA9B 018B 85F9 9B04 8C16 054A 8BF9 9B03"            /* ú›.‹…ù›.Œ..J‹ù›. */
	$"1F00 639C FA9B 0024 0012 F6FF 02AE 9AA1"            /* ..cœú›.$..öÿ.®š¡ */
	$"F89B 0188 8FFA 9B03 4200 2954 F99B 0399"            /* ø›.ˆú›.B.)Tù›.™ */
	$"6D61 8AFA 9B05 6E03 0183 999E FA9B 0296"            /* maŠú›.n..ƒ™ú›.– */
	$"8190 F99B 0292 8190 FA9B 0498 1F01 478E"            /* ù›.’ú›.˜..G */
	$"F99B FA67 0100 70F6 FF04 D190 999F 9FFC"            /* ù›úg..pöÿ.Ñ™ŸŸü */
	$"9F04 6800 1D6C A0FA 9F03 9868 6184 FA9F"            /* Ÿ.h..l úŸ.˜ha„úŸ */
	$"0492 0702 7698 FA9F 0545 00C9 FF91 90F1"            /* .’..v˜úŸ.E.Éÿ‘ñ */
	$"9F04 8648 265C D1F6 FF01 C58D F19F 068C"            /* Ÿ.†H&\Ñöÿ.ÅñŸ.Œ */
	$"5C28 359F A98D F99F 0073 FB69 002F FD69"            /* \(5Ÿ©ùŸ.sûi./ıi */
	$"0072 FD9F FD9F FC69 0115 87FA 9F03 5F00"            /* .rıŸıŸüi..‡úŸ._. */
	$"1C98 FA9F 037A 0022 9AFA 9F05 6800 1C48"            /* .˜úŸ.z."šúŸ.h..H */
	$"9FA0 FA9F 0275 5C82 FA9F 068A 0701 AEC5"            /* Ÿ úŸ.u\‚úŸ.Š..®Å */
	$"8D98 FA9F 018D 88F9 9F04 8E19 074D 8DF9"            /* ˜úŸ.ˆùŸ...Mù */
	$"9F02 2100 65F9 9F01 2615 F6FF 03AF 9EA2"            /* Ÿ.!.eùŸ.&.öÿ.¯¢ */
	$"9FF9 9F01 8C91 FA9F 0445 002C 589E FA9F"            /* ŸùŸ.Œ‘úŸ.E.,XúŸ */
	$"039B 6F63 8CFA 9F05 7006 0385 9BA0 FA9F"            /* .›ocŒúŸ.p..…› úŸ */
	$"0298 8494 F99F 0296 8492 FA9F 049B 2103"            /* .˜„”ùŸ.–„’úŸ.›!. */
	$"4990 F99F FA69 0100 71F6 FF02 D39C A4FA"            /* IùŸúi..qöÿ.Óœ¤ú */
	$"A904 7207 2877 AAFB A904 A9A3 726C 8FFA"            /* ©.r.(wªû©.©£rlú */
	$"A904 9E11 0C83 A3FA A905 5107 CCFF 9E9C"            /* ©...ƒ£ú©.Q.Ìÿœ */
	$"F1A9 0492 5835 69D3 F6FF 01CA 9AF1 A906"            /* ñ©.’X5iÓöÿ.Êšñ©. */
	$"986B 3944 A5B2 9AF9 A900 80FB 7600 3AFD"            /* ˜k9D¥²šù©.€ûv.:ı */
	$"7600 80F9 A9FC 7601 2196 FCA9 FFA9 036B"            /* v.€ù©üv.!–ü©ÿ©.k */
	$"0729 A3FA A903 8607 2CA4 FAA9 0472 0728"            /* .)£ú©.†.,¤ú©.r.( */
	$"54AA F9A9 0280 678D FAA9 0696 110B B5CA"            /* Tªù©.€gú©.–..µÊ */
	$"9AA3 FAA9 0197 95F9 A904 9924 125B 9AF9"            /* š£ú©.—•ù©.™$.[šù */
	$"A902 2E07 6FF9 A901 3022 F6FF 02B7 A8AC"            /* ©...où©.0"öÿ.·¨¬ */
	$"F8A9 0197 9BFB A905 A951 073E 64A8 FAA9"            /* ø©.—›û©.©Q.>d¨ú© */
	$"03A6 7B6E 97FA A905 7D10 0D92 A5AA FAA9"            /* .¦{n—ú©.}..’¥ªú© */
	$"02A3 8F9F F9A9 02A0 8F9E FAA9 04A5 2F0C"            /* .£Ÿù©. ú©.¥/. */
	$"579E F9A9 FA76 0107 7602 8BF6 FF02 CF8E"            /* Wù©úv..v.‹öÿ.Ï */
	$"96FA 9804 6300 0D63 97FA 9803 7500 005C"            /* –ú˜.c..c—ú˜.u..\ */
	$"F998 FF00 0155 94FA 9805 4000 C8FF 6C8E"            /* ù˜ÿ..U”ú˜.@.Èÿl */
	$"F998 015B 81FA 9804 954D 1D21 C9F7 FF01"            /* ù˜.[ú˜.•M.!É÷ÿ. */
	$"C48B F998 0164 73F9 9804 7728 175A 8BF9"            /* Ä‹ù˜.dsù˜.w(.Z‹ù */
	$"9800 1600 27F8 5F01 113F F998 0600 115F"            /* ˜...'ø_..?ù˜..._ */
	$"5F32 008E FA98 0363 001A 94FA 9803 7A00"            /* _2.ú˜.c..”ú˜.z. */
	$"2095 FA98 0463 0012 3E99 F998 FF00 0050"            /*  •ú˜.c..>™ù˜ÿ..P */
	$"FA98 068C 0000 ADC4 8B94 FA98 0353 0013"            /* ú˜.Œ..­Ä‹”ú˜.S.. */
	$"94FA 9803 2F00 1C8C F998 021F 0070 F998"            /* ”ú˜./..Œù˜...pù˜ */
	$"0022 000A F6FF 02A5 9697 F998 0229 0079"            /* .".Âöÿ.¥–—ù˜.).y */
	$"FA98 0440 0029 4D95 FA98 0387 0000 79FA"            /* ú˜.@.)M•ú˜.‡..yú */
	$"9804 7500 0055 94F9 9802 7500 68F9 9802"            /* ˜.u..U”ù˜.u.hù˜. */
	$"6300 71F9 9803 1F00 1F8E F998 0100 32FB"            /* c.qù˜....ù˜..2û */
	$"5F00 A2F6 FF04 CF90 989A 9AFC 9A04 6500"            /* _.¢öÿ.Ï˜ššüš.e. */
	$"1065 99FA 9A03 7700 005F F99A FF00 0159"            /* .e™úš.w.._ùšÿ..Y */
	$"96FA 9A05 4300 C9FF 6F90 F99A 015E 83FA"            /* –úš.C.Éÿoùš.^ƒú */
	$"9A04 9750 1F25 C9F7 FF01 C58D F99A 0166"            /* š.—P.%É÷ÿ.Åùš.f */
	$"76F9 9A04 7A2B 1A5C 8DF9 9A01 1A29 F862"            /* vùš.z+.\ùš..)øb */
	$"0113 42FD 9AFD 9A06 0013 6262 3401 90FA"            /* ..Bıšıš...bb4.ú */
	$"9A03 6500 1C96 FA9A 037D 0022 97FA 9A04"            /* š.e..–úš.}."—úš. */
	$"6500 1542 9BF9 9AFF 0000 52FA 9A06 8E00"            /* e..B›ùšÿ..Rúš.. */
	$"00AE C58D 96FA 9A03 5500 1596 FA9A 0331"            /* .®Å–úš.U..–úš.1 */
	$"001E 8EF9 9A02 2100 72F9 9A01 250D F6FF"            /* ..ùš.!.rùš.%.öÿ */
	$"03A8 9899 9AFA 9A02 2C00 7CFA 9A04 4300"            /* .¨˜™šúš.,.|úš.C. */
	$"2C4F 97FA 9A03 8A00 007C FA9A 0477 0000"            /* ,O—úš.Š..|úš.w.. */
	$"5896 F99A 0277 006B F99A 0265 0073 F99A"            /* X–ùš.w.kùš.e.sùš */
	$"0321 0021 90F9 9A01 0034 FB62 00A3 F6FF"            /* .!.!ùš..4ûb.£öÿ */
	$"02D3 9CA3 FAA4 036F 071C 6FFA A404 A484"            /* .Óœ£ú¤.o..oú¤.¤„ */
	$"0707 6BF9 A4FF 0701 66A1 FAA4 054F 07CC"            /* ..kù¤ÿ..f¡ú¤.O.Ì */
	$"FF7D 9CF9 A401 6C8F FAA4 04A2 5E2E 31CC"            /* ÿ}œù¤.lú¤.¢^.1Ì */
	$"F7FF 01CA 9AF9 A401 7382 F9A4 0487 3B28"            /* ÷ÿ.Êšù¤.s‚ù¤.‡;( */
	$"6B9A F9A4 0125 34F8 6C01 1D4D F9A4 0607"            /* kšù¤.%4øl..Mù¤.. */
	$"1D6C 6C3F 0A9C FCA4 FFA4 036F 0729 A2FA"            /* .ll?Âœü¤ÿ¤.o.)¢ú */
	$"A403 8A07 2CA2 FAA4 046F 0722 4EA6 F9A4"            /* ¤.Š.,¢ú¤.o."N¦ù¤ */
	$"FF07 005E FAA4 069A 0707 B5CA 9AA1 FAA4"            /* ÿ..^ú¤.š..µÊš¡ú¤ */
	$"0363 0721 A1FA A403 3B07 2A9B F9A4 022E"            /* .c.!¡ú¤.;.*›ù¤.. */
	$"0780 F9A4 012F 19F6 FF01 B0A3 F8A4 0239"            /* .€ù¤./.öÿ.°£ø¤.9 */
	$"078A FBA4 05A4 4F07 3E5D A2FA A403 9707"            /* .Šû¤.¤O.>]¢ú¤.—. */
	$"078A FAA4 0484 0707 64A1 F9A4 0284 0777"            /* .Šú¤.„..d¡ù¤.„.w */
	$"F9A4 026F 0781 F9A4 032E 072E 9EF9 A401"            /* ù¤.o.ù¤....ù¤. */
	$"073F FB6C 00A8 0272 F6FF 01CF 8EF9 9504"            /* .?ûl.¨.röÿ.Ïù•. */
	$"6100 0D63 96FA 9503 7200 008C F995 FF00"            /* a..c–ú•.r..Œù•ÿ. */
	$"0144 91FA 9505 3F00 C8FF 6B8E F995 010C"            /* .D‘ú•.?.Èÿkù•.. */
	$"3CF9 9503 8500 00AD F7FF 01C4 8BF9 9502"            /* <ù•.…..­÷ÿ.Ä‹ù•. */
	$"1F13 8CF9 9503 3F00 258B F995 0016 005F"            /* ..Œù•.?.%‹ù•..._ */
	$"F8FF 012C 76F9 9505 002E FFFF 7A2C F995"            /* øÿ.,vù•...ÿÿz,ù• */
	$"0261 001A F995 0385 0020 92FA 9504 6100"            /* .a..ù•.…. ’ú•.a. */
	$"123E 99F9 95FF 0000 75FA 9506 8A00 00AD"            /* .>™ù•ÿ..uú•.Š..­ */
	$"BD8B 91FA 9502 5200 42F9 9503 2E00 0087"            /* ½‹‘ú•.R.Bù•....‡ */
	$"F995 021F 0075 F995 0021 0000 F6FF 01A5"            /* ù•...uù•.!..öÿ.¥ */
	$"96F8 9502 1401 8FFA 9504 3F00 384D 94FA"            /* –ø•...ú•.?.8M”ú */
	$"9503 8500 0D91 FA95 0472 0000 3194 F995"            /* •.…..‘ú•.r..1”ù• */
	$"0272 0077 F995 025C 0090 F995 031F 0000"            /* .r.wù•.\.ù•.... */
	$"8EF9 9501 007A EFFF 01CF 90FE 97FC 9704"            /* ù•..zïÿ.Ïş—ü—. */
	$"6300 1065 98FA 9703 7500 008F F997 FF00"            /* c..e˜ú—.u..ù—ÿ. */
	$"0146 94FA 9705 4200 C9FF 6E90 F997 010F"            /* .F”ú—.B.Éÿnù—.. */
	$"3EF9 9703 8700 00AE F7FF 01C5 8DF9 9702"            /* >ù—.‡..®÷ÿ.Åù—. */
	$"2115 8EF9 9703 4200 278D F997 011A 62F8"            /* !.ù—.B.'ù—..bø */
	$"FF01 2F78 FD97 FD97 0500 30FF FF7B 30F9"            /* ÿ./xı—ı—..0ÿÿ{0ù */
	$"9702 6300 1CF9 9703 8700 2295 FA97 0463"            /* —.c..ù—.‡."•ú—.c */
	$"0015 429B F997 FF00 0077 FA97 068C 0000"            /* ..B›ù—ÿ..wú—.Œ.. */
	$"AEBE 8D94 FA97 0254 0044 F997 0330 0002"            /* ®¾”ú—.T.Dù—.0.. */
	$"8AF9 9702 2100 77F9 9701 2400 F6FF 03A8"            /* Šù—.!.wù—.$.öÿ.¨ */
	$"9897 97FA 9702 1703 91FA 9704 4200 3B4F"            /* ˜——ú—...‘ú—.B.;O */
	$"96FA 9703 8700 1094 FA97 0475 0000 3396"            /* –ú—.‡..”ú—.u..3– */
	$"F997 0275 0079 F997 025E 0092 F997 0321"            /* ù—.u.yù—.^.’ù—.! */
	$"0002 90F9 9701 007C EFFF 01D3 9CF9 A204"            /* ..ù—..|ïÿ.Óœù¢. */
	$"6E07 1C6F A3FB A204 A282 0707 9BF9 A2FF"            /* n..o£û¢.¢‚..›ù¢ÿ */
	$"0701 519F FAA2 054E 07CC FF7B 9CF9 A201"            /* ..QŸú¢.N.Ìÿ{œù¢. */
	$"1B4C F9A2 0395 0707 B5F7 FF01 CA9A F9A2"            /* .Lù¢.•..µ÷ÿ.Êšù¢ */
	$"022E 229A F9A2 034E 0733 9AF9 A201 256C"            /* .."šù¢.N.3šù¢.%l */
	$"F8FF 0139 85F9 A205 073A FFFF 833B FBA2"            /* øÿ.9…ù¢..:ÿÿƒ;û¢ */
	$"FFA2 026E 0729 F9A2 0395 072C A0FA A204"            /* ÿ¢.n.)ù¢.•., ú¢. */
	$"6E07 224E A6F9 A2FF 0700 83FA A206 9907"            /* n."N¦ù¢ÿ..ƒú¢.™. */
	$"07B5 C39A 9FFA A202 6207 4EF9 A203 3A07"            /* .µÃšŸú¢.b.Nù¢.:. */
	$"0C98 F9A2 022E 0784 F9A2 012E 07F6 FF01"            /* .˜ù¢...„ù¢...öÿ. */
	$"B0A3 F8A2 0222 0D9E FBA2 05A2 4E07 4A5D"            /* °£ø¢.".û¢.¢N.J] */
	$"A1FA A203 9507 1BA0 FAA2 0482 0707 3EA1"            /* ¡ú¢.•.. ú¢.‚..>¡ */
	$"F9A2 0282 0786 F9A2 0269 079F F9A2 032E"            /* ù¢.‚.†ù¢.i.Ÿù¢.. */
	$"070C 9EF9 A201 0783 FAFF 0280 F6FF 02C4"            /* ..ù¢..ƒúÿ.€öÿ.Ä */
	$"8D94 FA91 045E 000D 6395 FA91 0370 0000"            /* ”ú‘.^..c•ú‘.p.. */
	$"95F9 91FF 0001 448F FA91 053E 00C8 FF6B"            /* •ù‘ÿ..Dú‘.>.Èÿk */
	$"8EF9 9102 0C2C 8DFA 9103 8300 00AD F7FF"            /* ù‘..,ú‘.ƒ..­÷ÿ */
	$"01C4 8CF9 9102 1F08 7FF9 9103 3E00 058C"            /* .ÄŒù‘....ù‘.>..Œ */
	$"F991 0016 005F F8FF 014C 8EF9 9106 002E"            /* ù‘..._øÿ.Lù‘... */
	$"FFFF 7A64 94FA 9103 5E00 1A94 FA91 0383"            /* ÿÿzd”ú‘.^..”ú‘.ƒ */
	$"0020 90FA 9104 5E00 123E 99F9 91FF 0000"            /* . ú‘.^..>™ù‘ÿ.. */
	$"7AFA 9106 8600 00AD 9B8B 8FFA 9102 5100"            /* zú‘.†..­›‹ú‘.Q. */
	$"4DF9 9103 2C00 007A F991 021F 007F F991"            /* Mù‘.,..zù‘....ù‘ */
	$"0021 0000 F6FF 02A5 9692 F991 0209 0A8E"            /* .!..öÿ.¥–’ù‘.ÆÂ */
	$"FA91 043E 0044 4D92 FA91 0383 0013 90FA"            /* ú‘.>.DM’ú‘.ƒ..ú */
	$"9104 7000 002C 94F9 9102 7000 75F9 9102"            /* ‘.p..,”ù‘.p.uù‘. */
	$"5100 8FF9 9103 1F00 008E F991 0100 7AEF"            /* Q.ù‘....ù‘..zï */
	$"FF04 C58F 9794 94FC 9404 6200 1065 97FA"            /* ÿ.Å—””ü”.b..e—ú */
	$"9403 7200 0097 F994 FF00 0146 91FA 9405"            /* ”.r..—ù”ÿ..F‘ú”. */
	$"4000 C9FF 6E90 F994 020F 3090 FA94 0385"            /* @.Éÿnù”..0ú”.… */
	$"0000 AEF7 FF01 C48E F994 0221 0A82 F994"            /* ..®÷ÿ.Äù”.!Â‚ù” */
	$"0340 0008 8EF9 9401 1A62 F8FF 014E 90FD"            /* .@..ù”..bøÿ.Nı */
	$"94FD 9406 0030 FFFF 7B67 96FA 9403 6200"            /* ”ı”..0ÿÿ{g–ú”.b. */
	$"1C97 FA94 0385 0022 92FA 9404 6200 1542"            /* .—ú”.…."’ú”.b..B */
	$"9BF9 94FF 0000 7CFA 9406 8A00 00AE 9C8D"            /* ›ù”ÿ..|ú”.Š..®œ */
	$"91FA 9402 5300 4FF9 9403 3000 007C F994"            /* ‘ú”.S.Où”.0..|ù” */
	$"0221 0081 F994 0124 00F6 FF03 A698 9594"            /* .!.ù”.$.öÿ.¦˜•” */
	$"FA94 020B 0C91 FA94 0440 0047 4F96 FA94"            /* ú”...‘ú”.@.GO–ú” */
	$"0385 0015 92FA 9404 7200 002F 96F9 9402"            /* .…..’ú”.r../–ù”. */
	$"7200 78F9 9402 5300 91F9 9403 2100 0090"            /* r.xù”.S.‘ù”.!.. */
	$"F994 0100 7CEF FF02 CA9C A2FA A004 6C07"            /* ù”..|ïÿ.Êœ¢ú .l. */
	$"1C6F A2FB A004 A081 0707 A2F9 A0FF 0701"            /* .o¢û . ..¢ù ÿ.. */
	$"519E FAA0 054E 07CC FF7B 9CF9 A002 1B3E"            /* Qú .N.Ìÿ{œù ..> */
	$"9CFA A003 9407 07B5 F7FF 01CA 9BF9 A002"            /* œú .”..µ÷ÿ.Ê›ù . */
	$"2E15 90F9 A003 4E07 129B F9A0 0125 6CF8"            /* ..ù .N..›ù .%lø */
	$"FF01 5B9E F9A0 0607 3AFF FF83 75A1 FCA0"            /* ÿ.[ù ..:ÿÿƒu¡ü  */
	$"FFA0 036C 0729 A2FA A003 9407 2C9F FAA0"            /* ÿ .l.)¢ú .”.,Ÿú  */
	$"046C 0722 4EA6 F9A0 FF07 008A FAA0 0697"            /* .l."N¦ù ÿ..Šú .— */
	$"0707 B5A1 9A9E FAA0 0261 0759 F9A0 0339"            /* ..µ¡šú .a.Yù .9 */
	$"0707 8CF9 A002 2E07 8EF9 A001 2C07 F6FF"            /* ..Œù ...ù .,.öÿ */
	$"02AF A3A1 F9A0 0216 179C FBA0 05A0 4E07"            /* .¯£¡ù ...œû . N. */
	$"545D A1FA A003 9407 219F FAA0 0481 0707"            /* T]¡ú .”.!Ÿú ... */
	$"39A1 F9A0 0281 0785 F9A0 0261 079E F9A0"            /* 9¡ù ..…ù .a.ù  */
	$"032E 0707 9EF9 A001 0783 FAFF 027B F6FF"            /* ....ù ..ƒúÿ.{öÿ */
	$"02C4 8D92 FA8E 045C 000D 6394 FA8E 036E"            /* .Ä’ú.\..c”ú.n */
	$"0000 95F9 8EFF 0001 448D FA8E 043D 00C8"            /* ..•ùÿ..Dú.=.È */
	$"FF6B F88E 020C 2E8B FA8E 0381 0000 ADF7"            /* ÿkø...‹ú...­÷ */
	$"FF01 C48C F98E 021F 087F F98E 033D 0000"            /* ÿ.ÄŒù....ù.=.. */
	$"7AF9 8E00 1600 5FF8 FF00 8AF8 8E06 002E"            /* zù..._øÿ.Šø... */
	$"FFFF 9590 92FA 8E03 5C00 1A92 FA8E 0281"            /* ÿÿ•’ú.\..’ú. */
	$"0026 F98E 045C 0012 3E99 F98E FF00 007A"            /* .&ù.\..>™ùÿ..z */
	$"FA8E 0684 0000 AD96 8B8D FA8E 0250 0045"            /* ú.„..­–‹ú.P.E */
	$"F98E 032B 0000 7AF9 8E02 1F00 86F9 8E00"            /* ù.+..zù...†ù. */
	$"2001 00D4 F7FF 02A4 9690 F98E 0209 198C"            /*  ..Ô÷ÿ.¤–ù.Æ.Œ */
	$"FA8E 043D 0044 4D91 FA8E 0381 0013 8FFA"            /* ú.=.DM‘ú...ú */
	$"8E04 6E00 002C 94F9 8E02 5C00 73F9 8E02"            /* .n..,”ù.\.sù. */
	$"5000 7FF9 8E02 1F00 00F8 8E01 007A EFFF"            /* P..ù....ø..zïÿ */
	$"04C4 8F96 9191 FC91 045F 0010 6596 FA91"            /* .Ä–‘‘ü‘._..e–ú‘ */
	$"0371 0000 97F9 91FF 0001 468F FA91 053F"            /* .q..—ù‘ÿ..Fú‘.? */
	$"00C9 FF6E 90F9 9102 0F30 8EFA 9103 8400"            /* .Éÿnù‘..0ú‘.„. */
	$"00AE F7FF 01C4 8EF9 9102 210A 82F9 9103"            /* .®÷ÿ.Äù‘.!Â‚ù‘. */
	$"3F00 007C F991 011A 62F8 FF01 8C90 FD91"            /* ?..|ù‘..bøÿ.Œı‘ */
	$"FD91 0600 30FF FF96 9295 FA91 035F 001C"            /* ı‘..0ÿÿ–’•ú‘._.. */
	$"96FA 9103 8400 2990 FA91 045F 0015 429B"            /* –ú‘.„.)ú‘._..B› */
	$"F991 FF00 007C FA91 0687 0000 AE97 8D8F"            /* ù‘ÿ..|ú‘.‡..®— */
	$"FA91 0252 0047 F991 032F 0000 7CF9 9102"            /* ú‘.R.Gù‘./..|ù‘. */
	$"2100 8AF9 9102 2200 D4F7 FF03 A698 9491"            /* !.Šù‘.".Ô÷ÿ.¦˜”‘ */
	$"FA91 020B 1C8F FA91 043F 0047 4F95 FA91"            /* ú‘...ú‘.?.GO•ú‘ */
	$"0284 0015 F991 0471 0000 2F96 F991 025F"            /* .„..ù‘.q../–ù‘._ */
	$"0076 F991 0252 0081 F991 0321 0000 90F9"            /* .vù‘.R.ù‘.!..ù */
	$"9101 007C EFFF 02CA 9CA1 FA9C 046B 071C"            /* ‘..|ïÿ.Êœ¡úœ.k.. */
	$"6FA1 FB9C 049C 7F07 07A2 F99C FF07 0151"            /* o¡ûœ.œ...¢ùœÿ..Q */
	$"9BFA 9C04 4D07 CCFF 7BF8 9C02 1B3E 9AFA"            /* ›úœ.M.Ìÿ{øœ..>šú */
	$"9C03 9107 07B5 F7FF 01CA 9BF9 9C02 2E15"            /* œ.‘..µ÷ÿ.Ê›ùœ... */
	$"8FF9 9C03 4D07 078C F99C 0125 6CF8 FF00"            /* ùœ.M..Œùœ.%løÿ. */
	$"99F8 9C06 073A FFFF 9F9F A1FC 9CFF 9C03"            /* ™øœ..:ÿÿŸŸ¡üœÿœ. */
	$"6B07 29A1 FA9C 0291 0733 F99C 046B 0722"            /* k.)¡úœ.‘.3ùœ.k." */
	$"4EA6 F99C FF07 008A FA9C 0695 0707 B599"            /* N¦ùœÿ..Šúœ.•..µ™ */
	$"9A9B FA9C 025F 0752 F99C 0339 0707 8CF9"            /* š›úœ._.Rùœ.9..Œù */
	$"9C02 2E07 97F9 9C02 2C07 D6F7 FF02 AFA3"            /* œ...—ùœ.,.Ö÷ÿ.¯£ */
	$"9FF9 9C02 1629 9BFB 9C05 9C4D 0754 5DA0"            /* Ÿùœ..)›ûœ.œM.T]  */
	$"FA9C 0391 0721 9EFA 9C04 7F07 0739 A1F9"            /* úœ.‘.!úœ....9¡ù */
	$"9C02 6B07 84F9 9C02 5F07 8FF9 9C03 2E07"            /* œ.k.„ùœ._.ùœ... */
	$"079E F99C 0107 83FA FF02 7AF6 FF02 C48D"            /* .ùœ..ƒúÿ.zöÿ.Ä */
	$"8EFA 8B04 5B00 0D63 92FA 8B03 6C00 0095"            /* ú‹.[..c’ú‹.l..• */
	$"F98B FF00 0044 F98B 053C 00C8 FF6B 8EF9"            /* ù‹ÿ..Dù‹.<.Èÿkù */
	$"8B02 0C2E 8AFA 8B03 7F00 00B8 F7FF 01C3"            /* ‹...Šú‹....¸÷ÿ.Ã */
	$"8CF9 8B02 1F08 7DF9 8B03 3C00 007A F98B"            /* Œù‹...}ù‹.<..zù‹ */
	$"0016 005F F8FF 019E 8EF9 8B06 002E FFFF"            /* ..._øÿ.ù‹...ÿÿ */
	$"9F90 91FA 8B03 6100 1A92 FA8B 037F 002B"            /* Ÿ‘ú‹.a..’ú‹...+ */
	$"8CFA 8B04 5B00 083E 99F9 8BFF 0000 7AFA"            /* Œú‹.[..>™ù‹ÿ..zú */
	$"8B04 8200 00AD 96F8 8B02 4F00 44F9 8B03"            /* ‹.‚..­–ø‹.O.Dù‹. */
	$"2B00 007A F98B 021F 0091 F98B 001F 0100"            /* +..zù‹...‘ù‹.... */
	$"C8F7 FF02 A496 8EF9 8B02 000A 0FFA 0C04"            /* È÷ÿ.¤–ù‹..Â.ú.. */
	$"0200 444D 91FA 8B03 7F00 138E FA8B 046C"            /* ..DM‘ú‹....ú‹.l */
	$"0000 2C94 F98B 025B 0072 F98B 024F 007B"            /* ..,”ù‹.[.rù‹.O.{ */
	$"F98B 031F 0000 8EF9 8B01 007A EFFF 04C4"            /* ù‹....ù‹..zïÿ.Ä */
	$"8F90 8E8E FC8E 045D 0010 6595 FA8E 036F"            /* ü.]..e•ú.o */
	$"0000 97F9 8EFF 0001 468D FA8E 053E 00C9"            /* ..—ùÿ..Fú.>.É */
	$"FF6E 90F9 8E02 0F31 8CFA 8E03 8200 00B9"            /* ÿnù..1Œú.‚..¹ */
	$"F7FF 00C4 F88E 0221 0A81 F98E 033E 0000"            /* ÷ÿ.Äø.!Âù.>.. */
	$"7CF9 8E01 1A62 F8FF 01A0 90FD 8EFD 8E06"            /* |ù..bøÿ. ıı. */
	$"0030 FFFF A192 94FA 8E03 6300 1C95 FA8E"            /* .0ÿÿ¡’”ú.c..•ú */
	$"0282 002F F98E 045D 000A 429B F98E FF00"            /* .‚./ù.].ÂB›ùÿ. */
	$"007C FA8E 0685 0000 AE97 8D8D FA8E 0251"            /* .|ú.…..®—ú.Q */
	$"0046 F98E 032E 0000 7CF9 8E02 2100 94F9"            /* .Fù....|ù.!.”ù */
	$"8E02 2100 C9F7 FF03 A698 918E FA8E 0200"            /* .!.É÷ÿ.¦˜‘ú.. */
	$"0C12 FA0F 0406 0047 4F94 FA8E 0382 0015"            /* ..ú....GO”ú.‚.. */
	$"90FA 8E04 6F00 002F 96F9 8E02 5D00 75F9"            /* ú.o../–ù.].uù */
	$"8E02 5100 7FF9 8E03 2100 0090 F98E 0100"            /* .Q..ù.!..ù.. */
	$"7CEF FF02 CA9C 9CFA 9A04 6807 1C6F A0FB"            /* |ïÿ.Êœœúš.h..o û */
	$"9A04 9A7C 0707 A2F9 9AFF 0700 51F9 9A05"            /* š.š|..¢ùšÿ..Qùš. */
	$"4C07 CCFF 7B9C F99A 021B 3E99 FA9A 0390"            /* L.Ìÿ{œùš..>™úš. */
	$"0707 BFF7 FF01 C99B F99A 022E 158F F99A"            /* ..¿÷ÿ.É›ùš...ùš */
	$"034C 0707 8CF9 9A01 256C F8FF 01AA 9CF9"            /* .L..Œùš.%løÿ.ªœù */
	$"9A06 073A FFFF AB9F A0FC 9AFF 9A03 6F07"            /* š..:ÿÿ«Ÿ üšÿš.o. */
	$"29A0 FA9A 0290 073A F99A 0468 0715 4EA6"            /* ) úš..:ùš.h..N¦ */
	$"F99A FF07 008A FA9A 0492 0707 B599 F89A"            /* ùšÿ..Šúš.’..µ™øš */
	$"025E 0750 F99A 0338 0707 8CF9 9A02 2E07"            /* .^.Pùš.8..Œùš... */
	$"9FF9 9A02 2B07 CCF7 FF02 AFA3 9CF9 9A02"            /* Ÿùš.+.Ì÷ÿ.¯£œùš. */
	$"0817 1EFB 1B05 1B10 0754 5D9F FA9A 0390"            /* ...û.....T]Ÿúš. */
	$"0721 9CFA 9A04 7C07 0739 A1F9 9A02 6807"            /* .!œúš.|..9¡ùš.h. */
	$"82F9 9A02 5E07 8DF9 9A03 2E07 079E F99A"            /* ‚ùš.^.ùš....ùš */
	$"0107 83FA FF02 76F6 FF02 C48C 8AFA 8804"            /* ..ƒúÿ.vöÿ.ÄŒŠúˆ. */
	$"5900 0063 91FA 8803 6900 0095 F988 FF00"            /* Y..c‘úˆ.i..•ùˆÿ. */
	$"0044 F988 053B 00C8 FF6B 8EF9 8802 0C2E"            /* .Dùˆ.;.Èÿkùˆ... */
	$"87FA 8803 7C00 00C8 F7FF 01C3 8BF9 8802"            /* ‡úˆ.|..È÷ÿ.Ã‹ùˆ. */
	$"1F08 7FF9 8803 3B00 007A F988 0016 005F"            /* ...ùˆ.;..zùˆ..._ */
	$"F8FF 019E 8EF9 8806 002E FFFF A090 90FA"            /* øÿ.ùˆ...ÿÿ ú */
	$"8803 6900 1A91 FA88 037C 002B 8CFA 8804"            /* ˆ.i..‘úˆ.|.+Œúˆ. */
	$"5900 003E 99F9 88FF 0000 7AFA 8805 8000"            /* Y..>™ùˆÿ..zúˆ.€. */
	$"00AD 968B F988 024E 0044 F988 032A 0000"            /* .­–‹ùˆ.N.Dùˆ.*.. */
	$"7AF9 8802 1F08 90F9 8800 1F01 00C8 F7FF"            /* zùˆ...ùˆ....È÷ÿ */
	$"02A3 958D F988 0100 0FF7 C802 734D 90FA"            /* .£•ùˆ...÷È.sMú */
	$"8803 7C00 138D FA88 0469 0000 2C94 F988"            /* ˆ.|..úˆ.i..,”ùˆ */
	$"0259 0073 F988 024E 007A F988 031F 0000"            /* .Y.sùˆ.N.zùˆ.... */
	$"8EF9 8801 007A EFFF 04C4 8E8C 8B8B FC8B"            /* ùˆ..zïÿ.ÄŒ‹‹ü‹ */
	$"045B 0000 6594 FA8B 036D 0000 97F9 8BFF"            /* .[..e”ú‹.m..—ù‹ÿ */
	$"0000 46F9 8B05 3D00 C9FF 6E90 F98B 020F"            /* ..Fù‹.=.Éÿnù‹.. */
	$"318A FA8B 0380 0000 C9F7 FF01 C48D F98B"            /* 1Šú‹.€..É÷ÿ.Äù‹ */
	$"0221 0A82 F98B 033D 0000 7CF9 8B01 1A62"            /* .!Â‚ù‹.=..|ù‹..b */
	$"F8FF 019F 90FD 8BFD 8B06 0030 FFFF A192"            /* øÿ.Ÿı‹ı‹..0ÿÿ¡’ */
	$"92FA 8B03 6D00 1D94 FA8B 0380 002F 8EFA"            /* ’ú‹.m..”ú‹.€./ú */
	$"8B04 5B00 0042 9BF9 8BFF 0000 7CFA 8B05"            /* ‹.[..B›ù‹ÿ..|ú‹. */
	$"8300 00AE 978D F98B 0250 0046 F98B 032C"            /* ƒ..®—ù‹.P.Fù‹., */
	$"0000 7CF9 8B02 210B 92F9 8B02 2100 C9F7"            /* ..|ù‹.!.’ù‹.!.É÷ */
	$"FF03 A597 8F8B FA8B 0100 12F7 C902 754F"            /* ÿ.¥—‹ú‹...÷É.uO */
	$"92FA 8B03 8000 158F FA8B 046D 0000 2F96"            /* ’ú‹.€..ú‹.m../– */
	$"F98B 025B 0077 F98B 0250 007D F98B 0321"            /* ù‹.[.wù‹.P.}ù‹.! */
	$"0000 90F9 8B01 007C EFFF 02CA 9B99 FA97"            /* ..ù‹..|ïÿ.Ê›™ú— */
	$"0466 0709 6FA0 FB97 0497 7B07 07A2 F997"            /* .f.Æo û—.—{..¢ù— */
	$"FF07 0151 98FA 9705 4A07 CCFF 7B9C F997"            /* ÿ..Q˜ú—.J.Ìÿ{œù— */
	$"011B 3EF9 9703 8E07 07CC F7FF 01C9 9AF9"            /* ..>ù—...Ì÷ÿ.Éšù */
	$"9702 2E15 90F9 9703 4A07 078C F997 0125"            /* —...ù—.J..Œù—.% */
	$"6CF8 FF01 AA9C F997 0607 3AFF FFAB 9F9F"            /* løÿ.ªœù—..:ÿÿ«ŸŸ */
	$"FC97 FF97 037B 0729 9FFA 9703 8E07 3A9A"            /* ü—ÿ—.{.)Ÿú—..:š */
	$"FA97 0466 0707 4EA6 F997 FF07 008A FA97"            /* ú—.f..N¦ù—ÿ..Šú— */
	$"0690 0707 B599 9A98 FA97 025E 0751 F997"            /* ...µ™š˜ú—.^.Qù— */
	$"0336 0707 8CF9 9702 2E16 9EF9 9702 2B07"            /* .6..Œù—...ù—.+. */
	$"CCF7 FF02 AEA2 9BF9 9701 071F FACC FECC"            /* Ì÷ÿ.®¢›ù—...úÌşÌ */
	$"027D 5D9E FA97 038E 0721 9BFA 9704 7B07"            /* .}]ú—..!›ú—.{. */
	$"0739 A1F9 9702 6607 84F9 9702 5E07 8DF9"            /* .9¡ù—.f.„ù—.^.ù */
	$"9703 2E07 079E F997 0107 83FA FF02 79F6"            /* —....ù—..ƒúÿ.yö */
	$"FF02 C38C 87FA 8504 5700 0063 90FA 8503"            /* ÿ.ÃŒ‡ú….W..cú…. */
	$"6700 0095 F985 FF00 0144 86FA 8505 3A00"            /* g..•ù…ÿ..D†ú….:. */
	$"C8FF 6B8E F985 020C 2F84 FA85 037A 0000"            /* Èÿkù…../„ú….z.. */
	$"C8F7 FF01 C38B F985 021F 0083 F985 033A"            /* È÷ÿ.Ã‹ù…...ƒù….: */
	$"0000 7AF9 8500 1600 5FF8 FF01 9C8E F985"            /* ..zù…..._øÿ.œù… */
	$"0600 2EFF FF9F 908F FA85 0367 002A 8BF9"            /* ...ÿÿŸú….g.*‹ù */
	$"8502 002B 8AFA 8504 5700 003E 99F9 85FF"            /* …..+Šú….W..>™ù…ÿ */
	$"0000 7AFA 8506 7D00 00AD 968B 86FA 8502"            /* ..zú….}..­–‹†ú…. */
	$"4D00 46F9 8503 2900 007A F985 0220 4F87"            /* M.Fù….)..zù…. O‡ */
	$"F985 001E 0100 B3F7 FF02 A295 8BFA 8502"            /* ù…....³÷ÿ.¢•‹ú…. */
	$"8100 12F7 FF02 7A50 8FFA 8503 7A00 138C"            /* ..÷ÿ.zPú….z..Œ */
	$"FA85 0467 0000 2C94 F985 0257 0075 F985"            /* ú….g..,”ù….W.uù… */
	$"024D 0078 F985 031F 0000 8EF9 8501 007A"            /* .M.xù…....ù…..z */
	$"EFFF 04C4 8E8B 8787 FC87 0459 0000 6592"            /* ïÿ.Ä‹‡‡ü‡.Y..e’ */
	$"FA87 036B 0000 97F9 87FF 0001 468A FA87"            /* ú‡.k..—ù‡ÿ..FŠú‡ */
	$"053D 00C9 FF6D 90F9 8701 0F31 F987 037D"            /* .=.Éÿmù‡..1ù‡.} */
	$"0000 C9F7 FF01 C48D F987 0221 0386 F987"            /* ..É÷ÿ.Äù‡.!.†ù‡ */
	$"033D 0000 7CF9 8701 1A62 F8FF 019F 90FD"            /* .=..|ù‡..bøÿ.Ÿı */
	$"87FD 8706 0030 FFFF A192 92FA 8703 6B00"            /* ‡ı‡..0ÿÿ¡’’ú‡.k. */
	$"2C8D F987 0200 2F8D FA87 0459 0000 429B"            /* ,ù‡../ú‡.Y..B› */
	$"F987 FF00 007C FA87 0681 0000 AE97 8D8A"            /* ù‡ÿ..|ú‡...®—Š */
	$"FA87 0250 0048 F987 032B 0000 7CF9 8702"            /* ú‡.P.Hù‡.+..|ù‡. */
	$"2452 8BF9 8702 2000 B4F7 FF03 A497 8D87"            /* $R‹ù‡. .´÷ÿ.¤—‡ */
	$"FB87 0283 0015 F7FF 027B 5291 FA87 037D"            /* û‡.ƒ..÷ÿ.{R‘ú‡.} */
	$"0015 8EFA 8704 6B00 002F 96F9 8702 5900"            /* ..ú‡.k../–ù‡.Y. */
	$"78F9 8702 5000 7BF9 8703 2100 0090 F987"            /* xù‡.P.{ù‡.!..ù‡ */
	$"0100 7CEF FF02 C99B 97FA 9504 6507 076F"            /* ..|ïÿ.É›—ú•.e..o */
	$"9FFB 9504 9579 0707 A2F9 95FF 0701 5196"            /* Ÿû•.•y..¢ù•ÿ..Q– */
	$"FA95 0549 07CC FF7B 9CF9 9501 1B3F F995"            /* ú•.I.Ìÿ{œù•..?ù• */
	$"038C 0707 CCF7 FF01 C99A F995 022E 0D94"            /* .Œ..Ì÷ÿ.Éšù•...” */
	$"F995 0349 0707 8CF9 9501 256C F8FF 01A9"            /* ù•.I..Œù•.%løÿ.© */
	$"9CF9 9506 073A FFFF AB9F 9EFC 95FF 9503"            /* œù•..:ÿÿ«Ÿü•ÿ•. */
	$"7907 3699 F995 0207 3A99 FA95 0465 0707"            /* y.6™ù•..:™ú•.e.. */
	$"4EA6 F995 FF07 008A FA95 068F 0707 B599"            /* N¦ù•ÿ..Šú•...µ™ */
	$"9A96 FA95 025D 0752 F995 0335 0707 8CF9"            /* š–ú•.].Rù•.5..Œù */
	$"9502 305F 97F9 9502 2A07 B9F7 FF02 ADA2"            /* •.0_—ù•.*.¹÷ÿ.­¢ */
	$"99FA 9502 9107 22FA FFFE FF02 8361 9CFA"            /* ™ú•.‘."úÿşÿ.ƒaœú */
	$"9503 8C07 219A FA95 0479 0707 39A1 F995"            /* •.Œ.!šú•.y..9¡ù• */
	$"0265 0785 F995 025D 078B F995 032E 0707"            /* .e.…ù•.].‹ù•.... */
	$"9EF9 9501 0783 FAFF 0278 F6FF 02BD 8C85"            /* ù•..ƒúÿ.xöÿ.½Œ… */
	$"FA82 0454 0000 638F FA82 0365 0000 95F9"            /* ú‚.T..cú‚.e..•ù */
	$"82FF 0001 4484 FA82 0539 00C8 FF6B 8EF9"            /* ‚ÿ..D„ú‚.9.Èÿkù */
	$"8202 0C2E 80FA 8203 7800 00C8 F7FF 01C3"            /* ‚...€ú‚.x..È÷ÿ.Ã */
	$"8BF9 8202 1F00 84F9 8203 3900 007A F982"            /* ‹ù‚...„ù‚.9..zù‚ */
	$"0016 005F F8FF 019C 8EF9 8206 002E FFFF"            /* ..._øÿ.œù‚...ÿÿ */
	$"9F90 8EFA 8203 6500 2C85 F982 0200 2B87"            /* Ÿú‚.e.,…ù‚..+‡ */
	$"FA82 0454 0000 3E99 F982 FF00 007A FA82"            /* ú‚.T..>™ù‚ÿ..zú‚ */
	$"067B 0000 AD96 8B84 FA82 024C 004C F982"            /* .{..­–‹„ú‚.L.Lù‚ */
	$"0328 0000 7AF9 8200 75F8 8201 8315 0100"            /* .(..zù‚.uø‚.ƒ... */
	$"ADF7 FF02 A195 88FA 8202 7800 27F7 FF02"            /* ­÷ÿ.¡•ˆú‚.x.'÷ÿ. */
	$"9185 8EFA 8203 7800 138B FA82 0465 0000"            /* ‘…ú‚.x..‹ú‚.e.. */
	$"2C94 F982 0254 0073 F982 0239 0076 F982"            /* ,”ù‚.T.sù‚.9.vù‚ */
	$"031F 0000 8EF9 8201 007A EFFF 04BE 8E88"            /* ....ù‚..zïÿ.¾ˆ */
	$"8585 FC85 0458 0000 6591 FA85 0368 0000"            /* ……ü….X..e‘ú….h.. */
	$"97F9 85FF 0001 4687 FA85 053C 00C9 FF6D"            /* —ù…ÿ..F‡ú….<.Éÿm */
	$"90F9 8502 0F31 83FA 8503 7B00 00C9 F7FF"            /* ù…..1ƒú….{..É÷ÿ */
	$"01C3 8DF9 8502 2100 86F9 8503 3C00 007C"            /* .Ãù….!.†ù….<..| */
	$"F985 011A 62F8 FF01 9F90 FD85 FD85 0600"            /* ù…..bøÿ.Ÿı…ı….. */
	$"30FF FFA0 9291 FA85 0368 002F 88F9 8502"            /* 0ÿÿ ’‘ú….h./ˆù…. */
	$"002F 8BFA 8504 5800 0042 9BF9 85FF 0000"            /* ./‹ú….X..B›ù…ÿ.. */
	$"7CFA 8506 7F00 00AE 978D 87FA 8502 4F00"            /* |ú…....®—‡ú….O. */
	$"4EF9 8503 2A00 007C F985 0077 F885 0386"            /* Nù….*..|ù….wø….† */
	$"1700 AEF7 FF03 A397 8B85 FB85 027B 0029"            /* ..®÷ÿ.£—‹…û….{.) */
	$"F7FF 0292 8790 FA85 037B 0015 8DFA 8504"            /* ÷ÿ.’‡ú….{..ú…. */
	$"6800 002F 96F9 8502 5800 77F9 8502 3C00"            /* h../–ù….X.wù….<. */
	$"79F9 8503 2100 0090 F985 0100 7CEF FF02"            /* yù….!..ù…..|ïÿ. */
	$"C29B 95FA 9104 6307 076F 9EFB 9104 9177"            /* Â›•ú‘.c..oû‘.‘w */
	$"0707 A2F9 91FF 0701 5195 FA91 0549 07CC"            /* ..¢ù‘ÿ..Q•ú‘.I.Ì */
	$"FF7B 9CF9 9102 1B3E 90FA 9103 8B07 07CC"            /* ÿ{œù‘..>ú‘.‹..Ì */
	$"F7FF 01C9 9AF9 9102 2E07 95F9 9103 4907"            /* ÷ÿ.Éšù‘...•ù‘.I. */
	$"078C F991 0125 6CF8 FF01 A99C F991 0607"            /* .Œù‘.%løÿ.©œù‘.. */
	$"3AFF FFAA 9F9C FC91 FF91 0377 0739 95F9"            /* :ÿÿªŸœü‘ÿ‘.w.9•ù */
	$"9102 073A 97FA 9104 6307 074E A6F9 91FF"            /* ‘..:—ú‘.c..N¦ù‘ÿ */
	$"0700 8AFA 9106 8D07 07B5 999A 95FA 9102"            /* ..Šú‘...µ™š•ú‘. */
	$"5C07 57F9 9103 3407 078C F991 0084 F891"            /* \.Wù‘.4..Œù‘.„ø‘ */
	$"0394 2107 B5F7 FF02 ACA2 97FA 9102 8B07"            /* .”!.µ÷ÿ.¬¢—ú‘.‹. */
	$"34FA FFFE FF02 9795 9CFA 9103 8B07 2199"            /* 4úÿşÿ.—•œú‘.‹.!™ */
	$"FA91 0477 0707 39A1 F991 0263 0784 F991"            /* ú‘.w..9¡ù‘.c.„ù‘ */
	$"0249 0788 F991 032E 0707 9EF9 9101 0783"            /* .I.ˆù‘....ù‘..ƒ */
	$"FAFF 0274 F6FF 02B8 8D83 FA7F 0452 0000"            /* úÿ.töÿ.¸ƒú..R.. */
	$"638E FA7F 0363 0000 95F9 7FFF 0001 4482"            /* cú..c..•ù.ÿ..D‚ */
	$"FA7F 0538 00C8 FF6B 8EF9 7F02 0C26 7DFA"            /* ú..8.Èÿkù...&}ú */
	$"7F03 7600 00C8 F7FF 01C2 8BF9 7F02 1F00"            /* ..v..È÷ÿ.Â‹ù.... */
	$"85F9 7F03 3800 007A F97F 0016 005F F8FF"            /* …ù..8..zù...._øÿ */
	$"019C 8EF9 7F06 002E FFFF 9E8F 8EFA 7F03"            /* .œù....ÿÿú.. */
	$"6300 2C83 F97F 0200 2B85 FA7F 0452 0000"            /* c.,ƒù...+…ú..R.. */
	$"3E99 F97F FF00 007A FA7F 0679 0000 AD96"            /* >™ù.ÿ..zú..y..­– */
	$"8B82 FA7F 034A 0049 84FA 7F03 2700 007A"            /* ‹‚ú..J.I„ú..'..z */
	$"F17F 0373 3F0D 0A01 6ED6 F7FF 029C 9586"            /* ñ..s?.Â.nÖ÷ÿ.œ•† */
	$"FA7F 026D 002E F7FF 02BF 8F8D FA7F 0376"            /* ú..m..÷ÿ.¿ú..v */
	$"0013 8AFA 7F04 6300 002C 94F9 7F02 4E00"            /* ..Šú..c..,”ù..N. */
	$"73F9 7F02 3800 75F9 7F03 1F00 008E F97F"            /* sù..8.uù.....ù. */
	$"0100 7AEF FF04 B98F 8682 82FC 8204 5500"            /* ..zïÿ.¹†‚‚ü‚.U. */
	$"0065 91FA 8203 6600 0097 F982 FF00 0146"            /* .e‘ú‚.f..—ù‚ÿ..F */
	$"85FA 8205 3B00 C9FF 6D90 F982 020F 2881"            /* …ú‚.;.Éÿmù‚..( */
	$"FA82 0379 0000 C9F7 FF01 C38D F982 0221"            /* ú‚.y..É÷ÿ.Ãù‚.! */
	$"0087 F982 033B 0000 7CF9 8201 1A62 F8FF"            /* .‡ù‚.;..|ù‚..bøÿ */
	$"019F 90FD 82FD 8206 0030 FFFF A091 90FA"            /* .Ÿı‚ı‚..0ÿÿ ‘ú */
	$"8203 6600 2F86 F982 0200 2F88 FA82 0455"            /* ‚.f./†ù‚../ˆú‚.U */
	$"0000 429B F982 FF00 007C FA82 067C 0000"            /* ..B›ù‚ÿ..|ú‚.|.. */
	$"AE97 8D85 FA82 034E 004C 87FA 8203 2900"            /* ®—…ú‚.N.L‡ú‚.). */
	$"007C F182 0577 4311 0C6F D6F7 FF03 9F97"            /* .|ñ‚.wC..oÖ÷ÿ.Ÿ— */
	$"8882 FB82 0270 0030 F7FF 02C1 918F FA82"            /* ˆ‚û‚.p.0÷ÿ.Á‘ú‚ */
	$"0379 0015 8CFA 8204 6600 002F 96F9 8202"            /* .y..Œú‚.f../–ù‚. */
	$"5100 76F9 8202 3B00 78F9 8203 2100 0090"            /* Q.vù‚.;.xù‚.!.. */
	$"F982 0100 7CEF FF02 C09B 94FA 8F04 6207"            /* ù‚..|ïÿ.À›”ú.b. */
	$"076F 9CFB 8F04 8F76 0707 A2F9 8FFF 0701"            /* .oœû.v..¢ùÿ.. */
	$"5192 FA8F 0548 07CC FF7B 9CF9 8F01 1B34"            /* Q’ú.H.Ìÿ{œù..4 */
	$"F98F 0388 0707 CCF7 FF01 C89A F98F 022E"            /* ù.ˆ..Ì÷ÿ.Èšù.. */
	$"0796 F98F 0348 0707 8CF9 8F01 256C F8FF"            /* .–ù.H..Œù.%løÿ */
	$"01A9 9CF9 8F06 073A FFFF AA9E 9CFC 8FFF"            /* .©œù..:ÿÿªœüÿ */
	$"8F03 7607 3994 F98F 0207 3A96 FA8F 0462"            /* .v.9”ù..:–ú.b */
	$"0707 4EA6 F98F FF07 008A FA8F 068B 0707"            /* ..N¦ùÿ..Šú.‹.. */
	$"B599 9A92 FA8F 035B 0755 94FA 8F03 3407"            /* µ™š’ú.[.U”ú.4. */
	$"078C F18F 0584 4F1B 1577 D8F7 FF02 A9A2"            /* .Œñ.„O..wØ÷ÿ.©¢ */
	$"96FA 8F02 7F07 3AFA FFFE FF02 C59E 9BFA"            /* –ú...:úÿşÿ.Å›ú */
	$"8F03 8807 2198 FA8F 0476 0707 39A1 F98F"            /* .ˆ.!˜ú.v..9¡ù */
	$"025E 0783 F98F 0248 0787 F98F 032E 0707"            /* .^.ƒù.H.‡ù.... */
	$"9EF9 8F01 0783 FAFF 0293 F6FF 02B8 8D82"            /* ù..ƒúÿ.“öÿ.¸‚ */
	$"FA7A 0450 0000 638D FA7A 0361 0000 95F9"            /* úz.P..cúz.a..•ù */
	$"7AFF 0001 4480 FA7A 0536 00C8 FF69 8EF9"            /* zÿ..D€úz.6.Èÿiù */
	$"7A02 0C26 7BFA 7A03 7300 00D1 F7FF 01C2"            /* z..&{úz.s..Ñ÷ÿ.Â */
	$"8AF9 7A02 1F00 85F9 7A02 3600 00F8 7A00"            /* Šùz...…ùz.6..øz. */
	$"160B 1F50 647C 8585 7B7A FFFF 9B8E F97A"            /* ...Pd|……{zÿÿ›ùz */
	$"0600 2EFF FF84 8E8C FA7A 0367 002C 81F9"            /* ...ÿÿ„Œúz.g.,ù */
	$"7A02 002B 83FA 7A04 5000 003E 99F9 7AFF"            /* z..+ƒúz.P..>™ùzÿ */
	$"00F9 7A06 7600 00AD 968A 80FA 7A03 4900"            /* .ùz.v..­–Š€úz.I. */
	$"4685 FA7A 0226 0000 F27A 0576 3F00 003B"            /* F…úz.&..òz.v?..; */
	$"C9F5 FF02 9295 83FA 7A02 6100 2EF7 FF02"            /* Éõÿ.’•ƒúz.a..÷ÿ. */
	$"C38E 8CFA 7A03 7300 1287 FA7A 0461 0000"            /* ÃŒúz.s..‡úz.a.. */
	$"2C94 F97A 0249 0072 F97A 0236 0073 F97A"            /* ,”ùz.I.rùz.6.sùz */
	$"031F 0000 8EF9 7A08 0027 5469 7D85 8576"            /* ....ùz..'Ti}……v */
	$"92F6 FF04 B98F 847D 7DFC 7D04 5200 0065"            /* ’öÿ.¹„}}ü}.R..e */
	$"8FFA 7D03 6400 0097 F97D FF00 0146 82FA"            /* ú}.d..—ù}ÿ..F‚ú */
	$"7D05 3900 C9FF 6D90 F97D 020F 287F FA7D"            /* }.9.Éÿmù}..(.ú} */
	$"0377 0000 D2F7 FF01 C38C F97D 0221 0087"            /* .w..Ò÷ÿ.ÃŒù}.!.‡ */
	$"F97D 0339 0000 7CF9 7D0C 1A21 5267 7F87"            /* ù}.9..|ù}..!Rg.‡ */
	$"877D 7BFF FF9E 90FD 7DFD 7D06 0030 FFFF"            /* ‡}{ÿÿı}ı}..0ÿÿ */
	$"8690 8EFA 7D03 6900 2F83 F97D 0200 2F85"            /* †ú}.i./ƒù}../… */
	$"FA7D 0452 0000 429B F97D FF00 007C FA7D"            /* ú}.R..B›ù}ÿ..|ú} */
	$"0679 0000 AE97 8C82 FA7D 034C 0048 88FA"            /* .y..®—Œ‚ú}.L.Hˆú */
	$"7D03 2800 007C F37D 0579 4301 003D C9F5"            /* }.(..|ó}.yC..=Éõ */
	$"FF03 9597 867D FB7D 0264 0030 F7FF 02C3"            /* ÿ.•—†}û}.d.0÷ÿ.Ã */
	$"908E FA7D 0377 0015 8AFA 7D04 6400 002F"            /* ú}.w..Šú}.d../ */
	$"96F9 7D02 4C00 75F9 7D02 3900 76F9 7D03"            /* –ù}.L.uù}.9.vù}. */
	$"2100 0090 F97D 0800 2A57 6C80 8787 7892"            /* !..ù}..*Wl€‡‡x’ */
	$"F6FF 02BF 9B92 FA8C 045E 0707 6F9B FB8C"            /* öÿ.¿›’úŒ.^..o›ûŒ */
	$"048C 7307 07A2 F98C FF07 0151 90FA 8C05"            /* .Œs..¢ùŒÿ..QúŒ. */
	$"4707 CCFF 7A9C F98C 021B 348D FA8C 0386"            /* G.ÌÿzœùŒ..4úŒ.† */
	$"0707 D4F7 FF01 C89A F98C 022E 0796 F98C"            /* ..Ô÷ÿ.ÈšùŒ...–ùŒ */
	$"0247 0707 F88C 0C25 2F61 778E 9596 8B85"            /* .G..øŒ.%/aw•–‹… */
	$"FFFF A99C F98C 0607 3AFF FF90 9C9A FC8C"            /* ÿÿ©œùŒ..:ÿÿœšüŒ */
	$"FF8C 037A 0739 91F9 8C02 073A 92FA 8C04"            /* ÿŒ.z.9‘ùŒ..:’úŒ. */
	$"5E07 074E A6F9 8CFF 0700 8AFA 8C06 8807"            /* ^..N¦ùŒÿ..ŠúŒ.ˆ. */
	$"07B5 9999 90FA 8C03 5A07 5295 FA8C 0232"            /* .µ™™úŒ.Z.R•úŒ.2 */
	$"0707 F28C 0588 4F0B 0747 CBF5 FF02 A0A2"            /* ..òŒ.ˆO..GËõÿ. ¢ */
	$"94FA 8C02 7307 3AFA FFFE FF02 C99C 9AFA"            /* ”úŒ.s.:úÿşÿ.Éœšú */
	$"8C03 8607 2097 FA8C 0473 0707 39A1 F98C"            /* Œ.†. —úŒ.s..9¡ùŒ */
	$"025A 0782 F98C 0247 0786 F98C 032E 0707"            /* .Z.‚ùŒ.G.†ùŒ.... */
	$"9EF9 8C08 0738 657C 8F95 9685 9802 82F6"            /* ùŒ..8e|•–…˜.‚ö */
	$"FF02 B88C 81FA 7804 4E00 0063 8CFA 7803"            /* ÿ.¸Œúx.N..cŒúx. */
	$"5E00 0095 F978 FF00 0144 7DFA 7805 3500"            /* ^..•ùxÿ..D}úx.5. */
	$"C8FF 698E F978 020C 267A FA78 0271 0000"            /* Èÿiùx..&zúx.q.. */
	$"F6FF 01C2 8AF9 7802 1F00 82F9 7803 3500"            /* öÿ.ÂŠùx...‚ùx.5. */
	$"007A F978 0073 FB7A 050F 47FF FF9B 8EF9"            /* .zùx.sûz..Gÿÿ›ù */
	$"7806 002E FFFF 848D 8BFA 7803 7100 2C7F"            /* x...ÿÿ„‹úx.q.,. */
	$"F978 0200 2B84 FA78 044E 0000 3E99 F978"            /* ùx..+„úx.N..>™ùx */
	$"FF00 007A FA78 0673 0000 AD96 887D FA78"            /* ÿ..zúx.s..­–ˆ}úx */
	$"0348 0046 83FA 7803 2500 007A F478 0657"            /* .H.Fƒúx.%..zôx.W */
	$"0B00 1CA0 FFFF F5FF 0291 9582 FA78 0257"            /* ... ÿÿõÿ.‘•‚úx.W */
	$"002E F7FF 02C2 8D8B FA78 0371 0012 85FA"            /* ..÷ÿ.Â‹úx.q..…ú */
	$"7804 5E00 0C2C 94F9 7802 4800 6DF9 7802"            /* x.^..,”ùx.H.mùx. */
	$"3500 72F9 7803 1F00 008E F978 0071 FB7A"            /* 5.rùx....ùx.qûz */
	$"0100 70F6 FF04 B98E 847B 7BFC 7B04 5100"            /* ..pöÿ.¹„{{ü{.Q. */
	$"0065 8EFA 7B03 6200 0097 F97B FF00 0146"            /* .eú{.b..—ù{ÿ..F */
	$"81FA 7B05 3900 C9FF 6C90 F97B 020F 297D"            /* ú{.9.Éÿlù{..)} */
	$"FA7B 0275 0000 F6FF 01C3 8CF9 7B02 2100"            /* ú{.u..öÿ.ÃŒù{.!. */
	$"85F9 7B03 3900 007C F97B 0077 FB7D 0511"            /* …ù{.9..|ù{.wû}.. */
	$"49FF FF9C 90FD 7BFD 7B06 0030 FFFF 858F"            /* Iÿÿœı{ı{..0ÿÿ… */
	$"8EFA 7B03 7500 2F82 F97B 0200 2F86 FA7B"            /* ú{.u./‚ù{../†ú{ */
	$"0451 0000 429B F97B FF00 007C FA7B 0677"            /* .Q..B›ù{ÿ..|ú{.w */
	$"0000 AE97 8B81 FA7B 034C 0048 85FA 7B03"            /* ..®—‹ú{.L.H…ú{. */
	$"2700 007C F47B 045A 0D00 1EA0 F3FF 0394"            /* '..|ô{.Z... óÿ.” */
	$"9784 7BFB 7B02 5A00 30F7 FF02 C38F 8DFA"            /* —„{û{.Z.0÷ÿ.Ãú */
	$"7B03 7500 1488 FA7B 0462 0010 2F96 F97B"            /* {.u..ˆú{.b../–ù{ */
	$"024C 0070 F97B 0239 0076 F97B 0321 0000"            /* .L.pù{.9.vù{.!.. */
	$"90F9 7B00 75FB 7D01 0071 F6FF 02BF 9B91"            /* ù{.uû}..qöÿ.¿›‘ */
	$"FA88 045D 0707 6F9A FB88 0488 7107 07A2"            /* úˆ.]..ošûˆ.ˆq..¢ */
	$"F988 FF07 0151 8EFA 8805 4607 CCFF 7A9C"            /* ùˆÿ..Qúˆ.F.Ìÿzœ */
	$"F988 021B 348B FA88 0284 0707 F6FF 01C8"            /* ùˆ..4‹úˆ.„..öÿ.È */
	$"99F9 8802 2E07 92F9 8803 4607 078C F988"            /* ™ùˆ...’ùˆ.F..Œùˆ */
	$"0086 FB8C 051E 53FF FFA8 9CF9 8806 073A"            /* .†ûŒ..Sÿÿ¨œùˆ..: */
	$"FFFF 8F9B 9AFC 88FF 8803 8407 398F F988"            /* ÿÿ›šüˆÿˆ.„.9ùˆ */
	$"0207 3A94 FA88 045D 0707 4EA6 F988 FF07"            /* ..:”úˆ.]..N¦ùˆÿ. */
	$"008A FA88 0686 0707 B599 998E FA88 0359"            /* .Šúˆ.†..µ™™úˆ.Y */
	$"0753 92FA 8803 3207 078C F488 0467 1707"            /* .S’úˆ.2..Œôˆ.g.. */
	$"28A5 F3FF 029F A291 FA88 0268 073A FAFF"            /* (¥óÿ.Ÿ¢‘úˆ.h.:úÿ */
	$"FEFF 02C8 9C99 FA88 0384 0720 96FA 8804"            /* şÿ.Èœ™úˆ.„. –úˆ. */
	$"7107 1C39 A1F9 8802 5907 7CF9 8802 4607"            /* q..9¡ùˆ.Y.|ùˆ.F. */
	$"85F9 8803 2E07 079E F988 0084 FB8C 0109"            /* …ùˆ....ùˆ.„ûŒ.Æ */
	$"7602 7FF6 FF02 B78C 7FFA 7504 4C00 0062"            /* v..öÿ.·Œ.úu.L..b */
	$"8BFA 7503 5C00 0095 F975 FF00 0144 7BFA"            /* ‹úu.\..•ùuÿ..D{ú */
	$"7505 3400 C8FF 698E F975 020C 2781 FA75"            /* u.4.Èÿiùu..'úu */
	$"026F 0000 F6FF 01C1 8AF9 7502 1F00 83F9"            /* .o..öÿ.ÁŠùu...ƒù */
	$"7503 3400 007A F875 FB75 050C 47FF FF9A"            /* u.4..zøuûu..Gÿÿš */
	$"8EF9 7506 002E FFFF 888C 8AFA 7503 6F00"            /* ùu...ÿÿˆŒŠúu.o. */
	$"2C7F F975 020C 2B82 FA75 044C 0000 3E99"            /* ,.ùu..+‚úu.L..>™ */
	$"F975 FF00 007A FA75 0671 0000 AD96 887B"            /* ùuÿ..zúu.q..­–ˆ{ */
	$"FA75 035C 4F7F 79FA 7503 2400 007A F575"            /* úu.\O.yúu.$..zõu */
	$"0446 0008 6DDD FEFF F5FF 028F 9580 FA75"            /* .F..mİşÿõÿ.•€úu */
	$"084C 0016 7D8F A4AB ACAC FEAD 03A3 AB8D"            /* .L..}¤«¬¬ş­.£« */
	$"8AFA 7503 6F00 0884 FA75 045C 0014 2C94"            /* Šúu.o..„úu.\..,” */
	$"F975 0247 0063 F975 0234 007A F975 031F"            /* ùu.G.cùu.4.zùu.. */
	$"0000 8EF2 7501 0070 F6FF 04B8 8E82 7878"            /* ..òu..pöÿ.¸‚xx */
	$"FC78 044F 0000 648D FA78 035F 0000 97F9"            /* üx.O..dúx._..—ù */
	$"78FF 0001 467F FA78 0538 00C9 FF6C 90F9"            /* xÿ..F.úx.8.Éÿlù */
	$"7802 0F29 83FA 7802 7200 00F6 FF01 C28C"            /* x..)ƒúx.r..öÿ.ÂŒ */
	$"F978 0221 0085 F978 0338 0000 7CF2 7805"            /* ùx.!.…ùx.8..|òx. */
	$"0F49 FFFF 9C90 FD78 FD78 0600 30FF FF8B"            /* .Iÿÿœıxıx..0ÿÿ‹ */
	$"8E8D FA78 0372 002F 81F9 7802 0F2F 84FA"            /* úx.r./ùx../„ú */
	$"7804 4F00 0040 9BF9 78FF 0000 7CFA 7806"            /* x.O..@›ùxÿ..|úx. */
	$"7500 00AE 978B 7FFA 7803 5E52 827B FA78"            /* u..®—‹.úx.^R‚{úx */
	$"0326 0000 7CF5 7804 4900 0A6F DDF2 FF03"            /* .&..|õx.I.Âoİòÿ. */
	$"9197 8278 FB78 054F 0019 8091 A6FE ADFE"            /* ‘—‚xûx.O..€‘¦ş­ş */
	$"AE03 A4AD 8F8C FA78 0372 000B 86FA 7804"            /* ®.¤­Œúx.r..†úx. */
	$"5F00 172F 96F9 7802 4A00 66F9 7802 3800"            /* _../–ùx.J.fùx.8. */
	$"7DF9 7803 2100 0090 F278 0100 71F6 FF02"            /* }ùx.!..òx..qöÿ. */
	$"BF9B 8FFA 8604 5B07 076F 99FB 8604 866F"            /* ¿›ú†.[..o™û†.†o */
	$"0707 A2F9 86FF 0701 518D FA86 0545 07CC"            /* ..¢ù†ÿ..Qú†.E.Ì */
	$"FF7A 9CF9 8602 1B35 91FA 8602 8307 07F6"            /* ÿzœù†..5‘ú†.ƒ..ö */
	$"FF01 C899 F986 022E 0794 F986 0345 0707"            /* ÿ.È™ù†...”ù†.E.. */
	$"8CF2 8605 1B53 FFFF A69C F986 0607 3AFF"            /* Œò†..Sÿÿ¦œù†..:ÿ */
	$"FF96 9A99 FC86 FF86 0383 0739 8EF9 8602"            /* ÿ–š™ü†ÿ†.ƒ.9ù†. */
	$"1B3A 91FA 8604 5B07 074E A6F9 86FF 0700"            /* .:‘ú†.[..N¦ù†ÿ.. */
	$"8AFA 8606 8407 07B5 9998 8DFA 8603 6D61"            /* Šú†.„..µ™˜ú†.ma */
	$"8F8A FA86 0331 0707 8CF5 8604 5707 1579"            /* Šú†.1..Œõ†.W..y */
	$"DEF2 FF02 9EA2 8FFA 8605 5B07 2487 9CAF"            /* Şòÿ.¢ú†.[.$‡œ¯ */
	$"FEB5 00B6 FFB6 03AC B59B 98FA 8603 8307"            /* şµ.¶ÿ¶.¬µ›˜ú†.ƒ. */
	$"1594 FA86 046F 0725 39A1 F986 0258 0771"            /* .”ú†.o.%9¡ù†.X.q */
	$"F986 0245 078D F986 032E 0707 9EF2 8601"            /* ù†.E.ù†....ò†. */
	$"0776 0265 F6FF 02AE 8B7C FA71 0449 0000"            /* .v.eöÿ.®‹|úq.I.. */
	$"628A FA71 035A 0000 95F9 71FF 0001 4479"            /* bŠúq.Z..•ùqÿ..Dy */
	$"FA71 0533 00C8 FF68 8EF9 7102 0C27 84FA"            /* úq.3.Èÿhùq..'„ú */
	$"7102 6D00 00F6 FF01 C188 F971 021F 0083"            /* q.m..öÿ.Áˆùq...ƒ */
	$"F971 0333 0000 79F8 71FB 7105 0C47 FFFF"            /* ùq.3..yøqûq..Gÿÿ */
	$"998E F971 0600 2EFF FF90 8A88 FA71 036D"            /* ™ùq...ÿÿŠˆúq.m */
	$"002C 7CF9 7102 0C2B 80FA 7104 4900 003E"            /* .,|ùq..+€úq.I..> */
	$"99F9 71FF 0000 7AFA 7106 6F00 00AD 9688"            /* ™ùqÿ..zúq.o..­–ˆ */
	$"79F0 7104 6401 0002 7AF5 7107 6F44 0D03"            /* yğq.d...zõq.oD.. */
	$"6CDD FFFF F5FF 028D 947D FA71 0349 465F"            /* lİÿÿõÿ.”}úq.IF_ */
	$"72FA 7304 5401 828C 88FA 7103 7040 5C7B"            /* rús.T.‚Œˆúq.p@\{ */
	$"FA71 045A 002E 2094 F971 0246 0063 F971"            /* úq.Z.. ”ùq.F.cùq */
	$"0222 007F F971 031F 0000 8EF2 7101 0070"            /* ."..ùq....òq..p */
	$"F6FF 04AF 8D80 7575 FC75 044D 0000 648C"            /* öÿ.¯€uuüu.M..dŒ */
	$"FA75 035D 0000 97F9 75FF 0001 467C FA75"            /* úu.]..—ùuÿ..F|úu */
	$"0536 00C9 FF6C 90F9 7502 0F29 86FA 7502"            /* .6.Éÿlùu..)†úu. */
	$"7000 00F6 FF01 C28B F975 0221 0086 F975"            /* p..öÿ.Â‹ùu.!.†ùu */
	$"0336 0000 7CF2 7505 0F49 FFFF 9B90 FD75"            /* .6..|òu..Iÿÿ›ıu */
	$"FD75 0600 30FF FF92 8C8C FA75 0370 002F"            /* ıu..0ÿÿ’ŒŒúu.p./ */
	$"80F9 7502 0F2F 82FA 7504 4D00 0040 9BF9"            /* €ùu../‚úu.M..@›ù */
	$"75FF 0000 7CFA 7506 7200 00AE 978B 7CF0"            /* uÿ..|úu.r..®—‹|ğ */
	$"7504 6703 0006 7CF5 7505 7247 1106 6DDD"            /* u.g...|õu.rG..mİ */
	$"F3FF 038F 9680 75FB 7503 4D48 6476 FA77"            /* óÿ.–€uûu.MHdvúw */
	$"0458 0585 8E8B FA75 0373 435F 7DFA 7504"            /* .X.…‹úu.sC_}úu. */
	$"5D00 3022 96F9 7502 4900 65F9 7502 2500"            /* ].0"–ùu.I.eùu.%. */
	$"82F9 7503 2100 0090 F275 0100 71F6 FF02"            /* ‚ùu.!..òu..qöÿ. */
	$"B79A 8EFA 8404 5A07 076F 99FB 8404 846E"            /* ·šú„.Z..o™û„.„n */
	$"0707 A2F9 84FF 0701 518B FA84 0544 07CC"            /* ..¢ù„ÿ..Q‹ú„.D.Ì */
	$"FF7A 9CF9 8402 1B35 94FA 8402 8107 07F6"            /* ÿzœù„..5”ú„...ö */
	$"FF01 C799 F984 022E 0794 F984 0344 0707"            /* ÿ.Ç™ù„...”ù„.D.. */
	$"8CF2 8405 1B53 FFFF A59C F984 0607 3AFF"            /* Œò„..Sÿÿ¥œù„..:ÿ */
	$"FF9F 9998 FC84 FF84 0381 0739 8EF9 8402"            /* ÿŸ™˜ü„ÿ„..9ù„. */
	$"1B3A 90FA 8404 5A07 074E A6F9 84FF 0700"            /* .:ú„.Z..N¦ù„ÿ.. */
	$"8AFA 8406 8207 07B5 9998 8BF0 8404 770D"            /* Šú„.‚..µ™˜‹ğ„.w. */
	$"0710 8CF5 8405 8357 1D10 75DE F3FF 029C"            /* ..Œõ„.ƒW..uŞóÿ.œ */
	$"A28E FA84 025A 5773 FD86 FE87 0587 670F"            /* ¢ú„.ZWsı†ş‡.‡g. */
	$"929B 97F9 8402 526F 8CFA 8404 6E07 3A2F"            /* ’›—ù„.RoŒú„.n.:/ */
	$"A1F9 8402 5807 71F9 8402 3007 90F9 8403"            /* ¡ù„.X.qù„.0.ù„. */
	$"2E07 079E F284 0107 7602 50F6 FF02 AB8B"            /* ...ò„..v.Pöÿ.«‹ */
	$"7AFA 6E04 4700 0062 88FA 6E03 5800 0095"            /* zún.G..bˆún.X..• */
	$"F96E FF00 0144 77FA 6E05 3200 C8FF 688E"            /* ùnÿ..Dwún.2.Èÿh */
	$"F96E 020C 2783 FA6E 026B 0000 F6FF 01C1"            /* ùn..'ƒún.k..öÿ.Á */
	$"88F9 6E02 1F00 79F9 6E03 3200 0079 F96E"            /* ˆùn...yùn.2..yùn */
	$"0016 FA00 0447 FFFF 958E F96E 0600 2EFF"            /* ..ú..Gÿÿ•ùn...ÿ */
	$"FF8F 8887 FA6E 036B 002C 7AF9 6E02 0C2B"            /* ÿˆ‡ún.k.,zùn..+ */
	$"7DFA 6E04 4700 003D 99F9 6EFF 0000 7AFA"            /* }ún.G..=™ùnÿ..zú */
	$"6E06 6C00 00AD 9687 77F0 6E04 4800 002E"            /* n.l..­–‡wğn.H... */
	$"79F3 6E05 613A 1B28 A1FF F5FF 028B 947B"            /* yón.a:.(¡ÿõÿ.‹”{ */
	$"FA6E 0147 6BF8 6E04 4500 7A84 87EF 6E04"            /* ún.Gkøn.E.z„‡ïn. */
	$"5800 2E1A 94F9 6E02 3600 62F9 6E02 2100"            /* X...”ùn.6.bùn.!. */
	$"73F9 6E03 1F00 008E F96E F900 0070 F6FF"            /* sùn....ùnù..pöÿ */
	$"04AD 8D7D 7171 FC71 044A 0000 648B FA71"            /* .­}qqüq.J..d‹úq */
	$"035B 0000 97F9 71FF 0001 467A FA71 0535"            /* .[..—ùqÿ..Fzúq.5 */
	$"00C9 FF6B 90F9 7102 0F29 85FA 7102 6E00"            /* .Éÿkùq..)…úq.n. */
	$"00F6 FF01 C28B F971 0221 007C F971 0335"            /* .öÿ.Â‹ùq.!.|ùq.5 */
	$"0000 7CF9 7100 1AFA 0004 49FF FF97 90FD"            /* ..|ùq..ú..Iÿÿ—ı */
	$"71FD 7106 0030 FFFF 918B 8BFA 7103 6E00"            /* qıq..0ÿÿ‘‹‹úq.n. */
	$"2F7D F971 020F 2F80 FA71 044A 0000 409B"            /* /}ùq../€úq.J..@› */
	$"F971 FF00 007C FA71 066F 0000 AE97 8A7A"            /* ùqÿ..|úq.o..®—Šz */
	$"F071 044C 0002 307C F371 0464 3D1D 2AA2"            /* ğq.L..0|óq.d=.*¢ */
	$"F4FF 038E 967D 71FB 7101 4A6E F871 0448"            /* ôÿ.–}qûq.Jnøq.H */
	$"007C 868A EF71 045B 0030 1D96 F971 023A"            /* .|†Šïq.[.0.–ùq.: */
	$"0064 F971 0224 0077 F971 0321 0000 90F9"            /* .dùq.$.wùq.!..ù */
	$"71F9 0000 71F6 FF02 B59A 8CFA 8104 5807"            /* qù..qöÿ.µšŒú.X. */
	$"076E 98FB 8104 816C 0707 A2F9 81FF 0701"            /* .n˜û.l..¢ùÿ.. */
	$"518A FA81 0544 07CC FF79 9CF9 8102 1B35"            /* QŠú.D.Ìÿyœù..5 */
	$"94FA 8102 8007 07F6 FF01 C798 F981 022E"            /* ”ú.€..öÿ.Ç˜ù.. */
	$"078C F981 0344 0707 8CF9 8100 25FA 0704"            /* .Œù.D..Œù.%ú.. */
	$"53FF FFA3 9CF9 8106 073A FFFF 9E99 97FC"            /* Sÿÿ£œù..:ÿÿ™—ü */
	$"81FF 8103 8007 398C F981 021B 3A8E FA81"            /* ÿ.€.9Œù..:ú */
	$"0458 0707 4DA6 F981 FF07 008A FA81 0680"            /* .X..M¦ùÿ..Šú.€ */
	$"0707 B599 978A F081 0459 070C 388C F381"            /* ..µ™—Šğ.Y..8Œó */
	$"0475 4F2C 36A6 F4FF 029A A18C FA81 0158"            /* .uO,6¦ôÿ.š¡Œú.X */
	$"7FF9 8105 8157 078B 9597 EF81 046C 073A"            /* .ù.W.‹•—ï.l.: */
	$"29A1 F981 0248 0770 F981 022F 0784 F981"            /* )¡ù.H.pù./.„ù */
	$"032E 0707 9CF9 81F9 0700 7602 59F6 FF02"            /* ....œùù..v.Yöÿ. */
	$"AB8B 79FA 6B04 4500 0061 87FA 6B03 5700"            /* «‹yúk.E..a‡úk.W. */
	$"0095 F96B FF00 0144 76FA 6B05 3100 C8FF"            /* .•ùkÿ..Dvúk.1.Èÿ */
	$"688E F96B 020C 2583 FA6B 0269 0000 F6FF"            /* hùk..%ƒúk.i..öÿ */
	$"01C1 88F9 6B02 1F00 76F9 6B03 3100 0079"            /* .Áˆùk...vùk.1..y */
	$"F96B 0016 005F F8FF 0188 8EF9 6B06 002E"            /* ùk..._øÿ.ˆùk... */
	$"FFFF 7687 86FA 6B03 6900 2C79 F96B 020C"            /* ÿÿv‡†úk.i.,yùk.. */
	$"387B FA6B 0445 0000 3D99 F96B FF00 007A"            /* 8{úk.E..=™ùkÿ..z */
	$"FA6B 0669 0000 AD96 8676 FA6B 0262 596C"            /* úk.i..­–†vúk.bYl */
	$"FA6B 046C 1300 0081 F16B 0359 3027 7100"            /* úk.l...ñk.Y0'q. */
	$"DEF6 FF02 8A92 79FA 6B01 5468 F86B 0444"            /* Şöÿ.Š’yúk.Thøk.D */
	$"0077 5E86 EF6B 0457 002E 1A94 F96B 0231"            /* .w^†ïk.W...”ùk.1 */
	$"0061 F96B 0220 006F F96B 031F 0000 8EF9"            /* .aùk. .oùk....ù */
	$"6B01 007A EFFF 04AC 8D7C 6F6F FC6F 0448"            /* k..zïÿ.¬|ooüo.H */
	$"0000 638B FA6F 035A 0000 97F9 6FFF 0001"            /* ..c‹úo.Z..—ùoÿ.. */
	$"4679 FA6F 0534 00C9 FF6B 90F9 6F02 0F27"            /* Fyúo.4.Éÿkùo..' */
	$"85FA 6F02 6D00 00F6 FF01 C28B F96F 0221"            /* …úo.m..öÿ.Â‹ùo.! */
	$"0078 F96F 0334 0000 7CF9 6F01 1A62 F8FF"            /* .xùo.4..|ùo..bøÿ */
	$"018B 90FD 6FFD 6F06 0030 FFFF 788A 8AFA"            /* .‹ıoıo..0ÿÿxŠŠú */
	$"6F03 6D00 2F7C F96F 020F 3A7F FA6F 0448"            /* o.m./|ùo..:.úo.H */
	$"0000 3F9B F96F FF00 007C FA6F 066E 0000"            /* ..?›ùoÿ..|úo.n.. */
	$"AE97 8879 FA6F 0165 5CF9 6F04 7015 0000"            /* ®—ˆyúo.e\ùo.p... */
	$"83F1 6F04 5C33 2A73 DEF6 FF03 8C95 7C6F"            /* ƒño.\3*sŞöÿ.Œ•|o */
	$"FB6F 0158 6CF8 6F04 4700 7961 88EF 6F04"            /* ûo.Xløo.G.yaˆïo. */
	$"5A00 301D 96F9 6F02 3400 64F9 6F02 2200"            /* Z.0.–ùo.4.dùo.". */
	$"71F9 6F03 2100 0090 F96F 0100 7CEF FF02"            /* qùo.!..ùo..|ïÿ. */
	$"B59A 8BFA 7F04 5507 076E 97FB 7F04 7F6B"            /* µš‹ú..U..n—û...k */
	$"0707 A2F9 7FFF 0701 5187 FA7F 0543 07CC"            /* ..¢ù.ÿ..Q‡ú..C.Ì */
	$"FF79 9CF9 7F02 1B32 92FA 7F02 7D07 07F6"            /* ÿyœù...2’ú..}..ö */
	$"FF01 C798 F97F 022E 0788 F97F 0343 0707"            /* ÿ.Ç˜ù....ˆù..C.. */
	$"8CF9 7F01 256C F8FF 0198 9EF9 7F06 073A"            /* Œù..%løÿ.˜ù...: */
	$"FFFF 8898 96FC 7FFF 7F03 7D07 398A F97F"            /* ÿÿˆ˜–ü.ÿ..}.9Šù. */
	$"021B 478C FA7F 0455 0707 4CA6 F97F FF07"            /* ..GŒú..U..L¦ù.ÿ. */
	$"008A F97F FF07 03B5 9997 87FA 7F02 766D"            /* .Šù.ÿ..µ™—‡ú..vm */
	$"80FA 7F04 8020 0707 91F1 7F04 6D46 3B80"            /* €ú..€ ..‘ñ..mF;€ */
	$"DFF6 FF02 99A0 8BFA 7F01 677C F97F 057F"            /* ßöÿ.™ ‹ú..g|ù... */
	$"5507 876D 96EF 7F04 6B07 3A29 A1F9 7F02"            /* U.‡m–ï..k.:)¡ù.. */
	$"4307 70F9 7F01 2E07 F87F 032E 0707 9CF9"            /* C.pù....ø.....œù */
	$"7F01 0783 FAFF 026E F6FF 02AA 8B79 FA68"            /* ...ƒúÿ.nöÿ.ª‹yúh */
	$"0444 0000 6186 FA68 0354 0000 95F9 68FF"            /* .D..a†úh.T..•ùhÿ */
	$"0001 4373 FA68 0530 00C8 FF68 8EF9 6802"            /* ..Csúh.0.Èÿhùh. */
	$"0C1D 82FA 6802 6700 0FF6 FF01 C088 F968"            /* ..‚úh.g..öÿ.Àˆùh */
	$"021F 0072 F968 0330 0000 79F9 6800 1600"            /* ...rùh.0..yùh... */
	$"5FF8 FF01 878E F968 0600 2EFF FF71 8685"            /* _øÿ.‡ùh...ÿÿq†… */
	$"F968 0200 2C77 F968 020C 386E FA68 0444"            /* ùh..,wùh..8núh.D */
	$"0000 3C99 F968 FF00 007A F968 FF00 03AD"            /* ..<™ùhÿ..zùhÿ..­ */
	$"9686 73FA 6802 4300 44F9 6803 1F00 028A"            /* –†súh.C.Dùh....Š */
	$"F068 0265 4D31 013A C9F7 FF02 8791 77FA"            /* ğh.eM1.:É÷ÿ.‡‘wú */
	$"6803 542F 315D FA68 0443 0077 4785 FA68"            /* h.T/1]úh.C.wG…úh */
	$"036B 3131 50FA 6804 5400 2E1A 94F9 6802"            /* .k11Púh.T...”ùh. */
	$"3000 5FF9 6802 1F00 6FF9 6803 1F00 008E"            /* 0._ùh...oùh.... */
	$"F968 0100 7AEF FF04 AC8D 7C6C 6CFC 6C04"            /* ùh..zïÿ.¬|llül. */
	$"4600 0063 88FA 6C03 5800 0097 F96C FF00"            /* F..cˆúl.X..—ùlÿ. */
	$"0145 77FA 6C05 3300 C9FF 6B90 F96C 020F"            /* .Ewúl.3.Éÿkùl.. */
	$"1F85 FA6C 026B 0012 F6FF 01C1 8BF9 6C02"            /* .…úl.k..öÿ.Á‹ùl. */
	$"2100 75F9 6C03 3300 007C F96C 011A 62F8"            /* !.uùl.3..|ùl..bø */
	$"FF01 8A90 FD6C FD6C 0600 30FF FF75 8888"            /* ÿ.Šılıl..0ÿÿuˆˆ */
	$"F96C 0200 2F7A F96C 020F 3A71 FA6C 0446"            /* ùl../zùl..:qúl.F */
	$"0000 3F9B F96C FF00 007C F96C FF00 03AE"            /* ..?›ùlÿ..|ùlÿ..® */
	$"9788 77FA 6C02 4600 46F9 6C03 2100 058C"            /* —ˆwúl.F.Fùl.!..Œ */
	$"F06C 0469 5034 3DC9 F7FF 038A 947A 6CFB"            /* ğl.iP4=É÷ÿ.Š”zlû */
	$"6C03 5832 3461 FA6C 0446 0079 4987 FA6C"            /* l.X24aúl.F.yI‡úl */
	$"036E 3434 53FA 6C04 5800 301C 96F9 6C02"            /* .n44Súl.X.0.–ùl. */
	$"3300 62F9 6C02 2100 71F9 6C03 2100 0090"            /* 3.bùl.!.qùl.!.. */
	$"F96C 0100 7CEF FF02 B49A 8BFA 7B04 5407"            /* ùl..|ïÿ.´š‹ú{.T. */
	$"076D 96FB 7B04 7B68 0707 A2F9 7BFF 0701"            /* .m–û{.{h..¢ù{ÿ.. */
	$"5085 FA7B 0542 07CC FF79 9CF9 7B02 1B2A"            /* P…ú{.B.Ìÿyœù{..* */
	$"92F9 7B01 071F F6FF 01C7 98F9 7B02 2E07"            /* ’ù{...öÿ.Ç˜ù{... */
	$"86F9 7B03 4207 078C F97B 0125 6CF8 FF01"            /* †ù{.B..Œù{.%løÿ. */
	$"979C F97B 0607 3AFF FF85 9695 FC7B FE7B"            /* —œù{..:ÿÿ…–•ü{ş{ */
	$"0207 3988 F97B 021B 4781 FA7B 0454 0707"            /* ..9ˆù{..Gú{.T.. */
	$"4CA6 F97B FF07 008A FA7B 067C 0707 B599"            /* L¦ù{ÿ..Šú{.|..µ™ */
	$"9685 FA7B 0254 0753 F97B 032E 0710 99F0"            /* –…ú{.T.Sù{....™ğ */
	$"7B04 7A62 474C CCF7 FF02 979F 88FA 7B03"            /* {.zbGLÌ÷ÿ.—Ÿˆú{. */
	$"683F 4272 FB7B 057B 5407 8758 95FA 7B03"            /* h?Brû{.{T.‡X•ú{. */
	$"7D42 4264 FA7B 0468 073A 29A1 F97B 0242"            /* }BBdú{.h.:)¡ù{.B */
	$"076E F97B 022E 077F F97B 032E 0707 9CF9"            /* .nù{....ù{....œù */
	$"7B01 0783 FAFF 0271 F6FF 02AA 8A77 FA65"            /* {..ƒúÿ.qöÿ.ªŠwúe */
	$"0442 0000 5F85 FA65 0352 0000 94F9 65FF"            /* .B.._…úe.R..”ùeÿ */
	$"0001 4371 FA65 052F 00C8 FF67 8DF9 6502"            /* ..Cqúe./.Èÿgùe. */
	$"0C1D 7FF9 6501 0012 F6FF 01C0 87F9 6502"            /* ...ùe...öÿ.À‡ùe. */
	$"1F00 73F9 6503 2F00 0079 F965 0016 005F"            /* ..sùe./..yùe..._ */
	$"F8FF 0186 8EF9 6506 002E FFFF 7186 84F9"            /* øÿ.†ùe...ÿÿq†„ù */
	$"6502 0033 76F9 6501 1B38 F965 0442 0000"            /* e..3vùe..8ùe.B.. */
	$"3C98 F965 FF00 007A F965 FF00 03AD 9686"            /* <˜ùeÿ..zùeÿ..­–† */
	$"71FA 6502 4200 6CF9 6503 1E00 0187 F965"            /* qúe.B.lùe....‡ùe */
	$"0149 5DF8 6500 1B01 00AD F7FF 0285 9075"            /* .I]øe....­÷ÿ.…u */
	$"FA65 035E 0006 6FFA 6504 4200 773C 84F9"            /* úe.^..oúe.B.w<„ù */
	$"65FF 0000 61FA 6504 5200 2E2E 94F9 6502"            /* eÿ..aúe.R...”ùe. */
	$"2F00 5DF9 6502 1E00 6EF9 6503 1F00 008E"            /* /.]ùe...nùe.... */
	$"F965 0100 7AEF FF04 AC8C 7A68 68FC 6804"            /* ùe..zïÿ.¬Œzhhüh. */
	$"4400 0062 87FA 6803 5500 0096 F968 FF00"            /* D..b‡úh.U..–ùhÿ. */
	$"0145 75FA 6805 3200 C9FF 6B8F F968 020F"            /* .Euúh.2.Éÿkùh.. */
	$"1F81 F968 0100 15F6 FF01 C18A F968 0221"            /* .ùh...öÿ.ÁŠùh.! */
	$"0076 F968 0332 0000 7BF9 6801 1A62 F8FF"            /* .vùh.2..{ùh..bøÿ */
	$"0188 90FD 68FD 6806 0030 FFFF 7588 86F9"            /* .ˆıhıh..0ÿÿuˆ†ù */
	$"6802 0035 79F9 6801 1D3A F968 0444 0000"            /* h..5yùh..:ùh.D.. */
	$"3E9A F968 FF00 007C F968 FF00 03AE 9788"            /* >šùhÿ..|ùhÿ..®—ˆ */
	$"75FA 6802 4500 6EF9 6803 2000 038A F968"            /* uúh.E.nùh. ..Šùh */
	$"014D 61F8 6802 1D00 AEF7 FF03 8792 7868"            /* .Maøh...®÷ÿ.‡’xh */
	$"FB68 0362 0009 71FA 6804 4500 793E 86F9"            /* ûh.b.Æqúh.E.y>†ù */
	$"68FF 0000 63FA 6804 5500 3031 96F9 6802"            /* hÿ..cúh.U.01–ùh. */
	$"3200 5FF9 6802 2000 70F9 6803 2100 0090"            /* 2._ùh. .pùh.!.. */
	$"F968 0100 7CEF FF02 B499 88FA 7904 5207"            /* ùh..|ïÿ.´™ˆúy.R. */
	$"076D 95FB 7904 7966 0707 A2F9 79FF 0701"            /* .m•ûy.yf..¢ùyÿ.. */
	$"5084 FA79 0540 07CC FF78 9BF9 7902 1B2A"            /* P„úy.@.Ìÿx›ùy..* */
	$"8FFA 7902 7A07 22F6 FF01 C798 F979 022E"            /* úy.z."öÿ.Ç˜ùy.. */
	$"0786 F979 0340 0707 8CF9 7901 256C F8FF"            /* .†ùy.@..Œùy.%løÿ */
	$"0197 9CF9 7906 073A FFFF 8597 94FC 79FE"            /* .—œùy..:ÿÿ…—”üyş */
	$"7902 0740 87F9 7901 2947 F979 0452 0707"            /* y..@‡ùy.)Gùy.R.. */
	$"4CA5 F979 FF07 008A FA79 067A 0707 B599"            /* L¥ùyÿ..Šúy.z..µ™ */
	$"9684 FA79 0253 077C F979 032C 070D 97F9"            /* –„úy.S.|ùy.,..—ù */
	$"7901 5A70 F879 0228 07B5 F7FF 0296 9F86"            /* y.Zpøy.(.µ÷ÿ.–Ÿ† */
	$"FA79 0373 0714 82FB 7905 7953 0787 4A94"            /* úy.s..‚ûy.yS.‡J” */
	$"FA79 037A 0707 72FA 7904 6607 3A42 A1F9"            /* úy.z..rúy.f.:B¡ù */
	$"7902 4007 6DF9 7902 2C07 7DF9 7903 2E07"            /* y.@.mùy.,.}ùy... */
	$"079C F979 0107 83FA FF02 980D D1A3 998F"            /* .œùy..ƒúÿ.˜.Ñ£™ */
	$"9091 9192 9294 9266 8A75 FA62 043F 0000"            /* ‘‘’’”’fŠuúb.?.. */
	$"5E84 FA62 0350 0000 94F9 62FF 0001 436F"            /* ^„úb.P..”ùbÿ..Co */
	$"FA62 052F 00C8 FF67 8DF9 6202 0C1E 7FFA"            /* úb./.Èÿgùb....ú */
	$"6202 6300 12F6 FF01 C087 F962 021F 0073"            /* b.c..öÿ.À‡ùb...s */
	$"F962 032F 0000 78F9 6200 1600 5FF8 FF01"            /* ùb./..xùb..._øÿ. */
	$"948E F962 0600 2EFF FF70 8583 F962 0200"            /* ”ùb...ÿÿp…ƒùb.. */
	$"3B75 F962 011F 38F9 6204 3F00 003B 98F9"            /* ;uùb..8ùb.?..;˜ù */
	$"62FF 0000 7AFA 6206 6300 00AD 9685 6FFA"            /* bÿ..zúb.c..­–…oú */
	$"6202 4200 6EF9 6203 1D00 0079 F962 021F"            /* b.B.nùb....yùb.. */
	$"0670 F962 0015 0100 B4F7 FF02 848F 72FA"            /* .pùb....´÷ÿ.„rú */
	$"6203 6300 296E FA62 0442 0077 3B83 FA62"            /* b.c.)núb.B.w;ƒúb */
	$"0363 0006 7DFA 6204 5000 3F31 92F9 6202"            /* .c..}úb.P.?1’ùb. */
	$"2F00 5CF9 6202 1F00 6CF9 6203 1F00 008D"            /* /.\ùb...lùb.... */
	$"F962 0100 7AFA FF0F D2A5 9B91 9294 9495"            /* ùb..zúÿ.Ò¥›‘’””• */
	$"9596 9568 8C78 6565 FC65 0443 0000 6186"            /* •–•hŒxeeüe.C..a† */
	$"FA65 0353 0000 96F9 65FF 0001 4572 FA65"            /* úe.S..–ùeÿ..Erúe */
	$"0531 00C9 FF6B 8FF9 6502 0F20 81FA 6502"            /* .1.Éÿkùe.. úe. */
	$"6600 15F6 FF01 C18A F965 0221 0077 F965"            /* f..öÿ.ÁŠùe.!.wùe */
	$"0331 0000 7BF9 6501 1A62 F8FF 0196 90FD"            /* .1..{ùe..bøÿ.–ı */
	$"65FD 6506 0030 FFFF 7387 86F9 6502 003E"            /* eıe..0ÿÿs‡†ùe..> */
	$"78F9 6501 213A F965 0443 0000 3E9A F965"            /* xùe.!:ùe.C..>šùe */
	$"FF00 007C FA65 0666 0000 AE97 8772 FA65"            /* ÿ..|úe.f..®—‡rúe */
	$"0244 0071 F965 0320 0000 7BF9 6502 2108"            /* .D.qùe. ..{ùe.!. */
	$"73F9 6502 1700 B5F7 FF03 8691 7665 FB65"            /* sùe...µ÷ÿ.†‘veûe */
	$"0366 002B 71FA 6504 4400 793E 86FA 6503"            /* .f.+qúe.D.y>†úe. */
	$"6600 0880 FA65 0453 0042 3495 F965 0231"            /* f..€úe.S.B4•ùe.1 */
	$"005F F965 0221 006E F965 0321 0000 8FF9"            /* ._ùe.!.nùe.!..ù */
	$"6501 007C FAFF 06D4 AEA6 9E9F A0A0 FDA1"            /* e..|úÿ.Ô®¦Ÿ  ı¡ */
	$"0277 9987 FA77 0451 0707 6C94 FB77 0477"            /* .w™‡úw.Q..l”ûw.w */
	$"6507 07A2 F977 FF07 0150 82FA 7705 4007"            /* e..¢ùwÿ..P‚úw.@. */
	$"CCFF 789C F977 021B 2A8F FA77 0278 0722"            /* Ìÿxœùw..*úw.x." */
	$"F6FF 01C7 98F9 7702 2E07 87F9 7703 4007"            /* öÿ.Ç˜ùw...‡ùw.@. */
	$"078B F977 0125 6CF8 FF01 A29C F977 0607"            /* .‹ùw.%løÿ.¢œùw.. */
	$"3AFF FF84 9694 FC77 FE77 0207 4A87 F977"            /* :ÿÿ„–”üwşw..J‡ùw */
	$"012E 47F9 7704 5107 074A A4F9 77FF 0700"            /* ..Gùw.Q..J¤ùwÿ.. */
	$"8AFA 7706 7807 07B5 9996 82FA 7702 5307"            /* Šúw.x..µ™–‚úw.S. */
	$"80F9 7703 2B07 078C F977 022E 1383 F977"            /* €ùw.+..Œùw...ƒùw */
	$"0222 07BA F7FF 0295 9E85 FA77 0378 0736"            /* .".º÷ÿ.•…úw.x.6 */
	$"82FB 7705 7753 0787 4A94 FA77 0378 0713"            /* ‚ûw.wS.‡J”úw.x.. */
	$"8EFA 7704 6507 4845 A1F9 7702 4007 6DF9"            /* úw.e.HE¡ùw.@.mù */
	$"7702 2E07 7CF9 7703 2E07 079C F977 0107"            /* w...|ùw....œùw.. */
	$"83FA FF02 8C03 BE90 786E FB6F 0345 007C"            /* ƒúÿ.Œ.¾xnûo.E.| */
	$"72FA 5D04 3C00 005C 83FA 5D03 4D00 0092"            /* rú].<..\ƒú].M..’ */
	$"F95D FF00 0143 6CFA 5D05 2C00 C8FF 678D"            /* ù]ÿ..Clú].,.Èÿg */
	$"F95D 020C 1E7C FA5D 025F 0012 F6FF 01C0"            /* ù]...|ú]._..öÿ.À */
	$"86F9 5D02 1F00 75F9 5D03 2C00 0078 F95D"            /* †ù]...uù].,..xù] */
	$"0016 005F F8FF 0195 8DF9 5D06 002E FFFF"            /* ..._øÿ.•ù]...ÿÿ */
	$"6F83 82F9 5D02 003B 72F9 5D01 1F38 F95D"            /* oƒ‚ù]..;rù]..8ù] */
	$"043C 0000 3A97 F95D FF00 0079 FA5D 065F"            /* .<..:—ù]ÿ..yú]._ */
	$"0000 AD96 846C FA5D 023F 0078 F95D 031C"            /* ..­–„lú].?.xù].. */
	$"0000 78F9 5D02 1F0A 7CF9 5D00 1401 00C8"            /* ..xù]..Â|ù]....È */
	$"F7FF 0281 8D70 F95D 0200 296B FA5D 043F"            /* ÷ÿ.pù]..)kú].? */
	$"0088 3B82 FA5D 035F 0007 7BFA 5D04 4D00"            /* .ˆ;‚ú]._..{ú].M. */
	$"4830 92F9 5D02 1F00 5BF9 5D02 1F00 6CF9"            /* H0’ù]...[ù]...lù */
	$"5D03 1F00 0C8D F95D 0100 7AFA FF02 BF92"            /* ]....ù]..zúÿ.¿’ */
	$"7BFA 7205 4800 7F76 6262 FC62 043F 0000"            /* {úr.H..vbbüb.?.. */
	$"5E86 FA62 0351 0000 95F9 62FF 0001 4570"            /* ^†úb.Q..•ùbÿ..Ep */
	$"FA62 0530 00C9 FF69 8FF9 6202 0F20 80FA"            /* úb.0.Éÿiùb.. €ú */
	$"6202 6400 15F6 FF01 C188 F962 0221 0078"            /* b.d..öÿ.Áˆùb.!.x */
	$"F962 0330 0000 7AF9 6201 1A62 F8FF 0197"            /* ùb.0..zùb..bøÿ.— */
	$"8FFD 62FD 6206 0030 FFFF 7286 84F9 6202"            /* ıbıb..0ÿÿr†„ùb. */
	$"003E 76F9 6201 213A F962 043F 0000 3C99"            /* .>vùb.!:ùb.?..<™ */
	$"F962 FF00 007C FA62 0664 0000 AE97 8770"            /* ùbÿ..|úb.d..®—‡p */
	$"FA62 0243 007A F962 031E 0000 7BF9 6202"            /* úb.C.zùb....{ùb. */
	$"210C 80F9 6202 1600 C9F7 FF03 848F 7362"            /* !.€ùb...É÷ÿ.„sb */
	$"FA62 0200 2B6F FA62 0443 008B 3D84 FA62"            /* úb..+oúb.C.‹=„úb */
	$"0364 0009 7FFA 6204 5100 4A33 95F9 6202"            /* .d.Æ.úb.Q.J3•ùb. */
	$"2200 5EF9 6202 2100 6EF9 6203 2100 108F"            /* ".^ùb.!.nùb.!.. */
	$"F962 0100 7CFA FF02 C49F 8AFA 8203 5707"            /* ùb..|úÿ.ÄŸŠú‚.W. */
	$"8D84 FA72 044E 0707 6B92 FB72 0472 6307"            /* „úr.N..k’ûr.rc. */
	$"07A1 F972 FF07 0150 80FA 7205 3E07 CCFF"            /* .¡ùrÿ..P€úr.>.Ìÿ */
	$"789B F972 021B 2A8E FA72 0276 0722 F6FF"            /* x›ùr..*úr.v."öÿ */
	$"01C5 97F9 7202 2E07 87F9 7203 3E07 078B"            /* .Å—ùr...‡ùr.>..‹ */
	$"F972 0125 6CF8 FF01 A29B F972 0607 3AFF"            /* ùr.%løÿ.¢›ùr..:ÿ */
	$"FF83 9492 FC72 FE72 0207 4A84 F972 012E"            /* ÿƒ”’ürşr..J„ùr.. */
	$"47F9 7204 4E07 0749 A4F9 72FF 0700 88FA"            /* Gùr.N..I¤ùrÿ..ˆú */
	$"7206 7607 07B5 9995 80FA 7202 5207 86F9"            /* r.v..µ™•€úr.R.†ù */
	$"7203 2A07 078B F972 022E 178E F972 0221"            /* r.*..‹ùr...ùr.! */
	$"07CC F7FF 0291 9B83 FA72 0373 0736 7FFB"            /* .Ì÷ÿ.‘›ƒúr.s.6.û */
	$"7205 7252 0797 4991 FA72 0376 0714 8DFA"            /* r.rR.—I‘úr.v..ú */
	$"7204 6307 5045 A0F9 7202 2F07 6CF9 7202"            /* r.c.PE ùr./.lùr. */
	$"2E07 7CF9 7203 2E07 1C9B F972 0107 83FA"            /* ..|ùr....›ùr..ƒú */
	$"FF02 8C02 B790 6CFA 5A03 3D00 5968 FA5A"            /* ÿ.Œ.·lúZ.=.YhúZ */
	$"043A 0000 5C82 FA5A 034C 0000 92F9 5AFF"            /* .:..\‚úZ.L..’ùZÿ */
	$"0001 426B FA5A 052B 00C8 FF66 8CF9 5A02"            /* ..BkúZ.+.ÈÿfŒùZ. */
	$"0C1B 7CFA 5A02 5E00 12F6 FF01 BF85 F95A"            /* ..|úZ.^..öÿ.¿…ùZ */
	$"031F 0067 62FA 5A03 2B00 0078 F95A 0016"            /* ...gbúZ.+..xùZ.. */
	$"005F F8FF 0195 8DF9 5A06 002E FFFF 6282"            /* ._øÿ.•ùZ...ÿÿb‚ */
	$"81F9 5A02 003B 70F9 5A01 1F38 F95A 043A"            /* ùZ..;pùZ..8ùZ.: */
	$"0000 3997 F95A FF00 0079 FA5A 065D 0000"            /* ..9—ùZÿ..yúZ.].. */
	$"AD96 846B FA5A 023E 006E F95A 031B 0000"            /* ­–„kúZ.>.nùZ.... */
	$"78F9 5A02 1F01 83F9 5A00 1301 00C8 F7FF"            /* xùZ...ƒùZ....È÷ÿ */
	$"027F 8B6E F95A 0200 2968 FA5A 043E 0091"            /* ..‹nùZ..)húZ.>.‘ */
	$"3A81 FA5A 035E 0007 7AFA 5A04 4C00 482E"            /* :úZ.^..zúZ.L.H. */
	$"91F9 5A01 1B00 F85A 021F 006C F95A 031F"            /* ‘ùZ...øZ...lùZ.. */
	$"0012 8CF9 5A01 007A FAFF 02B8 926F FA5E"            /* ..ŒùZ..zúÿ.¸’oú^ */
	$"0542 005B 6C5E 5EFC 5E04 3D00 005E 84FA"            /* .B.[l^^ü^.=..^„ú */
	$"5E03 4F00 0095 F95E FF00 0144 6EFA 5E05"            /* ^.O..•ù^ÿ..Dnú^. */
	$"2F00 C9FF 698E F95E 020F 1D80 FA5E 0262"            /* /.Éÿiù^...€ú^.b */
	$"0015 F6FF 01C0 88F9 5E03 2100 6B65 FA5E"            /* ..öÿ.Àˆù^.!.keú^ */
	$"032F 0000 7AF9 5E01 1A62 F8FF 0197 8FFD"            /* ./..zù^..bøÿ.—ı */
	$"5EFD 5E06 0030 FFFF 6685 83F9 5E02 003D"            /* ^ı^..0ÿÿf…ƒù^..= */
	$"73F9 5E01 213A F95E 043D 0000 3C99 F95E"            /* sù^.!:ù^.=..<™ù^ */
	$"FF00 007B FA5E 0662 0000 AE97 866E FA5E"            /* ÿ..{ú^.b..®—†nú^ */
	$"0242 0070 F95E 031D 0000 7AF9 5E02 2103"            /* .B.pù^....zù^.!. */
	$"86F9 5E02 1600 C9F7 FF03 828D 715E FA5E"            /* †ù^...É÷ÿ.‚q^ú^ */
	$"0200 2B6D FA5E 0442 0094 3C83 FA5E 0362"            /* ..+mú^.B.”<ƒú^.b */
	$"0009 7DFA 5E04 4F00 4A31 94F9 5E02 1D00"            /* .Æ}ú^.O.J1”ù^... */
	$"5DF9 5E02 2100 6EF9 5E03 2100 158E F95E"            /* ]ù^.!.nù^.!..ù^ */
	$"0100 7CFA FF02 BE9F 7FFA 7003 4F07 687C"            /* ..|úÿ.¾Ÿ.úp.O.h| */
	$"FA70 044D 0707 6991 FB70 0470 6107 07A1"            /* úp.M..i‘ûp.pa..¡ */
	$"F970 FF07 014F 7DFA 7005 3D07 CCFF 789B"            /* ùpÿ..O}úp.=.Ìÿx› */
	$"F970 021B 288D FA70 0273 0722 F6FF 01C5"            /* ùp..(úp.s."öÿ.Å */
	$"96F9 7003 2E07 7A77 FA70 033D 0707 8BF9"            /* –ùp...zwúp.=..‹ù */
	$"7001 256C F8FF 01A2 9BF9 7006 073A FFFF"            /* p.%løÿ.¢›ùp..:ÿÿ */
	$"7892 91FC 70FE 7002 0749 83F9 7001 2E47"            /* x’‘üpşp..Iƒùp..G */
	$"F970 044D 0707 48A4 F970 FF07 0088 FA70"            /* ùp.M..H¤ùpÿ..ˆúp */
	$"0673 0707 B599 957D FA70 0251 077B F970"            /* .s..µ™•}úp.Q.{ùp */
	$"0329 0707 8BF9 7002 2E0D 94F9 7002 2107"            /* .)..‹ùp...”ùp.!. */
	$"CCF7 FF02 909A 81F9 7002 0736 7CFB 7005"            /* Ì÷ÿ.šùp..6|ûp. */
	$"7051 079F 4990 FA70 0373 0714 8CFA 7004"            /* pQ.ŸIúp.s..Œúp. */
	$"6107 5043 A0F9 7002 2907 6BF9 7002 2E07"            /* a.PC ùp.).kùp... */
	$"7BF9 7003 2E07 229B F970 0107 83FA FF02"            /* {ùp..."›ùp..ƒúÿ. */
	$"8A02 B290 69FA 5802 4200 4AF9 5804 3900"            /* Š.²iúX.B.JùX.9. */
	$"005B 81FA 5803 4900 0091 F958 FF00 0142"            /* .[úX.I..‘ùXÿ..B */
	$"68FA 5805 2A00 C8FF 668C F958 020C 127B"            /* húX.*.ÈÿfŒùX...{ */
	$"FA58 025C 0027 F6FF 01BF 85F9 5803 1F00"            /* úX.\.'öÿ.¿…ùX... */
	$"676E FA58 032A 0000 77F9 5800 1600 5FF8"            /* gnúX.*..wùX..._ø */
	$"FF01 948C F958 0600 2EFF FF62 8180 F958"            /* ÿ.”ŒùX...ÿÿb€ùX */
	$"020C 3B6E F958 011F 38F9 5804 3900 0038"            /* ..;nùX..8ùX.9..8 */
	$"96F9 58FF 0000 78FA 5806 5B00 00AD 9683"            /* –ùXÿ..xúX.[..­–ƒ */
	$"68FA 5802 3D00 6CF9 5803 1A00 0078 F958"            /* húX.=.lùX....xùX */
	$"021F 0091 F958 0013 0100 D1F7 FF02 7D8A"            /* ...‘ùX....Ñ÷ÿ.}Š */
	$"6CF9 5802 0029 67FA 5804 3D00 9139 7FFA"            /* lùX..)gúX.=.‘9.ú */
	$"5803 5C00 0779 FA58 0449 0048 2090 F958"            /* X.\..yúX.I.H ùX */
	$"021A 0065 F958 021F 006B F958 031F 0012"            /* ...eùX...kùX.... */
	$"8CF9 5801 007A FAFF 02B3 926D FA5B 0245"            /* ŒùX..zúÿ.³’mú[.E */
	$"004D FE5B FC5B 043B 0000 5D83 FA5B 034D"            /* .Mş[ü[.;..]ƒú[.M */
	$"0000 94F9 5BFF 0001 446C FA5B 052E 00C9"            /* ..”ù[ÿ..Dlú[...É */
	$"FF69 8EF9 5B02 0F14 7DFA 5B02 5F00 29F6"            /* ÿiù[...}ú[._.)ö */
	$"FF01 C087 F95B 0321 0069 71FA 5B03 2E00"            /* ÿ.À‡ù[.!.iqú[... */
	$"007A F95B 011A 62F8 FF01 968E FD5B FD5B"            /* .zù[..bøÿ.–ı[ı[ */
	$"0600 30FF FF65 8482 F95B 020F 3D71 F95B"            /* ..0ÿÿe„‚ù[..=qù[ */
	$"0121 3BF9 5B04 3B00 003B 98F9 5BFF 0000"            /* .!;ù[.;..;˜ù[ÿ.. */
	$"7BFA 5B06 5E00 00AE 9785 6CFA 5B02 4000"            /* {ú[.^..®—…lú[.@. */
	$"6EF9 5B03 1C00 007A F95B 0221 0094 F95B"            /* nù[....zù[.!.”ù[ */
	$"0215 00D2 F7FF 0380 8C6F 5BFA 5B02 022B"            /* ...Ò÷ÿ.€Œo[ú[..+ */
	$"69FA 5B04 4000 943C 82FA 5B03 5F00 097C"            /* iú[.@.”<‚ú[._.Æ| */
	$"FA5B 044D 004A 2492 F95B 021C 0067 F95B"            /* ú[.M.J$’ù[...gù[ */
	$"0221 006E F95B 0321 0015 8EF9 5B01 007C"            /* .!.nù[.!..ù[..| */
	$"FAFF 02BA 9F7D FA6D 0255 075B F96D 044A"            /* úÿ.ºŸ}úm.U.[ùm.J */
	$"0707 6890 FB6D 046D 5E07 07A0 F96D FF07"            /* ..hûm.m^.. ùmÿ. */
	$"014F 7CFA 6D05 3D07 CCFF 779B F96D 021B"            /* .O|úm.=.Ìÿw›ùm.. */
	$"208C FA6D 0272 0734 F6FF 01C5 96F9 6D03"            /*  Œúm.r.4öÿ.Å–ùm. */
	$"2E07 7881 FA6D 033D 0707 8AF9 6D01 256C"            /* ..xúm.=..Šùm.%l */
	$"F8FF 01A1 9BF9 6D06 073A FFFF 7791 90FC"            /* øÿ.¡›ùm..:ÿÿw‘ü */
	$"6DFE 6D02 1B49 81F9 6D01 2E47 F96D 044A"            /* mşm..Iùm..Gùm.J */
	$"0707 47A3 F96D FF07 0087 FA6D 0671 0707"            /* ..G£ùmÿ..‡úm.q.. */
	$"B599 947C FA6D 0250 077A F96D 0328 0707"            /* µ™”|úm.P.zùm.(.. */
	$"8BF9 6D02 2E07 A0F9 6D02 2007 D4F7 FF02"            /* ‹ùm... ùm. .Ô÷ÿ. */
	$"8F99 7FF9 6D02 0B35 7AFB 6D05 6D50 079F"            /* ™.ùm..5zûm.mP.Ÿ */
	$"488F FA6D 0372 0714 8BFA 6D04 5E07 5033"            /* Húm.r..‹úm.^.P3 */
	$"9FF9 6D02 2807 73F9 6D02 2E07 7BF9 6D03"            /* Ÿùm.(.sùm...{ùm. */
	$"2E07 229B F96D 0107 83FA FF02 8702 A690"            /* .."›ùm..ƒúÿ.‡.¦ */
	$"67FA 5402 4C00 4AF9 5404 3600 0059 7FFA"            /* gúT.L.JùT.6..Y.ú */
	$"5403 4700 0091 F954 FF00 0142 66FA 5405"            /* T.G..‘ùTÿ..BfúT. */
	$"2900 C8FF 668C F954 020C 1278 FA54 025A"            /* ).ÈÿfŒùT...xúT.Z */
	$"002E F6FF 01BF 85F9 5403 1F00 676C FA54"            /* ..öÿ.¿…ùT...glúT */
	$"0329 000C 77F9 5400 1600 5FF8 FF01 928C"            /* .)..wùT..._øÿ.’Œ */
	$"F954 0600 2EFF FF5F 807F F954 020C 3B5C"            /* ùT...ÿÿ_€.ùT..;\ */
	$"F954 011D 39F9 5404 3600 0038 95F9 54FF"            /* ùT..9ùT.6..8•ùTÿ */
	$"0000 78FA 5406 5900 00AD 9682 66FA 5402"            /* ..xúT.Y..­–‚fúT. */
	$"3C00 6CF9 5403 1900 0077 F954 021F 008F"            /* <.lùT....wùT... */
	$"F954 0012 0000 F6FF 027A 8669 F954 0209"            /* ùT....öÿ.z†iùT.Æ */
	$"2965 FA54 043C 0091 397D FA54 035A 0007"            /* )eúT.<.‘9}úT.Z.. */
	$"78FA 5404 4700 482B 90F9 5402 1900 5EF9"            /* xúT.G.H+ùT...^ù */
	$"5402 1F00 69F9 5403 1F00 128C F954 0100"            /* T...iùT....ŒùT.. */
	$"7AFA FF02 A892 6BFA 5802 4F00 4DFE 58FC"            /* zúÿ.¨’kúX.O.MşXü */
	$"5804 3A00 005B 82FA 5803 4A00 0094 F958"            /* X.:..[‚úX.J..”ùX */
	$"FF00 0144 69FA 5805 2C00 C9FF 688E F958"            /* ÿ..DiúX.,.ÉÿhùX */
	$"020F 147B FA58 025D 0030 F6FF 01C0 87F9"            /* ...{úX.].0öÿ.À‡ù */
	$"5803 2100 696F FA58 032C 0010 79F9 5801"            /* X.!.ioúX.,..yùX. */
	$"1A62 F8FF 0195 8EFD 58FD 5806 0030 FFFF"            /* .bøÿ.•ıXıX..0ÿÿ */
	$"6483 81F9 5802 0F3D 5FF9 5801 203C F958"            /* dƒùX..=_ùX. <ùX */
	$"043A 0000 3A97 F958 FF00 007A FA58 065C"            /* .:..:—ùXÿ..zúX.\ */
	$"0000 AE97 8469 FA58 023F 006E F958 031C"            /* ..®—„iúX.?.nùX.. */
	$"0000 7AF9 5802 2100 91F9 5801 1400 F6FF"            /* ..zùX.!.‘ùX...öÿ */
	$"037D 8A6D 58FA 5802 0B2B 68FA 5804 3F00"            /* .}ŠmXúX..+húX.?. */
	$"943B 81FA 5803 5D00 097B FA58 044A 004A"            /* ”;úX.].Æ{úX.J.J */
	$"2F92 F958 021C 0061 F958 0221 006D F958"            /* /’ùX...aùX.!.mùX */
	$"0321 0015 8EF9 5801 007C FAFF 02B0 9F7B"            /* .!..ùX..|úÿ.°Ÿ{ */
	$"FA6B 0262 075B F96B 0448 0707 678F FB6B"            /* úk.b.[ùk.H..gûk */
	$"046B 5D07 07A0 F96B FF07 014F 7AFA 6B05"            /* .k].. ùkÿ..Ozúk. */
	$"3C07 CCFF 779A F96B 021B 208A FA6B 0270"            /* <.Ìÿwšùk.. Šúk.p */
	$"073A F6FF 01C4 95F9 6B03 2E07 787F FA6B"            /* .:öÿ.Ä•ùk...x.úk */
	$"033C 071C 8AF9 6B01 256C F8FF 01A1 9BF9"            /* .<..Šùk.%løÿ.¡›ù */
	$"6B06 073A FFFF 7691 8FFC 6BFE 6B02 1B49"            /* k..:ÿÿv‘ükşk..I */
	$"71F9 6B01 2B48 F96B 0448 0707 47A2 F96B"            /* qùk.+Hùk.H..G¢ùk */
	$"FF07 0087 FA6B 066F 0707 B599 927A FA6B"            /* ÿ..‡úk.o..µ™’zúk */
	$"024F 0779 F96B 0327 0707 8AF9 6B02 2E07"            /* .O.yùk.'..Šùk... */
	$"9EF9 6B01 1F07 F6FF 028D 977C F96B 0216"            /* ùk...öÿ.—|ùk.. */
	$"3579 FB6B 056B 4F07 9F48 8EFA 6B03 7007"            /* 5yûk.kO.ŸHúk.p. */
	$"148A FA6B 045D 0750 3D9F F96B 0227 076E"            /* .Šúk.].P=Ÿùk.'.n */
	$"F96B 022E 077A F96B 032E 0722 9BF9 6B01"            /* ùk...zùk..."›ùk. */
	$"0783 FAFF 0284 02A1 8F65 FA51 0258 0049"            /* .ƒúÿ.„.¡eúQ.X.I */
	$"F951 0434 0000 587D FA51 0345 0000 90F9"            /* ùQ.4..X}úQ.E..ù */
	$"51FF 0001 4064 FA51 0528 00C8 FF65 8BF9"            /* Qÿ..@dúQ.(.Èÿe‹ù */
	$"5102 0C11 78FA 5102 5800 2EF6 FF01 BF84"            /* Q...xúQ.X..öÿ.¿„ */
	$"F951 031F 005E 69FA 5103 2800 1276 F951"            /* ùQ...^iúQ.(..vùQ */
	$"0016 005F F8FF 0191 8CF9 5106 002E FFFF"            /* ..._øÿ.‘ŒùQ...ÿÿ */
	$"5E7F 7CF9 5101 0C3A F851 0117 39F9 5104"            /* ^.|ùQ..:øQ..9ùQ. */
	$"3400 0036 95F9 51FF 0000 71FA 5106 5700"            /* 4..6•ùQÿ..qúQ.W. */
	$"00AD 9681 64FA 5102 3B00 62F9 5103 1700"            /* .­–dúQ.;.bùQ... */
	$"0077 F951 021F 0082 F951 0011 0000 F6FF"            /* .wùQ...‚ùQ....öÿ */
	$"0278 8567 F951 0209 2062 FA51 043B 0091"            /* .x…gùQ.Æ búQ.;.‘ */
	$"387C FA51 0358 0007 77FA 5104 4500 503B"            /* 8|úQ.X..wúQ.E.P; */
	$"8FF9 5102 1700 5DF9 5102 1000 67F9 5103"            /* ùQ...]ùQ...gùQ. */
	$"1F00 128B F951 0100 7AFA FF02 A291 69FA"            /* ...‹ùQ..zúÿ.¢‘iú */
	$"5502 5C00 4DFE 55FC 5504 3800 005A 81FA"            /* U.\.MşUüU.8..Zú */
	$"5503 4900 0092 F955 FF00 0143 67FA 5505"            /* U.I..’ùUÿ..CgúU. */
	$"2B00 C9FF 688D F955 020F 147A FA55 025C"            /* +.ÉÿhùU...zúU.\ */
	$"0030 F6FF 01C0 86F9 5503 2100 616E FA55"            /* .0öÿ.À†ùU.!.anúU */
	$"032B 0015 79F9 5501 1A62 F8FF 0194 8EFD"            /* .+..yùU..bøÿ.”ı */
	$"55FD 5506 0030 FFFF 6381 80F9 5501 0F3C"            /* UıU..0ÿÿc€ùU..< */
	$"F855 011B 3CF9 5504 3800 003A 97F9 55FF"            /* øU..<ùU.8..:—ùUÿ */
	$"0000 73FA 5506 5B00 00AE 9784 67FA 5502"            /* ..súU.[..®—„gúU. */
	$"3E00 64F9 5503 1B00 0079 F955 0221 0085"            /* >.dùU....yùU.!.… */
	$"F955 0114 00F6 FF03 7B87 6B55 FA55 020B"            /* ùU...öÿ.{‡kUúU.. */
	$"2466 FA55 043E 0094 3B7F FA55 035C 0009"            /* $fúU.>.”;.úU.\.Æ */
	$"7AFA 5504 4900 523D 91F9 5502 1B00 61F9"            /* zúU.I.R=‘ùU...aù */
	$"5502 1300 69F9 5503 2100 158D F955 0100"            /* U...iùU.!..ùU.. */
	$"7CFA FF02 AC9E 7AFA 6802 6F07 5BF9 6804"            /* |úÿ.¬zúh.o.[ùh. */
	$"4707 0766 8EFB 6804 685B 0707 9FF9 68FF"            /* G..fûh.h[..Ÿùhÿ */
	$"0701 4F79 FA68 053B 07CC FF77 9AF9 6802"            /* ..Oyúh.;.Ìÿwšùh. */
	$"1B20 8AFA 6802 6F07 3AF6 FF01 C495 F968"            /* . Šúh.o.:öÿ.Ä•ùh */
	$"032E 076E 7DFA 6803 3B07 2288 F968 0125"            /* ...n}úh.;."ˆùh.% */
	$"6CF8 FF01 A09A F968 0607 3AFF FF75 908D"            /* løÿ. šùh..:ÿÿu */
	$"FC68 FE68 011B 49F8 6801 2748 F968 0447"            /* ühşh..Iøh.'Hùh.G */
	$"0707 46A2 F968 FF07 0080 FA68 066E 0707"            /* ..F¢ùhÿ..€úh.n.. */
	$"B599 9179 FA68 024E 0770 F968 0327 0707"            /* µ™‘yúh.N.pùh.'.. */
	$"8AF9 6802 2E07 94F9 6801 1F07 F6FF 028B"            /* Šùh...”ùh...öÿ.‹ */
	$"957B F968 0216 2F77 FB68 0568 4E07 9F47"            /* •{ùh../wûh.hN.ŸG */
	$"8DFA 6803 6F07 148A FA68 045B 0759 499E"            /* úh.o..Šúh.[.YI */
	$"F968 0227 076E F968 021F 0778 F968 032E"            /* ùh.'.nùh...xùh.. */
	$"0722 9AF9 6801 0783 FAFF 0281 0299 8D64"            /* ."šùh..ƒúÿ..™d */
	$"F94E 0100 49F9 4E04 3200 0057 7BFA 4E03"            /* ùN..IùN.2..W{úN. */
	$"4300 0085 F94E FF00 0140 62FA 4E05 2700"            /* C..…ùNÿ..@búN.'. */
	$"C8FF 658B F94E 020C 0677 FA4E 0255 002E"            /* Èÿe‹ùN...wúN.U.. */
	$"F6FF 01BE 83F9 4E03 1F00 5A68 FA4E 0327"            /* öÿ.¾ƒùN...ZhúN.' */
	$"0012 75F9 4E00 1600 5FF8 FF01 908B F94E"            /* ..uùN..._øÿ.‹ùN */
	$"0600 2EFF FF5E 7C7B F94E 010C 3AF8 4E01"            /* ...ÿÿ^|{ùN..:øN. */
	$"1639 F94E 0432 0000 3692 F94E FF00 006C"            /* .9ùN.2..6’ùNÿ..l */
	$"FA4E 0654 0000 AD96 8162 FA4E 023A 005C"            /* úN.T..­–búN.:.\ */
	$"F94E 0316 0000 76F9 4E02 1F00 7AF9 4E00"            /* ùN....vùN...zùN. */
	$"1000 02F6 FF02 7683 64F9 4EFF 1600 5FFA"            /* ...öÿ.vƒdùNÿ.._ú */
	$"4E04 3A00 912E 7AFA 4E03 5500 0776 FA4E"            /* N.:.‘.zúN.U..vúN */
	$"0443 005F 3B8F F94E 0221 005D F94E 020C"            /* .C._;ùN.!.]ùN.. */
	$"005F F94E 031F 0017 8BF9 4E01 007A FAFF"            /* ._ùN....‹ùN..zúÿ */
	$"029B 8F67 F952 0100 4DFE 52FC 5204 3500"            /* .›gùR..MşRüR.5. */
	$"0059 7FFA 5203 4700 0088 F952 FF00 0143"            /* .Y.úR.G..ˆùRÿ..C */
	$"65FA 5205 2A00 C9FF 688D F952 020F 097A"            /* eúR.*.ÉÿhùR..Æz */
	$"FA52 025A 0030 F6FF 01BF 85F9 5203 2100"            /* úR.Z.0öÿ.¿…ùR.!. */
	$"5C6C FA52 032A 0015 78F9 5201 1A62 F8FF"            /* \lúR.*..xùR..bøÿ */
	$"0194 8DFD 52FD 5206 0030 FFFF 6280 7DF9"            /* .”ıRıR..0ÿÿb€}ù */
	$"5201 0F3C F852 011A 3CF9 5204 3500 0039"            /* R..<øR..<ùR.5..9 */
	$"95F9 52FF 0000 6EFA 5206 5800 00AE 9784"            /* •ùRÿ..núR.X..®—„ */
	$"65FA 5202 3D00 5EF9 5203 1A00 0079 F952"            /* eúR.=.^ùR....yùR */
	$"0221 007D F952 0113 06F6 FF03 7986 6852"            /* .!.}ùR...öÿ.y†hR */
	$"FA52 FF1A 0064 FA52 043D 0094 307D FA52"            /* úRÿ..dúR.=.”0}úR */
	$"035A 0009 79FA 5204 4700 623D 91F9 5202"            /* .Z.ÆyúR.G.b=‘ùR. */
	$"2400 5FF9 5202 0F00 62F9 5203 2100 1B8D"            /* $._ùR...bùR.!.. */
	$"F952 0100 7CFA FF02 A69B 78F9 6501 075B"            /* ùR..|úÿ.¦›xùe..[ */
	$"F965 0445 0707 658D FB65 0465 5A07 0796"            /* ùe.E..eûe.eZ..– */
	$"F965 FF07 014E 77FA 6505 3A07 CCFF 7699"            /* ùeÿ..Nwúe.:.Ìÿv™ */
	$"F965 021B 1388 FA65 026D 073A F6FF 01C4"            /* ùe...ˆúe.m.:öÿ.Ä */
	$"94F9 6503 2E07 687C FA65 033A 0722 88F9"            /* ”ùe...h|úe.:."ˆù */
	$"6501 256C F8FF 019F 9AF9 6506 073A FFFF"            /* e.%løÿ.Ÿšùe..:ÿÿ */
	$"758E 8CFC 65FE 6501 1B49 F865 0126 48F9"            /* uŒüeşe..Iøe.&Hù */
	$"6504 4507 0746 A1F9 65FF 0700 79FA 6506"            /* e.E..F¡ùeÿ..yúe. */
	$"6C07 07B5 9991 77FA 6502 4E07 6DF9 6503"            /* l..µ™‘wúe.N.mùe. */
	$"2607 0788 F965 022E 078D F965 011E 10F6"            /* &..ˆùe...ùe...ö */
	$"FF02 8894 79F9 6502 2527 76FB 6505 654E"            /* ÿ.ˆ”yùe.%'vûe.eN */
	$"079F 3C8C FA65 036D 0714 88FA 6504 5A07"            /* .Ÿ<Œúe.m..ˆúe.Z. */
	$"6C49 9EF9 6502 2F07 6DF9 6502 1B07 6EF9"            /* lIùe./.mùe...nù */
	$"6503 2E07 279A F965 0107 83FA FF02 9F02"            /* e...'šùe..ƒúÿ.Ÿ. */
	$"8E8B 62F9 4A01 0C51 F94A 0430 0000 557A"            /* ‹bùJ..QùJ.0..Uz */
	$"FA4A 0340 051B 80F9 4AFF 0001 405F FA4A"            /* úJ.@..€ùJÿ..@_úJ */
	$"0526 00C8 FF64 8AF9 4A02 0C06 76FA 4A02"            /* .&.ÈÿdŠùJ...vúJ. */
	$"5300 2EF6 FF01 BE82 F94A 031F 005A 65FA"            /* S..öÿ.¾‚ùJ...Zeú */
	$"4A03 2600 1275 F94A 0016 0B20 5F75 8A91"            /* J.&..uùJ... _uŠ‘ */
	$"918F 8AFF FF8F 8BF9 4A06 002E FFFF 487B"            /* ‘Šÿÿ‹ùJ...ÿÿH{ */
	$"79F9 4A01 1044 F84A 0116 47F9 4A04 3000"            /* yùJ..DøJ..GùJ.0. */
	$"0035 92F9 4A02 0015 72FA 4A06 5100 00AD"            /* .5’ùJ...rúJ.Q..­ */
	$"9680 5FFA 4A02 3900 55F9 4A03 1500 0675"            /* –€_úJ.9.UùJ....u */
	$"F94A 021F 0073 F94A 0010 0012 F6FF 0273"            /* ùJ...sùJ....öÿ.s */
	$"8162 F94A 0233 1F5D FA4A 0439 00A6 2979"            /* bùJ.3.]úJ.9.¦)y */
	$"FA4A 0353 0007 75FA 4A04 4000 5F3B 8EF9"            /* úJ.S..uúJ.@._;ù */
	$"4A02 1F00 5CF9 4A02 0C00 59F9 4A03 1F00"            /* J...\ùJ...YùJ... */
	$"2E8B F94A 0B00 2B63 798C 9191 8A9F 908D"            /* .‹ùJ..+cyŒ‘‘ŠŸ */
	$"65F9 4F01 0F53 FE4F FC4F 0433 0000 587C"            /* eùO..SşOüO.3..X| */
	$"FA4F 0345 071E 83F9 4FFF 0001 4363 FA4F"            /* úO.E..ƒùOÿ..CcúO */
	$"0529 00C9 FF67 8CF9 4F02 0F09 78FA 4F02"            /* .).ÉÿgŒùO..ÆxúO. */
	$"5800 30F6 FF01 BF85 F94F 0321 005C 68FA"            /* X.0öÿ.¿…ùO.!.\hú */
	$"4F03 2900 1577 F94F 0C1A 2262 778C 9494"            /* O.)..wùO.."bwŒ”” */
	$"918B FFFF 928D FD4F FD4F 0600 30FF FF4C"            /* ‘‹ÿÿ’ıOıO..0ÿÿL */
	$"7F7C F94F 0112 46F8 4F01 1A49 F94F 0433"            /* .|ùO..FøO..IùO.3 */
	$"0000 3895 F94F 0202 1775 FA4F 0655 0000"            /* ..8•ùO...uúO.U.. */
	$"AE97 8363 FA4F 023C 0058 F94F 0319 0008"            /* ®—ƒcúO.<.XùO.... */
	$"78F9 4F02 2100 77F9 4F01 1215 F6FF 0377"            /* xùO.!.wùO...öÿ.w */
	$"8465 4FFA 4F02 3622 61FA 4F04 3C00 A82B"            /* „eOúO.6"aúO.<.¨+ */
	$"7BFA 4F03 5800 0978 FA4F 0445 0062 3D90"            /* {úO.X.ÆxúO.E.b= */
	$"F94F 0221 005F F94F 020F 005B F94F 0321"            /* ùO.!._ùO...[ùO.! */
	$"0030 8DF9 4F0B 002E 657B 8F94 948C A09C"            /* .0ùO...e{””Œ œ */
	$"9A77 F963 001B F863 0444 0707 648B FB63"            /* šwùc..øc.D..d‹ûc */
	$"0463 5812 2A91 F963 FF07 014E 75FA 6305"            /* .cX.*‘ùcÿ..Nuúc. */
	$"3907 CCFF 7699 F963 021B 1387 FA63 026B"            /* 9.Ìÿv™ùc...‡úc.k */
	$"073A F6FF 01C4 94F9 6303 2E07 6879 FA63"            /* .:öÿ.Ä”ùc...hyúc */
	$"0339 0722 87F9 630C 252C 6C81 99A0 A09E"            /* .9."‡ùc.%,l™   */
	$"94FF FF9E 99F9 6306 073A FFFF 5A8D 8BFC"            /* ”ÿÿ™ùc..:ÿÿZ‹ü */
	$"63FE 6301 1F54 F863 0126 57F9 6304 4407"            /* cşc..Tøc.&Wùc.D. */
	$"0745 A0F9 6302 0C25 81FA 6306 6907 07B5"            /* .E ùc..%úc.i..µ */
	$"9991 75FA 6302 4D07 65F9 6303 2507 1388"            /* ™‘uúc.M.eùc.%..ˆ */
	$"F963 022E 0785 F963 011E 22F6 FF02 8692"            /* ùc...…ùc.."öÿ.†’ */
	$"77F9 6302 4432 73FB 6305 634D 07AF 368A"            /* wùc.D2sûc.cM.¯6Š */
	$"FA63 036B 0714 87FA 6304 5807 6C49 9CF9"            /* úc.k..‡úc.X.lIœù */
	$"6302 2E07 6DF9 6302 1B07 67F9 6303 2E07"            /* c...mùc...gùc... */
	$"3A99 F963 0807 386E 859B A0A0 98A4 024C"            /* :™ùc..8n…›  ˜¤.L */
	$"0275 6855 EF48 0534 0000 3E5F 4CF0 4804"            /* .uhUïH.4..>_LğH. */
	$"4700 003F 5DFA 4805 2600 C8FF 648A F948"            /* G..?]úH.&.ÈÿdŠùH */
	$"020C 0583 FA48 0252 003B F6FF 01BE 82F9"            /* ...ƒúH.R.;öÿ.¾‚ù */
	$"4803 1F00 5062 FA48 0326 0012 75F9 4800"            /* H...PbúH.&..uùH. */
	$"4600 4FFC 5105 1147 FFFF 8F8B F948 0600"            /* F.OüQ..Gÿÿ‹ùH.. */
	$"2EFF FF47 7379 E448 052E 0000 2867 53F0"            /* .ÿÿGsyäH....(gSğ */
	$"4806 4400 00AF 9680 5DFA 4803 3900 4D64"            /* H.D..¯–€]úH.9.Md */
	$"FA48 0315 0012 75F9 4803 1F00 6759 FA48"            /* úH....uùH...gYúH */
	$"000F 0012 F6FF 037C 4545 4CF9 4801 524C"            /* ....öÿ.|EELùH.RL */
	$"FA48 0439 00AD 2977 FA48 0352 0007 75FA"            /* úH.9.­)wúH.R..uú */
	$"4804 3F00 5F3B 8DF9 4802 1F00 5BF9 4802"            /* H.?._;ùH...[ùH. */
	$"0C00 58F9 4803 1F00 2E8A F948 0042 FB51"            /* ..XùH....ŠùH.BûQ */
	$"0400 7077 6C59 F44C FC4C 0536 0000 4262"            /* ..pwlYôLüL.6..Bb */
	$"4FF0 4C04 4A00 0042 61FA 4C05 2900 C9FF"            /* OğL.J..BaúL.).Éÿ */
	$"678C F94C 020F 0885 FA4C 0255 003D F6FF"            /* gŒùL...…úL.U.=öÿ */
	$"01BF 84F9 4C03 2100 5265 FA4C 0329 0015"            /* .¿„ùL.!.ReúL.).. */
	$"77F9 4C01 4953 FC54 0513 49FF FF92 8DFD"            /* wùL.ISüT..Iÿÿ’ı */
	$"4CFD 4C06 0030 FFFF 4A77 7CE4 4C05 3000"            /* LıL..0ÿÿJw|äL.0. */
	$"002A 6B57 F04C 0647 0000 B097 8261 FA4C"            /* .*kWğL.G..°—‚aúL */
	$"033C 004F 67FA 4C03 1700 1577 F94C 0321"            /* .<.OgúL....wùL.! */
	$"0069 5BFA 4C01 1215 F6FF 037F 4848 4FF9"            /* .i[úL...öÿ..HHOù */
	$"4C01 554F FA4C 043C 00AE 2B7A FA4C 0355"            /* L.UOúL.<.®+zúL.U */
	$"0009 77FA 4C04 4300 623D 8FF9 4C02 2100"            /* .ÆwúL.C.b=ùL.!. */
	$"5EF9 4C02 0F00 5AF9 4C03 2100 308C F94C"            /* ^ùL...ZùL.!.0ŒùL */
	$"0046 FB54 0403 7186 7A6B EF5F 0546 0707"            /* .FûT..q†zkï_.F.. */
	$"4F71 64FC 5FF5 5F04 5D07 074E 73FA 5F05"            /* Oqdü_õ_.]..Nsú_. */
	$"3907 CCFF 7699 F95F 021B 1294 FA5F 0269"            /* 9.Ìÿv™ù_...”ú_.i */
	$"0745 F6FF 01C4 92F9 5F03 2E07 5F77 FA5F"            /* .Eöÿ.Ä’ù_..._wú_ */
	$"0339 0722 87F9 5F01 5D67 FC68 0520 53FF"            /* .9."‡ù_.]güh. Sÿ */
	$"FF9E 99F9 5F06 073A FFFF 5B87 8CFC 5FE9"            /* ÿ™ù_..:ÿÿ[‡Œü_é */
	$"5F05 3F07 0738 7969 F05F 065B 0707 B799"            /* _.?..8yiğ_.[..·™ */
	$"9173 FA5F 034C 075B 79FA 5F03 2507 2287"            /* ‘sú_.L.[yú_.%."‡ */
	$"F95F 032E 0778 6EFA 5F01 1D22 F6FF 038E"            /* ù_...xnú_.."öÿ. */
	$"5A57 63F9 5F01 6964 FB5F 055F 4C07 B535"            /* ZWcù_.idû_._L.µ5 */
	$"8AFA 5F03 6907 1486 FA5F 0457 076C 499C"            /* Šú_.i..†ú_.W.lIœ */
	$"F95F 022E 076C F95F 021B 0766 F95F 032E"            /* ù_...lù_...fù_.. */
	$"073A 99F9 5F00 5AFB 6801 0C76 024D 04AD"            /* .:™ù_.Zûh..v.M.­ */
	$"1D1A 3C49 F344 084C 3300 0090 1D22 344C"            /* ..<IóD.L3..."4L */
	$"F144 0407 0045 3F5B FA44 0524 00C8 FF64"            /* ñD...E?[úD.$.Èÿd */
	$"88F9 4402 0C00 91FA 4402 4F00 48F6 FF01"            /* ˆùD...‘úD.O.Höÿ. */
	$"BD81 F944 031F 0048 5FFA 4403 2400 2473"            /* ½ùD...H_úD.$.$s */
	$"F844 FB44 050C 47FF FF8E 88F9 4408 002E"            /* øDûD..GÿÿˆùD... */
	$"FFFF 943D 2C31 4EE7 4407 3000 029B 381F"            /* ÿÿ”=,1NçD.0..›8. */
	$"2F49 F244 0743 0A00 6BFF 967D 5AFA 4403"            /* /IòD.CÂ.kÿ–}ZúD. */
	$"3600 4362 FA44 0313 0017 73F9 4403 1F00"            /* 6.CbúD....sùD... */
	$"585A FA44 000D 0024 F6FF 05E1 6317 1935"            /* XZúD...$öÿ.ác..5 */
	$"49F2 4404 3600 AD28 75FA 4403 4F00 0671"            /* IòD.6.­(uúD.O..q */
	$"FA44 043C 005F 3B8B F944 021F 0059 F944"            /* úD.<._;‹ùD...YùD */
	$"030C 0057 4EFA 4403 1F00 2C88 F244 0600"            /* ...WNúD...,ˆòD.. */
	$"70AE 1F1D 3F4E F648 FE48 0850 3600 0091"            /* p®..?NöHşH.P6..‘ */
	$"1F25 3850 F248 0547 0900 4842 5EFA 4805"            /* .%8PòH.GÆ.HB^úH. */
	$"2700 C9FF 678B F948 020F 0094 FA48 0253"            /* '.Éÿg‹ùH...”úH.S */
	$"004A F6FF 01BE 84F9 4803 2100 4A63 FA48"            /* .Jöÿ.¾„ùH.!.JcúH */
	$"0327 0027 76F2 4805 0F49 FFFF 908B FD48"            /* .'.'vòH..Iÿÿ‹ıH */
	$"FD48 0800 30FF FF96 4030 3552 E748 0733"            /* ıH..0ÿÿ–@05RçH.3 */
	$"0006 9E3A 2132 4EF2 4807 470C 006D FF97"            /* ..:!2NòH.G..mÿ— */
	$"815E FA48 033A 0045 65FA 4803 1600 1B77"            /* ^úH.:.EeúH....w */
	$"F948 0321 005A 5EFA 4801 1127 F6FF 03E1"            /* ùH.!.Z^úH..'öÿ.á */
	$"651A 1C01 394E F248 043A 00AE 2A78 FA48"            /* e...9NòH.:.®*xúH */
	$"0353 0009 75FA 4804 4000 623D 8DF9 4802"            /* .S.ÆuúH.@.b=ùH. */
	$"2100 5BF9 4803 0F00 5952 FA48 0321 002F"            /* !.[ùH...YRúH.!./ */
	$"8BF2 4806 0071 B42E 2B51 62F3 5C08 6446"            /* ‹òH..q´.+Qbó\.dF */
	$"0709 972A 3547 63FD 5CF6 5C05 5913 0753"            /* .Æ—*5Gcı\ö\.Y..S */
	$"4D70 FA5C 0538 07CC FF76 98F9 5C02 1B07"            /* Mpú\.8.Ìÿv˜ù\... */
	$"A0FA 5C02 6707 50F6 FF01 C392 F95C 032E"            /*  ú\.g.Pöÿ.Ã’ù\.. */
	$"0759 75FA 5C03 3807 3286 F25C 051B 53FF"            /* .Yuú\.8.2†ò\..Sÿ */
	$"FF9C 98F9 5C08 073A FFFF A055 3F45 65FE"            /* ÿœ˜ù\..:ÿÿ U?Eeş */
	$"5CEA 5C07 4307 10A4 4331 4261 F25C 075B"            /* \ê\.C..¤C1Baò\.[ */
	$"1707 76FF 998F 70FA 5C03 4A07 5076 FA5C"            /* ..vÿ™pú\.J.Pvú\ */
	$"0322 0727 87F9 5C03 2E07 6670 FA5C 011C"            /* .".'‡ù\...fpú\.. */
	$"32F6 FF05 E171 2728 4A62 F35C 055C 4A07"            /* 2öÿ.áq'(Jbó\.\J. */
	$"B535 86FA 5C03 6707 1384 FA5C 0453 076C"            /* µ5†ú\.g..„ú\.S.l */
	$"499A F95C 022E 0769 F95C 031B 0765 65FA"            /* Išù\...iù\...eeú */
	$"5C03 2E07 3998 F25C 0107 7602 9D07 FFA6"            /* \...9˜ò\..v..ÿ¦ */
	$"1200 0930 3948 F940 0E4C 3C2A 0B00 0891"            /* ..Æ09Hù@.L<*...‘ */
	$"FFC5 2900 001E 3A4C F940 0946 4330 1100"            /* ÿÅ)...:Lù@ÆFC0.. */
	$"0065 C83E 4FFA 3B05 1D00 C8FF 647F F93B"            /* .eÈ>Oú;...Èÿd.ù; */
	$"0208 0084 FA3B 0240 0048 F6FF 01BD 7CF9"            /* ...„ú;.@.Höÿ.½|ù */
	$"3B03 1600 4751 FA3B 031D 002E 6EF8 3BFB"            /* ;...GQú;....nø;û */
	$"3B05 0847 FFFF 8E80 F93B 0100 2EFE FF06"            /* ;..Gÿÿ€ù;...şÿ. */
	$"851F 0002 2634 47EE 400E 4543 3017 000D"            /* …...&4Gî@.EC0... */
	$"A5FF D94F 0000 203D 50F8 400A 4733 1F00"            /* ¥ÿÙO.. =Pø@ÂG3.. */
	$"007D FFFF 967D 50FA 3B03 2B00 3651 FA3B"            /* .}ÿÿ–}Pú;.+.6Qú; */
	$"0310 002E 6FF9 3B03 1600 4A4A FA3B 000B"            /* ....où;...JJú;.. */
	$"002E F5FF 07E0 8019 0009 1B2B 32F5 3B04"            /* ..õÿ.à€..Æ.+2õ;. */
	$"2B00 AD27 6FFA 3B03 4000 0666 FA3B 0432"            /* +.­'oú;.@..fú;.2 */
	$"0062 4986 F93B 0216 0051 F93B FF00 0154"            /* .bI†ù;...Qù;ÿ..T */
	$"4EFA 3B03 1600 2C80 F23B 0900 70FF A815"            /* Nú;...,€ò;Æ.pÿ¨. */
	$"000B 323C 4DF9 450E 5040 2E0D 000A 92FF"            /* ..2<MùE.P@...Â’ÿ */
	$"C52B 0001 213D 4FF9 4509 4947 3313 0002"            /* Å+..!=OùEÆIG3... */
	$"67C8 4052 FA3E 0520 00C9 FF66 82F9 3E02"            /* gÈ@Rú>. .Éÿf‚ù>. */
	$"0A00 86FA 3E02 4500 4AF6 FF01 BE80 F93E"            /* Â.†ú>.E.Jöÿ.¾€ù> */
	$"0319 004A 54FA 3E03 2000 3071 F23E 050A"            /* ...JTú>. .0qò>.Â */
	$"49FF FF90 82FD 3EFD 3E01 0030 FEFF 0687"            /* Iÿÿ‚ı>ı>..0şÿ.‡ */
	$"2100 0629 384C EE45 0E49 4733 1A00 10A6"            /* !..)8LîE.IG3...¦ */
	$"FFDB 5101 0024 4253 F845 0A4C 3621 0001"            /* ÿÛQ..$BSøEÂL6!.. */
	$"7FFF FF97 8154 FA3E 032F 003A 54FA 3E03"            /* .ÿÿ—Tú>./.:Tú>. */
	$"1300 3071 F93E 0319 004D 4FFA 3E01 0D30"            /* ..0qù>...MOú>..0 */
	$"F5FF 02E0 821B 0400 0B1E 2F35 F53E 042F"            /* õÿ.à‚...../5õ>./ */
	$"00AE 2A71 FA3E 0345 0009 69FA 3E04 3600"            /* .®*qú>.E.Æiú>.6. */
	$"644C 88F9 3E02 1900 54F9 3E03 0100 5752"            /* dLˆù>...Tù>...WR */
	$"FA3E 0319 002F 82F2 3E09 0071 FFAD 2008"            /* ú>.../‚ò>Æ.qÿ­ . */
	$"1640 5062 F95A 0F64 543D 1707 1598 FFC8"            /* .@PbùZ.dT=...˜ÿÈ */
	$"3609 0B2F 4F64 5AFA 5A09 5E5B 441F 070C"            /* 6Æ./OdZúZÆ^[D... */
	$"70CC 4D65 FA52 052F 07CC FF75 90F9 5202"            /* pÌMeúR./.ÌÿuùR. */
	$"1607 94FA 5202 5807 50F6 FF01 C38E F952"            /* ..”úR.X.Pöÿ.ÃùR */
	$"0325 0759 66FA 5203 2F07 3A83 F252 0516"            /* .%.YfúR./.:ƒòR.. */
	$"53FF FF9C 90F9 5201 073A FEFF 068F 2B07"            /* SÿÿœùR..:şÿ.+. */
	$"1038 485F EE5A 0E5D 5B44 2507 1BAC FFDC"            /* .8H_îZ.][D%..¬ÿÜ */
	$"5B0B 0731 5367 F85A 0A5F 482F 090B 85FF"            /* [..1SgøZÂ_H/Æ.…ÿ */
	$"FF99 8F66 FA52 033E 0746 66FA 5203 1F07"            /* ÿ™fúR.>.FfúR... */
	$"3A83 F952 0325 075B 62FA 5201 1A3A F5FF"            /* :ƒùR.%.[búR..:õÿ */
	$"07E0 8824 0716 2A3F 46F6 5205 523E 07B5"            /* .àˆ$..*?FöR.R>.µ */
	$"3481 FA52 0358 0713 7AFA 5204 4907 6E52"            /* 4úR.X..zúR.I.nR */
	$"96F9 5202 2507 64F9 5203 0B07 6363 FA52"            /* –ùR.%.dùR...ccúR */
	$"0325 0739 90F2 5201 0776 026E FFFF 02DD"            /* .%.9òR..v.nÿÿ.İ */
	$"9438 FE00 0113 16FD 1201 1910 FE00 0233"            /* ”8ş....ı....ş..3 */
	$"8EDD FDFF 06AD 5202 0000 071D FC12 0919"            /* İıÿ.­R.....ü.Æ. */
	$"0100 001A 75CD FFCD 38F7 2E03 CDFF 6D3A"            /* ....uÍÿÍ8÷..Íÿm: */
	$"F72E 0035 F82E 0069 F6FF 01C1 49F7 2E00"            /* ÷..5ø..iöÿ.ÁI÷.. */
	$"35F7 2E01 5439 F82E FA2E 0468 FFFF 8638"            /* 5÷..T9ø.ú..hÿÿ†8 */
	$"F82E 0054 FCFF 06A5 4408 0000 0C16 FD12"            /* ø..Tüÿ.¥D.....ı. */
	$"FF19 0405 0005 161D FD12 0116 13FE 0002"            /* ÿ.......ı....ş.. */
	$"1F87 DDFD FF06 C266 0900 0005 21FD 1207"            /* .‡İıÿ.ÂfÆ...!ı.. */
	$"1B19 0100 000B 61C7 FEFF 01A6 51F7 2E00"            /* ......aÇşÿ.¦Q÷.. */
	$"32F7 2E01 5439 F72E 0033 F82E 0054 F3FF"            /* 2÷..T9÷..3ø..Tóÿ */
	$"04DF 9455 3008 F700 FE12 0417 2EB8 3B33"            /* .ß”U0.÷.ş....¸;3 */
	$"F82E FF30 F82E 0290 693E F72E 0032 F72E"            /* ø.ÿ0ø..i>÷..2÷. */
	$"0033 F72E 0153 38F1 2E05 88FF FFDD 953A"            /* .3÷..S8ñ..ˆÿÿİ•: */
	$"FE00 0115 1AFD 1501 1C12 FE00 0235 8FDD"            /* ş....ı....ş..5İ */
	$"FDFF 06AE 5406 0000 0920 FC15 091C 0300"            /* ıÿ.®T...Æ ü.Æ... */
	$"001C 76CE FFCD 3AF7 3003 CEFF 6F3C F730"            /* ..vÎÿÍ:÷0.Îÿo<÷0 */
	$"0038 F830 006C F6FF 01C2 4DF7 3000 38F7"            /* .8ø0.löÿ.ÂM÷0.8÷ */
	$"3001 573C F130 046B FFFF 873A FD30 FC30"            /* 0.W<ñ0.kÿÿ‡:ı0ü0 */
	$"0057 FCFF 06A6 450A 0000 0F1A FD15 FF1C"            /* .Wüÿ.¦EÂ....ı.ÿ. */
	$"0407 0007 1A20 FD15 011A 15FE 0002 2188"            /* ..... ı....ş..!ˆ */
	$"DDFD FF06 C368 0B00 0007 24FD 1507 1E1C"            /* İıÿ.Ãh....$ı.... */
	$"0300 000D 63C7 FEFF 01A8 54F7 3000 34F7"            /* ....cÇşÿ.¨T÷0.4÷ */
	$"3001 573C F730 0035 F830 0057 F3FF 00DF"            /* 0.W<÷0.5ø0.Wóÿ.ß */
	$"0395 5732 0AF7 00FE 1504 1B30 B93E 35F8"            /* .•W2Â÷.ş...0¹>5ø */
	$"30FF 32F8 3002 926C 42F7 3000 34F7 3000"            /* 0ÿ2ø0.’lB÷0.4÷0. */
	$"35F7 3001 553A F130 0A8A FFFF DE9B 4407"            /* 5÷0.U:ñ0ÂŠÿÿŞ›D. */
	$"0708 2126 FD22 0128 1EFE 0702 3E96 DEFD"            /* ..!&ı".(.ş..>–Şı */
	$"FF05 B35C 1007 0714 002B FC22 0927 0D07"            /* ÿ.³\.....+ü"Æ'.. */
	$"0726 7CD2 FFD1 46F7 3A03 D1FF 7C47 F73A"            /* .&|ÒÿÑF÷:.Ñÿ|G÷: */
	$"0043 F83A 0071 F6FF 01C8 59F7 3A00 43F7"            /* .Cø:.qöÿ.ÈY÷:.C÷ */
	$"3A01 6148 F13A 0473 FFFF 9446 F83A 0061"            /* :.aHñ:.sÿÿ”Fø:.a */
	$"FCFF 04AB 4F15 0707 011B 26FD 22FF 2804"            /* üÿ.«O.....&ı"ÿ(. */
	$"1207 1226 2BFD 2201 2621 FE07 022B 8FDE"            /* ...&+ı".&!ş..+Ş */
	$"FDFF 06C7 6F16 0707 1230 FD22 072A 270D"            /* ıÿ.Ço....0ı".*'. */
	$"0707 1769 CAFE FF01 AA62 F73A 003F F73A"            /* ...iÊşÿ.ªb÷:.?÷: */
	$"0161 48F7 3A00 40F8 3A00 61F3 FF04 DF9B"            /* .aH÷:.@ø:.aóÿ.ß› */
	$"5F3C 15F7 07FF 2205 2227 3ABF 4842 F83A"            /* _<.÷.ÿ"."':¿HBø: */
	$"013C 3DF8 3A02 9871 4DF7 3A00 3FF7 3A00"            /* .<=ø:.˜qM÷:.?÷:. */
	$"40F7 3A01 5F46 F13A 008F 00E9 FCFF 03D6"            /* @÷:._Fñ:..éüÿ.Ö */
	$"B083 58FB 4803 5C8A ADD4 F8FF 03DD BB97"            /* °ƒXûH.\Š­Ôøÿ.İ»— */
	$"68FB 4803 5076 A2CA ABFF E3FF 02C1 905A"            /* hûH.Pv¢Ê«ÿãÿ.ÁZ */
	$"FB48 0454 717A 7158 FB48 0358 83A2 CAF8"            /* ûH.TqzqXûH.Xƒ¢Êø */
	$"FF04 DFB8 926B 4CFC 4803 5071 9EBF CEFF"            /* ÿ.ß¸’kLüH.Pq¿Îÿ */
	$"EDFF 05CA ADC4 C8C8 CBA6 FF03 D7B2 855A"            /* íÿ.Ê­ÄÈÈË¦ÿ.×²…Z */
	$"FB4A 005E 028B AED4 F8FF 03DD BD98 6BFB"            /* ûJ.^.‹®Ôøÿ.İ½˜kû */
	$"4A03 5278 A3CB 9BFF F3FF 02C2 915C FB4A"            /* J.Rx£Ë›ÿóÿ.Â‘\ûJ */
	$"0455 737C 735A FB4A 035A 85A3 CBF8 FF04"            /* .Us|sZûJ.Z…£Ëøÿ. */
	$"DFB9 926D 4DFC 4A03 5273 9FC0 BEFF FDFF"            /* ß¹’mMüJ.RsŸÀ¾ÿıÿ */
	$"05CB AEC5 C9C9 CBA6 FF03 D8B7 8B63 FB50"            /* .Ë®ÅÉÉË¦ÿ.Ø·‹cûP */
	$"0366 8FB5 D6F8 FF03 DEC2 9C73 FB50 0359"            /* .fµÖøÿ.ŞÂœsûP.Y */
	$"7FA9 CE8D FF01 C796 0065 FB50 045E 7B83"            /* .©Îÿ.Ç–.eûP.^{ƒ */
	$"7B63 FB50 0363 8BA9 CEF8 FF04 DFC0 9776"            /* {cûP.c‹©Îøÿ.ßÀ—v */
	$"53FC 5003 597B A3C4 BAFF 05CE B5CA CCCC"            /* SüP.Y{£Äºÿ.ÎµÊÌÌ */
	$"CEFB FFB1 FF00 00FF"                                /* Îûÿ±ÿ..ÿ */
};

data 'PICT' (8003, "gather dialog header", purgeable) {
	$"6A80 0000 0000 0025 01AB 0011 02FF 0C00"            /* j€.....%.«...ÿ.. */
	$"FFFE 0000 0048 0000 0048 0000 0000 0000"            /* ÿş...H...H...... */
	$"0025 01AB 0000 0000 0001 000A 0000 0000"            /* .%.«.......Â.... */
	$"0025 01AB 009A 0000 00FF 86AC 0000 0000"            /* .%.«.š...ÿ†¬.... */
	$"0025 01AB 0000 0004 0000 0000 0048 0000"            /* .%.«.........H.. */
	$"0048 0000 0010 0020 0003 0008 0000 0000"            /* .H..... ........ */
	$"0000 0000 0000 0000 0000 0000 0025 01AB"            /* .............%.« */
	$"0000 0000 0025 01AB 0040 00A9 E0FF 00E2"            /* .....%.«.@.©àÿ.â */
	$"FDD9 FED8 00DF ABFF 89FF 00E0 FDD9 FED8"            /* ıÙşØ.ß«ÿ‰ÿ.àıÙşØ */
	$"00E2 9EFF 00DC FED9 FED8 00DE F1FF 00DB"            /* .âÿ.ÜşÙşØ.Şñÿ.Û */
	$"FED9 FED8 00DD FEFF 00DD FDD9 FFD8 00DC"            /* şÙşØ.İşÿ.İıÙÿØ.Ü */
	$"C4FF 01E2 DBFD D9FF D800 E0D6 FF81 FFDE"            /* Äÿ.âÛıÙÿØ.àÖÿÿŞ */
	$"FF01 E0DB FED9 FED8 00E2 ADFF F2FF 00DC"            /* ÿ.àÛşÙşØ.â­ÿòÿ.Ü */
	$"FDD9 FFD8 00DE F1FF 00DB FDD9 FFD8 00DD"            /* ıÙÿØ.Şñÿ.ÛıÙÿØ.İ */
	$"FEFF 01DD DBFE D9FF D800 DCC4 FF00 E3FE"            /* şÿ.İÛşÙÿØ.ÜÄÿ.ãş */
	$"DCFE DB01 D9E0 81FF B3FF 02E1 DCDC FDDB"            /* ÜşÛ.Ùàÿ³ÿ.áÜÜıÛ */
	$"01D9 E2D8 FFC7 FF01 DDDC FCDB 00DF F1FF"            /* .ÙâØÿÇÿ.İÜüÛ.ßñÿ */
	$"01DD DCFC DB00 DEFE FF02 DEDC DCFD DB00"            /* .İÜüÛ.Şşÿ.ŞÜÜıÛ. */
	$"DDE6 FF00 FF03 73FA FF04 D7C0 B0A2 97FE"            /* İæÿ.ÿ.súÿ.×À°¢—ş */
	$"A303 AAAF AFB2 FEBB 03C3 C9C9 D7FD FF11"            /* £.ª¯¯²ş».ÃÉÉ×ıÿ. */
	$"CDA0 887A 7571 6E69 6663 5D58 6982 96CE"            /* Í ˆzuqnifc]Xi‚–Î */
	$"FFC9 FAC5 FCC7 F8C8 FEC5 FEC7 FCC8 FDC5"            /* ÿÉúÅüÇøÈşÅşÇüÈıÅ */
	$"FEC7 FEC8 01C4 E0FA C5FD C7FA C807 C7C9"            /* şÇşÈ.ÄàúÅıÇúÈ.ÇÉ */
	$"C5C5 B8B7 B7B3 FEA9 069C 9998 988C A4BB"            /* ÅÅ¸··³ş©.œ™˜˜Œ¤» */
	$"00D7 F0FF 0DBE A09E 8F8E 8080 7175 8190"            /* .×ğÿ.¾ €€qu */
	$"9FAD C3F8 FF00 D4FB C5FC C7FC C8F9 C5FC"            /* Ÿ­Ãøÿ.ÔûÅüÇüÈùÅü */
	$"C7FA C800 C7FE C5FE C7FC C8FD C5FE C7FD"            /* ÇúÈ.ÇşÅşÇüÈıÅşÇı */
	$"C800 C7FE C5FE C7FD C800 C9FE FF0B E2C3"            /* È.ÇşÅşÇıÈ.Éşÿ.âÃ */
	$"9785 7973 706D 6865 625C 045A 6E85 A0D8"            /* —…yspmheb\.Zn… Ø */
	$"FCFF 06CA C5C5 BAB7 B7B3 FEA9 079E 9998"            /* üÿ.ÊÅÅº··³ş©.™˜ */
	$"988D A1BA D4FB FF00 C7FE C5FE C7FD C8FD"            /* ˜¡ºÔûÿ.ÇşÅşÇıÈı */
	$"C5FE C7FD C800 C4F0 FF04 CFBE AE9F 9AFE"            /* ÅşÇıÈ.Äğÿ.Ï¾®Ÿšş */
	$"A303 ADAF AFB4 FEBB 03C5 C9C8 DEFE FF0F"            /* £.­¯¯´ş».ÅÉÈŞşÿ. */
	$"D6AB 8B7C 7671 6E6B 6663 5E59 647B 94BF"            /* Ö«‹|vqnkfc^Yd{”¿ */
	$"F9FF 07D1 A587 7A76 716E 6912 6563 5F5F"            /* ùÿ.Ñ¥‡zvqni.ec__ */
	$"6479 7773 726F 6B67 635F 5A62 768E B6FD"            /* dywsrokgc_Zbv¶ı */
	$"FF00 D5FB C5FC C7FA C800 D5FA FF04 D8C1"            /* ÿ.ÕûÅüÇúÈ.Õúÿ.ØÁ */
	$"B2A4 99FE A403 ACB0 B0B3 FEBD 03C3 CACA"            /* ²¤™ş¤.¬°°³ş½.ÃÊÊ */
	$"D7FD FF11 CDA2 8C7D 7875 716D 6966 615B"            /* ×ıÿ.Í¢Œ}xuqmifa[ */
	$"6D85 98CE FFCA F9C7 FAC8 FBC9 FDC7 FDC8"            /* m…˜ÎÿÊùÇúÈûÉıÇıÈ */
	$"FFC9 00C8 FDC7 00C8 FEC8 FFC9 01C5 E0FA"            /* ÿÉ.ÈıÇ.ÈşÈÿÉ.Åàú */
	$"C7FB C8FC C907 C7CA C7C7 B9B8 B8B4 FEAA"            /* ÇûÈüÉ.ÇÊÇÇ¹¸¸´şª */
	$"009F FE9A 038E A5BD D7F0 FF0D BFA1 9F91"            /* .Ÿşš.¥½×ğÿ.¿¡Ÿ‘ */
	$"9083 8373 7784 92A0 AFC4 F8FF 01D4 C5FB"            /* ƒƒsw„’ ¯Äøÿ.ÔÅû */
	$"C7FB C8FF C901 C8C5 FAC7 FAC8 FCC9 00C8"            /* ÇûÈÿÉ.ÈÅúÇúÈüÉ.È */
	$"FDC7 FFC8 FFC8 FEC9 FDC7 FCC8 FFC9 00C8"            /* ıÇÿÈÿÈşÉıÇüÈÿÉ.È */
	$"FDC7 FDC8 FFC9 00CA FEFF 10E2 C399 887C"            /* ıÇıÈÿÉ.Êşÿ.âÃ™ˆ| */
	$"7773 706C 6865 5F5D 7187 A1D8 FCFF 06CB"            /* wsplhe_]q‡¡Øüÿ.Ë */
	$"C7C7 BBB8 B8B4 FEAA 00A0 FE9A 038F A3BB"            /* ÇÇ»¸¸´şª. şš.£» */
	$"D4FB FF00 C8FD C7FD C8FF C9FC C7FD C8FF"            /* Ôûÿ.ÈıÇıÈÿÉüÇıÈÿ */
	$"C900 C5F0 FF04 D1BF AFA1 9CFE A402 AFB0"            /* É.Åğÿ.Ñ¿¯¡œş¤.¯° */
	$"B000 B5FE BD03 C7CA C9DF FEFF 0FD6 AC8D"            /* °.µş½.ÇÊÉßşÿ.Ö¬ */
	$"8079 7571 6E69 6662 5C67 7F96 C0F9 FF1A"            /* €yuqnifb\g.–Àùÿ. */
	$"D2A8 8A7D 7975 716D 6866 6363 677C 7A77"            /* Ò¨Š}yuqmhfccg|zw */
	$"7672 6E6B 6663 5E65 7890 B8FD FF00 D5FA"            /* vrnkfc^ex¸ıÿ.Õú */
	$"C7FB C8FC C900 D5FA FF04 D9C7 B9AD A4FE"            /* ÇûÈüÉ.Õúÿ.ÙÇ¹­¤ş */
	$"AE03 B4B8 B8BA FEC3 03C9 CDCD D9FD FF0B"            /* ®.´¸¸ºşÃ.ÉÍÍÙıÿ. */
	$"D1AC 988C 8885 827F 7C79 7570 057F 92A3"            /* Ñ¬˜Œˆ…‚.|yup..’£ */
	$"D2FF CDF7 CBF9 CCFE CDFC CBFD CC01 CDCC"            /* ÒÿÍ÷ËùÌşÍüËıÌ.ÍÌ */
	$"FCCB FCCC 01CA E1F8 CBFA CCFF CD07 CBCD"            /* üËüÌ.ÊáøËúÌÿÍ.ËÍ */
	$"CBCB C0BF BFBB FEB3 00A9 FEA5 039B AFC3"            /* ËËÀ¿¿»ş³.©ş¥.›¯Ã */
	$"D9F0 FF0D C3AB AA9E 9C91 9185 8892 9FAA"            /* Ùğÿ.Ã«ªœ‘‘…ˆ’Ÿª */
	$"B7C9 F8FF 00D6 00CA F9CB FACC 00CA F7CB"            /* ·Éøÿ.Ö.ÊùËúÌ.Ê÷Ë */
	$"FACC FFCD 00CC FBCB FECC 01CD CCFB CBFE"            /* úÌÿÍ.ÌûËşÌ.ÍÌûËş */
	$"CCFF CD00 CCFC CBFD CCFF CDFE FF10 E2C9"            /* ÌÿÍ.ÌüËıÌÿÍşÿ.âÉ */
	$"A496 8B87 8481 7F7B 7873 7182 96AA DBFC"            /* ¤–‹‡„.{xsq‚–ªÛü */
	$"FF06 CECB CBC1 BFBF BBFE B300 AAFE A503"            /* ÿ.ÎËËÁ¿¿»ş³.ªş¥. */
	$"9BAD C1D6 FBFF 00CC FCCB FFCC FFCC 00CD"            /* ›­ÁÖûÿ.ÌüËÿÌÿÌ.Í */
	$"FBCB FDCC 01CD CAF0 FF04 D3C4 B7AA A6FE"            /* ûËıÌ.ÍÊğÿ.ÓÄ·ª¦ş */
	$"AE03 B7B8 B8BB FEC3 03CB CDCC DFFE FF0F"            /* ®.·¸¸»şÃ.ËÍÌßşÿ. */
	$"D8B5 9A8E 8A85 8280 7C79 7771 7A8D A1C4"            /* ØµšŠ…‚€|ywqz¡Ä */
	$"F9FF 1AD4 B097 8C8A 8582 807B 7977 777B"            /* ùÿ.Ô°—ŒŠ…‚€{yww{ */
	$"8C8A 8686 8380 7D79 7773 7888 9CBE FDFF"            /* ŒŠ††ƒ€}ywsxˆœ¾ıÿ */
	$"00D7 F9CB FACC FECD 00D7 0443 FCFF 06D2"            /* .×ùËúÌşÍ.×.Cüÿ.Ò */
	$"A986 817D 7878 FD77 FE76 FD75 1B6E 2A96"            /* ©†}xxıwşvıu.n*– */
	$"FFE1 C2A2 8F8B 8D8F 9196 9492 9190 8B7B"            /* ÿáÂ¢‹‘–”’‘‹{ */
	$"6850 4646 7B82 716D 6EFE 6FFE 70FE 71FA"            /* hPFF{‚qmnşoşpşqú */
	$"7204 703C 716E 6FFE 7014 7172 7240 2F76"            /* r.p<qnoşp.qrr@/v */
	$"6E6F 6F70 7071 7272 5E12 D982 6F6D 6EFE"            /* nooppqrr^.Ù‚omnş */
	$"6FFF 70FE 71FC 7202 3E1A 83FE 72FE 71FF"            /* oÿpşqür.>.ƒşrşqÿ */
	$"6F07 6E6C 6B6B 6964 5950 0349 589B D7F3"            /* o.nlkkidYP.IX›×ó */
	$"FF11 9F82 8281 8180 7F7D 766E 625B 5852"            /* ÿ.Ÿ‚‚€.}vnb[XR */
	$"5275 9BCC FCFF 03B0 796E 6EFE 6FFE 70FF"            /* Ru›Ìüÿ.°ynnşoşpÿ */
	$"71FC 7203 676C 6D6E FE6F FF70 FD71 FA72"            /* qür.glmnşoÿpıqúr */
	$"315C 586E 6F6F 7070 7172 7259 136F 6D6F"            /* 1\XnooppqrrY.omo */
	$"6F70 7071 7272 6825 4C71 6E6F 7070 7171"            /* oppqrrh%Lqnoppqq */
	$"7272 4432 FFDE BA99 8E8B 8E90 9295 9492"            /* rrD2ÿŞº™‹’•”’ */
	$"9190 8606 7762 4A45 4D8D CDFE FF03 8C73"            /* ‘†.wbJEMÍşÿ.Œs */
	$"7272 FE71 FF6F 0B6E 6C6C 6B69 645A 5149"            /* rrşqÿo.nllkidZQI */
	$"5594 D3FE FF16 826E 6E6F 6F70 7171 7271"            /* U”Óşÿ.‚nnoopqqrq */
	$"3533 796E 6F70 7071 7172 7252 09F3 FF05"            /* 53ynoppqqrrRÆóÿ. */
	$"E1C9 9F84 807C FB77 FE76 FD75 1765 1BC8"            /* áÉŸ„€|ûwşvıu.e.È */
	$"FFCC A58F 8D8C 8F91 9694 9291 908C 7C6D"            /* ÿÌ¥Œ‘–”’‘Œ|m */
	$"5546 4368 B6FD FF09 C8A0 8F8C 8D90 9196"            /* UFCh¶ıÿÆÈ Œ‘– */
	$"9494 1C92 918E 8783 868C 8F94 9594 9290"            /* ””.’‘‡ƒ†Œ”•”’ */
	$"8E82 6E5C 4843 57A3 DFFF B677 6C6E 6F6F"            /* ‚n\HCW£ßÿ¶wlnoo */
	$"FE70 FE71 FC72 026C 2696 FCFF 07D2 AA88"            /* şpşqür.l&–üÿ.Òªˆ */
	$"8481 7B7A 7AFB 79FE 781C 7771 2E97 FFE1"            /* „{zzûyşx.wq.—ÿá */
	$"C3A3 918D 8F92 9598 9795 9592 8D7F 6C54"            /* Ã£‘’•˜—••’.lT */
	$"4A49 7D84 7570 71FE 72FE 73FE 75FA 7604"            /* JI}„upqşrşsşuúv. */
	$"733F 7571 72FE 7309 7576 7644 3178 7172"            /* s?uqrşsÆuvvD1xqr */
	$"7273 0A73 7576 7662 14D9 8472 7071 FE72"            /* rsÂsuvvb.Ù„rpqşr */
	$"FF73 FE75 FC76 0342 1D85 76FD 750E 7372"            /* ÿsşuüv.B.…vıu.sr */
	$"7271 6F6E 6E6D 675D 544E 5C9C D7F3 FF00"            /* rqonnmg]TN\œ×óÿ. */
	$"A0FD 840C 8281 8179 7065 5E5B 5755 789E"            /*  ı„.‚ype^[WUx */
	$"CCFC FF03 B27B 7171 FE72 FE73 FF75 FC76"            /* Ìüÿ.²{qqşrşsÿuüv */
	$"036B 6F70 71FE 72FF 73FD 75FA 7606 5F5B"            /* .kopqşrÿsıuúv._[ */
	$"7172 7273 7331 7576 765B 1671 7072 7273"            /* qrrss1uvv[.qprrs */
	$"7375 7676 6C28 4E75 7172 7373 7575 7676"            /* suvvl(Nuqrssuuvv */
	$"4734 FFDE BB9B 908D 9094 9697 9695 9492"            /* G4ÿŞ»›”–—–•”’ */
	$"887A 654F 4951 8FCD FEFF 018E 76FD 750E"            /* ˆzeOIQÍşÿ.vıu. */
	$"7372 7271 6F6F 6E6D 675D 554E 5996 D3FE"            /* srrqoonmg]UNY–Óş */
	$"FF16 8471 7172 7273 7575 7675 3935 7B71"            /* ÿ.„qqrrsuuvu95{q */
	$"7273 7375 7576 7655 0BF3 FF05 E1CA A186"            /* rssuuvvU.óÿ.áÊ¡† */
	$"8380 FE7A FC79 FD78 1877 681D C9FF CDA6"            /* ƒ€şzüyıx.wh.ÉÿÍ¦ */
	$"918F 8E92 9598 9796 9594 8F7F 6F5A 4A47"            /* ‘’•˜—–•”.oZJG */
	$"6CB8 FDFF 26C9 A191 8E8F 9295 9897 9695"            /* l¸ıÿ&É¡‘’•˜—–• */
	$"9490 8A86 8A8E 9196 9796 9594 9185 715F"            /* ”Š†Š‘–—–•”‘…q_ */
	$"4D47 5BA4 DFFF B77A 6F71 7272 FE73 FE75"            /* MG[¤ßÿ·zoqrrşsşu */
	$"FC76 026F 2897 FCFF 06D4 B396 9290 8B8B"            /* üv.o(—üÿ.Ô³–’‹‹ */
	$"FD8A FE88 FD87 1282 3B99 FFE1 C8AD 9E9A"            /* ıŠşˆı‡.‚;™ÿáÈ­š */
	$"9B9F A0A3 A2A1 A09F 9A8E 0A7D 6961 5F8C"            /* ›Ÿ £¢¡ ŸšÂ}ia_Œ */
	$"9285 8282 8383 FD84 FE85 FA86 1F84 4F85"            /* ’…‚‚ƒƒı„ş…ú†.„O… */
	$"8383 8484 8585 8686 533E 8882 8384 8485"            /* ƒƒ„„……††S>ˆ‚ƒ„„… */
	$"8586 8673 20DB 9283 8282 8384 84FC 85FC"            /* …††s Û’ƒ‚‚ƒ„„ü…ü */
	$"8602 512B 94FE 86FE 850D 8483 8381 8080"            /* †.Q+”ş†ş….„ƒƒ€€ */
	$"7F7A 716B 646F A6D8 F3FF 00AA FD92 0C91"            /* .zqkdo¦Øóÿ.ªı’.‘ */
	$"908F 8A82 7973 706C 6C87 A8CF FCFF 00B9"            /* Š‚yspll‡¨Ïüÿ.¹ */
	$"038C 8282 83FE 84FD 85FC 8602 7C81 82FE"            /* .Œ‚‚ƒş„ı…ü†.|‚ş */
	$"83FE 84FD 85FA 8638 716C 8283 8484 8585"            /* ƒş„ı…ú†8ql‚ƒ„„…… */
	$"8686 6D21 8282 8384 8485 8586 867D 355C"            /* ††m!‚‚ƒ„„……††}5\ */
	$"8582 8384 8485 8586 8657 3FFF DFC1 A59E"            /* …‚ƒ„„……††W?ÿßÁ¥ */
	$"999C A0A1 A3A2 A1A0 9F97 8A78 655F 659A"            /* ™œ ¡£¢¡ Ÿ—Šxe_eš */
	$"D1FE FF00 9BFE 86FE 850D 8483 8381 8080"            /* Ñşÿ.›ş†ş….„ƒƒ€€ */
	$"7F7A 716B 656D A0D5 FEFF 0292 8383 FE84"            /* .zqkem Õşÿ.’ƒƒş„ */
	$"FF85 0E86 8547 428B 8283 8484 8585 8686"            /* ÿ….†…GB‹‚ƒ„„……†† */
	$"6615 F3FF 05E1 CDAB 9591 8EFB 8AFE 88FD"            /* f.óÿ.áÍ«•‘ûŠşˆı */
	$"8717 7929 CCFF D1AF 9E9B 9B9F A0A3 A2A1"            /* ‡.y)ÌÿÑ¯››Ÿ £¢¡ */
	$"A09F 9B8E 816F 615C 7CBE FDFF 26CC AB9E"            /*  Ÿ›oa\|¾ıÿ&Ì« */
	$"9B9B 9FA0 A3A2 A1A1 A09C 9794 979B 9EA2"            /* ››Ÿ £¢¡¡ œ—”—›¢ */
	$"A2A1 A1A0 9E92 8275 635D 6DAD DFFF BE8A"            /* ¢¡¡ ’‚uc]m­ßÿ¾Š */
	$"8182 8383 FE84 FE85 FC86 0181 3500 9902"            /* ‚ƒƒş„ş…ü†.5.™. */
	$"A7FE FF06 CFA4 9998 999C A2F4 A408 7C00"            /* §şÿ.Ï¤™˜™œ¢ô¤.|. */
	$"96D7 B0A0 9FA0 A2F5 A404 9069 3644 91EF"            /* –×° Ÿ ¢õ¤.i6D‘ï */
	$"A402 8F00 7AF9 A403 0906 87A2 FAA4 0445"            /* ¤..zù¤.Æ.‡¢ú¤.E */
	$"00CF 8697 F2A4 0300 1268 94F4 A401 9B8E"            /* .Ï†—ò¤...h”ô¤.› */
	$"0581 613E 3E73 D9F5 FF01 9E90 F8A4 0E9F"            /* .a>>sÙõÿ.ø¤.Ÿ */
	$"988A 7764 4C3D 5595 D8FF FFB2 8BA0 F3A4"            /* ˜ŠwdL=U•Øÿÿ²‹ ó¤ */
	$"016C 9FEF A401 453B F9A4 0333 005D 9CFA"            /* .lŸï¤.E;ù¤.3.]œú */
	$"A403 7800 2596 F9A4 0716 2EBD A8A0 9FA0"            /* ¤.x.%–ù¤...½¨ Ÿ  */
	$"A3F8 A4FF A409 A187 5E2F 2C99 FFFF 9294"            /* £ø¤ÿ¤Æ¡‡^/,™ÿÿ’” */
	$"F4A4 0A9B 9082 633F 3C70 D3FF 8B97 FAA4"            /* ô¤Â›‚c?<pÓÿ‹—ú¤ */
	$"0395 0009 97F9 A402 3300 D8F5 FF01 C4A0"            /* .•.Æ—ù¤.3.Øõÿ.Ä  */
	$"FE99 019F A2F4 A407 5800 BAAA A09F 9FA1"            /* ş™.Ÿ¢ô¤.X.ºª ŸŸ¡ */
	$"F5A4 0B91 7142 305C D6DC B2A0 9F9F A2FB"            /* õ¤.‘qB0\ÖÜ² ŸŸ¢û */
	$"A4F0 A408 9977 4E30 52C9 B78C 9FF3 A402"            /* ¤ğ¤.™wN0RÉ·ŒŸó¤. */
	$"7C00 96FE FF06 D1A5 9B9A 9B9F A4F4 A608"            /* |.–şÿ.Ñ¥›š›Ÿ¤ô¦. */
	$"7F00 97D8 B2A2 A1A2 A4F5 A604 926D 3A47"            /* ..—Ø²¢¡¢¤õ¦.’m:G */
	$"94EF A602 9100 7CF9 A603 0B08 8AA4 FEA6"            /* ”ï¦.‘.|ù¦...Š¤ş¦ */
	$"FDA6 0447 00CF 8899 F2A6 0300 156B 96F4"            /* ı¦.G.Ïˆ™ò¦...k–ô */
	$"A607 9E91 8464 4342 76D9 F5FF 01A0 92F8"            /* ¦.‘„dCBvÙõÿ. ’ø */
	$"A60E A19A 8D7A 6750 4259 97D9 FFFF B38D"            /* ¦.¡šzgPBY—Ùÿÿ³ */
	$"A2F3 A601 6FA1 EFA6 0147 3EFC A6FE A603"            /* ¢ó¦.o¡ï¦.G>ü¦ş¦. */
	$"3500 5F9F FAA6 037A 0027 98F9 A607 1A30"            /* 5._Ÿú¦.z.'˜ù¦..0 */
	$"BEAA A2A1 A2A5 F6A6 09A3 8A62 3230 9AFF"            /* ¾ª¢¡¢¥ö¦Æ£Šb20šÿ */
	$"FF95 96F4 A60A 9E92 8566 4440 72D3 FF8D"            /* ÿ•–ô¦Â’…fD@rÓÿ */
	$"99FA A603 9700 0B99 F9A6 0235 00D8 F5FF"            /* ™ú¦.—..™ù¦.5.Øõÿ */
	$"01C5 A2FE 9B01 A1A4 F9A6 FCA6 075A 00BB"            /* .Å¢ş›.¡¤ù¦ü¦.Z.» */
	$"ACA2 A1A1 A3F5 A60B 9475 4533 5FD7 DCB3"            /* ¬¢¡¡£õ¦.”uE3_×Ü³ */
	$"A2A1 A1A4 EAA6 089B 7951 3354 CAB8 8EA1"            /* ¢¡¡¤ê¦.›yQ3TÊ¸¡ */
	$"F3A6 027F 0097 FEFF 06D3 AFA6 A5A6 A9AD"            /* ó¦...—şÿ.Ó¯¦¥¦©­ */
	$"F4AF 088B 0799 D9B9 ACAB ABAD F7AF FFAF"            /* ô¯.‹.™Ù¹¬««­÷¯ÿ¯ */
	$"049F 7C4C 5AA0 EFAF 029F 0787 F9AF 0316"            /* .Ÿ|LZ ï¯.Ÿ.‡ù¯.. */
	$"1298 AEFA AF04 5307 D397 A4F2 AF03 0722"            /* .˜®ú¯.S.Ó—¤ò¯.." */
	$"7BA2 F4AF 07A9 9E92 7758 5583 DBF5 FF01"            /* {¢ô¯.©’wXUƒÛõÿ. */
	$"AA9F F8AF 0CAB A59A 8A7A 6557 69A0 DBFF"            /* ªŸø¯.«¥šŠzeWi Ûÿ */
	$"FFBA 019A ACF3 AF01 7AAB EFAF 0153 49F9"            /* ÿº.š¬ó¯.z«ï¯.SIù */
	$"AF03 3E07 6CA9 FAAF 0383 0733 A3F9 AF07"            /* ¯.>.l©ú¯.ƒ.3£ù¯. */
	$"253A C2B3 ACAB ACAE F6AF 09AD 9773 4542"            /* %:Â³¬«¬®ö¯Æ­—sEB */
	$"A2FF FFA1 A2F4 AF0A A99F 9479 5954 80D5"            /* ¢ÿÿ¡¢ô¯Â©Ÿ”yYT€Õ */
	$"FF9A A4FB AF04 AFA3 0716 A4F9 AF02 4007"            /* ÿš¤û¯.¯£..¤ù¯.@. */
	$"D9F5 FF06 CAAB A5A6 A6AB ADF4 AF07 6607"            /* Ùõÿ.Ê«¥¦¦«­ô¯.f. */
	$"C1B4 ACAB ABAD F5AF 0BA0 8457 456E D8DD"            /* Á´¬««­õ¯. „WEnØİ */
	$"BAAC ABAB ADEA AF08 A688 6445 64CC BF9B"            /* º¬««­ê¯.¦ˆdEdÌ¿› */
	$"ABF3 AF01 8B07 0099 022C FFFF 03C5 A0A4"            /* «ó¯.‹..™.,ÿÿ.Å ¤ */
	$"A1F0 A005 7A00 724A A3A4 F0A0 0288 277F"            /* ¡ğ .z.rJ£¤ğ .ˆ'. */
	$"EFA0 028D 006F F9A0 0209 0065 F9A0 0443"            /* ï ..où .Æ.eù .C */
	$"00C8 5F96 F2A0 0300 1246 92F2 A0FF A004"            /* .È_–ò ...F’ò ÿ . */
	$"963D 1D43 D3F6 FF01 9E90 F3A0 099E 7744"            /* –=.CÓöÿ.ó ÆwD */
	$"2C2A 86FF B28B 9EF3 A001 689B EFA0 0143"            /* ,*†ÿ²‹ó .h›ï .C */
	$"2BF9 A003 3200 3D9A FAA0 0377 0000 95F9"            /* +ù .2.=šú .w..•ù */
	$"A004 1609 2BA5 A1F5 A0FC A006 731B 139C"            /*  ..Æ+¥¡õ ü .s..œ */
	$"FF92 92F0 A006 9948 1D38 CA8A 95FA A003"            /* ÿ’’ğ .™H.8ÊŠ•ú . */
	$"9100 0083 F9A0 0232 00C8 F7FF 03E2 B7A0"            /* ‘..ƒù .2.È÷ÿ.â·  */
	$"A1EF A004 5500 0B91 A5F0 A006 952C 1B4E"            /* ¡ï .U..‘¥ğ .•,.N */
	$"9FA1 A4F8 A0ED A005 481B 279A 8C9B F3A0"            /* Ÿ¡¤ø í .H.'šŒ›ó  */
	$"087A 0096 FFFF C7A2 A6A4 F0A3 057C 0073"            /* .z.–ÿÿÇ¢¦¤ğ£.|.s */
	$"4DA5 A6F0 A302 8B2A 82EF A302 8F00 71F9"            /* M¥¦ğ£.‹*‚ï£..qù */
	$"A303 0B00 67A2 FEA3 FDA3 0446 00C8 6298"            /* £...g¢ş£ı£.F.Èb˜ */
	$"F2A3 0300 1549 95F0 A304 983F 2045 D4F6"            /* ò£...I•ğ£.˜? EÔö */
	$"FF01 A092 F3A3 09A0 7A47 302E 87FF B38D"            /* ÿ. ’ó£Æ zG0.‡ÿ³ */
	$"A0F3 A301 6C9F EFA3 0146 2EFC A3FE A303"            /*  ó£.lŸï£.F.ü£ş£. */
	$"3400 3F9E FAA3 037A 0001 97F9 A304 1A0B"            /* 4.?ú£.z..—ù£... */
	$"2EA8 A4F0 A306 761E 169E FF94 95F0 A306"            /* .¨¤ğ£.v..ÿ”•ğ£. */
	$"9B4A 1F3A CA8C 97FA A303 9500 0085 F9A3"            /* ›J.:ÊŒ—ú£.•..…ù£ */
	$"0234 00C9 F7FF 03E2 B8A2 A4F4 A3FC A304"            /* .4.É÷ÿ.â¸¢¤ô£ü£. */
	$"5900 0D94 A8F0 A306 9730 1E50 A1A3 A6E4"            /* Y..”¨ğ£.—0.P¡£¦ä */
	$"A305 4C1E 299B 8E9F F3A3 087C 0097 FFFF"            /* £.L.)›Ÿó£.|.—ÿÿ */
	$"CBAC AFAD F0AC 0588 0779 58AE AFF4 ACFD"            /* Ë¬¯­ğ¬.ˆ.yX®¯ô¬ı */
	$"AC02 963A 8EEF AC02 9C07 7CF9 AC02 1607"            /* ¬.–:ï¬.œ.|ù¬... */
	$"75F9 AC04 5207 CC6F A3F2 AC03 0722 59A1"            /* uù¬.R.Ìo£ò¬.."Y¡ */
	$"F0AC 04A3 4F2F 53D6 F6FF 01AA 9FF3 AC07"            /* ğ¬.£O/SÖöÿ.ªŸó¬. */
	$"AA88 5A42 3E90 FFBA 019A AAF3 AC01 77A9"            /* ªˆZB>ÿº.šªó¬.w© */
	$"EFAC 0152 3AF9 AC03 3D07 4CA8 FAAC 0383"            /* ï¬.R:ù¬.=.L¨ú¬.ƒ */
	$"070A A3F9 AC04 2514 39B0 ADF0 AC06 822C"            /* .Â£ù¬.%.9°­ğ¬.‚, */
	$"22A3 FFA0 A1F0 AC06 A559 2F49 CD99 A3FB"            /* "£ÿ ¡ğ¬.¥Y/IÍ™£û */
	$"AC04 ACA1 0707 92F9 AC02 3F07 CCF7 FF03"            /* ¬.¬¡..’ù¬.?.Ì÷ÿ. */
	$"E2BF ABAD EFAC 0465 0717 A0B0 F0AC 06A1"            /* â¿«­ï¬.e.. °ğ¬.¡ */
	$"3E2C 5CAB ADAF E4AC 055B 2C38 A59B A9F3"            /* >,\«­¯ä¬.[,8¥›©ó */
	$"AC01 8807 0099 02C7 04FF D497 A0A0 F89E"            /* ¬.ˆ..™.Ç.ÿÔ—  ø */
	$"0199 92FA 9E04 7800 482E 98F9 9E03 8853"            /* .™’ú.x.H.˜ù.ˆS */
	$"779A FA9E 0167 46FC 7900 97FA 9E00 8DFD"            /* wšú.gFüy.—ú.ı */
	$"7902 6900 6EF9 9E02 0900 5BF9 9E04 4200"            /* y.i.nù.Æ.[ù.B. */
	$"C83E 95F9 9EFA 7903 0012 3291 FA9E 019A"            /* È>•ùúy...2‘ú.š */
	$"8AFB 9EFE 9E03 971A 0054 F6FF 019E 90F0"            /* Šûş.—..Töÿ.ğ */
	$"9E06 6824 2086 B28B 9BFA 9E00 8DFB 7901"            /* .h$ †²‹›ú.ûy. */
	$"4F76 FD79 0088 F99E 007B FD79 0132 2BF9"            /* Ovıy.ˆù.{ıy.2+ù */
	$"9E03 4200 389A FA9E 0375 0000 95F9 9E03"            /* .B.8šú.u..•ù. */
	$"1600 2F9B F99E 047A 5783 9E9E FB9E 0540"            /* ../›ù.zWƒû.@ */
	$"005F FF92 91F9 9E00 85F8 9E05 9B22 003B"            /* ._ÿ’‘ù.…ø.›".; */
	$"8894 FA9E 0390 0000 80F9 9E02 3100 C8F7"            /* ˆ”ú...€ù.1.È÷ */
	$"FF03 BE99 A29F F89E 0196 97FA 9E03 5400"            /* ÿ.¾™¢Ÿø.–—ú.T. */
	$"0095 F99E 0395 5D72 99FA 9E04 7600 018C"            /* .•ù.•]r™ú.v..Œ */
	$"95F7 9EED 9E05 8D00 005F 8C9A FA9E 0088"            /* •÷í..._Œšú.ˆ */
	$"FB79 075C 0096 FFD4 99A2 A2F8 A001 9B95"            /* ûy.\.–ÿÔ™¢¢ø .›• */
	$"FAA0 047A 004A 309A F9A0 038B 5579 9CFA"            /* ú .z.J0šù .‹Uyœú */
	$"A001 6949 FC7B 0099 FAA0 0090 FD7B 026D"            /*  .iIü{.™ú .ı{.m */
	$"0070 F9A0 020B 005D FDA0 FDA0 0445 00C8"            /* .pù ...]ı ı .E.È */
	$"4297 F9A0 FA7B 0300 1535 94FA A001 9C8C"            /* B—ù ú{...5”ú .œŒ */
	$"F8A0 0399 1D02 55F6 FF01 A092 F0A0 066C"            /* ø .™..Uöÿ. ’ğ .l */
	$"2724 88B3 8D9E FAA0 0090 FB7B 0151 78FD"            /* '$ˆ³ú .û{.Qxı */
	$"7B00 8BF9 A000 7FFD 7B01 342E FCA0 FEA0"            /* {.‹ù ..ı{.4.ü ş  */
	$"0345 003A 9CFA A003 7700 0097 F9A0 031A"            /* .E.:œú .w..—ù .. */
	$"0031 9EF9 A002 7C5A 86F9 A005 4300 62FF"            /* .1ù .|Z†ù .C.bÿ */
	$"9494 F9A0 0087 F8A0 059E 2502 3D8B 96FA"            /* ””ù .‡ø .%.=‹–ú */
	$"A003 9200 0082 F9A0 0233 00C9 F7FF 03BF"            /*  .’..‚ù .3.É÷ÿ.¿ */
	$"9BA4 A1F8 A003 9899 A0A0 FCA0 0358 0000"            /* ›¤¡ø .˜™  ü .X.. */
	$"97F9 A003 975F 769B FAA0 0478 0003 8E97"            /* —ù .—_v›ú .x..— */
	$"E3A0 058F 0100 628E 9CFA A000 8CFB 7B07"            /* ã ...bœú .Œû{. */
	$"5E00 97FF D6A4 ACAC F8AA 01A6 A1FA AA04"            /* ^.—ÿÖ¤¬¬øª.¦¡úª. */
	$"8707 503B A5F9 AA05 9663 85A8 AAAA FCAA"            /* ‡.P;¥ùª.–c…¨ªªüª */
	$"0175 57FC 8700 A4FA AA00 9BFD 8702 7B07"            /* .uWü‡.¤úª.›ı‡.{. */
	$"7BF9 AA02 1607 69F9 AA04 5107 CC4F A2F9"            /* {ùª...iùª.Q.ÌO¢ù */
	$"AAFA 8703 0722 42A0 FAAA 01A8 98F8 AA03"            /* ªú‡.."B úª.¨˜øª. */
	$"A428 0C5D F6FF 01AA 9FF0 AA04 7A36 3391"            /* ¤(.]öÿ.ªŸğª.z63‘ */
	$"BA01 9AA8 FAAA 009B FB87 015E 85FD 8700"            /* º.š¨úª.›û‡.^…ı‡. */
	$"97F9 AA00 8BFD 8701 403A F9AA 0351 0747"            /* —ùª.‹ı‡.@:ùª.Q.G */
	$"A6FA AA03 8107 07A2 F9AA 0325 073B A8F9"            /* ¦úª...¢ùª.%.;¨ù */
	$"AA02 8865 92F9 AA05 4F08 6CFF A0A0 F9AA"            /* ª.ˆe’ùª.O.lÿ  ùª */
	$"0095 F8AA 05A8 2F0B 4798 A2FB AA04 AA9F"            /* .•øª.¨/.G˜¢ûª.ªŸ */
	$"0707 8FF9 AA02 3E07 CCF7 FF03 C4A5 AEAB"            /* ..ùª.>.Ì÷ÿ.Ä¥®« */
	$"F8AA 01A3 A4FA AA03 6407 07A2 F9AA 03A3"            /* øª.£¤úª.d..¢ùª.£ */
	$"6C81 A6FA AA04 8409 0D9B A3E3 AA05 9B0A"            /* l¦úª.„Æ.›£ãª.›Â */
	$"076F 9BA6 FAAA 0097 FB87 016D 0700 9903"            /* .o›¦úª.—û‡.m..™. */
	$"0A03 FFD2 9598 F99A 0394 2402 6FFA 9A04"            /* Â.ÿÒ•˜ùš.”$.oúš. */
	$"7600 482C 96F9 9A03 3000 1091 FA9A 0765"            /* v.H,–ùš.0..‘úš.e */
	$"0003 4747 1C09 92FA 9A02 5400 3EFE 4701"            /* ..GG.Æ’úš.T.>şG. */
	$"006E F99A 0309 005B 9BFA 9A04 4200 C83A"            /* .nùš.Æ.[›úš.B.È: */
	$"94F9 9A01 001C FB47 0257 3190 FA9A 038E"            /* ”ùš...ûG.W1úš. */
	$"000A 48FD 9AFD 9A02 3E00 46F6 FF01 9E90"            /* .ÂHıšıš.>.Föÿ. */
	$"F99A 0168 79F9 9A05 7A30 1B5F 8B99 FA9A"            /* ùš.hyùš.z0._‹™úš */
	$"0254 003E F847 0100 63F9 9A01 0914 FE47"            /* .T.>øG..cùš.Æ.şG */
	$"011C 2CF9 9A03 4200 3898 FA9A 0372 0000"            /* ..,ùš.B.8˜úš.r.. */
	$"95F9 9A02 1600 2FF8 9A04 0900 2E9A 9AFB"            /* •ùš.../øš.Æ..ššû */
	$"9A05 4200 5FFF 9190 F99A 0200 073F F99A"            /* š.B._ÿ‘ùš...?ùš */
	$"043E 002E 6F94 FA9A 038E 0000 8BF9 9A02"            /* .>..o”úš...‹ùš. */
	$"3000 B2F7 FF02 B795 98F9 9A03 7C1B 008A"            /* 0.²÷ÿ.·•˜ùš.|..Š */
	$"FA9A 0454 0000 949B FA9A 0342 0007 87FA"            /* úš.T..”›úš.B..‡ú */
	$"9A04 7600 0066 94F8 9A00 0000 50F8 9A01"            /* š.v..f”øš...Pøš. */
	$"211C F99A 058E 0000 4D8C 98FA 9A02 4200"            /* !.ùš...MŒ˜úš.B. */
	$"34FB 4704 AFFF D297 9AF9 9C03 9626 0671"            /* 4ûG.¯ÿÒ—šùœ.–&.q */
	$"FA9C 0479 004A 2F98 F99C 0333 0012 94FA"            /* úœ.y.J/˜ùœ.3..”ú */
	$"9C07 6700 0649 491F 0B95 FA9C 0257 0040"            /* œ.g..II..•úœ.W.@ */
	$"FE49 0100 70F9 9C03 0B00 5D9E FE9C FD9C"            /* şI..pùœ...]şœıœ */
	$"0444 00C8 3C97 F99C 0100 1FFB 4902 5934"            /* .D.È<—ùœ...ûI.Y4 */
	$"92FA 9C03 9000 0C4A F99C 0242 0048 F6FF"            /* ’úœ...Jùœ.B.Höÿ */
	$"01A0 92F9 9C01 6C7C F99C 057D 331D 628D"            /* . ’ùœ.l|ùœ.}3.b */
	$"9BFA 9C02 5700 40F8 4901 0065 F99C 010B"            /* ›úœ.W.@øI..eùœ.. */
	$"17FE 4901 1F2F FC9C FE9C 0344 003A 9BFA"            /* .şI../üœşœ.D.:›ú */
	$"9C03 7600 0097 F99C 021A 0031 F89C 020B"            /* œ.v..—ùœ...1øœ.. */
	$"0030 F99C 0544 0062 FF94 92F9 9C02 000A"            /* .0ùœ.D.bÿ”’ùœ..Â */
	$"43F9 9C04 4200 3072 96FA 9C03 9000 008D"            /* Cùœ.B.0r–úœ... */
	$"F99C 0233 00B3 F7FF 02B8 979A F99C 057F"            /* ùœ.3.³÷ÿ.¸—šùœ.. */
	$"1D01 8D9C 9CFC 9C04 5700 0096 9EFA 9C03"            /* ..œœüœ.W..–úœ. */
	$"4400 0A8A FA9C 0479 0000 6996 F89C 0100"            /* D.ÂŠúœ.y..i–øœ.. */
	$"53F8 9C01 241F F99C 0590 0000 4F8E 9AFA"            /* Søœ.$.ùœ...Ošú */
	$"9C02 4400 36FB 4904 B0FF D4A2 A5F9 A603"            /* œ.D.6ûI.°ÿÔ¢¥ù¦. */
	$"A131 107C FAA6 0485 0750 39A3 F9A6 053E"            /* ¡1.|ú¦.….P9£ù¦.> */
	$"071C 9FA6 A6FC A607 7107 1053 5329 15A1"            /* ..Ÿ¦¦ü¦.q..SS).¡ */
	$"FAA6 0264 074A FE53 0107 7BF9 A603 1607"            /* ú¦.d.JşS..{ù¦... */
	$"69A8 FAA6 0450 07CC 49A2 F9A6 0107 29FB"            /* i¨ú¦.P.ÌI¢ù¦..)û */
	$"5302 6540 9FFA A603 9C07 1755 F9A6 024D"            /* S.e@Ÿú¦.œ..Uù¦.M */
	$"074E F6FF 01AA 9FF9 A601 7787 F9A6 038A"            /* .Nöÿ.ªŸù¦.w‡ù¦.Š */
	$"432B 6E01 9AA5 FAA6 0264 074A F853 0107"            /* C+n.š¥ú¦.d.JøS.. */
	$"70F9 A601 1620 FE53 0129 3BF9 A603 5007"            /* pù¦.. şS.);ù¦.P. */
	$"47A5 FAA6 037F 0707 A2F9 A602 2507 3BF8"            /* G¥ú¦....¢ù¦.%.;ø */
	$"A602 1607 3CF9 A605 5007 6CFF A09F F9A6"            /* ¦...<ù¦.P.lÿ Ÿù¦ */
	$"0207 144E F9A6 044D 073A 80A1 FBA6 04A6"            /* ...Nù¦.M.:€¡û¦.¦ */
	$"9C07 079A F9A6 023E 07B9 F7FF 02BF A2A5"            /* œ..šù¦.>.¹÷ÿ.¿¢¥ */
	$"F9A6 038A 280A 99FA A604 6407 07A1 A8FA"            /* ù¦.Š(Â™ú¦.d..¡¨ú */
	$"A603 5007 1495 FAA6 0485 0707 77A1 F8A6"            /* ¦.P..•ú¦.…..w¡ø¦ */
	$"0107 5EF8 A601 2F2A F9A6 059C 0707 5A9B"            /* ..^ø¦./*ù¦.œ..Z› */
	$"A5FA A602 5007 3FFB 5300 B303 0203 FFD1"            /* ¥ú¦.P.?ûS.³...ÿÑ */
	$"9598 F997 0380 0000 85FA 9704 7300 482C"            /* •˜ù—.€..…ú—.s.H, */
	$"95F9 9702 2F00 3EF9 9707 6300 27FF FF5F"            /* •ù—./.>ù—.c.'ÿÿ_ */
	$"4294 FA97 0252 00C8 FEFF 0100 86F9 9703"            /* B”ú—.R.Èşÿ..†ù—. */
	$"0900 5B98 FA97 043F 00C8 3A92 F997 0100"            /* Æ.[˜ú—.?.È:’ù—.. */
	$"5FFA FF01 318F FA97 038B 0000 6CFD 97FD"            /* _úÿ.1ú—.‹..lı—ı */
	$"9702 3D00 2EF6 FF01 9C90 F997 0222 168C"            /* —.=..öÿ.œù—.".Œ */
	$"F997 0463 0016 8B96 FA97 0252 00C8 F8FF"            /* ù—.c..‹–ú—.R.Èøÿ */
	$"0100 8DF9 9701 0948 FEFF 014D 4EF9 9703"            /* ..ù—.ÆHşÿ.MNù—. */
	$"3F00 3896 FA97 0370 0000 95F9 9703 1600"            /* ?.8–ú—.p..•ù—... */
	$"2F99 F997 0409 0068 9997 FB97 053F 005F"            /* /™ù—.Æ.h™—û—.?._ */
	$"FF81 8FF9 97FF 0000 68F9 9704 3D00 2E52"            /* ÿù—ÿ..hù—.=..R */
	$"91FA 9703 8B00 008E F997 022F 00AD F7FF"            /* ‘ú—.‹..ù—./.­÷ÿ */
	$"02B6 9596 F997 0364 000C 92FA 9704 5200"            /* .¶•–ù—.d..’ú—.R. */
	$"0094 98FA 9702 3F00 32F9 9704 7300 0039"            /* .”˜ú—.?.2ù—.s..9 */
	$"94F8 9700 0000 6DF8 9701 1F36 F997 058B"            /* ”ø—...mø—..6ù—.‹ */
	$"0000 3B8C 96FA 9702 3F00 AAF9 FF02 D297"            /* ..;Œ–ú—.?.ªùÿ.Ò— */
	$"9AF9 9903 8200 0087 FA99 0476 004A 2F97"            /* šù™.‚..‡ú™.v.J/— */
	$"F999 0232 0042 F999 0765 0029 FFFF 6244"            /* ù™.2.Bù™.e.)ÿÿbD */
	$"96FA 9902 5500 C9FE FF01 0088 F999 030B"            /* –ú™.U.Éşÿ..ˆù™.. */
	$"005D 9AFE 99FD 9904 4300 C83C 95F9 9901"            /* .]šş™ı™.C.È<•ù™. */
	$"0062 FAFF 0134 91FA 9903 8D00 006E F999"            /* .búÿ.4‘ú™...nù™ */
	$"023F 0030 F6FF 019F 92F9 9902 251A 8EF9"            /* .?.0öÿ.Ÿ’ù™.%.ù */
	$"9904 6500 198D 98FA 9902 5500 C9F8 FF01"            /* ™.e..˜ú™.U.Éøÿ. */
	$"028F F999 010B 4AFE FF01 4F51 FC99 FE99"            /* .ù™..Jşÿ.OQü™ş™ */
	$"0343 003A 98FA 9903 7200 0097 F999 031A"            /* .C.:˜ú™.r..—ù™.. */
	$"0031 9BF9 9903 0B00 6B9B FA99 0543 0062"            /* .1›ù™...k›ú™.C.b */
	$"FF84 91F9 99FF 0000 6CF9 9904 3F00 3054"            /* ÿ„‘ù™ÿ..lù™.?.0T */
	$"94FA 9903 8D00 0090 F999 0232 00AE F7FF"            /* ”ú™...ù™.2.®÷ÿ */
	$"02B7 9798 F999 0566 0010 9599 99FC 9904"            /* .·—˜ù™.f..•™™ü™. */
	$"5500 0096 9AFA 9902 4300 35F9 9904 7600"            /* U..–šú™.C.5ù™.v. */
	$"003B 96F8 9901 006F F899 0121 39F9 9905"            /* .;–ø™..oø™.!9ù™. */
	$"8D00 003D 8E98 FA99 0243 00AA F9FF 02D4"            /* ..=˜ú™.C.ªùÿ.Ô */
	$"A2A5 F9A4 038F 0707 95FA A404 8307 5039"            /* ¢¥ù¤...•ú¤.ƒ.P9 */
	$"A2F9 A402 3D07 4EFE A4FC A407 6F07 34FF"            /* ¢ù¤.=.Nş¤ü¤.o.4ÿ */
	$"FF6C 50A2 FAA4 0262 07CC FEFF 0108 96F9"            /* ÿlP¢ú¤.b.Ìşÿ..–ù */
	$"A403 1607 69A5 FAA4 044F 07CC 49A1 F9A4"            /* ¤...i¥ú¤.O.ÌI¡ù¤ */
	$"0107 6CFA FF01 409E FAA4 039A 0707 79F9"            /* ..lúÿ.@ú¤.š..yù */
	$"A402 4C07 3AF6 FF01 A99F F9A4 022E 279B"            /* ¤.L.:öÿ.©Ÿù¤..'› */
	$"F9A4 026F 0725 019A A3FA A402 6207 CCF8"            /* ù¤.o.%.š£ú¤.b.Ìø */
	$"FF01 0C9B F9A4 0116 50FE FF01 555C F9A4"            /* ÿ..›ù¤..Pşÿ.U\ù¤ */
	$"034F 0747 A3FA A403 7C07 07A2 F9A4 0325"            /* .O.G£ú¤.|..¢ù¤.% */
	$"073B A5F9 A403 1607 78A5 FAA4 054F 076C"            /* .;¥ù¤...x¥ú¤.O.l */
	$"FF91 9EF9 A4FF 0700 76F9 A404 4C07 3A62"            /* ÿ‘ù¤ÿ..vù¤.L.:b */
	$"A0FB A404 A49A 0707 9CF9 A402 3D07 B5F7"            /*  û¤.¤š..œù¤.=.µ÷ */
	$"FF02 BEA2 A3F9 A403 7207 1BA1 FAA4 0462"            /* ÿ.¾¢£ù¤.r..¡ú¤.b */
	$"0707 A1A5 FAA4 024F 0740 F9A4 0483 0707"            /* ..¡¥ú¤.O.@ù¤.ƒ.. */
	$"46A1 F8A4 0107 7DF8 A401 2E46 F9A4 059A"            /* F¡ø¤..}ø¤..Fù¤.š */
	$"0707 459B A3FA A402 4F07 ACFB FF00 FF02"            /* ..E›£ú¤.O.¬ûÿ.ÿ. */
	$"F603 FFD1 9598 F994 037A 0000 90FA 9403"            /* ö.ÿÑ•˜ù”.z..ú”. */
	$"7100 482C F894 032F 004A 96FA 9407 6100"            /* q.H,ø”./.J–ú”.a. */
	$"2EFF FF5F 7891 FA94 0251 00C8 FEFF 0126"            /* .ÿÿ_x‘ú”.Q.Èşÿ.& */
	$"90F9 9403 0900 5B96 FA94 043E 00C8 3A92"            /* ù”.Æ.[–ú”.>.È:’ */
	$"F994 0100 5FFA FF01 498E FA94 0388 0000"            /* ù”.._úÿ.Iú”.ˆ.. */
	$"6EFD 94FD 9402 3C00 2EF6 FF01 9C90 F994"            /* nı”ı”.<..öÿ.œù” */
	$"0221 0880 F994 0361 0000 83F9 9402 5100"            /* .!.€ù”.a..ƒù”.Q. */
	$"C8F8 FF01 3091 F994 0109 48FE FF01 4879"            /* Èøÿ.0‘ù”.ÆHşÿ.Hy */
	$"F994 023E 0038 F994 027A 0000 F894 0316"            /* ù”.>.8ù”.z..ø”.. */
	$"002F 98F9 9404 0900 6E96 94FB 9405 3E00"            /* ./˜ù”.Æ.n–”û”.>. */
	$"5FFF 668E F994 FF00 0071 F994 043C 002E"            /* _ÿfù”ÿ..qù”.<.. */
	$"3B90 FA94 0388 0000 98F9 9402 2F00 A5F7"            /* ;ú”.ˆ..˜ù”./.¥÷ */
	$"FF02 B695 96F9 9403 5E00 1990 FA94 0451"            /* ÿ.¶•–ù”.^..ú”.Q */
	$"0000 9496 FA94 033E 003B 96FA 9403 7100"            /* ..”–ú”.>.;–ú”.q. */
	$"002C F794 0000 006F F894 011F 36F9 9404"            /* .,÷”...oø”..6ù”. */
	$"8800 0035 8CF9 9402 3E00 AAF9 FF02 D197"            /* ˆ..5Œù”.>.ªùÿ.Ñ— */
	$"9AF9 9603 7D00 0094 FA96 0375 004A 2FF8"            /* šù–.}..”ú–.u.J/ø */
	$"9603 3100 4E98 FA96 0763 0030 FFFF 627A"            /* –.1.N˜ú–.c.0ÿÿbz */
	$"94FA 9602 5400 C9FE FF01 2892 F996 030B"            /* ”ú–.T.Éşÿ.(’ù–.. */
	$"005D 98FE 96FD 9604 4200 C83C 95F9 9601"            /* .]˜ş–ı–.B.È<•ù–. */
	$"0062 FAFF 014C 90FA 9603 8B00 0070 F996"            /* .búÿ.Lú–.‹..pù– */
	$"023F 0030 F6FF 019F 92F9 9602 240A 82F9"            /* .?.0öÿ.Ÿ’ù–.$Â‚ù */
	$"9603 6300 0085 F996 0254 00C9 F8FF 0133"            /* –.c..…ù–.T.Éøÿ.3 */
	$"95F9 9601 0B4A FEFF 014A 7BFC 96FE 9602"            /* •ù–..Jşÿ.J{ü–ş–. */
	$"4200 3AF9 9602 7D00 00F8 9603 1A00 319A"            /* B.:ù–.}..ø–...1š */
	$"F996 030B 0070 99FA 9605 4200 62FF 6990"            /* ù–...p™ú–.B.bÿi */
	$"F996 FF00 0073 F996 043F 0030 3D94 FA96"            /* ù–ÿ..sù–.?.0=”ú– */
	$"038B 0000 9AF9 9602 3100 A6F7 FF02 B797"            /* .‹..šù–.1.¦÷ÿ.·— */
	$"98F9 9605 6200 1C94 9696 FC96 0454 0000"            /* ˜ù–.b..”––ü–.T.. */
	$"9698 FA96 0342 003E 98FA 9603 7500 002F"            /* –˜ú–.B.>˜ú–.u../ */
	$"F796 0100 72F8 9601 2139 F996 048B 0000"            /* ÷–..rø–.!9ù–.‹.. */
	$"388E F996 0242 00AA F9FF 02D4 A2A5 F9A1"            /* 8ù–.B.ªùÿ.Ô¢¥ù¡ */
	$"038A 0707 9FFA A103 8207 5039 F8A1 053D"            /* .Š..Ÿú¡.‚.P9ø¡.= */
	$"075B A3A1 A1FC A107 6D07 3AFF FF6C 88A0"            /* .[£¡¡ü¡.m.:ÿÿlˆ  */
	$"FAA1 0262 07CC FEFF 0135 9FF9 A103 1607"            /* ú¡.b.Ìşÿ.5Ÿù¡... */
	$"69A3 FAA1 034E 07CC 49F8 A101 076C FAFF"            /* i£ú¡.N.ÌIø¡..lúÿ */
	$"015B 9CFA A103 9807 077A F9A1 024C 073A"            /* .[œú¡.˜..zù¡.L.: */
	$"F6FF 01A9 9FF9 A102 2E15 90F9 A102 6D07"            /* öÿ.©Ÿù¡...ù¡.m. */
	$"0800 94F9 A102 6207 CCF8 FF01 3FA0 F9A1"            /* ..”ù¡.b.Ìøÿ.? ù¡ */
	$"0116 50FE FF01 5088 F9A1 024E 0747 F9A1"            /* ..Pşÿ.Pˆù¡.N.Gù¡ */
	$"038A 0707 A2F9 A103 2507 3BA5 F9A1 0316"            /* .Š..¢ù¡.%.;¥ù¡.. */
	$"077F A3FA A105 4E07 6CFF 7A9C F9A1 FF07"            /* ..£ú¡.N.lÿzœù¡ÿ. */
	$"007D F9A1 044C 073A 499F FBA1 04A1 9807"            /* .}ù¡.L.:IŸû¡.¡˜. */
	$"07A5 F9A1 023D 07AB F7FF 02BE A2A3 F9A1"            /* .¥ù¡.=.«÷ÿ.¾¢£ù¡ */
	$"036E 0729 9FFA A104 6207 07A1 A3FA A103"            /* .n.)Ÿú¡.b..¡£ú¡. */
	$"4E07 4AA3 FAA1 0382 0707 39F7 A101 0780"            /* N.J£ú¡.‚..9÷¡..€ */
	$"F8A1 012E 46F9 A104 9807 073E 9BF9 A102"            /* ø¡..Fù¡.˜..>›ù¡. */
	$"4E07 ACFB FF00 FF03 0003 FFD1 9598 F990"            /* N.¬ûÿ.ÿ...ÿÑ•˜ù */
	$"0270 0002 F990 0470 004A 2C94 F990 032E"            /* .p..ù.p.J,”ù.. */
	$"004A 95FA 9007 5E00 2EFF FF9F 8B8F FA90"            /* .J•ú.^..ÿÿŸ‹ú */
	$"0251 00C8 FEFF 0166 8FF9 9003 0900 5B94"            /* .Q.Èşÿ.fù.Æ.[” */
	$"FA90 043E 00C8 3A91 F990 0100 5FFA FF01"            /* ú.>.È:‘ù.._úÿ. */
	$"7A8D FA90 0386 0000 70FD 90FD 9002 3C00"            /* zú.†..pıı.<. */
	$"2EF6 FF00 9CF8 9002 2108 81F9 9004 5E00"            /* .öÿ.œø.!.ù.^. */
	$"0063 91FA 9002 5100 C8F8 FF01 7591 F990"            /* .c‘ú.Q.Èøÿ.u‘ù */
	$"0109 48FE FF00 7AF8 9003 4200 3894 FA90"            /* .ÆHşÿ.zø.B.8”ú */
	$"0378 0000 94F9 9003 1600 2F98 F990 0409"            /* .x..”ù.../˜ù.Æ */
	$"006E 9590 FB90 053E 005F FF5F 8DF9 90FF"            /* .n•û.>._ÿ_ùÿ */
	$"0000 6CF9 9003 3C00 2E36 F990 0386 0006"            /* ..lù.<..6ù.†.. */
	$"94F9 9002 2E00 96F7 FF02 B595 95F9 9003"            /* ”ù...–÷ÿ.µ••ù. */
	$"5300 288F FA90 0451 0000 9494 FA90 033E"            /* S.(ú.Q..””ú.> */
	$"003B 96FA 9004 7000 002C 94F9 9001 8300"            /* .;–ú.p..,”ù.ƒ. */
	$"006F F890 011F 36F9 9005 8600 0035 8C91"            /* .oø..6ù.†..5Œ‘ */
	$"FA90 023E 00AA F9FF 02D1 979A F994 0372"            /* ú.>.ªùÿ.Ñ—šù”.r */
	$"0006 92FA 9404 7200 4D2F 96F9 9403 3100"            /* ..’ú”.r.M/–ù”.1. */
	$"4E97 FA94 0761 0030 FFFF A18D 92FA 9402"            /* N—ú”.a.0ÿÿ¡’ú”. */
	$"5300 C9FE FF01 6991 F994 030B 005D 97FE"            /* S.Éşÿ.i‘ù”...]—ş */
	$"94FD 9403 4000 C83C F894 0100 62FA FF01"            /* ”ı”.@.È<ø”..búÿ. */
	$"7C8F FA94 038A 0000 72F9 9402 3E00 30F6"            /* |ú”.Š..rù”.>.0ö */
	$"FF01 9F92 F994 0224 0A84 F994 0461 0000"            /* ÿ.Ÿ’ù”.$Â„ù”.a.. */
	$"6595 FA94 0253 00C9 F8FF 0077 F894 010B"            /* e•ú”.S.Éøÿ.wø”.. */
	$"4AFE FF01 7C92 FC94 FE94 0344 003A 96FA"            /* Jşÿ.|’ü”ş”.D.:–ú */
	$"9403 7B00 0096 F994 031A 0031 9AF9 9403"            /* ”.{..–ù”...1šù”. */
	$"0B00 7097 FA94 0540 0062 FF63 8FF9 94FF"            /* ..p—ú”.@.bÿcù”ÿ */
	$"0000 6EF9 9404 3E00 3039 92FA 9403 8A00"            /* ..nù”.>.09’ú”.Š. */
	$"0896 F994 0231 0097 F7FF 02B6 9797 F994"            /* .–ù”.1.—÷ÿ.¶——ù” */
	$"0555 002A 9194 94FC 9404 5300 0096 96FA"            /* .U.*‘””ü”.S..––ú */
	$"9403 4000 3E98 FA94 0472 0000 2F96 F994"            /* ”.@.>˜ú”.r../–ù” */
	$"0285 0072 F894 0121 39F9 9404 8A00 0038"            /* .….rø”.!9ù”.Š..8 */
	$"8EF9 9402 4000 AAF9 FF02 D4A2 A5F9 9F02"            /* ù”.@.ªùÿ.Ô¢¥ùŸ. */
	$"7F07 10F9 9F04 8007 5239 A1F9 9F05 3C07"            /* ...ùŸ.€.R9¡ùŸ.<. */
	$"5BA2 9F9F FC9F 076C 073A FFFF AA9A 9EFA"            /* [¢ŸŸüŸ.l.:ÿÿªšú */
	$"9F02 6107 CCFE FF01 789E F99F 0316 0769"            /* Ÿ.a.Ìşÿ.xùŸ...i */
	$"A2FA 9F04 4E07 CC49 A0F9 9F01 076C FAFF"            /* ¢úŸ.N.ÌI ùŸ..lúÿ */
	$"018C 9CFA 9F03 9707 077D F99F 024A 073A"            /* .ŒœúŸ.—..}ùŸ.J.: */
	$"F6FF 00A9 F89F 022C 1591 F99F 026C 0707"            /* öÿ.©øŸ.,.‘ùŸ.l.. */
	$"0172 A0FA 9F02 6107 CCF8 FF01 85A0 F99F"            /* .r úŸ.a.Ìøÿ.… ùŸ */
	$"0116 50FE FF00 85F8 9F03 5107 47A1 FA9F"            /* ..Pşÿ.…øŸ.Q.G¡úŸ */
	$"0388 0707 A1F9 9F03 2507 3BA4 F99F 0316"            /* .ˆ..¡ùŸ.%.;¤ùŸ.. */
	$"077F A2FA 9F05 4E07 6CFF 729C F99F FF07"            /* ..¢úŸ.N.lÿrœùŸÿ. */
	$"0079 F99F 034A 073A 46FA 9F04 9F97 0713"            /* .yùŸ.J.:FúŸ.Ÿ—.. */
	$"A2F9 9F02 3C07 99F7 FF02 BEA2 A2F9 9F03"            /* ¢ùŸ.<.™÷ÿ.¾¢¢ùŸ. */
	$"6207 359E FA9F 0461 0707 A1A1 FA9F 034E"            /* b.5úŸ.a..¡¡úŸ.N */
	$"074A A3FA 9F04 8007 0739 A1F9 9F02 9407"            /* .J£úŸ.€..9¡ùŸ.”. */
	$"80F8 9F01 2E46 F99F 0597 0707 3E9B A0FA"            /* €øŸ..FùŸ.—..>› ú */
	$"9F02 4E07 ACFB FF00 FF03 0603 FFCF 9498"            /* Ÿ.N.¬ûÿ.ÿ...ÿÏ”˜ */
	$"F98D 0368 0001 12FA 0C04 0900 5F2C 91F9"            /* ù.h...ú..Æ._,‘ù */
	$"8D03 2C00 4A92 FA8D 065C 002E FFFF B28B"            /* .,.J’ú.\..ÿÿ²‹ */
	$"F98D 024F 00C8 FEFF 0188 8EF9 8D03 0900"            /* ù.O.Èşÿ.ˆù.Æ. */
	$"5B91 FA8D 043C 00C8 3A90 F98D 0100 5FFA"            /* [‘ú.<.È:ù.._ú */
	$"FF01 908C FA8D 0383 0000 68FD 8DFD 8D02"            /* ÿ.Œú.ƒ..hıı. */
	$"3B00 2EF6 FF01 9C90 F98D 0220 0882 F98D"            /* ;..öÿ.œù. .‚ù */
	$"045C 0000 5B8F FA8D 024F 00C8 F8FF 0188"            /* .\..[ú.O.Èøÿ.ˆ */
	$"90F9 8D01 0948 FEFF 01A9 8EF9 8D03 4F00"            /* ù.ÆHşÿ.©ù.O. */
	$"3C91 FA8D 0376 0000 92F9 8D03 1600 2F96"            /* <‘ú.v..’ù.../– */
	$"F98D 0409 006E 918D FB8D 053C 005F FF5F"            /* ù.Æ.n‘û.<._ÿ_ */
	$"8CF9 8DFF 0000 68F9 8D04 3B00 2E36 8EFA"            /* Œùÿ..hù.;..6ú */
	$"8D03 8300 1392 F98D 022C 0096 F7FF 02B5"            /* .ƒ..’ù.,.–÷ÿ.µ */
	$"9594 F98D 034E 000D 0DFA 0C04 0500 0094"            /* •”ù.N...ú.....” */
	$"90FA 8D03 3C00 3B94 FA8D 046D 0000 2C94"            /* ú.<.;”ú.m..,” */
	$"F98D 0180 0000 6FF8 8D01 1B32 F98D 0583"            /* ù.€..oø..2ù.ƒ */
	$"0000 358C 8FFA 8D02 3C00 AAF9 FF02 D196"            /* ..5Œú.<.ªùÿ.Ñ– */
	$"9AF9 8F03 6B00 0315 FA0F 040B 0062 2F95"            /* šù.k...ú....b/• */
	$"F98F 0330 004E 95FA 8F06 5E00 30FF FFB3"            /* ù.0.N•ú.^.0ÿÿ³ */
	$"8DF9 8F02 5200 C9FE FF01 8B90 F98F 030B"            /* ù.R.Éşÿ.‹ù.. */
	$"005D 94FE 8FFD 8F04 3F00 C83C 92F9 8F01"            /* .]”şı.?.È<’ù. */
	$"0062 FAFF 0192 8EFA 8F03 8600 006B F98F"            /* .búÿ.’ú.†..kù */
	$"023D 0030 F6FF 019E 92F9 8F02 220A 85F9"            /* .=.0öÿ.’ù."Â…ù */
	$"8F04 5E00 005D 91FA 8F02 5200 C9F8 FF01"            /* .^..]‘ú.R.Éøÿ. */
	$"8B92 F98F 010B 4AFE FF01 AB90 FC8F FE8F"            /* ‹’ù..Jşÿ.«üş */
	$"0352 003E 95FA 8F03 7800 0095 F98F 031A"            /* .R.>•ú.x..•ù.. */
	$"0031 99F9 8F03 0B00 7094 FA8F 053F 0062"            /* .1™ù...p”ú.?.b */
	$"FF63 8EF9 8FFF 0000 6BF9 8F04 3D00 3039"            /* ÿcùÿ..kù.=.09 */
	$"91FA 8F03 8600 1595 F98F 0230 0097 F7FF"            /* ‘ú.†..•ù.0.—÷ÿ */
	$"02B6 9796 F98F 0551 0010 100F 0FFC 0F04"            /* .¶—–ù.Q.....ü.. */
	$"0700 0096 92FA 8F03 3F00 3D96 FA8F 0470"            /* ...–’ú.?.=–ú.p */
	$"0000 2F96 F98F 0283 0071 F88F 011D 34F9"            /* ../–ù.ƒ.qø..4ù */
	$"8F05 8600 0038 8E91 FA8F 023F 00AA F9FF"            /* .†..8‘ú.?.ªùÿ */
	$"02D3 A1A5 F99B 0377 070D 20FA 1B04 1607"            /* .Ó¡¥ù›.w.. ú.... */
	$"6C39 A0F9 9B05 3B07 5BA1 9B9B FC9B 0669"            /* l9 ù›.;.[¡››ü›.i */
	$"073A FFFF BA9A F99B 025F 07CC FEFF 0198"            /* .:ÿÿºšù›._.Ìşÿ.˜ */
	$"9EF9 9B03 1607 699F FA9B 044D 07CC 499F"            /* ù›...iŸú›.M.ÌIŸ */
	$"F99B 0107 6CFA FF00 9FF9 9B03 9407 0777"            /* ù›..lúÿ.Ÿù›.”..w */
	$"F99B 0249 073A F6FF 01A9 9FF9 9B02 2C15"            /* ù›.I.:öÿ.©Ÿù›.,. */
	$"92F9 9B02 6907 0701 699E FA9B 025F 07CC"            /* ’ù›.i...iú›._.Ì */
	$"F8FF 0198 9FF9 9B01 1650 FEFF 01B3 9CF9"            /* øÿ.˜Ÿù›..Pşÿ.³œù */
	$"9B03 5F07 4AA0 FA9B 0385 0707 A0F9 9B03"            /* ›._.J ú›.….. ù›. */
	$"2507 3BA4 F99B 0316 077D A0FA 9B04 4D07"            /* %.;¤ù›...} ú›.M. */
	$"6CFF 72F8 9BFF 0700 77F9 9B04 4907 3A46"            /* lÿrø›ÿ..wù›.I.:F */
	$"9EFB 9B04 9B94 0722 A0F9 9B02 3B07 99F7"            /* û›.›”." ù›.;.™÷ */
	$"FF02 BDA2 A2F9 9B03 5D07 1B1C FA1B 0412"            /* ÿ.½¢¢ù›.]...ú... */
	$"0707 A19F FA9B 034D 074A A1FA 9B04 7D07"            /* ..¡Ÿú›.M.J¡ú›.}. */
	$"0739 A1F9 9B02 9107 7FF8 9B01 2942 F99B"            /* .9¡ù›.‘..ø›.)Bù› */
	$"0594 0707 3E9B 9EFA 9B02 4D07 ACFB FF00"            /* .”..>›ú›.M.¬ûÿ. */
	$"FF02 FA03 FFCF 9498 F98A 0262 0000 F7AD"            /* ÿ.ú.ÿÏ”˜ùŠ.b..÷­ */
	$"029A 2C91 F98A 032C 004A 91FA 8A07 5A00"            /* .š,‘ùŠ.,.J‘úŠ.Z. */
	$"2EFF FFB0 8B8B FA8A 024F 00C8 FEFF 0187"            /* .ÿÿ°‹‹úŠ.O.Èşÿ.‡ */
	$"8EF9 8A03 0900 5B8F FA8A 043C 00C8 3A90"            /* ùŠ.Æ.[úŠ.<.È: */
	$"F98A 0100 5FFA FF01 908B FA8A 0381 0000"            /* ùŠ.._úÿ.‹úŠ... */
	$"66FD 8AFD 8A02 3A00 2EF6 FF01 9B90 F98A"            /* fıŠıŠ.:..öÿ.›ùŠ */
	$"021F 0883 F98A 045A 0000 5B8D FA8A 024F"            /* ...ƒùŠ.Z..[úŠ.O */
	$"00C8 F8FF 0188 8FF9 8A01 0948 FEFF 01A9"            /* .Èøÿ.ˆùŠ.ÆHşÿ.© */
	$"8EF9 8A03 4F00 4690 FA8A 0373 0000 8EF9"            /* ùŠ.O.FúŠ.s..ù */
	$"8A03 1600 2F96 F98A 0409 006E 8F8A FB8A"            /* Š.../–ùŠ.Æ.nŠûŠ */
	$"053C 005F FF5F 8BF9 8AFF 0000 68F9 8A04"            /* .<._ÿ_‹ùŠÿ..hùŠ. */
	$"3A00 2E36 8EFA 8A03 8100 1D91 F98A 022C"            /* :..6úŠ...‘ùŠ., */
	$"007F F7FF 02B4 9594 F98A 0243 0021 F8AD"            /* ..÷ÿ.´•”ùŠ.C.!ø­ */
	$"0399 0094 8EFA 8A03 3C00 3B92 FA8A 046B"            /* .™.”úŠ.<.;’úŠ.k */
	$"0000 2C94 F98A 017D 0000 6FF8 8A02 0C2A"            /* ..,”ùŠ.}..oøŠ..* */
	$"90FA 8A05 8100 0039 8C8D FA8A 023C 00AA"            /* úŠ...9ŒúŠ.<.ª */
	$"F9FF 02D1 969A F98D 0265 0000 F7AE 029C"            /* ùÿ.Ñ–šù.e..÷®.œ */
	$"2F94 F98D 032F 004E 94FA 8D05 5C00 30FF"            /* /”ù./.N”ú.\.0ÿ */
	$"FFB2 F88D 0251 00C9 FEFF 018A 90F9 8D03"            /* ÿ²ø.Q.Éşÿ.Šù. */
	$"0B00 5D91 FE8D FD8D 043E 00C8 3C92 F98D"            /* ..]‘şı.>.È<’ù */
	$"0100 62FA FF00 92F9 8D03 8400 0069 F98D"            /* ..búÿ.’ù.„..iù */
	$"023D 0030 F6FF 019E 92F9 8D02 210B 85F9"            /* .=.0öÿ.’ù.!.…ù */
	$"8D04 5C00 005D 8FFA 8D02 5100 C9F8 FF01"            /* .\..]ú.Q.Éøÿ. */
	$"8B91 F98D 010B 4AFE FF01 AA90 FC8D FE8D"            /* ‹‘ù..Jşÿ.ªüş */
	$"0351 0048 92FA 8D03 7600 0090 F98D 031A"            /* .Q.H’ú.v..ù.. */
	$"0031 98F9 8D03 0B00 7091 FA8D 043E 0062"            /* .1˜ù...p‘ú.>.b */
	$"FF63 F88D FF00 006B F98D 043D 0030 3990"            /* ÿcøÿ..kù.=.09 */
	$"FA8D 0384 001F 94F9 8D02 2F00 81F7 FF02"            /* ú.„..”ù./.÷ÿ. */
	$"B597 96F9 8D02 4500 24FE AEFB AE03 9A00"            /* µ—–ù.E.$ş®û®.š. */
	$"9691 FA8D 033E 003D 95FA 8D04 6E00 002F"            /* –‘ú.>.=•ú.n../ */
	$"96F9 8D02 8100 71F8 8D02 0F2C 92FA 8D05"            /* –ù..qø..,’ú. */
	$"8400 003B 8E8F FA8D 023E 00AA F9FF 02D3"            /* „..;ú.>.ªùÿ.Ó */
	$"A1A5 F999 0271 0707 F7B5 02A5 399F F999"            /* ¡¥ù™.q..÷µ.¥9Ÿù™ */
	$"053B 075B 9F99 99FC 9906 6707 3AFF FFB9"            /* .;.[Ÿ™™ü™.g.:ÿÿ¹ */
	$"9AF9 9902 5E07 CCFE FF01 989C F999 0316"            /* šù™.^.Ìşÿ.˜œù™.. */
	$"0768 9EFA 9904 4C07 CC49 9FF9 9901 076C"            /* .hú™.L.ÌIŸù™..l */
	$"FAFF 019F 9AFA 9903 9207 0775 F999 0249"            /* úÿ.Ÿšú™.’..uù™.I */
	$"073A F6FF 01A9 9FF9 9902 2B16 94F9 9902"            /* .:öÿ.©Ÿù™.+.”ù™. */
	$"6707 0701 699B FA99 025E 07CC F8FF 0198"            /* g...i›ú™.^.Ìøÿ.˜ */
	$"9EF9 9901 1650 FEFF 01B3 9CF9 9903 5E07"            /* ù™..Pşÿ.³œù™.^. */
	$"529E FA99 0384 0707 9CF9 9903 2507 3BA3"            /* Rú™.„..œù™.%.;£ */
	$"F999 0316 077D 9EFA 9905 4C07 6CFF 729A"            /* ù™...}ú™.L.lÿrš */
	$"F999 FF07 0076 F999 0449 073A 469C FB99"            /* ù™ÿ..vù™.I.:Fœû™ */
	$"0499 9207 2B9F F999 023B 0786 F7FF 02BB"            /* .™’.+Ÿù™.;.†÷ÿ.» */
	$"A2A1 F999 0250 072F F8B5 03A4 07A1 9CFA"            /* ¢¡ù™.P./øµ.¤.¡œú */
	$"9903 4C07 4AA0 FA99 047C 0707 39A1 F999"            /* ™.L.J ú™.|..9¡ù™ */
	$"028F 077F F899 021B 399F FA99 0592 0707"            /* ...ø™..9Ÿú™.’.. */
	$"439B 9BFA 9902 4C07 ACFB FF00 FF02 FF03"            /* C››ú™.L.¬ûÿ.ÿ.ÿ. */
	$"FFCF 9498 F987 025C 0000 F7FF 02AD 3290"            /* ÿÏ”˜ù‡.\..÷ÿ.­2 */
	$"F987 032B 004A 90FA 8707 5800 3DFF FFB0"            /* ù‡.+.Jú‡.X.=ÿÿ° */
	$"8B88 FA87 024E 00C8 FEFF 0187 8DF9 8703"            /* ‹ˆú‡.N.Èşÿ.‡ù‡. */
	$"0900 5A8D FA87 043B 00C8 3A8F F987 0100"            /* Æ.Zú‡.;.È:ù‡.. */
	$"5FFA FF01 908A FA87 0380 0000 6BFD 87FD"            /* _úÿ.Šú‡.€..kı‡ı */
	$"8702 3A00 2EF6 FF01 9B8F F987 021F 0883"            /* ‡.:..öÿ.›ù‡...ƒ */
	$"F987 0458 0000 5B8B FA87 024E 00C8 F8FF"            /* ù‡.X..[‹ú‡.N.Èøÿ */
	$"0188 8FF9 8701 0948 FEFF 019E 8DF9 8703"            /* .ˆù‡.ÆHşÿ.ù‡. */
	$"4E00 468E FA87 0375 0000 8CF9 8703 1600"            /* N.Fú‡.u..Œù‡... */
	$"2F95 F987 0409 006E 8D87 FB87 053B 005F"            /* /•ù‡.Æ.n‡û‡.;._ */
	$"FF5F 8AF9 87FF 0000 6CF9 8704 3A00 2E36"            /* ÿ_Šù‡ÿ..lù‡.:..6 */
	$"8DFA 8703 8005 6988 F987 022B 007A F7FF"            /* ú‡.€.iˆù‡.+.z÷ÿ */
	$"02B3 9592 F987 0242 002E F8FF 03CE 1194"            /* .³•’ù‡.B..øÿ.Î.” */
	$"8CFA 8703 3B00 3B91 FA87 0469 0000 2C94"            /* Œú‡.;.;‘ú‡.i..,” */
	$"F987 017C 0000 6FF8 8702 0C2A 8EFA 8705"            /* ù‡.|..oø‡..*ú‡. */
	$"8000 0047 8C8B FA87 023B 00AA F9FF 02CF"            /* €..GŒ‹ú‡.;.ªùÿ.Ï */
	$"969A F98A 025F 0000 F7FF 02AE 3492 F98A"            /* –šùŠ._..÷ÿ.®4’ùŠ */
	$"032E 004E 92FA 8A07 5B00 3FFF FFB2 8D8B"            /* ...N’úŠ.[.?ÿÿ²‹ */
	$"FA8A 0250 00C9 FEFF 018A 8FF9 8A03 0B00"            /* úŠ.P.Éşÿ.ŠùŠ... */
	$"5C8F FE8A FD8A 043D 00C8 3C91 F98A 0100"            /* \şŠıŠ.=.È<‘ùŠ.. */
	$"62FA FF01 928C FA8A 0382 0000 6DF9 8A02"            /* búÿ.’ŒúŠ.‚..mùŠ. */
	$"3C00 30F6 FF01 9C91 F98A 0221 0B86 F98A"            /* <.0öÿ.œ‘ùŠ.!.†ùŠ */
	$"045B 0000 5D8D FA8A 0250 00C9 F8FF 018B"            /* .[..]úŠ.P.Éøÿ.‹ */
	$"91F9 8A01 0B4A FEFF 01A0 8FFC 8AFE 8A03"            /* ‘ùŠ..Jşÿ. üŠşŠ. */
	$"5000 4890 FA8A 0377 0000 8EF9 8A03 1A00"            /* P.HúŠ.w..ùŠ... */
	$"3197 F98A 030B 0070 8FFA 8A05 3D00 62FF"            /* 1—ùŠ...púŠ.=.bÿ */
	$"638C F98A FF00 006E F98A 043C 0030 398F"            /* cŒùŠÿ..nùŠ.<.09 */
	$"FA8A 0382 076C 8CF9 8A02 2E00 7CF7 FF02"            /* úŠ.‚.lŒùŠ...|÷ÿ. */
	$"B497 95F9 8A02 4400 30FE FFFB FF03 CF13"            /* ´—•ùŠ.D.0şÿûÿ.Ï. */
	$"968F FA8A 033D 003D 94FA 8A04 6C00 002F"            /* –úŠ.=.=”úŠ.l../ */
	$"96F9 8A02 7F00 71F8 8A02 0F2C 90FA 8A05"            /* –ùŠ...qøŠ..,úŠ. */
	$"8200 0049 8E8E FA8A 023D 00AA F9FF 02D3"            /* ‚..IúŠ.=.ªùÿ.Ó */
	$"A1A5 F996 026D 0707 F7FF 02B5 3F9F F996"            /* ¡¥ù–.m..÷ÿ.µ?Ÿù– */
	$"053A 075B 9F96 96FC 9607 6607 47FF FFB9"            /* .:.[Ÿ––ü–.f.Gÿÿ¹ */
	$"9A98 FA96 025E 07CC FEFF 0198 9CF9 9603"            /* š˜ú–.^.Ìşÿ.˜œù–. */
	$"1607 689B FA96 044A 07CC 499E F996 0107"            /* ..h›ú–.J.ÌIù–.. */
	$"6CFA FF01 9F99 FA96 0390 0707 78F9 9602"            /* lúÿ.Ÿ™ú–...xù–. */
	$"4807 3AF6 FF01 A89F F996 022B 1694 F996"            /* H.:öÿ.¨Ÿù–.+.”ù– */
	$"0266 0707 0169 99FA 9602 5E07 CCF8 FF01"            /* .f...i™ú–.^.Ìøÿ. */
	$"989E F996 0116 50FE FF01 AA9B F996 035E"            /* ˜ù–..Pşÿ.ª›ù–.^ */
	$"0752 9CFA 9603 8507 079A F996 0325 073B"            /* .Rœú–.…..šù–.%.; */
	$"A3F9 9603 1607 7D9B FA96 054A 076C FF72"            /* £ù–...}›ú–.J.lÿr */
	$"99F9 96FF 0700 79F9 9604 4807 3A46 9BFB"            /* ™ù–ÿ..yù–.H.:F›û */
	$"9604 9690 1177 98F9 9602 3A07 83F7 FF02"            /* –.–.w˜ù–.:.ƒ÷ÿ. */
	$"BBA2 A1F9 9602 4F07 3AF8 FF03 D31E A19A"            /* »¢¡ù–.O.:øÿ.Ó.¡š */
	$"FA96 034A 074A A0FA 9604 7A07 0739 A1F9"            /* ú–.J.J ú–.z..9¡ù */
	$"9602 8E07 7FF8 9602 1B39 9CFA 9605 9007"            /* –...ø–..9œú–.. */
	$"0753 9B9A FA96 024A 07AC FBFF 00FF 02F8"            /* .S›šú–.J.¬ûÿ.ÿ.ø */
	$"03FF CF92 98F9 8302 5200 00F7 FF02 AD6B"            /* .ÿÏ’˜ùƒ.R..÷ÿ.­k */
	$"8FF9 8303 2A00 4A8D FA83 0755 0048 FFFF"            /* ùƒ.*.Júƒ.U.Hÿÿ */
	$"AF8B 85FA 8302 4C00 C8FE FF01 878C F983"            /* ¯‹…úƒ.L.Èşÿ.‡Œùƒ */
	$"0309 0059 8AFA 8304 3900 C83A 8EF9 8301"            /* .Æ.YŠúƒ.9.È:ùƒ. */
	$"005F FAFF 0190 88FA 8303 7C00 006E FD83"            /* ._úÿ.ˆúƒ.|..nıƒ */
	$"FD83 0239 002E F6FF 019A 8FF9 8302 1D03"            /* ıƒ.9..öÿ.šùƒ... */
	$"84F9 8304 5500 005B 87FA 8302 4C00 C8F8"            /* „ùƒ.U..[‡úƒ.L.Èø */
	$"FF01 888E F983 0109 48FE FF01 9B8E F983"            /* ÿ.ˆùƒ.ÆHşÿ.›ùƒ */
	$"034C 0046 8BFA 8303 7C00 0088 F983 0316"            /* .L.F‹úƒ.|..ˆùƒ.. */
	$"002F 94F9 8304 0900 6E8A 83FB 8305 3900"            /* ./”ùƒ.Æ.nŠƒûƒ.9. */
	$"5FFF 5E88 F983 FF00 006D F983 0439 002E"            /* _ÿ^ˆùƒÿ..mùƒ.9.. */
	$"368B FA83 0180 79F7 8302 2000 71F7 FF02"            /* 6‹úƒ.€y÷ƒ. .q÷ÿ. */
	$"B095 91F9 8302 3900 3CF8 FF03 CE5F 9488"            /* °•‘ùƒ.9.<øÿ.Î_”ˆ */
	$"FA83 0339 003B 90FA 8304 6600 002C 94F9"            /* úƒ.9.;úƒ.f..,”ù */
	$"8301 7500 006C F883 020C 2A8B FA83 057C"            /* ƒ.u..løƒ..*‹úƒ.| */
	$"0000 478C 88FA 8302 3900 AAF9 FF02 CF95"            /* ..GŒˆúƒ.9.ªùÿ.Ï• */
	$"9AF9 8602 5500 00F7 FF02 AE6D 91F9 8603"            /* šù†.U..÷ÿ.®m‘ù†. */
	$"2C00 4D8F FA86 0758 004A FFFF B28D 88FA"            /* ,.Mú†.X.Jÿÿ²ˆú */
	$"8602 4F00 C9FE FF01 8A8E F986 030B 005B"            /* †.O.Éşÿ.Šù†...[ */
	$"8DFE 86FD 8604 3C00 C83C 90F9 8601 0062"            /* ş†ı†.<.È<ù†..b */
	$"FAFF 0192 8BFA 8603 8000 0070 F986 023B"            /* úÿ.’‹ú†.€..pù†.; */
	$"0030 F6FF 019C 91F9 8601 2007 F886 0458"            /* .0öÿ.œ‘ù†. .ø†.X */
	$"0000 5D8B FA86 024F 00C9 F8FF 018B 90F9"            /* ..]‹ú†.O.Éøÿ.‹ù */
	$"8601 0B4A FEFF 019E 90FC 86FE 8603 4F00"            /* †..Jşÿ.ü†ş†.O. */
	$"488E FA86 0380 0000 8CF9 8603 1A00 3196"            /* Hú†.€..Œù†...1– */
	$"F986 030B 0070 8DFA 8605 3C00 62FF 628B"            /* ù†...pú†.<.bÿb‹ */
	$"F986 FF00 006F F986 043B 0030 398E FA86"            /* ù†ÿ..où†.;.09ú† */
	$"0183 7BF7 8602 2400 73F7 FF02 B397 94F9"            /* .ƒ{÷†.$.s÷ÿ.³—”ù */
	$"8602 3B00 3EFE FFFB FF03 CF62 968C FA86"            /* †.;.>şÿûÿ.Ïb–Œú† */
	$"033C 003D 92FA 8604 6900 002F 96F9 8602"            /* .<.=’ú†.i../–ù†. */
	$"7800 6FF8 8602 0F2C 8EFA 8605 8000 0049"            /* x.oø†..,ú†.€..I */
	$"8E8B FA86 023C 00AA F9FF 02D3 A0A5 F994"            /* ‹ú†.<.ªùÿ.Ó ¥ù” */
	$"0263 0707 F7FF 02B5 7B9E F994 0539 075B"            /* .c..÷ÿ.µ{ù”.9.[ */
	$"9B94 94FC 9407 6407 50FF FFB8 9A96 FA94"            /* ›””ü”.d.Pÿÿ¸š–ú” */
	$"025C 07CC FEFF 0197 9BF9 9403 1607 6799"            /* .\.Ìşÿ.—›ù”...g™ */
	$"FA94 0449 07CC 499C F994 0107 6CFA FF01"            /* ú”.I.ÌIœù”..lúÿ. */
	$"9F98 FA94 038E 0707 7BF9 9402 4707 3AF6"            /* Ÿ˜ú”...{ù”.G.:ö */
	$"FF01 A69F F994 022A 1195 F994 0264 0707"            /* ÿ.¦Ÿù”.*.•ù”.d.. */
	$"0169 97FA 9402 5C07 CCF8 FF01 989C F994"            /* .i—ú”.\.Ìøÿ.˜œù” */
	$"0116 50FE FF01 A99C F994 035C 0752 9AFA"            /* ..Pşÿ.©œù”.\.Ršú */
	$"9403 8E07 0798 F994 0325 073B A2F9 9403"            /* ”...˜ù”.%.;¢ù”. */
	$"1607 7D99 FA94 0549 076C FF72 98F9 94FF"            /* ..}™ú”.I.lÿr˜ù”ÿ */
	$"0700 7AF9 9404 4707 3A46 9AFB 9402 9491"            /* ..zù”.G.:Fšû”.”‘ */
	$"88F7 9402 3007 7BF7 FF02 B9A2 A0F9 9402"            /* ˆ÷”.0.{÷ÿ.¹¢ ù”. */
	$"4707 46F8 FF03 D36F A198 FA94 0349 074A"            /* G.Føÿ.Óo¡˜ú”.I.J */
	$"9FFA 9404 7807 0739 A1F9 9402 8707 7CF8"            /* Ÿú”.x..9¡ù”.‡.|ø */
	$"9402 1B39 9AFA 9405 8E07 0753 9B98 FA94"            /* ”..9šú”...S›˜ú” */
	$"0249 07AC FBFF 00FF 02EF 03FF CE90 98F9"            /* .I.¬ûÿ.ÿ.ï.ÿÎ˜ù */
	$"8102 4A00 00F7 FF02 C08C 8FF9 8103 2900"            /* .J..÷ÿ.ÀŒù.). */
	$"4A8C FA81 0753 0048 FFFF AF8B 83FA 8102"            /* JŒú.S.Hÿÿ¯‹ƒú. */
	$"4C00 C8FE FF01 868B F981 0309 0059 87FA"            /* L.Èşÿ.†‹ù.Æ.Y‡ú */
	$"8104 3900 C83A 8DF9 8101 005F FAFF 018F"            /* .9.È:ù.._úÿ. */
	$"87FA 8103 7A00 006E FD81 FD81 0238 002E"            /* ‡ú.z..nıı.8.. */
	$"F6FF 019A 8FF9 8101 1D00 F881 0453 0000"            /* öÿ.šù...ø.S.. */
	$"5B85 FA81 024C 00C8 F8FF 0188 8DF9 8101"            /* […ú.L.Èøÿ.ˆù. */
	$"0948 FEFF 019B 8FF9 8103 5000 4683 FA81"            /* ÆHşÿ.›ù.P.Fƒú */
	$"037A 0000 87F9 8103 1600 2F94 F981 0409"            /* .z..‡ù.../”ù.Æ */
	$"006E 8881 FB81 0539 005F FF5E 87F9 81FF"            /* .nˆû.9._ÿ^‡ùÿ */
	$"0000 71F9 8104 3800 2E36 8BF0 8104 6117"            /* ..qù.8..6‹ğ.a. */
	$"0014 A4F7 FF02 AF94 90F9 8102 3800 48F8"            /* ..¤÷ÿ.¯”ù.8.Hø */
	$"FF03 D586 9487 FA81 0339 003B 8FFA 8104"            /* ÿ.Õ†”‡ú.9.;ú. */
	$"6500 002C 94F9 8101 6500 0065 F881 020C"            /* e..,”ù.e..eø.. */
	$"2A8A FA81 057A 0000 478C 86FA 8102 3900"            /* *Šú.z..GŒ†ú.9. */
	$"AAF9 FF02 CF92 9AF9 8302 4E00 00F7 FF02"            /* ªùÿ.Ï’šùƒ.N..÷ÿ. */
	$"C18E 91F9 8303 2C00 4D8E FA83 0757 004A"            /* Á‘ùƒ.,.Múƒ.W.J */
	$"FFFF B08D 86FA 8302 4E00 C9FE FF01 888E"            /* ÿÿ°†úƒ.N.Éşÿ.ˆ */
	$"F983 030B 005B 8BFE 83FD 8304 3B00 C83C"            /* ùƒ...[‹şƒıƒ.;.È< */
	$"90F9 8301 0062 FAFF 0191 8AFA 8303 7D00"            /* ùƒ..búÿ.‘Šúƒ.}. */
	$"0070 F983 023B 0030 F6FF 019B 91F9 8301"            /* .pùƒ.;.0öÿ.›‘ùƒ. */
	$"1F00 F883 0457 0000 5D88 FA83 024E 00C9"            /* ..øƒ.W..]ˆúƒ.N.É */
	$"F8FF 018B 8FF9 8301 0B4A FEFF 019C 91FC"            /* øÿ.‹ùƒ..Jşÿ.œ‘ü */
	$"83FE 8303 5200 4886 FA83 037D 0002 8BF9"            /* ƒşƒ.R.H†úƒ.}..‹ù */
	$"8303 1A00 3196 F983 030B 0070 8BFA 8305"            /* ƒ...1–ùƒ...p‹úƒ. */
	$"3B00 62FF 628A F983 FF00 0073 F983 043B"            /* ;.bÿbŠùƒÿ..sùƒ.; */
	$"0030 398D F083 0464 1A00 16A5 F7FF 02B2"            /* .09ğƒ.d...¥÷ÿ.² */
	$"9694 F983 023B 004A FEFF FBFF 03D6 8896"            /* –”ùƒ.;.Jşÿûÿ.Öˆ– */
	$"8AFA 8303 3B00 3D91 FA83 0467 0000 2F96"            /* Šúƒ.;.=‘úƒ.g../– */
	$"F983 0267 0067 F883 020F 2C8C FA83 057D"            /* ùƒ.g.gøƒ..,Œúƒ.} */
	$"0000 498E 8AFA 8302 3B00 AAF9 FF02 D29F"            /* ..IŠúƒ.;.ªùÿ.ÒŸ */
	$"A5F9 9002 5B07 07F7 FF02 C59B 9EF9 9005"            /* ¥ù.[..÷ÿ.Å›ù. */
	$"3907 5B9A 9090 FC90 0762 0750 FFFF B89A"            /* 9.[šü.b.Pÿÿ¸š */
	$"94FA 9002 5C07 CCFE FF01 969A F990 0316"            /* ”ú.\.Ìşÿ.–šù.. */
	$"0767 97FA 9004 4807 CC49 9CF9 9001 076C"            /* .g—ú.H.ÌIœù..l */
	$"FAFF 019F 97FA 9003 8C07 077A F990 0247"            /* úÿ.Ÿ—ú.Œ..zù.G */
	$"073A F6FF 01A6 9FF9 9002 2907 91F9 9002"            /* .:öÿ.¦Ÿù.).‘ù. */
	$"6207 0701 6995 FA90 025C 07CC F8FF 0198"            /* b...i•ú.\.Ìøÿ.˜ */
	$"9CF9 9001 1650 FEFF 01A8 9EF9 9003 5F07"            /* œù..Pşÿ.¨ù._. */
	$"5292 FA90 038C 070C 97F9 9003 2507 3BA1"            /* R’ú.Œ..—ù.%.;¡ */
	$"F990 0316 077D 97FA 9005 4807 6CFF 7297"            /* ù...}—ú.H.lÿr— */
	$"F990 FF07 007F F990 0447 073A 4699 FB90"            /* ùÿ...ù.G.:F™û */
	$"F690 0471 2607 20AC F7FF 02B8 A29F F990"            /* ö.q&. ¬÷ÿ.¸¢Ÿù */
	$"0247 0750 F8FF 03D8 97A1 96FA 9003 4807"            /* .G.Pøÿ.Ø—¡–ú.H. */
	$"499E FA90 0477 0707 39A1 F990 0277 0775"            /* Iú.w..9¡ù.w.u */
	$"F890 021B 3998 FA90 058C 0707 539B 96FA"            /* ø..9˜ú.Œ..S›–ú */
	$"9002 4807 ACFB FF00 FF03 1D03 FFCE 9098"            /* .H.¬ûÿ.ÿ...ÿÎ˜ */
	$"F97D 0243 0010 F7FF 02D4 8C8E F97D 0329"            /* ù}.C..÷ÿ.ÔŒù}.) */
	$"0045 8BFA 7D07 5100 48FF FFAF 8B81 FA7D"            /* .E‹ú}.Q.Hÿÿ¯‹ú} */
	$"024A 00C8 FEFF 0186 8BF9 7D03 0900 5885"            /* .J.Èşÿ.†‹ù}.Æ.X… */
	$"FA7D 0438 00C8 3A8D F97D 0A00 3584 95AA"            /* ú}.8.È:ù}Â.5„•ª */
	$"B0B2 B09B 8F86 FA7D 0378 0000 70FD 7DFD"            /* °²°›†ú}.x..pı}ı */
	$"7D02 3800 2EF6 FF01 9990 F97D 021C 0081"            /* }.8..öÿ.™ù}... */
	$"F97D 0451 0000 5B83 FA7D 0D4A 0071 8C9E"            /* ù}.Q..[ƒú}.J.qŒ */
	$"AFB0 B2A8 C5FF FF88 8DF9 7D01 0948 FEFF"            /* ¯°²¨Åÿÿˆù}.ÆHşÿ */
	$"019A 8FF9 7D02 5100 46F9 7D03 7800 0A86"            /* .šù}.Q.Fù}.x.Â† */
	$"F97D 0316 002F 92F9 7D04 0900 6E85 7DFB"            /* ù}.../’ù}.Æ.n…}û */
	$"7D05 3800 5FFF 5E86 F97D FF00 006E F97D"            /* }.8._ÿ^†ù}ÿ..nù} */
	$"0438 002E 368A F27D 056B 2500 0875 DBF6"            /* .8..6Šò}.k%..uÛö */
	$"FF02 AF94 90F9 7D02 2900 48F8 FF03 DC8B"            /* ÿ.¯”ù}.).Høÿ.Ü‹ */
	$"9485 FA7D 0338 003A 8EFA 7D04 6300 002C"            /* ”…ú}.8.:ú}.c.., */
	$"94F9 7D01 6300 005D F87D 0207 2A86 FA7D"            /* ”ù}.c..]ø}..*†ú} */
	$"0578 0000 478B 84FA 7D0D 3800 5F8C 9BAD"            /* .x..G‹„ú}.8._Œ›­ */
	$"B0B2 A9C3 FFCE 929A F981 0246 0013 F7FF"            /* °²©ÃÿÎ’šù.F..÷ÿ */
	$"02D4 8E90 F981 032B 0047 8DFA 8107 5400"            /* .Ôù.+.Gú.T. */
	$"4AFF FFB0 8D84 FA81 024E 00C9 FEFF 0188"            /* Jÿÿ°„ú.N.Éşÿ.ˆ */
	$"8DF9 8103 0B00 5A88 FE81 FD81 043B 00C8"            /* ù...Zˆşı.;.È */
	$"3C8F F981 0A00 3886 97AC B2B3 B29C 918A"            /* <ùÂ.8†—¬²³²œ‘Š */
	$"FA81 037B 0000 72F9 8102 3A00 30F6 FF01"            /* ú.{..rù.:.0öÿ. */
	$"9B92 F981 021F 0084 F981 0454 0000 5D86"            /* ›’ù...„ù.T..]† */
	$"FA81 0D4E 0073 8D9F B0B2 B3A9 C7FF FF8B"            /* ú.N.sŸ°²³©Çÿÿ‹ */
	$"8FF9 8101 0B4A FEFF 019C 91FC 81FE 8102"            /* ù..Jşÿ.œ‘üş. */
	$"5400 48F9 8103 7B00 0C8A F981 031A 0031"            /* T.Hù.{..Šù...1 */
	$"95F9 8103 0B00 7088 FA81 053B 0062 FF62"            /* •ù...pˆú.;.bÿb */
	$"8AF9 81FF 0000 70F9 8104 3A00 3039 8CF2"            /* Šùÿ..pù.:.09Œò */
	$"8105 6E28 000A 76DB F6FF 02B0 9692 F981"            /* .n(.ÂvÛöÿ.°–’ù */
	$"022B 004A FEFF FBFF 03DC 8D96 87FA 8103"            /* .+.Jşÿûÿ.Ü–‡ú. */
	$"3B00 3C91 FA81 0466 0000 2F96 F981 0266"            /* ;.<‘ú.f../–ù.f */
	$"005F F881 0209 2C8A FA81 057B 0000 498D"            /* ._ø.Æ,Šú.{..I */
	$"87FA 810D 3B00 628E 9EAF B2B3 AAC4 FFD2"            /* ‡ú.;.b¯²³ªÄÿÒ */
	$"9FA5 F98E 0252 0720 F7FF 02D6 9B9C F98E"            /* Ÿ¥ù.R. ÷ÿ.Ö›œù */
	$"0538 0755 998E 8EFC 8E07 6107 50FF FFB8"            /* .8.U™ü.a.Pÿÿ¸ */
	$"9A91 FA8E 025B 07CC FEFF 0196 99F9 8E03"            /* š‘ú.[.Ìşÿ.–™ù. */
	$"1607 6695 FA8E 0448 07CC 499B F98E 0A07"            /* ..f•ú.H.ÌI›ùÂ. */
	$"438E A1B4 B9BA B9A4 9E97 FA8E 038A 0707"            /* C¡´¹º¹¤—ú.Š.. */
	$"7DF9 8E02 4607 3AF6 FF01 A59F F98E 0229"            /* }ù.F.:öÿ.¥Ÿù.) */
	$"0792 F98E 0261 0707 0169 94FA 8E0D 5B07"            /* .’ù.a...i”ú.[. */
	$"7C97 A9B8 B9BA B0C9 FFFF 989B F98E 0116"            /* |—©¸¹º°Éÿÿ˜›ù.. */
	$"50FE FF01 A69E F98E 0261 0752 F98E 038A"            /* Pşÿ.¦ù.a.Rù.Š */
	$"0719 96F9 8E03 2507 3BA1 F98E 0316 077D"            /* ..–ù.%.;¡ù...} */
	$"96FA 8E05 4807 6CFF 7197 F98E FF07 007B"            /* –ú.H.lÿq—ùÿ..{ */
	$"F98E 0446 073A 4699 FB8E F88E 057B 3307"            /* ù.F.:F™ûø.{3. */
	$"147C DCF6 FF02 B8A1 9EF9 8E02 3807 50F8"            /* .|Üöÿ.¸¡ù.8.Pø */
	$"FF03 DD9A A195 FA8E 0348 0749 9CFA 8E04"            /* ÿ.İš¡•ú.H.Iœú. */
	$"7507 0739 A1F9 8E02 7507 6BF8 8E02 1439"            /* u..9¡ù.u.kø..9 */
	$"96FA 8E05 8A07 0753 9A95 FA8E 0848 0769"            /* –ú.Š..Sš•ú.H.i */
	$"98A8 B7B9 BAB2 00C7 0316 03FF CD8E 98F9"            /* ˜¨·¹º².Ç...ÿÍ˜ù */
	$"7902 3B00 12F7 FF02 D88B 8DF9 7903 2800"            /* y.;..÷ÿ.Ø‹ùy.(. */
	$"3988 FA79 074F 0048 FFFF AE8B 7DFA 7902"            /* 9ˆúy.O.Hÿÿ®‹}úy. */
	$"4900 C8FE FF01 848A F979 034F 6378 80FA"            /* I.Èşÿ.„Šùy.Ocx€ú */
	$"7904 3600 C83A 8CF9 7902 4A63 76FD 77FF"            /* y.6.È:Œùy.Jcvıwÿ */
	$"1201 7D85 FA79 0375 0000 6DFD 79FD 7902"            /* ..}…úy.u..mıyıy. */
	$"3600 2EF6 FF01 9990 F979 021B 0082 F979"            /* 6..öÿ.™ùy...‚ùy */
	$"044F 0000 5B80 FA79 0265 536E FD77 065C"            /* .O..[€úy.eSnıw.\ */
	$"0096 FFFF 878C F979 0109 48FE FF01 958E"            /* .–ÿÿ‡Œùy.ÆHşÿ.• */
	$"F979 024F 0046 F979 0375 000A 85F9 7903"            /* ùy.O.Fùy.u.Â…ùy. */
	$"1600 2F91 F979 0409 006E 8379 FB79 0536"            /* ../‘ùy.Æ.nƒyûy.6 */
	$"005F FF5D 85F9 79FF 0000 6BF9 7904 3600"            /* ._ÿ]…ùyÿ..kùy.6. */
	$"2E36 88F4 7905 722C 0000 5BD4 F4FF 02AE"            /* .6ˆôy.r,..[Ôôÿ.® */
	$"918F F979 0228 0048 F8FF 03DE 8A94 82FA"            /* ‘ùy.(.Høÿ.ŞŠ”‚ú */
	$"7903 3600 368D FA79 045F 000B 2C94 F979"            /* y.6.6úy._..,”ùy */
	$"015F 0000 5DF8 7902 002A 82FA 7905 7500"            /* ._..]øy..*‚úy.u. */
	$"004C 8B82 FA79 025C 536D FD77 0666 0596"            /* .L‹‚úy.\Smıw.f.– */
	$"FFCE 909A F97C 023D 0015 F7FF 02D8 8D8F"            /* ÿÎšù|.=..÷ÿ.Ø */
	$"F97C 032A 003B 8BFA 7C07 5200 4AFF FFAF"            /* ù|.*.;‹ú|.R.Jÿÿ¯ */
	$"8D81 FA7C 024C 00C9 FEFF 0187 8CF9 7C03"            /* ú|.L.Éşÿ.‡Œù|. */
	$"5266 7B82 FE7C FD7C 0439 00C8 3C8E F97C"            /* Rf{‚ş|ı|.9.È<ù| */
	$"024D 6779 FD7A 0314 1580 87FA 7C03 7800"            /* .Mgyız...€‡ú|.x. */
	$"006F F97C 0239 0030 F6FF 019A 92F9 7C02"            /* .où|.9.0öÿ.š’ù|. */
	$"1E00 85F9 7C04 5200 005D 83FA 7C02 6857"            /* ..…ù|.R..]ƒú|.hW */
	$"72FD 7A06 5F02 97FF FF8A 8EF9 7C01 0B4A"            /* rız._.—ÿÿŠù|..J */
	$"FEFF 0197 90FC 7CFE 7C02 5200 48F9 7C03"            /* şÿ.—ü|ş|.R.Hù|. */
	$"7800 0C88 F97C 031A 0031 94F9 7C03 0B00"            /* x..ˆù|...1”ù|... */
	$"7085 FA7C 0539 0062 FF61 87F9 7CFF 0000"            /* p…ú|.9.bÿa‡ù|ÿ.. */
	$"6DF9 7C04 3900 3039 8BF4 7C05 7530 0000"            /* mù|.9.09‹ô|.u0.. */
	$"5DD4 F4FF 02AF 9491 F97C 022A 004A FEFF"            /* ]Ôôÿ.¯”‘ù|.*.Jşÿ */
	$"FBFF 03DE 8C96 85FA 7C03 3900 398F FA7C"            /* ûÿ.ŞŒ–…ú|.9.9ú| */
	$"0463 000D 2F96 F97C 0263 005F F87C 0200"            /* .c../–ù|.c._ø|.. */
	$"2C84 FA7C 0578 0000 4E8D 85FA 7C02 5F57"            /* ,„ú|.x..N…ú|._W */
	$"70FD 7A06 6907 97FF D19C A4F9 8B02 4807"            /* pız.i.—ÿÑœ¤ù‹.H. */
	$"22F7 FF02 D99A 9BF9 8B05 3607 4897 8B8B"            /* "÷ÿ.Ùš›ù‹.6.H—‹‹ */
	$"FC8B 075E 0750 FFFF B79A 8FFA 8B02 5A07"            /* ü‹.^.Pÿÿ·šú‹.Z. */
	$"CCFE FF01 9598 F98B 0361 758A 8FFA 8B04"            /* Ìşÿ.•˜ù‹.auŠú‹. */
	$"4607 CC49 9AF9 8B02 5A76 88FD 8A03 2022"            /* F.ÌIšù‹.ZvˆıŠ. " */
	$"8E95 FA8B 0387 0707 7AF9 8B02 4507 3AF6"            /* •ú‹.‡..zù‹.E.:ö */
	$"FF01 A59F F98B 0228 0792 F98B 025E 0707"            /* ÿ.¥Ÿù‹.(.’ù‹.^.. */
	$"0169 90FA 8B03 7764 8288 FE8A 066E 0C99"            /* .iú‹.wd‚ˆşŠ.n.™ */
	$"FFFF 979A F98B 0116 50FE FF01 A29C F98B"            /* ÿÿ—šù‹..Pşÿ.¢œù‹ */
	$"025E 0752 F98B 0387 0719 95F9 8B03 2507"            /* .^.Rù‹.‡..•ù‹.%. */
	$"3BA0 F98B 0316 077D 92FA 8B05 4607 6CFF"            /* ; ù‹...}’ú‹.F.lÿ */
	$"7195 F98B FF07 0078 F98B 0445 073A 4697"            /* q•ù‹ÿ..xù‹.E.:F— */
	$"FB8B FA8B 0583 3D07 0964 D6F4 FF02 B79F"            /* û‹ú‹.ƒ=.ÆdÖôÿ.·Ÿ */
	$"9EF9 8B02 3607 50F8 FF03 DF99 A191 FA8B"            /* ù‹.6.Pøÿ.ß™¡‘ú‹ */
	$"0346 0746 9BFA 8B04 7207 1939 A1F9 8B02"            /* .F.F›ú‹.r..9¡ù‹. */
	$"7207 6BF8 8B02 0739 91FA 8B05 8707 0757"            /* r.kø‹..9‘ú‹.‡..W */
	$"9A91 FA8B 036E 6380 88FE 8A01 7A11 0099"            /* š‘ú‹.nc€ˆşŠ.z..™ */
	$"02C2 03FF CD8D 98F9 7702 3800 12F7 FF02"            /* .Â.ÿÍ˜ùw.8..÷ÿ. */
	$"D88A 8CF9 7703 2700 3886 FA77 074D 0052"            /* ØŠŒùw.'.8†úw.M.R */
	$"FFFF AE8B 7CFA 7702 4800 C8FE FF01 8488"            /* ÿÿ®‹|úw.H.Èşÿ.„ˆ */
	$"EE77 0435 00C8 3A8B F277 0300 125F 84FA"            /* îw.5.È:‹òw..._„ú */
	$"7703 7200 3884 FD77 FD77 0235 002E F6FF"            /* w.r.8„ıwıw.5..öÿ */
	$"0198 90F9 7702 1B00 83F9 7704 4D00 005B"            /* .˜ùw...ƒùw.M..[ */
	$"7FF3 7706 5300 96FF FF86 8BF9 7701 0948"            /* .ów.S.–ÿÿ†‹ùw.ÆH */
	$"FEFF 018C 8EF9 7702 4D00 46F9 7703 7600"            /* şÿ.Œùw.M.Fùw.v. */
	$"0A84 F977 0316 002F 90F9 7704 0900 6E81"            /* Â„ùw.../ùw.Æ.n */
	$"77FB 7705 3500 5FFF 5D84 F977 0200 2E81"            /* wûw.5._ÿ]„ùw... */
	$"F977 0435 002E 3687 F577 0468 1000 16A1"            /* ùw.5..6‡õw.h...¡ */
	$"F2FF 02AD 908F F977 021B 0048 F7FF 0288"            /* òÿ.­ùw...H÷ÿ.ˆ */
	$"9480 FA77 0335 0029 8BFA 7704 5E00 0D2C"            /* ”€úw.5.)‹úw.^.., */
	$"94F9 7701 5E00 005C F877 0200 2A80 FA77"            /* ”ùw.^..\øw..*€úw */
	$"0572 0000 5B8B 80F3 7706 5E00 96FF CD8F"            /* .r..[‹€ów.^.–ÿÍ */
	$"9AF9 7A02 3B00 15F7 FF02 D88C 8FF9 7A03"            /* šùz.;..÷ÿ.ØŒùz. */
	$"2A00 3A8A FA7A 0750 0054 FFFF AF8D 7FFA"            /* *.:Šúz.P.Tÿÿ¯.ú */
	$"7A02 4A00 C9FE FF01 868B F27A FD7A 0438"            /* z.J.Éşÿ.†‹òzız.8 */
	$"00C8 3C8D F27A 0300 1563 86FA 7A03 7601"            /* .È<òz...c†úz.v. */
	$"3A86 F97A 0238 0030 F6FF 019A 92F9 7A02"            /* :†ùz.8.0öÿ.š’ùz. */
	$"1D00 85F9 7A04 5000 005D 81F3 7A06 5700"            /* ..…ùz.P..]óz.W. */
	$"97FF FF88 8DF9 7A01 0B4A FEFF 018E 90FC"            /* —ÿÿˆùz..Jşÿ.ü */
	$"7AFE 7A02 5000 48F9 7A03 7900 0C87 F97A"            /* zşz.P.Hùz.y..‡ùz */
	$"031A 0031 94F9 7A03 0B00 7084 FA7A 0538"            /* ...1”ùz...p„úz.8 */
	$"0062 FF61 86F9 7A02 0131 83F9 7A04 3800"            /* .bÿa†ùz..1ƒùz.8. */
	$"3039 8AF5 7A04 6B13 0019 A2F2 FF02 AE92"            /* 09Šõz.k...¢òÿ.®’ */
	$"91F9 7A02 1D00 4AFE FFFA FF02 8B96 83FA"            /* ‘ùz...Jşÿúÿ.‹–ƒú */
	$"7A03 3800 2B8D FA7A 0462 0010 2F96 F97A"            /* z.8.+úz.b../–ùz */
	$"0262 005E F87A 0200 2C83 FA7A 0576 0000"            /* .b.^øz..,ƒúz.v.. */
	$"5D8D 83F3 7A06 6200 97FF D19B A4F9 8802"            /* ]ƒóz.b.—ÿÑ›¤ùˆ. */
	$"4707 22F7 FF02 D999 9BF9 8805 3607 4796"            /* G."÷ÿ.Ù™›ùˆ.6.G– */
	$"8888 FC88 075C 075C FFFF B79A 8DFA 8802"            /* ˆˆüˆ.\.\ÿÿ·šúˆ. */
	$"5907 CCFE FF01 9598 EE88 0446 07CC 499A"            /* Y.Ìşÿ.•˜îˆ.F.ÌIš */
	$"F288 0307 2273 95FA 8803 850A 4692 F988"            /* òˆ.."s•úˆ.…ÂF’ùˆ */
	$"0245 073A F6FF 01A5 9FF9 8802 2707 94F9"            /* .E.:öÿ.¥Ÿùˆ.'.”ù */
	$"8802 5C07 0701 698F F388 0665 0799 FFFF"            /* ˆ.\...ióˆ.e.™ÿÿ */
	$"979A F988 0116 50FE FF01 9B9C F988 025C"            /* —šùˆ..Pşÿ.›œùˆ.\ */
	$"0752 F888 0207 1994 F988 0325 073B 9FF9"            /* .Røˆ...”ùˆ.%.;Ÿù */
	$"8803 1607 7D91 FA88 0546 076C FF71 95F9"            /* ˆ...}‘úˆ.F.lÿq•ù */
	$"8802 0A3D 8FF9 8804 4507 3A46 97FB 88FB"            /* ˆ.Â=ùˆ.E.:F—ûˆû */
	$"8804 791D 0722 A8F2 FF02 B69F 9EF9 8802"            /* ˆ.y.."¨òÿ.¶Ÿùˆ. */
	$"2707 50F7 FF02 98A1 90FA 8803 4607 3699"            /* '.P÷ÿ.˜¡úˆ.F.6™ */
	$"FA88 0471 071C 39A1 F988 0271 0769 F888"            /* úˆ.q..9¡ùˆ.q.iøˆ */
	$"0207 3990 FA88 0585 0707 649A 90F3 8801"            /* ..9úˆ.…..dšóˆ. */
	$"7107 0099 02E7 03FF CC8C 98F9 7307 3D28"            /* q..™.ç.ÿÌŒ˜ùs.=( */
	$"394A 5C6B 7373 FE75 046E 328C 8A8C F973"            /* 9J\kssşu.n2ŒŠŒùs */
	$"033D 2C5B 82FA 7307 4C00 5FFF FFAD 8A7A"            /* .=,[‚ús.L._ÿÿ­Šz */
	$"FA73 0247 00C8 FEFF 0183 87EE 7304 3400"            /* ús.G.Èşÿ.ƒ‡îs.4. */
	$"C839 8AF2 7303 0012 4083 FA73 0272 6F77"            /* È9Šòs...@ƒús.row */
	$"FC73 FE73 0372 1600 49F6 FF01 9890 F973"            /* üsşs.r..Iöÿ.˜ùs */
	$"021A 0082 F973 044C 0000 5B7C F373 0652"            /* ...‚ùs.L..[|ós.R */
	$"0096 FFFF 868A F973 0109 48FE FF01 8C8D"            /* .–ÿÿ†Šùs.ÆHşÿ.Œ */
	$"F973 024C 0046 F873 0200 0A83 F973 0316"            /* ùs.L.Føs..Âƒùs.. */
	$"002F 90F9 7304 0900 6E7F 73FB 7305 3400"            /* ./ùs.Æ.n.sûs.4. */
	$"5FFF 5D83 F973 016F 77F9 7305 751C 0043"            /* _ÿ]ƒùs.owùs.u..C */
	$"3686 F573 045F 1F09 4AB5 F2FF 02AC 908E"            /* 6†õs._.ÆJµòÿ.¬ */
	$"F973 0728 2E3D 4F5F 6F73 73FE 7504 6524"            /* ùs.(.=O_ossşu.e$ */
	$"6D92 7DFA 7303 4428 5286 FA73 045C 001D"            /* m’}ús.D(R†ús.\.. */
	$"2C94 F973 0155 0000 5AF8 7302 002A 7DFA"            /* ,”ùs.U..Zøs..*}ú */
	$"7305 7000 005B 8B7F F373 065C 0096 FFCD"            /* s.p..[‹.ós.\.–ÿÍ */
	$"8E9A F977 0740 2A3C 4D5E 6E77 77FE 7804"            /* šùw.@*<M^nwwşx. */
	$"7134 8D8C 8EF9 7703 402F 5D85 FA77 074E"            /* q4Œùw.@/]…úw.N */
	$"0062 FFFF AF8C 7CFA 7702 4A00 C9FE FF01"            /* .bÿÿ¯Œ|úw.J.Éşÿ. */
	$"868A F277 FD77 0438 00C8 3C8D F277 0300"            /* †Šòwıw.8.È<òw.. */
	$"1544 86FA 7702 7672 7AF9 7703 7619 004C"            /* .D†úw.vrzùw.v..L */
	$"F6FF 019A 92F9 7702 1C00 85F9 7704 4E00"            /* öÿ.š’ùw...…ùw.N. */
	$"005D 7FF3 7706 5500 97FF FF88 8DF9 7701"            /* .].ów.U.—ÿÿˆùw. */
	$"0B4A FEFF 018E 8FFC 77FE 7702 4E00 48F8"            /* .Jşÿ.üwşw.N.Hø */
	$"7702 000C 86F9 7703 1A00 3192 F977 030B"            /* w...†ùw...1’ùw.. */
	$"0070 82FA 7705 3800 62FF 6186 F977 0172"            /* .p‚úw.8.bÿa†ùw.r */
	$"7AF9 7705 781E 0045 3988 F577 0463 220B"            /* zùw.x..E9ˆõw.c". */
	$"4CB6 F2FF 02AE 9290 F977 052B 303F 5162"            /* L¶òÿ.®’ùw.+0?Qb */
	$"72FF 77FE 7804 6827 7095 81FA 7703 472B"            /* rÿwşx.h'p•úw.G+ */
	$"5488 FA77 045F 001F 2F96 F977 0259 005C"            /* Tˆúw._../–ùw.Y.\ */
	$"F877 0200 2C81 FA77 0573 0000 5D8D 81F3"            /* øw..,úw.s..]ó */
	$"7706 5F00 97FF D19B A5F9 8605 4E35 485B"            /* w._.—ÿÑ›¥ù†.N5H[ */
	$"6E7F FE87 FF88 0483 4392 999A F986 054E"            /* n.ş‡ÿˆ.ƒC’™šù†.N */
	$"3B6C 9286 86FC 8607 5B07 6CFF FFB7 998C"            /* ;l’††ü†.[.lÿÿ·™Œ */
	$"FA86 0259 07CC FEFF 0194 97EE 8604 4507"            /* ú†.Y.Ìşÿ.”—î†.E. */
	$"CC48 99F2 8603 0722 5294 FA86 0285 8188"            /* ÌH™ò†.."R”ú†.…ˆ */
	$"F986 0385 2407 54F6 FF01 A59F F986 0227"            /* ù†.…$.Töÿ.¥Ÿù†.' */
	$"0792 F986 025B 0707 0169 8DF3 8606 6407"            /* .’ù†.[...ió†.d. */
	$"99FF FF96 99F9 8601 1650 FEFF 019B 9CF9"            /* ™ÿÿ–™ù†..Pşÿ.›œù */
	$"8602 5B07 52F8 8602 0719 94F9 8603 2507"            /* †.[.Rø†...”ù†.%. */
	$"3B9F F986 0316 077D 8FFA 8605 4507 6CFF"            /* ;Ÿù†...}ú†.E.lÿ */
	$"7194 F986 0181 88F8 8604 2A07 4E46 96FB"            /* q”ù†.ˆø†.*.NF–û */
	$"86FB 8604 712F 1653 BAF2 FF02 B69F 9CF9"            /* †û†.q/.Sºòÿ.¶Ÿœù */
	$"8605 363C 4C5F 7183 FE87 FF88 0479 347D"            /* †.6<L_qƒş‡ÿˆ.y4} */
	$"A18E FA86 0355 3562 96FA 8604 6F07 2839"            /* ¡ú†.U5b–ú†.o.(9 */
	$"A1F9 8602 6707 68F8 8602 0739 8EFA 8605"            /* ¡ù†.g.hø†..9ú†. */
	$"8407 0764 9A8F F386 016F 0700 9902 AA03"            /* „..dšó†.o..™.ª. */
	$"FFCC 8898 F970 0054 F770 045A 007A 718B"            /* ÿÌˆ˜ùp.T÷p.Z.zq‹ */
	$"EE70 0748 005F FFFF AD8A 77FA 7002 4600"            /* îp.H._ÿÿ­Šwúp.F. */
	$"C8FE FF01 8286 F970 032B 2224 75FA 7004"            /* Èşÿ.‚†ùp.+"$uúp. */
	$"3300 C839 88F9 70FA 2203 0012 3182 F270"            /* 3.È9ˆùpú"...1‚òp */
	$"FE70 0338 0006 D6F6 FF01 9790 F970 0219"            /* şp.8..Ööÿ.—ùp.. */
	$"0081 F970 0448 0000 5B79 FA70 0054 FB22"            /* .ùp.H..[yúp.Tû" */
	$"0617 0096 FFFF 8588 F970 0109 48FE FF01"            /* ...–ÿÿ…ˆùp.ÆHşÿ. */
	$"8A8C F970 0250 0047 F870 0200 0A81 F970"            /* ŠŒùp.P.Gøp..Âùp */
	$"0316 002F 8FF9 7004 0900 6E7B 70FB 7005"            /* .../ùp.Æ.n{pûp. */
	$"3300 5FFF 5D82 EF70 0548 0000 CD36 85F4"            /* 3._ÿ]‚ïp.H..Í6…ô */
	$"7005 6F4D 1F0F 69D9 F4FF 02AB 8E8D F970"            /* p.oM..iÙôÿ.«ùp */
	$"004D F770 0446 003D 927B EF70 045A 0021"            /* .M÷p.F.=’{ïp.Z.! */
	$"2C94 F970 0148 0000 5AF8 7002 002A 7BFA"            /* ,”ùp.H..Zøp..*{ú */
	$"7005 6E00 005B 8A7B FA70 0047 FB22 061B"            /* p.n..[Š{úp.Gû".. */
	$"0096 FFCD 8B9A F973 0058 F773 045D 007B"            /* .–ÿÍ‹šùs.X÷s.].{ */
	$"738D EE73 074C 0062 FFFF AE8C 7AFA 7302"            /* sîs.L.bÿÿ®Œzús. */
	$"4800 C9FE FF01 8588 F973 032E 2527 78FE"            /* H.Éşÿ.…ˆùs..%'xş */
	$"73FD 7304 3500 C83B 8CF9 73FA 2503 0015"            /* sıs.5.È;Œùsú%... */
	$"3384 EF73 033A 0008 D7F6 FF01 9992 F973"            /* 3„ïs.:..×öÿ.™’ùs */
	$"021B 0083 F973 044C 0000 5D7C FA73 0057"            /* ...ƒùs.L..]|ús.W */
	$"FB25 061B 0097 FFFF 878C F973 010B 4AFE"            /* û%...—ÿÿ‡Œùs..Jş */
	$"FF01 8C8E FC73 FE73 0253 0049 F873 0200"            /* ÿ.Œüsşs.S.Iøs.. */
	$"0C84 F973 031A 0031 91F9 7303 0B00 707F"            /* .„ùs...1‘ùs...p. */
	$"FA73 0535 0062 FF61 84EF 7305 4A00 00CE"            /* ús.5.bÿa„ïs.J..Î */
	$"3987 F473 0572 5022 116B D9F4 FF02 AC90"            /* 9‡ôs.rP".kÙôÿ.¬ */
	$"8FF9 7300 4FFC 73FC 7304 4800 4095 7DEF"            /* ùs.Oüsüs.H.@•}ï */
	$"7304 5D00 242F 96F9 7302 4C00 5CF8 7302"            /* s.].$/–ùs.L.\øs. */
	$"002C 7FFA 7305 7100 005D 8C7F FA73 004A"            /* .,.ús.q..]Œ.ús.J */
	$"FB25 061E 0097 FFD1 98A5 F982 0066 F782"            /* û%...—ÿÑ˜¥ù‚.f÷‚ */
	$"046D 0783 8299 F382 FC82 0759 076C FFFF"            /* .m.ƒ‚™ó‚ü‚.Y.lÿÿ */
	$"B699 88FA 8202 5707 CCFE FF01 9296 F982"            /* ¶™ˆú‚.W.Ìşÿ.’–ù‚ */
	$"033A 3032 86FA 8204 4407 CC48 98F9 82FA"            /* .:02†ú‚.D.ÌH˜ù‚ú */
	$"3003 0722 4092 EF82 0347 0713 D8F6 FF01"            /* 0.."@’ï‚.G..Øöÿ. */
	$"A49F F982 0226 0791 F982 0259 0707 0169"            /* ¤Ÿù‚.&.‘ù‚.Y...i */
	$"8AFA 8200 65FB 3006 2507 99FF FF95 98F9"            /* Šú‚.eû0.%.™ÿÿ•˜ù */
	$"8201 1650 FEFF 0199 9AF9 8202 6207 53F8"            /* ‚..Pşÿ.™šù‚.b.Sø */
	$"8202 0719 91F9 8203 2507 3B9E F982 0316"            /* ‚...‘ù‚.%.;ù‚.. */
	$"077D 8CFA 8205 4407 6CFF 7092 EF82 0559"            /* .}Œú‚.D.lÿp’ï‚.Y */
	$"0709 D146 95FB 82F9 8204 6131 1D72 DCF4"            /* .ÆÑF•û‚ù‚.a1.rÜô */
	$"FF02 B59C 9BF9 8200 5EF7 8204 5707 4EA1"            /* ÿ.µœ›ù‚.^÷‚.W.N¡ */
	$"8CEF 8204 6D07 2C39 A1F9 8202 5907 68F8"            /* Œï‚.m.,9¡ù‚.Y.hø */
	$"8202 0739 8CFA 8205 8107 0764 998C FA82"            /* ‚..9Œú‚...d™Œú‚ */
	$"0059 FB30 0129 0700 9902 AF03 FFCC 8798"            /* .Yû0.)..™.¯.ÿÌ‡˜ */
	$"F96D 004D F76D 0458 0080 4C8A EE6D 0746"            /* ùm.M÷m.X.€LŠîm.F */
	$"005F FFFF AC8A 75FA 6D02 4500 C8FE FF01"            /* ._ÿÿ¬Šuúm.E.Èşÿ. */
	$"8185 F96D 0309 0025 79FA 6D04 3200 C839"            /* …ùm.Æ.%yúm.2.È9 */
	$"88F9 6D01 0032 FB7A 0285 3181 F26D FE6D"            /* ˆùm..2ûz.…1òmşm */
	$"0365 0500 71F6 FF01 968F F96D 0217 0075"            /* .e..qöÿ.–ùm...u */
	$"F96D 0446 0000 5B77 FA6D 0245 006B FB7A"            /* ùm.F..[wúm.E.kûz */
	$"04C0 FFFF 7688 F96D 0109 48FE FF01 888B"            /* .Àÿÿvˆùm.ÆHşÿ.ˆ‹ */
	$"F96D 0258 004A F86D 0200 0A80 F96D 0316"            /* ùm.X.Jøm..Â€ùm.. */
	$"002F 8EF9 6D04 0900 6E79 6DFB 6D05 3200"            /* ./ùm.Æ.nymûm.2. */
	$"5FFF 5D81 EF6D 0565 0900 6135 84F2 6D04"            /* _ÿ]ïm.eÆ.a5„òm. */
	$"6239 252F B7F5 FF02 A98C 8CF9 6D00 52F7"            /* b9%/·õÿ.©ŒŒùm.R÷ */
	$"6D04 4500 1992 78EF 6D04 5800 212C 94F9"            /* m.E..’xïm.X.!,”ù */
	$"6D01 4600 005A F96D 0370 0022 79FA 6D05"            /* m.F..Zùm.p."yúm. */
	$"6C00 005B 8A79 FA6D 0232 005B FB7A 04C0"            /* l..[Šyúm.2.[ûz.À */
	$"FFCC 8A9A F970 0050 F770 045B 0081 4E8D"            /* ÿÌŠšùp.P÷p.[.N */
	$"EE70 0749 0062 FFFF AE8C 78FA 7002 4800"            /* îp.I.bÿÿ®Œxúp.H. */
	$"C9FE FF01 8488 F970 030B 0027 7CFE 70FD"            /* Éşÿ.„ˆùp...'|şpı */
	$"7004 3500 C83B 8BF9 7001 0034 FB7C 0286"            /* p.5.È;‹ùp..4û|.† */
	$"3383 EF70 0368 0700 72F6 FF01 9891 F970"            /* 3ƒïp.h..röÿ.˜‘ùp */
	$"021B 0078 F970 0449 0000 5D7A FA70 0248"            /* ...xùp.I..]zúp.H */
	$"006D FB7C 04C1 FFFF 788B F970 010B 4AFE"            /* .mû|.Áÿÿx‹ùp..Jş */
	$"FF01 8B8D FC70 FE70 025B 004D F870 0200"            /* ÿ.‹üpşp.[.Møp.. */
	$"0C83 F970 031A 0031 91F9 7003 0B00 707C"            /* .ƒùp...1‘ùp...p| */
	$"FA70 0535 0062 FF61 84EF 7005 690B 0062"            /* úp.5.bÿa„ïp.i..b */
	$"3986 F270 0465 3C28 32B8 F5FF 02AB 8E8F"            /* 9†òp.e<(2¸õÿ.« */
	$"F970 0055 FC70 FC70 0448 001B 957B EF70"            /* ùp.Uüpüp.H..•{ïp */
	$"045B 0024 2F96 F970 0249 005C F970 0373"            /* .[.$/–ùp.I.\ùp.s */
	$"0025 7CFA 7005 6F00 005D 8C7C FA70 0235"            /* .%|úp.o..]Œ|úp.5 */
	$"005C FB7C 04C1 FFCF 97A5 F980 005F F780"            /* .\û|.ÁÿÏ—¥ù€._÷€ */
	$"046C 0787 5B99 F380 FC80 0757 076C FFFF"            /* .l.‡[™ó€ü€.W.lÿÿ */
	$"B699 86FA 8002 5707 CCFE FF01 9296 F980"            /* ¶™†ú€.W.Ìşÿ.’–ù€ */
	$"0316 0733 8BFA 8004 4307 CC48 98F9 8001"            /* ...3‹ú€.C.ÌH˜ù€. */
	$"073F FB83 028E 4091 EF80 0378 1107 7AF6"            /* .?ûƒ.@‘ï€.x..zö */
	$"FF01 A39F F980 0225 0787 F980 0257 0707"            /* ÿ.£Ÿù€.%.‡ù€.W.. */
	$"0169 88FA 8002 5707 76FB 8304 C2FF FF88"            /* .iˆú€.W.vûƒ.Âÿÿˆ */
	$"98F9 8001 1650 FEFF 0198 9AF9 8002 6C07"            /* ˜ù€..Pşÿ.˜šù€.l. */
	$"58F8 8002 0719 90F9 8003 2507 3B9C F980"            /* Xø€...ù€.%.;œù€ */
	$"0316 077D 8BFA 8005 4307 6CFF 7091 EF80"            /* ...}‹ú€.C.lÿp‘ï€ */
	$"0579 1507 6B46 94FB 80F8 8004 764E 383F"            /* .y..kF”û€ø€.vN8? */
	$"BEF5 FF02 B39B 9BF9 8000 66F7 8004 5707"            /* ¾õÿ.³››ù€.f÷€.W. */
	$"27A1 8AEF 8004 6C07 2C39 A1F9 8002 5707"            /* '¡Šï€.l.,9¡ù€.W. */
	$"68F9 8003 8307 308A F980 FF07 0264 998B"            /* hù€.ƒ.0Šù€ÿ..d™‹ */
	$"FA80 0243 0764 FB83 00C2 02FE 03FF CB85"            /* ú€.C.dûƒ.Â.ş.ÿË… */
	$"98F9 6B03 484F 4F62 FA6B 0455 0096 228A"            /* ˜ùk.HOObúk.U.–"Š */
	$"F96B 0359 4F4F 67FA 6B07 4500 5FFF FFAC"            /* ùk.YOOgúk.E._ÿÿ¬ */
	$"8A72 FA6B 0244 00C8 FEFF 0181 85F9 6B03"            /* Šrúk.D.Èşÿ.…ùk. */
	$"0900 5777 FA6B 0431 00C8 3987 F96B 0100"            /* Æ.Wwúk.1.È9‡ùk.. */
	$"5FFA FF01 3480 FA6B 036D 1917 63FD 6BFD"            /* _úÿ.4€úk.m..cıkı */
	$"6B02 3000 3BF6 FF01 968F F96B 0217 0072"            /* k.0.;öÿ.–ùk...r */
	$"F96B 0445 0000 5B75 FA6B 0244 00C8 F8FF"            /* ùk.E..[uúk.D.Èøÿ */
	$"0173 87F9 6B01 0948 FEFF 017F 8AF9 6B02"            /* .s‡ùk.ÆHşÿ..Šùk. */
	$"5500 52F8 6B02 000A 80F9 6B03 1600 2F8E"            /* U.Røk..Â€ùk.../ */
	$"F96B 0409 006E 786B FB6B 0531 005F FF5C"            /* ùk.Æ.nxkûk.1._ÿ\ */
	$"81F9 6B02 1617 59F9 6B04 3100 2E44 83F1"            /* ùk...Yùk.1..Dƒñ */
	$"6B05 6C54 3A33 72C5 F7FF 02A8 8C8C F96B"            /* k.lT:3rÅ÷ÿ.¨ŒŒùk */
	$"034A 4F4F 67FA 6B04 4400 0092 77FA 6B03"            /* .JOOgúk.D..’wúk. */
	$"5C4F 4F63 FA6B 0455 002C 2C94 F96B 0145"            /* \OOcúk.U.,,”ùk.E */
	$"0000 5AF9 6B03 6800 1F75 FA6B 0569 0000"            /* ..Zùk.h..uúk.i.. */
	$"5E8A 78FA 6B02 3100 AAF9 FF02 CC88 9AF9"            /* ^Šxúk.1.ªùÿ.Ìˆšù */
	$"6E03 4A52 5265 FA6E 0459 0097 258C F96E"            /* n.JRReún.Y.—%Œùn */
	$"035C 5252 69FA 6E07 4800 62FF FFAD 8C76"            /* .\RRiún.H.bÿÿ­Œv */
	$"FA6E 0247 00C9 FEFF 0183 88F9 6E03 0B00"            /* ún.G.Éşÿ.ƒˆùn... */
	$"597A FE6E FD6E 0434 00C8 3B8A F96E 0100"            /* Yzşnın.4.È;Šùn.. */
	$"62FA FF01 3683 FA6E 036F 1B1B 66F9 6E02"            /* búÿ.6ƒún.o..fùn. */
	$"3300 3DF6 FF01 9891 F96E 021A 0075 F96E"            /* 3.=öÿ.˜‘ùn...uùn */
	$"0448 0000 5D78 FA6E 0247 00C9 F8FF 0176"            /* .H..]xún.G.Éøÿ.v */
	$"8AF9 6E01 0B4A FEFF 0182 8CFC 6EFE 6E02"            /* Šùn..Jşÿ.‚Œünşn. */
	$"5900 54F8 6E02 000C 83F9 6E03 1A00 3190"            /* Y.Tøn...ƒùn...1 */
	$"F96E 030B 0070 7AFA 6E05 3400 62FF 5F83"            /* ùn...pzún.4.bÿ_ƒ */
	$"F96E 021A 1B5C F96E 0434 0030 4686 F16E"            /* ùn...\ùn.4.0F†ñn */
	$"056F 583D 3675 C5F7 FF02 AA8E 8EF9 6E05"            /* .oX=6uÅ÷ÿ.ªùn. */
	$"4E52 526B 6E6E FC6E 0447 0000 957A FA6E"            /* NRRknnün.G..•zún */
	$"035F 5252 66FA 6E04 5900 302F 96F9 6E02"            /* ._RRfún.Y.0/–ùn. */
	$"4800 5CF9 6E03 6C00 2178 FA6E 056D 0000"            /* H.\ùn.l.!xún.m.. */
	$"618C 7BFA 6E02 3400 AAF9 FF02 CF96 A5F9"            /* aŒ{ún.4.ªùÿ.Ï–¥ù */
	$"7D03 5A5F 5F75 FA7D 0469 0799 3198 F97D"            /* }.Z__uú}.i.™1˜ù} */
	$"056B 5F5F 797D 7DFC 7D07 5507 6CFF FFB6"            /* .k__y}}ü}.U.lÿÿ¶ */
	$"9985 FA7D 0255 07CC FEFF 0191 96F9 7D03"            /* ™…ú}.U.Ìşÿ.‘–ù}. */
	$"1607 6688 FA7D 0443 07CC 4897 F97D 0107"            /* ..fˆú}.C.ÌH—ù}.. */
	$"6CFA FF01 4590 FA7D 0380 2625 76F9 7D02"            /* lúÿ.Eú}.€&%vù}. */
	$"4007 45F6 FF01 A39F F97D 0225 0786 F97D"            /* @.Eöÿ.£Ÿù}.%.†ù} */
	$"0255 0707 0168 86FA 7D02 5507 CCF8 FF01"            /* .U...h†ú}.U.Ìøÿ. */
	$"8697 F97D 0116 50FE FF01 909A F97D 0269"            /* †—ù}..Pşÿ.šù}.i */
	$"0761 F87D 0207 1990 F97D 0325 073A 9CF9"            /* .aø}...ù}.%.:œù */
	$"7D03 1607 7D88 FA7D 0543 076C FF70 91F9"            /* }...}ˆú}.C.lÿp‘ù */
	$"7DFF 2500 6DF9 7D04 4207 3A52 94FB 7DF7"            /* }ÿ%.mù}.B.:R”û}÷ */
	$"7D05 7F68 5048 81C9 F7FF 02B3 9B9A F97D"            /* }..hPHÉ÷ÿ.³›šù} */
	$"035D 5F5F 7AFA 7D04 5507 07A1 88FA 7D03"            /* .]__zú}.U..¡ˆú}. */
	$"6F5F 5F75 FA7D 0469 073B 39A1 F97D 0255"            /* o__uú}.i.;9¡ù}.U */
	$"0768 F87D 0207 2B86 F97D FF07 0267 998A"            /* .hø}..+†ù}ÿ..g™Š */
	$"FA7D 0243 07AC FBFF 00FF 0305 03FF CA83"            /* ú}.C.¬ûÿ.ÿ...ÿÊƒ */
	$"97F9 6603 3D00 0065 FA66 0453 0096 1A88"            /* —ùf.=..eúf.S.–.ˆ */
	$"F966 0322 0000 73FA 6607 4200 6BFF FFAB"            /* ùf."..súf.B.kÿÿ« */
	$"886F FA66 0243 00C8 FEFF 0180 84F9 6603"            /* ˆoúf.C.Èşÿ.€„ùf. */
	$"0900 5B73 FA66 0430 00C8 3886 F966 0100"            /* Æ.[súf.0.È8†ùf.. */
	$"5FFA FF01 577D F966 0200 147C FD66 FD66"            /* _úÿ.W}ùf...|ıfıf */
	$"0231 0048 F6FF 0195 8FF9 6602 1600 72F9"            /* .1.Höÿ.•ùf...rù */
	$"6604 4200 005B 71FA 6602 4300 C8F8 FF01"            /* f.B..[qúf.C.Èøÿ. */
	$"7286 F966 0109 48FE FF01 7988 F966 0253"            /* r†ùf.ÆHşÿ.yˆùf.S */
	$"0052 F866 0205 0A7D F966 0316 002E 8DF9"            /* .Røf..Â}ùf....ù */
	$"6604 0900 6D75 66FB 6605 3000 5FFF 5C7F"            /* f.Æ.mufûf.0._ÿ\. */
	$"F966 0200 0F7B F966 0431 002E 5082 FA66"            /* ùf...{ùf.1..P‚úf */
	$"0168 5BF8 6603 6928 0068 F7FF 02A6 888A"            /* .h[øf.i(.h÷ÿ.¦ˆŠ */
	$"F966 032A 0005 6CFA 6604 4300 0092 73FA"            /* ùf.*..lúf.C..’sú */
	$"6603 3000 006E FA66 0453 0032 2B94 F966"            /* f.0..núf.S.2+”ùf */
	$"0142 0001 5A6C F966 0200 1F71 F966 FF00"            /* .B..Zlùf...qùfÿ. */
	$"0270 8875 FA66 0230 00AA F9FF 02CB 8599"            /* .pˆuúf.0.ªùÿ.Ë…™ */
	$"F969 0340 0000 67FA 6904 5700 971C 8BF9"            /* ùi.@..gúi.W.—.‹ù */
	$"6903 2600 0177 FA69 0745 006D FFFF AD8B"            /* i.&..wúi.E.mÿÿ­‹ */
	$"72FA 6902 4600 C9FE FF01 8286 F969 030B"            /* rúi.F.Éşÿ.‚†ùi.. */
	$"005D 77FE 69FD 6904 3300 C83A 88F9 6901"            /* .]wşiıi.3.È:ˆùi. */
	$"0062 FAFF 015A 81F9 6902 0016 80F9 6902"            /* .búÿ.Zùi...€ùi. */
	$"3400 4AF6 FF01 9791 F969 0219 0076 F969"            /* 4.Jöÿ.—‘ùi...vùi */
	$"0445 0000 5D75 FA69 0246 00C9 F8FF 0175"            /* .E..]uúi.F.Éøÿ.u */
	$"88F9 6901 0B4A FEFF 017C 8BFC 69FE 6902"            /* ˆùi..Jşÿ.|‹üişi. */
	$"5700 54F8 6902 070C 81F9 6903 1A00 308F"            /* W.Tøi...ùi...0 */
	$"F969 030B 0070 78FA 6905 3300 62FF 5F82"            /* ùi...pxúi.3.bÿ_‚ */
	$"F969 0200 117F F969 0434 0030 5284 FA69"            /* ùi....ùi.4.0R„úi */
	$"016C 5DF8 6903 6D2A 016B F7FF 02A8 8B8D"            /* .l]øi.m*.k÷ÿ.¨‹ */
	$"F969 052E 0007 6F69 69FC 6904 4600 0095"            /* ùi....oiiüi.F..• */
	$"77FA 6903 3300 0071 FA69 0457 0035 2E96"            /* wúi.3..qúi.W.5.– */
	$"F969 0345 005C 6FF9 6902 0021 75F9 69FF"            /* ùi.E.\oùi..!uùiÿ */
	$"0002 718B 78FA 6902 3300 AAF9 FF02 CE94"            /* ..q‹xúi.3.ªùÿ.Î” */
	$"A4F9 7A03 5007 0777 FA7A 0467 0799 2998"            /* ¤ùz.P..wúz.g.™)˜ */
	$"F97A 0533 070A 857A 7AFC 7A07 5307 76FF"            /* ùz.3.Â…zzüz.S.vÿ */
	$"FFB5 9883 FA7A 0254 07CC FEFF 0191 95F9"            /* ÿµ˜ƒúz.T.Ìşÿ.‘•ù */
	$"7A03 1607 6986 FA7A 0442 07CC 4796 F97A"            /* z...i†úz.B.ÌG–ùz */
	$"0107 6CFA FF01 698F FA7A 037B 0724 8EF9"            /* ..lúÿ.iúz.{.$ù */
	$"7A02 4207 50F6 FF01 A29E F97A 0224 0786"            /* z.B.Pöÿ.¢ùz.$.† */
	$"F97A 0253 0707 0168 84FA 7A02 5407 CCF8"            /* ùz.S...h„úz.T.Ìø */
	$"FF01 8596 F97A 0116 50FE FF01 8C98 F97A"            /* ÿ.…–ùz..Pşÿ.Œ˜ùz */
	$"0267 0761 F87A 0212 198F F97A 0325 073A"            /* .g.aøz...ùz.%.: */
	$"9BF9 7A03 1607 7D86 FA7A 0542 076C FF6F"            /* ›ùz...}†úz.B.lÿo */
	$"90F9 7A02 071D 8DF9 7A04 4207 3A5F 92FB"            /* ùz...ùz.B.:_’û */
	$"7A02 7A7C 6EF8 7A03 7D3A 0B73 F7FF 02B0"            /* z.z|nøz.}:.s÷ÿ.° */
	$"9899 F97A 033B 0712 80FA 7A04 5407 07A0"            /* ˜™ùz.;..€úz.T..  */
	$"85FA 7A03 4207 0780 FA7A 0467 0742 38A1"            /* …úz.B..€úz.g.B8¡ */
	$"F97A 0353 0768 80FA 7A03 7B07 2B84 FA7A"            /* ùz.S.h€úz.{.+„úz */
	$"057B 0707 7699 87FA 7A02 4207 ACFB FF00"            /* .{..v™‡úz.B.¬ûÿ. */
	$"FF03 0E03 FFCA 8196 F963 033C 000F 78FA"            /* ÿ...ÿÊ–ùc.<..xú */
	$"6304 5100 961A 87F9 6303 2200 267C FA63"            /* c.Q.–.‡ùc.".&|úc */
	$"0740 007A FFFF AB88 6DFA 6302 4200 C8FE"            /* .@.zÿÿ«ˆmúc.B.Èş */
	$"FF01 7F83 F963 0309 005B 71FA 6304 2F00"            /* ÿ..ƒùc.Æ.[qúc./. */
	$"C838 86F9 6301 005F FAFF 017C 7DFA 6303"            /* È8†ùc.._úÿ.|}úc. */
	$"6400 1A7A FD63 FD63 0231 004A F6FF 0194"            /* d..zıcıc.1.Jöÿ.” */
	$"8FF9 6303 1500 7371 FA63 0440 0000 5A6F"            /* ùc...sqúc.@..Zo */
	$"FA63 0242 00C8 F8FF 0179 85F9 6301 0948"            /* úc.B.Èøÿ.y…ùc.ÆH */
	$"FEFF 0177 88F9 6302 5100 52F8 6302 0C0A"            /* şÿ.wˆùc.Q.Røc..Â */
	$"7CF9 6303 1600 2E8C F963 0409 006D 7263"            /* |ùc....Œùc.Æ.mrc */
	$"FB63 052F 005F FF5C 7DF9 6302 000B 7FF9"            /* ûc./._ÿ\}ùc....ù */
	$"6304 3100 4542 81FA 6303 6400 3364 F963"            /* c.1.EBúc.d.3dùc */
	$"0222 007A F7FF 02A5 878A F963 0331 003A"            /* .".z÷ÿ.¥‡Šùc.1.: */
	$"6DFA 6304 4200 0091 71FA 6303 2F00 1C84"            /* múc.B..‘qúc./..„ */
	$"FA63 0451 0039 2194 F963 0146 0001 5A76"            /* úc.Q.9!”ùc.F..Zv */
	$"FA63 0364 001F 70FA 6305 6400 0070 8873"            /* úc.d..púc.d..pˆs */
	$"FA63 022F 00AA F9FF 02CA 8398 F967 033F"            /* úc./.ªùÿ.Êƒ˜ùg.? */
	$"0011 7BFA 6704 5400 971C 8AF9 6703 2500"            /* ..{úg.T.—.Šùg.%. */
	$"287F FA67 0744 007C FFFF AC8B 71FA 6702"            /* (.úg.D.|ÿÿ¬‹qúg. */
	$"4500 C9FE FF01 8286 F967 030B 005D 76FE"            /* E.Éşÿ.‚†ùg...]vş */
	$"67FD 6704 3200 C83A 88F9 6701 0062 FAFF"            /* gıg.2.È:ˆùg..búÿ */
	$"FF80 FA67 0368 001C 7CF9 6702 3300 4DF6"            /* ÿ€úg.h..|ùg.3.Mö */
	$"FF01 9691 F967 0319 0076 76FA 6704 4400"            /* ÿ.–‘ùg...vvúg.D. */
	$"005C 73FA 6702 4500 C9F8 FF01 7B88 F967"            /* .\súg.E.Éøÿ.{ˆùg */
	$"010B 4AFE FF01 7A8B FC67 FE67 0254 0054"            /* ..Jşÿ.z‹ügşg.T.T */
	$"F867 020F 0C80 F967 031A 0030 8EF9 6703"            /* øg...€ùg...0ùg. */
	$"0B00 7076 FA67 0532 0062 FF5F 81F9 6702"            /* ..pvúg.2.bÿ_ùg. */
	$"000D 82F9 6704 3300 4745 83FA 6703 6800"            /* ..‚ùg.3.GEƒúg.h. */
	$"3568 F967 0225 007C F7FF 02A8 8A8C F967"            /* 5hùg.%.|÷ÿ.¨ŠŒùg */
	$"0533 003C 7067 67FC 6704 4500 0094 75FA"            /* .3.<pggüg.E..”uú */
	$"6703 3200 1E86 FA67 0454 003C 2496 F967"            /* g.2..†úg.T.<$–ùg */
	$"0349 005C 79F9 6702 0021 73FA 6705 6800"            /* .I.\yùg..!súg.h. */
	$"0071 8B77 FA67 0232 00AA F9FF 02CE 91A3"            /* .q‹wúg.2.ªùÿ.Î‘£ */
	$"F978 034E 071D 8AFA 7804 6607 9929 97F9"            /* ùx.N..Šúx.f.™)—ù */
	$"7805 3207 358D 7878 FC78 0751 0783 FFFF"            /* x.2.5xxüx.Q.ƒÿÿ */
	$"B598 81FA 7802 5307 CCFE FF01 9094 F978"            /* µ˜úx.S.Ìşÿ.”ùx */
	$"0316 0769 84FA 7804 4007 CC47 96F9 7801"            /* ...i„úx.@.ÌG–ùx. */
	$"076C FAFF FF8E FA78 0379 0729 8BF9 7802"            /* .lúÿÿúx.y.)‹ùx. */
	$"4007 52F6 FF01 A19E F978 0322 0786 84FA"            /* @.Röÿ.¡ùx.".†„ú */
	$"7802 5107 0701 6882 FA78 0253 07CC F8FF"            /* x.Q...h‚úx.S.Ìøÿ */
	$"018B 96F9 7801 1650 FEFF 018A 98F9 7802"            /* .‹–ùx..Pşÿ.Š˜ùx. */
	$"6607 61F8 7802 1B19 8DF9 7803 2507 3A9A"            /* f.aøx...ùx.%.:š */
	$"F978 0316 077D 84FA 7805 4007 6CFF 6F8F"            /* ùx...}„úx.@.lÿo */
	$"F978 0207 1A8F F978 0440 074D 5091 FB78"            /* ùx...ùx.@.MP‘ûx */
	$"0478 7909 4379 F978 0232 0783 F7FF 02B0"            /* .xyÆCyùx.2.ƒ÷ÿ.° */
	$"9798 F978 0340 0749 81FA 7804 5307 07A0"            /* —˜ùx.@.Iúx.S..  */
	$"84FA 7803 4007 2994 FA78 0466 0747 30A1"            /* „úx.@.)”úx.f.G0¡ */
	$"F978 0357 0768 87FA 7803 7907 2B83 FA78"            /* ùx.W.h‡úx.y.+ƒúx */
	$"0579 0707 7698 85FA 7802 4007 ACFB FF00"            /* .y..v˜…úx.@.¬ûÿ. */
	$"FF03 1103 FFC9 8096 F961 0348 0019 77FA"            /* ÿ...ÿÉ€–ùa.H..wú */
	$"6104 4F00 961A 86F9 6103 2100 3A7B FA61"            /* a.O.–.†ùa.!.:{úa */
	$"073E 007A FFFF AB87 6CFA 6102 4000 C8FE"            /* .>.zÿÿ«‡lúa.@.Èş */
	$"FF01 7F83 F961 0309 005B 70FA 6104 2E00"            /* ÿ..ƒùa.Æ.[púa... */
	$"C838 85F9 6101 005F FAFF 018A 7CFA 6103"            /* È8…ùa.._úÿ.Š|úa. */
	$"6200 1A78 FD61 FD61 0230 005F F6FF 0194"            /* b..xıaıa.0._öÿ.” */
	$"8FF9 6103 1500 756F FA61 043E 0000 5A6E"            /* ùa...uoúa.>..Zn */
	$"FA61 0240 00C8 F8FF 0181 85F9 6101 0948"            /* úa.@.Èøÿ.…ùa.ÆH */
	$"FEFF 0177 87F9 6102 5A00 54F8 6102 0C13"            /* şÿ.w‡ùa.Z.Tøa... */
	$"7BF9 6103 1600 2E8B F961 0409 006D 7061"            /* {ùa....‹ùa.Æ.mpa */
	$"FB61 052E 005F FF5C 7DF9 6102 000A 88F9"            /* ûa..._ÿ\}ùa..Âˆù */
	$"6104 3000 4835 80FA 6103 6200 2179 F961"            /* a.0.H5€úa.b.!yùa */
	$"0221 007A F7FF 02A4 8588 F961 0332 003A"            /* .!.z÷ÿ.¤…ˆùa.2.: */
	$"6BFA 6104 4000 0090 6FFA 6103 2E00 2B83"            /* kúa.@..oúa...+ƒ */
	$"FA61 044F 0047 1A94 F961 0140 0001 4D75"            /* úa.O.G.”ùa.@..Mu */
	$"FA61 0362 001F 6EFA 6105 6200 0070 8871"            /* úa.b..núa.b..pˆq */
	$"FA61 022E 00AA F9FF 02CA 8298 F964 034C"            /* úa...ªùÿ.Ê‚˜ùd.L */
	$"001B 7AFA 6404 5300 971C 8AF9 6403 2500"            /* ..zúd.S.—.Šùd.%. */
	$"3C7D FA64 0742 007C FFFF AC8A 6FFA 6402"            /* <}úd.B.|ÿÿ¬Šoúd. */
	$"4400 C9FE FF01 8185 F964 030B 005D 73FE"            /* D.Éşÿ.…ùd...]sş */
	$"64FD 6404 3100 C83A 87F9 6401 0062 FAFF"            /* dıd.1.È:‡ùd..búÿ */
	$"018C 7FFA 6403 6600 1C7B F964 0233 0062"            /* .Œ.úd.f..{ùd.3.b */
	$"F6FF 0196 91F9 6403 1700 7873 FA64 0442"            /* öÿ.–‘ùd...xsúd.B */
	$"0000 5C71 FA64 0244 00C9 F8FF 0184 87F9"            /* ..\qúd.D.Éøÿ.„‡ù */
	$"6401 0B4A FEFF 017A 8AFC 64FE 6402 5D00"            /* d..Jşÿ.zŠüdşd.]. */
	$"57F8 6402 0F15 7FF9 6403 1A00 308D F964"            /* Wød....ùd...0ùd */
	$"030B 006F 73FA 6405 3100 62FF 5E80 F964"            /* ...osúd.1.bÿ^€ùd */
	$"0200 0C8B F964 0433 004A 3883 FA64 0366"            /* ...‹ùd.3.J8ƒúd.f */
	$"0024 7CF9 6402 2500 7CF7 FF02 A687 8BF9"            /* .$|ùd.%.|÷ÿ.¦‡‹ù */
	$"6405 3500 3D6E 6464 FC64 0444 0000 9272"            /* d.5.=nddüd.D..’r */
	$"FA64 0331 002E 85FA 6404 5300 491C 96F9"            /* úd.1..…úd.S.I.–ù */
	$"6403 4400 4F78 FA64 0366 0021 71FA 6405"            /* d.D.Oxúd.f.!qúd. */
	$"6600 0071 8B75 FA64 0231 00AA F9FF 02CD"            /* f..q‹uúd.1.ªùÿ.Í */
	$"90A3 F975 035B 0728 88FA 7504 6407 9929"            /* £ùu.[.(ˆúu.d.™) */
	$"96F9 7505 3207 498C 7575 FC75 0750 0783"            /* –ùu.2.IŒuuüu.P.ƒ */
	$"FFFF B598 7FFA 7502 5307 CCFE FF01 9092"            /* ÿÿµ˜.úu.S.Ìşÿ.’ */
	$"F975 0316 0769 82FA 7504 3F07 CC47 96F9"            /* ùu...i‚úu.?.ÌG–ù */
	$"7501 076C FAFF 0199 8EFA 7503 7707 298A"            /* u..lúÿ.™úu.w.)Š */
	$"F975 0240 076C F6FF 01A1 9EF9 7503 2207"            /* ùu.@.löÿ.¡ùu.". */
	$"8782 FA75 0250 0707 0168 81FA 7502 5307"            /* ‡‚úu.P...húu.S. */
	$"CCF8 FF01 9296 F975 0116 50FE FF01 8A98"            /* Ìøÿ.’–ùu..Pşÿ.Š˜ */
	$"F975 0270 0763 F875 021B 228D F975 0325"            /* ùu.p.cøu.."ùu.% */
	$"073A 99F9 7503 1607 7D83 FA75 053F 076C"            /* .:™ùu...}ƒúu.?.l */
	$"FF6F 8EF9 7502 0719 98F9 7504 4007 5045"            /* ÿoùu...˜ùu.@.PE */
	$"90FB 7504 7577 0730 8BF9 7502 3207 83F7"            /* ûu.uw.0‹ùu.2.ƒ÷ */
	$"FF02 AF96 97F9 7503 4207 4A7F FA75 0453"            /* ÿ.¯–—ùu.B.J.úu.S */
	$"0707 9F82 FA75 033F 0738 94FA 7504 6407"            /* ..Ÿ‚úu.?.8”úu.d. */
	$"5329 A1F9 7503 5307 5C86 FA75 0378 072B"            /* S)¡ùu.S.\†úu.x.+ */
	$"81FA 7505 7707 0776 9884 FA75 023F 07AC"            /* úu.w..v˜„úu.?.¬ */
	$"FBFF 00FF 0311 03FF C97C 95F9 5C03 4700"            /* ûÿ.ÿ...ÿÉ|•ù\.G. */
	$"1975 FA5C 044D 0096 1985 F95C 0320 003A"            /* .uú\.M.–.…ù\. .: */
	$"79FA 5C07 3C00 7AFF FFAA 8768 FA5C 023F"            /* yú\.<.zÿÿª‡hú\.? */
	$"00C8 FEFF 017D 81F9 5C03 0900 5B6D FA5C"            /* .Èşÿ.}ù\.Æ.[mú\ */
	$"042C 00C8 3684 F95C 0100 5FFA FF01 887B"            /* .,.È6„ù\.._úÿ.ˆ{ */
	$"FA5C 035F 000D 86FD 5CFD 5C02 2F00 5FF6"            /* ú\._..†ı\ı\./._ö */
	$"FF01 928E F95C 0314 006F 6DFA 5C04 3C00"            /* ÿ.’ù\...omú\.<. */
	$"005A 6BFA 5C02 3F00 C8F8 FF01 8084 F95C"            /* .Zkú\.?.Èøÿ.€„ù\ */
	$"0109 48FE FF01 7586 F95C 025F 0054 F85C"            /* .ÆHşÿ.u†ù\._.Tø\ */
	$"020C 1A79 F95C 0316 002C 8AF9 5C04 0900"            /* ...yù\...,Šù\.Æ. */
	$"6D6D 5CFB 5C05 2C00 5FFF 5B7B F95C 0200"            /* mm\û\.,._ÿ[{ù\.. */
	$"0A86 F95C 042F 0058 347F FA5C 035F 001A"            /* Â†ù\./.X4.ú\._.. */
	$"77F9 5C02 2000 8AF7 FF02 A382 86F9 5C03"            /* wù\. .Š÷ÿ.£‚†ù\. */
	$"2C00 3867 FA5C 043F 0000 7F6C FA5C 032C"            /* ,.8gú\.?...lú\., */
	$"002B 82FA 5C04 4D00 471A 92F9 5C01 3F00"            /* .+‚ú\.M.G.’ù\.?. */
	$"0149 71FA 5C03 5300 1F72 FA5C 055F 0000"            /* .Iqú\.S..rú\._.. */
	$"7087 6FFA 5C02 2C00 AAF9 FF02 CA80 97F9"            /* p‡oú\.,.ªùÿ.Ê€—ù */
	$"6103 4A00 1B78 FA61 0450 0097 1C87 F961"            /* a.J..xúa.P.—.‡ùa */
	$"0324 003C 7BFA 6107 3F00 7CFF FFAC 8A6C"            /* .$.<{úa.?.|ÿÿ¬Šl */
	$"FA61 0243 00C9 FEFF 0180 84F9 6103 0B00"            /* úa.C.Éşÿ.€„ùa... */
	$"5D70 FE61 FD61 0430 00C8 3A86 F961 0100"            /* ]pşaıa.0.È:†ùa.. */
	$"62FA FF01 8B7D FA61 0363 0010 8AF9 6102"            /* búÿ.‹}úa.c..Šùa. */
	$"3200 62F6 FF01 9590 F961 0316 0072 70FA"            /* 2.böÿ.•ùa...rpú */
	$"6104 3F00 025C 6EFA 6102 4300 C9F8 FF01"            /* a.?..\núa.C.Éøÿ. */
	$"8286 F961 010B 4AFE FF01 788A FC61 FE61"            /* ‚†ùa..Jşÿ.xŠüaşa */
	$"0263 0057 F861 020F 1C7C F961 031A 002F"            /* .c.Wøa...|ùa.../ */
	$"8CF9 6103 0B00 6F70 FA61 0530 0062 FF5E"            /* Œùa...opúa.0.bÿ^ */
	$"7DF9 6102 000C 88F9 6104 3200 5A38 81FA"            /* }ùa...ˆùa.2.Z8ú */
	$"6103 6300 1C7A F961 0224 008B F7FF 02A4"            /* a.c..zùa.$.‹÷ÿ.¤ */
	$"858A F961 052F 003A 6C61 61FC 6104 4300"            /* …Šùa./.:laaüa.C. */
	$"0082 70FA 6103 3000 2E84 FA61 0450 0049"            /* .‚púa.0..„úa.P.I */
	$"1C95 F961 0343 004C 75FA 6103 5700 2176"            /* .•ùa.C.Luúa.W.!v */
	$"FA61 0563 0000 718A 72FA 6102 3000 AAF9"            /* úa.c..qŠrúa.0.ªù */
	$"FF02 CD8E A2F9 7203 5A07 2886 FA72 0462"            /* ÿ.Í¢ùr.Z.(†úr.b */
	$"0799 2995 F972 0531 0749 8A72 72FC 7207"            /* .™)•ùr.1.IŠrrür. */
	$"4E07 83FF FFB4 977C FA72 0251 07CC FEFF"            /* N.ƒÿÿ´—|úr.Q.Ìşÿ */
	$"018F 91F9 7203 1607 6980 FA72 043E 07CC"            /* .‘ùr...i€úr.>.Ì */
	$"4794 F972 0107 6CFA FF01 988D FA72 0375"            /* G”ùr..lúÿ.˜úr.u */
	$"071C 96F9 7202 3F07 6CF6 FF01 A19E F972"            /* ..–ùr.?.löÿ.¡ùr */
	$"0321 0783 80FA 7202 4E07 0C01 687F FA72"            /* .!.ƒ€úr.N...h.úr */
	$"0251 07CC F8FF 0191 94F9 7201 1650 FEFF"            /* .Q.Ìøÿ.‘”ùr..Pşÿ */
	$"0187 97F9 7202 7607 63F8 7202 1B29 8BF9"            /* .‡—ùr.v.cør..)‹ù */
	$"7203 2507 3998 F972 0316 077C 81FA 7205"            /* r.%.9˜ùr...|úr. */
	$"3E07 6CFF 6F8D F972 0207 1996 F972 043F"            /* >.lÿoùr...–ùr.? */
	$"0762 448F FB72 0472 7507 2988 F972 0231"            /* .bDûr.ru.)ˆùr.1 */
	$"078F F7FF 02AE 9296 F972 033C 0747 7CFA"            /* .÷ÿ.®’–ùr.<.G|ú */
	$"7204 5107 0790 80FA 7203 3E07 3891 FA72"            /* r.Q..€úr.>.8‘úr */
	$"0462 0753 29A0 F972 0351 075A 84FA 7203"            /* .b.S) ùr.Q.Z„úr. */
	$"6807 2B85 FA72 0575 0707 7697 82FA 7202"            /* h.+…úr.u..v—‚úr. */
	$"3E07 ACFB FF00 FF03 1103 FFC9 7A94 F95A"            /* >.¬ûÿ.ÿ...ÿÉz”ùZ */
	$"0347 0017 73FA 5A04 4A00 9629 84F9 5A03"            /* .G..súZ.J.–)„ùZ. */
	$"2000 3A77 FA5A 073A 007A FFFF A987 66FA"            /*  .:wúZ.:.zÿÿ©‡fú */
	$"5A02 3E00 C8FE FF01 7C81 F95A 0309 005B"            /* Z.>.Èşÿ.|ùZ.Æ.[ */
	$"6BFA 5A04 2B00 C836 83F9 5A01 005F FAFF"            /* kúZ.+.È6ƒùZ.._úÿ */
	$"0187 7AFA 5A03 5D00 0A85 FD5A FD5A 022E"            /* .‡zúZ.].Â…ıZıZ.. */
	$"0073 F6FF 0191 8EF9 5A03 1300 666B FA5A"            /* .söÿ.‘ùZ...fkúZ */
	$"043A 0012 5968 FA5A 023E 00C8 F8FF 017F"            /* .:..YhúZ.>.Èøÿ.. */
	$"83F9 5A06 0948 FFFF C872 86F9 5A02 5D00"            /* ƒùZ.ÆHÿÿÈr†ùZ.]. */
	$"53F8 5A02 0C1A 67F9 5A03 1600 2B88 F95A"            /* SøZ...gùZ...+ˆùZ */
	$"0409 006C 6B5A FB5A 052B 005F FF5B 7AF9"            /* .Æ.lkZûZ.+._ÿ[zù */
	$"5AFF 0000 84F9 5A04 2E00 5F34 7DFA 5A03"            /* Zÿ..„ùZ..._4}úZ. */
	$"5D00 1A73 F95A 0220 0096 F7FF 02A1 8185"            /* ]..sùZ. .–÷ÿ.¡… */
	$"F95A 032E 0038 65FA 5A04 3E00 007F 6BFA"            /* ùZ...8eúZ.>...kú */
	$"5A03 2B00 2B81 FA5A 044A 0052 1992 F95A"            /* Z.+.+úZ.J.R.’ùZ */
	$"013E 0001 486F FA5A 034A 001F 80FA 5A05"            /* .>..HoúZ.J..€úZ. */
	$"5D00 0070 876D FA5A 022B 00AA F9FF 02C9"            /* ]..p‡múZ.+.ªùÿ.É */
	$"7D96 F95D 0349 001B 76FA 5D04 4E00 972C"            /* }–ù].I..vú].N.—, */
	$"86F9 5D03 2200 3C7A FA5D 073D 007C FFFF"            /* †ù].".<zú].=.|ÿÿ */
	$"AB8A 69FA 5D02 4200 C9FE FF01 8083 F95D"            /* «Šiú].B.Éşÿ.€ƒù] */
	$"030B 005D 6EFE 5DFD 5D04 2F00 C83A 85F9"            /* ...]nş]ı]./.È:…ù */
	$"5D01 0062 FAFF 018B 7CFA 5D03 6100 0C88"            /* ]..búÿ.‹|ú].a..ˆ */
	$"F95D 0231 0076 F6FF 0194 90F9 5D03 1500"            /* ù].1.vöÿ.”ù]... */
	$"696E FA5D 043D 0015 5B6C FA5D 0242 00C9"            /* inú].=..[lú].B.É */
	$"F8FF 0182 85F9 5D06 0B4A FFFF C876 88FC"            /* øÿ.‚…ù]..JÿÿÈvˆü */
	$"5DFE 5D02 6100 57F8 5D02 0F1C 6BF9 5D03"            /* ]ş].a.Wø]...kù]. */
	$"1A00 2E8B F95D 030B 006E 6EFA 5D05 2F00"            /* ...‹ù]...nnú]./. */
	$"62FF 5E7C F95D 0200 0286 F95D 0431 0062"            /* bÿ^|ù]...†ù].1.b */
	$"3680 FA5D 0361 001C 76F9 5D02 2200 97F7"            /* 6€ú].a..vù].".—÷ */
	$"FF02 A383 87F9 5D05 3000 3A68 5D5D FC5D"            /* ÿ.£ƒ‡ù].0.:h]]ü] */
	$"0442 0000 816D FA5D 032F 002E 83FA 5D04"            /* .B..mú]./..ƒú]. */
	$"4E00 541C 95F9 5D03 4200 4C72 FA5D 034E"            /* N.T.•ù].B.Lrú].N */
	$"0021 83FA 5D05 6100 0071 8A70 FA5D 022F"            /* .!ƒú].a..qŠpú]./ */
	$"00AA F9FF 02CD 8DA1 F96F 0359 0728 85FA"            /* .ªùÿ.Í¡ùo.Y.(…ú */
	$"6F04 6107 993D 94F9 6F05 3007 4988 6F6F"            /* o.a.™=”ùo.0.Iˆoo */
	$"FC6F 074C 0783 FFFF B397 7AFA 6F02 5107"            /* üo.L.ƒÿÿ³—zúo.Q. */
	$"CCFE FF01 8E91 F96F 0316 0769 7DFA 6F04"            /* Ìşÿ.‘ùo...i}úo. */
	$"3D07 CC46 94F9 6F01 076C FAFF 0198 8CFA"            /* =.ÌF”ùo..lúÿ.˜Œú */
	$"6F03 7207 1996 F96F 023E 077D F6FF 01A0"            /* o.r..–ùo.>.}öÿ.  */
	$"9CF9 6F03 2107 787D FA6F 024C 0722 0167"            /* œùo.!.x}úo.L.".g */
	$"7CFA 6F02 5107 CCF8 FF01 9094 F96F 0616"            /* |úo.Q.Ìøÿ.”ùo.. */
	$"50FF FFCC 8696 F96F 0273 0762 F86F 021B"            /* PÿÿÌ†–ùo.s.bøo.. */
	$"297B F96F 0325 0738 97F9 6F03 1607 7C7F"            /* ){ùo.%.8—ùo...|. */
	$"FA6F 053D 076C FF6E 8CF9 6F02 070C 94F9"            /* úo.=.lÿnŒùo...”ù */
	$"6F04 3E07 6C44 8EFB 6F04 6F72 0729 86F9"            /* o.>.lDûo.or.)†ù */
	$"6F02 3007 99F7 FF02 AD91 95F9 6F03 3E07"            /* o.0.™÷ÿ.­‘•ùo.>. */
	$"4779 FA6F 0451 0707 907D FA6F 033D 0738"            /* Gyúo.Q..}úo.=.8 */
	$"91FA 6F04 6107 5C29 A0F9 6F03 5107 5A82"            /* ‘úo.a.\) ùo.Q.Z‚ */
	$"FA6F 0361 072B 90FA 6F05 7207 0776 9780"            /* úo.a.+úo.r..v—€ */
	$"FA6F 023D 07AC FBFF 00FF 0311 03FF C879"            /* úo.=.¬ûÿ.ÿ...ÿÈy */
	$"92F9 5703 3F00 1771 FA57 0448 0096 2083"            /* ’ùW.?..qúW.H.– ƒ */
	$"F957 031F 003A 76FA 5707 3800 83FF FFA9"            /* ùW...:vúW.8.ƒÿÿ© */
	$"8664 FA57 023D 00C8 FEFF 017C 80F9 5703"            /* †dúW.=.Èşÿ.|€ùW. */
	$"0900 5B68 FA57 042A 00C8 3682 F957 0100"            /* Æ.[húW.*.È6‚ùW.. */
	$"5FFA FF01 8779 FA57 035B 000A 84FD 57FD"            /* _úÿ.‡yúW.[.Â„ıWı */
	$"5702 2E00 7AF6 FF01 918E F957 0313 0067"            /* W...zöÿ.‘ùW...g */
	$"67FA 5704 3800 1259 66FA 5702 3D00 C8F8"            /* gúW.8..YfúW.=.Èø */
	$"FF01 7F82 F957 0609 48FF FFC9 7285 F957"            /* ÿ..‚ùW.ÆHÿÿÉr…ùW */
	$"025B 0053 F857 0219 1A65 F957 0316 002B"            /* .[.SøW...eùW...+ */
	$"87F9 5704 0900 6B68 57FB 5705 2A00 5FFF"            /* ‡ùW.Æ.khWûW.*._ÿ */
	$"5A79 F957 FF00 0083 F957 042E 0066 347C"            /* ZyùWÿ..ƒùW...f4| */
	$"FA57 035B 0011 77F9 5702 1F00 96F7 FF02"            /* úW.[..wùW...–÷ÿ. */
	$"A180 84F9 5703 3600 3663 FA57 043D 0009"            /* ¡€„ùW.6.6cúW.=.Æ */
	$"7F67 FA57 032A 002B 80FA 5704 4800 5B19"            /* .gúW.*.+€úW.H.[. */
	$"91F9 5701 3D00 0148 6CFA 5703 4800 1F80"            /* ‘ùW.=..HlúW.H..€ */
	$"FA57 055B 0010 7087 6BFA 5702 2A00 AAF9"            /* úW.[..p‡kúW.*.ªù */
	$"FF02 C97B 95F9 5B03 4400 1B75 FA5B 044D"            /* ÿ.É{•ù[.D..uú[.M */
	$"0097 2485 F95B 0322 003C 79FA 5B07 3B00"            /* .—$…ù[.".<yú[.;. */
	$"85FF FFAB 8867 FA5B 0240 00C9 FEFF 017F"            /* …ÿÿ«ˆgú[.@.Éşÿ.. */
	$"82F9 5B03 0B00 5D6C FE5B FD5B 042E 00C8"            /* ‚ù[...]lş[ı[...È */
	$"3985 F95B 0100 62FA FF01 8A7C FA5B 035E"            /* 9…ù[..búÿ.Š|ú[.^ */
	$"000C 86F9 5B02 3100 7CF6 FF01 9490 F95B"            /* ..†ù[.1.|öÿ.”ù[ */
	$"0315 0069 6BFA 5B04 3B00 155B 69FA 5B02"            /* ...ikú[.;..[iú[. */
	$"4000 C9F8 FF01 8185 F95B 060B 4AFF FFC9"            /* @.Éøÿ.…ù[..JÿÿÉ */
	$"7687 FC5B FE5B 025F 0057 F85B FF1C 0068"            /* v‡ü[ş[._.Wø[ÿ..h */
	$"F95B 031A 002E 8AF9 5B03 0B00 6E6D FA5B"            /* ù[....Šù[...nmú[ */
	$"052E 0062 FF5D 7CF9 5BFF 0000 85F9 5B04"            /* ...bÿ]|ù[ÿ..…ù[. */
	$"3100 6836 80FA 5B03 5E00 1379 F95B 0222"            /* 1.h6€ú[.^..yù[." */
	$"0097 F7FF 02A2 8287 F95B 053A 003A 675B"            /* .—÷ÿ.¢‚‡ù[.:.:g[ */
	$"5BFC 5B04 4000 0B81 6CFA 5B03 2E00 2E83"            /* [ü[.@..lú[....ƒ */
	$"FA5B 044D 005D 1C94 F95B 0340 004C 70FA"            /* ú[.M.].”ù[.@.Lpú */
	$"5B03 4D00 2182 FA5B 055E 0013 718A 6EFA"            /* [.M.!‚ú[.^..qŠnú */
	$"5B02 2E00 AAF9 FF02 CD8B A1F9 6D03 5307"            /* [...ªùÿ.Í‹¡ùm.S. */
	$"2884 FA6D 045E 0799 3394 F96D 0530 0749"            /* („úm.^.™3”ùm.0.I */
	$"876D 6DFC 6D07 4A07 8BFF FFB3 9679 FA6D"            /* ‡mmüm.J.‹ÿÿ³–yúm */
	$"0250 07CC FEFF 018E 90F9 6D03 1607 697C"            /* .P.Ìşÿ.ùm...i| */
	$"FA6D 043D 07CC 4692 F96D 0107 6CFA FF01"            /* úm.=.ÌF’ùm..lúÿ. */
	$"978B FA6D 0371 0719 94F9 6D02 3E07 83F6"            /* —‹úm.q..”ùm.>.ƒö */
	$"FF01 A09C F96D 0320 0778 7BFA 6D02 4A07"            /* ÿ. œùm. .x{úm.J. */
	$"2201 677A FA6D 0250 07CC F8FF 0190 92F9"            /* ".gzúm.P.Ìøÿ.’ù */
	$"6D06 1650 FFFF CD86 96F9 6D02 7207 62F8"            /* m..PÿÿÍ†–ùm.r.bø */
	$"6D02 2829 79F9 6D03 2507 3897 F96D 0316"            /* m.()yùm.%.8—ùm.. */
	$"077B 7CFA 6D05 3D07 6CFF 6E8B F96D FF07"            /* .{|úm.=.lÿn‹ùmÿ. */
	$"0094 F96D 043E 0771 448D FB6D 046D 7107"            /* .”ùm.>.qDûm.mq. */
	$"1F88 F96D 0230 0799 F7FF 02AC 9095 F96D"            /* .ˆùm.0.™÷ÿ.¬•ùm */
	$"0349 0747 78FA 6D04 5007 1790 7BFA 6D03"            /* .I.Gxúm.P..{úm. */
	$"3D07 3890 FA6D 045E 0764 28A0 F96D 0350"            /* =.8úm.^.d( ùm.P */
	$"075A 80FA 6D03 5E07 2B90 FA6D 0571 0720"            /* .Z€úm.^.+úm.q.  */
	$"7697 7FFA 6D02 3D07 ACFB FF00 FF03 1103"            /* v—.úm.=.¬ûÿ.ÿ... */
	$"FFC7 7691 F953 0348 0012 6FFA 5304 4600"            /* ÿÇv‘ùS.H..oúS.F. */
	$"9620 81F9 5303 1E00 3A73 FA53 0735 0096"            /* – ùS...:súS.5.– */
	$"FFFF A884 62FA 5302 3C00 C8FE FF01 7A7F"            /* ÿÿ¨„búS.<.Èşÿ.z. */
	$"F953 0309 005B 66FA 5304 2900 C836 81F9"            /* ùS.Æ.[fúS.).È6ù */
	$"5301 005F FAFF 0185 77FA 5303 5800 0182"            /* S.._úÿ.…wúS.X..‚ */
	$"FD53 FD53 022C 008A F6FF 0190 8DF9 5303"            /* ıSıS.,.Šöÿ.ùS. */
	$"1200 6364 FA53 0435 0012 5964 FA53 023C"            /* ..cdúS.5..YdúS.< */
	$"00C8 F8FF 017C 81F9 5306 0948 FFFF D570"            /* .Èøÿ.|ùS.ÆHÿÿÕp */
	$"84F9 5302 5900 4DF8 5302 1F1A 62F9 5303"            /* „ùS.Y.MøS...bùS. */
	$"1600 2A85 F953 0409 0069 6653 FB53 0529"            /* ..*…ùS.Æ.ifSûS.) */
	$"005F FF5A 77F9 53FF 0000 80F9 5304 2C00"            /* ._ÿZwùSÿ..€ùS.,. */
	$"7A33 7BFA 5303 5800 0A82 F953 021E 00A8"            /* z3{úS.X.Â‚ùS...¨ */
	$"F7FF 029F 7C82 F953 0338 0036 61FA 5304"            /* ÷ÿ.Ÿ|‚ùS.8.6aúS. */
	$"3C00 127D 65FA 5303 2900 2B7F FA53 0446"            /* <..}eúS.).+.úS.F */
	$"005B 1B90 F953 012E 0001 4868 FA53 0346"            /* .[.ùS....HhúS.F */
	$"001E 7DFA 5305 5800 1270 8668 FA53 0229"            /* ..}úS.X..p†húS.) */
	$"00AA F9FF 02C8 7994 F957 034C 0015 72FA"            /* .ªùÿ.Èy”ùW.L..rú */
	$"5704 4900 9724 83F9 5703 2100 3C77 FA57"            /* W.I.—$ƒùW.!.<wúW */
	$"0739 0097 FFFF AA87 64FA 5702 3F00 C9FE"            /* .9.—ÿÿª‡dúW.?.Éş */
	$"FF01 7D81 F957 030B 005D 68FE 57FD 5704"            /* ÿ.}ùW...]hşWıW. */
	$"2C00 C839 83F9 5701 0062 FAFF 0188 7AFA"            /* ,.È9ƒùW..búÿ.ˆzú */
	$"5703 5B00 0384 F957 022F 008B F6FF 0192"            /* W.[..„ùW./.‹öÿ.’ */
	$"8FF9 5703 1400 6567 FA57 0439 0015 5B66"            /* ùW...egúW.9..[f */
	$"FA57 023F 00C9 F8FF 0180 83F9 5706 0B4A"            /* úW.?.Éøÿ.€ƒùW..J */
	$"FFFF D673 86FC 57FE 5702 5C00 50F8 5702"            /* ÿÿÖs†üWşW.\.PøW. */
	$"211C 65F9 5703 1A00 2C87 F957 030B 006C"            /* !.eùW...,‡ùW...l */
	$"69FA 5705 2C00 62FF 5D79 F957 FF00 0082"            /* iúW.,.bÿ]yùWÿ..‚ */
	$"F957 042F 007C 367D FA57 035B 000C 85F9"            /* ùW./.|6}úW.[..…ù */
	$"5702 2100 A9F7 FF02 A080 85F9 5705 3A00"            /* W.!.©÷ÿ. €…ùW.:. */
	$"3A63 5757 FC57 043F 0015 8168 FA57 032C"            /* :cWWüW.?..húW., */
	$"002E 81FA 5704 4900 5D1D 92F9 5703 3100"            /* ..úW.I.].’ùW.1. */
	$"4A6B FA57 0349 0020 80FA 5705 5B00 1571"            /* JkúW.I. €úW.[..q */
	$"886C FA57 022C 00AA F9FF 02CC 88A0 F969"            /* ˆlúW.,.ªùÿ.Ìˆ ùi */
	$"035E 0722 83FA 6904 5C07 9933 91F9 6905"            /* .^."ƒúi.\.™3‘ùi. */
	$"2F07 4986 6969 FC69 0748 0799 FFFF B395"            /* /.I†iiüi.H.™ÿÿ³• */
	$"76FA 6902 4F07 CCFE FF01 8D8F F969 0316"            /* vúi.O.Ìşÿ.ùi.. */
	$"0769 7AFA 6904 3C07 CC46 91F9 6901 076C"            /* .izúi.<.ÌF‘ùi..l */
	$"FAFF 0196 8AFA 6903 6F07 0F92 F969 023D"            /* úÿ.–Šúi.o..’ùi.= */
	$"078F F6FF 019F 9CF9 6903 1F07 7278 FA69"            /* .öÿ.Ÿœùi...rxúi */
	$"0248 0722 0167 78FA 6902 4F07 CCF8 FF01"            /* .H.".gxúi.O.Ìøÿ. */
	$"8E91 F969 0616 50FF FFD7 8495 F969 0270"            /* ‘ùi..Pÿÿ×„•ùi.p */
	$"075C F869 022E 2977 F969 0325 0736 95F9"            /* .\øi..)wùi.%.6•ù */
	$"6903 1607 7A7A FA69 053C 076C FF6D 88F9"            /* i...zzúi.<.lÿmˆù */
	$"69FF 0700 90F9 6904 3D07 8344 8CFB 6904"            /* iÿ..ùi.=.ƒDŒûi. */
	$"696F 0717 92F9 6902 2F07 AEF7 FF02 AA8E"            /* io..’ùi./.®÷ÿ.ª */
	$"92F9 6903 4A07 4775 FA69 044F 0722 8F79"            /* ’ùi.J.Guúi.O."y */
	$"FA69 033C 0738 8FFA 6904 5C07 642A 9FF9"            /* úi.<.8úi.\.d*Ÿù */
	$"6903 4007 597B FA69 035C 072B 8EFA 6905"            /* i.@.Y{úi.\.+úi. */
	$"6F07 2276 967C FA69 023C 07AC FBFF 00FF"            /* o."v–|úi.<.¬ûÿ.ÿ */
	$"0311 03FF C773 90F9 5003 4700 096D FA50"            /* ...ÿÇsùP.G.ÆmúP */
	$"0444 0096 2080 F950 031D 003A 72FA 5007"            /* .D.– €ùP...:rúP. */
	$"3300 96FF FFA8 835E FA50 023B 00C8 FEFF"            /* 3.–ÿÿ¨ƒ^úP.;.Èşÿ */
	$"0179 7CF9 5003 0900 5B63 FA50 0428 00C8"            /* .y|ùP.Æ.[cúP.(.È */
	$"3680 F950 0100 5FFA FF01 8576 FA50 0355"            /* 6€ùP.._úÿ.…vúP.U */
	$"0000 80FD 50FD 5002 2B00 96F6 FF01 8F8D"            /* ..€ıPıP.+.–öÿ. */
	$"F950 0311 0059 62FA 5004 3300 1258 61FA"            /* ùP...YbúP.3..Xaú */
	$"5002 3B00 C8F8 FF01 7C80 F950 0609 48FF"            /* P.;.Èøÿ.|€ùP.ÆHÿ */
	$"FFD5 6E82 F950 0252 003F F850 021F 1A5F"            /* ÿÕn‚ùP.R.?øP..._ */
	$"F950 0316 0034 84F9 5004 0900 6864 50FB"            /* ùP...4„ùP.Æ.hdPû */
	$"5005 2800 5FFF 5976 F950 FF00 0078 F950"            /* P.(._ÿYvùPÿ..xùP */
	$"042B 007A 3379 FA50 0355 000A 81F9 5002"            /* .+.z3yúP.U.ÂùP. */
	$"1D00 ADF7 FF02 9E7A 81F9 5003 4000 2E5D"            /* ..­÷ÿ.zùP.@..] */
	$"FA50 043B 0012 7C63 FA50 0328 002B 7DFA"            /* úP.;..|cúP.(.+}ú */
	$"5004 4400 5B2F 8FF9 5001 2800 0147 66FA"            /* P.D.[/ùP.(..Gfú */
	$"5003 4400 1E7C FA50 0555 0012 7085 66FA"            /* P.D..|úP.U..p…fú */
	$"5002 2800 AAF9 FF02 C877 92F9 5403 4C00"            /* P.(.ªùÿ.Èw’ùT.L. */
	$"0B70 FA54 0448 0097 2482 F954 0320 003C"            /* .púT.H.—$‚ùT. .< */
	$"76FA 5407 3600 97FF FFA9 8663 FA54 023E"            /* vúT.6.—ÿÿ©†cúT.> */
	$"00C9 FEFF 017C 80F9 5403 0B00 5D67 FE54"            /* .Éşÿ.|€ùT...]gşT */
	$"FD54 042B 00C8 3983 F954 0100 62FA FF01"            /* ıT.+.È9ƒùT..búÿ. */
	$"8879 FA54 035A 0000 82F9 5402 2F00 97F6"            /* ˆyúT.Z..‚ùT./.—ö */
	$"FF01 928F F954 0313 005B 65FA 5404 3600"            /* ÿ.’ùT...[eúT.6. */
	$"155A 65FA 5402 3E00 C9F8 FF01 7F82 F954"            /* .ZeúT.>.Éøÿ..‚ùT */
	$"060B 4AFF FFD5 7185 FC54 FE54 0257 0043"            /* ..JÿÿÕq…üTşT.W.C */
	$"F854 0221 1C63 F954 031A 0036 86F9 5403"            /* øT.!.cùT...6†ùT. */
	$"0B00 6C67 FA54 052B 0062 FF5C 79F9 54FF"            /* ..lgúT.+.bÿ\yùTÿ */
	$"0000 7BF9 5404 2F00 7C35 7CFA 5403 5A00"            /* ..{ùT./.|5|úT.Z. */
	$"0C83 F954 0220 00AE F7FF 029F 7C83 F954"            /* .ƒùT. .®÷ÿ.Ÿ|ƒùT */
	$"0544 0030 6254 54FC 5404 3E00 1580 66FA"            /* .D.0bTTüT.>..€fú */
	$"5403 2B00 2E80 FA54 0448 005D 3291 F954"            /* T.+..€úT.H.]2‘ùT */
	$"032B 004A 69FA 5403 4800 207F FA54 055A"            /* .+.JiúT.H. .úT.Z */
	$"0015 7187 69FA 5402 2B00 AAF9 FF02 CB87"            /* ..q‡iúT.+.ªùÿ.Ë‡ */
	$"9FF9 6703 5E07 1681 FA67 045B 0799 3390"            /* Ÿùg.^..úg.[.™3 */
	$"F967 052E 0749 8467 67FC 6707 4607 99FF"            /* ùg...I„ggüg.F.™ÿ */
	$"FFB2 9475 FA67 024E 07CC FEFF 018C 8EF9"            /* ÿ²”uúg.N.Ìşÿ.Œù */
	$"6703 1607 6878 FA67 043B 07CC 4690 F967"            /* g...hxúg.;.ÌFùg */
	$"0107 6CFA FF01 9688 FA67 036D 0707 90F9"            /* ..lúÿ.–ˆúg.m..ù */
	$"6702 3C07 99F6 FF01 9E9B F967 031F 0767"            /* g.<.™öÿ.›ùg...g */
	$"76FA 6702 4607 2201 6676 FA67 024E 07CC"            /* vúg.F.".fvúg.N.Ì */
	$"F8FF 018E 90F9 6706 1650 FFFF D783 94F9"            /* øÿ.ùg..Pÿÿ×ƒ”ù */
	$"6702 6907 51F8 6702 2E29 75F9 6703 2507"            /* g.i.Qøg..)uùg.%. */
	$"4394 F967 0316 0779 78FA 6705 3B07 6CFF"            /* C”ùg...yxúg.;.lÿ */
	$"6D87 F967 FF07 008B F967 043C 0783 438B"            /* m‡ùgÿ..‹ùg.<.ƒC‹ */
	$"FB67 0467 6D07 1791 F967 022E 07B5 F7FF"            /* ûg.gm..‘ùg...µ÷ÿ */
	$"02AA 8C91 F967 0354 073B 73FA 6704 4E07"            /* .ªŒ‘ùg.T.;súg.N. */
	$"228E 77FA 6703 3B07 388E FA67 045B 0764"            /* "wúg.;.8úg.[.d */
	$"449E F967 033B 0759 7AFA 6703 5B07 2A8D"            /* Dùg.;.Yzúg.[.* */
	$"FA67 056D 0722 7696 7AFA 6702 3B07 ACFB"            /* úg.m."v–zúg.;.¬û */
	$"FF00 FF03 3903 FFC5 718F F94D 034F 0003"            /* ÿ.ÿ.9.ÿÅqùM.O.. */
	$"6CFA 4D04 4300 9620 88F9 4D03 1D00 3A71"            /* lúM.C.– ˆùM...:q */
	$"FA4D 0731 0096 FFFF A683 5CFA 4D02 3A00"            /* úM.1.–ÿÿ¦ƒ\úM.:. */
	$"C8FE FF01 797C F94D 0309 005B 61FA 4D04"            /* Èşÿ.y|ùM.Æ.[aúM. */
	$"2700 C835 7FF9 4D04 0057 CECE D3FE D902"            /* '.È5.ùM..WÎÎÓşÙ. */
	$"D884 75FA 4D03 5300 007B FD4D FD4D 022B"            /* Ø„uúM.S..{ıMıM.+ */
	$"009B F6FF 018F 8CF9 4D03 1000 5A5F FA4D"            /* .›öÿ.ŒùM...Z_úM */
	$"0431 0017 585E FA4D 053A 00B5 CECE D7FE"            /* .1..X^úM.:.µÎÎ×ş */
	$"D904 DFFF FF7B 7FF9 4D06 0948 FFFF CC6D"            /* Ù.ßÿÿ{.ùM.ÆHÿÿÌm */
	$"81F8 4D01 0046 F84D 021F 1A5E F94D 0316"            /* øM..FøM...^ùM.. */
	$"0038 83F9 4D04 0900 6162 4DFB 4D05 2700"            /* .8ƒùM.Æ.abMûM.'. */
	$"5FFF 5975 F94D FF00 0081 F94D 042B 0091"            /* _ÿYuùMÿ..ùM.+.‘ */
	$"3379 FA4D 0353 0000 7FF9 4D02 1D00 ADF7"            /* 3yúM.S...ùM...­÷ */
	$"FF02 9C79 7FF9 4D03 4200 285B FA4D 043A"            /* ÿ.œy.ùM.B.([úM.: */
	$"0012 7C61 FA4D 0327 002B 7CFA 4D04 4300"            /* ..|aúM.'.+|úM.C. */
	$"5B29 8FF9 4D01 2700 0147 64FA 4D03 3500"            /* [)ùM.'..GdúM.5. */
	$"127A FA4D 0553 0012 7084 64FA 4D05 2700"            /* .zúM.S..p„dúM.'. */
	$"9ACE CED6 FED9 04DE FFC7 7591 F951 0352"            /* šÎÎÖşÙ.ŞÿÇu‘ùQ.R */
	$"0006 6FFA 5104 4600 9724 8BF9 5103 2000"            /* ..oúQ.F.—$‹ùQ. . */
	$"3C75 FA51 0734 0097 FFFF A986 61FA 5102"            /* <uúQ.4.—ÿÿ©†aúQ. */
	$"3D00 C9FE FF01 7C7F F951 030B 005D 65FE"            /* =.Éşÿ.|.ùQ...]eş */
	$"51FD 5104 2A00 C839 82F9 5104 0059 CFCF"            /* QıQ.*.È9‚ùQ..YÏÏ */
	$"D4FE D902 D887 78FA 5103 5800 007D F951"            /* ÔşÙ.Ø‡xúQ.X..}ùQ */
	$"022E 009C F6FF 0191 8EF9 5103 1300 5C63"            /* ...œöÿ.‘ùQ...\c */
	$"FA51 0434 001B 5A63 FA51 053D 00B7 CFCF"            /* úQ.4..ZcúQ.=.·ÏÏ */
	$"D8FE D904 DFFF FF7F 82F9 5106 0B4A FFFF"            /* ØşÙ.ßÿÿ.‚ùQ..Jÿÿ */
	$"CC70 84FC 51FD 5101 0049 F851 0221 1C62"            /* Ìp„üQıQ..IøQ.!.b */
	$"F951 031A 003A 85F9 5103 0B00 6365 FA51"            /* ùQ...:…ùQ...ceúQ */
	$"052A 0062 FF5C 78F9 51FF 0000 83F9 5104"            /* .*.bÿ\xùQÿ..ƒùQ. */
	$"2E00 9235 7BFA 5103 5800 0081 F951 0220"            /* ..’5{úQ.X..ùQ.  */
	$"00AE F7FF 029E 7B82 F951 0545 002A 5F51"            /* .®÷ÿ.{‚ùQ.E.*_Q */
	$"51FC 5104 3D00 157F 64FA 5103 2A00 2E80"            /* QüQ.=...dúQ.*..€ */
	$"FA51 0446 005D 2C91 F951 032A 0049 67FA"            /* úQ.F.],‘ùQ.*.Igú */
	$"5103 3900 147C FA51 0558 0015 7186 67FA"            /* Q.9..|úQ.X..q†gú */
	$"5105 2A00 9BCF CFD7 FED9 04DE FFCB 859E"            /* Q.*.›ÏÏ×şÙ.ŞÿË… */
	$"F964 0366 0710 80FA 6404 5907 9933 98F9"            /* ùd.f..€úd.Y.™3˜ù */
	$"6405 2E07 4984 6464 FC64 0745 0799 FFFF"            /* d...I„ddüd.E.™ÿÿ */
	$"B294 72FA 6402 4E07 CCFE FF01 8C8E F964"            /* ²”rúd.N.Ìşÿ.Œùd */
	$"0316 0768 76FA 6404 3A07 CC45 90F9 6404"            /* ...hvúd.:.ÌEùd. */
	$"0764 D3D3 D6FD DB01 9587 FA64 036B 0707"            /* .dÓÓÖıÛ.•‡úd.k.. */
	$"8DF9 6402 3C07 A0F6 FF01 9E9B F964 031E"            /* ùd.<. öÿ.›ùd.. */
	$"0768 75FA 6402 4507 2901 6673 FA64 054E"            /* .huúd.E.).fsúd.N */
	$"07BD D3D3 D9FE DB04 E0FF FF8D 90F9 6406"            /* .½ÓÓÙşÛ.àÿÿùd. */
	$"1650 FFFF CF82 92F8 6401 0759 F864 022E"            /* .PÿÿÏ‚’ød..Yød.. */
	$"2973 F964 0325 0747 92F9 6403 1607 6F76"            /* )sùd.%.G’ùd...ov */
	$"FA64 053A 076C FF6D 87F9 64FF 0700 91F9"            /* úd.:.lÿm‡ùdÿ..‘ù */
	$"6404 3C07 9643 8BFB 6404 646B 0708 8FF9"            /* d.<.–C‹ûd.dk..ù */
	$"6402 2E07 B5F7 FF02 A98B 8FF9 6403 5507"            /* d...µ÷ÿ.©‹ùd.U. */
	$"3571 FA64 044E 0722 8E76 FA64 033A 0738"            /* 5qúd.N."vúd.:.8 */
	$"8EFA 6404 5907 643D 9EF9 6403 3A07 5878"            /* úd.Y.d=ùd.:.Xx */
	$"FA64 034A 0720 8BFA 6405 6B07 2276 9579"            /* úd.J. ‹úd.k."v•y */
	$"FA64 083A 07A0 D3D3 D8DB DBDC 00DF 033A"            /* úd.:. ÓÓØÛÛÜ.ß.: */
	$"03FF C56E 8DF9 4903 4D3D 3464 FA49 043F"            /* .ÿÅnùI.M=4dúI.? */
	$"0096 3291 F949 031C 0039 6FFA 4907 2F00"            /* .–2‘ùI...9oúI./. */
	$"96FF FFA6 825A FA49 0239 00C8 FEFF 0178"            /* –ÿÿ¦‚ZúI.9.Èşÿ.x */
	$"7AF9 4903 0900 5A5E FA49 0426 00C8 357D"            /* zùI.Æ.Z^úI.&.È5} */
	$"F949 0A0D 273E 546B 6C6C 431D 7F73 FA49"            /* ùIÂ.'>TkllC..súI */
	$"0351 0000 77FD 49FD 4902 2A00 ADF6 FF01"            /* .Q..wıIıI.*.­öÿ. */
	$"8E8C F949 030F 0053 5CFA 4904 2F00 2957"            /* ŒùI...S\úI./.)W */
	$"5CFA 490D 3D17 3248 5F6B 6C63 2597 FFFF"            /* \úI.=.2H_klc%—ÿÿ */
	$"797D F949 0609 48FF FFB9 6B80 F849 011C"            /* y}ùI.ÆHÿÿ¹k€øI.. */
	$"50F8 4902 3138 55F9 4903 1600 3681 F949"            /* PøI.18UùI...6ùI */
	$"0427 385F 5749 FB49 0526 005F FF58 72F9"            /* .'8_WIûI.&._ÿXrù */
	$"49FF 0000 79F9 4904 2A00 9632 77FA 4903"            /* Iÿ..yùI.*.–2wúI. */
	$"5100 0078 F949 021C 00C8 F7FF 029A 757C"            /* Q..xùI...È÷ÿ.šu| */
	$"F949 0347 3D46 59FA 4904 3900 127A 5DFA"            /* ùI.G=FYúI.9..z]ú */
	$"4903 2600 2A7B FA49 043F 0066 208E F949"            /* I.&.*{úI.?.f ùI */
	$"0126 0001 4562 FA49 032F 0011 76FA 4905"            /* .&..EbúI./..vúI. */
	$"5100 1270 8362 FA49 0D2B 1630 4A5D 6B6C"            /* Q..pƒbúI.+.0J]kl */
	$"682A 96FF C771 8FF9 4E03 5140 3868 FA4E"            /* h*–ÿÇqùN.Q@8húN */
	$"0444 0097 3494 F94E 031F 003B 72FA 4E07"            /* .D.—4”ùN...;rúN. */
	$"3200 97FF FFA8 845D FA4E 023C 00C9 FEFF"            /* 2.—ÿÿ¨„]úN.<.Éşÿ */
	$"017B 7DF9 4E03 0B00 5C62 FE4E FD4E 0429"            /* .{}ùN...\bşNıN.) */
	$"00C8 3881 F94E 0A10 2A42 586E 6F6F 4620"            /* .È8ùNÂ.*BXnooF  */
	$"8177 FA4E 0354 0000 7AF9 4E02 2C00 AEF6"            /* wúN.T..zùN.,.®ö */
	$"FF01 918E F94E 0312 0055 61FA 4E04 3200"            /* ÿ.‘ùN...UaúN.2. */
	$"2C59 5FFA 4E0D 401A 344C 636E 6F66 2898"            /* ,Y_úN.@.4Lcnof(˜ */
	$"FFFF 7C80 F94E 060B 4AFF FFBA 6E83 FC4E"            /* ÿÿ|€ùN..JÿÿºnƒüN */
	$"FD4E 011E 53F8 4E02 333C 5AF9 4E03 1A00"            /* ıN..SøN.3<ZùN... */
	$"3983 F94E 032A 3B63 5BFA 4E05 2900 62FF"            /* 9ƒùN.*;c[úN.).bÿ */
	$"5B76 F94E FF00 007C F94E 042C 0097 347A"            /* [vùNÿ..|ùN.,.—4z */
	$"FA4E 0354 0000 7BF9 4E02 1F00 C9F7 FF02"            /* úN.T..{ùN...É÷ÿ. */
	$"9B78 80F9 4E05 4C40 495C 4E4E FC4E 043C"            /* ›x€ùN.L@I\NNüN.< */
	$"0015 7D62 FA4E 0329 002C 7DFA 4E04 4400"            /* ..}búN.).,}úN.D. */
	$"6824 90F9 4E03 2900 4865 FA4E 0332 0014"            /* h$ùN.).HeúN.2.. */
	$"79FA 4E05 5400 1571 8565 FA4E 0D2F 1933"            /* yúN.T..q…eúN./.3 */
	$"4D61 6E6F 6C2E 97FF CB83 9BF9 6203 6552"            /* Manol.—ÿËƒ›ùb.eR */
	$"4679 FA62 0457 0799 42A0 F962 052C 0748"            /* Fyúb.W.™B ùb.,.H */
	$"8262 62FC 6207 4307 99FF FFB0 9270 FA62"            /* ‚bbüb.C.™ÿÿ°’púb */
	$"024C 07CC FEFF 018B 8CF9 6203 1607 6873"            /* .L.Ìşÿ.‹Œùb...hs */
	$"FA62 0439 07CC 458F F962 0A1C 3651 6980"            /* úb.9.ÌEùbÂ.6Qi€ */
	$"8181 5530 9086 FA62 0368 0707 8AF9 6202"            /* U0†úb.h..Šùb. */
	$"3B07 B5F6 FF01 9E9B F962 031D 0763 72FA"            /* ;.µöÿ.›ùb...crú */
	$"6202 4307 3E01 6571 FA62 0D51 2644 5B76"            /* b.C.>.eqúb.Q&D[v */
	$"8081 7835 9AFF FF8C 8EF9 6206 1650 FFFF"            /* €x5šÿÿŒùb..Pÿÿ */
	$"C080 91F8 6201 2C66 F862 0243 4E6C F962"            /* À€‘øb.,føb.CNlùb */
	$"0325 0746 91F9 6203 3B4C 756E FA62 0539"            /* .%.F‘ùb.;Lunúb.9 */
	$"076C FF6C 85F9 62FF 0700 8AF9 6204 3B07"            /* .lÿl…ùbÿ..Šùb.;. */
	$"9943 88FB 6204 6268 0707 8AF9 6202 2C07"            /* ™Cˆûb.bh..Šùb.,. */
	$"CCF7 FF02 A688 8DF9 6203 5E50 596F FA62"            /* Ì÷ÿ.¦ˆùb.^PYoúb */
	$"044C 0722 8D73 FA62 0339 0736 8CFA 6204"            /* .L."súb.9.6Œúb. */
	$"5707 6D33 9CF9 6203 3907 5876 FA62 0343"            /* W.m3œùb.9.Xvúb.C */
	$"0720 87FA 6205 6807 2276 9477 FA62 083F"            /* . ‡úb.h."v”wúb.? */
	$"2642 5C73 8181 7D3B 0099 02AB 04FF D24E"            /* &B\s};.™.«.ÿÒN */
	$"2A3F EF46 043D 0096 3B90 F946 031B 0038"            /* *?ïF.=.–;ùF...8 */
	$"6DFA 4607 2C00 98FF FFA5 8158 FA46 0238"            /* múF.,.˜ÿÿ¥XúF.8 */
	$"00C8 FEFF 0177 79F9 4603 0900 595C FA46"            /* .Èşÿ.wyùF.Æ.Y\úF */
	$"0425 00C8 347C F246 0300 1263 72FA 4603"            /* .%.È4|òF...crúF. */
	$"4E00 007A FD46 FD46 0229 00AD F6FF 018E"            /* N..zıFıF.).­öÿ. */
	$"8BF9 4603 0F00 4859 FA46 042C 0029 575A"            /* ‹ùF...HYúF.,.)WZ */
	$"F346 063B 0096 FFFF 797C F946 0709 48FF"            /* óF.;.–ÿÿy|ùF.ÆHÿ */
	$"FFBA 6678 55E4 4604 450B 0026 5AF4 46FC"            /* ÿºfxUäF.E..&ZôFü */
	$"4606 4E1E 005F FF58 71F9 46FF 0000 72F9"            /* F.N.._ÿXqùFÿ..rù */
	$"4604 2900 A232 76FA 4603 4E00 006C F946"            /* F.).¢2vúF.N..lùF */
	$"021B 00C8 F7FF 03AC 3C2B 43EF 4604 3800"            /* ...È÷ÿ.¬<+CïF.8. */
	$"146C 5BFA 4603 2500 2979 FA46 043D 0070"            /* .l[úF.%.)yúF.=.p */
	$"208D F946 0125 0001 455D FA46 032C 0011"            /*  ùF.%..E]úF.,.. */
	$"75FA 4605 4E00 2A70 835F F346 073D 0096"            /* uúF.N.*pƒ_óF.=.– */
	$"FFD3 512E 44EF 4A04 4200 973D 92F9 4A03"            /* ÿÓQ.DïJ.B.—=’ùJ. */
	$"1E00 3B70 FA4A 0730 0099 FFFF A884 5BFA"            /* ..;púJ.0.™ÿÿ¨„[ú */
	$"4A02 3B00 C9FE FF01 7A7C F94A 030B 005B"            /* J.;.Éşÿ.z|ùJ...[ */
	$"5FFE 4AFD 4A04 2800 C838 80F2 4A03 0015"            /* _şJıJ.(.È8€òJ... */
	$"6775 FA4A 0352 0000 7DF9 4A02 2C00 AEF6"            /* guúJ.R..}ùJ.,.®ö */
	$"FF01 908D F94A 0311 004A 5CFA 4A04 3000"            /* ÿ.ùJ...J\úJ.0. */
	$"2C59 5DF3 4A06 3F00 97FF FF7C 7FF9 4A07"            /* ,Y]óJ.?.—ÿÿ|.ùJ. */
	$"0B4A FFFF BB69 7B5A FD4A E84A 0449 0F00"            /* .Jÿÿ»i{ZıJèJ.I.. */
	$"285D EF4A 0652 2000 62FF 5B75 F94A FF00"            /* (]ïJ.R .bÿ[uùJÿ. */
	$"0075 F94A 042C 00A3 3478 FA4A 0352 0000"            /* .uùJ.,.£4xúJ.R.. */
	$"6FF9 4A02 1E00 C9F7 FF03 AC40 2F46 F44A"            /* oùJ...É÷ÿ.¬@/FôJ */
	$"FC4A 043B 0017 6E5F FA4A 0328 002B 7CFA"            /* üJ.;..n_úJ.(.+|ú */
	$"4A04 4200 7124 8FF9 4A03 2800 4861 FA4A"            /* J.B.q$ùJ.(.HaúJ */
	$"0330 0013 78FA 4A05 5200 2C71 8563 F34A"            /* .0..xúJ.R.,q…cóJ */
	$"0742 0097 FFD5 643F 57EF 5E04 5507 9949"            /* .B.—ÿÕd?Wï^.U.™I */
	$"9FF9 5E05 2C07 4781 5E5E FC5E 0740 079C"            /* Ÿù^.,.G^^ü^.@.œ */
	$"FFFF B092 6EFA 5E02 4C07 CCFE FF01 8A8B"            /* ÿÿ°’nú^.L.Ìşÿ.Š‹ */
	$"F95E 0316 0767 71FA 5E04 3807 CC45 8EF2"            /* ù^...gqú^.8.ÌEò */
	$"5E03 0722 7885 FA5E 0366 0707 8BF9 5E02"            /* ^.."x…ú^.f..‹ù^. */
	$"3B07 B5F6 FF01 9C9A F95E 031D 0759 6FFA"            /* ;.µöÿ.œšù^...Yoú */
	$"5E02 4007 3E01 656F F35E 0650 0799 FFFF"            /* ^.@.>.eoó^.P.™ÿÿ */
	$"8C8D F95E 0716 50FF FFC1 7C8B 6CE4 5E04"            /* Œù^..PÿÿÁ|‹lä^. */
	$"5D19 0734 6EEF 5E06 652C 076C FF6C 84F9"            /* ]..4nï^.e,.lÿl„ù */
	$"5EFF 0700 82F9 5E04 3B07 A943 87FB 5E04"            /* ^ÿ..‚ù^.;.©C‡û^. */
	$"5E66 0707 80F9 5E02 2C07 CCF7 FF03 B453"            /* ^f..€ù^.,.Ì÷ÿ.´S */
	$"405B EF5E 044C 0725 7D71 FA5E 0338 0736"            /* @[ï^.L.%}qú^.8.6 */
	$"8BFA 5E04 5507 7632 9BF9 5E03 3807 5772"            /* ‹ú^.U.v2›ù^.8.Wr */
	$"FA5E 0340 071F 87FA 5E05 6607 3676 9475"            /* ú^.@..‡ú^.f.6v”u */
	$"F35E 0155 0700 9902 CBFF FF05 CA3B 1320"            /* ó^.U..™.Ëÿÿ.Ê;.  */
	$"3848 F244 043C 0096 3990 F944 031B 0038"            /* 8HòD.<.–9ùD...8 */
	$"6CFA 4407 2B00 ADFF FFA5 8155 FA44 0238"            /* lúD.+.­ÿÿ¥UúD.8 */
	$"00C8 FEFF 0177 79F9 4403 0900 595A FA44"            /* .Èşÿ.wyùD.Æ.YZúD */
	$"0425 00C8 347C F244 0300 1247 71FA 4403"            /* .%.È4|òD...GqúD. */
	$"4D00 0072 FD44 FD44 0229 00C1 F6FF 018D"            /* M..rıDıD.).Áöÿ. */
	$"8BF9 4403 0D00 4857 FA44 042B 0029 5758"            /* ‹ùD...HWúD.+.)WX */
	$"F344 063B 0096 FFFF 787B F944 0109 48FE"            /* óD.;.–ÿÿx{ùD.ÆHş */
	$"FF04 843B 2E2F 47E8 4408 4839 0A00 5728"            /* ÿ.„;./GèD.H9Â.W( */
	$"1C30 45F6 44FD 4407 461F 0028 CDFF 5870"            /* .0EöDıD.F..(ÍÿXp */
	$"F944 FF00 0069 F944 0429 00AD 3275 FA44"            /* ùDÿ..iùD.).­2uúD */
	$"034D 0000 71F9 4402 1B00 CEF6 FF04 AC29"            /* .M..qùD...Îöÿ.¬) */
	$"1228 3FF1 4404 3800 2E69 5AFA 4403 2500"            /* .(?ñD.8..iZúD.%. */
	$"2979 FA44 043C 0070 208C F944 0116 0001"            /* )yúD.<.p ŒùD.... */
	$"3F5A FA44 032B 0011 73FA 4405 4D00 2E70"            /* ?ZúD.+..súD.M..p */
	$"825E F344 0A3C 0096 FFFF CB3E 1622 3B4D"            /* ‚^óDÂ<.–ÿÿË>.";M */
	$"F247 0440 0097 3B92 F947 031E 003A 6FFA"            /* òG.@.—;’ùG...:oú */
	$"4707 2F00 AEFF FFA8 8359 FA47 023B 00C9"            /* G./.®ÿÿ¨ƒYúG.;.É */
	$"FEFF 017A 7BF9 4703 0B00 5B5D FE47 FD47"            /* şÿ.z{ùG...[]şGıG */
	$"0428 00C8 387F F247 0300 154A 75FA 4703"            /* .(.È8.òG...JuúG. */
	$"5000 0075 F947 022B 00C2 F6FF 018F 8DF9"            /* P..uùG.+.Âöÿ.ù */
	$"4703 1100 4A5B FA47 042F 002C 595B F347"            /* G...J[úG./.,Y[óG */
	$"063E 0097 FFFF 7B7F F947 010B 4AFE FF06"            /* .>.—ÿÿ{.ùG..Jşÿ. */
	$"863E 3132 4C47 47EA 4708 4D3C 0C00 582A"            /* †>12LGGêG.M<..X* */
	$"1F33 49F2 4707 4A21 002A CEFF 5B73 F947"            /* .3IòG.J!.*Îÿ[sùG */
	$"FF00 006C F947 042B 00AE 3478 FA47 0350"            /* ÿ..lùG.+.®4xúG.P */
	$"0000 75F9 4702 1E00 CFF6 FF04 AD2C 152A"            /* ..uùG...Ïöÿ.­,.* */
	$"43F6 47FC 4704 3B00 306C 5DFA 4703 2800"            /* CöGüG.;.0l]úG.(. */
	$"2B7B FA47 0440 0071 248E F947 0319 0043"            /* +{úG.@.q$ùG...C */
	$"5DFA 4703 2F00 1377 FA47 0550 0030 7184"            /* ]úG./..wúG.P.0q„ */
	$"62F3 470A 4000 97FF FFCE 4C22 314D 61F2"            /* bóGÂ@.—ÿÿÎL"1Maò */
	$"5C04 5307 9947 9FF9 5C05 2B07 4780 5C5C"            /* \.S.™GŸù\.+.G€\\ */
	$"FC5C 073F 07B5 FFFF B091 6CFA 5C02 4A07"            /* ü\.?.µÿÿ°‘lú\.J. */
	$"CCFE FF01 8A8B F95C 0316 0767 70FA 5C04"            /* Ìşÿ.Š‹ù\...gpú\. */
	$"3807 CC45 8DF2 5C03 0722 5A84 FA5C 0365"            /* 8.ÌEò\.."Z„ú\.e */
	$"0707 81F9 5C02 3A07 C7F6 FF01 9C9A F95C"            /* ..ù\.:.Çöÿ.œšù\ */
	$"031C 0759 6DFA 5C02 3F07 3E01 656E F35C"            /* ...Ymú\.?.>.enó\ */
	$"064F 0799 FFFF 8B8D F95C 0116 50FE FF04"            /* .O.™ÿÿ‹ù\..Pşÿ. */
	$"9451 3F43 5FE8 5C07 614F 1907 6133 2E45"            /* ”Q?C_è\.aO..a3.E */
	$"F15C 075D 2C07 33CF FF6C 83F9 5CFF 0700"            /* ñ\.],.3Ïÿlƒù\ÿ.. */
	$"78F9 5C04 3A07 B542 86FB 5C04 5C65 0707"            /* xù\.:.µB†û\.\e.. */
	$"83F9 5C02 2B07 D2F6 FF04 B43B 2138 55F1"            /* ƒù\.+.Òöÿ.´;!8Uñ */
	$"5C04 4A07 3A7A 6FFA 5C03 3807 368A FA5C"            /* \.J.:zoú\.8.6Šú\ */
	$"0453 0776 329B F95C 0326 0751 70FA 5C03"            /* .S.v2›ù\.&.Qpú\. */
	$"3F07 1F86 FA5C 0565 073A 7692 73F3 5C01"            /* ?..†ú\.e.:v’só\. */
	$"5307 0099 02FC FEFF 07CB 5B08 0007 1F27"            /* S..™.üşÿ.Ë[....' */
	$"32F5 3A04 3200 962C 84F9 3A03 1500 365B"            /* 2õ:.2.–,„ù:...6[ */
	$"FA3A 0725 00AD FFFF A47F 48FA 3A02 2B00"            /* ú:.%.­ÿÿ¤.Hú:.+. */
	$"C8FE FF01 756E F93A 0306 0055 49FA 3A04"            /* Èşÿ.unù:...UIú:. */
	$"1D00 C834 6DF2 3A03 0012 2F65 FA3A 033F"            /* ..È4mò:.../eú:.? */
	$"0000 5FFD 3AFD 3A02 2000 C8F6 FF01 8C7D"            /* .._ı:ı:. .Èöÿ.Œ} */
	$"F93A 030B 0040 4CFA 3A04 2500 2955 4AF3"            /* ù:...@Lú:.%.)UJó */
	$"3A06 3000 96FF FF77 6FF9 3A01 0648 FEFF"            /* :.0.–ÿÿwoù:..Hşÿ */
	$"07E0 790B 0007 2738 4AEE 400E 4543 3412"            /* .ày...'8Jî@.EC4. */
	$"0000 63FF D934 0000 1A3A 4AF9 400B 4045"            /* ..cÿÙ4...:Jù@.@E */
	$"3522 0000 25CD FFFF 5767 F93A FF00 0053"            /* 5"..%ÍÿÿWgù:ÿ..S */
	$"F93A 0420 00B3 3167 FA3A 033F 0000 64F9"            /* ù:. .³1gú:.?..dù */
	$"3A01 1500 F4FF 07B9 4700 0011 1B30 2FF5"            /* :...ôÿ.¹G....0/õ */
	$"3A04 2B00 2E68 4AFA 3A03 1D00 2966 FA3A"            /* :.+..hJú:...)fú: */
	$"0432 0070 3485 F93A 0110 0001 364E FA3A"            /* .2.p4…ù:....6Nú: */
	$"0325 0010 67FA 3A05 3F00 2E70 7F4E F33A"            /* .%..gú:.?..p.Nó: */
	$"0232 0096 FEFF 07CB 5C0A 0009 222A 35F5"            /* .2.–şÿ.Ë\Â.Æ"*5õ */
	$"3E04 3500 972F 86F9 3E03 1900 395E FA3E"            /* >.5.—/†ù>...9^ú> */
	$"0728 00AE FFFF A582 4CFA 3E02 2F00 C9FE"            /* .(.®ÿÿ¥‚Lú>./.Éş */
	$"FF01 7870 F93E 0308 0058 4EFE 3EFD 3E04"            /* ÿ.xpù>...XNş>ı>. */
	$"2000 C836 6FF2 3E03 0015 3268 FA3E 0344"            /*  .È6oò>...2hú>.D */
	$"0000 63F9 3E02 2400 C9F6 FF01 8F80 F93E"            /* ..cù>.$.Éöÿ.€ù> */
	$"030D 0043 4FFA 3E04 2800 2C58 4EF3 3E06"            /* ...COú>.(.,XNó>. */
	$"3300 97FF FF7A 72F9 3E01 084A FEFF 06E0"            /* 3.—ÿÿzrù>..Jşÿ.à */
	$"7A0D 0009 2A3B 004E EE45 0E49 4638 1400"            /* z..Æ*;.NîE.IF8.. */
	$"0065 FFDB 3600 001D 3D4F F845 0A49 3926"            /* .eÿÛ6...=OøEÂI9& */
	$"0000 27CD FFFF 5A6B F93E FF00 0057 F93E"            /* ..'ÍÿÿZkù>ÿ..Wù> */
	$"0424 00B4 346B FA3E 0344 0000 66F9 3E01"            /* .$.´4kú>.D..fù>. */
	$"1900 F4FF 07BA 4802 0013 1D33 32FA 3EFC"            /* ..ôÿ.ºH....32ú>ü */
	$"3E04 2F00 306C 4EFA 3E03 2000 2B69 FA3E"            /* >./.0lNú>. .+iú> */
	$"0435 0071 3687 F93E 0313 0039 51FA 3E03"            /* .5.q6‡ù>...9Qú>. */
	$"2800 1369 FA3E 0544 0030 7182 51F3 3E02"            /* (..iú>.D.0q‚Qó>. */
	$"3500 97FE FF07 CD64 1507 142E 3A46 F552"            /* 5.—şÿ.Íd....:FõR */
	$"0448 0799 3994 F952 0526 0746 6E52 52FC"            /* .H.™9”ùR.&.FnRRü */
	$"5207 3907 B5FF FFAF 905F FA52 023E 07CC"            /* R.9.µÿÿ¯_úR.>.Ì */
	$"FEFF 0187 81F9 5203 1307 645F FA52 042F"            /* şÿ.‡ùR...d_úR./ */
	$"07CC 447F F252 0307 223F 7AFA 5203 5707"            /* .ÌD.òR.."?zúR.W. */
	$"076F F952 0231 07CC F6FF 019B 8EF9 5203"            /* .oùR.1.Ìöÿ.›ùR. */
	$"1A07 5162 FA52 0239 073E 0164 61F3 5206"            /* ..QbúR.9.>.daóR. */
	$"4407 99FF FF8A 81F9 5201 1350 FEFF 07E1"            /* D.™ÿÿŠùR..Pşÿ.á */
	$"8117 0714 3B4D 63EE 5A0E 5E5B 471F 0707"            /* ...;McîZ.^[G... */
	$"6DFF DC3F 0807 2A4F 63F8 5A0A 5D4A 3208"            /* mÿÜ?..*OcøZÂ]J2. */
	$"0731 CFFF FF6B 7AF9 52FF 0700 65F9 5204"            /* .1ÏÿÿkzùRÿ..eùR. */
	$"3107 B942 7AFB 5204 5257 0707 73F9 5201"            /* 1.¹BzûR.RW..sùR. */
	$"2607 F4FF 07BE 510C 071F 2A44 45F5 5204"            /* &.ôÿ.¾Q...*DEõR. */
	$"3E07 3A79 61FA 5203 2F07 3579 FA52 0448"            /* >.:yaúR./.5yúR.H */
	$"0776 4496 F952 031F 0746 64FA 5203 3907"            /* .vD–ùR...FdúR.9. */
	$"1F7B FA52 0557 073A 7690 63F3 5201 4807"            /* .{úR.W.:vcóR.H. */
	$"0099 0268 FCFF 03D9 7C3C 15F3 0004 0812"            /* .™.hüÿ.Ù|<.ó.... */
	$"9E3E 24F7 1200 1AF7 1204 B3FF FF9E 34F7"            /* >$÷...÷..³ÿÿ4÷ */
	$"1200 CAFE FF01 591A F712 001A F712 02CA"            /* ..Êşÿ.Y.÷...÷..Ê */
	$"2517 F112 0227 2A15 F712 0019 FD12 FB12"            /* %.ñ..'*.÷...ı.û. */
	$"00D4 F6FF 017C 21F7 1200 19F7 1201 3F1F"            /* .Ôöÿ.|!÷...÷..?. */
	$"F012 049E FFFF 5219 F812 0058 FCFF 02D9"            /* ğ..ÿÿR.ø..Xüÿ.Ù */
	$"851C FE00 0113 17FD 12FF 1B04 0700 0619"            /* ….ş....ı.ÿ...... */
	$"16FC 1206 1D01 0000 0761 BDFD FF01 A247"            /* .ü.......a½ıÿ.¢G */
	$"FE00 0106 1BFC 1200 1B00 07FE 0001 3EAD"            /* ş....ü.....ş..>­ */
	$"FDFF 0140 16F7 1200 1AF7 1202 CA21 14F7"            /* ıÿ.@.÷...÷..Ê!.÷ */
	$"1200 17F7 12F2 FF03 CD6C 360A F300 0310"            /* ...÷.òÿ.Íl6Âó... */
	$"123F 20F7 1200 1AF7 1202 7B45 26F7 1200"            /* .? ÷...÷..{E&÷.. */
	$"19F7 1201 1614 F812 023F 7B2C F012 009E"            /* .÷....ø..?{,ğ.. */
	$"FCFF 03DB 7D3E 19F3 0004 0A15 9F42 27F7"            /* üÿ.Û}>.ó..Â.ŸB'÷ */
	$"1500 1DF7 1504 B4FF FFA0 38F7 1500 CBFE"            /* ...÷..´ÿÿ 8÷..Ëş */
	$"FF01 5C1C F715 001D FD15 FB15 02CA 281A"            /* ÿ.\.÷...ı.û..Ê(. */
	$"F115 022A 2E19 F715 001B F715 00D4 F6FF"            /* ñ..*..÷...÷..Ôöÿ */
	$"0180 25F7 1500 1CF7 1501 4321 F015 049F"            /* .€%÷...÷..C!ğ..Ÿ */
	$"FFFF 551B F815 005A FCFF 04DB 861E 0000"            /* ÿÿU.ø..Züÿ.Û†... */
	$"0200 151A FD15 061E 1D09 0008 1C1A FC15"            /* ....ı....Æ....ü. */
	$"061F 0300 000A 63BD FDFF 06A3 4801 0000"            /* .....Âc½ıÿ.£H... */
	$"081D FC15 011D 09FE 0001 40AF FDFF 0145"            /* ..ü...Æş..@¯ıÿ.E */
	$"1AF7 1500 1CF7 1502 CB25 16F7 1500 1BF7"            /* .÷...÷..Ë%.÷...÷ */
	$"15F2 FF03 CE6E 390D F800 FC00 0312 1542"            /* .òÿ.În9.ø.ü....B */
	$"24F7 1500 1DF7 1502 7C47 29F7 1500 1BF7"            /* $÷...÷..|G)÷...÷ */
	$"1501 1A16 F815 0242 7C30 F015 009F FCFF"            /* ....ø..B|0ğ..Ÿüÿ */
	$"03DC 8446 24F3 0704 1522 A24E 35F7 2200"            /* .Ü„F$ó..."¢N5÷". */
	$"2BFE 22FA 2204 B9FF FFAA 48F7 2200 CEFE"            /* +ş"ú".¹ÿÿªH÷".Îş */
	$"FF01 6E2A F722 002B F722 02CE 3628 F122"            /* ÿ.n*÷".+÷".Î6(ñ" */
	$"023A 3D26 F722 0029 F722 00D6 F6FF 018D"            /* .:=&÷".)÷".Ööÿ. */
	$"33F7 2200 29F7 2200 5400 30F0 2204 A2FF"            /* 3÷".)÷".T.0ğ".¢ÿ */
	$"FF68 29F8 2200 62FC FF02 DC8E 28FE 0701"            /* ÿh)ø".büÿ.Ü(ş.. */
	$"2127 FD22 062A 2914 0712 2726 FC22 062B"            /* !'ı".*)...'&ü".+ */
	$"0D07 0713 69C1 FDFF 06A9 510B 0707 122A"            /* ....iÁıÿ.©Q....* */
	$"FC22 012A 14FE 0701 49B4 FDFF 0157 28F7"            /* ü".*.ş..I´ıÿ.W(÷ */
	$"2200 2AF7 2202 CE34 24FB 22FD 2200 29F7"            /* ".*÷".Î4$û"ı".)÷ */
	$"22F2 FF03 D275 4019 F307 031F 224E 33F7"            /* "òÿ.Òu@.ó..."N3÷ */
	$"2200 2BF7 2202 8354 38F7 2200 29F7 2201"            /* ".+÷".ƒT8÷".)÷". */
	$"2824 F822 024E 833F F022 00A2 00DF F8FF"            /* ($ø".Nƒ?ğ".¢.ßøÿ */
	$"06D6 B3AA B4BA BABD FECF 00D6 95FF AEFF"            /* .Ö³ª´ºº½şÏ.Ö•ÿ®ÿ */
	$"03CE AD83 5FFB 4705 5B71 8175 5D49 FC47"            /* .Î­ƒ_ûG.[qu]IüG */
	$"0454 7898 B3D9 F9FF 04D1 AF8E 6F4E FC47"            /* .Tx˜³Ùùÿ.Ñ¯oNüG */
	$"0050 036D 8EB2 CEB9 FF06 CFAF AAB8 BABA"            /* .P.m²Î¹ÿ.Ï¯ª¸ºº */
	$"C0FE CF00 DBD8 FFCD FF06 D6B4 AAB5 BBBB"            /* ÀşÏ.ÛØÿÍÿ.Ö´ªµ»» */
	$"BEFE CF00 D6C0 FF83 FF01 CFAD 0184 62FB"            /* ¾şÏ.ÖÀÿƒÿ.Ï­.„bû */
	$"4905 5C72 8376 5E4C FC49 0457 7A99 B4D9"            /* I.\rƒv^LüI.Wz™´Ù */
	$"F9FF 04D2 B08F 7050 FC49 0451 6E8F B2CF"            /* ùÿ.Ò°pPüI.Qn²Ï */
	$"B9FF 08CF B0AA B9BB BBC1 CFCF 01CF DBA4"            /* ¹ÿ.Ï°ª¹»»ÁÏÏ.ÏÛ¤ */
	$"FF06 D8B8 ACB8 C1C1 C2FE D300 D8EB FF81"            /* ÿ.Ø¸¬¸ÁÁÂşÓ.Øëÿ */
	$"FFD8 FF03 D3B2 8B68 FB53 0564 788A 7A66"            /* ÿØÿ.Ó²‹hûS.dxŠzf */
	$"54FC 5304 5E80 9FB8 DBF9 FF04 D4B4 9677"            /* TüS.^€Ÿ¸Ûùÿ.Ô´–w */
	$"59FC 5304 5B73 96B6 D3DB FFDF FF06 D3B4"            /* YüS.[s–¶ÓÛÿßÿ.Ó´ */
	$"ACBE C1C1 C5FE D300 DCAE FF00 FF00 00FF"            /* ¬¾ÁÁÅşÓ.Ü®ÿ.ÿ..ÿ */
};

