/*
 *  PlayerImage_sdl.cpp

	Copyright (C) 2001 and beyond by Woody Zenfell, III
	and the "Aleph One" developers.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

 */

#include	"PlayerImage_sdl.h"

#include	"world.h"
#include	"player.h"
#include	"interface.h"
#include	"shell.h"
#include	"collection_definition.h"

extern short bit_depth;

int16 PlayerImage::sNumOutstandingObjects = 0;

PlayerImage::~PlayerImage() {
    if(mLegsSurface != NULL)
        SDL_FreeSurface(mLegsSurface);
    
    if(mTorsoSurface != NULL)
        SDL_FreeSurface(mTorsoSurface);
        
    if(mLegsData != NULL)
        free(mLegsData);
        
    if(mTorsoData != NULL)
        free(mTorsoData);
        
    objectDestroyed();
}



void
PlayerImage::setRandomFlatteringView() {
    int16 theView;
    do {
        theView = local_random() % 8;
    } while(theView >= 3 && theView <= 5);
    setView(theView);
}



void
PlayerImage::updateLegsDrawingInfo() {
    // Remove old data, if applicable
    if(mLegsSurface != NULL) {
        SDL_FreeSurface(mLegsSurface);
        mLegsSurface = NULL;
    }
    
    if(mLegsData != NULL) {
        free(mLegsData);
        mLegsData = NULL;
    }
    
    mLegsValid = false;

    // Get the player shape definitions
    player_shape_definitions*	theShapeDefinitions	= get_player_shape_definitions();
    
    // tryAgain tells us whether we made any random choices in the loop.  If so, a failed effort results in
    // another trip through the loop (hopefully with different random choices :) ).
    bool	tryAgain = false;
    
    // For safety - don't keep looping if we seem not to be finding good data... eventually, give up as invalid.
    int16	theNumberOfTriesLeft = 100;

    do {
        // We're using up one of our attempts...
        theNumberOfTriesLeft--;
    
        // Find the leg action
        // We use a local here (instead of using the member directly) so we still which things to pick randomly if we loop.
        int16 theLegsAction = mLegsAction;
        if(mLegsAction == NONE) {
            theLegsAction = local_random() % NUMBER_OF_PLAYER_ACTIONS;
            tryAgain = true;
        }
        // If the user fed us a bad value, stop (with invalid data)
        else if(theLegsAction < 0 || theLegsAction >= NUMBER_OF_PLAYER_ACTIONS)
            break;
        
        // Find the high-level shape index
        uint16	theLegsHighLevelShapeIndex	= theShapeDefinitions->legs[theLegsAction];
        
        // Find out how many animation frames there are for the chosen legs
        shape_animation_data*	theLegsAnimationData = get_shape_animation_data(
                                    BUILD_DESCRIPTOR(theShapeDefinitions->collection, theLegsHighLevelShapeIndex));
        
        // If this failed, either give up or try again
        if(theLegsAnimationData == NULL)
            continue;
        
        // Find a view for the legs
        int16 theLegsView = mLegsView;
        if(theLegsView == NONE) {
            theLegsView = local_random() % 8;//theLegsAnimationData->number_of_views;
            tryAgain = true;
        }
        else if(theLegsView < 0 || theLegsView >= 8)
            break;
        
        // Find an animation frame
        int16 theLegsFrame = mLegsFrame;
        if(theLegsFrame == NONE) {
            theLegsFrame = local_random() % theLegsAnimationData->frames_per_view;
            tryAgain = true;
        }
        else if(theLegsFrame < 0 || theLegsFrame >= theLegsAnimationData->frames_per_view)
            break;

        // Calculate the low-level shape index index
        uint16 theLegsLowLevelShapeIndexIndex	= theLegsAnimationData->frames_per_view * theLegsView + theLegsFrame;
        
        // Finally, we can look up the low-level shape index
        uint16 theLegsLowLevelShapeIndex	= theLegsAnimationData->low_level_shape_indexes[theLegsLowLevelShapeIndexIndex];

        // Find a legs color
        int16 theLegsColor = mLegsColor;
        if(theLegsColor == NONE) {
            theLegsColor = local_random() % 8;
            tryAgain = true;
        }
        else if(theLegsColor < 0 || theLegsColor >= 8)
            break;
        
        low_level_shape_definition *theLegsLowLevelShape =
                    get_low_level_shape_definition(theShapeDefinitions->collection, theLegsLowLevelShapeIndex);
        
        if(theLegsLowLevelShape == NULL)
            continue;

        // Get the shape surfaces for the given collection, CLUT (according to color/team), and low-level shape index.
	if (bit_depth == 8) continue;
	mLegsSurface	= get_shape_surface(theLegsLowLevelShapeIndex,
                            BUILD_COLLECTION(theShapeDefinitions->collection, theLegsColor), &mLegsData, mLegsBrightness);

        if(mLegsSurface == NULL)
            continue;

        // Fill in rect information
        mLegsRect.x = -theLegsLowLevelShape->key_x;
        mLegsRect.y = -theLegsLowLevelShape->key_y;
        mLegsRect.w = mLegsSurface->w;
        mLegsRect.h = mLegsSurface->h;

        // We're clear.  Copy our temporary variables into the data members.
        mLegsAction	= theLegsAction;
        mLegsView	= theLegsView;
        mLegsFrame	= theLegsFrame;
        mLegsColor	= theLegsColor;
        
        // We are _valid_.  Sweet.
        mLegsValid	= true;

        // Success - get me outta here!
        break;

    } while(tryAgain && theNumberOfTriesLeft > 0);

    // Valid or not, the legs are no longer dirty.
    mLegsDirty = false;
}



