/*
 *  WadImageCache.cpp - an on-disk cache for thumbnail images in WAD files
 
	Copyright (C) 2014 and beyond by Jeremiah Morris
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

#include "cseries.h"
#include "WadImageCache.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "InfoTree.h"

#ifdef HAVE_SDL_IMAGE
#include <SDL_image.h>
#endif
#ifdef HAVE_PNG
#include "IMG_savepng.h"
#endif

#include "game_errors.h"
#include "sdl_resize.h"
#include "Logging.h"

WadImageCache* WadImageCache::instance() {
	static WadImageCache *m_instance = nullptr;
	if (!m_instance) {
		m_instance = new WadImageCache;
	}
	
	return m_instance;
}

SDL_Surface *WadImageCache::image_from_desc(WadImageDescriptor& desc)
{
	SDL_Surface *surface = NULL;
	OpenedFile wad_file;
	if (open_wad_file_for_reading(desc.file, wad_file))
	{
		struct wad_header header;
		if (read_wad_header(wad_file, &header))
		{
			struct wad_data *wad;
			wad = read_indexed_wad_from_file(wad_file, &header, desc.index, true);
			if (wad)
			{
				void *data;
				size_t length;
				data = extract_type_from_wad(wad, desc.tag, &length);
				if (data && length)
				{
					SDL_RWops *rwops = SDL_RWFromConstMem(data, length);
#ifdef HAVE_SDL_IMAGE
					surface = IMG_Load_RW(rwops, 1);
#else
					surface = SDL_LoadBMP_RW(rwops, 1);
#endif
				}
				free_wad(wad);
			}
		}
		close_wad_file(wad_file);
	}
	clear_game_error();
	return surface;
}

SDL_Surface *WadImageCache::image_from_name(std::string& name) const
{
	FileSpecifier file;
	file.SetToImageCacheDir();
	file.AddPart(name);
	
	OpenedFile of;
	if (!file.Open(of))
		return NULL;
	
#ifdef HAVE_SDL_IMAGE
	SDL_Surface *img = IMG_Load_RW(of.GetRWops(), 0);
#else
	SDL_Surface *img = SDL_LoadBMP_RW(of.GetRWops(), 0);
#endif
	return img;
}

void WadImageCache::delete_storage_for_name(std::string& name) const
{
	FileSpecifier file;
	file.SetToImageCacheDir();
	file.AddPart(name);
	file.Delete();
}

SDL_Surface *WadImageCache::resize_image(SDL_Surface *original, int width, int height) const
{
	if (original) {
		if (original->w != width || original->h != height) {
			return SDL_Resize(original, width, height, false, 1);
		}
	}
	return NULL;
}

std::string WadImageCache::image_to_new_name(SDL_Surface *image, int32 *filesize) const
{
	// create name
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	std::string ustr = boost::uuids::to_string(u);
	
	FileSpecifier File;
	File.SetToImageCacheDir();
	File.AddPart(ustr);
	
	FileSpecifier TempFile;
	TempFile.SetTempName(File);
	
	int ret;
//#if defined(HAVE_PNG) && defined(HAVE_SDL_IMAGE)
//	ret = aoIMG_SavePNG(TempFile.GetPath(), image, IMG_COMPRESS_DEFAULT, NULL, 0);
#ifdef HAVE_SDL_IMAGE
	ret = IMG_SavePNG(image, TempFile.GetPath());
#else
	ret = SDL_SaveBMP(image, TempFile.GetPath());
#endif
	if (ret == 0 && TempFile.Rename(File))
	{
		if (filesize)
		{
			OpenedFile of;
			if (File.Open(of))
				of.GetLength(*filesize);
		}
		return ustr;
	}
	
	if (filesize)
		*filesize = 0;
	return "";
}

std::string WadImageCache::add_to_cache(cache_key_t key, SDL_Surface *surface)
{
	int32 filesize = 0;
	std::string name = image_to_new_name(surface, &filesize);
	if (!name.empty())
	{
		m_used.push_front(cache_pair_t(key, cache_value_t(name, filesize)));
		m_cacheinfo[key] = m_used.begin();
		m_cache_dirty = true;
		
		m_cachesize += filesize;
		apply_cache_limit();
		autosave_cache();
	}
	return name;
}

bool WadImageCache::apply_cache_limit()
{
	bool deleted = false;
	while (m_cachesize > m_sizelimit && m_used.size())
	{
		deleted = true;
		cache_pair_t last_item = m_used.back();
		m_used.pop_back();
		delete_storage_for_name(last_item.second.first);
		m_cachesize -= last_item.second.second;
		m_cacheinfo.erase(last_item.first);
		m_cache_dirty = true;
	}
	return deleted;
}

std::string WadImageCache::retrieve_name(WadImageDescriptor& desc, int width, int height, bool mark_accessed)
{
	cache_key_t key = cache_key_t(desc, width, height);
	
	std::map<cache_key_t, cache_iter_t>::iterator it = m_cacheinfo.find(key);
	if (it != m_cacheinfo.end()) {
		if (mark_accessed && it->second != m_used.begin())
		{
			m_used.splice(m_used.begin(), m_used, it->second);
			m_cache_dirty = true;
		}
		return it->second->second.first;
	}
	return std::string("");
}

bool WadImageCache::is_cached(WadImageDescriptor& desc, int width, int height) const
{
	cache_key_t key = cache_key_t(desc, width, height);
	
	return (m_cacheinfo.find(key) != m_cacheinfo.end());
}

void WadImageCache::cache_image(WadImageDescriptor& desc, int width, int height, SDL_Surface *image)
{
	std::string name = retrieve_name(desc, width, height, true);
	if (!name.empty())
	{
		autosave_cache();
		return;
	}
	
	SDL_Surface *resized_image = NULL;
	if (image)
	{
		resized_image = resize_image(image, width, height);
	}
	else
	{
		image = image_from_desc(desc);
		if (image)
		{
			resized_image = resize_image(image, width, height);
			SDL_FreeSurface(image);
		}
	}
	if (!resized_image)
		return;
	
	name = add_to_cache(cache_key_t(desc, width, height), resized_image);
	SDL_FreeSurface(resized_image);
}

void WadImageCache::remove_image(WadImageDescriptor& desc, int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		// Partial key specified; walk the map to find all matches
		for (std::map<cache_key_t, cache_iter_t>::iterator it = m_cacheinfo.begin(); it != m_cacheinfo.end(); )
		{
			if (boost::tuples::get<0>(it->first) == desc)
			{
				delete_storage_for_name(it->second->second.first);
				m_used.erase(it->second);
				m_cacheinfo.erase(it++);
				m_cache_dirty = true;
			}
			else
			{
				++it;
			}
		}
	}
	else
	{
		cache_key_t key = cache_key_t(desc, width, height);
		
		std::map<cache_key_t, cache_iter_t>::iterator it = m_cacheinfo.find(key);
		if (it != m_cacheinfo.end()) {
			delete_storage_for_name(it->second->second.first);
			m_used.erase(it->second);
			m_cacheinfo.erase(it);
			m_cache_dirty = true;
		}
	}
	autosave_cache();
}

SDL_Surface *WadImageCache::retrieve_image(WadImageDescriptor& desc, int width, int height)
{
	std::string name = retrieve_name(desc, width, height, true);
	if (name.empty())
		return NULL;
	
	return image_from_name(name);
}

SDL_Surface *WadImageCache::get_image(WadImageDescriptor& desc, int width, int height, SDL_Surface *image)
{
	SDL_Surface *surface = retrieve_image(desc, width, height);
	if (surface)
		return surface;

	if (image)
	{
		surface = resize_image(image, width, height);
	}
	else
	{
		image = image_from_desc(desc);
		if (image)
		{
			surface = resize_image(image, width, height);
			SDL_FreeSurface(image);
		}
	}

	if (surface)
	{
		add_to_cache(cache_key_t(desc, width, height), surface);
	}
	return surface;
}

void WadImageCache::initialize_cache()
{
	FileSpecifier info;
	info.SetToImageCacheDir();
	info.AddPart("Cache.ini");
	if (!info.Exists())
		return;
	
	InfoTree pt;
	try {
		pt = InfoTree::load_ini(info);
	} catch (InfoTree::ini_error e) {
		logError("Could not read image cache from %s (%s)", info.GetPath(), e.what());
	}
	
	for (InfoTree::iterator it = pt.begin(); it != pt.end(); ++it)
	{
		std::string name = it->first;
		InfoTree ptc = it->second;
		
		WadImageDescriptor desc;
		
		std::string path;
		ptc.read("path", path);
		desc.file = FileSpecifier(path);
		
		ptc.read("checksum", desc.checksum);
		ptc.read("index", desc.index);
		ptc.read("tag", desc.tag);
		
		int width = 0;
		ptc.read("width", width);
		int height = 0;
		ptc.read("height", height);
		size_t filesize = 0;
		ptc.read("filesize", filesize);
		
		cache_key_t key = cache_key_t(desc, width, height);
		cache_value_t val = cache_value_t(name, filesize);
		m_used.push_front(cache_pair_t(key, val));
		m_cacheinfo[key] = m_used.begin();
		m_cachesize += filesize;
	}
}

void WadImageCache::save_cache()
{
	if (!m_cache_dirty)
		return;
	
	InfoTree pt;
	
	for (cache_iter_t it = m_used.begin(); it != m_used.end(); ++it)
	{
		std::string name = it->second.first;
		WadImageDescriptor desc = boost::tuples::get<0>(it->first);
		
		pt.put(name + ".path", desc.file.GetPath());
		pt.put(name + ".checksum", desc.checksum);
		pt.put(name + ".index", desc.index);
		pt.put(name + ".tag", desc.tag);
		pt.put(name + ".width", boost::tuples::get<1>(it->first));
		pt.put(name + ".height", boost::tuples::get<2>(it->first));
		pt.put(name + ".filesize", it->second.second);
	}
	
	FileSpecifier info;
	info.SetToImageCacheDir();
	info.AddPart("Cache.ini");
	try {
		pt.save_ini(info);
		m_cache_dirty = false;
	} catch (InfoTree::ini_error e) {
		logError("Could not save image cache to %s (%s)", info.GetPath(), e.what());
		return;
	}
}

