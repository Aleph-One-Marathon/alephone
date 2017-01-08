/*
Copyright (C) 2011 Florian Zwoch
Copyright (C) 2011 Mark Olsen
Copyright (C) 2014 Eric Wasylishen
Copyright (C) 2016 Jeremiah Morris
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <SDL.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hidsystem/event_status_driver.h>
#include "mouse_cocoa.h"

struct osx_mouse_data
{
	SDL_mutex *mouse_mutex;
	bool should_exit;
	int mouse_x;
	int mouse_y;
};

struct osx_mouse_data *cur_mdata = NULL;

static void input_callback(void *ctx, IOReturn result, void *sender, IOHIDValueRef value)
{
	struct osx_mouse_data *mdata = static_cast<struct osx_mouse_data *>(ctx);
	IOHIDElementRef elem = IOHIDValueGetElement(value);
	uint32_t page = IOHIDElementGetUsagePage(elem);
	uint32_t usage = IOHIDElementGetUsage(elem);
	uint32_t val = IOHIDValueGetIntegerValue(value);

	if (page == kHIDPage_GenericDesktop) {
		switch (usage) {
			case kHIDUsage_GD_X:
				SDL_LockMutex(mdata->mouse_mutex);
				mdata->mouse_x += val;
				SDL_UnlockMutex(mdata->mouse_mutex);
				break;
			case kHIDUsage_GD_Y:
				SDL_LockMutex(mdata->mouse_mutex);
				mdata->mouse_y += val;
				SDL_UnlockMutex(mdata->mouse_mutex);
				break;
			default:
				break;
		}
	}
}

static int OSX_Mouse_Thread(void *ctx)
{
	if (!ctx)
		return 0;
	struct osx_mouse_data *mdata = static_cast<struct osx_mouse_data *>(ctx);
	
	IOHIDManagerRef hid_manager = IOHIDManagerCreate(kCFAllocatorSystemDefault, kIOHIDOptionsTypeNone);
	if (!hid_manager) {
		SDL_DestroyMutex(mdata->mouse_mutex);
		delete mdata;
		return 0;
	}
	
	if (IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
		CFRelease(hid_manager);
		SDL_DestroyMutex(mdata->mouse_mutex);
		delete mdata;
		return 0;
	}
	
	IOHIDManagerRegisterInputValueCallback(hid_manager, input_callback, mdata);
	IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		
	uint32_t page = kHIDPage_GenericDesktop;
	uint32_t usage = kHIDUsage_GD_Mouse;
	CFDictionaryRef dict = NULL;
	CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
	CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
	const void *keys[2] = { CFSTR(kIOHIDDeviceUsagePageKey), CFSTR(kIOHIDDeviceUsageKey) };
	const void *vals[2] = { pageNumRef, usageNumRef };

	if (pageNumRef && usageNumRef)
		dict = CFDictionaryCreate(kCFAllocatorDefault, keys, vals, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (pageNumRef)
		CFRelease(pageNumRef);
	if (usageNumRef)
		CFRelease(usageNumRef);
	IOHIDManagerSetDeviceMatching(hid_manager, dict);
	if (dict)
		CFRelease(dict);

	while (!mdata->should_exit) {
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false);
	}

	IOHIDManagerUnscheduleFromRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
	
	CFRelease(hid_manager);
	SDL_DestroyMutex(mdata->mouse_mutex);
	delete mdata;
	return 0;
}

int OSX_Mouse_Init(void)
{
	if (cur_mdata)
		OSX_Mouse_Shutdown();
	
	cur_mdata = new struct osx_mouse_data();
	if (!cur_mdata)
		return -1;

	cur_mdata->mouse_mutex = SDL_CreateMutex();
	if (!cur_mdata->mouse_mutex) {
		delete cur_mdata;
		cur_mdata = NULL;
		return -1;
	}
	
	SDL_Thread *thread = SDL_CreateThread(OSX_Mouse_Thread, "OSX_Mouse_Thread", cur_mdata);
	if (!thread) {
		SDL_DestroyMutex(cur_mdata->mouse_mutex);
		delete cur_mdata;
		cur_mdata = NULL;
		return -1;
	}
	
	SDL_DetachThread(thread);
	return 0;
}

void OSX_Mouse_Shutdown(void)
{
	if (!cur_mdata)
		return;

	cur_mdata->should_exit = true;
	cur_mdata = NULL;
}

void OSX_Mouse_GetMouseMovement(int *mouse_x, int *mouse_y)
{
	if (!cur_mdata)
		return;

	SDL_LockMutex(cur_mdata->mouse_mutex);
	*mouse_x = cur_mdata->mouse_x;
	*mouse_y = cur_mdata->mouse_y;
	cur_mdata->mouse_x = 0;
	cur_mdata->mouse_y = 0;
	SDL_UnlockMutex(cur_mdata->mouse_mutex);
}

