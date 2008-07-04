/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

*/

/*
 *  FileHandler_SDL.cpp - Platform-independant file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */
#ifndef SDL_RFORK_HACK
#include "cseries.h"
#include "FileHandler.h"
#include "resource_manager.h"

#include "shell.h"
#include "interface.h"
#include "game_errors.h"
#include "tags.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string>
#include <vector>

#include <SDL_endian.h>

#ifdef HAVE_UNISTD_H
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#endif

#ifdef __MACOS__
#include "mac_rwops.h"
#endif

#if defined(__WIN32__)
#define PATH_SEP '\\'
#elif !defined(__MACOS__)
#define PATH_SEP '/'
#else
#define PATH_SEP ':'
#endif

#ifdef __MVCPP__

#include <direct.h>			// for mkdir()
#include <io.h>				// for access()
#define R_OK  4				// for access(), this checks for read access.  6 should be used for read and write access both.
#include <sys/types.h>		// for stat()
#include <sys/stat.h>		// for stat()

#endif

#include "sdl_dialogs.h"
#include "sdl_widgets.h"
#include "SoundManager.h" // !

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;
extern DirectorySpecifier local_data_dir, preferences_dir, saved_games_dir, recordings_dir;

extern bool is_applesingle(SDL_RWops *f, bool rsrc_fork, long &offset, long &length);
extern bool is_macbinary(SDL_RWops *f, long &data_length, long &rsrc_length);

/*
 *  Opened file
 */

OpenedFile::OpenedFile() : f(NULL), err(0), is_forked(false), fork_offset(0), fork_length(0) {}

bool OpenedFile::IsOpen()
{
	return f != NULL;
}

bool OpenedFile::Close()
{
	if (f) {
		SDL_RWclose(f);
		f = NULL;
		err = 0;
	}
	is_forked = false;
	fork_offset = 0;
	fork_length = 0;
	return true;
}

bool OpenedFile::GetPosition(long &Position)
{
	if (f == NULL)
		return false;

	err = 0;
	Position = SDL_RWtell(f) - fork_offset;
	return true;
}

bool OpenedFile::SetPosition(long Position)
{
	if (f == NULL)
		return false;

	err = 0;
	if (SDL_RWseek(f, Position + fork_offset, SEEK_SET) < 0)
		err = errno;
	return err == 0;
}

bool OpenedFile::GetLength(long &Length)
{
	if (f == NULL)
		return false;

	if (is_forked)
		Length = fork_length;
	else {
		long pos = SDL_RWtell(f);
		SDL_RWseek(f, 0, SEEK_END);
		Length = SDL_RWtell(f);
		SDL_RWseek(f, pos, SEEK_SET);
	}
	err = 0;
	return true;
}

bool OpenedFile::Read(long Count, void *Buffer)
{
	if (f == NULL)
		return false;

	err = 0;
	return (SDL_RWread(f, Buffer, 1, Count) == Count);
}

bool OpenedFile::Write(long Count, void *Buffer)
{
	if (f == NULL)
		return false;

	err = 0;
	return (SDL_RWwrite(f, Buffer, 1, Count) == Count);
}


SDL_RWops *OpenedFile::TakeRWops ()
{
	SDL_RWops *taken = f;
	f = NULL;
	Close ();
	return taken;
}

/*
 *  Loaded resource
 */

LoadedResource::LoadedResource() : p(NULL), size(0) {}

bool LoadedResource::IsLoaded()
{
	return p != NULL;
}

void LoadedResource::Unload()
{
	if (p) {
		free(p);
		p = NULL;
		size = 0;
	}
}

size_t LoadedResource::GetLength()
{
	return size;
}

void *LoadedResource::GetPointer(bool DoDetach)
{
	void *ret = p;
	if (DoDetach)
		Detach();
	return ret;
}

void LoadedResource::SetData(void *data, size_t length)
{
	Unload();
	p = data;
	size = length;
}

