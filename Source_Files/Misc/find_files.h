#ifndef __FIND_FILES_H
#define __FIND_FILES_H

/*

Aug 25, 2000 (Loren Petrich):
	Abstracting the file handling
	
Aug 26, 2000 (Loren Petrich):
	Creating an object-oriented file finder

*/

class FileSpecifier;
class DirectorySpecifier;


/* Find all files of a given type.. */

// Finds every type of file
const int WILDCARD_TYPE = NONE;


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
	
#ifdef mac
	// MacOS-specific temporary stuff
	CInfoPBRec pb; /* static to prevent stack overflow.. */
	OSType type_to_find;
	OSErr Err;
#endif
	
	bool Enumerate(DirectorySpecifier& Dir);

public:
	short version;			/* Version Control (Set to 0)		<-  */
	short flags;			/* Search flags 					<-  */
	short search_type;		/* Search type						<-  */
	
	DirectorySpecifier BaseDir;
	int Type;				/* OSType to find					<-  */
	
	FileSpecifier *buffer; 	/* Destination						<-> */
	short max;				/* Maximum matches to return		<-  */
	short count;			/* Count of matches found 			->  */

	/* Callback	functions, if returns TRUE, you add it.  If */
	/*  callback==NULL, adds all found.							<-  */
	Boolean (*callback)(FileSpecifier& File, void *data);
	void *user_data;		/* Passed to callback above.		<-  */
	
	// Clears out the memory contents
	void Clear();
	
	// Does the finding
	bool Find();
	
#ifdef mac
	short GetError() {return Err;}
#endif
};

#endif
