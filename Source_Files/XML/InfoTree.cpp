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
#include "cseries.h"
#include "shell.h"
#include "TextStrings.h"

#include <type_traits>

#include <boost/function.hpp>
#include <boost/version.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/iostreams/stream.hpp>

namespace pt = boost::property_tree;
namespace io = boost::iostreams;

class InfoTreeFileStream : public io::stream<opened_file_device>
{ 
private:
	OpenedFile f;
public:
	~InfoTreeFileStream() { close(); }
	explicit InfoTreeFileStream(FileSpecifier path, bool write = false)
	{ 
		if (!(write ? path.OpenForWritingText(f) : path.Open(f)))
		{ 
			const auto code = std::to_string(path.GetError());
			const auto mode = write ? "writing" : "reading";
			const auto msg = std::string("couldn't open '") + path.GetPath() + "' for " + mode + " (error " + code + ")";
			throw InfoTree::unexpected_error(msg);
		} 
		open(f);
	}
};

InfoTree InfoTree::load_xml(FileSpecifier filename)
{
	InfoTreeFileStream stream(filename);
	InfoTree xtree;
	pt::read_xml<pt::ptree>(stream, xtree);
	return xtree;
}

InfoTree InfoTree::load_xml(std::istringstream& stream)
{
	InfoTree xtree;
	pt::read_xml<pt::ptree>(stream, xtree);
	return xtree;
}

static void write_indented_xml(std::ostream& dest, const pt::ptree& src)
{
	using settings_t = pt::xml_writer_settings< std::conditional<BOOST_VERSION >= 105600, std::string, char>::type >;
	pt::write_xml<pt::ptree>(dest, src, settings_t(' ', 2));
} 

void InfoTree::save_xml(FileSpecifier filename) const
{
	InfoTreeFileStream stream(filename, /*write:*/ true);
	write_indented_xml(stream, *this);
}

void InfoTree::save_xml(std::ostringstream& stream) const
{
	write_indented_xml(stream, *this);
}

InfoTree InfoTree::load_ini(FileSpecifier filename)
{
	InfoTreeFileStream stream(filename);
	InfoTree itree;
	pt::read_ini<pt::ptree>(stream, itree);
	return itree;
}

InfoTree InfoTree::load_ini(std::istringstream& stream)
{
	InfoTree itree;
	pt::read_ini<pt::ptree>(stream, itree);
	return itree;
}

void InfoTree::save_ini(FileSpecifier filename) const
{
	InfoTreeFileStream stream(filename, /*write:*/ true);
	pt::write_ini<pt::ptree>(stream, *this);
}

void InfoTree::save_ini(std::ostringstream& stream) const
{
	pt::write_ini<pt::ptree>(stream, *this);
}

bool InfoTree::read_fixed(std::string path, _fixed& value, float min, float max) const
{
	float temp;
	if (read_attr_bounded(path, temp, min, max))
	{
		value = FIXED_ONE * temp + 0.5;
		return true;
	}
	return false;
}

bool InfoTree::read_wu(std::string path, short& value, float min, float max) const
{
	float temp;
	if (read_attr_bounded(path, temp, min, max))
	{
		value = WORLD_ONE * temp + 0.5;
		return true;
	}
	return false;
}

bool InfoTree::read_angle(std::string path, angle& value) const
{
	float temp;
	if (read_attr(path, temp))
	{
		temp = temp - 360*static_cast<int>(temp/360);
		while (temp < 0)
			temp += 360;
		while (temp >= 360)
			temp -= 360;
		value = FULL_CIRCLE*(temp/360) + 0.5;
		return true;
	}
	return false;
}

bool InfoTree::read_path(std::string key, FileSpecifier& file) const
{
	std::string path;
	if (read_attr(key, path))
	{
		file.SetNameWithPath(path.c_str());
		return true;
	}
	return false;
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

bool InfoTree::read_shape(shape_descriptor& descriptor, bool allow_empty) const
{
	uint16 seq = UNONE;
	bool seq_present = read_attr_bounded<uint16>("seq", seq, 0, MAXIMUM_SHAPES_PER_COLLECTION-1);
	if (!seq_present)
		seq_present = read_attr_bounded<uint16>("frame", seq, 0, MAXIMUM_SHAPES_PER_COLLECTION-1);
	
	uint16 coll = UNONE;
	bool coll_present = read_attr_bounded<uint16>("coll", coll, 0, MAXIMUM_COLLECTIONS-1);
	
	uint16 clut = 0;
	read_attr_bounded<uint16>("clut", clut, 0, MAXIMUM_CLUTS_PER_COLLECTION-1);
	
	if (coll_present && seq_present)
	{
		descriptor = BUILD_DESCRIPTOR(BUILD_COLLECTION(coll, clut), seq);
		return true;
	}
	else if (!coll_present && !seq_present && allow_empty)
	{
		descriptor = UNONE;
		return true;
	}
	return false;
}

bool InfoTree::read_damage(damage_definition& def) const
{
	bool status = false;
	if (read_indexed("type", def.type, NUMBER_OF_DAMAGE_TYPES, true))
		status = true;
	if (read_indexed("flags", def.flags, 2))
		status = true;
	if (read_attr("base", def.base))
		status = true;
	if (read_attr("random", def.random))
		status = true;
	if (read_fixed("scale", def.scale))
		status = true;
	return status;
}

bool InfoTree::read_font(FontSpecifier& font) const
{
	bool status = false;
	if (read_attr("size", font.Size))
		status = true;
	if (read_attr("style", font.Style))
		status = true;
	if (read_attr("file", font.File))
		status = true;
	if (status)
		font.Update();
	return status;
}


typedef boost::iterator_range<InfoTree::const_assoc_iterator> _match_range_type;

InfoTree::const_child_range InfoTree::children_named(std::string key) const
{
	std::pair<const_assoc_iterator, const_assoc_iterator> matches = equal_range(key);
	_match_range_type match_range = boost::make_iterator_range(matches.first, matches.second);
	return boost::adaptors::values(static_cast<const _match_range_type&>(match_range));
}
