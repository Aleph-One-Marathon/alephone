/* script_parser.h */

/*
June 13, 2001 (Loren Petrich): 
	Added script-length output to script parser
*/


#ifndef _SCRIPT_PARSER_DEF
#define _SCRIPT_PARSER_DEF

#include "cstypes.h"

enum /* procedure traps */
{
	idle = 1,
	init,
	tag_switch,
	light_switch,
	platform_switch,
	terminal_enter,
	terminal_exit,
	pattern_buffer,
	got_item,
	
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
	int opcode; 	/* the instruction index */
	short mode;
	float op1;
	float op2;
	float op3;
};



bool init_pfhortran(void);
void dispose_pfhortran(void);
bool is_pfhortran_on(void);
script_instruction *parse_script(char *input, int *length_ptr);

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
