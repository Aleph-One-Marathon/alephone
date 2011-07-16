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

*/
/*
 *  csfiles_beos.cpp - Some BeOS-specific functions that didn't fit elsewhere
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL_rwops.h>
#include <SDL_error.h>

#include <AppKit.h>
#include <StorageKit.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <fs_attr.h>


/*
 *  Find application and preferences directories
 */

string get_application_directory(void)
{
	app_info info;
	be_app->GetAppInfo(&info);
	BEntry entry(&info.ref);
	BPath path(&entry), dir;
	path.GetParent(&dir);
	return dir.Path();
}

string get_preferences_directory(void)
{
	BPath prefs_dir;
	find_directory(B_USER_SETTINGS_DIRECTORY, &prefs_dir, true);
	prefs_dir.Append("Aleph One");
	return prefs_dir.Path();
}


/*
 *  Handling of resource forks in BFS attributes. The BeOS HFS implementation
 *  makes the resource forks of files available as a file attribute with the
 *  name "MACOS:RFORK". We handle these attributes here to make it possible
 *  to copy Marathon data files from Mac CD-ROMs without any need for file
 *  conversion.
 */

#define ATTR_NAME "MACOS:RFORK"

bool has_rfork_attribute(const char *file)
{
	int fd = open(file, O_RDONLY);
	if (fd < 0)
		return false;

	struct attr_info info;
	bool result = (fs_stat_attr(fd, ATTR_NAME, &info) == 0);

	close(fd);
	return result;
}

struct rfork_data {
	int fd;			// fd of file
	off_t current;	// current position in attribute
	off_t size;		// size of attribute
};

static int rfork_seek(SDL_RWops *context, int offset, int whence)
{
	rfork_data *d = (rfork_data *)context->hidden.unknown.data1;

	off_t newpos;
	switch (whence) {
		case SEEK_SET:
			newpos = offset;
			break;
		case SEEK_CUR:
			newpos = d->current + offset;
			break;
		case SEEK_END:
			newpos = d->size + offset;
			break;
		default:
			SDL_SetError("Unknown value for 'whence'");
			return -1;
	}

	if (newpos < 0 || newpos > d->size) {
		SDL_Error(SDL_EFSEEK);
		return -1;
	}

	d->current = newpos;
	return newpos;
}

static int rfork_read(SDL_RWops *context, void *ptr, int size, int maxnum)
{
	rfork_data *d = (rfork_data *)context->hidden.unknown.data1;

	int num = maxnum;
	if (d->current + (num*size) > d->size)
		num = (d->size - d->current) / size;

	ssize_t actual = fs_read_attr(d->fd, ATTR_NAME, B_RAW_TYPE, d->current, ptr, num * size);
	if (actual < 0)
		return actual;
	else {
		d->current += actual;
		return actual / size;
	}
}

static int rfork_write(SDL_RWops *context, const void *ptr, int size, int maxnum)
{
	rfork_data *d = (rfork_data *)context->hidden.unknown.data1;

	int num = maxnum;
	if (d->current + (num*size) > d->size)
		num = (d->size - d->current) / size;

	ssize_t actual = fs_write_attr(d->fd, ATTR_NAME, B_RAW_TYPE, d->current, ptr, num * size);
	if (actual < 0)
		return actual;
	else {
		d->current += actual;
		return actual / size;
	}
}

static int rfork_close(SDL_RWops *context)
{
	if (context) {
		rfork_data *d = (rfork_data *)context->hidden.unknown.data1;
		close(d->fd);
		free(d);
		free(context);
	}
	return 0;
}

SDL_RWops *sdl_rw_from_rfork(const char *file, bool writable)
{
	SDL_RWops *rwops = NULL;

	int fd = open(file, writable ? O_RDWR : O_RDONLY);
	if (fd < 0) {
		SDL_SetError("Couldn't open %s", file);
		return NULL;
	} else {
		struct attr_info info;
		fs_stat_attr(fd, ATTR_NAME, &info);

		rwops = SDL_AllocRW();
		if (rwops == NULL) {
			close(fd);
			return NULL;
		}

		rfork_data *d = (rfork_data *)malloc(sizeof(struct rfork_data));
		if (d == NULL) {
			free(rwops);
			close(fd);
			return NULL;
		}

		d->fd = fd;
		d->current = 0;
		d->size = info.size;

		rwops->seek = rfork_seek;
		rwops->read = rfork_read;
		rwops->write = rfork_write;
		rwops->close = rfork_close;
		rwops->hidden.unknown.data1 = d;

		return rwops;
	}
}