void LoadedResource::Detach()
{
	p = NULL;
	size = 0;
}


/*
 *  Opened resource file
 */

OpenedResourceFile::OpenedResourceFile() : f(NULL), saved_f(NULL), err(0) {}

bool OpenedResourceFile::Push()
{
	saved_f = cur_res_file();
	if (saved_f != f)
		use_res_file(f);
	err = 0;
	return true;
}

bool OpenedResourceFile::Pop()
{
	if (f != saved_f)
		use_res_file(saved_f);
	err = 0;
	return true;
}

bool OpenedResourceFile::Check(uint32 Type, int16 ID)
{
	Push();
	bool result = has_1_resource(Type, ID);
	err = result ? 0 : errno;
	Pop();
	return result;
}

bool OpenedResourceFile::Get(uint32 Type, int16 ID, LoadedResource &Rsrc)
{
	Push();
	bool success = get_1_resource(Type, ID, Rsrc);
	err = success ? 0 : errno;
	Pop();
	return success;
}

bool OpenedResourceFile::IsOpen()
{
	return f != NULL;
}

bool OpenedResourceFile::Close()
{
	if (f) {
		close_res_file(f);
		f = NULL;
		err = 0;
	}
	return true;
}


/*
 *  File specification
 */
//AS: Constructor moved here to fix linking errors
FileSpecifier::FileSpecifier(): err(0) {}
const FileSpecifier &FileSpecifier::operator=(const FileSpecifier &other)
{
	if (this != &other) {
		name = other.name;
		err = other.err;
	}
	return *this;
}

// Create file
bool FileSpecifier::Create(Typecode Type)
{
	Delete();
	// files are automatically created when opened for writing
	err = 0;
	return true;
}

// Create directory
bool FileSpecifier::CreateDirectory()
{
	err = 0;
#if defined(__WIN32__)
	if (mkdir(GetPath()) < 0)
#else
	if (mkdir(GetPath(), 0777) < 0)
#endif
		err = errno;
	return err == 0;
}

// Open data file
bool FileSpecifier::Open(OpenedFile &OFile, bool Writable)
{
	OFile.Close();

	SDL_RWops *f;
#ifdef __MACOS__
	if (!Writable)
		f = OFile.f = open_fork_from_existing_path(GetPath(), false);
	else
#endif
		f = OFile.f = SDL_RWFromFile(GetPath(), Writable ? "wb+" : "rb");

	err = f ? 0 : errno;
	if (f == NULL) {
		set_game_error(systemError, err);
		return false;
	}
	if (Writable)
		return true;

	// Transparently handle AppleSingle and MacBinary files on reading
	long offset, data_length, rsrc_length;
	if (is_applesingle(f, false, offset, data_length)) {
		OFile.is_forked = true;
		OFile.fork_offset = offset;
		OFile.fork_length = data_length;
		SDL_RWseek(f, offset, SEEK_SET);
		return true;
	} else if (is_macbinary(f, data_length, rsrc_length)) {
		OFile.is_forked = true;
		OFile.fork_offset = 128;
		OFile.fork_length = data_length;
		SDL_RWseek(f, 128, SEEK_SET);
		return true;
	}
	SDL_RWseek(f, 0, SEEK_SET);
	return true;
}

// Open resource file
bool FileSpecifier::Open(OpenedResourceFile &OFile, bool Writable)
{
	OFile.Close();

	OFile.f = open_res_file(*this);
	err = OFile.f ? 0 : errno;
	if (OFile.f == NULL) {
		set_game_error(systemError, err);
		return false;
	} else
		return true;
}

// Check for existence of file
bool FileSpecifier::Exists()
{
	// Check whether the file is readable
	err = 0;
	if (access(GetPath(), R_OK) < 0)
		err = errno;
	return err == 0;
}

