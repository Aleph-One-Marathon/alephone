/* script_parser.c

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

4/14/2000 - Created by Chris Pruett

10/29/00 - Mark Levin
	Change error code of evalute_operand to -32767 from -1 to avoid conflicts with -1 
	appearing in scripts

June 13, 2001 (Loren Petrich): 
	Added script-length output to script parser

March 11, 2002 (Br'fin (Jeremy Parsons)):
	Rewrote to use FileHandler classes instead of fopen and company
	SDL has embedded language definition info
	Defaults to Carbon private resources directory if no language def in app directory
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cseries.h"

#include "script_parser.h"
#include "script_instructions.h"
#include "FileHandler.h"
#include <vector>
using namespace std;

// The max number of 'blats', string units, that a single line can contain
// label: instruction op1, op2, op3  == 5 blats

#define MAXBLATS 5

// The name of the Pfhortran Language Def File.  Path could be included here.

#define LANGDEFPATH "Pfhortran Language Definition"

enum /* symbol modes */
{absolute, variable};

/* Instruction Modes:
	m = 0: x , y and z are absolute values
	m = 1: x is a variable (y is a value, z is a value)
	m = 2: y is a variable (x is a value, z is a value)
	m = 3: x and y are both variables (z is a value)
	m = 4: z is a variable (x and y are values)
	m = 5: z and x are variables (y is a value)
	m = 6: z and y are variables (x is a value)
	m = 7: x, y, and z are all variables
*/

enum /* errors */
{
	kInvalid_Instruction = 1,
	kInvalid_Directive,
	kInvalid_Operator,
	kSymbol_Not_Found,
	kInvalid_Mode,
	kToo_Few_Arguments,
	kBad_Arguments,
	
	NUMBER_OF_ERRORS

};


struct symbol_def
{
	char *symbol;
	float val;
	short mode;
	struct symbol_def *next;

};

struct error_def
{
	short error;
	short line;
};


/* Global Variables */


symbol_def **glob_hash;
symbol_def **instruction_hash;
bool pfhortran_is_on;
short error_count;

bind_table procedure_bindings[NUMBER_OF_TRAPS];	/*Binding table for procedure traps */

/* Local Function Prototypes */

symbol_def *get_hash(short key, symbol_def **the_hash);
void init_hash();

void dispose_hash();

void lowercase_string(char *string);
short put_symbol(char *symbol, float val, short mode, symbol_def **the_hash);
void dispose_pfhortran(void);

/* Pfhortran Language Defination Init/Parsing Code

This code opens up the Pfhortran Language Defination File and parses it so that 
scripts can be parsed.  Each defination is added to the instruction_hash so that it can
be identified later.  File manip is done with standard ansi C calls for compatability
purposes.  These init/dispose functions are only called once at the application's startup
and shutdown.

*/

bool init_pfhortran(void)
{	
	pfhortran_is_on = false;	// In case we have some errors

	// init the instruction hash
	instruction_hash = (symbol_def **)malloc(sizeof(symbol_def *) * 256);
	memset(instruction_hash,0,sizeof(symbol_def *) * 256);
	if (!instruction_hash)
		return false;

	bool err = false;

	// LP change: made this code always read the language-definition file
// #ifdef SDL
#if 1
	// Read tokens from array
	static const struct {
		const char *str;
		int val;
	} tokens[] = {
#include "language_definition.h"
		NULL, 0
	};

	for (int i=0; tokens[i].str != NULL; i++) {
		char str[256];
		strcpy(str, tokens[i].str);	// string gets modified by lowercase_string()
		if (!put_symbol(str, (float)tokens[i].val, absolute, instruction_hash)) {
			err = true;
			break;
		}
	}
	
#else
		
	// Read tokens from language definition file
	FileSpecifier FileSpec;

	FileSpec.SetToApp();
	FileSpec.SetName(LANGDEFPATH, _typecode_theme);
#if defined(TARGET_API_MAC_CARBON)
	if(!FileSpec.Exists())
	{
		FileSpec.SetParentToResources();
		FileSpec.SetName(LANGDEFPATH, _typecode_theme);
	}
#endif
	
	OpenedFile OFile;
	if (FileSpec.Open(OFile))
	{
		long Len = 0;
		OFile.GetLength(Len);
		if (Len <= 0) {
			dispose_pfhortran();
			return false;
		}
		
		vector<char> FileContents(Len);
		if (!OFile.Read(Len,&FileContents[0])) {
			dispose_pfhortran();
			return false;
		}
		
		// Entire buffer is now in FileContents vector
		char *index = &FileContents[0];

		char input_str[256];
		int input_val;
		while (sscanf(index, "%s %x\n", input_str, &input_val) != EOF)
		{
			// Skip past line
			while((*index != '\n' && *index != '\r')&& (index - &FileContents[0] < Len))
				index++;
			// Skip over trailing carriage return
			if(*index == '\r') index++;
			if(*index == '\n') index++;
			
			if (input_str && input_str[0] != '#')
			{
				lowercase_string(input_str);
				
				if (!put_symbol(input_str,(float)input_val,absolute,instruction_hash))
				{
					err = true;
					break;
				}
			
			}
			
			input_str[0] = 0;
		
		}
	}
#endif
	
	if (err)
	{
		dispose_pfhortran();
		return false;
	}
	
	pfhortran_is_on = true;
	
	init_instructions();
	
	return true;
}

