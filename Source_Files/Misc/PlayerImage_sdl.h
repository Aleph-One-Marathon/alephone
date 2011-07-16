/*
 *  PlayerImage_sdl.h

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

#ifndef	PLAYERIMAGE_SDL_H
#define	PLAYERIMAGE_SDL_H

#include	"cseries.h"

class PlayerImage {
public:
    // Class will mark and load collection when needed; unmark when done.
    
    // Default constructor will randomly choose everything needed to draw a player image.
    // You may change state values after construction, and no effort is wasted - expensive image
    // fetching/interpretation is only performed when needed.
    PlayerImage() :
        mLegsView(NONE),
        mTorsoView(NONE),
        mLegsColor(NONE),
        mTorsoColor(NONE),
        mLegsAction(NONE),
        mTorsoAction(NONE),
        mPseudoWeapon(NONE),
        mLegsFrame(NONE),
        mTorsoFrame(NONE),
        mLegsBrightness(1.0f),
        mTorsoBrightness(1.0f),
        mTiny(false),
        
        mLegsDirty(true),
        mTorsoDirty(true),
        mLegsValid(false),
        mTorsoValid(false),
        
        mLegsSurface(NULL),
        mTorsoSurface(NULL),
        mLegsData(NULL),
        mTorsoData(NULL)
        
        { objectCreated(); }
    
    ~PlayerImage();

    // In all "set" routines, a value of NONE (-1) means to pick one at random.
    // If randomly picking values results in something that's easily detected as invalid (e.g., got a NULL pointer
    // somewhere), values will be re-picked.  If user specified any out-of-range values, or if re-picking failed
    // too many times, you'll get an invalid torso or legs, and they won't be drawn.  Use the status inquiry functions
    // below to see whether valid values were found.
    
    // Views: 0 through 7.  0 is facing the viewer; each increment rotates player to HIS right 45 degrees.
    void	setLegsView(int16 inView)	{ if(mLegsView != inView)	{ mLegsView = inView;	mLegsDirty = true; } }
    void	setTorsoView(int16 inView)	{ if(mTorsoView != inView)	{ mTorsoView = inView;	mTorsoDirty = true; } }
    void	setView(int16 inView)		{ setLegsView(inView);	setTorsoView(inView); }
    void	setRandomFlatteringView(); // picks torso and legs at same view; avoids "butt shots"
    
    // Colors: 0 through 7.  The standard player/team colors.
    void	setLegsColor(int16 inColor)	{ if(mLegsColor != inColor)	{ mLegsColor = inColor;	mLegsDirty = true; } }
    void	setTeamColor(int16 inColor)	{ setLegsColor(inColor); }
    void	setTorsoColor(int16 inColor)	{ if(mTorsoColor != inColor)	{ mTorsoColor = inColor; mTorsoDirty = true; } }
    void	setPlayerColor(int16 inColor)	{ setTorsoColor(inColor); }
    void	setColor(int16 inColor)		{ setLegsColor(inColor); setTorsoColor(inColor); }
    
    // Leg actions: _player_running, etc. from player.h
    void	setLegsAction(int16 inAction)	{ if(mLegsAction != inAction)	{ mLegsAction = inAction; mLegsDirty = true; } }
    // Torso actions: _shape_weapon_firing, etc. from weapons.h
    void	setTorsoAction(int16 inAction)	{ if(mTorsoAction != inAction)	{ mTorsoAction = inAction; mTorsoDirty = true; } }
    void	setRandomAction()		{ setLegsAction(NONE);	setTorsoAction(NONE); }
    // Pseudo-weapon: _weapon_missile_launcher, etc. from weapons.h
    void	setPseudoWeapon(int16 inWeapon)	{ if(mPseudoWeapon != inWeapon)	{ mPseudoWeapon = inWeapon; mTorsoDirty = true; } }
    
    // Frame: numbered 0 through...?  (depends on action, whether torso or legs, etc.)
    void	setLegsFrame(int16 inFrame)	{ if(mLegsFrame != inFrame)	{ mLegsFrame = inFrame;	mLegsDirty = true; } }
    void	setTorsoFrame(int16 inFrame)	{ if(mTorsoFrame != inFrame)	{ mTorsoFrame = inFrame; mTorsoDirty = true; } }
    void	setRandomFrame()		{ setLegsFrame(NONE);	setTorsoFrame(NONE); }

    // Brightness: 0.0f - 1.0f
    void    setLegsBrightness(float inBrightness) {
                if(mLegsBrightness != inBrightness) { mLegsBrightness = inBrightness; mLegsDirty = true; } }
    void    setTorsoBrightness(float inBrightness) {
                if(mTorsoBrightness != inBrightness) { mTorsoBrightness = inBrightness; mTorsoDirty = true; } }
    void    setBrightness(float inBrightness) { setLegsBrightness(inBrightness); setTorsoBrightness(inBrightness); }
    
    // Tiny (quarter-sized): true or false
    void    setTiny(bool inTiny) { if(mTiny != inTiny) { mTiny = inTiny; mTorsoDirty = true; mLegsDirty = true; } }


    // Update routines are called to synchronize drawing data (expensive) with state
    // Users should usually not call these - state checking and drawing routines will do it for you.
    // Left them public just in case someone wants to force an update, e.g. to prefetch data for faster drawing later.
    void	updateLegsDrawingInfo();
    void	updateTorsoDrawingInfo();
    void	updateDrawingInfo() {
                    if(mLegsDirty) 	updateLegsDrawingInfo();
                    if(mTorsoDirty)	updateTorsoDrawingInfo();
                }


    // Status checking routines will make sure status is up-to-date before returning value
    bool	canDrawLegs()	{ updateDrawingInfo(); return mLegsValid; }
    bool	canDrawTorso()	{ updateDrawingInfo(); return mTorsoValid; }
    bool	canDrawSomething()	{ updateDrawingInfo(); return mLegsValid || mTorsoValid; }
    bool	canDrawPlayer()	{ updateDrawingInfo(); return mLegsValid && mTorsoValid; }
    
    // I should really have a full complement of these, and also perhaps give the option to resolve
    // NONEs to a value, or not.
    int16	getTeam() { updateDrawingInfo(); return mLegsColor; }
    
    
    // Drawing routines will call update routines to make sure they draw up-to-date images.
    void	drawAt(SDL_Surface* inSurface, int16 inX, int16 inY);
    
    
protected:
    // STATE DATA
    int16		mLegsView;	// which way the legs appear to face
    int16		mTorsoView;	// which way the torso appears to face
    int16		mLegsColor;	// usually corresponds to player's team-color
    int16		mTorsoColor;	// usually corresponds to player's color
    int16		mLegsAction;	// what the legs appear to be doing
    int16		mTorsoAction;	// what the torso appears to be doing
    int16		mPseudoWeapon;	// what weapon the torso appears to be holding
    int16		mLegsFrame;	// which frame of animation the legs are at
    int16		mTorsoFrame;	// which frame of animation the torso is at
    float       mLegsBrightness;    // how well-lit the legs are
    float       mTorsoBrightness;   // how well-lit the torso is
    bool		mTiny;		// whether the player icon is quarter-sized
    
    // STATE <-> DRAWING DATA LINK
    bool		mLegsDirty;	// legs drawing info needs to be updated before the next draw
    bool		mTorsoDirty;	// torso drawing info needs to be updated before the next draw
    
    bool		mLegsValid;	// there is valid leg-drawing data available
    bool		mTorsoValid;	// there is valid torso-drawing data available
    
    // DRAWING DATA
    SDL_Surface*	mLegsSurface;	// source image for legs
    SDL_Surface*	mTorsoSurface;	// source image for torso
    byte*		mLegsData;	// extra data we must hold onto and free() when done
    byte*		mTorsoData;	// extra data we must hold onto and free() when done
    SDL_Rect		mDrawRect;	// such that the player image has origin at 0,0
    SDL_Rect		mLegsRect;	// such that the player image has origin at 0,0
    SDL_Rect		mTorsoRect;	// such that the player image has origin at 0,0
    
    
    // CLASS DATA
    static int16	sNumOutstandingObjects;	// count of objects created but not destroyed; used to mark/unmark collections
    
    // CLASS INTERNAL METHODS
    static void		objectCreated();
    static void		objectDestroyed();
};

#endif//PLAYERIMAGE_SDL_H
