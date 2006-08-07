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
#include <boost/function.hpp>

using namespace std;

Console *Console::m_instance = NULL;

Console *Console::instance() {
  if (!m_instance) {
    m_instance = new Console;
  }
  return m_instance;
}

void Console::enter() {
	// commands are processed before callbacks
	if (m_buffer[0] == '.')
	{
		// see if there's a command
		
		string command;
		string remainder;
		
		string::size_type pos = m_buffer.find(' ');
		if (pos != string::npos && pos < m_buffer.size())
		{
			remainder = m_buffer.substr(pos + 1);
			command = m_buffer.substr(1, pos);
		}
		else 
		{
			command = m_buffer.substr(1);
		}
		
		transform(command.begin(), command.end(), command.begin(), tolower);
		command_map::iterator it = m_commands.find(command);
		if (it != m_commands.end())
		{
			it->second(remainder);
		}
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

void Console::register_command(string command, boost::function<void(const string&)> f)
{
	transform(command.begin(), command.end(), command.begin(), tolower);
	m_commands[command] = f;
}

void Console::unregister_command(string command)
{
	transform(command.begin(), command.end(), command.begin(), tolower);
	m_commands.erase(command);
}
