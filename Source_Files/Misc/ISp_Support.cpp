/*
	Loren Petrich: This code was originally written by Ben Thompson;
	I've modified it to improve InputSprocket support.
	
July 29, 2000 (Loren Petrich):
	Added icons and given them new symbolic names
	
	Added support for local-event queue; this is for handling quits, pauses,
	screen and sound resolution changes, etc.
	
	Changed error-code asserts into vasserts to be clearer

Jul 30, 2000 (Loren Petrich):
	Added refractory period for all local-event buttons;
	this keeps them from having awkward repeats.
*/


/* marathon includes */
#include "ISp_Support.h"
#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
#include "player.h"     // for get_absolute_pitch_range()
#include "shell.h"
#include "preferences.h"
#include "LocalEvents.h"
#include <math.h>

//Private Function Prototypes

/* macintosh includes */
#include <InputSprocket.h>
#include <CursorDevices.h>
#include <Traps.h>

const OSType kCreatorCode		= '26.A';
const OSType kSubCreatorCode	= 'BENT';
#define	kResourceID_setl			128

// LP change: renamed
enum
{
	// Global events:
	Icon_FirePrimary	 = 1000,
	Icon_FireSecondary	 = 1001,
	Icon_MoveForward	 = 1002,
	Icon_MoveBackward	 = 1003,
	Icon_TurnLeft		 = 1004,
	Icon_TurnRight		 = 1005,
	Icon_MoveLeft		 = 1006,
	Icon_MoveRight		 = 1007,
	Icon_LookLeft		 = 1008,
	Icon_LookRight		 = 1009,
	Icon_LookUp			 = 1010,
	Icon_LookDown		 = 1011,
	Icon_LookForward	 = 1012,
	Icon_PrevWeapon		 = 1013,
	Icon_NextWeapon		 = 1014,
	Icon_Action			 = 1015,
	Icon_RunNotWalk		 = 1016,
	Icon_SideNotTurn	 = 1017,
	Icon_LookNotTurn	 = 1018,
	Icon_AutoMap		 = 1019,
	Icon_Microphone		 = 1020,
	
	// Local events:
	Icon_Quit			 = 1100,
	Icon_Pause			 = 1101,
	Icon_SoundDown		 = 1102,
	Icon_SoundUp		 = 1103,
	Icon_MapOut			 = 1104,
	Icon_MapIn			 = 1105,
	Icon_InvenPrev		 = 1106,
	Icon_InvenNext		 = 1107,
	Icon_SwitchPlayer	 = 1108,
	Icon_BkgdTasks		 = 1109,
	Icon_FramesPerSec	 = 1110,
	Icon_PixelRes		 = 1111,
	Icon_ScreenDown		 = 1112,
	Icon_ScreenUp		 = 1113,
	Icon_BrightDown		 = 1114,
	Icon_BrightUp		 = 1115,
	Icon_SwitchSides	 = 1116,
	Icon_ChaseCam		 = 1117,
	Icon_TunnelVision	 = 1118,
	Icon_Crosshairs		 = 1119,
	Icon_ShowPosition	 = 1120,
	Icon_Screenshot		 = 1121,
	Icon_ResetTxtrs		 = 1122,

	// Axis stuff:
	Icon_LookHorizontal	 = 1200,
	Icon_LookVertical	 = 1201,
	Icon_Move			 = 1202,
};

// LP change: renamed
enum {
	// All global events, those transmitted across the network.
	// They are used to compose action_flags
	Button_FirstGlobal = 0,
	Button_LastGlobal = 20,
	// All local events, those that have only local effects
	Button_FirstLocal = 21,
	Button_LastLocal = 43,
	kAxisHoriz,
	kAxisVert,
	kAxisMove,
	kNeedCount	// LP: calculated automatically
};


// Give the local-event buttons a refractory period so that they will not
// repeat too fast
const int LocalButtonRefractoryTime = 10;
static int LocalButtonTimes[Button_LastLocal-Button_FirstLocal+1] =
	{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0};

static Boolean	canDoISp = true;
static Boolean	active = false;
static long		gElementActions[kNeedCount] =
					{
					// Global actions
					_left_trigger_state, _right_trigger_state, 
					_moving_forward, _moving_backward,
					_turning_left, _turning_right,
					_sidestepping_left, _sidestepping_right,
					_looking_left, _looking_right,
					_looking_up, _looking_down, _looking_center,
					_cycle_weapons_backward, _cycle_weapons_forward,
					_action_trigger_state, _run_dont_walk,
					_sidestep_dont_turn, _look_dont_turn,
					_toggle_map, _microphone_button,
					// Local actions
					LocalEvent_Quit, LocalEvent_Pause,
					LocalEvent_SoundDown, LocalEvent_SoundUp,
					LocalEvent_MapOut, LocalEvent_MapIn,
					LocalEvent_InvenPrev, LocalEvent_InvenNext,
					LocalEvent_SwitchPlayer, LocalEvent_BkgdTasks,
					LocalEvent_FramesPerSec, LocalEvent_PixelRes,
					LocalEvent_ScreenDown, LocalEvent_ScreenUp,
					LocalEvent_BrightDown, LocalEvent_BrightUp,
					LocalEvent_SwitchSides, LocalEvent_ChaseCam,
					LocalEvent_TunnelVision, LocalEvent_Crosshairs,
					LocalEvent_ShowPosition, LocalEvent_Screenshot,
					LocalEvent_ResetTxtrs,
					// Axis stuff
					0, 0, 0};

