// This is a "smart pointer" class, which deletes a pointed-to object when done.
// This was cribbed from an article on C++ in www.32bitsonline.com


#pragma once


template<class T> class SmartPtr {

	T* ptr;
	
	void clear_ptr() {if (ptr != 0) delete ptr;}

public:
	SmartPtr(T* _ptr = 0) {ptr = _ptr;}
	~SmartPtr() {clear_ptr();}
	
	// Pointer-like functionality
	T* operator->() {return ptr;}
	T& operator*() {return *ptr;}
	T* operator()() {return ptr;}
	operator T*() {return ptr;}
	T* operator=(T* _ptr) {clear_ptr(); ptr = _ptr; return ptr;}
	bool operator==(T* _ptr) {return (ptr == _ptr);}
	bool operator!=(T* _ptr) {return (ptr != _ptr);}
	// Won't implement + or - because this is not an array
	
	// More verbose versions
	// Is object present?
	bool present() {return (ptr != 0);}
	// Accept an object
	T* accept(T* _ptr) {clear_ptr(); ptr = _ptr; return ptr;}
	// Release it
	T* release() {T* _ptr = ptr; ptr = 0; return _ptr;}
	// Clear it
	void clear() {clear_ptr(); ptr = 0;}
};
