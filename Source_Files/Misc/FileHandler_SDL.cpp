/*
 *  FileHandler_SDL.cpp - Platform-independant file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "resource_manager.h"

#include "shell.h"
#include "interface.h"
#include "game_errors.h"

#include <stdio.h>
#include <errno.h>
#include <string>

#include <SDL_endian.h>

#ifdef HAVE_UNISTD_H
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#endif


// From shell_sdl.cpp
extern FileSpecifier global_data_dir, local_data_dir;


/*
 *  Opened file
 */

OpenedFile::OpenedFile() : f(NULL) {}

bool OpenedFile::IsOpen()
{
	return f != NULL;
}

bool OpenedFile::Close()
{
	if (f) {
		SDL_FreeRW(f);
		f = NULL;
	}
	return true;
}

bool OpenedFile::GetPosition(long &Position)
{
	if (f == NULL)
		return false;

	Position = SDL_RWtell(f);
	return true;
}

bool OpenedFile::SetPosition(long Position)
{
	if (f == NULL)
		return false;

	return SDL_RWseek(f, Position, SEEK_SET) >= 0;
}

bool OpenedFile::GetLength(long &Length)
{
	if (f == NULL)
		return false;

	long pos = SDL_RWtell(f);
	SDL_RWseek(f, 0, SEEK_END);
	Length = SDL_RWtell(f);
	SDL_RWseek(f, pos, SEEK_SET);
	return true;
}

bool OpenedFile::SetLength(long Length)
{
	// impossible to do in a platform-independant way
	printf("*** OpenedFile::SetLength(%ld)\n", Length);
	return false;
}

bool OpenedFile::Read(long Count, void *Buffer)
{
	if (f == NULL)
		return false;

	return SDL_RWread(f, Buffer, 1, Count) == Count;
}

bool OpenedFile::Write(long Count, void *Buffer)
{
	if (f == NULL)
		return false;

	return SDL_RWwrite(f, Buffer, 1, Count) == Count;
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

void LoadedResource::Detach()
{
	p = NULL;
	size = 0;
}


/*
 *  Opened resource file
 */

OpenedResourceFile::OpenedResourceFile() : f(NULL), saved_f(NULL) {}

bool OpenedResourceFile::Push()
{
	saved_f = cur_res_file();
	if (saved_f != f)
		use_res_file(f);
	return true;
}

bool OpenedResourceFile::Pop()
{
	if (f != saved_f)
		use_res_file(saved_f);
	return true;
}

bool OpenedResourceFile::Check(uint32 Type, int16 ID)
{
	Push();
	bool result = has_1_resource(Type, ID);
	Pop();
	return result;
}

bool OpenedResourceFile::Get(uint32 Type, int16 ID, LoadedResource &Rsrc)
{
	Push();
	bool success = get_1_resource(Type, ID, Rsrc);
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
	}
	return true;
}


/*
 *  File specification
 */

const FileSpecifier &FileSpecifier::operator=(const FileSpecifier &other)
{
	if (this != &other)
		name = other.name;
	return *this;
}

void FileSpecifier::GetName(char *Name) const
{
	strcpy(Name, name.c_str());
}

void FileSpecifier::SetName(const char *Name, int Type)
{
	name = Name;
}

// Create file
bool FileSpecifier::Create(int Type)
{
	Delete();
	// files are automatically created when opened for writing
	return true;
}

// Create directory
bool FileSpecifier::CreateDirectory() const
{
#if defined(__unix__) || defined(__BEOS__)

	return mkdir(name.c_str(), 0777) == 0;

#else
#error FileSpecifier::CreateDirectory() not implemented for this platform
#endif
}

// Open data file
bool FileSpecifier::Open(OpenedFile &OFile, bool Writable)
{
	OFile.Close();
	OFile.f = SDL_RWFromFile(name.c_str(), Writable ? "wb" : "rb");
	if (!OFile.IsOpen())
		set_game_error(systemError, OFile.GetError());
	return OFile.IsOpen();
}

// Open resource file
bool FileSpecifier::Open(OpenedResourceFile &OFile, bool Writable)
{
	OFile.Close();
	OFile.f = open_res_file(*this);
	if (!OFile.IsOpen())
		set_game_error(systemError, OFile.GetError());
	return OFile.IsOpen();
}

// Check for existence of file
bool FileSpecifier::Exists()
{
#if defined(__unix__) || defined(__BEOS__)

	// Check whether the file is readable
	return access(name.c_str(), R_OK) == 0;

#else
#error FileSpecifier::Exists() not implemented for this platform
#endif
}

// Get modification date
TimeType FileSpecifier::GetDate()
{
#if defined(__unix__) || defined(__BEOS__)

	struct stat st;
	if (stat(name.c_str(), &st) < 0)
		return 0;
	return st.st_mtime;

#else
#error FileSpecifier::GetDate() not implemented for this platform
#endif
}

