//
//  MetalHelper.h
//  AlephOne
//
//  Created by Dustin Wenz on 2/19/21.
//

#include "SDL.h"

#ifndef MetalHelper_h
#define MetalHelper_h


extern void* injectMacOS(SDL_Window *main_screen);
extern void refreshMacOS(SDL_Window *main_screen);
extern void swapWindowMacOS(SDL_Window *main_screen);

#endif /* MetalHelper_h */

