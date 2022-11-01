/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

#ifndef MAC_APP_STORE

#include "sdl_dialogs.h"
#include "sdl_fonts.h"
#include "sdl_widgets.h"

#include <string_view>
#include <sstream>

namespace {
	// string_view::starts_with is new in C++20...
	bool starts_with(const std::string_view& haystack, const std::string_view& needle) {
		return haystack.size() >= needle.size() && haystack.substr(0, needle.size()) == needle;
	}
	class LineMuncher {
		std::string_view rest;
		size_t line_no;
	public:
		LineMuncher(std::string_view rest) : rest(rest), line_no(0) {
			// Skip the signature of ultimate evil, if we see it
			if(starts_with(rest, "\xEF\xBB\xBF")) {
				rest.remove_prefix(3);
			}
		}
		bool yield_line(std::string_view& out_view) {
			if(rest.empty()) return false;
			auto newline_pos = rest.find('\n');
			if(newline_pos == std::string_view::npos) {
				out_view = rest;
				rest = "";
			}
			else {
				out_view = rest.substr(0, newline_pos);
				rest = rest.substr(newline_pos+1);
			}
			if(out_view.size() > 0 && out_view[out_view.size()-1] == '\r')
				out_view.remove_suffix(1);
			++line_no;
			return true;
		}
		size_t get_last_line_number() const {
			return line_no;
		}
		std::string get_rest() const {
			std::string ret(rest);
			auto p = ret.find('\r');
			while(p != std::string::npos) {
				ret.erase(p);
				p = ret.find('\r');
			}
			return ret;
		}
	};
	class Option {
		std::string_view instigating_line;
	protected:
		Option(std::string_view instigating_line) : instigating_line(instigating_line) {}
	public:
		virtual ~Option() {}
	    void reoutput(std::ostringstream& out) {
			out << instigating_line << "\n";
			synthesize(out);
		}
		virtual void synthesize(std::ostringstream& out) {}
		virtual void add_widget(table_placer* placer, dialog& d) = 0;
		static std::unique_ptr<Option> parse(std::string_view);
	};
	class Spacer : public Option {
	public:
		virtual ~Spacer() {}
		Spacer(std::string_view instigating_line) : Option(instigating_line) {}
		void add_widget(table_placer* placer, dialog& d) override {
			placer->add_row(new w_spacer);
		}
	};
	class Text : public Option {
		std::string label;
		w_static_text* widget;
	public:
		virtual ~Text() {}
		Text(std::string_view instigating_line, std::string_view label) : Option(instigating_line), label(label) {
			widget = new w_static_text(this->label.c_str());
		}
		void add_widget(table_placer* placer, dialog& d) override {
			placer->dual_add_row(widget, d);
		}
	};
	class Toggle : public Option {
		std::string variable_name;
		std::string label;
		w_toggle* widget;
		w_label* label_widget;
	public:
		virtual ~Toggle() {}
		Toggle(std::string_view instigating_line, std::string_view variable_name, std::string_view label, bool is_on) : Option(instigating_line), variable_name(variable_name), label(label) {
			widget = new w_toggle(is_on);
			label_widget = widget->label(this->label.c_str());
		}
		void add_widget(table_placer* placer, dialog& d) override {
			placer->dual_add(label_widget, d);
			placer->dual_add(widget, d);
		}
		void synthesize(std::ostringstream& out) override {
			out << "local " << variable_name << " = ";
			if(widget->get_selection())
				out << "true\n";
			else
				out << "false\n";
		}
	};
	class Select : public Option {
		std::string variable_name;
		std::string label;
		std::vector<std::string> options;
		std::vector<const char*> option_c_strs; // >:( -SB
		w_select* widget;
		w_label* label_widget;
	public:
		virtual ~Select() {}
		Select(std::string_view instigating_line, std::string_view variable_name, std::string_view label, std::vector<std::string> options, size_t selection) : Option(instigating_line), variable_name(variable_name), label(label), options(options) {
			for(auto& str : this->options) {
				option_c_strs.push_back(str.c_str());
			}
			option_c_strs.push_back(nullptr);
			widget = new w_select(selection, &option_c_strs[0]);
			label_widget = widget->label(this->label.c_str());
		}
		void add_widget(table_placer* placer, dialog& d) override {
			placer->dual_add(label_widget, d);
			placer->dual_add(widget, d);
		}
		void synthesize(std::ostringstream& out) override {
			out << "local " << variable_name << " = " << widget->get_selection()+1 << "\n";
		}
	};
	void parse_failure(LineMuncher& lines, std::string error) {
		std::ostringstream out;
		out << "Error on line " << lines.get_last_line_number() << ": " << error;
		alert_user(out.str().c_str());
	}
	void unexpected_eof(LineMuncher& lines) {
		parse_failure(lines, "File ended without reaching \"--! end configuration\"");
	}
	bool parse_config_line(std::string_view rest, std::vector<std::string>& parsed, LineMuncher& lines) {
		if(rest.size() < 5 || rest.substr(0, 4) != "--! ") {
			parse_failure(lines, "Expected another \"--! \" line, but didn't get one");
			return false;
		}
		rest.remove_prefix(4); // strip off the "--! "
		while(!rest.empty()) {
			std::string element;
			while(!rest.empty()) {
				if(rest[0] == '"') {
					rest.remove_prefix(1);
					while(!rest.empty() && rest[0] != '"') {
						if(rest[0] == '\\') {
							rest.remove_prefix(1);
							if(rest.empty()) {
								parse_failure(lines, "Unmatched backslash at end of line");
								return false;
							}
						}
						element.push_back(rest[0]);
						rest.remove_prefix(1);
					}
					if(rest.empty()) {
						parse_failure(lines, "Unmatched double quote (\")");
						return false;
					}
					assert(rest[0] == '"');
					rest.remove_prefix(1);
				}
				else if(rest[0] == '\\') {
					rest.remove_prefix(1);
					if(rest.empty()) {
						parse_failure(lines, "Unmatched backslash at end of line");
						return false;
					}
					element.push_back(rest[0]);
					rest.remove_prefix(1);
				}
				else if(rest[0] == ' ') {
					parsed.push_back(element);
					element.clear();
					rest.remove_prefix(1);
				}
				else {
					element.push_back(rest[0]);
					rest.remove_prefix(1);
				}
			}
			parsed.push_back(element);
			while(!rest.empty() && rest[0] == ' ') {
				rest.remove_prefix(1);
			}
		}
		return parsed.size() > 0;
	}
	bool check_lua_line(std::string_view lua_line, std::string_view variable_name, std::string_view& value) {
		bool parsed_ok = false;
		if(starts_with(lua_line, "local ")) {
			lua_line.remove_prefix(6);
			if(starts_with(lua_line, variable_name)) {
				lua_line.remove_prefix(variable_name.size());
				if(starts_with(lua_line, " = ")) {
					lua_line.remove_prefix(3);
					value = lua_line;
					parsed_ok = true;
				}
			}
		}
		return parsed_ok;
	}
	// Assumes that the "end configuration" case has already been ruled out.
	std::unique_ptr<Option> parse_option(std::string_view next_line, LineMuncher& lines) {
		std::vector<std::string> parsed;
		if(!parse_config_line(next_line, parsed, lines)) return nullptr;
		assert(parsed.size() > 0);
		if(parsed[0] == "spacer") {
			if(parsed.size() != 1) {
				parse_failure(lines, "\"spacer\" widget can't have any parameters.");
				return nullptr;
			}
			return std::make_unique<Spacer>(next_line);
		}
		else if(parsed[0] == "text") {
			if(parsed.size() != 2) {
				parse_failure(lines, "\"text\" widget requires one parameter.");
				return nullptr;
			}
			return std::make_unique<Text>(next_line, parsed[1]);
		}
		else if(parsed[0] == "toggle") {
			if(parsed.size() != 3) {
				parse_failure(lines, "\"toggle\" widget requires two parameters.");
				return nullptr;
			}
			std::string_view lua_line;
			if(!lines.yield_line(lua_line)) {
				unexpected_eof(lines);
				return nullptr;
			}
			std::string_view value;
			if(!check_lua_line(lua_line, parsed[1], value) || (value != "true" && value != "false")) {
				std::ostringstream out;
				out << "This line didn't match the widget specification. "
					"In order to be valid, the line must be exactly: "
					"`local " << parsed[1] << " = VALUE` "
					"where VALUE is either `true` or `false`.";
				parse_failure(lines, out.str());
				return nullptr;
			}
			return std::make_unique<Toggle>(next_line, parsed[1], parsed[2], value == "true");
		}
		else if(parsed[0] == "select") {
			if(parsed.size() < 5) {
				parse_failure(lines, "\"select\" widget requires at least five parameters.");
				return nullptr;
			}
			std::string_view lua_line;
			if(!lines.yield_line(lua_line)) {
				unexpected_eof(lines);
				return nullptr;
			}
			std::string_view value;
			if(check_lua_line(lua_line, parsed[1], value)) {
				char* endp;
				unsigned long selection = strtoul(&value[0], &endp, 10);
				if(endp == &value[value.size()] && selection > 0 && selection <= parsed.size() - 3) {
					std::string variable_name = parsed[1];
					std::string label = parsed[2];
					parsed.erase(parsed.begin());
					parsed.erase(parsed.begin());
					parsed.erase(parsed.begin());
					return std::make_unique<Select>(next_line, variable_name, label, parsed, selection - 1);
				}
			}
			std::ostringstream out;
			out << "This line didn't match the widget specification. "
				"In order to be valid, the line must be exactly: "
				"`local " << parsed[1] << " = VALUE` "
				"where VALUE is a number between 1 and " << parsed.size() - 3 << " (the number of values).";
			parse_failure(lines, out.str());
			return nullptr;
		}
		else {
			parse_failure(lines, "Unknown widget type. Known widget types: spacer, text, toggle, select");
			return nullptr;
		}
	}
	bool with_configurable_script(w_file_chooser* chooser, std::function<void(FileSpecifier&, LineMuncher&, std::string_view&, std::string_view&)> callback) {
		auto file = chooser->get_file();
		if (file.GetPath()[0] && file.Exists()) {
			OpenedFile script_file;
			if (file.Open(script_file)) {
				int32 script_length;
				script_file.GetLength(script_length);
				std::vector<char> script_buffer(script_length);
				if (script_file.Read(script_length, &script_buffer[0])) {
					script_file.Close();
					std::string_view orig_script(&script_buffer[0], script_buffer.size());
					LineMuncher lines(orig_script);
					std::string_view first_line;
					if(lines.yield_line(first_line)) {
						// string_view.starts_with isn't in C++17
						if(starts_with(first_line, "--! configuration ")) {
							callback(file, lines, first_line, orig_script);
							return true;
						}
					}
				}
			}
		}
		return false;
	}
	void real_config_dialog(FileSpecifier& fs, LineMuncher& lines, std::string_view& first_line, std::string_view& orig_script) {
		if(first_line != "--! configuration 1.0"
		   && !starts_with(first_line, "--! configuration 1.0 ")) {
			alert_user(expand_app_variables("This script requires a newer version of $appName$ for configuration. Upgrade to a newer version of $appName$, or configure the script by editing it manually.").c_str());
			return;
		}

		std::vector<std::unique_ptr<Option>> options;
		while(true) {
			std::string_view next_line;
			if(!lines.yield_line(next_line)) {
				unexpected_eof(lines);
				return;
			}
			if(next_line == "--! end configuration") break;
			auto neu = parse_option(next_line, lines);
			if(!neu) return;
			options.emplace_back(std::move(neu));
		}
		if(options.empty()) {
			parse_failure(lines, "This script doesn't actually contain any options.");
			return;
		}

		dialog d;
		vertical_placer *placer = new vertical_placer;
		w_title *w_header = new w_title("SCRIPT OPTIONS");
		placer->dual_add(w_header, d);
		placer->add(new w_spacer, true);

		table_placer *table = new table_placer(2, get_theme_space(ITEM_WIDGET), true);
		table->col_flags(0, placeable::kAlignRight);
		table->col_flags(1, placeable::kAlignLeft);

		for(auto& option : options) {
			option->add_widget(table, d);
		}

		placer->add(table, true);

		placer->add(new w_spacer, true);
		horizontal_placer* button_placer = new horizontal_placer;
		w_button* accept_w = new w_button("ACCEPT", dialog_ok, &d);
		button_placer->dual_add(accept_w, d);
		w_button* cancel_w = new w_button("CANCEL", dialog_cancel, &d);
		button_placer->dual_add(cancel_w, d);

		placer->add(button_placer, true);

		d.set_widget_placer(placer);

		if (d.run() == 0) {
			std::ostringstream new_script;
			new_script << first_line << "\n";
			for(auto& option : options) {
				option->reoutput(new_script);
			}
			new_script << "--! end configuration\n" << lines.get_rest();
			std::string new_script_string = new_script.str();
			if(new_script_string != orig_script) {
				FileSpecifier temp;
				temp.SetTempName(fs);
				OpenedFile outfile;
				if(!temp.Open(outfile, true) || !outfile.Write(new_script_string.size(), new_script_string.data())) {
					alert_user("Unable to save changed settings.");
					return;
				}
				outfile.Close();
				fs.Delete(); // might be symlinked :| -SB
				if(!temp.Rename(fs)) {
					alert_user("Unable to save changed settings.");
					return;
				}
			}
		}
	}
}

bool script_is_configurable(w_file_chooser* chooser) {
	return with_configurable_script(chooser, [](FileSpecifier&, LineMuncher&, std::string_view&, std::string_view&) {});
}

void script_configuration_dialog(void* param) {
	w_file_chooser* chooser = reinterpret_cast<w_file_chooser*>(param);
	if(!with_configurable_script(chooser, real_config_dialog)) {
		alert_user("The selected script does not contain any configuration options.");
	}
}

#endif
