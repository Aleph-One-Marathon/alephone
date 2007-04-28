/*
 *  Console.cpp - console utilities for Aleph One

 Copyright (C) 2005 and beyond by Gregory Smith
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
#include "Console.h"
#include "Logging.h"

#include <string>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace std;

Console *Console::m_instance = NULL;

Console *Console::instance() {
	if (!m_instance) {
		m_instance = new Console;
	}
	return m_instance;
}

static inline void lowercase(string& s)
{
	transform(s.begin(), s.end(), s.begin(), tolower);
}

static pair<string, string> split(string buffer)
{
	string command;
	string remainder;

	string::size_type pos = buffer.find(' ');
	if (pos != string::npos && pos < buffer.size())
	{
		remainder = buffer.substr(pos + 1);
		command = buffer.substr(0, pos);
	}
	else
	{
		command = buffer;
	}

	return pair<string, string>(command, remainder);
}

void CommandParser::register_command(string command, boost::function<void(const string&)> f)
{
	lowercase(command);
	m_commands[command] = f;
}

void CommandParser::register_command(string command, const CommandParser& command_parser)
{
	lowercase(command);
	m_commands[command] = boost::bind(&CommandParser::parse_and_execute, command_parser, _1);
}

void CommandParser::unregister_command(string command)
{
	lowercase(command);
	m_commands.erase(command);
}

void CommandParser::parse_and_execute(const std::string& command_string)
{
	pair<string, string> cr = split(command_string);

	string command = cr.first;
	string remainder = cr.second;

	lowercase(command);
	
	command_map::iterator it = m_commands.find(command);
	if (it != m_commands.end())
	{
		it->second(remainder);
	}
}


void Console::enter() {
	// macros are processed first
	if (m_buffer[0] == '.')
	{
		pair<string, string> mr = split(m_buffer.substr(1));

		string input = mr.first;
		string output = mr.second;

		lowercase(input);

		std::map<string, string>::iterator it = m_macros.find(input);
		if (it != m_macros.end())
		{
			if (output != "")
				m_buffer = it->second + " " + output;
			else
				m_buffer = it->second;
		}
	}

	// commands are processed before callbacks
	if (m_buffer[0] == '.')
	{
		parse_and_execute(m_buffer.substr(1));
	} else if (!m_callback) {
		logAnomaly("console enter activated, but no callback set");
	} else {
		m_callback(m_buffer);
	}
	
	m_callback.clear();
	m_buffer.clear();
	m_displayBuffer.clear();
	m_active = false;
#if defined(SDL)
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(0);
#endif
}

void Console::abort() {
	m_buffer.clear();
	m_displayBuffer.clear();
	if (!m_callback) {
		logAnomaly("console abort activated, but no callback set");
	} else {
		m_callback(m_buffer);
	}

	m_callback.clear();
	m_active = false;
#if defined(SDL)
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(0);
#endif
}

void Console::backspace() {
	if (!m_buffer.empty()) {
		m_buffer.erase(m_buffer.size() - 1);
		m_displayBuffer.erase(m_displayBuffer.size() - 2);
		m_displayBuffer += "_";
	}
}

void Console::clear() {
	m_buffer.clear();
	m_displayBuffer = m_prompt + " _";
}

void Console::key(const char c) {
	m_buffer += c;
	m_displayBuffer.erase(m_displayBuffer.size() - 1);
	m_displayBuffer += c;
	m_displayBuffer += "_";
}

void Console::activate_input(boost::function<void (const std::string&)> callback,
			     const std::string& prompt)
{
	assert(!m_active);
	m_callback = callback;
	m_buffer.clear();
	m_displayBuffer = m_prompt = prompt;
	m_displayBuffer += " _";
	m_active = true;

#if defined(SDL)
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1);
#endif
}

void Console::deactivate_input() {
	m_buffer.clear();
	m_displayBuffer.clear();
	
	m_callback.clear();
	m_active = false;
#if defined(SDL)
	SDL_EnableKeyRepeat(0, 0);
	SDL_EnableUNICODE(0);
#endif
}

void Console::register_macro(string input, string output)
{
	lowercase(input);
	m_macros[input] = output;
}

void Console::unregister_macro(string input)
{
	lowercase(input);
	m_macros.erase(input);
}

class XML_MacroParser : public XML_ElementParser
{
	string input;
	string output;
public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();

	XML_MacroParser() : XML_ElementParser("macro") {}
};

bool XML_MacroParser::Start()
{
	input.clear();
	output.clear();
	return true;
}

bool XML_MacroParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (StringsEqual(Tag, "input"))
	{
		input = Value;
		return true;
	}
	else if (StringsEqual(Tag, "output"))
	{
		output = Value;
		return true;
	}

	UnrecognizedTag();
	return false;
}

bool XML_MacroParser::AttributesDone()
{
	if (input == "")
	{
		AttribsMissing();
		return false;
	}

	Console::instance()->register_macro(input, output);
	return true;
}

static XML_MacroParser MacroParser;
static XML_ElementParser MacrosParser("macros");

XML_ElementParser *Console_GetParser()
{
	MacrosParser.AddChild(&MacroParser);

	return &MacrosParser;
}