bool FileSpecifier::IsDir()
{
	struct stat st;
	err = 0;
	if (stat(GetPath(), &st) < 0)
		return false;
	return (S_ISDIR(st.st_mode));
}

// Get modification date
TimeType FileSpecifier::GetDate()
{
	struct stat st;
	err = 0;
	if (stat(GetPath(), &st) < 0) {
		err = errno;
		return 0;
	}
	return st.st_mtime;
}

// Determine file type
Typecode FileSpecifier::GetType()
{

	// if there's an extension, assume it's correct
	const char *extension = strrchr(GetPath(), '.');
	if (extension) {
		if (strcasecmp(extension, ".sce2") == 0 || strcasecmp(extension, ".sceA") == 0) return _typecode_scenario;
		
		// don't bother checking files with these extensions
		// this helps a lot because there can be many of these files
		if (
			strcasecmp(extension, ".dds") == 0 ||
			strcasecmp(extension, ".jpg") == 0 ||
			strcasecmp(extension, ".png") == 0
			) 
		{
			return _typecode_unknown;
		}
	}

	// Open file
	OpenedFile f;
	if (!Open(f))
		return _typecode_unknown;
	SDL_RWops *p = f.GetRWops();
	long file_length = 0;
	f.GetLength(file_length);

	// Check for Sounds file
	{
		f.SetPosition(0);
		uint32 version = SDL_ReadBE32(p);
		uint32 tag = SDL_ReadBE32(p);
		if ((version == 0 || version == 1) && tag == FOUR_CHARS_TO_INT('s', 'n', 'd', '2'))
			return _typecode_sounds;
	}

	// Check for Map/Physics file
	{
		f.SetPosition(0);
		int version = SDL_ReadBE16(p);
		int data_version = SDL_ReadBE16(p);
		if ((version == 0 || version == 1 || version == 2 || version == 4) && (data_version == 0 || data_version == 1 || data_version == 2)) {
			SDL_RWseek(p, 68, SEEK_CUR);
			int32 directory_offset = SDL_ReadBE32(p);
			if (directory_offset >= file_length)
				goto not_map;
			f.SetPosition(128);
			uint32 tag = SDL_ReadBE32(p);
			// ghs: I do not believe this list is comprehensive
			//      I think it's just what we've seen so far?
			switch (tag) {
			case LINE_TAG:
			case POINT_TAG:
			case SIDE_TAG:
				return _typecode_scenario;
				break;
			case MONSTER_PHYSICS_TAG:
				return _typecode_physics;
				break;
			}
				
		}
not_map: ;
	}

	// Check for Shapes file
	{
		f.SetPosition(0);
		for (int i=0; i<32; i++) {
			uint32 status_flags = SDL_ReadBE32(p);
			int32 offset = SDL_ReadBE32(p);
			int32 length = SDL_ReadBE32(p);
			int32 offset16 = SDL_ReadBE32(p);
			int32 length16 = SDL_ReadBE32(p);
			if (status_flags != 0
			 || (offset != NONE && (offset >= file_length || offset + length > file_length))
			 || (offset16 != NONE && (offset16 >= file_length || offset16 + length16 > file_length)))
				goto not_shapes;
			SDL_RWseek(p, 12, SEEK_CUR);
		}
		return _typecode_shapes;
not_shapes: ;
	}

	// Not identified
	return _typecode_unknown;
}

// Get free space on disk
bool FileSpecifier::GetFreeSpace(unsigned long &FreeSpace)
{
	// This is impossible to do in a platform-independant way, so we
	// just return 16MB which should be enough for everything
	FreeSpace = 16 * 1024 * 1024;
	err = 0;
	return true;
}

