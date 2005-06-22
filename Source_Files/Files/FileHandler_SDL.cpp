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


#if defined(__WIN32__)
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

#ifdef __MVCPP__

#include <direct.h>			// for mkdir()
#include <io.h>				// for access()
#define R_OK  4				// for access(), this checks for read access.  6 should be used for read and write access both.
#include <sys/types.h>		// for stat()
#include <sys/stat.h>		// for stat()

#endif

// From shell_sdl.cpp
extern vector<DirectorySpecifier> data_search_path;
extern DirectorySpecifier local_data_dir, preferences_dir, saved_games_dir, recordings_dir;


/*
 *  Utility functions
 */

bool is_applesingle(SDL_RWops *f, bool rsrc_fork, long &offset, long &length)
{
	// Check header
	SDL_RWseek(f, 0, SEEK_SET);
	uint32 id = SDL_ReadBE32(f);
	uint32 version = SDL_ReadBE32(f);
	if (id != 0x00051600 || version != 0x00020000)
		return false;

	// Find fork
	uint32 req_id = rsrc_fork ? 2 : 1;
	SDL_RWseek(f, 0x18, SEEK_SET);
	int num_entries = SDL_ReadBE16(f);
	while (num_entries--) {
		uint32 id = SDL_ReadBE32(f);
		int32 ofs = SDL_ReadBE32(f);
		int32 len = SDL_ReadBE32(f);
		//printf(" entry id %d, offset %d, length %d\n", id, ofs, len);
		if (id == req_id) {
			offset = ofs;
			length = len;
			return true;
		}
	}
	return false;
}

bool is_macbinary(SDL_RWops *f, long &data_length, long &rsrc_length)
{
	// This only recognizes MacBinary II files
	SDL_RWseek(f, 0, SEEK_SET);
	uint8 header[128];
	SDL_RWread(f, header, 1, 128);
	if (header[0] || header[1] > 63 || header[74] || header[122] < 0x81 || header[123] < 0x81)
		return false;

	// Check CRC
	uint16 crc = 0;
	for (int i=0; i<124; i++) {
		uint16 data = header[i] << 8;
		for (int j=0; j<8; j++) {
			if ((data ^ crc) & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
			data <<= 1;
		}
	}
	//printf("crc %02x\n", crc);
	if (crc != ((header[124] << 8) | header[125]))
		return false;

	// CRC valid, extract fork sizes
	data_length = (header[83] << 24) | (header[84] << 16) | (header[85] << 8) | header[86];
	rsrc_length = (header[87] << 24) | (header[88] << 16) | (header[89] << 8) | header[90];
	return true;
}


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
	if (SDL_RWread(f, Buffer, 1, Count) != Count)
		err = errno;
	return err == 0;
}

bool OpenedFile::Write(long Count, void *Buffer)
{
	if (f == NULL)
		return false;

	err = 0;
	if (SDL_RWwrite(f, Buffer, 1, Count) != Count)
		err = errno;
	return err == 0;
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

	SDL_RWops *f = OFile.f = SDL_RWFromFile(GetPath(), Writable ? "wb" : "rb");
	err = f ? 0 : errno;
	if (f == NULL) {
		set_game_error(systemError, err);
		return false;
	}
	if (Writable)
		return true;

	// Transparently handle AppleSingle and MacBinary II files on reading
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
			if (tag == FOUR_CHARS_TO_INT('L', 'I', 'N', 'S') || tag == FOUR_CHARS_TO_INT('P', 'N', 'T', 'S') || tag == FOUR_CHARS_TO_INT('S', 'I', 'D', 'S'))
				return _typecode_scenario;
			if (tag == FOUR_CHARS_TO_INT('M', 'N', 'p', 'x'))
				return _typecode_physics;
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

#endif
