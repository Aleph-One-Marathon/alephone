/*
 *  Plugins.cpp - plugin manager

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

#include "cseries.h"
#include "Plugins.h"

#include <algorithm>

#include "alephversion.h"
#include "FileHandler.h"
#include "game_errors.h"
#include "Logging.h"
#include "preferences.h"
#include "InfoTree.h"
#include "XML_ParseTreeRoot.h"
#include "Scenario.h"

#include <boost/algorithm/string/predicate.hpp>

namespace algo = boost::algorithm;

class PluginLoader {
public:
	PluginLoader() { }
	~PluginLoader() { }
	
	bool ParsePlugin(FileSpecifier& file);
	bool ParseDirectory(FileSpecifier& dir);
};

bool Plugin::compatible() const {
	if (required_version.size() > 0 && A1_DATE_VERSION < required_version)
		return false;

	if (required_scenarios.size() == 0)
		return true;
	std::string scenName = Scenario::instance()->GetName();
	std::string scenID = Scenario::instance()->GetID();
	std::string scenVers = Scenario::instance()->GetVersion();
	for (std::vector<ScenarioInfo>::const_iterator it = required_scenarios.begin(); it != required_scenarios.end(); ++it)
	{
		if ((it->name.empty() || it->name == scenName) &&
		    (it->scenario_id.empty() || it->scenario_id == scenID) &&
		    (it->version.empty() || it->version == scenVers))
			return true;
	}
	return false;
}
bool Plugin::allowed() const {
	if (stats_lua.empty() || network_preferences->allow_stats)
		return true;
	
	return false;
}
bool Plugin::valid() const {
	if (!enabled)
		return false;
	
	if (!environment_preferences->use_solo_lua &&
		Plugins::instance()->mode() == Plugins::kMode_Solo)
		return !overridden_solo;
	
	return !overridden;
}

bool Plugin::get_resource(uint32_t checksum, uint32_t type, int id, LoadedResource& rsrc) const
{
	for (auto it = map_patches.rbegin(); it != map_patches.rend(); ++it)
	{
		if (it->parent_checksums.count(checksum))
		{
			auto key = std::make_pair(type, id);
			auto rsrc_it = it->resource_map.find(key);
			if (rsrc_it != it->resource_map.end())
			{
				auto path = rsrc_it->second;
				ScopedSearchPath ssp(directory);
				FileSpecifier file;
				if (file.SetNameWithPath(path.c_str()))
				{
					OpenedFile ofile;
					if (file.Open(ofile))
					{
						int32 length;
						if (ofile.GetLength(length))
						{
							void *data = malloc(length);
							ofile.Read(length, data);
							rsrc.SetData(data, length);
							
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

Plugins* Plugins::instance() {
	static Plugins* m_instance = nullptr;
	if (!m_instance) {
		m_instance = new Plugins;
	}

	return m_instance;
}

bool Plugins::disable(const boost::filesystem::path& path) { //std path is not supported before mac os 10.15 so we are using boost path instead
	for (std::vector<Plugin>::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it) {
		if (it->directory.GetPath() == path) {
			it->enabled = false;
			m_validated = false;
			return true;
		}
	}

	return false;
}

bool Plugins::enable(const boost::filesystem::path& path)
{
	for (auto& p : m_plugins)
	{
		if (p.directory.GetPath() == path)
		{
			p.enabled = true;
			m_validated = false;
			return true;
		}
	}

	return false;
}

static void load_mmls(const Plugin& plugin, bool load_menu_mml_only)
{
	ScopedSearchPath ssp(plugin.directory);
	for (std::vector<std::string>::const_iterator it = plugin.mmls.begin(); it != plugin.mmls.end(); ++it) 
	{
		FileSpecifier file;
		if (file.SetNameWithPath(it->c_str()))
		{
			ParseMMLFromFile(file, load_menu_mml_only);
		}
		else
		{
			logWarning("%s Plugin: %s not found; ignoring", plugin.name.c_str(), it->c_str());
		}
	}
}

void Plugins::load_mml(bool load_menu_mml_only) {
	validate();

	for (std::vector<Plugin>::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it) 
	{
		if (it->valid())
		{
			load_mmls(*it, load_menu_mml_only);
		}
	}
}

void load_shapes_patch(SDL_RWops* p, bool override_replacements);

void Plugins::load_shapes_patches(bool is_opengl)
{
	validate();
	for (std::vector<Plugin>::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
	{
		if (it->valid())
		{
			ScopedSearchPath ssp(it->directory);

			for (std::vector<ShapesPatch>::iterator shapes_patch = it->shapes_patches.begin(); shapes_patch != it->shapes_patches.end(); ++shapes_patch)
			{
				if (is_opengl || !shapes_patch->requires_opengl)
				{
					FileSpecifier file;
					if (file.SetNameWithPath(shapes_patch->path.c_str()))
					{
						OpenedFile ofile;
						if (file.Open(ofile))
						{
							load_shapes_patch(ofile.GetRWops(), false);
						}
						
					}
					else
					{
						logWarning("%s Plugin: %s not found; ignoring", it->name.c_str(), shapes_patch->path.c_str());
					}
				}
			}
		}
	}
}

const Plugin* Plugins::find_hud_lua()
{
	validate();
	std::vector<Plugin>::const_reverse_iterator rend = m_plugins.rend();
	for (std::vector<Plugin>::const_reverse_iterator rit = m_plugins.rbegin(); rit != rend; ++rit)
	{
		if (rit->hud_lua.size() && rit->valid())
		{
			return &(*rit);
		}
	}
	
	return 0;
}

const Plugin* Plugins::find_solo_lua()
{
	validate();
	std::vector<Plugin>::const_reverse_iterator rend = m_plugins.rend();
	for (std::vector<Plugin>::const_reverse_iterator rit = m_plugins.rbegin(); rit != rend; ++rit)
	{
		if (rit->solo_lua.size() && rit->valid())
		{
			return &(*rit);
		}
	}

	return 0;
}

const Plugin* Plugins::find_stats_lua()
{
	validate();
	std::vector<Plugin>::const_reverse_iterator rend = m_plugins.rend();
	for (std::vector<Plugin>::const_reverse_iterator rit = m_plugins.rbegin(); rit != rend; ++rit)
	{
		if (rit->stats_lua.size() && rit->valid())
		{
			return &(*rit);
		}
	}
	
	return 0;
}

const Plugin* Plugins::find_theme()
{
	validate();
	std::vector<Plugin>::const_reverse_iterator rend = m_plugins.rend();
	for (std::vector<Plugin>::const_reverse_iterator rit = m_plugins.rbegin(); rit != rend; ++rit)
	{
		if (rit->theme.size() && rit->valid())
		{
			return &(*rit);
		}
	}

	return 0;
}

static bool plugin_file_exists(const Plugin& Data, std::string Path)
{
	FileSpecifier f = Data.directory + Path;
	return f.Exists();
}

static int utf8_to_int(const std::string& s)
{
	auto mac_roman = utf8_to_mac_roman(s);
	if (mac_roman.size() == 4)
	{
		return FOUR_CHARS_TO_INT(mac_roman[0],
								 mac_roman[1],
								 mac_roman[2],
								 mac_roman[3]);
	}
	else
	{
		return 0;
	}
}

bool PluginLoader::ParsePlugin(FileSpecifier& file_name)
{
	OpenedFile file;
	if (file_name.Open(file)) 
	{
		int32 data_size;
		file.GetLength(data_size);
		std::vector<char> file_data;
		file_data.resize(data_size);

		if (file.Read(data_size, &file_data[0]))
		{
			DirectorySpecifier current_plugin_directory;
			file_name.ToDirectory(current_plugin_directory);

			char name[256];
			current_plugin_directory.GetName(name);
			
			std::istringstream strm(std::string(file_data.begin(), file_data.end()));
			try {
				InfoTree root = InfoTree::load_xml(strm).get_child("plugin");
				
				Plugin Data = Plugin();
				Data.directory = current_plugin_directory;
				Data.enabled = true;

				Data.auto_enable = true;
				root.read_attr("auto_enable", Data.auto_enable);
				Data.enabled = Data.auto_enable;

				root.read_attr("name", Data.name);
				root.read_attr("version", Data.version);
				root.read_attr("description", Data.description);
				root.read_attr("minimum_version", Data.required_version);
				
				if (root.read_attr("hud_lua", Data.hud_lua) &&
					!plugin_file_exists(Data, Data.hud_lua))
					Data.hud_lua = "";
				
				if (root.read_attr("solo_lua", Data.solo_lua) &&
					!plugin_file_exists(Data, Data.solo_lua))
					Data.solo_lua = "";
				
				if (root.read_attr("stats_lua", Data.stats_lua) &&
					!plugin_file_exists(Data, Data.stats_lua))
					Data.stats_lua = "";
				
				if (root.read_attr("theme_dir", Data.theme) &&
					!plugin_file_exists(Data, Data.theme + "/theme2.mml"))
					Data.theme = "";
				
				for (const InfoTree &tree : root.children_named("mml"))
				{
					std::string mml_path;
					if (tree.read_attr("file", mml_path) &&
						plugin_file_exists(Data, mml_path))
						Data.mmls.push_back(mml_path);
				}

				for (const InfoTree &tree : root.children_named("shapes_patch"))
				{
					ShapesPatch patch;
					tree.read_attr("file", patch.path);
					tree.read_attr("requires_opengl", patch.requires_opengl);
					if (plugin_file_exists(Data, patch.path))
						Data.shapes_patches.push_back(patch);
				}

				for (const InfoTree &tree : root.children_named("scenario"))
				{
					ScenarioInfo info;
					tree.read_attr("name", info.name);
					if (info.name.size() > 31)
						info.name.erase(31);
					
					tree.read_attr("id", info.scenario_id);
					if (info.scenario_id.size() > 23)
						info.scenario_id.erase(23);
					
					tree.read_attr("version", info.version);
					if (info.version.size() > 7)
						info.version.erase(7);
					
					if (info.name.size() || info.scenario_id.size())
						Data.required_scenarios.push_back(info);
				}

				for (const InfoTree& tree : root.children_named("map_patch"))
				{
					MapPatch patch;
					for (const InfoTree& cs_tree : tree.children_named("checksum"))
					{
						auto cs = cs_tree.get_value(static_cast<uint32_t>(0));
						patch.parent_checksums.insert(cs);
					}

					for (const InfoTree& rsrc_tree : tree.children_named("resource"))
					{
						std::string path;
						int id;
						std::string type;
						
						rsrc_tree.read_attr("type", type);
						rsrc_tree.read_attr("id", id);
						rsrc_tree.read_attr("data", path);

						auto key = std::make_pair(utf8_to_int(type), id);
						if (key.first)
						{
							patch.resource_map.insert(std::make_pair(key, path));
						}
					}

					if (patch.parent_checksums.size() &&
						patch.resource_map.size())
					{
						Data.map_patches.push_back(patch);
					}
				}
				
				if (Data.name.length()) {
					std::sort(Data.mmls.begin(), Data.mmls.end());
					if (Data.theme.size()) {
						Data.hud_lua = "";
						Data.solo_lua = "";
						Data.shapes_patches.clear();
						Data.map_patches.clear();
					}
					Plugins::instance()->add(Data);
				}
				
			} catch (const InfoTree::parse_error& e) {
				logError("There were parsing errors in %s Plugin.xml: %s", name, e.what());
			} catch (const InfoTree::path_error& e) {
				logError("There were parsing errors in %s Plugin.xml: %s", name, e.what());
			} catch (const InfoTree::data_error& e) {
				logError("There were parsing errors in %s Plugin.xml: %s", name, e.what());
			} catch (const InfoTree::unexpected_error& e) {
				logError("There were parsing errors in %s Plugin.xml: %s", name, e.what());
			}
		}

		return true;
	}
	return false;
}

bool PluginLoader::ParseDirectory(FileSpecifier& dir) 
{
	std::vector<dir_entry> de;
	if (!dir.ReadDirectory(de))
		return false;
	
	for (std::vector<dir_entry>::const_iterator it = de.begin(); it != de.end(); ++it) {
		FileSpecifier file = dir + it->name;
		if (it->name == "Plugin.xml")
		{
			ParsePlugin(file);
		}
		else if (it->is_directory && it->name[0] != '.') 
		{
			ParseDirectory(file);
		}
		else if (algo::ends_with(it->name, ".zip") || algo::ends_with(it->name, ".ZIP"))
		{
			// search it for a Plugin.xml file
			for (const auto& zip_entry : file.ReadZIP())
			{
				if (zip_entry == "Plugin.xml" || algo::ends_with(zip_entry, "/Plugin.xml"))
				{
					std::string archive = file.GetPath();
					FileSpecifier file_name = FileSpecifier(archive.substr(0, archive.find_last_of('.'))) + zip_entry;
					ParsePlugin(file_name);
				}
			}
		}
	}

	return true;
}

extern std::vector<DirectorySpecifier> data_search_path;

void Plugins::enumerate() {

	logContext("parsing plugins");
	PluginLoader loader;
	
	for (std::vector<DirectorySpecifier>::const_iterator it = data_search_path.begin(); it != data_search_path.end(); ++it) {
		DirectorySpecifier path = *it + "Plugins";
		loader.ParseDirectory(path);
	}
	std::sort(m_plugins.begin(), m_plugins.end());
	clear_game_error();
	m_validated = false;
}

bool Plugins::get_resource(uint32_t type, int id, LoadedResource& rsrc)
{
	for (auto it = m_plugins.rbegin(); it != m_plugins.rend(); ++it)
	{
		if (it->enabled &&
			it->compatible() &&
			it->get_resource(m_map_checksum, type, id, rsrc))
		{
			return true;
		}
	}

	return false;
}

void Plugins::set_map_checksum(uint32_t checksum)
{
	m_map_checksum = checksum;

	// Prepend any plugins with patches that use this checksum to the search
	// path. This isn't the ideal solution: ideally, MML and Lua should use the
	// search path from which each component is drawn. For example, if Plugin A
	// replaces MML and Plugin B replaces Lua, then MML should use Plugin A
	// search path and Lua should use Plugin B search path. That is very
	// complicated, so let's try this simplified approach for now.

	while (!m_search_paths.empty())
	{
		m_search_paths.pop();
	}

	for (auto& p : m_plugins)
	{
		if (p.enabled &&
			p.compatible())
		{
			for (auto& patch : p.map_patches)
			{
				if (patch.parent_checksums.count(m_map_checksum))
				{
					m_search_paths.emplace(p.directory);
				}
			}
		}
	}
}

// enforce all-or-nothing loading of plugins which contain
// an "only one-at-a-time" item, like a Lua script or theme
void Plugins::validate()
{
	if (m_validated)
		return;
	m_validated = true;
	
	// determine active plugins including solo Lua
	bool found_solo_lua = false;
	bool found_hud_lua = false;
	bool found_stats_lua = false;
	bool found_theme = false;
	for (std::vector<Plugin>::reverse_iterator rit = m_plugins.rbegin(); rit != m_plugins.rend(); ++rit)
	{
		rit->overridden_solo = false;
		if (!rit->enabled || !rit->compatible() || !rit->allowed() ||
			(found_solo_lua && rit->solo_lua.size()) ||
			(found_hud_lua && rit->hud_lua.size()) ||
			(found_stats_lua && rit->stats_lua.size()) ||
			(found_theme && rit->theme.size()))
		{
			rit->overridden_solo = true;
			continue;
		}

		if (rit->solo_lua.size())
			found_solo_lua = true;
		if (rit->hud_lua.size())
			found_hud_lua = true;
		if (rit->stats_lua.size())
			found_stats_lua = true;
		if (rit->theme.size())
			found_theme = true;
	}
	
	// determine active plugins excluding solo Lua
	found_hud_lua = false;
	found_stats_lua = false;
	found_theme = false;
	for (std::vector<Plugin>::reverse_iterator rit = m_plugins.rbegin(); rit != m_plugins.rend(); ++rit)
	{
		rit->overridden = false;
		if (!rit->enabled || !rit->compatible() || !rit->allowed() ||
			(rit->solo_lua.size()) ||
			(found_hud_lua && rit->hud_lua.size()) ||
			(found_stats_lua && rit->stats_lua.size()) ||
			(found_theme && rit->theme.size()))
		{
			rit->overridden = true;
			continue;
		}
		
		if (rit->hud_lua.size())
			found_hud_lua = true;
		if (rit->stats_lua.size())
			found_stats_lua = true;
		if (rit->theme.size())
			found_theme = true;
	}
}
