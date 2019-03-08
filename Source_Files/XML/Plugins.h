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

#include "FileHandler.h"
#include <string>
#include <vector>

struct ScenarioInfo {
	std::string name;
	std::string scenario_id;
	std::string version;
};
struct ShapesPatch {
	bool requires_opengl;
	std::string path;
};

struct Plugin {
	DirectorySpecifier directory;
	std::string name;
	std::string description;
	std::string version;
	std::vector<std::string> mmls;
	std::string hud_lua;
	std::string solo_lua;
	std::string stats_lua;
	std::string theme;
	std::string required_version;
	std::vector<ShapesPatch> shapes_patches;
	std::vector<ScenarioInfo> required_scenarios;

	bool enabled;
	bool overridden;
	bool overridden_solo;
	bool compatible() const;
	bool allowed() const;
	bool valid() const;

	bool operator<(const Plugin& other) const {
		return name < other.name;
	}
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
	void load_mml();

	void load_shapes_patches(bool opengl);

	void disable(const std::string& path);

	iterator begin() { return m_plugins.begin(); }
	iterator end() { return m_plugins.end(); }

	const Plugin* find_hud_lua();
	const Plugin* find_solo_lua();
	const Plugin* find_stats_lua();
	const Plugin* find_theme();
private:
	Plugins() { }
	void add(Plugin plugin) { m_plugins.push_back(plugin); }
	void validate();

	std::vector<Plugin> m_plugins;
	bool m_validated = false;
	GameMode m_mode = kMode_Menu;
};


#endif
