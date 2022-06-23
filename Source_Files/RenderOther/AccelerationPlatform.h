//
//  AccelerationPlatform.h
//  AlephOne
//
//  Created by Dustin Wenz on 3/2/21.
//

#include <SDL2/SDL.h>

#ifndef AccelerationPlatform_h
#define AccelerationPlatform_h

extern void* injectAccelerationContext(SDL_Window *main_screen);
extern void refreshAccelerationContext(SDL_Window *main_screen);
extern void swapAcceleratedWindow(SDL_Window *main_screen);

#endif /* AccelerationPlatform_hpp */