// Determine file type
int FileSpecifier::GetType()
{
	// Open file
	OpenedFile f;
	if (!Open(f))
		return NONE;
	SDL_RWops *p = f.GetRWops();
	long file_length = 0;
	f.GetLength(file_length);

	// Check for Sounds file
	{
		SDL_RWseek(p, 0, SEEK_SET);
		uint32 version = SDL_ReadBE32(p);
		uint32 tag = SDL_ReadBE32(p);
		if (version == 1 && tag == FOUR_CHARS_TO_INT('s', 'n', 'd', '2'))
			return _typecode_sounds;
	}

	// Check for Map/Physics file
	{
		SDL_RWseek(p, 0, SEEK_SET);
		int version = SDL_ReadBE16(p);
		int data_version = SDL_ReadBE16(p);
		if ((version == 0 || version == 1 || version == 2 || version == 4) && (data_version == 0 || data_version == 1 || data_version == 2)) {
			SDL_RWseek(p, 68, SEEK_CUR);
			int32 directory_offset = SDL_ReadBE32(p);
			if (directory_offset >= file_length)
				goto not_map;
			SDL_RWseek(p, 128, SEEK_SET);
			uint32 tag = SDL_ReadBE32(p);
			if (tag == FOUR_CHARS_TO_INT('L', 'I', 'N', 'S') || tag == FOUR_CHARS_TO_INT('P', 'N', 'T', 'S'))
				return _typecode_scenario;
			if (tag == FOUR_CHARS_TO_INT('M', 'N', 'p', 'x'))
				return _typecode_physics;
		}
not_map: ;
	}

	// Check for Shapes file
	{
		SDL_RWseek(p, 0, SEEK_SET);
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
	return NONE;
}

// Get free space on disk
bool FileSpecifier::GetFreeSpace(unsigned long &FreeSpace)
{
	// This is impossible to do in a platform-independant way, so we
	// just return 16MB which should be enough for everything
	FreeSpace = 16 * 1024 * 1024;
	return true;
}

// Delete file
bool FileSpecifier::Delete()
{
	return remove(name.c_str()) == 0;
}

// Set to local (per-user) data directory
void FileSpecifier::SetToLocalDataDir()
{
	name = local_data_dir.name;
}

// Set to global data directory
void FileSpecifier::SetToGlobalDataDir()
{
	name = global_data_dir.name;
}

// Add part to path name
void FileSpecifier::AddPart(const string &part)
{
#if defined(__unix__) || defined(__BEOS__)

	if (name.length() && name[name.length() - 1] == '/')
		name += part;
	else
		name = name + "/" + part;

#else
#error FileSpecifier::AddPart() not implemented for this platform
#endif
}

// Get last element of path
void FileSpecifier::GetLastPart(char *part) const
{
#if defined(__unix__) || defined(__BEOS__)

	string::size_type pos = name.rfind('/');
	if (pos == string::npos)
		part[0] = 0;
	else
		strcpy(part, name.substr(pos + 1).c_str());

#else
#error FileSpecifier::GetLastPart() not implemented for this platform
#endif
}

// Read directory contents
bool FileSpecifier::ReadDirectory(vector<dir_entry> &vec) const
{
#if defined(__unix__) || defined(__BEOS__)

	vec.clear();

	DIR *d = opendir(name.c_str());
	if (d == NULL)
		return false;
	struct dirent *de = readdir(d);
	while (de) {
		if (de->d_name[0] != '.' || (de->d_name[1] && de->d_name[1] != '.')) {
			FileSpecifier full_path = name;
			full_path.AddPart(de->d_name);
			struct stat st;
			if (stat(full_path.name.c_str(), &st) == 0)
				vec.push_back(dir_entry(de->d_name, st.st_size, S_ISDIR(st.st_mode), false));
		}
		de = readdir(d);
	}
	closedir(d);
	return true;

#else
#error FileSpecifier::ReadDirectory() not implemented for this platform
#endif
}


/*
 *  Get FileSpecifiers for data files
 */

bool get_file_spec(FileSpecifier &spec, short listid, short item, short pathsid)
{
	spec = global_data_dir;
	if (getcstr(temporary, listid, item)) {
		spec.AddPart(temporary);
		return spec.Exists();
	}
	return false;
}

void get_default_map_spec(FileSpecifier &file)
{
	if (!get_file_spec(file, strFILENAMES, filenameDEFAULT_MAP, strPATHS))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void get_default_physics_spec(FileSpecifier &file)
{
	get_file_spec(file, strFILENAMES, filenamePHYSICS_MODEL, strPATHS);
}

void get_default_sounds_spec(FileSpecifier &file)
{
	get_file_spec(file, strFILENAMES, filenameSOUNDS8, strPATHS);
}

bool get_default_music_spec(FileSpecifier &file)
{
	return get_file_spec(file, strFILENAMES, filenameMUSIC, strPATHS);
}

void get_default_shapes_spec(FileSpecifier &file)
{
	if (!get_file_spec(file, strFILENAMES, filenameSHAPES8, strPATHS))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}
