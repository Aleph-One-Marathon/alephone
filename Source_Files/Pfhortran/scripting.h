/* script.h
   Monday, Febuary 14th, 2000 (Chris Pruett)

Oct 14, 2000 (Loren Petrich)
	Added function for loading a script from some in-memory data;
	this is an alternative to separate functions for loading from a resource or a file  
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
int load_script_data(void *Data, int Len);
 
void script_init(void);
 
bool script_in_use(void);
 
/*bool instruction_finished(void);*/

void do_next_instruction(void);
  
bool script_Camera_Active(void);
 
void activate_tag_switch_trap(int which);
void activate_light_switch_trap(int which);
void activate_platform_switch_trap(int which);
void activate_terminal_enter_trap(int which);
void activate_terminal_exit_trap(int which);
void activate_pattern_buffer_trap(int which);
void activate_got_item_trap(int which);
 
#endif
