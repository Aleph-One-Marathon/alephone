#ifndef _MAC_CHECKBOX_
#define _MAC_CHECKBOX_
/*
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