// Exchange two files
bool FileSpecifier::Exchange(FileSpecifier &other)
{
	// Create temporary name (this is cheap, we should make sure that the
	// name is not already in use...)
	FileSpecifier tmp;
	ToDirectory(tmp);
	tmp.AddPart("exchange_tmp_file");

	err = 0;
	if (rename(GetPath(), tmp.GetPath()) < 0)
		err = errno;
	else
		rename(other.GetPath(), GetPath());
	if (rename(tmp.GetPath(), other.GetPath()) < 0)
		err = errno;
	return err == 0;
}

// Delete file
bool FileSpecifier::Delete()
{
	err = 0;
	if (remove(GetPath()) < 0)
		err = errno;
	return err == 0;
}

// Set to local (per-user) data directory
void FileSpecifier::SetToLocalDataDir()
{
	name = local_data_dir.name;
}

// Set to preferences directory
void FileSpecifier::SetToPreferencesDir()
{
	name = preferences_dir.name;
}

// Set to saved games directory
void FileSpecifier::SetToSavedGamesDir()
{
	name = saved_games_dir.name;
}

// Set to recordings directory
void FileSpecifier::SetToRecordingsDir()
{
	name = recordings_dir.name;
}

void FileSpecifier::SetToFirstDataDir()
{
  name = data_search_path[0].name;
}

// Traverse search path, look for file given relative path name
bool FileSpecifier::SetNameWithPath(const char *NameWithPath)
{
	FileSpecifier full_path;
	string rel_path = NameWithPath;

#ifdef __WIN32__
	// For cross-platform compatibility reasons, "NameWithPath" uses Unix path
	// syntax, so we have to convert it to MS-DOS syntax here (replacing '/' by '\')
	for (size_t k=0; k<rel_path.size(); k++)
		if (rel_path[k] == '/')
			rel_path[k] = '\\';
#elif defined(__MWERKS__)
	for (size_t k=0; k < rel_path.size(); k++)
		if (rel_path[k] == '/')
			rel_path[k] = ':';
#endif

	vector<DirectorySpecifier>::const_iterator i = data_search_path.begin(), end = data_search_path.end();
	while (i != end) {
		full_path = *i + rel_path;
		if (full_path.Exists()) {
			name = full_path.name;
			err = 0;
			return true;
		}
		i++;
	}
	err = ENOENT;
	return false;
}

// Get last element of path
void FileSpecifier::GetName(char *part) const
{
	string::size_type pos = name.rfind(PATH_SEP);
	if (pos == string::npos)
		strcpy(part, name.c_str());
	else
		strcpy(part, name.substr(pos + 1).c_str());
}

// Add part to path name
void FileSpecifier::AddPart(const string &part)
{
	if (name.length() && name[name.length() - 1] == PATH_SEP)
		name += part;
	else
		name = name + PATH_SEP + part;

	canonicalize_path();
}

// Split path to base and last part
void FileSpecifier::SplitPath(string &base, string &part) const
{
	string::size_type pos = name.rfind(PATH_SEP);
	if (pos == string::npos) {
		base = name;
		part.erase();
	} else if (pos == 0) {
		base = PATH_SEP;
		part = name.substr(1);
	} else {
		base = name.substr(0, pos);
		part = name.substr(pos + 1);
	}
}

// Fill file specifier with base name
void FileSpecifier::ToDirectory(DirectorySpecifier &dir)
{
	string part;
	SplitPath(dir, part);
}

// Set file specifier from directory specifier
void FileSpecifier::FromDirectory(DirectorySpecifier &Dir)
{
	name = Dir.name;
}

// Canonicalize path
void FileSpecifier::canonicalize_path(void)
{
#if !defined(__WIN32__)

	// Replace multiple consecutive '/'s by a single '/'
	while (true) {
		string::size_type pos = name.find("//");
		if (pos == string::npos)
			break;
		name.erase(pos, 1);
	}

#endif

	// Remove trailing '/'
	// ZZZ: only if we're not naming the root directory /
	if (!name.empty() && name[name.size()-1] == PATH_SEP && name.size() != 1)
		name.erase(name.size()-1, 1);
}

