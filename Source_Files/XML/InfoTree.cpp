/*

	Copyright (C) 2015 and beyond by Jeremiah Morris
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
	
	Boost.PropertyTree-based structured-data reader and writer
*/

#include "InfoTree.h"
#include "shell.h"
#include "XML_ElementParser.h"  // for DeUTF8_C

InfoTree InfoTree::load_xml(FileSpecifier filename)
{
	boost::property_tree::ptree xtree;
	boost::property_tree::read_xml(filename.GetPath(), xtree);
	return InfoTree(xtree);
}

InfoTree InfoTree::load_xml(std::istringstream& stream)
{
	boost::property_tree::ptree xtree;
	boost::property_tree::read_xml(stream, xtree);
	return InfoTree(xtree);
}

void InfoTree::save_xml(FileSpecifier filename) const
{
	boost::property_tree::write_xml(filename.GetPath(),
									boost::property_tree::ptree(*this),
									std::locale(),
									boost::property_tree::xml_writer_make_settings(' ', 2));
}

void InfoTree::save_xml(std::ostringstream& stream) const
{
	boost::property_tree::write_xml(stream,
									boost::property_tree::ptree(*this),
									boost::property_tree::xml_writer_make_settings(' ', 2));
}

InfoTree InfoTree::load_ini(FileSpecifier filename)
{
	boost::property_tree::ptree itree;
	boost::property_tree::read_ini(filename.GetPath(), itree);
	return InfoTree(itree);
}

InfoTree InfoTree::load_ini(std::istringstream& stream)
{
	boost::property_tree::ptree itree;
	boost::property_tree::read_ini(stream, itree);
	return InfoTree(itree);
}

void InfoTree::save_ini(FileSpecifier filename) const
{
	boost::property_tree::write_ini(filename.GetPath(),
									boost::property_tree::ptree(*this));
}

void InfoTree::save_ini(std::ostringstream& stream) const
{
	boost::property_tree::write_ini(stream,
									boost::property_tree::ptree(*this));
}

bool InfoTree::read_path(std::string key, char *dest) const
{
	std::string path;
	if (read_attr(key, path))
	{
		expand_symbolic_paths(dest, path.c_str(), 255);
		return true;
	}
	return false;
}

void InfoTree::put_attr_path(std::string key, std::string filepath)
{
	char tempstr[256];
	contract_symbolic_paths(tempstr, filepath.c_str(), 255);
	put_attr(key, tempstr);
}

bool InfoTree::read_cstr(std::string key, char *dest, int maxlen) const
{
	std::string str;
	if (read_attr(key, str))
	{
		DeUTF8_C(str.c_str(), str.length(), dest, maxlen);
		return true;
	}
	return false;
}

void InfoTree::put_cstr(std::string key, std::string cstr)
{
	put(key, mac_roman_to_utf8(cstr));
}

void InfoTree::put_attr_cstr(std::string key, std::string cstr)
{
	put_attr(key, mac_roman_to_utf8(cstr));
}



static const float ColorToTree = 1 / static_cast<float>(65535);
static const float TreeToColor = static_cast<float>(65535);

static bool _get_color_part(const InfoTree *tree, std::string key, uint16& part)
{
	float value;
	if (tree->read_attr(key, value))
	{
		part = value * TreeToColor;
		return true;
	}
	return false;
}

template<typename T> bool _get_color(const InfoTree *tree, T& color)
{
	bool found_r = _get_color_part(tree, "red", color.red);
	bool found_g = _get_color_part(tree, "green", color.green);
	bool found_b = _get_color_part(tree, "blue", color.blue);
	return found_r || found_g || found_b;
}

static void _set_color_part(InfoTree& tree, std::string key, uint16 part)
{
	tree.put(std::string("<xmlattr>.") + key, part * ColorToTree);
}

template<typename T> InfoTree _make_color(const T& color)
{
	InfoTree ctree;
	_set_color_part(ctree, "red", color.red);
	_set_color_part(ctree, "green", color.green);
	_set_color_part(ctree, "blue", color.blue);
	return ctree;
}
template<typename T> InfoTree _make_color(const T& color, size_t index)
{
	InfoTree ctree;
	ctree.put("<xmlattr>.index", index);
	_set_color_part(ctree, "red", color.red);
	_set_color_part(ctree, "green", color.green);
	_set_color_part(ctree, "blue", color.blue);
	return ctree;
}


bool InfoTree::read_color(RGBColor& color) const
{
	return _get_color(this, color);
}

bool InfoTree::read_color(rgb_color& color) const
{
	return _get_color(this, color);
}

void InfoTree::add_color(std::string path, const RGBColor& color)
{
	add_child(path, _make_color(color));
}
void InfoTree::add_color(std::string path, const RGBColor& color, size_t index)
{
	add_child(path, _make_color(color, index));
}
void InfoTree::add_color(std::string path, const rgb_color& color)
{
	add_child(path, _make_color(color));
}
void InfoTree::add_color(std::string path, const rgb_color& color, size_t index)
{
	add_child(path, _make_color(color, index));
}
