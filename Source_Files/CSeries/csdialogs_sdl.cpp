/*
 *  csdialogs_sdl.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
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

 *  The purpose of this file is to provide a few easy-to-implement dialog-item
 *  modification and query functions, for greater source compatibility between
 *  SDL and traditional Mac versions.
 *
 *  Created by woody on Wed Sep 19 2001.
 */

#include	"cseries.h"
#include	"sdl_dialogs.h"
#include	"sdl_widgets.h"

void
modify_control_enabled(
	DialogPtr inDialog,
	short inWhichItem,
	short inChangeEnable) {

    assert(inDialog != NULL);
    
    widget*	theWidget = inDialog->get_widget_by_id(inWhichItem);
    
    assert(theWidget != NULL);
  
    if(inChangeEnable != NONE)
        theWidget->set_enabled(inChangeEnable == CONTROL_ACTIVE ? true : false);
}



/*************************************************************************************************
 *
 * Function: get_selection_control_value
 * Purpose:  given a dialog and an item number, extract the value of the control
 *
 *************************************************************************************************/
// Works only on w_select (and subclasses).
short
get_selection_control_value(DialogPtr dialog, short which_control)
{
    assert(dialog != NULL);
    
    w_select*	theSelectionWidget = dynamic_cast<w_select*>(dialog->get_widget_by_id(which_control));
    
    assert(theSelectionWidget != NULL);
    
    return theSelectionWidget->get_selection() + 1;
}



void
copy_cstring_to_static_text(DialogPtr dialog, short item, const char* cstring) {
	assert(dialog != NULL);
	
	w_static_text*	theStaticText = dynamic_cast<w_static_text*>(dialog->get_widget_by_id(item));
	
	assert(theStaticText != NULL);
	
	theStaticText->set_text(cstring);
}


