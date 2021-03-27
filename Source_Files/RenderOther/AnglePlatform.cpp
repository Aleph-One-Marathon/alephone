//
//  AnglePlatform.cpp
//  AlephOne
//
//  Created by Dustin Wenz on 3/2/21.
//

/*
 This is intended to hold any platform-specific code for implementing ANGLE on Windows, MacOS, Linux, etc.
 */

#include "AnglePlatform.h"
#include "Logging.h"

#ifdef __MACOSX__
    #include "MacOSHelper.h"
#endif

void* injectAngle(SDL_Window *main_screen)
{
    #ifdef __MACOSX__
        return injectMacOS(main_screen);
    #endif
        
    return NULL;
}

void refreshAngle(SDL_Window *main_screen)
{
    #ifdef __MACOSX__
        refreshMacOS(main_screen);
    #endif

}

void swapWindowAngle(SDL_Window *main_screen)
{
    #ifdef __MACOSX__
        swapWindowMacOS(main_screen);
    #endif
    
}
