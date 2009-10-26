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

#include "FileHandler.h"
#include "Logging.h"
#include "XML_ParseTreeRoot.h"

bool PluginHandler::GetData()
{
	if (m_data.size() == 0) {
		return false;
	}

	Buffer = &m_data[0];
	BufLen = m_data.size();
	LastOne = true;

	return true;
}

void PluginHandler::ReportReadError()
{
	logError1("Error reading %s plugin resources", m_name.c_str());
}

void PluginHandler::ReportParseError(const char* ErrorString, int LineNumber) 
{
	logError3("XML parsing error: %s at line %d in %s Plugin.mml", ErrorString, LineNumber, m_name.c_str());
}

const int MaxErrorsToShow = 7;

void PluginHandler::ReportInterpretError(const char* ErrorString)
{
	if (GetNumInterpretErrors() < MaxErrorsToShow) {
		logError(ErrorString);
	}
}

bool PluginHandler::RequestAbort()
{
	return (GetNumInterpretErrors() >= MaxErrorsToShow);
}

extern std::vector<DirectorySpecifier> data_search_path;

class SearchPathOverride 
{
public:
	SearchPathOverride(const DirectorySpecifier& dir) 
	{
		data_search_path.insert(data_search_path.begin(), dir);
	}

	~SearchPathOverride() 
	{
		data_search_path.erase(data_search_path.begin());
	}
};

bool PluginHandler::ParsePlugin(FileSpecifier& file_name)
{
	OpenedFile file;
	if (file_name.Open(file)) 
	{
		
		int32 data_size;
		file.GetLength(data_size);
		m_data.resize(data_size);

		if (file.Read(data_size, &m_data[0])) 
		{
			DirectorySpecifier dir;
			file_name.ToDirectory(dir);
			SearchPathOverride override(dir);

			char name[256];
			dir.GetName(name);
			m_name = name;
			
			if (!DoParse()) 
			{
				logError1("There were parsing errors in %s Plugin.mml\n", m_name.c_str());
			}
		}

		m_data.clear();
		return true;
	}
	return false;
}

bool PluginHandler::ParseDirectory(FileSpecifier& dir) 
{
	std::vector<dir_entry> de;
	if (!dir.ReadDirectory(de))
		return false;
	std::sort(de.begin(), de.end());
	
	for (std::vector<dir_entry>::const_iterator it = de.begin(); it != de.end(); ++it) {
		if (it->is_directory) {
			FileSpecifier file_name = dir + it->name + "Plugin.mml";
			ParsePlugin(file_name);
		}
	}
}

void LoadPlugins()
{
	logContext("parsing plugins");
	PluginHandler loader;
	loader.CurrentElement = &RootParser;
	
	// ParseDirectory messes with the search path!
	std::vector<DirectorySpecifier> search_path = data_search_path;
	for (std::vector<DirectorySpecifier>::const_iterator it = search_path.begin(); it != search_path.end(); ++it) {
		DirectorySpecifier path = *it + "Plugins";
		loader.ParseDirectory(path);
	}
}