void dispose_pfhortran(void)
{
	int x;
	symbol_def *temp, *old;	
	for (x = 0;x < 256;x++)
		if ((temp = get_hash(x,instruction_hash)) != NULL)
		{
			while ((old = temp) != NULL)
			{	
				temp = temp->next;
				free(old);
			}
		}


	free(instruction_hash);

}

bool is_pfhortran_on(void)
{
	return pfhortran_is_on;
}


/* Binding Table Code

The Bind Table is an array of "traps": offsets to functions that are "bound" to specific game
events.  For example, the platform_switch trap, if defined by the script's author, will be
called every time a platform switch is toggled.  The which variable contains which platform
has changed, so each procedure can act on demand.  The Bind Table is cleared every time a map
is reloaded.

*/

void clear_bind_table(void)
{
	memset(procedure_bindings,0,sizeof(procedure_bindings));
}

void add_trap(short trap, int start_offset)
{
	if (trap < NUMBER_OF_TRAPS)
	{
		procedure_bindings[trap].current_offset = start_offset;
		procedure_bindings[trap].start_offset = start_offset;
		procedure_bindings[trap].active = false;
		procedure_bindings[trap].which = 0;
		procedure_bindings[trap].available = true;
		procedure_bindings[trap].instruction_decay = 0;
	}

}

bool trap_active(short trap)
{
	return (procedure_bindings[trap].available && procedure_bindings[trap].active);
}

int get_trap_offset(short trap)
{
	return procedure_bindings[trap].current_offset;
}

int get_trap_start(short trap)
{
	return procedure_bindings[trap].start_offset;
}

int get_trap_value(short trap)
{
	return procedure_bindings[trap].which;
}

void set_trap_value(short trap, int val)
{
	procedure_bindings[trap].which = val;
}

bind_table *get_bind_table(void)
{
	return procedure_bindings;
}

void set_trap_instruction(short trap, int offset)
{
	procedure_bindings[trap].current_offset = offset;
}

void reset_trap(short trap)
{
	procedure_bindings[trap].current_offset = procedure_bindings[trap].start_offset;
	procedure_bindings[trap].active = false;
	procedure_bindings[trap].instruction_decay = 0;
}

void activate_trap(short trap)
{
	if (procedure_bindings[trap].available)
		procedure_bindings[trap].active = true;
}

uint32 get_trap_instruction_decay(short trap)
{
	return procedure_bindings[trap].instruction_decay;
}

void set_trap_instruction_decay(short trap, uint32 decay)
{
	procedure_bindings[trap].instruction_decay = decay;
}




// Suppressed for MSVC compatibility
#if 0
#pragma mark -
#endif

/* Symbol Table Code 

The symbol table is a hash containing definations of all the symbols (labels, variables, etc)
referenced in a single script.  The symbol table is necessary because it allows us to 
do relative jumps and variable management easily.

The hash is an array of 256 pointers to symbol_def structs.  The hash is a way of storing
and retreaving data in O(1).  It works by generating a key for each symbol, and using
that key as an index to the hash array.  In the event of a collision, where two or more
symbols share the same key, a linked list called a chain is built off of that index 
location.  To facilitate this, the symbol_def struct has a next pointer to go to the
next symbol in the chain.

Pfhortran uses the Mid Square method of generating hash keys.  The method is fairly simple,
but produces results good enough for our purposes.  It works by addding up all the ascii
values in a symbol, squareing the result, and taking the middle 8 bits from that new number.



*/

