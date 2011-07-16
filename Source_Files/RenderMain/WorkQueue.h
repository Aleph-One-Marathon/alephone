#ifndef _WORKQUEUE_H
#define _WORKQUEUE_H

/*
    This class is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) the most recent version.

    This class is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this class; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    ---
    
	Work Queue
	by Ian Rickard
	Sept 06, 2001

	template class to handle a fifo list of objects	efficiently.  Used by Aleph
	One's new vis tree generator.
*/

#include <vector>

template <class T>
class WorkQueue {
	enum {
		chunkSize = 4
	};
	struct elm {
		T data;
		int next;
		elm() {next = -1;}
	};
	// these are prefxed with e to make it easier to cache local copys inside funcs
	int eNext, eLast;
	int eFirstFree, eLastFree;
	vector<elm> eQ;
public:
	WorkQueue() {eNext = eLast = eFirstFree = eLastFree = -1;}

	void push(const T &it);
	T* pop(); // data is guranteed valid until next push()
		
	void flush() {
		eQ.clear();
		eNext = eLast = eFirstFree = eLastFree = -1;
	}
	
	void preallocate(int size) {
		eQ.reserve(size);
	}
};


template <class T>
void WorkQueue<T>::push(const T &it) {
	// cache these so CW5 doesn't make this function suck donkeyballs
	vector<elm> *q = &eQ;
	int next=eNext, last=eLast;
	int firstFree=eFirstFree, lastFree=eLastFree;
	
	if (firstFree == -1) { // need to alocate more.
		q->reserve(eQ.size()+chunkSize);
		
		firstFree = lastFree = q->size();
		q->push_back(elm());
		
		for (int i=1 ; i<chunkSize ; i++) {
			lastFree = q[0][lastFree].next = q->size();
			q->push_back(elm());
		}
	}
	int thisOne = firstFree;
	
	// delink from free list.
	eFirstFree = firstFree = q[0][thisOne].next;
	q[0][thisOne].next = -1;
	
	if (firstFree == -1) { // this is the last free slot
		lastFree = -1;
	}
	
	eLastFree = lastFree;
	
	if (last == -1) {
		// only item in active list
		eNext = eLast = thisOne;
	} else {
		// insert at end of active list
		q[0][last].next = thisOne;
		eLast = thisOne;
	}
	// finally, fill in the data
	q[0][thisOne].data = it;
	if (eLastFree == eLast) Debugger();
}

template <class T>
T* WorkQueue<T>::pop() {
	vector<elm> *q = &eQ;
	// cache these so CW5 doesn't make this function suck donkeyballs
	int next=eNext, last=eLast;
	int lastFree=eLastFree;
	int thisOne = next;
	
	if (next == -1)
		return NULL;
	
	// delink it from the active list
	eNext = next = q[0][thisOne].next;
	q[0][thisOne].next = -1;
	if (next == -1) {
		eLast = -1;
	}
	
	// link it into the free list
	if (lastFree == -1) {
		eFirstFree = thisOne;
	} else {
		q[0][lastFree].next = thisOne;
	}
	eLastFree = thisOne;
	
	if (eLastFree == eLast) Debugger();
	
	return &(q[0][thisOne].data);
}

#endif
