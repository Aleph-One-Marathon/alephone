/*
	keyboard_dialog.c

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

	Tuesday, June 13, 1995 5:49:10 PM- rdm created.

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added accessors for datafields now opaque in Carbon
	Now use UPP for callback procedures

Feb 6, 2002 (Br'fin (Jeremy Parsons)):
	Correcting usage of GetDialogKeyboardFocusItem() (already offset by 1)
*/

#include <string.h>

#include "macintosh_cseries.h"
#include "interface.h"
#include "shell.h"
#include "screen.h"

#define strKEYCODES_TO_ASCII        133
#define alrtDUPLICATE_KEY           136
#define OPTION_KEYCODE             0x3a

#ifdef USES_NIBS

const CFStringRef Window_Prefs_Keyboard = CFSTR("Prefs_Keyboard");

#endif

enum {
	dlogCONFIGURE_KEYS= 1000,
	dlogCONFIGURE_KEYS12,
	iFORWARD=3,
	iBACKWARD,
	iTURN_LEFT,
	iTURN_RIGHT,
	iSLIDE_LEFT,
	iSLIDE_RIGHT,
	iLOOK_LEFT,
	iLOOK_RIGHT,
	iLOOK_UP,
	iLOOK_DOWN,
	iLOOK_STRAIGHT,
	iPREV_WEAPON,
	iNEXT_WEAPON,
	iTRIGGER,
	iALT_TRIGGER,
	iSIDESTEP,
	iRUN,
	iLOOK,
	iACTION_KEY,
	iTOGGLE_MAP,
	iUSE_MICROPHONE,
	iKEY_LAYOUT_POPUP_TITLE,
	iMOVED_OK= 55,
	iKEY_LAYOUT_POPUP= 62
};

const int FIRST_KEY_ITEM = iFORWARD;  // first item in dlogCONFIGURE_KEYS with a keycode
const int LAST_KEY_ITEM = iUSE_MICROPHONE;  // last item in dlogCONFIGURE_KEYS with a keycode
const int NUMBER_OF_KEY_ITEMS = (LAST_KEY_ITEM - FIRST_KEY_ITEM + 1);

enum {
	_custom_keyboard_item= 4
};


#ifdef USES_NIBS


// Prototypes
static short keycode_to_charcode(short keycode);


struct KeyboardHandlerData
{
	short *KeyCodes;
	ControlRef KeyCtrls[NUMBER_OF_KEY_ITEMS];
	ControlRef KeysetPopup;
	
	// Finds the popup-menu setting corresponding to the current keycodes
	void SetPopup();
	
	// Updates the key codes from the popup-menu selection
	void UpdateKeyCodes();
	
	// Set a keycode;
	// returns whether it duplicated an existing one
	bool SetKeyCode(int Which, short KeyCode);
	
	// Each one
	void CodeToControl(int Which);
	
	// All of them
	void CodesToControls();
};


void KeyboardHandlerData::SetPopup()
{
	short KeySetup = find_key_setup(KeyCodes);
	
	if (KeySetup == NONE)
		KeySetup = _custom_keyboard_item;
	
	SetControl32BitValue(KeysetPopup, KeySetup+1);
}


void KeyboardHandlerData::UpdateKeyCodes()
{
	int KeySetup = GetControl32BitValue(KeysetPopup) - 1;
	
	if (KeySetup < NUMBER_OF_KEY_SETUPS)
		set_default_keys(KeyCodes, KeySetup);
	
	CodesToControls();
}


bool KeyboardHandlerData::SetKeyCode(int Which, short KeyCode)
{
	// Check for matches
	for (int k=0; k<NUMBER_OF_KEY_ITEMS; k++)
	{
		if (k != Which)
		{
			if (KeyCode == KeyCodes[k])
				return false;
		}
	}
	
	KeyCodes[Which] = KeyCode;
	CodeToControl(Which);
	SetPopup();
	return true;
}


void KeyboardHandlerData::CodeToControl(int Which)
{
	short KeyCode = KeyCodes[Which];
	
	if (KeyCode >= 0 && KeyCode < 0x7f)
	{
		getpstr(ptemporary, strKEYCODES_TO_ASCII, KeyCode);
		SetEditPascalText(KeyCtrls[Which], ptemporary);
	}
	else
		SetEditCText(KeyCtrls[Which], "-----");
	
	Draw1Control(KeyCtrls[Which]);
}

void KeyboardHandlerData::CodesToControls()
{
	for (int k=0; k<NUMBER_OF_KEY_ITEMS; k++)
		CodeToControl(k);
}