void
PlayerImage::updateTorsoDrawingInfo() {
    // Remove old data, if applicable
    if(mTorsoSurface != NULL) {
        SDL_FreeSurface(mTorsoSurface);
        mTorsoSurface = NULL;
    }
    
    if(mTorsoData != NULL) {
        free(mTorsoData);
        mTorsoData = NULL;
    }
    
    mTorsoValid = false;

    // Get the player shape definitions
    player_shape_definitions*	theShapeDefinitions	= get_player_shape_definitions();
    
    // tryAgain tells us whether we made any random choices in the loop.  If so, a failed effort results in
    // another trip through the loop (hopefully with different random choices :) ).
    bool	tryAgain = false;
    
    // For safety - don't keep looping if we seem not to be finding good data... eventually, give up as invalid.
    int16	theNumberOfTriesLeft = 100;

    do {
        // We're using up one of our attempts...
        theNumberOfTriesLeft--;
    
        // Find the torso action
        // We use a local here (instead of using the member directly) so we still which things to pick randomly if we loop.
        int16 theTorsoAction = mTorsoAction;
        if(mTorsoAction == NONE) {
            theTorsoAction = local_random() % PLAYER_TORSO_WEAPON_ACTION_COUNT;
            tryAgain = true;
        }
        // If the user fed us a bad value, stop (with invalid data)
        else if(theTorsoAction < 0 || theTorsoAction >= PLAYER_TORSO_WEAPON_ACTION_COUNT)
            break;
        
        // Find a torso pseudo-weapon
        int16 thePseudoWeapon = mPseudoWeapon;
        if(mPseudoWeapon == NONE) {
            thePseudoWeapon = local_random() % PLAYER_TORSO_SHAPE_COUNT;
            tryAgain = true;
        }
        else if(thePseudoWeapon < 0 || thePseudoWeapon >= PLAYER_TORSO_SHAPE_COUNT)
            break;
        
        // Find the high-level shape index
        uint16	theTorsoHighLevelShapeIndex;
        
        switch(theTorsoAction) {
            
            case _shape_weapon_firing:
                theTorsoHighLevelShapeIndex = theShapeDefinitions->firing_torsos[thePseudoWeapon];
            break;
            
            case _shape_weapon_idle:
                theTorsoHighLevelShapeIndex = theShapeDefinitions->torsos[thePseudoWeapon];
            break;
            
            case _shape_weapon_charging:
                theTorsoHighLevelShapeIndex = theShapeDefinitions->charging_torsos[thePseudoWeapon];
            break;
            
            default:
                // This staves off a compiler warning
                theTorsoHighLevelShapeIndex = 0;
                assert(false);

        }
        
        // Find out how many animation frames there are for the chosen torso
        shape_animation_data*	theTorsoAnimationData = get_shape_animation_data(
                                    BUILD_DESCRIPTOR(theShapeDefinitions->collection, theTorsoHighLevelShapeIndex));
        
        // If this failed, either give up or try again
        if(theTorsoAnimationData == NULL)
            continue;
        
        // Find a view for the torso
        int16 theTorsoView = mTorsoView;
        if(theTorsoView == NONE) {
            theTorsoView = local_random() % 8;//theTorsoAnimationData->number_of_views;
            tryAgain = true;
        }
        else if(theTorsoView < 0 || theTorsoView >= 8)
            break;
        
        // Find an animation frame
        int16 theTorsoFrame = mTorsoFrame;
        if(theTorsoFrame == NONE) {
            theTorsoFrame = local_random() % theTorsoAnimationData->frames_per_view;
            tryAgain = true;
        }
        else if(theTorsoFrame < 0 || theTorsoFrame >= theTorsoAnimationData->frames_per_view)
            break;

        // Calculate the low-level shape index index
        uint16 theTorsoLowLevelShapeIndexIndex	= theTorsoAnimationData->frames_per_view * theTorsoView + theTorsoFrame;
        
        // Finally, we can look up the low-level shape index
        uint16 theTorsoLowLevelShapeIndex	= theTorsoAnimationData->low_level_shape_indexes[theTorsoLowLevelShapeIndexIndex];

        // Find a torso color
        int16 theTorsoColor = mTorsoColor;
        if(theTorsoColor == NONE) {
            theTorsoColor = local_random() % 8;
            tryAgain = true;
        }
        else if(theTorsoColor < 0 || theTorsoColor >= 8)
            break;
        
        low_level_shape_definition *theTorsoLowLevelShape =
                    get_low_level_shape_definition(theShapeDefinitions->collection, theTorsoLowLevelShapeIndex);
        
        if(theTorsoLowLevelShape == NULL)
            continue;

        // Get the shape surfaces for the given collection, CLUT (according to color/team), and low-level shape index.
	if (bit_depth == 8) continue;
	mTorsoSurface	= get_shape_surface(theTorsoLowLevelShapeIndex,
                                BUILD_COLLECTION(theShapeDefinitions->collection, theTorsoColor), &mTorsoData, mTorsoBrightness);

        // Argh, it failed.  Why don't we wait for backup?
        if(mTorsoSurface == NULL)
            continue;

        // Fill in rect information
        mTorsoRect.x = -theTorsoLowLevelShape->origin_x;
        mTorsoRect.y = -theTorsoLowLevelShape->origin_y;
        mTorsoRect.w = mTorsoSurface->w;
        mTorsoRect.h = mTorsoSurface->h;

        // We're clear.  Copy our temporary variables into the data members.
        mTorsoAction	= theTorsoAction;
        mPseudoWeapon	= thePseudoWeapon;
        mTorsoView	= theTorsoView;
        mTorsoFrame	= theTorsoFrame;
        mTorsoColor	= theTorsoColor;
        
        // We are _valid_.  Sweet.
        mTorsoValid	= true;

        // Success - get me outta here!
        break;

    } while(tryAgain && theNumberOfTriesLeft > 0);

    // Valid or not, the torso is no longer dirty.
    mTorsoDirty = false;
}