void init_hash()
{
	glob_hash = (symbol_def **)malloc(sizeof(symbol_def *) * 256);
	memset(glob_hash,0,sizeof(symbol_def *) * 256);
	
}

void clear_hash()
{

	int x;
	symbol_def *temp, *old;	
	for (x = 0;x < 256;x++)
		if ((temp = get_hash(x,glob_hash)) != NULL)
		{
			while ((old = temp) != NULL)
			{	
				temp = temp->next;
				free(old);
			}
		}
		
		
	memset(glob_hash,0,sizeof(symbol_def *) * 256);

}

// we need to dispose all the contents of the hash as well...
//cleaned up by Mark Levin

void dispose_hash()
{
	int x;
	symbol_def *temp, *old;	
	for (x = 0;x < 256;x++)
		if ((temp = get_hash(x,glob_hash)) != NULL)
		{
			while ((old = temp) != NULL)
			{	
				temp = temp->next;
				free(old);
			}
		}


	free(glob_hash);
}




symbol_def *get_hash(short key, symbol_def **the_hash)
{
	return the_hash[key];
}

void add_hash(symbol_def *symbol, short key, symbol_def **the_hash)
{
	symbol->next = the_hash[key];
	the_hash[key] = symbol;
}

unsigned char calculate_hash_key(char *symbol)
{
	int key = 0;
	int x;
	
	for (x = 0;symbol[x];x++,key += symbol[x]);	/*sum up the ascii values */

	key *= key;
	
	key <<= 12;
	
	key >>= 12 + 12;
	
	return (unsigned char)key;

	//return (0x1FF & ((key * key) >> 10));			/* return the middle 8 bits */

}

void lowercase_string(char *string)
{
	unsigned int x;

	for (x = 0; x < strlen(string); x++)
		if (string[x] >= 65 && string[x] <= 90) 
			string[x] += 32;
}

symbol_def *get_symbol(char *symbol, symbol_def **the_hash)
{
	
	unsigned char key;					/* the key for the symbol */
	symbol_def *temp;					/* a temp pointer we will use to find and return our data */
	
	temp = NULL;						/* init to null */
	
	lowercase_string(symbol);
	
	key = calculate_hash_key(symbol);	/* get the hash value for pszLabel */
	
	temp = get_hash(key,the_hash);		 /* go get any records at location key in the hash */
	
	if (!temp)							/* if there were none, return NULL */
		return NULL;
		
										/* if we found a record, follow the chain (if available) until we find
										the symbol we are looking for */
		
	while (temp != NULL && (strcmp(temp->symbol,symbol) != 0) )
		temp = temp->next;
	
										/* return it (symbol will be NULL if it can't be found) */
	return temp;	

}


short put_symbol(char *symbol, float val, short mode, symbol_def **the_hash)
{
	symbol_def *new_symbol;
	
	lowercase_string(symbol);
	
	new_symbol = (symbol_def *)malloc(sizeof(symbol_def));
	
	if (!new_symbol)
		return false;
	
	new_symbol->symbol = (char *)malloc(strlen(symbol) + 1);
	
	if (!new_symbol->symbol)
		return false;
	
	strcpy(new_symbol->symbol, symbol);
	
	new_symbol->val = val;
	
	new_symbol->mode = mode;
	
	add_hash(new_symbol, calculate_hash_key(symbol), the_hash);
	
	return true;

}

// Suppressed for MSVC compatibility
#if 0
#pragma mark -
#endif