ISpElementListReference		gVirtualList = NULL;
ISpElementReference			gVirtualElements[kNeedCount] =
	{	// LP: prettyprinted to make them easier to match with gElementActions members
		// Global actions
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		// Local actions
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil, nil,
		nil, nil, nil,
		// Axis stuff
		nil, nil, nil
	};

// LP change:
#define kISpNeedFlag_Delta_AlreadyButton kISpNeedFlag_Axis_AlreadyButton

// LP change: removed the second "0," so that it fits with my version of the ISp SDK
static ISpNeed gNeeds[kNeedCount] =
{
	// Global actions
	{ "\pFire Primary", 	Icon_FirePrimary,		0, kISpElementKind_Button,		kISpElementLabel_Btn_Fire,			kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pFire Secondary", 	Icon_FireSecondary,		0, kISpElementKind_Button,		kISpElementLabel_Btn_SecondaryFire,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pMove Forward",		Icon_MoveForward, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_MoveForward,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pMove Backward",	Icon_MoveBackward, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_MoveBackward,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pLook Left", 		Icon_TurnLeft,			0, kISpElementKind_Button,		kISpElementLabel_Btn_TurnLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pLook Right", 		Icon_TurnRight, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_TurnRight,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pSidestep Left", 	Icon_MoveLeft, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_SlideLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pSidestep Right", 	Icon_MoveRight,		 	0, kISpElementKind_Button,		kISpElementLabel_Btn_SlideRight,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pGlance Left",	 	Icon_LookLeft, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_LookLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pGlance Right", 	Icon_LookRight, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_LookRight,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pLook Up",		 	Icon_LookUp, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_LookUp,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pLook Down",	 	Icon_LookDown, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_LookDown,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0},
	{ "\pLook Ahead",	 	Icon_LookForward,	 	0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pPrevious Weapon", 	Icon_PrevWeapon, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_Previous,		kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pNext Weapon",	 	Icon_NextWeapon, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_Next,			kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pAction", 			Icon_Action, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_Select,		kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pRun/Walk Toggle",	Icon_RunNotWalk,		0, kISpElementKind_Button,		kISpElementLabel_Btn_Run,			kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pMove Sideways",	Icon_SideNotTurn,		0, kISpElementKind_Button,		kISpElementLabel_Btn_SideStep,		kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pLook Sideways",	Icon_LookNotTurn,		0, kISpElementKind_Button,		kISpElementLabel_Btn_Look,			kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pAuto Map", 		Icon_AutoMap, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pMicrophone",	 	Icon_Microphone,		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	
	// Local actions
	{ "\pQuit",				Icon_Quit, 				0, kISpElementKind_Button,		kISpElementLabel_Btn_PauseResume,	kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pPause",			Icon_Pause, 			0, kISpElementKind_Button,		kISpElementLabel_Btn_PauseResume,	kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pSound Down",		Icon_SoundDown, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pSound Up",			Icon_SoundUp, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pMap Outward",		Icon_MapOut, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pMap Inward",		Icon_MapIn, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pInventory Prev",	Icon_InvenPrev, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pInventory Next",	Icon_InvenNext,			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pSwitch Player",	Icon_SwitchPlayer, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pBackground Tasks",	Icon_BkgdTasks, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pFrames Per Sec",	Icon_FramesPerSec, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pPixel Resolution",	Icon_PixelRes, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pScreen Res Down",	Icon_ScreenDown, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pScreen Res Up",	Icon_ScreenUp, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pBrightness Down",	Icon_BrightDown, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pBrightness Up",	Icon_BrightUp, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pSwitch Sides",		Icon_SwitchSides, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pChase Cam",		Icon_ChaseCam, 			0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pTunnel Vision",	Icon_TunnelVision, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pCrosshairs",		Icon_Crosshairs, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pShow Position",	Icon_ShowPosition, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pTake Screenshot",	Icon_Screenshot, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	{ "\pReset Textures",	Icon_ResetTxtrs, 		0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0},
	
	// Axis stuff
	{ "\pLook Horizontal",	Icon_LookHorizontal,	0, kISpElementKind_Delta,		kISpElementLabel_Delta_Yaw,			kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0},
	{ "\pLook Vertical",	Icon_LookVertical,		0, kISpElementKind_Delta,		kISpElementLabel_Delta_Pitch,		kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0},
	{ "\pMove",				Icon_Move, 				0, kISpElementKind_Delta,		kISpElementLabel_Delta_Z,			kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0},
};


//Starts up InputSprockets and adds all the default Needs and Controls
void initialize_ISp(void)
{
	OSErr err = noErr;
	
	err = ISpStartup();
	if(err != noErr)
	{
		canDoISp = false;
		return;
	}
	
	ISpDevices_ActivateClass(kISpDeviceClass_Mouse);
	if(input_preferences->input_device==_input_sprocket_only)  err = ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);
	else  err = ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);

	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpElement_NewVirtualFromNeeds(kNeedCount, gNeeds, gVirtualElements, 0);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpElementList_New(
			0,				// count
			NULL,		// needs
			&gVirtualList,			// virtual elements
			0);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	for(int a = 0; a < kNeedCount; a++)
	{
		err  = ISpElementList_AddElements (gVirtualList, 
					a, 		1, &gVirtualElements[a]);
		vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	}
	
	err = ISpInit(kNeedCount,	// count
		gNeeds,				// needs
		gVirtualElements,	// virtual elements
		kCreatorCode,		// app
		kSubCreatorCode,	// sub (we are using as a versioning)
		0,					// flags
		kResourceID_setl,	// set list resource id
		0);					// reserved
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
		
	err = ISpSuspend();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
}

void ShutDown_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	OSErr err = noErr;
	err = ISpStop();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpElement_DisposeVirtual(kNeedCount, gVirtualElements);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	ISpDevices_DeactivateClass(kISpDeviceClass_Mouse);
	
	err = ISpShutdown();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
}

