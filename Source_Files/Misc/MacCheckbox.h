#ifndef _MAC_CHECKBOX_
#define _MAC_CHECKBOX_
/*

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

	June 11, 2000 (Loren Petrich)
	
	This implements MacOS checkboxes
*/


class MacCheckbox
{
	// Private stuff
	ControlHandle CHdl;
	short Item;

public:

	// Get the item number
	short GetItem() {return Item;}
	
	// State routines; the get one returns which state
	bool GetState() {return GetControlValue(CHdl);}
	void SetState(bool State) {SetControlValue(CHdl, State);}
	void ToggleState() {SetState(!GetState());}
	
	// Toggle if the item number matches; return whether there was a match
	bool ToggleIfHit(short _Item)
	{bool IsHit = (Item == _Item); if (IsHit) ToggleState(); return IsHit;}
	
	// Constructor
	MacCheckbox(DialogPtr DPtr, short _Item, bool State): Item(_Item)
	{
		short ItemType;
		Rect ItemRect;
		GetDialogItem(DPtr, Item, &ItemType, (Handle *)&CHdl, &ItemRect);
		SetState(State);
	}
};

#endif
