// preferences_private.h
//
// Stuff implementers of the preferences system are interested in, but the rest of the game is not.
// Created by Woody Zenfell, III on 23 October 2001
//
// Initial contents: dialog item identifier constants ripped from preferences_macintosh.cpp

#ifndef PREFERENCES_PRIVATE_H
#define PREFERENCES_PRIVATE_H

#ifdef USES_NIBS
const CFStringRef Window_Preferences = CFSTR("Preferences");
const int iTABS = 3;

// For marking out the panes themselves
const OSType Sig_Pane = 'pane';

// For marking out the inhabitants of each pane
const OSType Sig_Graphics = 'grfx';
const OSType Sig_Sound = 'snds';
const OSType Sig_Player = 'plyr';
const OSType Sig_Input = 'inpt';
const OSType Sig_Environment = 'envr';
 
#endif

enum {
	ditlGRAPHICS= 4001,
	iCHOOSE_MONITOR=1,
	iHARDWARE_ACCELERATION,
	iNUMBER_OF_COLORS,
	iWINDOW_SIZE,
	iDETAIL,
	iBRIGHTNESS,
	iOPENGL_OPTIONS, // LP addition: OpenGL-options button
	iFILL_SCREEN // LP addition: fill-the-screen button
};

enum {
	ditlPLAYER= 4002,
	iDIFFICULTY_LEVEL= 1,
	iNAME,
	iCOLOR,
	iTEAM,
	// LP additions:
	iCHASE_CAM = 7,
	iCROSSHAIRS
};

// LP: added music and Ian Rickard's relative-to-system flag
enum {
	ditlSOUND= 4003,
	iSTEREO= 1,
	iACTIVE_PANNING,
	iAMBIENT_SOUND,
	iHIGH_QUALITY,
	iMORE_SOUNDS,
	iRELATIVE_VOLUME,
	iVOLUME,
	iCHANNELS,
	iMUSIC
};

// LP additions: InputSprocket, run/walk and swim/sink reversers;
// renamed "iSET_KEYS" to "iSET_INPUTS".
// LP: added Josh Elsasser's don't-switch-weapons patch
// LP: added separate InputSprocket button
enum {
	ditlINPUT= 4004,
	iMOUSE_CONTROL= 1,
	iKEYBOARD_CONTROL,
	iINPUT_SPROCKET_CONTROL,
	iSET_KEYS,
	iSET_INPUT_SPROCKET,
	iINTERCHANGE_RUN_WALK,
	iINTERCHANGE_SWIM_SINK,
	iDONT_SWITCH_TO_NEW_WEAPON,
	iDONT_AUTO_RECENTER,
	iUSE_INTERFACE_BUTTON_SOUNDS,
	iINVERT_MOUSE_VERTICAL,
	iVERTICAL_MOUSE_SENSITIVITY,
	iHORIZONTAL_MOUSE_SENSITIVITY
};

enum {
	ditlENVIRONMENT= 4005,
	iMAP= 1,
	iPHYSICS,
	iSHAPES,
	iSOUNDS,
	iPATCHES_LIST
};

enum {
	strPREFERENCES_GROUPS= 139,
	graphicsGroup= 0,
	playerGroup,
	soundGroup,
	inputGroup,
	environmentGroup,
	NUMBER_OF_PREFS_GROUPS
};


#endif//PREFERENCES_PRIVATE_H