void Start_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	if(input_preferences->input_device!=_input_sprocket_only && input_preferences->input_device!=_keyboard_or_game_pad) return;
	OSErr err = noErr;
	
	if(input_preferences->input_device==_input_sprocket_only)  err = ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);
	else  err = ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpResume();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	active = true;
}

void Stop_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	OSErr err = noErr;
	
	err = ISpSuspend();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	active = false;
}

long InputSprocketTestElements(void)
{
	if(!active) return 0; //Input Sprockets is not active, so return no action flags
	if(!canDoISp) return 0; //Input Sprockets is not available so return no action flags
	
	OSErr err = noErr;
	UInt32 data;
	long flags = 0;

	err = ISpTickle();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
// Check each of the action keys
// LP: These are all *global* actions, ones that are transmitted across the network
	for(int a = Button_FirstGlobal; a <= Button_LastGlobal; a++)
	{
		assert(gVirtualElements[a]);
		err = ISpElement_GetSimpleState(gVirtualElements[a], &data);
		vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
		if(data == kISpButtonDown) flags |= gElementActions[a];
	}

// LP: added handling of local actions, such as quitting or resolution change;
// there is no need for special handling of quitting.
	for(int a = Button_FirstLocal; a <= Button_LastLocal; a++)
	{
		// Check to see if the command is still in its refractory period
		// Make the time value a reference so as to simplify the code appearance
		int& ButtonTime = LocalButtonTimes[a-Button_FirstLocal];
		if (ButtonTime > 0)
		{
			ButtonTime--;
			continue;
		}
		
		assert(gVirtualElements[a]);
		err = ISpElement_GetSimpleState(gVirtualElements[a], &data);
		vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
		if(data == kISpButtonDown)
		{
			PostLocalEvent(gElementActions[a]);
			ButtonTime = LocalButtonRefractoryTime;
		}
	}
	
/* Handle all axis data!!! **********************************************/
	fixed delta_yaw = 0, delta_pitch = 0, delta_velocity = 0;
			
	ISpElement_GetComplexState(gVirtualElements[kAxisHoriz], sizeof(fixed), &delta_yaw);
	ISpElement_GetComplexState(gVirtualElements[kAxisVert], sizeof(fixed), &delta_pitch);
	ISpElement_GetComplexState(gVirtualElements[kAxisMove], sizeof(fixed), &delta_velocity);
	
	delta_pitch*= 2;
	
	if(delta_yaw != 0 || delta_pitch != 0 || delta_velocity != 0)
		flags= mask_in_absolute_positioning_information(flags, delta_yaw, delta_pitch, delta_velocity);
	
/* Done Handling all axis data!!! ****************************************/		
	return flags;
}

void ConfigureMarathonISpControls(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	OSErr err = noErr;
			
	err = ISpResume();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));

	if(input_preferences->input_device==_input_sprocket_only)  err = ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);
	else  err = ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpConfigure(nil);
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
	
	err = ISpSuspend();
	vassert(err == noErr, csprintf(temporary, "MacOS error code: %d", err));
}