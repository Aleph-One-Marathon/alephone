/*

	game_errors.c
	Monday, June 12, 1995 9:25:53 AM- rdm created.

*/

#include "cseries.h"
#include "game_errors.h"

#ifdef env68k
	#pragma segment file_io
#endif

static short last_type= systemError;
static short last_error= 0;

void set_game_error(
	short type, 
	short error_code)
{
	assert(type>=0 && type<NUMBER_OF_TYPES);
	last_type= type;
	last_error= error_code;
#ifdef DEBUG
	if(type==gameError) assert(error_code>=0 && error_code<NUMBER_OF_GAME_ERRORS);
#endif
}

short get_game_error(
	short *type)
{
	if(type)
	{
		*type= last_type;
	}
	
	return last_error;
}

bool error_pending(
	void)
{
	return (last_error!=0);
}

void clear_game_error(
	void)
{
	last_error= 0;
	last_type= 0;
}