void
PlayerImage::drawAt(SDL_Surface* inSurface, int16 inX, int16 inY) {
    SDL_Rect	theWorkingRect;
    
    if(canDrawLegs()) {
        theWorkingRect		= mLegsRect;
        theWorkingRect.x	+= inX;
        theWorkingRect.y	+= inY;
        
        SDL_BlitSurface(mLegsSurface, NULL, inSurface, const_cast<SDL_Rect*>(&theWorkingRect));
        
//        printf("legs bottom at %d\n", theWorkingRect.y + theWorkingRect.h);
    }
    
    if(canDrawTorso()) {
        theWorkingRect		= mTorsoRect;
        theWorkingRect.x	+= inX;
        theWorkingRect.y	+= inY;
        
        SDL_BlitSurface(mTorsoSurface, NULL, inSurface, const_cast<SDL_Rect*>(&theWorkingRect));
        
//        printf("torso top at %d\n", theWorkingRect.y);
    }
}



void
PlayerImage::objectCreated() {
    if(sNumOutstandingObjects == 0) {
        mark_collection(get_player_shape_definitions()->collection, true);
        load_collections(false, false);
        // XXX (ZZZ) ugly hack, making sure we don't load multiple times.
//        sNumOutstandingObjects++;
    }
    
    sNumOutstandingObjects++;
}

void
PlayerImage::objectDestroyed() {
    sNumOutstandingObjects--;
    
    if(sNumOutstandingObjects == 0) {
        mark_collection(get_player_shape_definitions()->collection, false);
    }

}
