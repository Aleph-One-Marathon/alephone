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

*/

/*
 *  computer_interface_mac.cpp - Terminal handling, MacOS specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

// LP addition: this is for getting the clipping rectangle to revert to
// when done with drawing the terminal text
extern GrafPtr world_pixels;

static struct terminal_key terminal_keys[]= {
	{0x7e, 0, 0, _terminal_page_up},  // arrow up
	{0x7d, 0, 0, _terminal_page_down},// arrow down
	{0x74, 0, 0, _terminal_page_up},   // page up
	{0x79, 0, 0, _terminal_page_down}, // page down
	{0x30, 0, 0, _terminal_next_state}, // tab
	{0x4c, 0, 0, _terminal_next_state}, // enter
	{0x24, 0, 0, _terminal_next_state}, // return
	{0x31, 0, 0, _terminal_next_state}, // space
	{0x3a, 0, 0, _terminal_next_state}, // command
	{0x35, 0, 0, _any_abort_key_mask}  // escape
};


static void	set_text_face(
	struct text_face_data *text_face)
{
	Style face= 0;
	RGBColor color;

	/* Set the computer interface font.. */

	/* Set the face/color crap */
	if(text_face->face & _bold_text) face |= bold;
	if(text_face->face & _italic_text) face |= italic;
	if(text_face->face & _underline_text) face |= underline;
	TextFace(face);

	/* Set the color */
	_get_interface_color(text_face->color+_computer_interface_text_color, &color);
	RGBForeColor(&color);
}

static bool calculate_line(
	char *base_text, 
	short width,
	short start_index,
	short text_end_index,
	short *end_index)
{
	bool done= FALSE;

	if(start_index!=text_end_index)
	{
		StyledLineBreakCode code;
		Fixed text_width;
		long end_of_line_offset= 1; /* non-zero.. */

		text_width= width;
		text_width <<= 16;

		code= StyledLineBreak(base_text, text_end_index, start_index,
			text_end_index, 0, &text_width, &end_of_line_offset);
		*end_index= end_of_line_offset;

		/* We assume the last line is empty, always.. */
		if(code==smBreakOverflow)
		{
			done= TRUE;
		}
//dprintf("Code: %d Length: %d Start: %d TextEnd: %d End: %d Star Text: %x", code, 
//	text_end_index, start_index, text_end_index, *end_index, &base_text[start_index]);
	} else {
		done= TRUE;
	}
	
	return done;
}
