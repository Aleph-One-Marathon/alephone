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

#ifdef HAVE_UNISTD_H
#include <sys/stat.h>
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
	printf("*** OpenedFile::SetLength(%d)\n", Length);
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

void FileSpecifier::GetName(char *Name)
{
	strcpy(Name, name.c_str());
}

void FileSpecifier::SetName(char *Name, int Type)
{
	name = Name;
}

bool FileSpecifier::Create(int Type)
{
	Delete();
	// files are automatically created when opened for writing
	return true;
}

bool FileSpecifier::Open(OpenedFile &OFile, bool Writable)
{
	OFile.Close();
	OFile.f = SDL_RWFromFile(name.c_str(), Writable ? "wb" : "rb");
	if (!OFile.IsOpen())
		set_game_error(systemError, OFile.GetError());
	return OFile.IsOpen();
}

bool FileSpecifier::Open(OpenedResourceFile &OFile, bool Writable)
{
	OFile.Close();
	OFile.f = open_res_file(*this);
	if (!OFile.IsOpen())
		set_game_error(systemError, OFile.GetError());
	return OFile.IsOpen();
}

bool FileSpecifier::Exists()
{
#if defined(__unix__) || defined(__BEOS__)
	return access(name.c_str(), R_OK) == 0;
#else
#error FileSpecifier::Exists() not implemented for this platform
#endif
}

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

int FileSpecifier::GetType()
{
	return NONE;
}

bool FileSpecifier::GetFreeSpace(unsigned long &FreeSpace)
{
	// This is impossible to do in a platform-independant way, so we
	// just return 16MB which should be enough for everything
	FreeSpace = 16 * 1024 * 1024;
	return true;
}

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
void FileSpecifier::GetLastPart(char *part)
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
