#ifndef __FIND_FILES_H
#define __FIND_FILES_H

/* Find all files of a given type.. */

#define WILDCARD_TYPE '****'

enum {
	_fill_buffer, 				/* Fill the buffer with matches */
	_callback_only				/* Ignore the buffer, and call the callback for each. */
};

enum {
	_ff_create_buffer= 0x0001,	/* Create the destination buffer, free later w/ Dispose */
	_ff_alphabetical= 0x0002,	/* Matches are returned in alphabetical order */
	_ff_recurse= 0x0004,		/* Recurse when I find a subdirectory */
	_ff_callback_with_catinfo= 0x0008 /* Callback with CInfoPBRec as second paramter */
};

struct find_file_pb {
	short version;			/* Version Control (Set to 0)		<-  */
	short flags;			/* Search flags 					<-  */
	short search_type;		/* Search type						<-  */
	
	short vRefNum;			/* Search start 					<-  */
	long directory_id;		/* directory start 					<-  */
	OSType type_to_find;	/* OSType to find					<-  */
	
	FSSpec *buffer; 		/* Destination						<-> */
	short max;				/* Maximum matches to return		<-  */
	short count;			/* Count of matches found 			->  */

	/* Callback	functions, if returns TRUE, you add it.  If */
	/*  callback==NULL, adds all found.							<-  */
	Boolean (*callback)(FSSpec *file, void *data);
	void *user_data;		/* Passed to callback above.		<-  */
};

/* Find the files! */
OSErr find_files(struct find_file_pb *param_block);

/* Useful function for comparing FSSpecs... */
Boolean equal_fsspecs(FSSpec *a, FSSpec *b);

#endif
