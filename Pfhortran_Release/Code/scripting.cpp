/* script.c
   Monday, Febuary 14th, 2000 (Chris Pruett)
   
	Feb 14th, 10:58 AM	- Got get_tokens working.  Lexical analizer is good to go.
	Feb 15th		   	- Basic instructions working! Yay!
	Feb 21st			- Scripts can turn tags on and off.  Basic variable management works (add, subtract, set)
						branching kind of works, but its kind of lame.  Needs more work.
	
 */
 
#define MAX_VARS 64	//The max number of script variables allowed in a single script
#define MAX_DEPTH 256	//The max levels deep function calls are allowed to get
#include "script_instructions.h"
// #include "script.h"


#include "cseries.h"
#include "tags.h"
#include "map.h"
#include "interface.h"
#include "game_wad.h"
#include "game_errors.h"


#include <Dialogs.h>
#include <TextUtils.h>
#include <Strings.h>
#include <Resources.h>
#include <Sound.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>



char *current_script;
char line[256];
int line_tokens[12];
script_instruction current_instruction;
int current_offset;
int lineStart[256];

long instruction_decay;
void (*instruction_lookup[NUMBER_OF_INSTRUCTIONS])(script_instruction);

float variable_lookup[MAX_VARS];
int variable_count;

int script_stack[MAX_DEPTH];
int stack_top = 0;
bool is_startup;

short camera_count;
short current_camera;
script_camera *cameras;

extern bool s_camera_Control;


/* local functions */
int get_tokens (char *pszSrc,int *prgiw);
int is_end(char *pszSrc, int istart);
int next_blat(char *pszSrc, int *pistart, int *pwlength, int fcomma);
void get_blat(char *src, char *dest, int istart);

script_instruction get_next_instruction(void);
void execute_instruction(script_instruction inst);
bool script_in_use(void);
void do_next_instruction(void);

 

 
 
 
/*init_instructions sets up the instruction_lookup array so that
functions can be called by casting back the pointer.  This is only
called once.*/
void init_instructions(void)
{


	instruction_lookup[Camera_Move] = s_Camera_Move;
	instruction_lookup[Camera_Look] = s_Camera_Look;
	instruction_lookup[Wait_Ticks] = s_Wait_Ticks;
	instruction_lookup[Inflict_Dammage] = s_Inflict_Dammage;
	instruction_lookup[Jump] = s_Jump;
	instruction_lookup[Enable_Player] = s_Enable_Player;
	instruction_lookup[Disable_Player] = s_Disable_Player;
	instruction_lookup[Script_End] = s_Script_End;
	instruction_lookup[Hide_Interface] = s_Hide_Interface;
	instruction_lookup[Show_Interface] = s_Show_Interface;
	instruction_lookup[Set_Tag_State] = s_Set_Tag_State;
	instruction_lookup[Get_Tag_State] = s_Get_Tag_State;
	instruction_lookup[Define] = s_Define;
	instruction_lookup[sAdd] = s_sAdd;
	instruction_lookup[sSubtract] = s_sSubtract;
	instruction_lookup[If_Equal] = s_If_Equal;
	instruction_lookup[Set] = s_Set;
	instruction_lookup[Call] = s_Call;
	instruction_lookup[Return] = s_Return;
	instruction_lookup[If_Greater] = s_If_Greater;
	instruction_lookup[If_Less] = s_If_Less;
	instruction_lookup[If_Not_Equal] = s_If_Not_Equal;
	instruction_lookup[Get_Life] = s_Get_Life;
	instruction_lookup[Set_Life] = s_Set_Life;
	instruction_lookup[Get_Oxygen] = s_Get_Oxygen;
	instruction_lookup[Set_Oxygen] = s_Set_Oxygen;
	instruction_lookup[On_Init] = s_On_Init;
	instruction_lookup[End_Init] = s_End_Init;
	instruction_lookup[Add_Item] = s_Add_Item;
	instruction_lookup[Select_Weapon] = s_Select_Weapon;
	instruction_lookup[Block_Start] = s_Block_Start;
	instruction_lookup[Block_End] = s_Block_End;
	instruction_lookup[Init_Cameras] = s_Init_Cameras;
	instruction_lookup[Select_Camera] = s_Select_Camera;
	instruction_lookup[Set_Camera_Poly] = s_Set_Camera_Poly;
	instruction_lookup[Set_Camera_Pos] = s_Set_Camera_Pos;
	instruction_lookup[Set_Camera_YP] = s_Set_Camera_YP;
	instruction_lookup[Activate_Camera] = s_Activate_Camera;
	instruction_lookup[Use_Camera] = s_Use_Camera;


}
 
void clean_up_script(void)
{
	if (current_script)
		free(current_script);
	
	if (cameras)
		free(cameras);
	
	cameras = NULL;
	
	camera_count = 0;
	current_camera = 0;

}
 
