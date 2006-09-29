// This is a function object for getting the full path of an object

#pragma once

#include <Carbon.h>

#include "SimpleVec.h"


struct GetFullPathObject {

	// Function for doing the finding;
	// it returns whether it was successful or not
	bool GetFullPath(FSSpec *Spec);
	
	// MacOS error code;
	// set to noErr if there was some other error,
	// such as an allocation error
	OSErr Err;
	
	// Vector object for C string;
	// Its number of characters is get_len() - 1
	// (last one is null terminator)
	simple_vector<char> FullPath;
	
	// Constructor
	GetFullPathObject(): Err(noErr) {}
};
