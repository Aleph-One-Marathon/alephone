/*
 *  TextLayoutHelper.h
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

#ifndef TEXTLAYOUTHELPER_H
#define	TEXTLAYOUTHELPER_H

// should eventually use list and some other sort mechanism, probably, for cheaper insertions.
#include <vector>

using std::vector;

class TextLayoutHelper {
public:
    TextLayoutHelper();
    ~TextLayoutHelper();
    
    // Remove all reservations
    void	removeAllReservations();
    
    // Reserve a place for the rectangle given.  Returns the smallest bottom that will not overlap previous reservations. 
    int 	reserveSpaceFor(int inLeft, unsigned int inWidth, int inLowestBottom, unsigned int inHeight);
    
protected:
    struct ReservationEnd;
    struct Reservation;
    
    typedef vector<ReservationEnd>	CollectionOfReservationEnds;
    
    CollectionOfReservationEnds		mReservationEnds;
};

struct TextLayoutHelper::ReservationEnd {
    int					mHorizontalCoordinate;
    TextLayoutHelper::Reservation*	mReservation;
    bool				mStartOfReservation;
};

struct TextLayoutHelper::Reservation {
    int		mBottom;
    int		mTop;
};

#endif//TEXTLAYOUTHELPER_H