// Read directory contents
bool FileSpecifier::ReadDirectory(vector<dir_entry> &vec)
{
	vec.clear();

#if defined(__MVCPP__)

	WIN32_FIND_DATA findData;

	// We need to add a wildcard to the search name
	string search_name;
	search_name = name;
	search_name += "\\*.*";

	HANDLE hFind = ::FindFirstFile(search_name.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		err = ::GetLastError();
		return false;
	}

	do {
		// Exclude current and parent directories
		if (findData.cFileName[0] != '.' ||
		    (findData.cFileName[1] && findData.cFileName[1] != '.')) {
			// Return found files to dir_entry
			long fileSize = (findData.nFileSizeHigh * MAXDWORD) + findData.nFileSizeLow;
			vec.push_back(dir_entry(findData.cFileName, fileSize,
			              (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0, false));
		}
	} while(::FindNextFile(hFind, &findData));

	if (!::FindClose(hFind))
		err = ::GetLastError(); // not sure if we should return this or not
	else
		err = 0;
	return true;

#else

	DIR *d = opendir(GetPath());

	if (d == NULL) {
		err = errno;
		return false;
	}
	struct dirent *de = readdir(d);
	while (de) {
		FileSpecifier full_path = name;
		full_path += de->d_name;
		struct stat st;
		if (stat(full_path.GetPath(), &st) == 0) {
			// Ignore files starting with '.' and the directories '.' and '..'
			if (de->d_name[0] != '.' || (S_ISDIR(st.st_mode) && !(de->d_name[1] == '\0' || de->d_name[1] == '.')))
				vec.push_back(dir_entry(de->d_name, st.st_size, S_ISDIR(st.st_mode), false));
		}
		de = readdir(d);
	}
	closedir(d);
	err = 0;
	return true;

#endif
}

// Copy file contents
bool FileSpecifier::CopyContents(FileSpecifier &source_name)
{
	err = 0;
	OpenedFile src, dst;
	if (source_name.Open(src)) {
		Delete();
		if (Open(dst, true)) {
			const int BUFFER_SIZE = 1024;
			uint8 buffer[BUFFER_SIZE];

			long length = 0;
			src.GetLength(length);

			while (length && err == 0) {
				long count = length > BUFFER_SIZE ? BUFFER_SIZE : length;
				if (src.Read(count, buffer)) {
					if (!dst.Write(count, buffer))
						err = dst.GetError();
				} else
					err = src.GetError();
				length -= count;
			}
		}
	} else
		err = source_name.GetError();
	if (err)
		Delete();
	return err == 0;
}

// ZZZ: Filesystem browsing list that lets user actually navigate directories...
class w_directory_browsing_list : public w_list<dir_entry>
{
public:
	w_directory_browsing_list(const FileSpecifier& inStartingDirectory, dialog* inParentDialog)
	: w_list<dir_entry>(entries, 400, 15, 0), parent_dialog(inParentDialog), current_directory(inStartingDirectory)
	{
		refresh_entries();
	}


	w_directory_browsing_list(const FileSpecifier& inStartingDirectory, dialog* inParentDialog, const string& inStartingFile)
	: w_list<dir_entry>(entries, 400, 15, 0), parent_dialog(inParentDialog), current_directory(inStartingDirectory)
	{
		refresh_entries();
		if(entries.size() != 0)
			select_entry(inStartingFile, false);
	}


	void set_directory_changed_callback(action_proc inCallback, void* inArg = NULL)
	{
		directory_changed_proc = inCallback;
		directory_changed_proc_arg = inArg;
	}


	void draw_item(vector<dir_entry>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
	{
		y += font->get_ascent();
		set_drawing_clip_rectangle(0, x, s->h, x + width);

		if(i->is_directory)
		{
			string theName = i->name + "/";
			draw_text(s, theName.c_str (), x, y, selected ? get_theme_color (ITEM_WIDGET, ACTIVE_STATE) : get_theme_color (ITEM_WIDGET, DEFAULT_STATE), font, style, true);
		}
		else
			draw_text(s, i->name.c_str (), x, y, selected ? get_theme_color (ITEM_WIDGET, ACTIVE_STATE) : get_theme_color (ITEM_WIDGET, DEFAULT_STATE), font, style, true);

		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}


	bool can_move_up_a_level()
	{
		string base;
		string part;
		current_directory.SplitPath(base, part);
		return (part != string());
	}


	void move_up_a_level()
	{
		string base;
		string part;
		current_directory.SplitPath(base, part);
		if(part != string())
		{
			FileSpecifier parent_directory(base);
			if(parent_directory.Exists())
			{
				current_directory = parent_directory;
				refresh_entries();
				select_entry(part, true);
				announce_directory_changed();
			}
		}
	}


	void item_selected(void)
	{
		current_directory.AddPart(entries[selection].name);

		if(entries[selection].is_directory)
		{
			refresh_entries();
			announce_directory_changed();
		}
		else
			parent_dialog->quit(0);
	}


	const FileSpecifier& get_file() { return current_directory; }
	
	
private:
	vector<dir_entry>	entries;
	dialog*			parent_dialog;
	FileSpecifier 		current_directory;
	action_proc		directory_changed_proc;
	void*			directory_changed_proc_arg;
	
	void refresh_entries()
	{
		if(current_directory.ReadDirectory(entries))
		{
			sort(entries.begin(), entries.end());
			num_items = entries.size();
			new_items();
		}
	}

	void select_entry(const string& inName, bool inIsDirectory)
	{
		dir_entry theEntryToFind(inName, NONE /* length - ignored for our purpose */, inIsDirectory);
		vector<dir_entry>::iterator theEntry = lower_bound(entries.begin(), entries.end(), theEntryToFind);
		if(theEntry != entries.end())
			set_selection(theEntry - entries.begin());
	}

	void announce_directory_changed()
	{
		if(directory_changed_proc != NULL)
			directory_changed_proc(directory_changed_proc_arg);
	}
};



class w_file_list : public w_list<dir_entry> {
public:
	w_file_list(const vector<dir_entry> &items) : w_list<dir_entry>(items, 400, 15, 0) {}

	void draw_item(vector<dir_entry>::const_iterator i, SDL_Surface *s, int16 x, int16 y, uint16 width, bool selected) const
	{
		y += font->get_ascent();
		set_drawing_clip_rectangle(0, x, s->h, x + width);
		draw_text(s, i->name.c_str (), x, y, selected ? get_theme_color (ITEM_WIDGET, ACTIVE_STATE) : get_theme_color (ITEM_WIDGET, DEFAULT_STATE), font, style, true);
		set_drawing_clip_rectangle(SHRT_MIN, SHRT_MIN, SHRT_MAX, SHRT_MAX);
	}
};

class w_read_file_list : public w_file_list {
public:
	w_read_file_list(const vector<dir_entry> &items, dialog *d) : w_file_list(items), parent(d) {}

	void item_selected(void)
	{
		parent->quit(0);
	}

private:
	dialog *parent;
};

static void
bounce_up_a_directory_level(void* inWidget)
{
	w_directory_browsing_list* theBrowser = static_cast<w_directory_browsing_list*>(inWidget);
	theBrowser->move_up_a_level();
}

enum
{
	iDIRBROWSE_UP_BUTTON = 100,
	iDIRBROWSE_DIR_NAME,
	iDIRBROWSE_BROWSER
};

static void
respond_to_directory_changed(void* inArg)
{
	// Get dialog and its directory browser
	dialog* theDialog = static_cast<dialog*>(inArg);
	w_directory_browsing_list* theBrowser = dynamic_cast<w_directory_browsing_list*>(theDialog->get_widget_by_id(iDIRBROWSE_BROWSER));
	
	// Update static text listing current directory
	w_static_text* theDirectoryName = dynamic_cast<w_static_text*>(theDialog->get_widget_by_id(iDIRBROWSE_DIR_NAME));
	theBrowser->get_file().GetName(temporary);
	theDirectoryName->set_text(temporary);
	
	// Update enabled state of Up button
	w_button* theUpButton = dynamic_cast<w_button*>(theDialog->get_widget_by_id(iDIRBROWSE_UP_BUTTON));
	theUpButton->set_enabled(theBrowser->can_move_up_a_level());

	// ghs: hack till static text can update its rect correctly
	theDialog->draw();
}

bool FileSpecifier::ReadDialog(Typecode type, const char *prompt)
{
	// Set default prompt
	if (prompt == NULL) {
		switch (type) {
			case _typecode_savegame:
				prompt = "CONTINUE SAVED GAME";
				break;
			case _typecode_film:
				prompt = "REPLAY SAVED FILM";
				break;
			default:
				prompt = "OPEN FILE";
				break;
		}
	}

	// Read directory
	FileSpecifier dir;
	string filename;
	switch (type) {
		case _typecode_savegame:
			dir.SetToSavedGamesDir();
			break;
		case _typecode_film:
			dir.SetToRecordingsDir();
			break;
	case _typecode_scenario:
	  dir.SetToFirstDataDir();
	  break;
		case _typecode_netscript:
		{
			// Go to most recently-used directory
			DirectorySpecifier theDirectory;
			SplitPath(theDirectory, filename);
			dir.FromDirectory(theDirectory);
			if(!dir.Exists())
				dir.SetToLocalDataDir();
			break;
		}
		default:
			dir.SetToLocalDataDir();
			break;
	}

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title(prompt), d);
	placer->add(new w_spacer, true);

	dir.GetName(temporary);
	w_static_text* directory_name_w = new w_static_text(temporary);
	directory_name_w->set_identifier(iDIRBROWSE_DIR_NAME);
	placer->dual_add(directory_name_w, d);

	placer->add(new w_spacer(), true);

	w_directory_browsing_list* list_w = ((type == _typecode_netscript)
		? new w_directory_browsing_list(dir, &d, filename)
		: new w_directory_browsing_list(dir, &d));
	list_w->set_identifier(iDIRBROWSE_BROWSER);
	list_w->set_directory_changed_callback(respond_to_directory_changed, &d);
	placer->dual_add(list_w, d);
	placer->add(new w_spacer, true);

	horizontal_placer *button_placer = new horizontal_placer;
	w_button* up_button_w = new w_button("UP", bounce_up_a_directory_level, list_w);
	button_placer->dual_add(up_button_w, d);

	up_button_w->set_identifier(iDIRBROWSE_UP_BUTTON);

	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	placer->add(button_placer, true);

	d.activate_widget(list_w);

	d.set_widget_placer(placer);

	// Run dialog
	bool result = false;
	if (d.run() == 0) { // OK
		*this = list_w->get_file();
		result = true;
	}

	// Redraw game window
	if (get_game_state() == _game_in_progress) update_game_window();
	return result;
}

class w_file_name : public w_text_entry {
public:
	w_file_name(dialog *d, const char *initial_name = NULL) : w_text_entry(31, initial_name), parent(d) {}
	~w_file_name() {}

	void event(SDL_Event & e)
	{
		// Return = close dialog
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN)
			parent->quit(0);
		w_text_entry::event(e);
	}

private:
	dialog *parent;
};

