/*
 *  FileHandler_SDL.cpp - Platform-independant file handling, SDL implementation
 *
 *  Written in 2000 by Christian Bauer
 */

#include "cseries.h"
#include "FileHandler.h"
#include "interface.h"
#include "game_errors.h"

#include <stdio.h>
#include <errno.h>
#include <string>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

// From shell_sdl.cpp
extern FileObject global_data_dir, local_data_dir;


/*
 *  Opened file
 */

bool OpenedFile::IsOpen()
{
	return handle != NULL;
}

bool OpenedFile::Close()
{
	if (!handle)
		return true;

	int res = fclose(handle);
	handle = NULL;
	return res == 0;
}

bool OpenedFile::GetPosition(long &Position)
{
	if (!handle)
		return false;

	Position = ftell(handle);
	return true;
}

bool OpenedFile::SetPosition(long Position)
{
	if (!handle)
		return false;

	return fseek(handle, Position, SEEK_SET) == 0;
}

bool OpenedFile::GetLength(long &Length)
{
	if (!handle)
		return false;

	long pos = ftell(handle);
	fseek(handle, 0, SEEK_END);
	Length = ftell(handle);
	fseek(handle, 0, SEEK_SET);
	return true;
}

bool OpenedFile::SetLength(long Length)
{
	// impossible to do in a platform-independant way
	return false;
}

bool OpenedFile::Read(long Count, void *Buffer)
{
	if (!handle)
		return false;

	return fread(Buffer, 1, Count, handle) == Count;
}

bool OpenedFile::Write(long Count, void *Buffer)
{
	if (!handle)
		return false;

	return fwrite(Buffer, 1, Count, handle) == Count;
}


/*
 *  File specification
 */

const FileObject &FileObject::operator=(const FileObject &other)
{
	if (this != &other)
		name = other.name;
	return *this;
}

void FileObject::GetName(char *Name)
{
	strcpy(Name, name.c_str());
}

void FileObject::SetName(char *Name, int Type)
{
	name = Name;
}

bool FileObject::Create(int Type)
{
	// files are automatically created when opened for writing
	return true;
}

bool FileObject::Open(OpenedFile &OFile, bool Writable = false)
{
	OFile.Close();
	OFile.handle = fopen(name.c_str(), Writable ? "wb" : "rb");
	if (!OFile.IsOpen())
		set_game_error(systemError, OFile.GetError());
	return OFile.IsOpen();
}

bool FileObject::Exists()
{
#ifdef __unix__
	return access(name.c_str(), R_OK) == 0;
#else
#error FileObject::Exists() not implemented for this platform
#endif
}

bool FileObject::GetFreeSpace(unsigned long &FreeSpace)
{
	// This is impossible to do in a platform-independant way, so we
	// just return 16MB which should be enough for everything
	FreeSpace = 16 * 1024 * 1024;
	return true;
}

bool FileObject::Delete()
{
	return remove(name.c_str()) == 0;
}

// Add part to path name
void FileObject::AddPart(const string &part)
{
#if defined(__unix__) || defined(__BEOS__)

	if (name.length() && name[name.length() - 1] == '/')
		name += part;
	else
		name = name + "/" + part;

#else
#error FileObject::AddPart() not implemented for this platform
#endif
}

// Get last element of path
void FileObject::GetLastPart(char *part)
{
#if defined(__unix__) || defined(__BEOS__)

	string::size_type pos = name.rfind('/');
	if (pos == string::npos)
		part[0] = 0;
	else
		strcpy(part, name.substr(pos + 1).c_str());

#else
#error FileObject::GetLastPart() not implemented for this platform
#endif
}


/*
 *  Get FileObjects for data files
 */

bool get_file_spec(FileObject &spec, short listid, short item, short pathsid)
{
printf("get_file_spec listid %d, item %d, pathsid %d\n", listid, item, pathsid);

	spec = global_data_dir;
	if (getcstr(temporary, listid, item)) {
		spec.AddPart(temporary);
printf(" -> %s\n", spec.name.c_str());
		return spec.Exists();
	}
	return false;
}

void get_default_map_spec(FileObject &file)
{
	if (!get_file_spec(file, strFILENAMES, filenameDEFAULT_MAP, strPATHS))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void get_default_physics_spec(FileObject &file)
{
	get_file_spec(file, strFILENAMES, filenamePHYSICS_MODEL, strPATHS);
}

void get_default_sounds_spec(FileObject &file)
{
	get_file_spec(file, strFILENAMES, filenameSOUNDS8, strPATHS);
}

bool get_default_music_spec(FileObject &file)
{
	return get_file_spec(file, strFILENAMES, filenameMUSIC, strPATHS);
}

void get_default_images_spec(FileObject &file)
{
	if (!get_file_spec(file, strFILENAMES, filenameIMAGES, strPATHS))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void get_default_shapes_spec(FileObject &file)
{
	if (!get_file_spec(file, strFILENAMES, filenameSHAPES8, strPATHS))
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
}

void find_preferences_location(FileObject &file)
{
	FileObject prefs_file = file;
	file = local_data_dir;
	file.AddPart(prefs_file.name);
}

void get_savegame_filedesc(FileObject &file)
{
	file = local_data_dir;
	if (getcstr(temporary, strFILENAMES, filenameDEFAULT_SAVE_GAME))
		file.AddPart(temporary);
}