// For each individual control
struct KeyboardControlHandlerData
{
	KeyboardHandlerData *HDPtr;
	int Which;
};


static short ValidateKey(UInt32 Key)
{
	// Cribbed from original find_key_hit()
	
	if (Key < 0 && Key > INT16_MAX)
		return NONE;
	
	short keycode = Key;
	short error_message= NONE;
	short charcode;

	charcode = keycode_to_charcode(keycode);
	switch(charcode)
	{
	case ',':
	case '.':
		if (keycode != 0x41) // IS_KEYPAD() is worthless here.
			error_message= keyIsUsedForSound;
		break;
		
	case '-':
	case '_':
	case '+':
	case '=':
		if (keycode != 0x4e && keycode != 0x45) // IS_KEYPAD() is worthless here
			error_message=  keyIsUsedForMapZooming;
		break;
		
	case '[':
	case ']':
		error_message= keyIsUsedForScrolling;
		break;
		
	default:
		switch(keycode)
		{
		case kcF1:
		case kcF2:
		case kcF3:
		case kcF4:
		case kcF5:
		case kcF6:
		case kcF7:
		case kcF8:
		case kcF9:
			error_message= keyIsUsedAlready;
			break;
			
		case 0x7f: /* This is the power key, which is not reliable... */
			keycode = NONE;
			break;
		}
		break;
	}
	
	if (error_message != NONE)
	{
		SysBeep(30);
		alert_user(infoError, strERRORS, error_message, 0);
		keycode= NONE;
	}

	return keycode;
}


static pascal OSStatus KeyboardHandler(
	EventHandlerCallRef HandlerCallRef,
	EventRef Event,
	void *Data
	)
{
	KeyboardControlHandlerData *CDPtr = (KeyboardControlHandlerData *)(Data);
	
	// Most of the code in this function is from Br'fin's keyboard-event parsing
	assert(GetEventClass(Event) == kEventClassKeyboard);
	short KeyCode = NONE;	// No key -- in case none was ever pressed
	
	switch(GetEventKind(Event))
	{
	case kEventRawKeyDown:
		{
			UInt32 RawKeycode;
			GetEventParameter(Event,
				kEventParamKeyCode, typeUInt32,
				NULL, sizeof(typeUInt32), NULL, &RawKeycode);
			KeyCode = ValidateKey(RawKeycode);
		}
		break;
		
	case kEventRawKeyModifiersChanged:
		{
			const UInt32 modiferMask = cmdKey | shiftKey | alphaLock | optionKey | controlKey;
			static UInt32 priorModifiers = 0;
			UInt32 modifiers;
			GetEventParameter(Event,
				kEventParamKeyModifiers, typeUInt32,
				NULL, sizeof(typeUInt32), NULL, &modifiers);
			modifiers &= modiferMask; // Strip off all elements we care about
			if(modifiers)
			{
				// We want bits only in modifiers...
				UInt32 new_modifiers= (modifiers ^ priorModifiers) & modifiers;
				switch(new_modifiers)
				{
				case cmdKey:
					KeyCode = 55;
					break;
					
				case shiftKey:
					KeyCode = 56;
					break;
					
				case alphaLock:
					KeyCode = 57;
					break;
					
				case optionKey:
					KeyCode = 58;
					break;
					
				case controlKey:
					KeyCode = 59;
					break;
				
				// Otherwise, no key (NONE)
				}
			}
			priorModifiers= modifiers;
		}
		break;
	
	case kEventRawKeyRepeat:
		// Do nothing
		break;
	}
	
	if (KeyCode != NONE)
	{
		// Update the keyboard display -- and beep if it could not be done
		if (!CDPtr->HDPtr->SetKeyCode(CDPtr->Which,KeyCode))
			SysBeep(30);
	}
	
	return noErr;
}


static void KeyboardDialogHandler(ParsedControl &Ctrl, void *Data)
{
	KeyboardHandlerData *HDPtr = (KeyboardHandlerData *)(Data);

	if (Ctrl.ID.signature == 0 && Ctrl.ID.id == iKEY_LAYOUT_POPUP)
		HDPtr->UpdateKeyCodes();
}


