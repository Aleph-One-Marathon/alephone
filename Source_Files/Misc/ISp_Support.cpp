
/* marathon includes */
#include "ISp_Support.h"
#include "macintosh_cseries.h"
#include "world.h"
#include "map.h"
#include "player.h"     // for get_absolute_pitch_range()
#include "shell.h"
#include "preferences.h"
#include <math.h>

//Private Function Prototypes

/* macintosh includes */
#include <InputSprocket.h>
#include <CursorDevices.h>
#include <Traps.h>

const OSType kCreatorCode		= '26.A';
const OSType kSubCreatorCode	= 'BENT';
#define	kResourceID_setl			128

enum
{
	kIconSuiteID_XThrust 		= 129, 
	kIconSuiteID_YThrust 		= 130,
	kIconSuiteID_ZThrust 		= 131,
	kIconSuiteID_XRotation		= 132,
	kIconSuiteID_YRotation		= 133,
	kIconSuiteID_ZRotation		= 134,
	kIconSuiteID_Look 			= 159,
	kIconSuiteID_Fire			= 141,
	kIconSuiteID_SecondaryFire	= 142,
	kIconSuiteID_ThrustForward	= 147,
	kIconSuiteID_ThrustBackward	= 148,
	kIconSuiteID_Pause			= 144,
	kIconSuiteID_Scroll			= 140,
	kIconSuiteID_LookLeft		= 149,
	kIconSuiteID_LookRight		= 150,
	kIconSuiteID_DeltaX			= 256,
	kIconSuiteID_DeltaY			= 257
};


enum {
	kBtnFirePrimary,
	kBtnMicrophone = 17,
	kBtnQuit,
	kAxisHoriz,
	kAxisVert,
	kAxisMove,
	kNeedCount = 22
};

static Boolean						canDoISp = true;
static Boolean						active = false;
static long							gElementActions[kNeedCount] = {
											_left_trigger_state, _right_trigger_state, 
											_moving_forward, 
											_moving_backward, _turning_left, 
											_turning_right, _sidestepping_left,
											_sidestepping_right, _looking_left,
											_looking_right, _looking_up,
											_looking_down, _looking_center,
											_cycle_weapons_backward, _cycle_weapons_forward,
											_action_trigger_state, _toggle_map, _microphone_button, 0, 0, 0, 0};
ISpElementListReference		gVirtualList = NULL;
ISpElementReference			gVirtualElements[kNeedCount] = {nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil};

// LP change:
#define kISpNeedFlag_Delta_AlreadyButton kISpNeedFlag_Axis_AlreadyButton

