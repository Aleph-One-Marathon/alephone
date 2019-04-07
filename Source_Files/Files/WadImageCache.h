/*
 *  WadImageCache.h - an on-disk cache for thumbnail images in WAD files
 
	Copyright (C) 2015 and beyond by Jeremiah Morris
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

#ifndef WAD_IMAGE_CACHE_H
#define WAD_IMAGE_CACHE_H

#include "FileHandler.h"
#include "wad.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <list>
#include <map>

struct WadImageDescriptor {
	FileSpecifier file;
	uint32 checksum;
	short index;
	WadDataType tag;
	
	bool operator<(const WadImageDescriptor& other) const
	{
		if (file != other.file)
			return strcmp(file.GetPath(), other.file.GetPath()) < 0;
		else if (checksum != other.checksum)
			return checksum < other.checksum;
		else if (index != other.index)
			return index < other.index;
		else if (tag != other.tag)
			return tag < other.tag;
		return false;
	}
	
	bool operator==(const WadImageDescriptor& other) const {
		return file == other.file &&
		       checksum == other.checksum &&
			   index == other.index &&
			   tag == other.tag;
	}
};

class WadImageCache {
public:
	typedef boost::tuple<WadImageDescriptor, int, int> cache_key_t;
	typedef std::pair<std::string, size_t> cache_value_t;
	typedef std::pair<cache_key_t, cache_value_t> cache_pair_t;
	typedef std::list<cache_pair_t>::iterator cache_iter_t;

	static WadImageCache* instance();
	
	// Call this at startup, before any other calls.
	void initialize_cache();
	
	// Reads an image from a wad and returns it at original size.
	// Does not touch the cache.
	static SDL_Surface *image_from_desc(WadImageDescriptor& desc);
	
	// Returns true if image is in cache. Does not change LRU info.
	bool is_cached(WadImageDescriptor& desc, int width, int height) const;
	
	// Add image to cache if it doesn't exist (and original is at
	// a different size). Updates LRU info if already cached.
	// If "surface" is provided, uses that for caching instead of
	// reading wadfile directly.
	void cache_image(WadImageDescriptor& desc, int width, int height, SDL_Surface *surface = NULL);
	
	// Deletes cache data for this image. If width and height are zero,
	// removes any cached sizes for image.
	void remove_image(WadImageDescriptor& desc, int width = 0, int height = 0);
	
	// Loads surface for cached image. Returns NULL if not in cache.
	// If return value is not NULL, surface must be freed by caller.
	SDL_Surface *retrieve_image(WadImageDescriptor& desc, int width, int height);
	
	// Loads surface for image, caching it if necessary.
	// If return value is not NULL, surface must be freed by caller.
	// If "surface" is provided, uses that for caching instead of
	// reading wad file directly.
	SDL_Surface *get_image(WadImageDescriptor& desc, int width, int height, SDL_Surface *surface = NULL);

	void save_cache();
	void set_cache_autosave(bool enabled) { m_autosave = enabled; }
	
	size_t size() { return m_cachesize; }
	size_t limit() { return m_sizelimit; }
	void set_limit(size_t bytes) { m_sizelimit = bytes; if (apply_cache_limit()) autosave_cache(); }
	

private:
	WadImageCache() { }
	
	SDL_Surface *image_from_name(std::string& name) const;
	void delete_storage_for_name(std::string& name) const;
	SDL_Surface *resize_image(SDL_Surface *original, int width, int height) const;
	std::string image_to_new_name(SDL_Surface *image, int32 *filesize = NULL) const;
	std::string add_to_cache(cache_key_t key, SDL_Surface *surface);
	bool apply_cache_limit();
	std::string retrieve_name(WadImageDescriptor& desc, int width, int height, bool mark_accessed = true);
	void autosave_cache() { if (m_autosave) save_cache(); }
	
	std::list<cache_pair_t> m_used;
	std::map<cache_key_t, cache_iter_t> m_cacheinfo;
	size_t m_cachesize = 0;
	size_t m_sizelimit = 300000000;
	bool m_autosave = true;
	bool m_cache_dirty = false;
};


#endif
