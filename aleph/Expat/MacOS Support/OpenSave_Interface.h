// This is intended to be a cross-platform open/save dialog-box interface

#ifndef OPENSAVE_INTERFACE
#define OPENSAVE_INTERFACE

struct OpenParameters {
	
	// For Win32:
	// File suffix (should this be an array of suffixes?)
	char *Suffix;
	
	// For MacOS:
	// Number of type codes (max. 4; -1 means look at all files)
	// Type-code list
	short NumTypes;
	unsigned long TypeList[4];
	
	// For both:
	char *Prompt;
};

struct SaveParameters {

	// For Win32:
	// File suffix (should this be an array of suffixes?)
	char *Suffix;
	
	// For MacOS:
	// Type and creator codes:
	unsigned long TypeCode;
	unsigned long CreatorCode;
	
	// For both:
	char *Prompt;
	char *DefaultName;
};

// The character-string pointers will all point to C strings.
// Note: setting a character-string pointer to zero (or NULL, NIL, ...)
// indicates that it will not be used.

// These structs ought to have C++ constructors that will set everything to zero
// and other sensible defaults.

#ifdef __cplusplus
extern "C" {
#endif

extern char *OpenFile(struct OpenParameters *OpenParms);
extern char *SaveFile(struct SaveParameters *SaveParms);

#ifdef __cplusplus
}
#endif

// Both routines return a string containing the full pathname.
// Neither routine truly opens a file; that can be done with stdio or iostreams
// "SaveFile" creates the file to be saved, and replaces the original file with that one.
// A safe save would create a temporary file instead of replacing the file to be replaced,
// would swap identities when the saving is done, and then delete the new temporary file
// (originally the file to be replaced).

// The char strings here were allocated with malloc() instead of new,
// as a concession to plain-C programming.
// Deallocate with free() instead of delete.


#endif
