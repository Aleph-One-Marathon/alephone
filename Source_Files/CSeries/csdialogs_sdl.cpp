/*
 *  csdialogs_sdl.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
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

 *  The purpose of this file is to provide a few easy-to-implement dialog-item
 *  modification and query functions, for greater source compatibility between
 *  SDL and traditional Mac versions.
 *
 *  Created by woody on Wed Sep 19 2001.
 */

#include	"cseries.h"
#include	"sdl_dialogs.h"
#include	"sdl_widgets.h"

// These fail silently, as half the existing calls to them are for widgets we don't use.
// Note that we merely disable/enable the items... this has semantic disagreement with the Mac
// version beyond "the user can see controls that have been hidden", though: if a control were
// enabled while hidden, or disabled and then shown, the user would be allowed to interact with
// the control in our version, but would not be able to on the Mac side.  Fortunately, I think
// the current code does not contain those usage patterns.
void HideDialogItem(DialogPtr dialog, short item_index) {
    widget* theWidget = dialog->get_widget_by_id(item_index);
    if(theWidget != NULL)
        theWidget->set_enabled(false);
}

void ShowDialogItem(DialogPtr dialog, short item_index) {
    widget* theWidget = dialog->get_widget_by_id(item_index);
    if(theWidget != NULL)
        theWidget->set_enabled(true);
}



// Item must be a w_number_entry.
long
extract_number_from_text_item(
	DialogPtr dlg,
	short item) {
        
    assert(dlg != NULL);
    
    w_number_entry*	theWidget = dynamic_cast<w_number_entry*>(dlg->get_widget_by_id(item));
    
    assert(theWidget != NULL);
    
    return theWidget->get_number();
}


// Item must be a w_number_entry.
void
insert_number_into_text_item(
	DialogPtr dlg,
	short item,
	long number) {
    
    assert(dlg != NULL);
    
    w_number_entry*	theWidget = dynamic_cast<w_number_entry*>(dlg->get_widget_by_id(item));
    
    assert(theWidget != NULL);
    
    theWidget->set_number(number);
}


// For enable/disable, widget may be anything - but see modify_control_enabled if that's all you're trying to do.
// For change value, widget must be a w_select (or subclass, but should not be a w_toggle due to different numbering scheme).
// (See modify_boolean_control if you're operating on a w_toggle.)
void
modify_selection_control(
	DialogPtr inDialog,
	short inWhichItem,
	short inChangeEnable,
	short inChangeValue) {

    assert(inDialog != NULL);
    
    widget*	theWidget = inDialog->get_widget_by_id(inWhichItem);
    
    assert(theWidget != NULL);
  
    if(inChangeEnable != NONE)
        theWidget->set_enabled(inChangeEnable == CONTROL_ACTIVE ? true : false);
        
    if(inChangeValue != NONE) {
        w_select*	theSelectionWidget = dynamic_cast<w_select*>(theWidget);
        assert(theSelectionWidget != NULL);
        
        theSelectionWidget->set_selection(inChangeValue - 1);
    }
}



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



void
modify_boolean_control(
	DialogPtr inDialog,
	short inWhichItem,
	short inChangeEnable,
	short inChangeValue) {

    assert(inDialog != NULL);
    
    widget*	theWidget = inDialog->get_widget_by_id(inWhichItem);
    
    assert(theWidget != NULL);
  
    if(inChangeEnable != NONE)
        theWidget->set_enabled(inChangeEnable == CONTROL_ACTIVE ? true : false);
        
    if(inChangeValue != NONE) {
        w_toggle*	theSelectionWidget = dynamic_cast<w_toggle*>(theWidget);
        assert(theSelectionWidget != NULL);
    
        theSelectionWidget->set_selection(inChangeValue);
    }
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



// Just the same, but for w_toggles.
bool
get_boolean_control_value(DialogPtr dialog, short which_control)
{
    assert(dialog != NULL);
    
    w_toggle*	theSelectionWidget = dynamic_cast<w_toggle*>(dialog->get_widget_by_id(which_control));
    
    assert(theSelectionWidget != NULL);
    
    return theSelectionWidget->get_selection();
}



void
copy_pstring_from_text_field(DialogPtr dialog, short item, unsigned char* pstring) {
    assert(dialog != NULL);
    
    w_text_entry*	theTextField = dynamic_cast<w_text_entry*>(dialog->get_widget_by_id(item));
    
    assert(theTextField != NULL);
    
    const char* 	source	= theTextField->get_text();
    char*		dest	= (char*) pstring;

    // pstring buffer is probably not going to be bigger than 256 bytes since that's max length of a pstr.
    if(strlen(source) > 255)
        strncpy(dest, source, 255);	// (leave room for terminating NULL)
    // OTOH don't want to fill all 256 memory locations (strncpy pads with '\0') if, say, caller only has 32 bytes for us...
    else
        strcpy(dest, source);
    
    // in-place conversion
    a1_c2pstr(dest);
}



void
copy_pstring_to_text_field(DialogPtr dialog, short item, const unsigned char* pstring) {
    assert(dialog != NULL);
    
    w_text_entry*	theTextField = dynamic_cast<w_text_entry*>(dialog->get_widget_by_id(item));
    
    assert(theTextField != NULL);
    
    unsigned char*	source	= pstrdup(pstring);
    char* 		string	= a1_p2cstr(source);
    
    theTextField->set_text(string);
   
    free(source);
}



void
copy_pstring_to_static_text(DialogPtr dialog, short item, const unsigned char* pstring) {
    assert(dialog != NULL);
    
    w_static_text*	theStaticText = dynamic_cast<w_static_text*>(dialog->get_widget_by_id(item));
    
    assert(theStaticText != NULL);
    
    unsigned char*	source	= pstrdup(pstring);
    char* 		string	= a1_p2cstr(source);
    
    theStaticText->set_text(string);
    
    free(source);
}
