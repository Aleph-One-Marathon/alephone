// Here is my extraction from the MoreFiles routines:

#include <Carbon.h>

#pragma once
extern
OSErr FSpGetFullPath(const FSSpec *spec,
					 short *fullPathLength,
					 Handle *fullPath);
/*	¦ Get a full pathname to a volume, directory or file.
	The GetFullPath function builds a full pathname to the specified
	object. The full pathname is returned in the newly created handle
	fullPath and the length of the full pathname is returned in
	fullPathLength. Your program is responsible for disposing of the
	fullPath handle.
	
	spec			input:	An FSSpec record specifying the object.
	fullPathLength	output:	The number of characters in the full pathname.
							If the function fails to create a full pathname,
							it sets fullPathLength to 0.
	fullPath		output:	A handle to the newly created full pathname
							buffer. If the function fails to create a
							full pathname, it sets fullPath to NULL.
	
	Result Codes
		noErr				0		No error	
		nsvErr				-35		No such volume
		ioErr				-36		I/O error
		bdNamErr			-37		Bad filename
		fnfErr				-43		File or directory does not exist
		paramErr			-50		No default volume
		memFullErr			-108	Not enough memory
		dirNFErr			-120	Directory not found or incomplete pathname
		afpAccessDenied		-5000	User does not have the correct access
		afpObjectTypeErr	-5025	Directory not found or incomplete pathname
	
	__________
	
	See also:	GetFullPath
*/