/* Parsing code 

To parse each script we do the following:
	
Pass 1 - Collect Symbols & Count Lines

	1) Read in the next line
	2) Parse the new line for labels
	3) Add labels where necessary
	4) Increment line_count
	
Pass 2 - Build Instruction List

	1) Init an array of instructions line_count lines long
	2) Read in the line

	For every line that contains an instruction...
		
		a) Parse the new line and figure out the instruction
		b) Parse any operands
		c) Build new instruction defination, add to list
	
	For every line that contains a directive...
	
		a) Parse the new line and figure out the directive
		b) Perform the directive
	
	3) Finally, our instuction list is built, so dispose the actual strings used.
	4) Dispose other structs, such as symbol table hash
	5) Directives should have built a binding list, so we are ready to execute.


For this code I have chosen to stay away from dynamic memory allocation for speed and stability
reasons.  Malloc and Mac Memory are not the best of friends. :)

*/

void get_blats(char *input, short max_blats, char blats[MAXBLATS][64])	/* returns an array of strings */
{
	char temp[256];
	char *nexttok;
	//char *blats[MAXBLATS];
	char *temp_char;
	short x;
	
		
	for (x = 0; x < MAXBLATS; x++)
		blats[x][0] = 0;
		
	if (!input)
		return;
	
	//temp = (char *)malloc(strlen(input) + 1);
	
	//if (!temp)
		//return;
		
	
	/*blats = (char **)malloc(sizeof(char *) * max_blats);
	
	
	if (!blats)
		return NULL; */
		
	//memset(blats,0,sizeof(blats));

	
	strcpy(temp, input);
	
	if ((temp_char = strchr(temp, '#')) != NULL)
		*temp_char = 0;
	
	if (temp[0] == 0)
	{
		//free(temp);
		//free(blats);
		return;
	}
	
	
	if (strchr(temp, ':'))
		nexttok = strtok(temp, ":");
	else 
		nexttok = NULL;
	
	if (nexttok) 
	{
		//blats[0] = (char *)malloc(strlen(nexttok) + 1);
		for(;nexttok[0] < 'A' || nexttok[0] > 'z';nexttok++);
		strcpy(blats[0], nexttok);
	} else
		blats[0][0] = 0;
		
	
	for (x = 1; x < max_blats; x++)
	{
		if (!nexttok)
			nexttok = strtok(temp, " ,\t");
		else
			nexttok = strtok(NULL, " ,\t");
		
		if (nexttok) 
		{
			//blats[x] = (char *)malloc(strlen(nexttok) + 1);
			strcpy(blats[x], nexttok);
		} else
			blats[x][0] = 0;
		
		
		if (!nexttok)
			break;
	}

	//free(temp);
	
}

void read_line(char *input, char output[256])
{
	//char *output;
	int x,y;

	if (strlen(input) == 0 || *(input-1) == 0x11)
		{
		output[0] = (char)EOF;
		return;
		}
	
	for (x=0;(input[x] != '\r' && input[x] != '\n' && input[x] != 0 && input[x] != 0x11);x++);
	
	//output = (char *)malloc(x + 1);
	
	for (y=0;y < x;y++)
		output[y]=input[y];
		
	
	output[y] = 0;
	
	//return output;
}

short match_opcode(char *input)			/* the instruction strings are held in the hash as well */
{
	symbol_def *instruction = NULL;
	
	if (!input)
		return 0;
		
	instruction = get_symbol(input,instruction_hash);
	
	if (!instruction)
		return 0;
	else
		return short(instruction->val);
}


float evaluate_operand(char *input, short *mode)
{
	symbol_def *symbol = NULL;
	
	*mode = absolute;
	
	if (!input || (input && input[0]) == '#' || (input[0] == 0))	// start of a comment?
		return -32767;
	
	symbol = get_symbol(input, glob_hash);
	
	if (!symbol)
		symbol = get_symbol(input, instruction_hash);
	
	if (!symbol)
	{
		char endChar = '\0';
		char *endCharPtr = &endChar;
		double val = 0;
		float fval;
		
		val = strtod(input, &endCharPtr);
		
		*mode = absolute;
		
		fval = (float)val;
		return fval;
	
	} else
	{
		*mode = symbol->mode;
		return symbol->val;
	}
	
}

void add_error(error_def *error_log, short error, int offset)
{
	
	error_log[error_count].error = error;
	error_log[error_count].line = offset+1;
	
	error_count++;
}

