/*
 *  precompiled-headers.h
 *  AlephOne-OSX
 *
 *  Created by Alexander Strange on Wed Jul 31 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */
#ifdef __GNUG__
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_image.h>

#if __GNUC__ >= 3
#import "cstypes.h"
#import "map.h"
#import "shell.h"
#import "render.h"
#import "OGL_Render.h"
#import "FileHandler.h"
#import "wad.h"
#import "preferences.h"
#import "scripting.h"
#import "network.h"
#import "mysound.h"
#import "csmacros.h"
#import "Model3D.h"
#import "dynamic_limits.h"
#import "effects.h"
#import "monsters.h"
#import "world.h"
#import "player.h"
#endif
#endif