#ifndef __FIND_FILES_H
#define __FIND_FILES_H

/*

Aug 25, 2000 (Loren Petrich):
	Abstracting the file handling
	
Aug 26, 2000 (Loren Petrich):
	Creating an object-oriented file finder

*/
#include "FileHandler.h"


/* Find all files of a given type.. */

// Finds every type of file
const int WILDCARD_TYPE = NONE;
// #define WILDCARD_TYPE '****'


enum {
	_fill_buffer, 				/* Fill the buffer with matches */
	_callback_only				/* Ignore the buffer, and call the callback for each. */
};

enum {
	// Filespec buffers are always created from outside,
	// and no alphabetical sorting is ever done
	// _ff_create_buffer= 0x0001,	/* Create the destination buffer, free later w/ Dispose */
	// _ff_alphabetical= 0x0002,	/* Matches are returned in alphabetical order */
	_ff_recurse= 0x0004,		/* Recurse when I find a subdirectory */
	_ff_callback_with_catinfo= 0x0008 /* Callback with CInfoPBRec as second paramter */
};

// File-finder object
class FileFinder
{
	// Temporary stuff:
	FileSpecifier TempFile;
	
	// MacOS-specific temporary stuff
	CInfoPBRec pb; /* static to prevent stack overflow.. */
	OSType type_to_find;
	OSErr Err;
	
	bool Enumerate(DirectorySpecifier& Dir);

	public:
// struct find_file_pb {
	short version;			/* Version Control (Set to 0)		<-  */
	short flags;			/* Search flags 					<-  */
	short search_type;		/* Search type						<-  */
	
	DirectorySpecifier BaseDir;
	// short vRefNum;			/* Search start 					<-  */
	// long directory_id;		/* directory start 					<-  */
	// Finds files using _typecode_whatever in tags.h
	int Type;				/* OSType to find					<-  */
	// OSType type_to_find;	/* OSType to find					<-  */
	
	FileSpecifier *buffer; 	/* Destination						<-> */
	// FSSpec *buffer; 		/* Destination						<-> */
	short max;				/* Maximum matches to return		<-  */
	short count;			/* Count of matches found 			->  */

	/* Callback	functions, if returns TRUE, you add it.  If */
	/*  callback==NULL, adds all found.							<-  */
	Boolean (*callback)(FileSpecifier& File, void *data);
	// Boolean (*callback)(FSSpec *file, void *data);
	void *user_data;		/* Passed to callback above.		<-  */
	
	// Clears out the memory contents
	void Clear();
	
	// Does the finding
	bool Find();
	
	short GetError() {return Err;}
};

/* Find the files! */
// OSErr find_files(struct find_file_pb *param_block);

/* Useful function for comparing FSSpecs... */
// Moved to operator== in FileHandler.h
// Boolean equal_fsspecs(FSSpec *a, FSSpec *b);

#endif