void report_errors(error_def *error_log, int length)
{
	int x;
	char error_string[64];
	FileSpecifier FileSpec;
	OpenedFile OFile;
	
	if (error_count <= 0)
		return;
		
	for (x = 0;x < length;x++)
	{
		error_string[0] = 0;
		
		switch (error_log[x].error)
		{
			case kInvalid_Instruction:
				sprintf(error_string,"Invalid Instruction Error, Line %d", error_log[x].line);
				break;
			case kInvalid_Directive:
				sprintf(error_string,"Invalid Directive Error, Line %d", error_log[x].line);
				break;
			case kInvalid_Operator:
				sprintf(error_string,"Invalid Operator Error, Line %d", error_log[x].line);
				break;
			case kSymbol_Not_Found:
				sprintf(error_string,"Unknown Symbol Error, Line %d", error_log[x].line);
				break;
			case kInvalid_Mode:
				sprintf(error_string,"Invalid Mode Error, Line %d", error_log[x].line);
				break;
			case kToo_Few_Arguments:
				sprintf(error_string,"Too Few Arguments Error, Line %d", error_log[x].line);
				break;
			case kBad_Arguments:
				sprintf(error_string,"Bad Arguments Error, Line %d", error_log[x].line);
				break;
		}
		
		if (error_string[0])
		{
			if (!OFile.IsOpen())
			{
#if defined(mac)
				FileSpec.SetToApp();
				FileSpec.SetName("Pfhortran Error Report", _typecode_theme);
#else
#warning JTP: An SDL Guru would probably wish to rework this
				FileSpec.SetToPreferencesDir();
				FileSpec.AddPart("pfhortran_error_report.txt");
#endif

				if(!FileSpec.Exists())
					FileSpec.Create(_typecode_theme);
					
				if (FileSpec.Open(OFile, true))
				{
					long length;
					if(OFile.GetLength(length))
					{
						OFile.SetPosition(length);
					}
				}
				else
					return;
			}
			
			strcat(error_string, "\n");
			OFile.Write(strlen(error_string), error_string);
		}
	
	}
}



