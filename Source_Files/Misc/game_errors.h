#ifndef __GAME_ERRORS_H
#define __GAME_ERRORS_H

/*

	game_errors.h
	Monday, June 12, 1995 9:24:29 AM- rdm created.

*/

enum { /* types */
	systemError,
	gameError,
	NUMBER_OF_TYPES
};

enum { /* Game Errors */
	errNone= 0,
	errMapFileNotSet,
	errIndexOutOfRange,
	errTooManyOpenFiles,
	errUnknownWadVersion,
	errWadIndexOutOfRange,
	errServerDied,
	errUnsyncOnLevelChange,
	NUMBER_OF_GAME_ERRORS
};

void set_game_error(short type, short error_code);
short get_game_error(short *type);
boolean error_pending(void);
void clear_game_error(void);

#endif
