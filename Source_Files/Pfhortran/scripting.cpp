/* script.c
   Monday, Febuary 14th, 2000 (Chris Pruett)
   
	Feb 14th, 10:58 AM	- Got get_tokens working.  Lexical analizer is good to go.
	Feb 15th		   	- Basic instructions working! Yay!
	Feb 21st			- Scripts can turn tags on and off.  Basic variable management works (add, subtract, set)
						branching kind of works, but its kind of lame.  Needs more work.
	
Aug 15, 2000 (Loren Petrich):
	Changed file handler to use my new object-oriented version
 */
 
#define MAX_VARS 64	//The max number of script variables allowed in a single script
#define MAX_DEPTH 256	//The max levels deep function calls are allowed to get



#include "script_instructions.h"
#include "script_parser.h"


#include "cseries.h"
#include "tags.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"

// LP addition:
#include "FileHandler_Mac.h"

#include <Dialogs.h>
#include <TextUtils.h>
#include <Strings.h>
#include <Resources.h>
#include <Sound.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>



script_instruction *current_script;
int current_instruction;


float variable_lookup[MAX_VARS];
int variable_count;

int script_stack[MAX_DEPTH];
int stack_top = 0;
bool is_startup;

short camera_count;
short current_camera;
script_camera *cameras;

short path_count;
short current_path;
path_header *script_paths;
short current_path_point;
path_list camera_point;


short current_trap;


extern bool s_camera_Control;
extern void (*instruction_lookup[NUMBER_OF_INSTRUCTIONS])(script_instruction);


int get_next_instruction(void);
void execute_instruction(int inst);
bool script_in_use(void);
void do_next_instruction(void);
bool instruction_finished(void);
 

 
 
 

 
void clean_up_script(void)
{
	if (current_script)
		free(current_script);
		
	current_script = NULL;
	
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

	clear_bind_table();

}
 
/*load_script loads the source script from a TEXT ID in the map resource fork,
initalizes lineCount to point to the beginning of every line, and initalizes
all script variables to zero*/
int load_script(int text_id)
{
	// FileDesc *cur_map;
	// FileError error;
	// fileref file_id;
	OSErr error;
	short file_id;
	Handle textHand;
	int app;
	int linecount;
	char *src;
	int x;

	if (!is_pfhortran_on())	/* we can't do too much if the pfhortran isn't running */
		return false;
	
	// LP changes:
	FileObject& cur_map = get_map_file();
	// cur_map = get_map_file();

	error= FSpOpenRF(&GetSpec(cur_map), fsRdPerm, &file_id);
	// error= FSpOpenRF((FSSpec *)cur_map, fsRdPerm, &file_id);

	if (error)
	{
		file_id= NONE;
		set_game_error(systemError, error);
		return script_ERROR;
		
	}

	app= CurResFile();

	UseResFile(file_id);

	textHand = Get1Resource('TEXT', text_id);

	if ((textHand) && (!*textHand))
		LoadResource(textHand);
		
	UseResFile(app);

	clean_up_script();
		


	if (textHand == NULL)
		current_script=0;
	else
	{	
		src=(char *)(*textHand);
		src[GetHandleSize(textHand)] = 0;
		
		current_script = parse_script(src);
		
		ReleaseResource(textHand);
		
		src = NULL; 
		
		current_instruction = 0;
		/*instruction_decay = 0;*/
		
		
		for (x=0;x < MAX_VARS;x++)
			variable_lookup[x] = 0;
		variable_count = 0;
		
	}

	if (file_id != app)
		CloseResFile(file_id);
		

	is_startup = false;
		 	
	 	
	return true;
}

void script_init(void)
{
	int init_start, old_start;

	if (!script_in_use())
		return;

	s_camera_Control = false;
	
	/*old_start = current_instruction;
	
	init_start = get_next_instruction();

	current_instruction = old_start;
	
	if (current_script[init_start].opcode == On_Init)
	{
		do
			do_next_instruction();
		while (is_startup && script_in_use());

	}*/
	
	activate_trap(init);
	
	if (trap_active(init))
	{
		current_trap = init;
		is_startup = true;
		
		do
			do_next_instruction();
		while (trap_active(init) && script_in_use());
	
		is_startup = false;;
	}
	
	activate_trap(idle);	/* start the idle script running */

}
 
/*do_next_instruction gets the next instruction and executes it*/
void do_next_instruction(void)
{
	
	if (is_startup)	/* on startup we want to execute ONLY the startup script */
	{
		if (trap_active(current_trap))
			{
				current_instruction = get_trap_offset(current_trap);
				execute_instruction(get_next_instruction());
				
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
				
				
				execute_instruction(get_next_instruction());
				
				if (trap_active(current_trap))
					set_trap_instruction(current_trap, current_instruction);
				else if (current_trap == idle)
				{
					reset_trap(idle);
					activate_trap(idle);
				}
			}
	}

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
void add_variable(int var)
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
	/*if (current_script)
		free(current_script);
	current_script = 0;
	current_instruction = 0;*/
	
	clean_up_script();
	
}

/*script_in_use returns TRUE of there 
is currently a script loaded, FALSE if there is not.*/
bool script_in_use(void)
{
	return (current_script);
}


/*instruction_finished returns TRUE of the last instuction is done, FALSE if it's not.*/

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
void execute_instruction(int inst)
{
	if (current_script[inst].opcode == 0 || current_script[inst].opcode > NUMBER_OF_INSTRUCTIONS)
		return;
	
	
	(*instruction_lookup[current_script[inst].opcode])(current_script[inst]);	//call the function

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
 
#pragma mark -

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
 
 
 
 
 
