/* script.c

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
   
	Feb 14th, 10:58 AM	- Got get_tokens working.  Lexical analizer is good to go.
	Feb 15th		   	- Basic instructions working! Yay!
	Feb 21st			- Scripts can turn tags on and off.  Basic variable management works (add, subtract, set)
						branching kind of works, but its kind of lame.  Needs more work.
	
Aug 15, 2000 (Loren Petrich):
	Changed file handler to use my new object-oriented version

June 13, 2001 (Loren Petrich): 
	Added script-length output to script parser
	Added bounds checking to execute_instruction;
		it will return whether it could execute that instruction
 */
 
#define MAX_VARS 64	//The max number of script variables allowed in a single script
#define MAX_DEPTH 256	//The max levels deep function calls are allowed to get


#include "cseries.h"

#include "scripting.h"
#include "script_instructions.h"
#include "script_parser.h"

#include "tags.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"

#include "FileHandler.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// LP: Added script length and set to 
vector<script_instruction> current_script;
int current_instruction = 0;


float variable_lookup[MAX_VARS];
int variable_count = 0;

int script_stack[MAX_DEPTH];
int stack_top = 0;
bool is_startup = false;

short camera_count = 0;
short current_camera = NONE;
script_camera *cameras = NULL;

short path_count = 0;
short current_path = NONE;
path_header *script_paths = NULL;
short current_path_point = NONE;
path_list camera_point;


short current_trap = NONE;


extern bool s_camera_Control;
extern void (*instruction_lookup[NUMBER_OF_INSTRUCTIONS])(script_instruction);


int get_next_instruction(void);
bool execute_instruction(int inst);
bool script_in_use(void);
bool instruction_finished(void);



void clean_up_script(void)
{
	current_script.clear();
	
	if (cameras)
		free(cameras);
		
	if (script_paths)
	{
		for(int x=0; x < path_count; x++)
			if (script_paths[x].the_path)
			{
				free(script_paths[x].the_path);
				script_paths[x].the_path = NULL;
			}
			
		free(script_paths);
	
	}
	
	cameras = NULL;
	
	camera_count = 0;
	current_camera = 0;
	current_instruction = 0;
	current_path = 0;
	current_path_point = 0;
	
	path_count = 0;
	script_paths = NULL;

	clear_bind_table();

}
 
/*load_script loads the source script from a TEXT ID in the map resource fork,
initalizes lineCount to point to the beginning of every line, and initalizes
all script variables to zero*/
int load_script(int text_id)
{
	if (!is_pfhortran_on())	/* we can't do too much if the pfhortran isn't running */
		return script_FALSE;
	
	// LP changes:
	FileSpecifier& CurrentMap = get_map_file();
	
	OpenedResourceFile OFile;
	if (!CurrentMap.Open(OFile))
	{
		set_game_error(systemError, CurrentMap.GetError());
		return script_ERROR;
	}
	
	LoadedResource TextRsrc;
	OFile.Get('T', 'E', 'X', 'T', text_id, TextRsrc);
	
	if (!TextRsrc.IsLoaded())
	{
		clean_up_script();
		is_startup = false;
		return script_TRUE;
	}
	
	int errcode = load_script_data(TextRsrc.GetPointer(),TextRsrc.GetLength());
	return errcode;
}


// LP addition:
int load_script_data(void *Data, size_t DataLen)
{
	if (!is_pfhortran_on())	/* we can't do too much if the pfhortran isn't running */
		return script_FALSE;
		
	char *origsrc = (char *)Data;
	size_t origlen = DataLen;
	
	clean_up_script();
	
	if (!origsrc || origlen == 0)
	{
		is_startup = false;
		
		return script_TRUE;
	}
	
	// Create a new copy to avoid buffer overflows,
	// and tack a C-string terminator on the end
	vector<char> src(origlen+1);
	memcpy(&src[0],origsrc,origlen);
	src[origlen] = 0;
	
	parse_script(&src[0],current_script);
	
	current_instruction = 0;
	/*instruction_decay = 0;*/
	
	for (int x=0; x<MAX_VARS; x++)
		variable_lookup[x] = 0;
	variable_count = 0;

	is_startup = false;
	
	return script_TRUE;
}


void script_init(bool restoring_saved)
{
	if (!script_in_use())
		return;
	
	s_camera_Control = false;
	
	// LP: always execute load instructions
	activate_trap(load);
	
	if (trap_active(load))
	{
		current_trap = init;
		is_startup = true;
		
		bool success = true;
		do
			success = do_next_instruction();
		while (trap_active(load) && script_in_use() && success);
	
		is_startup = false;
	}
		
	// LP: don't execute initial instructions if restoring a savegame
	if (restoring_saved)
	{
		is_startup = false;
	}
	else
	{
		activate_trap(init);
		
		if (trap_active(init))
		{
			current_trap = init;
			is_startup = true;
			
			bool success = true;
			do
				success = do_next_instruction();
			while (trap_active(init) && script_in_use() && success);
		
			is_startup = false;
		}
	}
	
	activate_trap(idle);	/* start the idle script running */
	
}
 
