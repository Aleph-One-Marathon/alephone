/* script_parser.h

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

June 13, 2001 (Loren Petrich): 
	Added script-length output to script parser

June 26, 2002 (Loren Petrich):
	Got script_parser to store the parsed script in a STL vector 
*/


#ifndef _SCRIPT_PARSER_DEF
#define _SCRIPT_PARSER_DEF

#include "cstypes.h"
#include <vector>
using namespace std;

// LP: added trap "load" for executing whenever a level is loaded;
// "init" will be for starting a level, and not for loading a savegame
enum /* procedure traps */
{
	idle = 1,
	load,
	init,
	tag_switch,
	light_switch,
	platform_switch,
	terminal_enter,
	terminal_exit,
	pattern_buffer,
	got_item,
	light_activated,
	platform_activated,
	
	NUMBER_OF_TRAPS

};

struct bind_table
{
	short current_offset;
	int start_offset;
	bool active;
	int which;
	bool available;
	uint32 instruction_decay;
};

struct script_instruction
{
	unsigned opcode; 	/* the instruction index */
	short mode;
	float op1;
	float op2;
	float op3;
};



bool init_pfhortran(void);
void dispose_pfhortran(void);
bool is_pfhortran_on(void);
void parse_script(char *input, vector<script_instruction>& instruction_list);

void clear_bind_table(void);
bind_table *get_bind_table(void);

int get_trap_value(short trap);
void set_trap_value(short trap, int val);
int get_trap_offset(short trap);
bool trap_active(short trap);
int get_trap_start(short trap);
void set_trap_instruction(short trap, int offset);
void reset_trap(short trap);
void activate_trap(short trap);
uint32 get_trap_instruction_decay(short trap);
void set_trap_instruction_decay(short trap, uint32 decay);

#endif
