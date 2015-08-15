/*

	Copyright (C) 2006 and beyond by Bungie Studios, Inc.
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

	Scenario tag parser
	by Gregory Smith 2006


	This is for handling scenario compatibility info
*/

#include "cseries.h"
#include "Scenario.h"
#include "InfoTree.h"

Scenario *Scenario::m_instance;

Scenario *Scenario::instance()
{
	if (!m_instance) m_instance = new Scenario();
	return m_instance;
}

void Scenario::AddCompatible(const string Compatible)
{
	m_compatibleVersions.push_back(string(Compatible, 0, 23));
}

bool Scenario::IsCompatible(const string Compatible)
{
	if (Compatible == "" || m_id == "") return true;
	if (Compatible == m_id) return true;
	for (int i = 0; i < m_compatibleVersions.size(); i++)
	{
		if (m_compatibleVersions[i] == Compatible) return true;
	}

	return false;
}

class XML_ScenarioParser : public XML_ElementParser
{
public:
	bool HandleAttribute(const char *Tag, const char *Value);

	XML_ScenarioParser() : XML_ElementParser("scenario") { }
};

bool XML_ScenarioParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag, "name"))
	{
		Scenario::instance()->SetName(Value);
		return true;
	}
	else if (StringsEqual(Tag, "version"))
	{
		Scenario::instance()->SetVersion(Value);
		return true;
	}
	else if (StringsEqual(Tag, "id"))
	{
		Scenario::instance()->SetID(Value);
		return true;
	}
	
	UnrecognizedTag();
	return false;
}

class XML_CanJoinParser : public XML_ElementParser
{
public:
	bool HandleString(const char *String, int Length);
	
	XML_CanJoinParser() : XML_ElementParser("can_join") { }
};

bool XML_CanJoinParser::HandleString(const char *String, int Length)
{
	Scenario::instance()->AddCompatible(string(String, Length));
	return true;
}

static XML_ScenarioParser ScenarioParser;

static XML_CanJoinParser CanJoinParser;


XML_ElementParser *Scenario_GetParser()
{	
	ScenarioParser.AddChild(&CanJoinParser);
	return &ScenarioParser;
}

void reset_mml_scenario()
{
	// no reset
}

void parse_mml_scenario(const InfoTree& root)
{
	std::string str;
	if (root.read_attr("name", str))
		Scenario::instance()->SetName(str);
	if (root.read_attr("id", str))
		Scenario::instance()->SetID(str);
	if (root.read_attr("version", str))
		Scenario::instance()->SetVersion(str);
	
	BOOST_FOREACH(InfoTree can_join, root.children_named("can_join"))
	{
		std::string compat = can_join.get_value("");
		if (compat.size())
			Scenario::instance()->AddCompatible(compat);
	}
}
