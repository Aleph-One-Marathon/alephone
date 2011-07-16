/*
	macintosh_wad_prefs.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Tuesday, August 29, 1995 3:53:22 PM- rdm created.

Feb 24, 2000 (Loren Petrich):
	Saving setting of Preferences dialog box between choices
	
Dec 31, 2000 (Mike Benonis):
	Changed Section Popup Menu item number from 3 to 4.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
*/

#include "cseries.h"
#include <string.h>

#include "map.h"
#include "wad.h"
#include "game_errors.h"

#include "shell.h" // for refPREFERENCES_DIALOG only
#include "wad_prefs.h"

extern struct preferences_info *prefInfo;

// LP change: moved current_pref_section out here
static short current_pref_section = 0;

/* ------------------ non portable code! */
static bool handle_switch(DialogPtr dialog, struct preferences_dialog_data *funcs,
	void **prefs, short new_section, short old_section);
static pascal Boolean preferences_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);

enum {
	dlogPREFERENCES_DIALOG= 4000,
	/* iOK */
	/* iCANCEL */
	iPREF_SECTION_POPUP= 4,
	NUMBER_VALID_PREF_ITEMS= 8
};	

/* -------------- Preferences Dialog --------- */
/* Kick ass, take names */
bool set_preferences(
	struct preferences_dialog_data *funcs, 
	short count,
	void (*reload_function)(void))
{
	DialogPtr dialog;
	short index, item_type, item_hit;
	Rect bounds;
	ControlHandle preferences_control;
	MenuHandle mHandle;
	void *preferences;
	short new_value;
	ModalFilterUPP modal_proc;
	
	assert(count);
	
	dialog= myGetNewDialog(dlogPREFERENCES_DIALOG, NULL,(WindowPtr) -1, refPREFERENCES_DIALOG);
	assert(dialog);
	
	/* Setup the popup list.. */
	GetDialogItem(dialog, iPREF_SECTION_POPUP, &item_type, 
		(Handle *) &preferences_control, &bounds);
	assert(preferences_control);
	mHandle= GetControlPopupMenuHandle(preferences_control);

	/* Append the preferences names to the popup. */
	for(index= 0; index<count; ++index)
	{
		AppendMenu(mHandle, "\p ");
		getpstr(ptemporary, funcs[index].resource_group, funcs[index].string_index);
		SetMenuItemText(mHandle, index+1, ptemporary);
	}
	
	/* Set our max value.. */
	SetControlMaximum(preferences_control, index+1); /* +1 because menus are one based. */

	/* Set the current prefs section.. */
	handle_switch(dialog, funcs, &preferences, current_pref_section, NONE);
	SetControlValue(preferences_control, current_pref_section+1);

	/* Show the dialog... */
	ShowWindow(GetDialogWindow(dialog));

	/* Create the modal proc. */
	modal_proc= NewModalFilterUPP(preferences_filter_proc);
	assert(modal_proc);

	/* Setup the filter procedure */
	/* Note that this doesn't allow for cancelling.. */	
	do {
		ModalDialog(modal_proc, &item_hit);
		
		switch(item_hit)
		{
			case iPREF_SECTION_POPUP:
				new_value= GetControlValue(preferences_control)-1;
				if(new_value != current_pref_section)
				{
					if(handle_switch(dialog, funcs, &preferences, 
						new_value, current_pref_section))
					{
						/* Changed it... */
						current_pref_section= new_value;
					}
				}
				break;
				
			case iOK:
				if(!funcs[current_pref_section].teardown_dialog_func(dialog, 
					NUMBER_VALID_PREF_ITEMS, preferences))
				{
					/* We can't tear down yet.. */
					item_hit= 4000;
				}
				break;
				
			case iCANCEL:
				if(funcs[current_pref_section].teardown_dialog_func(dialog, 
					NUMBER_VALID_PREF_ITEMS, preferences))
				{
					/* Reload the wadfile.. */
					reload_function();
				} else {
					/* Can't tear down yet... */
					item_hit= 4000;
				}
				break;
				
			default:
				funcs[current_pref_section].item_hit_func(dialog, 
					NUMBER_VALID_PREF_ITEMS, preferences, item_hit);
				break;
		}
	} while (item_hit>iCANCEL);
	
	// LP change: remembering which one it is (do one-based to zero-based)
	current_pref_section = GetControlValue(preferences_control) - 1;

	DisposeModalFilterUPP(modal_proc);	
	DisposeDialog(dialog);

	return item_hit==iOK;
}

/* ------------------ local code */
static bool handle_switch(
	DialogPtr dialog, 
	struct preferences_dialog_data *funcs,
	void **prefs,
	short new_section,
	short old_section)
{
	bool able_to_switch= true;

	/* Call the cleanup routines.. */
	if(old_section != NONE)
	{
		able_to_switch= funcs[old_section].teardown_dialog_func(dialog, 
			NUMBER_VALID_PREF_ITEMS, *prefs);
		if(able_to_switch)
		{
			short number_items= CountDITL(dialog)-NUMBER_VALID_PREF_ITEMS;
		
			/* Remove the old ones. */
			ShortenDITL(dialog, number_items);
		}
	}
	
	if(able_to_switch)
	{
		Handle theDITL;
	
		/* Add the data from the dialog... */
		theDITL= GetResource('DITL', funcs[new_section].ditl_id);
		assert(theDITL);
		
		/* Append it.. */
		AppendDITL(dialog, theDITL, overlayDITL);
		
		/* Free the memory */
		ReleaseResource(theDITL);

		*prefs= funcs[new_section].get_data();

		/* Called on setup (initialize your fields) */
		funcs[new_section].setup_dialog_func(dialog, NUMBER_VALID_PREF_ITEMS, 
			*prefs);
	}

	return able_to_switch;
}

static pascal Boolean preferences_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	short item_type, value, new_value;
	ControlHandle preferences_control;
	Rect bounds;
	bool handled= false;

	GetDialogItem(dialog, iPREF_SECTION_POPUP, &item_type, 
		(Handle *) &preferences_control, &bounds);
	new_value= value= GetControlValue(preferences_control);

	switch(event->what)
	{
		case keyDown:
		case autoKey:
			switch(event->message & charCodeMask)
			{
				case kPAGE_UP:
					new_value= value-1;
					break;

				case kPAGE_DOWN:
					new_value= value+1;
					break;

				case kHOME:
					new_value= 1;
					break;
					
				case kEND:
					new_value= 45;
					break;
			}

			new_value= PIN(new_value, 1, GetControlMaximum(preferences_control)-1);
			if(new_value != value)
			{
				SetControlValue(preferences_control, new_value);
				*item_hit= iPREF_SECTION_POPUP;
				event->what= nullEvent;
				handled= true;
			}
			break;
	}
	
	if(!handled)
	{
		handled= general_filter_proc(dialog, event, item_hit);
	}
	
	return handled;
}