// Note: the original keycodes are saved outside of this function;
// it's not necessary to save the original ones
bool configure_key_setup(
	short *keycodes)
{
	bool IsTinyScreen =
		(RECTANGLE_WIDTH(&(*GetWorldDevice())->gdRect)<640 || RECTANGLE_HEIGHT(&(*GetWorldDevice())->gdRect)<480);
	
	AutoNibWindow Window(GUI_Nib, Window_Prefs_Keyboard);
	
	// So that the handlers will work with the transmitted data
	KeyboardHandlerData HandlerData;
	HandlerData.KeyCodes = keycodes;
	
	AutoKeyboardWatcher Watcher(KeyboardHandler);
	
	KeyboardControlHandlerData CtrlData[NUMBER_OF_KEY_ITEMS];
	for (int k=0; k<NUMBER_OF_KEY_ITEMS; k++)
	{
		HandlerData.KeyCtrls[k] = GetCtrlFromWindow(Window(), 0, FIRST_KEY_ITEM + k);
		CtrlData[k].HDPtr = &HandlerData;
		CtrlData[k].Which = k;
		Watcher.Watch(HandlerData.KeyCtrls[k], CtrlData + k);
	}
	
	HandlerData.CodesToControls();
	
	// The keyset popup
	HandlerData.KeysetPopup = GetCtrlFromWindow(Window(), 0, iKEY_LAYOUT_POPUP);
	HandlerData.SetPopup();
	
	bool IsOK = RunModalDialog(Window(), true, KeyboardDialogHandler, &HandlerData);
	
	return IsOK;
}


#else

/* Necessary globals, only for the key_setup_filter_proc */
struct keyboard_setup_struct {
	KeyMap old_key_map;
	short *keycodes;	
	short current_key_setup;
};

/* -------- globals. */
static struct keyboard_setup_struct keyboard_setup_globals;

/* -------- private prototypes */
bool configure_key_setup(short *keycodes);
static pascal Boolean key_setup_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);
static short setup_key_dialog(DialogPtr dialog, short *keycodes);
static short set_current_keyboard_layout(DialogPtr dialog, short *keycodes);
static void fill_in_key_name(DialogPtr dialog, short *keycodes, short which);
static short find_key_hit(byte *key_map, byte *old_key_map);
static short find_duplicate_keycode(short *keycodes);
static short keycode_to_charcode(short keycode);
static bool is_pressed(short key_code);

/* ----------- entry point */
bool configure_key_setup(
	short *keycodes)
{
	short item_hit, location_of_duplicate, menu_selection;
	DialogPtr dialog;
	bool data_is_bad;
	ModalFilterUPP key_setup_filter_upp;
	short current_key_set;
	
	if (RECTANGLE_WIDTH(&(*GetWorldDevice())->gdRect)<640 || RECTANGLE_HEIGHT(&(*GetWorldDevice())->gdRect)<480)
	{
		dialog = myGetNewDialog(dlogCONFIGURE_KEYS12, NULL, (WindowPtr) -1, 0);
	}
	else
	{
		dialog = myGetNewDialog(dlogCONFIGURE_KEYS, NULL, (WindowPtr) -1, refCONFIGURE_KEYBOARD_DIALOG);
	}
	assert(dialog);
#if defined(TARGET_API_MAC_CARBON)
	key_setup_filter_upp= NewModalFilterUPP(key_setup_filter_proc);
#else
	key_setup_filter_upp= NewModalFilterProc(key_setup_filter_proc);
#endif
	
	/* Setup the keyboard dialog.. */
	current_key_set= setup_key_dialog(dialog, keycodes);
	
	/* Select the text.. */
	SelectDialogItemText(dialog, iFORWARD, 0, SHRT_MAX);

	set_dialog_cursor_tracking(false);
//#if defined(USE_CARBON_ACCESSORS)
	ShowWindow(GetDialogWindow(dialog));
/*
#else
	ShowWindow(dialog);
#endif
*/

	/* Setup the globals.. */
	GetKeys(keyboard_setup_globals.old_key_map);
	keyboard_setup_globals.keycodes= keycodes;
	keyboard_setup_globals.current_key_setup= current_key_set;

	do
	{
		do
		{
			ControlHandle control;
			short item_type;
			Rect bounds;

			ModalDialog(key_setup_filter_upp, &item_hit);
			
			switch(item_hit)
			{
				case iKEY_LAYOUT_POPUP:
					GetDialogItem(dialog, item_hit, &item_type, (Handle *) &control, &bounds);
					menu_selection= GetControlValue(control) - 1;
					if (menu_selection != _custom_keyboard_item && keyboard_setup_globals.current_key_setup != menu_selection)
					{
						set_default_keys(keycodes, menu_selection);
						
						// looks slightly nicer to deselect text before changing and reselecting it.
//#if defined(USE_CARBON_ACCESSORS)
						SelectDialogItemText(dialog, GetDialogKeyboardFocusItem(dialog) + 1, 0, 0);
						keyboard_setup_globals.current_key_setup= setup_key_dialog(dialog, keycodes);
						SelectDialogItemText(dialog, GetDialogKeyboardFocusItem(dialog) + 1, 0, SHRT_MAX);
/*
#else
						SelectDialogItemText(dialog, ((DialogRecord *) dialog)->editField + 1, 0, 0);
						keyboard_setup_globals.current_key_setup= setup_key_dialog(dialog, keycodes);
						SelectDialogItemText(dialog, ((DialogRecord *) dialog)->editField + 1, 0, SHRT_MAX);
#endif
*/
					}
					break;
					
				default:
					break;
			}
		} while(item_hit != iMOVED_OK && item_hit != iCANCEL);

		if (!is_pressed(OPTION_KEYCODE))
		{
			location_of_duplicate= find_duplicate_keycode(keycodes);
			if (item_hit == iMOVED_OK && location_of_duplicate != NONE)
			{
				data_is_bad = true;
				SelectDialogItemText(dialog, location_of_duplicate + FIRST_KEY_ITEM, 0, SHRT_MAX);
				ParamText(getpstr(ptemporary, strKEYCODES_TO_ASCII, keycodes[location_of_duplicate]), "\p", "\p", "\p");
				Alert(alrtDUPLICATE_KEY, NULL);
			}
			else
			{
				data_is_bad = false;
			}
		}
		else
		{
			data_is_bad = false;
		}
	} while (data_is_bad);

	DisposeModalFilterUPP(key_setup_filter_upp);
	DisposeDialog(dialog);

	set_dialog_cursor_tracking(true);
			
	return item_hit==iMOVED_OK;
}