/*load_script loads the source script from a TEXT ID in the map resource fork,
initalizes lineCount to point to the beginning of every line, and initalizes
all script variables to zero*/
int load_script(int text_id)
{
	FileDesc *cur_map;
	FileError error;
	fileref file_id;
	Handle textHand;
	int app;
	int linecount;
	char *src;
	int x;



	cur_map = get_map_file();

	error= FSpOpenRF((FSSpec *)cur_map, fsRdPerm, &file_id);

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

	clean_up_script;
		


	if (textHand == NULL)
		current_script=0;
	else
	{	
		src=(char *)(*textHand);
		
		current_script= (char *)malloc((strlen(src)+1)*sizeof(char));
		strcpy(current_script,src);
		
		ReleaseResource(textHand);
		
		//current_line = 0;
		current_offset = 0;
		instruction_decay = 0;
		
		linecount = 2;
		lineStart[1]=0;
		for (x=0;x<strlen(current_script);x++)
		{
			if (current_script[x] == '\r' || current_script[x] == '\n')
			{
				lineStart[linecount] = x+1;
				linecount++;
			}
		
		}
		
		
		if (instruction_lookup[Script_End] != s_Script_End)
			init_instructions();
		
		for (x=0;x < MAX_VARS;x++)
			variable_lookup[x] = 0;
		variable_count = 0;
		
	}

	if (file_id != app)
		CloseResFile(file_id);
		

	is_startup = script_FALSE;
		 	
	 	
	return script_TRUE;
}

void script_init(void)
{
	int old_offset;

	if (!script_in_use())
		return;

	s_camera_Control = false;
	
	old_offset = current_offset;

	current_instruction = get_next_instruction();
		
	current_offset = old_offset;

	if (current_instruction.opcode == On_Init)
	{
		do
			do_next_instruction();
		while (is_startup && script_in_use());

	}

}
 
/*do_next_instruction gets the next instruction and executes it*/
void do_next_instruction(void)
{
  
	current_instruction = get_next_instruction();
	execute_instruction(current_instruction);

}


/*set_instruction_decay sets the ammount of game ticks to wait until the instruction
is over*/
void set_instruction_decay(long decay)
{
	instruction_decay = decay;
}

/*jump_to_line sets the cursor to point at the beginning of the line
passed by newline*/
void jump_to_line(int newline)
{
	current_offset = lineStart[newline];
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
	if (current_script)
		free(current_script);
	current_script = 0;
	
}

/*script_in_use returns TRUE of there 
is currently a script loaded, FALSE if there is not.*/
bool script_in_use(void)
{
	return (current_script != 0);
}


/*instruction_finished returns TRUE of the last instuction is done, FALSE if it's not.*/

bool instruction_finished(void)
{
	return (machine_tick_count() > instruction_decay);
}


/*get_next_line returns the next line in the source string.*/

void get_next_line(void)
{
	int x;

	for (x=current_offset;(!is_end(current_script,x) && current_script[x] != '#');x++)
		line[x-current_offset] = current_script[x];

	line[x-current_offset]=0;

	current_offset = x+1;

	//current_line++;

}



/*get_next_instruction gets the next line of the source string and
parses it into a usable data structure.
*/
script_instruction get_next_instruction(void)
{
	script_instruction newInst;
	int tokens[12];
	char *opstring;
	long lT;			/*lT is a temp long that strtol will return its data to */
	char end;
	char *pchTend;		/*pchTend is a pointer to the terminating character of pszT*/

	newInst.opcode = 0;
	newInst.op1 = 0;
	newInst.op2 = 0;
	newInst.op3 = 0;
	newInst.op4 = 0;


	if (!script_in_use())
	{
		
		return newInst;
	}


	get_next_line();

	if (!get_tokens(line, tokens))
	{
		
		return newInst;
	}

	opstring = (char *)malloc(sizeof(char)*16);

	opstring[0] = '0';
	opstring[1] = 'x';
	opstring[2] = '0';
	opstring[3] = line[tokens[1]];	/* grab opcode */
	opstring[4] = line[tokens[1]+1];
	opstring[5] = 0;

	end = 0;
	pchTend= &end;	/*init our ending char */

	lT = strtol(opstring, &pchTend, 16);

	newInst.opcode = (short)lT;

	if (tokens[2] != -1)
	{
	 	get_blat(line, opstring,tokens[2]);
	 	
	 	sscanf(opstring, "%f", &newInst.op1);
	} 

	if (tokens[3] != -1)
	{
	 	get_blat(line, opstring,tokens[3]);
	 	
	 	sscanf(opstring, "%f", &newInst.op2);
	} 

	if (tokens[4] != -1)
	{
	 	get_blat(line, opstring,tokens[4]);
	 	
	 	sscanf(opstring, "%f", &newInst.op3);
	} 

	if (tokens[5] != -1)
	{
	 	get_blat(line, opstring,tokens[5]);
	 	
	 	sscanf(opstring, "%f", &newInst.op4);
	} 

	return newInst;


}
 
