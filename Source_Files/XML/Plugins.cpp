/*
 *  Plugins.cpp - plugin manager

	Copyright (C) 2009 and beyond by Gregory Smith
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

#include "cseries.h"
#include "Plugins.h"

#include <algorithm>

#include "alephversion.h"
#include "FileHandler.h"
#include "Logging.h"
#include "XML_Loader_SDL.h"
#include "XML_ParseTreeRoot.h"

bool Plugin::compatible() {
	return (required_version.size() == 0 || A1_DATE_VERSION >= required_version);
}

Plugins* Plugins::m_instance = 0;
Plugins* Plugins::instance() {
	if (!m_instance) {
		m_instance = new Plugins;
	}

	return m_instance;
}

void Plugins::disable(const std::string& path) {
	for (std::vector<Plugin>::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it) {
		if (it->directory == path) {
			it->enabled = false;
			return;
		}
	}
}

extern std::vector<DirectorySpecifier> data_search_path;

class ModifySearchPath {
public:
	ModifySearchPath(DirectorySpecifier dir) {
		data_search_path.insert(data_search_path.begin(), dir);
	}
	~ModifySearchPath() {
		data_search_path.erase(data_search_path.begin());
	}
};

void Plugins::load_mml() {
	XML_Loader_SDL loader;
	loader.CurrentElement = &RootParser;

	for (std::vector<Plugin>::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it) {
		if (it->enabled && it->compatible()) {
			ModifySearchPath msp(it->directory);
			for (std::vector<std::string>::iterator mml = it->mmls.begin(); mml != it->mmls.end(); ++mml) {
				FileSpecifier file;
				file.SetNameWithPath(mml->c_str());
				loader.ParseFile(file);
			}
		}
	}
}

static Plugin Data;

class XML_PluginMMLParser : public XML_ElementParser 
{
public:
	bool HandleAttribute(const char* Tag, const char* Value);

	XML_PluginMMLParser() : XML_ElementParser("mml") {}
};

bool XML_PluginMMLParser::HandleAttribute(const char* Tag, const char* Value)
{
	if (StringsEqual(Tag, "file")) {
		Data.mmls.push_back(Value);
		return true;
	}

	UnrecognizedTag();
	return false;
}

XML_PluginMMLParser PluginMMLParser;

static DirectorySpecifier current_plugin_directory;

class XML_PluginParser : public XML_ElementParser
{
	friend class XML_PluginMMLParser;
public:
	bool Start();
	bool HandleAttribute(const char* Tag, const char* Value);
	bool AttributesDone();
	bool End();
	
	XML_PluginParser() : XML_ElementParser("plugin") {}
};

bool XML_PluginParser::Start() {
	Data = Plugin();
	Data.directory = current_plugin_directory;
	Data.enabled = true;
	return true;
}

bool XML_PluginParser::HandleAttribute(const char* Tag, const char* Value)
{
	if (StringsEqual(Tag, "name")) {
		Data.name = Value;
		return true;
	} else if (StringsEqual(Tag, "version")) {
		Data.version = Value;
		return true;
	} else if (StringsEqual(Tag, "description")) {
		Data.description = Value;
		return true;
	} else if (StringsEqual(Tag, "minimum_version")) {
		Data.required_version = Value;
		return true;
	}

	UnrecognizedTag();
	return false;
}

bool XML_PluginParser::AttributesDone()
{
	if (Data.name == "") {
		AttribsMissing();
		return false;
	}
	
	return true;
}

bool XML_PluginParser::End() {
	std::sort(Data.mmls.begin(), Data.mmls.end());
	Plugins::instance()->add(Data);
	return true;
}

XML_ElementParser PluginRootParser("");
XML_PluginParser PluginParser;

bool PluginLoader::GetData()
{
	if (m_data.size() == 0) {
		return false;
	}

	Buffer = &m_data[0];
	BufLen = m_data.size();
	LastOne = true;

	return true;
}

void PluginLoader::ReportReadError()
{
	logError1("Error reading %s plugin resources", m_name.c_str());
}

void PluginLoader::ReportParseError(const char* ErrorString, int LineNumber) 
{
	logError3("XML parsing error: %s at line %d in %s Plugin.xml", ErrorString, LineNumber, m_name.c_str());
}

const int MaxErrorsToShow = 7;

void PluginLoader::ReportInterpretError(const char* ErrorString)
{
	if (GetNumInterpretErrors() < MaxErrorsToShow) {
		logError(ErrorString);
	}
}

bool PluginLoader::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}

bool PluginLoader::ParsePlugin(FileSpecifier& file_name)
{
	OpenedFile file;
	if (file_name.Open(file)) 
	{
		
		int32 data_size;
		file.GetLength(data_size);
		m_data.resize(data_size);

		if (file.Read(data_size, &m_data[0])) 
		{
			file_name.ToDirectory(current_plugin_directory);

			char name[256];
			current_plugin_directory.GetName(name);
			m_name = name;
			
			if (!DoParse()) 
			{
				logError1("There were parsing errors in %s Plugin.xml\n", m_name.c_str());
			}
		}

		m_data.clear();
		return true;
	}
	return false;
}

bool PluginLoader::ParseDirectory(FileSpecifier& dir) 
{
	std::vector<dir_entry> de;
	if (!dir.ReadDirectory(de))
		return false;
	std::sort(de.begin(), de.end());
	
	for (std::vector<dir_entry>::const_iterator it = de.begin(); it != de.end(); ++it) {
		if (it->is_directory) {
			FileSpecifier file_name = dir + it->name + "Plugin.xml";
			ParsePlugin(file_name);
		}
	}

	return true;
}

void Plugins::enumerate() {
	PluginParser.AddChild(&PluginMMLParser);
	PluginRootParser.AddChild(&PluginParser);

	logContext("parsing plugins");
	PluginLoader loader;
	loader.CurrentElement = &PluginRootParser;
	
	for (std::vector<DirectorySpecifier>::const_iterator it = data_search_path.begin(); it != data_search_path.end(); ++it) {
		DirectorySpecifier path = *it + "Plugins";
		loader.ParseDirectory(path);
	}
}
