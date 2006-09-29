// Here are some (relatively) simple vector and array objects
// set up using templates.

#pragma once


#include <algorithm.h>


// I'm defining this vector object here, because I sometimes
// cannot get STL's allocator to work properly on some objects
// (STL containers like "vector" are dependent on it).
// These will be rather simple, and resizable only rather crudely,
// but they will have some features I find useful.

template<class T> class simple_vector {

	int length;
	T *contents;

	void allocate()
		{if (length > 0) contents = new T[length]; else {length = 0; contents = 0;}}
	void deallocate() {if (length > 0) delete []contents;}

	void copy_in(T *newctnts) {for (int i=0; i<length; i++) contents[i] = newctnts[i];}

public:
	// Constructors and destructors
	simple_vector(int _length = 0): length(_length) {allocate();}
	simple_vector(simple_vector<T> &v): length(v.length) {allocate(); copy_in(v.contents);}
	
	// More STL-ish constructors
	simple_vector(int _length, T *v): length(_length)
		{allocate(); copy(v,v+length,contents);}
	simple_vector(T *v_begin, T *v_end)
		{length = v_end - v_begin; allocate(); copy(v_begin,v_end,contents);}
	~simple_vector() {deallocate();}

	// Accessors
	int get_len() {return length;}
	T &operator[](int indx) {return contents[indx];}
	// Imitation of pointer semantics
	T *operator+(int indx) {return contents + indx;}
		
	// Reallocation of internal space (destroys previous contents):
	void reallocate(int _length) {deallocate(); length = _length; allocate();}
	
	// Copies without changing original
	simple_vector &operator=(simple_vector<T> &v) {
		reallocate(v.length);
		copy_in(v.contents);
		return *this;
	}
	
	// Swaps contents
	void swap_with(simple_vector<T> &v) {
		swap(length,v.length);
		swap(contents,v.contents);
	}
	
	// Fills with some value
	void fill(const T& x) {
		for (int i=0; i<length; i++) contents[i] = x;
	}
	
	// STL beginnings and endings
	T *begin() {return contents;}
	T *end() {return (contents + length);}
	T *rbegin() {return (contents + (length - 1));}
	T *rend() {return (contents - 1);}
};


// This is an array class: a vector of vectors stored linearly
// The dimension of those vectors is N.

template<class T, int N> class simple_array {

	int length;
	T *contents;

	void allocate()
		{if (length > 0) contents = new T[N*length]; else {length = 0; contents = 0;}}
	void deallocate() {if (length != 0) delete []contents;}

	void copy_in(T *newctnts) {for (int i=0; i<N*length; i++) contents[i] = newctnts[i];}

public:
	// Constructors and destructors
	simple_array(int _length = 0): length(_length) {allocate();}
	simple_array(simple_array<T,N> &a): length(a.length) {allocate(); copy_in(a.contents);}

	~simple_array() {deallocate();}

	// Accessors
	int get_len() {return length;}
	int get_veclen() {return N;}
	T *operator[](int indx) {return (contents + N*indx);}
	
	// Reallocation of internal space (destroys previous contents):
	void reallocate(int _length) {deallocate(); length = _length; allocate();}
	
	// Copies without changing original
	simple_array &operator=(simple_array<T,N> &a) {
		reallocate(a.length);
		copy_in(a.contents);
		return *this;
	}
	
	// Swaps contents
	void swap_with(simple_array<T,N> &a) {
		swap(length,a.length);
		swap(contents,a.contents);
	}
	
	// Fills with some value
	void fill(const T& x) {
		for (int i=0; i<N*length; i++) contents[i] = x;
	}

	// No STL-vector compatibility here
};
