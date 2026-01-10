/*
 *  Plugins.h - a plugin manager

	Copyright (C) 2009 and beyond by Gregory Smith
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

#ifndef PLUGINS_H
#define PLUGINS_H

#include <list>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "FileHandler.h"

struct ScenarioInfo {
	std::string name;
	std::string scenario_id;
	std::string version;
};
struct ShapesPatch {
	bool requires_opengl;
	std::string path;
};

class SoloLuaWriteAccess {
public:
	// excludes all other Lua
	static constexpr uint32_t world = 0x01;

	// excludes all other "exclusive" Lua of this type
	static constexpr uint32_t music = 0x02;

	static constexpr uint32_t exclusive_mask = world | music;

	// allowed, and multiples allowed, with any types except world
	static constexpr uint32_t ephemera = 0x04;
	static constexpr uint32_t sound = 0x08;

	SoloLuaWriteAccess() : m_flags{world} { };
	SoloLuaWriteAccess(uint32_t flags) : m_flags{flags} { };
	SoloLuaWriteAccess(const std::string& csv);

	bool is_excluded(uint32_t flags) const;

	uint32_t get_flags() const { return m_flags; }
	uint32_t get_exclusive_flags() const;
	
private:
	uint32_t m_flags;
};

struct MapPatch
{
	std::set<uint32_t> parent_checksums;
	using resource_key_t = std::pair<uint32_t, int>;
	std::map<resource_key_t, std::string> resource_map;
};

struct Plugin {
	DirectorySpecifier directory;
	std::string name;
	std::string description;
	std::string version;
	std::vector<std::string> mmls;
	std::string hud_lua;
	std::string solo_lua;
	SoloLuaWriteAccess solo_lua_write_access;
	std::string stats_lua;
	std::string theme;
	std::string required_version;
	std::vector<ShapesPatch> shapes_patches;
	std::vector<ScenarioInfo> required_scenarios;
	std::vector<MapPatch> map_patches;

	bool auto_enable;
	bool enabled;
	bool overridden;
	bool overridden_solo;
	bool compatible() const;
	bool allowed() const;
	bool valid() const;

	bool operator<(const Plugin& other) const {
		return name < other.name;
	}

	bool get_resource(uint32_t checksum, uint32_t type, int id, LoadedResource& rsrc) const;
};

class Plugins {
	friend class PluginLoader;
public:
	static Plugins* instance();
	typedef std::vector<Plugin>::iterator iterator;
	
	enum GameMode { kMode_Menu, kMode_Solo, kMode_Net };
	
	void enumerate();
	void invalidate() { m_validated = false; }
	void set_mode(GameMode mode) { m_mode = mode; }
	GameMode mode() { return m_mode; }
	void load_mml(bool load_menu_mml_only);

	void load_shapes_patches(bool opengl);

	bool disable(const boost::filesystem::path& path);
	bool enable(const boost::filesystem::path& path);

	iterator begin() { return m_plugins.begin(); }
	iterator end() { return m_plugins.end(); }

	const Plugin* find_hud_lua();
	std::vector<const Plugin*> find_solo_lua();
	const Plugin* find_stats_lua();
	const Plugin* find_theme();

	bool get_resource(uint32_t type, int id, LoadedResource& rsrc);
	void set_map_checksum(uint32_t checksum);
private:
	Plugins() { }

	void add(const Plugin& plugin) { m_plugins.push_back(plugin); }
	void validate();

	std::vector<Plugin> m_plugins;
	bool m_validated = false;
	GameMode m_mode = kMode_Menu;

	std::stack<ScopedSearchPath, std::list<ScopedSearchPath>> m_search_paths;

	uint32_t m_map_checksum;
};


#endif