/*do_next_instruction gets the next instruction and executes it*/
// LP: changed to return whether there was an instruction to execute
bool do_next_instruction(void)
{
	
	if (is_startup)	/* on startup we want to execute ONLY the startup script */
	{
		if (trap_active(current_trap))
			{
				current_instruction = get_trap_offset(current_trap);
				if (!execute_instruction(get_next_instruction())) return false;
				
				if (trap_active(current_trap))
					set_trap_instruction(current_trap, current_instruction);
			}
	
	} else
	{
		for (current_trap = 1; current_trap < NUMBER_OF_TRAPS && script_in_use(); current_trap++)
			if (trap_active(current_trap))
			{
				if (!instruction_finished())
					continue;
					
				current_instruction = get_trap_offset(current_trap);
				
				if (!execute_instruction(get_next_instruction())) return false;
				
				if (trap_active(current_trap))
					set_trap_instruction(current_trap, current_instruction);
				else if (current_trap == idle)
				{
					reset_trap(idle);
					activate_trap(idle);
				}
			}
	}
	
	return true;
}


/*set_instruction_decay sets the amount of game ticks to wait until the instruction
is over*/
void set_instruction_decay(long decay)
{
	set_trap_instruction_decay(current_trap, decay);
	/*instruction_decay = decay;*/
}

/*jump_to_line sets the cursor to point at the beginning of the line
passed by newline*/
void jump_to_line(int newline)
{
	current_instruction = newline;
}

/*add_variable initalizes a new script variable*/
void add_variable(float var)
{
	if (variable_count >= MAX_VARS)
		return;

	variable_lookup[variable_count] = var;
	variable_count ++;

}

/*set_variable sets the content of a script variable at index var in
variable_lookup to val*/
void set_variable(int var, float val)
{
	if (var >= MAX_VARS || var > variable_count)
		return;

	variable_lookup[var] = val;

}

/*get_variable returns the content of a script variable at index var in
variable_lookup*/
float get_variable(int var)
{
	if (var >= MAX_VARS || var > variable_count)
		return 0;
	return variable_lookup[var];
}

/*free_script purges the current script from memory*/
void free_script(void)
{
	
	clean_up_script();
	
}

/*script_in_use returns true of there 
is currently a script loaded, false if there is not.*/
bool script_in_use(void)
{
	return !current_script.empty();
}


/*instruction_finished returns true of the last instuction is done, false if it's not.*/

bool instruction_finished(void)
{
	/*return (machine_tick_count() > instruction_decay);*/
	return (machine_tick_count() > get_trap_instruction_decay(current_trap));
}





/*get_next_instruction gets the next line of the source string and
parses it into a usable data structure.
*/
int get_next_instruction(void)
{
	int new_instruction;
	
	new_instruction = current_instruction;
	current_instruction++;
	
	return new_instruction;

}
 
/*execute_instruction calls the function pointed to by the
opcode passed in inst, a script_instuction.
*/
// LP: changed to return whether there was an instruction to execute
bool execute_instruction(int inst)
{
	if (inst < 0 || inst >= int(current_script.size())) return false;
	
	if (current_script[inst].opcode == 0 || current_script[inst].opcode > NUMBER_OF_INSTRUCTIONS)
		return true;
	
	
	(*instruction_lookup[current_script[inst].opcode])(current_script[inst]);	//call the function
	
	return true;
}
 
void stack_push(int val)
{
	if (stack_top < MAX_DEPTH-1)
	{
		script_stack[stack_top] = val;
		stack_top++;
	}
}

int stack_pop(void)
{
	if (stack_top >= 0)
	{
		return (script_stack[--stack_top]);

	}
	
	return -1;
}
 
// Suppressed for MSVC compatibility
#if 0
#pragma mark -
#endif

void activate_tag_switch_trap(int which)
{
	activate_trap(tag_switch);
	set_trap_value(tag_switch, which);
}

void activate_light_switch_trap(int which)
{
	activate_trap(light_switch);
	set_trap_value(light_switch, which);
}

void activate_platform_switch_trap(int which)
{
	activate_trap(platform_switch);
	set_trap_value(platform_switch, which);
}

void activate_terminal_enter_trap(int which)
{
	activate_trap(terminal_enter);
	set_trap_value(terminal_enter, which);
}

void activate_terminal_exit_trap(int which)
{
	activate_trap(terminal_exit);
	set_trap_value(terminal_exit, which);
}

void activate_pattern_buffer_trap(int which)
{
	activate_trap(pattern_buffer);
	set_trap_value(pattern_buffer, which);
}

void activate_got_item_trap(int which)
{
	activate_trap(got_item);
	set_trap_value(got_item, which);
}

void activate_light_activated_trap(int which)
{
	activate_trap(light_activated);
	set_trap_value(light_activated, which);
}

void activate_platform_activated_trap(int which)
{
	activate_trap(platform_activated);
	set_trap_value(platform_activated, which);
}