class w_write_file_list : public w_file_list {
public:
	w_write_file_list(const vector<dir_entry> &items, const char *selection, dialog *d, w_file_name *w) : w_file_list(items), parent(d), name_widget(w)
	{
		if (selection) {
			vector<dir_entry>::const_iterator i, end = items.end();
			size_t num = 0;
			for (i = items.begin(); i != end; i++, num++) {
				if (i->name == selection) {
					set_selection(num);
					break;
				}
			}
		}
	}

	void item_selected(void)
	{
		name_widget->set_text(items[selection].name.c_str());
		parent->quit(0);
	}

private:
	dialog *parent;
	w_file_name *name_widget;
};

static bool confirm_save_choice(FileSpecifier & file);

bool FileSpecifier::WriteDialog(Typecode type, const char *prompt, const char *default_name)
{
	// Set default prompt
	if (prompt == NULL) {
		switch (type) {
			case _typecode_savegame:
				prompt = "SAVE GAME";
				break;
			case _typecode_film:
				prompt = "SAVE FILM";
				break;
			default:
				prompt = "SAVE FILE";
				break;
		}
	}

	// Read directory
	FileSpecifier dir;
	switch (type) {
		case _typecode_savegame:
			dir.SetToSavedGamesDir();
			break;
		case _typecode_film:
			dir.SetToRecordingsDir();
			break;
		default:
			dir.SetToLocalDataDir();
			break;
	}
	vector<dir_entry> entries;
	if (!dir.ReadDirectory(entries))
		return false;
	sort(entries.begin(), entries.end());

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_title(prompt), d);
	placer->add(new w_spacer(), true);

	horizontal_placer *file_name_placer = new horizontal_placer;
	w_file_name *name_w = new w_file_name(&d, default_name);
	file_name_placer->dual_add(name_w->label("File Name:"), d);
	file_name_placer->add_flags(placeable::kFill);
	file_name_placer->dual_add(name_w, d);

	w_write_file_list *list_w = new w_write_file_list(entries, default_name, &d, name_w);
	placer->dual_add(list_w, d);
	placer->add(new w_spacer(), true);

	placer->add_flags(placeable::kFill);
	placer->add(file_name_placer, true);
	placer->add_flags();
	placer->add(new w_spacer, true);

	horizontal_placer *button_placer = new horizontal_placer;
	button_placer->dual_add(new w_button("OK", dialog_ok, &d), d);
	button_placer->dual_add(new w_button("CANCEL", dialog_cancel, &d), d);

	placer->add(button_placer, true);

	d.set_widget_placer(placer);

	d.activate_widget(name_w);
	// Run dialog