/* ----------- private code. */
static pascal Boolean key_setup_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	short keycode, which_item, current_edit_field;
	KeyMap key_map;
	Point where;
	GrafPtr old_port;
	bool handled= false;
	
	GetPort(&old_port);
//#if defined(USE_CARBON_ACCESSORS)
        SetPort(GetWindowPort(GetDialogWindow(dialog)));
/*
#else
	SetPort(dialog);
#endif
*/	
	/* preprocess events */	
	switch(event->what)
	{
		case nullEvent:
		case keyDown:
		case autoKey:
			GetKeys(key_map);
			if (memcmp(key_map, keyboard_setup_globals.old_key_map, sizeof(KeyMap))) // the user has hit a new key
			{
//#if defined(USE_CARBON_ACCESSORS)
				current_edit_field= GetDialogKeyboardFocusItem(dialog);
/*
#else
				current_edit_field= ((DialogRecord *) dialog)->editField + 1;
#endif
*/
				keycode= find_key_hit((byte *)key_map, (byte *)keyboard_setup_globals.old_key_map);

				// update the text field
				if (keycode != NONE && keycode != keyboard_setup_globals.keycodes[current_edit_field - FIRST_KEY_ITEM])
				{
					keyboard_setup_globals.keycodes[current_edit_field - FIRST_KEY_ITEM]= keycode;
					fill_in_key_name(dialog, keyboard_setup_globals.keycodes, current_edit_field - FIRST_KEY_ITEM);
				}
				
				if (keycode != NONE)
				{
					// select the next item.
					*item_hit = current_edit_field < LAST_KEY_ITEM ? current_edit_field + 1 : FIRST_KEY_ITEM;
					SelectDialogItemText(dialog, *item_hit, 0, SHRT_MAX);
				}
				BlockMove(key_map, keyboard_setup_globals.old_key_map, sizeof(KeyMap));
			}
			
			/* Change the keysetup if necessary */
			keyboard_setup_globals.current_key_setup= 
				set_current_keyboard_layout(dialog, keyboard_setup_globals.keycodes);
			if (event->what != nullEvent)	handled = true;
			break;
			
		case mouseDown:
			where = event->where;
			GlobalToLocal(&where);
			which_item= FindDialogItem(dialog, where) + 1;
			if (which_item >= FIRST_KEY_ITEM && which_item <= LAST_KEY_ITEM)
			{
				SelectDialogItemText(dialog, which_item, 0, SHRT_MAX);
				*item_hit = which_item;
				handled = true;
			}
			break;
			
		case updateEvt:
			break;
	}

	SetPort(old_port);

	return handled ? true : general_filter_proc(dialog, event, item_hit);
}

static short setup_key_dialog(
	DialogPtr dialog, 
	short *keycodes)
{
	short key, current_key_setup;

	for (key= 0; key<NUMBER_OF_KEYS; key++)
	{
		fill_in_key_name(dialog, keycodes, key);
	}

	current_key_setup= set_current_keyboard_layout(dialog, keycodes);
	
	return current_key_setup;
}

