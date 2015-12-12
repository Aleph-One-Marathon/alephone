/*
 *  WindowedNthElementFinder.h

	Copyright (C) 2003 and beyond by Woody Zenfell, III
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

 *  Created by Woody Zenfell, III on Thu May 08 2003.
 *
 *  Finds the nth (0 is first) largest or nth smallest element from a window of recently inserted elements.
 */

#ifndef WINDOWEDNTHELEMENTFINDER_H
#define WINDOWEDNTHELEMENTFINDER_H

#include "CircularQueue.h"
#include <set>

template <typename tElementType>
class WindowedNthElementFinder {
public:
        WindowedNthElementFinder() : mQueue(0) {}
        
        explicit WindowedNthElementFinder(unsigned int inWindowSize) : mQueue(inWindowSize) {}

	void	reset() { reset(window_size()); }
        void	reset(unsigned int inWindowSize) { mQueue.reset(inWindowSize);  mSortedElements.clear(); }

        void	insert(const tElementType& inNewElement)
        {
                if(window_full())
                {
                        mSortedElements.erase(mSortedElements.find(mQueue.peek()));
                        mQueue.dequeue();
                }
                mSortedElements.insert(inNewElement);
                mQueue.enqueue(inNewElement);
        }

        // 0-based indexing (not 1-based as name might imply)
        const tElementType&	nth_smallest_element(unsigned int n)
        {
                assert(n < size());
                typename std::multiset<tElementType>::const_iterator i = mSortedElements.begin();
                for(unsigned int j = 0; j < n; ++j)
                        ++i;
                return *i;
        }

        // 0-based indexing (not 1-based as name might imply)
        const tElementType&	nth_largest_element(unsigned int n)
        {
                assert(n < size());
		typename std::multiset<tElementType>::const_reverse_iterator i = mSortedElements.rbegin();
                for(unsigned int j = 0; j < n; ++j)
                        ++i;
                return *i;
        }
        
        bool	window_full()		{ return size() == window_size(); }

        unsigned int size()		{ return mQueue.getCountOfElements(); }
        unsigned int window_size()	{ return mQueue.getTotalSpace(); }

private:
        CircularQueue<tElementType>	mQueue;
        std::multiset<tElementType>	mSortedElements;
};

#endif // WINDOWEDNTHELEMENTFINDER_H