again:
	bool result = false;
	if (d.run () == 0) { // OK
		if (strlen(name_w->get_text()) == 0) {
			play_dialog_sound(DIALOG_ERROR_SOUND);
			name_w->set_text(default_name);
			goto again;
		}
		name = dir.name;
		AddPart(name_w->get_text());
		result = confirm_save_choice(*this);
		if (!result)
			goto again;
	}

	// Redraw game window
	update_game_window();
	return result;
}

bool FileSpecifier::WriteDialogAsync(Typecode type, char *prompt, char *default_name)
{
	return FileSpecifier::WriteDialog(type, prompt, default_name);
}

static bool confirm_save_choice(FileSpecifier & file)
{
	// If the file doesn't exist, everything is alright
	if (!file.Exists())
		return true;

	// Construct message
	char name[256];
	file.GetName(name);
	char message[512];
	sprintf(message, "'%s' already exists.", name);

	// Create dialog
	dialog d;
	vertical_placer *placer = new vertical_placer;
	placer->dual_add(new w_static_text(message), d);
	placer->dual_add(new w_static_text("Ok to overwrite?"), d);
	placer->add(new w_spacer(), true);

	horizontal_placer *button_placer = new horizontal_placer;
	w_button *default_button = new w_button("YES", dialog_ok, &d);
	button_placer->dual_add(default_button, d);
	button_placer->dual_add(new w_button("NO", dialog_cancel, &d), d);

	placer->add(button_placer, true);

	d.activate_widget(default_button);

	d.set_widget_placer(placer);

	// Run dialog
	return d.run() == 0;
}


#endif