script_instruction *parse_script(char *input, int *length_ptr)
{
	bool done = false;
	char current_line[256];
	int offset = 0;
	//char **blats = NULL;
	char blats[MAXBLATS][64];
	
	int line_count = 0;
	// int total_lines;
	script_instruction *instruction_list;
	int mode;
	short op1mode,op2mode,op3mode;
	int var_count = 0;
	float val;
	
	bool output_errors = false;
	error_def *error_log=0;
	
	init_hash();
	clear_bind_table();
	
	error_count = 0;
	
	while (!done)
	{
		read_line(input + offset, current_line);
		
		if (current_line[0] == (char)EOF)
			done = true;
		else
		{
			
			
			offset += strlen(current_line) + 1;
			
			//blats = get_blats(current_line, MAXBLATS);
			get_blats(current_line, MAXBLATS, blats);
			
			
			mode = absolute;
			val = line_count;
			
			if (blats[0][0])
			{
				if (blats[1][0])
					if (match_opcode(blats[1]) == Define)
					{
						mode = variable;
						val = var_count;
						var_count++;
					}
					
				put_symbol(blats[0],val,mode,glob_hash);
			}
				
			if (blats[1][0] && blats[1][0]!= '_' )	/* underscore denotes compiler directive */
				line_count ++;
		
							
		}
	}

	/* done with pass 1, on to pass 2 */
	
	done = false;
	offset = 0;
	
	*length_ptr = line_count;
	// total_lines = line_count;
	
	instruction_list = (script_instruction *)malloc(sizeof(script_instruction) * line_count);
	error_log = (error_def *)malloc(sizeof(error_def) * line_count);
	
	memset(error_log,0,sizeof(error_def) * line_count);
	
	
	line_count = 0;
	
	while (!done)
	{
		read_line(input + offset, current_line);
		
		if (current_line[0] == (char)EOF)
			done = true;
		else
		{
			offset += strlen(current_line) + 1;
			
			get_blats(current_line, MAXBLATS, blats);
		
			//do it!
			if (blats[1][0] && blats[1][0] != '_' ) /* line is an instruction... decypher it */
			{
			
				instruction_list[line_count].op1 = 0;
				instruction_list[line_count].op2 = 0;
				instruction_list[line_count].op3 = 0;
				op1mode = absolute;
				op2mode = absolute;
				op3mode = absolute;
				
				
				if ((instruction_list[line_count].opcode = match_opcode(blats[1])) == 0)
					add_error(error_log,kInvalid_Instruction,line_count);
				
				
				
				if ((instruction_list[line_count].op1 = evaluate_operand(blats[2], &op1mode)) != -32767)
					if ((instruction_list[line_count].op2 = evaluate_operand(blats[3], &op2mode)) != -32767)
						instruction_list[line_count].op3 = evaluate_operand(blats[4], &op3mode);
				
				if (op1mode == absolute && op2mode == absolute && op3mode == absolute)
					instruction_list[line_count].mode = 0;
				else if (op1mode == variable && op2mode == absolute && op3mode == absolute)
					instruction_list[line_count].mode = 1;
				else if (op1mode == absolute && op2mode == variable && op3mode == absolute)
					instruction_list[line_count].mode = 2;
				else if (op1mode == variable && op2mode == variable && op3mode == absolute)
					instruction_list[line_count].mode = 3;
				else if (op1mode == absolute && op2mode == absolute && op3mode == variable)
					instruction_list[line_count].mode = 4;
				else if (op1mode == variable && op2mode == absolute && op3mode == variable)
					instruction_list[line_count].mode = 5;
				else if (op1mode == absolute && op2mode == variable && op3mode == variable)
					instruction_list[line_count].mode = 6;
				else if (op1mode == variable && op2mode == variable && op3mode == variable)
					instruction_list[line_count].mode = 7;
				else
				{
					instruction_list[line_count].mode = -1;	/* something is horribly wrong */
					add_error(error_log,kInvalid_Mode,line_count);
				}
				
				
				line_count ++;
			} else if (blats[1][0] && blats[1][0] == '_' ) /* line is a directive... decypher it */
			{
				lowercase_string(blats[1]);
				
				if (!strcmp(blats[1],"_procedure"))	/* procedure def */
				{
					if (blats[2])
					{
						lowercase_string(blats[2]);
						if (!strcmp(blats[2],"load"))
						{
							add_trap(load,line_count);
						} else if (!strcmp(blats[2],"init"))
						{
							add_trap(init,line_count);
						} else if (!strcmp(blats[2],"idle"))
						{
							add_trap(idle,line_count);
						} else if (!strcmp(blats[2],"tag_switch"))
						{
							add_trap(tag_switch,line_count);
						} else if (!strcmp(blats[2],"light_switch"))
						{
							add_trap(light_switch,line_count);
						} else if (!strcmp(blats[2],"platform_switch"))
						{
							add_trap(platform_switch,line_count);
						} else if (!strcmp(blats[2],"terminal_enter"))
						{
							add_trap(terminal_enter,line_count);
						} else if (!strcmp(blats[2],"terminal_exit"))
						{
							add_trap(terminal_exit,line_count);
						} else if (!strcmp(blats[2],"pattern_buffer"))
						{
							add_trap(pattern_buffer,line_count);
						} else if (!strcmp(blats[2],"got_item"))
						{
							add_trap(got_item,line_count);
						
						} else 
							add_error(error_log,kBad_Arguments,line_count);
						
					
					} else
						add_error(error_log,kToo_Few_Arguments,line_count);
						
				} else if (!strcmp(blats[1],"_report_errors"))	/* turn errors on/off */
				{
					if (blats[2])
					{
						lowercase_string(blats[2]);
						if (!strcmp(blats[2],"true"))
						{
							output_errors = true;
							
						} else if (!strcmp(blats[2],"false"))
						{
							output_errors = false;
						} else 
							add_error(error_log,kBad_Arguments,line_count);
						
						
					
					} else
						add_error(error_log,kToo_Few_Arguments,line_count);
						
				} else
					add_error(error_log,kInvalid_Directive,line_count);
			
			}
						
			
		}
	}
	
#if 0
	if (procedure_bindings[idle].available == false)
		add_trap(idle,0);	/* always have an idle event, even if not specifically defined */
	
	if (procedure_bindings[init].available == true)
		procedure_bindings[init].active == true;	/* set up any init events */
#endif
	
	dispose_hash();
	
	if (output_errors)
		report_errors(error_log, *length_ptr);
		// report_errors(error_log, total_lines);
	
	free(error_log);
	
	return instruction_list;

}

