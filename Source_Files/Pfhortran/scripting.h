/* script.h

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

   Monday, Febuary 14th, 2000 (Chris Pruett)

Oct 14, 2000 (Loren Petrich)
	Added function for loading a script from some in-memory data;
	this is an alternative to separate functions for loading from a resource or a file
	
June 13, 2001 (Loren Petrich): 
	Added bounds checking to execute_instruction;
		it will return whether it could execute that instruction
*/

#ifndef _SCRIPT_H
#define _SCRIPT_H

/* define out our three possible return states in english for greater readability */
// Here because load_script(), etc. returns them
#define script_TRUE 1
#define script_FALSE 0
#define script_ERROR -1

int load_script(int text_id);
void free_script(void);

// LP addition:
int load_script_data(void *Data, size_t DataLen);

// LP: don't execute initial instructions if restoring a savegame
void script_init(bool restoring_saved);
 
bool script_in_use(void);
 
/*bool instruction_finished(void);*/

bool do_next_instruction(void);
  
bool script_Camera_Active(void);
 
void activate_tag_switch_trap(int which);
void activate_light_switch_trap(int which);
void activate_platform_switch_trap(int which);
void activate_terminal_enter_trap(int which);
void activate_terminal_exit_trap(int which);
void activate_pattern_buffer_trap(int which);
void activate_got_item_trap(int which);
void activate_tag_activated_trap(int which);
void activate_light_activated_trap(int which);
void activate_platform_activated_trap(int which);

#endif
