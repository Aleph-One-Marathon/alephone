/*
	Resizable-List object:
	a C++ template class.
	
	It will get resized in a lazy fashion.
	
	Jan 30, 2000 (Loren Petrich):
*/


#ifndef RESIZABLE_LIST
#define RESIZABLE_LIST


template<class T> class ResizableList
{
	T* List;		// The list itself
	int Length;		// Current size of list
	
	// Private functions to reduce clutter in public declarations below
	
	// THis one allocates a new array, and sets its length inside the []'s.
	bool New(int NewLength)
	{
		return ((List = new T[Length = NewLength]) != NULL);
	}
	
	// The reallocator
	bool LazyReallocate(int NewLength) {
		if (NewLength != Length)
		{
			delete []List;
			return New(NewLength);
		}
		
		return true;
	}
	
public:
	
	// Constructor takes the initial capacity
	ResizableList(int NewLength = 1) {New(NewLength);}
		
	// Get the current length and capacity
	int GetLength() {return Length;}
	bool SetLength(int NewLength) {return LazyReallocate(NewLength);}
	
	// Index validity check
	bool IsInRange(int Indx) {return (Indx >= 0 && Indx < Length);}
	
	// Reference to member (works like an array reference);
	// this routine has no validity checks (could be built in, if desired)
	T& operator[] (int Indx) {return List[Indx];}
	
	// Pointer to member (works like + on a pointer)
	T* operator+ (int Indx) {return List + Indx;}
	
	// Pointers to beginnings and endings (inspired by the Standard Template Library)
	
	// To first member
	T* Begin() {return List;}
	
	// To just after last member
	T* End() {return (List + Length);}
	
	// To last member
	T* RevBegin() {return (List + (Length-1));}
	
	// To just before first member
	T* RevEnd() {return (List - 1);}
	
	// Destructor
	ResizableList() {delete[]List;}
};


#endif
