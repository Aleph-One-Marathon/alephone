/*
 *  TextLayoutHelper.cpp
 *  created for Marathon: Aleph One <http://source.bungie.org/>

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

 *  The code in this file is licensed to you under the GNU GPL.  As the copyright holder,
 *  however, I reserve the right to use this code as I see fit, without being bound by the
 *  GPL's terms.  This exemption is not intended to apply to modified versions of this file -
 *  if I were to use a modified version, I would be a licensee of whomever modified it, and
 *  thus must observe the GPL terms.
 *
 *  TextLayoutHelper assists with laying out a set of non-overlapping rectangles.  It's not
 *  as smart or as general as it could be, but it works.
 *
 *  Created by woody on Thu Nov 08 2001.
 */

#include	"TextLayoutHelper.h"

#include	<set>
#include	<assert.h>

TextLayoutHelper::TextLayoutHelper() {
}

TextLayoutHelper::~TextLayoutHelper() {
    removeAllReservations();
}

void
TextLayoutHelper::removeAllReservations() {
    CollectionOfReservationEnds::const_iterator i, end;
    i	= mReservationEnds.begin();
    end	= mReservationEnds.end();
    
    // Whack the reservation structures (only once per pair of reservation ends of course!)
    for(i = mReservationEnds.begin(); i != end; i++)
        if(i->mStartOfReservation == false)
            delete i->mReservation;
    
    mReservationEnds.clear();
}

int
TextLayoutHelper::reserveSpaceFor(int inLeft, unsigned int inWidth, int inLowestBottom, unsigned int inHeight) {
    Reservation*	theReservation = new Reservation;
    
    ReservationEnd	theLeftEnd;
    ReservationEnd	theRightEnd;
    
    theLeftEnd.mHorizontalCoordinate	= inLeft;
    theLeftEnd.mReservation		= theReservation;
    theLeftEnd.mStartOfReservation	= true;

    theRightEnd.mHorizontalCoordinate	= inLeft + inWidth;
    theRightEnd.mReservation		= theReservation;
    theRightEnd.mStartOfReservation	= false;
    
    // Walk through, finding place for left end.  Keep track of reservations opened and not closed.
    typedef std::multiset<Reservation*>	CollectionOfReservationPointers;
    CollectionOfReservationPointers	theReservations;

    CollectionOfReservationEnds::iterator	i	= mReservationEnds.begin();
    CollectionOfReservationEnds::const_iterator	end	= mReservationEnds.end();

    for( ; i != end; i++) {
        if(i->mHorizontalCoordinate > theLeftEnd.mHorizontalCoordinate)
            // Got to an entry that's past our start.  Stop.
            break;

        if(i->mStartOfReservation)
            // Start of reservation - we need to track it.
            theReservations.insert(i->mReservation);
        else
            // Reservation going out of scope - forget about it.
            theReservations.erase(i->mReservation);
    }
    
    // i points to end OR i points to first element after our left end.
    // Either way, put our left end where i points.
    i = mReservationEnds.insert(i, theLeftEnd);

    // Update the vector end.
    end = mReservationEnds.end();
    
    // Don't get hoisted on our own petard here...
    i++;
    
    // Continue our walk, recording new reservation openings (but not tracking closings).
    // Reservations that were open when we got here overlap our left side.  Reservations that open
    // before we can stick in our right end overlap our middle or right side.  Either way, they
    // could conflict with us, so we track them.
    for( ; i != end; i++) {
        if(i->mHorizontalCoordinate >= theRightEnd.mHorizontalCoordinate)
            // Found a reservation end outside our right end - stop here.
            break;
        
        // Only track reservation openings.
        if(i->mStartOfReservation)
            theReservations.insert(i->mReservation);
    }
    
    // i points to end OR points to first element at or after our right end.
    // Either way, put our right end where i points.
    mReservationEnds.insert(i, theRightEnd);
    
    // Now, walk through the reservations that overlap us in X-coordinate space.
    CollectionOfReservationPointers::const_iterator	end3			= theReservations.end();
    int							theCurrentBottom	= inLowestBottom;
    CollectionOfReservationPointers::const_iterator	k;
    do {
        k = theReservations.begin();
    
        for( ; k != end3; k++) {
			assert(inHeight == static_cast<unsigned int>(static_cast<int>(inHeight)));
			assert(0 <= static_cast<int>(inHeight));
            if(((*k)->mBottom > theCurrentBottom - static_cast<int>(inHeight)) && ((*k)->mTop < theCurrentBottom)) {
                // Found one that interferes with us.  Adjust our current bottom upwards.
                theCurrentBottom = (*k)->mTop;
                // We break (I hope this breaks only the for()!) so that the do-while can walk
                // through the whole list again to make sure our new location doesn't conflict.
                // I know, should probably sort and be smarter about this, oh well.
                break;
            }
        }
    } while(k != end3);
    
    // We've now found a bottom that interferes with no other rectangle in the reservation system.
    theReservation->mBottom	= theCurrentBottom;
    theReservation->mTop	= theCurrentBottom - inHeight;
    
    // theReservation's storage persists.  mReservationEnds now has a start and end for the newest reservation.
    
    // Tell the caller what we found for him.
    return theCurrentBottom;
}
