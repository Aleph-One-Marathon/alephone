#ifndef __WAD_PREFS_H
#define __WAD_PREFS_H

/*

	 wad_prefs.h
	 Wednesday, March 22, 1995 12:08:36 AM- rdm created.

Aug 21, 2000 (Loren Petrich):
	Added object-oriented file handling
*/

#include "FileHandler.h"

/* Open the file, and allocate whatever internal structures are necessary in the */
/*  preferences pointer.. */
bool w_open_preferences_file(char *PrefName, int Type);

typedef void (*prefs_initializer)(void *prefs);
typedef bool (*prefs_validater)(void *prefs);

void *w_get_data_from_preferences(
	WadDataType tag,					/* Tag to read */
	short expected_size,				/* Data size */
	prefs_initializer initialize,	/* Call if I have to allocate it.. */
	prefs_validater validate);	/* Verify function-> fixes if bad and returns true */
	
void w_write_preferences_file(void);

/* ------ local structures */
/* This is the structure used internally */
struct preferences_info {
	preferences_info() : wad(NULL) {}

	FileSpecifier PrefsFile;
	struct wad_data *wad;
};

/* This will need to be rewritten! */
#if defined(mac) || defined(SDL)
/*----------------- code from macintosh_wad_prefs.c */
#define LOCAL_TO_GLOBAL_DITL(id, first) (id+first)
#define GLOBAL_TO_LOCAL_DITL(id, first) (id-first)

/* ----- Run the new kick ass dialog box. */
struct preferences_dialog_data {
	short resource_group;					/* What STR# resource? */
	short string_index;						/* What indexed string? */
	short ditl_id;							/* What ditl do I append? */

	/* This prevents duplication-> you will already have this function */
	/* (generally just wraps get_data_from_preferences) */
	void *(*get_data)(void);

	/* Called on setup (initialize your fields) */
	void (*setup_dialog_func)(DialogPtr dialog, short first_item, void *prefs);

	/* Called when a user item is hit */
	void (*item_hit_func)(DialogPtr dialog, short first_item, void *prefs, 
		short item_hit);

	/* Use this to read in the edittext fields, etc. (return false to abort teardown) */
	bool (*teardown_dialog_func)(DialogPtr dialog, short first_item, void *prefs);
};

bool set_preferences(struct preferences_dialog_data *funcs, short count,
	void (*reload_function)(void));
#endif

#endif
