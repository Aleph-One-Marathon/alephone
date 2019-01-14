#ifndef __SW_TEXTURE_EXTRAS_H
#define __SW_TEXTURE_EXTRAS_H

/*
SW_TEXTURE_EXTRAS.H

	Copyright (C) 2007 Gregory Smith
 
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
#include "cstypes.h"
#include "shape_descriptors.h"
#include <vector>

class SW_Texture
{
public:
	SW_Texture() : m_opac_type(0), m_shape_descriptor(0), m_opac_scale(1.0), m_opac_shift(0.0) { }
	void descriptor(shape_descriptor ShapeDesc) { m_shape_descriptor = ShapeDesc; }
	int opac_type() { return m_opac_type; }
	void opac_type(int new_opac_type) { m_opac_type = new_opac_type; }

	uint8 *opac_table() { 
		if (m_opac_table.size() == 0)
			return 0; 
		else 
			return &m_opac_table.front(); 
	}

	void opac_shift(float new_opac_shift) { m_opac_shift = new_opac_shift; }
	void opac_scale(float new_opac_scale) { m_opac_scale = new_opac_scale; }

	void build_opac_table();

	void clear_opac_table() {
		m_opac_table.clear();
	}
	
private:
	shape_descriptor m_shape_descriptor;
	int m_opac_type;
	float m_opac_scale;
	float m_opac_shift;
	std::vector<uint8> m_opac_table;
};

class SW_Texture_Extras
{
public:
	static SW_Texture_Extras *instance() { 
		static SW_Texture_Extras *m_instance = nullptr;
		if (!m_instance) 
			m_instance = new SW_Texture_Extras(); 
		return m_instance; 
	}

	SW_Texture *GetTexture(shape_descriptor ShapeDesc);
	SW_Texture *AddTexture(shape_descriptor ShapeDesc);
	void Load(short Collection);
	void Unload(short Collection);
	void Reset();

private:
	SW_Texture_Extras() { }

	std::vector<SW_Texture> texture_list[NUMBER_OF_COLLECTIONS];
};

class InfoTree;
void parse_mml_software(const InfoTree& root);
void reset_mml_software();

#endif