static short set_current_keyboard_layout(
	DialogPtr dialog,
	short *keycodes)
{
	short current_key_setup;
	ControlHandle control;
	short item_type;
	Rect bounds;

	GetDialogItem(dialog, iKEY_LAYOUT_POPUP, &item_type, (Handle *) &control, &bounds);
	current_key_setup= find_key_setup(keycodes);
	if(current_key_setup==NONE)
	{
		current_key_setup= _custom_keyboard_item;
	}
	SetControlValue(control, current_key_setup+1);

	return current_key_setup;
}

static void fill_in_key_name(
	DialogPtr dialog, 
	short *keycodes, 
	short which)
{
	Rect item_box;
	short item_type;
	Handle item_handle;
	
	vassert(keycodes[which] >= 0 && keycodes[which] <= 0x7f,
		csprintf(temporary, "which = %d, keycodes[which] = %d", which, keycodes[which]));
	getpstr(ptemporary, strKEYCODES_TO_ASCII, keycodes[which]);
	GetDialogItem(dialog, which + FIRST_KEY_ITEM, &item_type, &item_handle, &item_box);
	SetDialogItemText(item_handle, ptemporary);
}

static short find_key_hit(
	byte *key_map, 
	byte *old_key_map)
{
	byte mask;
	byte bit_count = 0;
	short i;
	short keycode = NONE;
	
	for (i = 0; i < 16; i++)
	{
		if (key_map[i] > old_key_map[i]) // an extra bit is set
		{
			mask = key_map[i] ^ old_key_map[i];
			while (!(mask & 1))
			{
				mask >>= 1;
				bit_count++;
			}
			keycode = (i * 8) + bit_count; 
			break;
		}
	}

	if(keycode != NONE)
	{
		short error_message= NONE;
		short charcode;

		charcode = keycode_to_charcode(keycode);
		switch(charcode)
		{
			case ',':
			case '.':
				if (keycode != 0x41) // IS_KEYPAD() is worthless here.
				{
					error_message= keyIsUsedForSound;
				}
				break;
				
			case '-':
			case '_':
			case '+':
			case '=':
				if (keycode != 0x4e && keycode != 0x45) // IS_KEYPAD() is worthless here
				{
					error_message=  keyIsUsedForMapZooming;
				}
				break;
				
			case '[':
			case ']':
				error_message= keyIsUsedForScrolling;
				break;
				
			default:
				switch(keycode)
				{
					case kcF1:
					case kcF2:
					case kcF3:
					case kcF4:
					case kcF5:
					case kcF6:
					case kcF7:
					case kcF8:
					case kcF9:
						error_message= keyIsUsedAlready;
						break;
						
					case 0x7f: /* This is the power key, which is not reliable... */
						keycode = NONE;
						break;
				}
				break;
		}
	
		if (error_message != NONE)
		{
			alert_user(infoError, strERRORS, error_message, 0);
			keycode= NONE;
		}
	}
	
	return keycode;
}

static short find_duplicate_keycode(
	short *keycodes)
{
	short i, j;
	
	for (i= 0; i<NUMBER_OF_KEYS; i++)
	{
		for (j= i+1; j<NUMBER_OF_KEYS; j++)
		{
			if (keycodes[i] == keycodes[j])	return j;
		}
	}
	
	return NONE;
}

#endif

/* DANGER! DANGER! DANGER!! This is Alain's code, untouched.... */
static short keycode_to_charcode(
	short keycode)
{
	byte    locked;
	long    kchr_resource_id;
	long    bullshit_from_system;
	short   charcode;
	Handle  kchr_resource;

	static unsigned long state = 0;

	// get the resource and lock it down
	kchr_resource_id = GetScriptVariable(GetScriptManagerVariable(smKeyScript), smScriptKeys);
	kchr_resource = GetResource('KCHR', (short) kchr_resource_id); // probably DONÕt want to release it.
	assert(kchr_resource);
	locked = HGetState(kchr_resource) & 0x80;
	HLock(kchr_resource);

	// translate the key
	// note the 2 methods for getting the character code. the first one is what inside mac
	// tells me to do. the 2nd one is what i have to do, because of what keytrans returns
	// to me. oy vay. liars all around me.
	bullshit_from_system = KeyTranslate(*kchr_resource, keycode, &state);
//	charcode = (bullshit_from_system >> 16) & 0x00ff;
	charcode = bullshit_from_system & 0x000000ff;

	// unlock it, if that's the way the system had it.
	if (!locked) HUnlock(kchr_resource);
	
	return charcode;
}

static bool is_pressed(
	short key_code)
{
	KeyMap key_map;
	
	GetKeys(key_map);
	return ((((byte*)key_map)[key_code>>3] >> (key_code & 7)) & 1);
}

