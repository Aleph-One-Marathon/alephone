/*
	Growable-List object:
	a C++ template class.
	
	One adds objects to it and it will automatically expand to accommodate them.
	
	Jan 30, 2000 (Loren Petrich):
*/


#ifndef GROWABLE_LIST
#define GROWABLE_LIST


template<class T> class GrowableList
{
	T* List;		// The list itself
	int Length;		// Current effective size of list
	int Capacity;	// Current capacity of list
	
	// Private functions to reduce clutter in public declarations below
	// This one zeroes out the length and allocates a new list
	bool New(int _Capacity)
	{
		// Keep the capacity greater than 0:
		Capacity = _Capacity > 0 ? _Capacity : 1;
		
		// Reset the length
		Length = 0;
		
		// Allocate!
		bool Success = ((List = new T[Capacity]) != NULL);
		if (!Success)
		{
			// Failure fallback
			Capacity = 1;
			List = new T[Capacity];
		}
		return Success;
	}
	
	// This is the reallocation part; it will not extend the list
	bool Add_Reallocate()
	{
		// Copy the contents into a new array if necessary
		if (Length >= Capacity)
		{
			int NewCapacity = 2*Capacity;
			T* NewList;
			// Exit here if new member could not be added
			if((NewList = new T[NewCapacity]) == NULL) return false;
			for (int k=0; k<Capacity; k++)
				NewList[k] = List[k];
			delete []List;
			List = NewList;
			Capacity = NewCapacity;
		}
		return true;
	}
	
public:
	
	// Constructor takes the initial capacity
	GrowableList(int _Capacity = 1) {New(_Capacity);}
	
	// Add a new member that is copied in;
	// returns whether reallocation succeeded
	bool Add(const T& NewMember)
		{if (!Add_Reallocate()) return false; List[Length++] = NewMember; return true;}
	
	// Add a new member using its default constructor;
	// returns whether reallocation succeeded
	bool Add() {if (!Add_Reallocate()) return false; Length++; return true;}
	
	// Get the current length and capacity
	int GetLength() {return Length;}
	int GetCapacity() {return Capacity;}
	
	// Index validity check
	bool IsInRange(int Indx) {return (Indx >= 0 && Indx < Length);}
	
	// Reference to member (works like an array reference);
	// this routine has no validity checks (could be built in, if desired)
	T& operator[] (int Indx) {return List[Indx];}
		
	// Reset the length to zero
	void ResetLength() {Length = 0;}
	
	// Reset the capacity to some value; returns whether the reallocation succeeded
	bool ResetCapacity(int _Capacity = 1) {delete []List; return New(_Capacity);}
	
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
	~GrowableList() {delete[]List;}
};


#endif