// LP change: removed the second "0," so that it fits with my version of the ISp SDK
static ISpNeed gNeeds[kNeedCount] =
{
	// Standard Button Items
	{ "\pFire Primary", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_Fire,			kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pFire Secondary", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_SecondaryFire,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pMove Forward",		kIconSuiteID_ZThrust, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_MoveForward,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pMove Backward",	kIconSuiteID_ZThrust, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_MoveBackward,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pLook Left", 		kIconSuiteID_LookLeft,		0, kISpElementKind_Button,		kISpElementLabel_Btn_TurnLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pLook Right", 		kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_TurnRight,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pSidestep Left", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_SlideLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pSidestep Right", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_SlideRight,	kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pGlance Left",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_LookLeft,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pGlance Right", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_LookRight,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pLook Up",		 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_LookUp,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pLook Down",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_LookDown,		kISpNeedFlag_Button_AlreadyAxis | kISpNeedFlag_Button_ActiveWhenDown, 0, 0, 0	},
	{ "\pLook Ahead",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pPrevious Weapon", 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_Previous,		kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pNext Weapon",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_Next,			kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pAction", 			kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_Btn_Select,		kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pAuto Map", 		kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pMicrophone",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	{ "\pQuit",				kIconSuiteID_ZThrust, 		0, kISpElementKind_Button,		kISpElementLabel_Btn_PauseResume,	kISpNeedFlag_Utility , 0, 0, 0	},

	// Standard Delta Items
	{ "\pLook Horizontal",	kIconSuiteID_LookRight, 	0, kISpElementKind_Delta,		kISpElementLabel_Delta_Yaw,			kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0	},
	{ "\pLook Vertical",	kIconSuiteID_LookRight, 	0, kISpElementKind_Delta,		kISpElementLabel_Delta_Pitch,		kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0	},
	{ "\pMove",				kIconSuiteID_LookRight, 	0, kISpElementKind_Delta,		kISpElementLabel_Delta_Z,			kISpNeedFlag_Delta_AlreadyButton , 0, 0, 0	},

	//  Utitilty Class Items
	/*
	{ "\pMicrophone",	 	kIconSuiteID_LookRight, 	0, kISpElementKind_Button,		kISpElementLabel_None,				kISpNeedFlag_Button_AlreadyAxis , 0, 0, 0	},
	*/
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
	assert(err == noErr);
	
	err = ISpElement_NewVirtualFromNeeds(kNeedCount, gNeeds, gVirtualElements, 0);
	assert(err == noErr);
	
	err = ISpElementList_New(
			0,				// count
			NULL,		// needs
			&gVirtualList,			// virtual elements
			0);
	assert(err == noErr);
	
	for(int a = 0; a < kNeedCount; a++)
	{
		err  = ISpElementList_AddElements (gVirtualList, 
					a, 		1, &gVirtualElements[a]);
		assert(err == noErr);
	}
	
	err = ISpInit(kNeedCount,	// count
		gNeeds,				// needs
		gVirtualElements,	// virtual elements
		kCreatorCode,		// app
		kSubCreatorCode,	// sub (we are using as a versioning)
		0,					// flags
		kResourceID_setl,	// set list resource id
		0);					// reserved
	assert(err == noErr);
		
	err = ISpSuspend();
	assert(err == noErr);
}

void ShutDown_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	OSErr err = noErr;
	err = ISpStop();
	assert(err == noErr);
	
	err = ISpElement_DisposeVirtual(kNeedCount, gVirtualElements);
	assert(err == noErr);
	
	ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	ISpDevices_DeactivateClass(kISpDeviceClass_Mouse);
	
	err = ISpShutdown();
	assert(err == noErr);
}

void Start_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	if(input_preferences->input_device!=_input_sprocket_only && input_preferences->input_device!=_keyboard_or_game_pad) return;
	OSErr err = noErr;
	
	if(input_preferences->input_device==_input_sprocket_only)  err = ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);
	else  err = ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	assert(err == noErr);
	
	err = ISpResume();
	assert(err == noErr);
	active = true;
}

void Stop_ISp(void)
{
	if(!canDoISp) return; // Leave function because Input Sprockets is not available
	OSErr err = noErr;
	
	err = ISpSuspend();
	assert(err == noErr);
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
	assert(err == noErr);

//Special Cases	
	assert(gVirtualElements[kBtnQuit]);
	err = ISpElement_GetSimpleState(gVirtualElements[kBtnQuit], &data);
	assert(err == noErr);
	if(data == kISpButtonDown)
	{
		Stop_ISp();
		return 0;
	}
	
//Check each of the action keys
	for(int a = kBtnFirePrimary; a < kBtnMicrophone; a++)
	{
		assert(gVirtualElements[a]);
		err = ISpElement_GetSimpleState(gVirtualElements[a], &data);
		assert(err == noErr);
		if(data == kISpButtonDown) flags |= gElementActions[a];
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
	assert(err == noErr);

	if(input_preferences->input_device==_input_sprocket_only)  err = ISpDevices_ActivateClass(kISpDeviceClass_Keyboard);
	else  err = ISpDevices_DeactivateClass(kISpDeviceClass_Keyboard);
	assert(err == noErr);
	
	err = ISpConfigure(nil);
	assert(err==noErr);
	
	err = ISpSuspend();
	assert(err == noErr);
}