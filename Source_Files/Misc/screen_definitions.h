#ifndef __SCREEN_DEFINITIONS_H
#define __SCREEN_DEFINITIONS_H

/*

	screen_definitions.h
	Tuesday, July 11, 1995 5:32:20 PM- rdm created.

*/

/* -------- this contains the ids for the 8 bit picts */
/* the 16 bit versions are these ids + 10000 */
/* the 32 bit versions are these ids + 20000 */
enum {
	INTRO_SCREEN_BASE= 1000,
	MAIN_MENU_BASE= 1100,
	PROLOGUE_SCREEN_BASE= 1200,
	EPILOGUE_SCREEN_BASE= 1300,
	CREDIT_SCREEN_BASE= 1400,
	CHAPTER_SCREEN_BASE= 1500,
	COMPUTER_INTERFACE_BASE= 1600,
	INTERFACE_PANEL_BASE= 1700,
	FINAL_SCREEN_BASE= 1800
};

#endif

