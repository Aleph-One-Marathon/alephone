#ifndef _FILE_HANDLER_
#define _FILE_HANDLER_
/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html
	
	File-handler classes
	by Loren Petrich,
	August 11, 2000

	These are designed to provide some abstract interfaces to file and directory objects.
	
	Most of these routines return whether they had succeeded;
	more detailed error codes are API-specific.
	Attempted to support stdio I/O directly, but on my Macintosh, at least,
	the performance was much poorer. This is possibly due to "fseek" having to
	actually read the file or something.
	
	Merged all the Macintosh-specific code into these base classes, so that
	it will be selected with a preprocessor statement when more than one file-I/O
	API is supported.

Dec 7, 2000 (Loren Petrich):
	Added a MacOS-specific file-creation function that allows direct specification
	of type and creator codes

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Added TARGET_API_MAC_CARBON for Carbon.h
	Rearranged initializers in DirectorySpecifier constructor to appease compiler warnings

March 18, 2002 (Br'fin (Jeremy Parsons)):
	Added FileSpecifier::SetParentToResources for Carbon
*/

// For the filetypes
#include "tags.h"

#include <stddef.h>	// For size_t
#include <time.h>	// For time_t
#include <vector>
#include <SDL.h>

#include <errno.h>
#include <string>
#ifndef NO_STD_NAMESPACE
using std::string;
using std::vector;
#endif

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>

// Returned by .GetError() for unknown errors
constexpr int unknown_filesystem_error = -1;

/*
	Abstraction for opened files; it does reading, writing, and closing of such files,
	without doing anything to the files' specifications
*/
class OpenedFile
{
	// This class will need to set the refnum and error value appropriately 
	friend class FileSpecifier;
	friend class opened_file_device;
	
public:
	bool IsOpen();
	bool Close();
	
	bool GetPosition(int32& Position);
	bool SetPosition(int32 Position);
	
	bool GetLength(int32& Length);
	bool SetLength(int32 Length);
	
	bool Read(int32 Count, void *Buffer);
	bool Write(int32 Count, void *Buffer);
		
	OpenedFile();
	~OpenedFile() {Close();}	// Auto-close when destroying

	int GetError() {return err;}
	SDL_RWops *GetRWops() {return f;}
	SDL_RWops *TakeRWops();		// Hand over SDL_RWops

private:
	SDL_RWops *f;	// File handle
	int err;		// Error code
	bool is_forked;
	int32 fork_offset, fork_length;
};

class opened_file_device {
public:
	typedef char char_type;
	typedef boost::iostreams::seekable_device_tag category;
	std::streamsize read(char* s, std::streamsize n);
	std::streamsize write(const char* s, std::streamsize n);
	std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

	opened_file_device(OpenedFile& f);

private:
	OpenedFile& f;
};

/*
	Abstraction for loaded resources;
	this object will release that resource when it finishes.
	MacOS resource handles will be assumed to be locked.
*/
class LoadedResource
{
	// This class grabs a resource to be loaded into here
	friend class OpenedResourceFile;
	
public:
	// Resource loaded?
	bool IsLoaded();
	
	// Unloads the resource
	void Unload();
	
	// Get size of loaded resource
	size_t GetLength();
	
	// Get pointer (always present)
	void *GetPointer(bool DoDetach = false);

	// Make resource from raw resource data; the caller gives up ownership
	// of the pointed to memory block
	void SetData(void *data, size_t length);
	
	LoadedResource();
	~LoadedResource() {Unload();}	// Auto-unload when destroying

private:
	// Detaches an allocated resource from this object
	// (keep private to avoid memory leaks)
	void Detach();

public:
	void *p;		// Pointer to resource data (malloc()ed)
	size_t size;	// Size of data
};


/*
	Abstraction for opened resource files:
	it does opening, setting, and closing of such files;
	also getting "LoadedResource" objects that return pointers
*/
class OpenedResourceFile
{
	// This class will need to set the refnum and error value appropriately 
	friend class FileSpecifier;
	
public:
	
	// Pushing and popping the current file -- necessary in the MacOS version,
	// since resource forks are globally open with one of them the current top one.
	// Push() saves the earlier top one makes the current one the top one,
	// while Pop() restores the earlier top one.
	// Will leave SetResLoad in the state of true.
	bool Push();
	bool Pop();

	// Pushing and popping are unnecessary for the MacOS versions of Get() and Check()
	// Check simply checks if a resource is present; returns whether it is or not
	// Get loads a resource; returns whether or not one had been successfully loaded
	// CB: added functions that take 4 characters instead of uint32, which is more portable
	bool Check(uint32 Type, int16 ID);
	bool Check(uint8 t1, uint8 t2, uint8 t3, uint8 t4, int16 ID) {return Check(FOUR_CHARS_TO_INT(t1, t2, t3, t4), ID);}
	bool Get(uint32 Type, int16 ID, LoadedResource& Rsrc);
	bool Get(uint8 t1, uint8 t2, uint8 t3, uint8 t4, int16 ID, LoadedResource& Rsrc) {return Get(FOUR_CHARS_TO_INT(t1, t2, t3, t4), ID, Rsrc);}

	bool IsOpen();
	bool Close();
	
	OpenedResourceFile();
	~OpenedResourceFile() {Close();}	// Auto-close when destroying

	int GetError() {return err;}

private:
	int err;		// Error code
	SDL_RWops *f, *saved_f;
};


// Directories are treated like files
#define DirectorySpecifier FileSpecifier

// Directory entry, returned by FileSpecifier::ReadDirectory()
struct dir_entry {
	dir_entry() : is_directory(false), date(0) {}
	dir_entry(const string& n, bool is_dir, TimeType d = 0) : name(n), is_directory(is_dir), date(d) {}

	bool operator<(const dir_entry &other) const
	{
		if (is_directory == other.is_directory)
			return name < other.name;
		else	// Sort directories before files
			return is_directory > other.is_directory;
	}

	bool operator==(const dir_entry& other) const {
		return is_directory == other.is_directory && name == other.name;
	}

	string name;		// Entry name
	bool is_directory;	// Entry is a directory (plain file otherwise)
	TimeType date;          // modification date
};


/*
	Abstraction for file specifications;
	designed to encapsulate both directly-specified paths
	and MacOS FSSpecs
*/
class FileSpecifier
{	
public:
	// The typecodes here are the symbolic constants defined in tags.h (_typecode_creator, etc.)
	
	// Get the name (final path element) as a C string:
	// assumes enough space to hold it if getting (max. 256 bytes)
	void GetName(char *Name) const;
	
	//   Looks in all directories in the current data search
	//   path for a file with the relative path "NameWithPath" and
	//   sets the file specifier to the full path of the first file
	//   found.
	// "NameWithPath" follows Unix-like syntax: <dirname>/<dirname>/<dirname>/filename
	// A ":" will be translated into a "/" in the MacOS.
	// Returns whether or not the setting was successful
	bool SetNameWithPath(const char *NameWithPath);
	bool SetNameWithPath(const char* NameWithPath, const DirectorySpecifier& Directory);

	void SetTempName(const FileSpecifier& other);

	// Move the directory specification
	void ToDirectory(DirectorySpecifier& Dir);
	void FromDirectory(DirectorySpecifier& Dir);

	// These functions take an appropriate one of the typecodes used earlier;
	// this is to try to cover the cases of both typecode attributes
	// and typecode suffixes.
	bool Create(Typecode Type);
	
	// Opens a file:
	bool Open(OpenedFile& OFile, bool Writable=false);
	bool OpenForWritingText(OpenedFile& OFile); // converts LF to CRLF on Windows
	
	// Opens either a MacOS resource fork or some imitation of it:
	bool Open(OpenedResourceFile& OFile, bool Writable=false);
	
	// These calls are for creating dialog boxes to set the filespec
	// A null pointer means an empty string
	bool ReadDialog(Typecode Type, const char *Prompt=NULL);
	bool WriteDialog(Typecode Type, const char *Prompt=NULL, const char *DefaultName=NULL);
	
	// Write dialog box for savegames (must be asynchronous, allowing the sound
	// to continue in the background)
	bool WriteDialogAsync(Typecode Type, char *Prompt=NULL, char *DefaultName=NULL);
	
	// Check on whether a file exists, and its type
	bool Exists();
	bool IsDir();
	
	// Gets the modification date
	TimeType GetDate();
	
	// Returns _typecode_unknown if the type could not be identified;
	// the types returned are the _typecode_stuff in tags.h
	Typecode GetType();

	// How many bytes are free in the disk that the file lives in?
	bool GetFreeSpace(uint32& FreeSpace);
	
	// Copy file contents
	bool CopyContents(FileSpecifier& File);
	
	// Delete file
	bool Delete();

	// Rename file
	bool Rename(const FileSpecifier& Destination);

	// Copy file specification
	const FileSpecifier &operator=(const FileSpecifier &other);

	// hide extensions known to Aleph One
	static std::string HideExtension(const std::string& filename);
	
	const char *GetPath(void) const {return name.c_str();}

	FileSpecifier();
	FileSpecifier(const string &s) : name(s), err(0) {canonicalize_path();}
	FileSpecifier(const char *s) : name(s), err(0) {canonicalize_path();}
	FileSpecifier(const FileSpecifier &other) : name(other.name), err(other.err) {}

	bool operator==(const FileSpecifier &other) const {return name == other.name;}
	bool operator!=(const FileSpecifier &other) const {return name != other.name;}

	void SetToLocalDataDir();		// Per-user directory (for temporary files)
	void SetToPreferencesDir();		// Directory for preferences (per-user)
	void SetToSavedGamesDir();		// Directory for saved games (per-user)
	void SetToQuickSavesDir();		// Directory for auto-named saved games (per-user)
	void SetToImageCacheDir();		// Directory for image cache (per-user)
	void SetToRecordingsDir();		// Directory for recordings (per-user)

	void AddPart(const string &part);
	FileSpecifier &operator+=(const FileSpecifier &other) {AddPart(other.name); return *this;}
	FileSpecifier &operator+=(const string &part) {AddPart(part); return *this;}
	FileSpecifier &operator+=(const char *part) {AddPart(string(part)); return *this;}
	FileSpecifier operator+(const FileSpecifier &other) const {FileSpecifier a(name); a.AddPart(other.name); return a;}
	FileSpecifier operator+(const string &part) const {FileSpecifier a(name); a.AddPart(part); return a;}
	FileSpecifier operator+(const char *part) const {FileSpecifier a(name); a.AddPart(string(part)); return a;}

	void SplitPath(string &base, string &part) const;
	void SplitPath(DirectorySpecifier &base, string &part) const {string b; SplitPath(b, part); base = b;}

	bool CreateDirectory();
	
	// Return directory contents (following symlinks), excluding dot-prefixed files
	bool ReadDirectory(vector<dir_entry> &vec);
	vector<dir_entry> ReadDirectory() {vector<dir_entry> vec; ReadDirectory(vec); return vec;}
	
	// Return the names of all entries in a ZIP archive
	bool ReadZIP(vector<string> &vec);
	vector<string> ReadZIP() {vector<string> vec; ReadZIP(vec); return vec;}

	int GetError() const {return err;}

private:
	void canonicalize_path(void);

	string name;	// Path name
	int err;
};

// inserts dir before the search path, then restores the original path
// when going out of scope
class ScopedSearchPath
{
public:
	ScopedSearchPath(const DirectorySpecifier& dir);
	~ScopedSearchPath();
};

#endif

