/*
 *  Plugins.h - a plugin manager

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

#ifndef PLUGINS_H
#define PLUGINS_H

#include "XML_Configure.h"
#include <string>
#include <vector>

class FileSpecifier;

class PluginHandler : public XML_Configure {
public:
	PluginHandler() { }
	~PluginHandler() { }
	
	bool ParsePlugin(FileSpecifier& file);
	bool ParseDirectory(FileSpecifier& dir);

protected:
	virtual bool GetData();
	virtual void ReportReadError();
	virtual void ReportParseError(const char *ErrorString, int LineNumber);
	virtual void ReportInterpretError(const char* ErrorString);
	virtual bool RequestAbort();

private:
	std::string m_name;
	std::vector<char> m_data;
};

void LoadPlugins();

#endif