/*execute_instruction calls the function pointed to by the
opcode passed in inst, a script_instuction.
*/
void execute_instruction(script_instruction inst)
{
	if (inst.opcode == 0 || inst.opcode > NUMBER_OF_INSTRUCTIONS)
		return;
	
	
	(*instruction_lookup[inst.opcode])(inst);	//call the function

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
 
 
 
 /*is_white_space is a function that checks the current character against a list of
whitespace characters and returns the character's validity as whitespace. 
*/
int is_white_space(char *pszSrc, int istart)
{
   
	return ((*(pszSrc + istart) == ' ' || *(pszSrc + istart) == '\t'));
   
 }

/*FIsEnd is a function that checks the current character against a list of
'end' characters and returns if the character is an 'end' character or not.
End characters are semicolon, newline, EOF, and the null char.
*/
int is_end(char *pszSrc, int istart)
{
	return ((*(pszSrc + istart) == '#' || *(pszSrc + istart) == '\n' || 
			*(pszSrc + istart) == '\0' || *(pszSrc + istart) == '\r'));
	
}


 

/*FIsComma is a function that checks to see if the current character is a
comma or not.
*/
int is_comma(char *pszSrc, int istart)
{
     
	return (*(pszSrc + istart) == ',');
 	
}

 void get_blat(char *src, char *dest, int istart)
 {
 	int x;
 	
 	dest[0] = 0;
 	
 	for(x=istart;x < 256 && !is_end(src,x) && !is_white_space(src,x) && !is_comma(src,x);x++)
 		dest[x-istart] = src[x];
 		
 	dest[x-istart] = 0;
 	
 
 }
 

/*FIsColon is a function that checks to see if the current character is a
colon or not.
*/
int is_colon(char *pszSrc, int istart)
{
     
	return (*(pszSrc + istart) == ':');
 }


/*FBlatColon checks to see if a colon character is one of the characters in the string
passed in.  The string can be a skip or a blat.
*/
int blat_colon(char *pszSrc, int istart, int wlength)
{
	int cwcount;	/* a temp counter variable */
	
		/* loop through the string.  Stop if we find a colon or end character, or if we run out
		of space. */
		
	for (cwcount = istart; !is_end(pszSrc, cwcount) && *(pszSrc + cwcount) != ':' && cwcount < 256 && cwcount < istart + wlength;cwcount++);
	
	if (*(pszSrc + cwcount) == ':') /* found  a colon */
		return cwcount;
	
	return script_FALSE;
	
}


/*skip_comma checks to see if a comma character is one of the characters in the string
passed in.  The string can be a skip or a blat, but it is only used for skips.
*/
int skip_comma(char *pszSrc, int istart, int wlength)
{
	int cwcount;
	
		/* loop through the string.  Stop if we find a comma or end character, or if we run out
		of space. */
		

	for (cwcount = istart; !is_end(pszSrc, cwcount) && *(pszSrc + cwcount) != ',' && cwcount < 256 && cwcount < istart + wlength;cwcount++);
	
	if (*(pszSrc + cwcount) == ',') /* found  a comma */
		return script_TRUE;
	
	return script_FALSE;
	
}


/*NextBlat scans the string passed until it finds a non-end, non-whitespace character (the
beginning of a blat.  It then returns the location and length of the blat.
	Inputs: pszSrc, a pointer to the source string
			pistart, a pointer to the start of the string
			pwlength, a pointer to the length of the string
			fcomma, a boolean value dictating the validity of a comma in the preceeding skip.
				Commas can be required to be either present or absent from the skip.
	Outputs: script_TRUE(1) is returned if all goes well.  If true is returned, pwlength will
		point to the length of the next blat, and pistart will point to the start of the 
		next blat.
		script_FALSE (0) is returned if there is no more blats following the character pointed
		to by pistart (i.e., the end of the string has been reached).
		script_ERROR (-1) is returned if the grammer rules have been violated (i.e. a comma in the
		skip before the next blat when fcomma == script_FALSE).
*/
int next_blat(char *pszSrc, int *pistart, int *pwlength, int fcomma)
{
	int cwcount;	/* temp counter variable */
	
	/* loop through the string until we run out of whitespace, we find a comma and fcomma is false,
	we reach the end of the string, or we run out of characters (cwcount exceeds 255) */
	
	for (cwcount = *pistart; ( (fcomma == script_TRUE) ? (  is_comma(pszSrc, cwcount) || is_white_space(pszSrc, cwcount) ) : (  !is_comma(pszSrc, cwcount) && is_white_space(pszSrc, cwcount) )) && cwcount < 256 && !is_end(pszSrc, cwcount);cwcount++);
	
	if (is_end(pszSrc, cwcount))	/* did we reach the end of the string? */
	{
		if (skip_comma(pszSrc,*pistart,cwcount-*pistart)) /* was there a comma in between the last */
			return script_ERROR;								/* blat and the end?  If so, error. */
		else
			return script_FALSE;								/* if not, we are still done */
	}
	
	
	if (!fcomma && is_comma(pszSrc, cwcount))		/* is a comma present in the skip, even though we */
		return script_ERROR;								/* don't want one?  If so, error */
		
	if (fcomma && !skip_comma(pszSrc,*pistart,cwcount-*pistart))	/* is a comma presend when we */
		return script_ERROR;									/* absolutely must have one? if not, error */
		
	if (is_colon(pszSrc, cwcount))	/* does the blat start with a colon?  error! */
		return script_ERROR;

	
	*pistart = cwcount;		/* we have found the start of the blat */
	
	/*now, to find the length of the blat, loop through the string until we find whitespace, a comma,
	an end char, or until we run out of characters */
	
	for (cwcount = *pistart; !( is_comma(pszSrc, cwcount) || is_white_space(pszSrc, cwcount) ) && cwcount < 256 && !is_end(pszSrc, cwcount);cwcount++);
		
	*pwlength = cwcount-*pistart;	/*once we have found the end of the blat, we can get the length*/

	return script_TRUE;			/* and all was good */

}



/*FGetTokens takes a string passed in pszSrc and tokenizes it into up to twelve seperate
'blats', whose starting locations are returned in the array prigw
	Inputs: pszSrc, a pointer to the source string
			prigw,	a pointer to the dest array
	Outputs: script_FALSE (0) is returned if the string was sucessfully tokenized.
			script_TRUE (0) is returned if an error occured.
			prigw will contain the starting character positions of each token upon sucessful
			exit of FGetTokens. */

int get_tokens (char *pszSrc,int *prgiw)
{
	int cwCount = 0;	/* counter variable */
	int istart = 0;		/* character index variable */
	int wlength = 0;	/* temp length variable */
	int wColon = 0;		/* boolean/ index value */
	int wStatus;		/* temp stat variable */
	
	for (cwCount=0;cwCount<12;cwCount++)	/* init array to -1's */
	{
		prgiw[cwCount]=-1;
	}
	
	cwCount=0;
	
	wStatus = next_blat(pszSrc,&istart,&wlength,script_FALSE);	/* find the first blat */
		
	if (wStatus == script_TRUE)		/* if it was found then... */
	{
		wColon = blat_colon(pszSrc,0,istart);		/* check for a leading colon */
		if (wColon)
			return script_TRUE;							/* if one exits before a character, error out */
			
		wColon = blat_colon(pszSrc,istart,wlength);	/* if not, check to see if our current blat */
		if (wColon)									/* contains a colon */
		{
			prgiw[0] = istart;						/* if so, go into prigw index 0 */
			istart = wColon +1;
			cwCount++;
			
		} else {
			prgiw[0] = -1;							/* otherwise, go into prigw index 1 */
			prgiw[1] = istart;
			istart += wlength;
			cwCount+=2;
		}
	} else if (wStatus == script_FALSE)	/* no blat was found... we are finished */
		return script_TRUE;
	else if (wStatus == script_ERROR)		/* an error occured, drop out */
		return script_FALSE;
	
	wStatus = next_blat(pszSrc,&istart,&wlength,script_FALSE);	/* get second blat */
	
	if (wStatus == script_TRUE)				/* if we found it, save it */
	{
		prgiw[cwCount] = istart;
		istart += wlength;
		cwCount++;
	} else if (wStatus == script_FALSE)	/* otherwise, stop */
		return script_TRUE;
	else if (wStatus == script_ERROR)		/* or error, if we had problems */
		return script_FALSE;
	
	do	/* for all remaining blats, find and record start locations. Force commas after */
	{	/* rgiw[2] has been filled */
		wStatus = next_blat(pszSrc,&istart,&wlength,!(cwCount == 2 && prgiw[0]!=-1));
		if (wStatus == script_TRUE)
		{
			prgiw[cwCount] = istart;
			istart += wlength;
			cwCount++;
		}
	} while (wStatus == script_TRUE && istart < 256 && cwCount < 12);	/* stop if we run out of chars, have
																too many blats, run out of blats, or
																encounter an error */
	if (wStatus == script_ERROR)		/* report accordingly */			
		return script_FALSE;
	else
		return script_TRUE;
	
}
