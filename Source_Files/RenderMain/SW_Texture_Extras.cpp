/*
SW_TEXTURE_EXTRAS.CPP

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

#include "SW_Texture_Extras.h"
#include "collection_definition.h"
#include "interface.h"
#include "render.h"
#include "scottish_textures.h"
#include "InfoTree.h"
#include "screen.h"

extern short bit_depth;
void SW_Texture::build_opac_table()
{
	bitmap_definition *bitmap;
	void *shading_tables;
	get_shape_bitmap_and_shading_table(m_shape_descriptor, &bitmap, &shading_tables, 0);

	m_opac_table.resize(MAXIMUM_SHADING_TABLE_INDEXES);

	SDL_PixelFormat *fmt = MainScreenSurface()->format;
	if (bit_depth == 32)
	{
		for (int i = 0; i < MAXIMUM_SHADING_TABLE_INDEXES; i++)
		{
			float Opacity;
			if (m_opac_type == 1)
			{
				Opacity = 255.0;
			}
			else
			{
				uint32 color = ((uint32 *) shading_tables)[i + MAXIMUM_SHADING_TABLE_INDEXES * (number_of_shading_tables - 1)];
				uint32 r = (((color&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
				uint32 g = (((color&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
				uint32 b = (((color&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);
				if (m_opac_type == 2)
				{
					Opacity = (float) (r + g + b) / 3.0F;
					
				}
				else 
				{
					Opacity = (float)std::max(r, std::max(g, b));
				}
			}

			m_opac_table[i] = PIN(int32(m_opac_scale * Opacity + 255 * m_opac_shift + 0.5), 0, 255);
		}
	} 
	else if (bit_depth == 16)
	{
		for (int i = 0; i < MAXIMUM_SHADING_TABLE_INDEXES; i++)
		{
			float Opacity;
			if (m_opac_type == 1)
			{
				Opacity = 255.0;
			}
			else 
			{
				uint16 color = ((uint16 *) shading_tables)[i + MAXIMUM_SHADING_TABLE_INDEXES * (number_of_shading_tables - 1)];
				uint32 r = (((color&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
				uint32 g = (((color&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
				uint32 b = (((color&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);		
				if (m_opac_type == 2)
				{
					Opacity = (float) (r + g + b) / 3.0F;
				}
				else
				{
					Opacity = (float)std::max(r, std::max(g, b));
				}
			}
			m_opac_table[i] = PIN(int32(m_opac_scale * Opacity + 255 * m_opac_shift + 0.5), 0, 255);
		}	
	}
}

SW_Texture *SW_Texture_Extras::AddTexture(shape_descriptor ShapeDesc)
{
	short Collection = GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(ShapeDesc));
	short Bitmap = GET_DESCRIPTOR_SHAPE(ShapeDesc);

	if (Bitmap >= texture_list[Collection].size())
	{
		texture_list[Collection].resize(Bitmap + 1);
	}
	return &texture_list[Collection][Bitmap];
}

SW_Texture *SW_Texture_Extras::GetTexture(shape_descriptor ShapeDesc)
{
	short Collection = GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(ShapeDesc));
	short Bitmap = GET_DESCRIPTOR_SHAPE(ShapeDesc);

	if (Bitmap >= texture_list[Collection].size())
		return 0;
	return &texture_list[Collection][Bitmap];
}

void SW_Texture_Extras::Load(short collection_index)
{
	for (std::vector<SW_Texture>::iterator i = texture_list[collection_index].begin(); i != texture_list[collection_index].end(); i++)
	{
		i->build_opac_table();
	}
}

void SW_Texture_Extras::Unload(short collection_index)
{
	// unload all opacity tables
	for (std::vector<SW_Texture>::iterator i = texture_list[collection_index].begin(); i != texture_list[collection_index].end(); i++)
	{
		i->clear_opac_table();
	}
}

void SW_Texture_Extras::Reset()
{
	// remove all textures
	for (short i = 0; i < NUMBER_OF_COLLECTIONS; i++) {
		texture_list[i].clear();
	}
}


void reset_mml_software()
{
	SW_Texture_Extras::instance()->Reset();
}

void parse_mml_software(const InfoTree& root)
{
	BOOST_FOREACH(InfoTree ttree, root.children_named("texture"))
	{
		int16 coll, bitmap;
		if (!ttree.read_indexed("coll", coll, NUMBER_OF_COLLECTIONS) ||
			!ttree.read_indexed("bitmap", bitmap, MAXIMUM_SHAPES_PER_COLLECTION))
			continue;
		
		SW_Texture *tex = SW_Texture_Extras::instance()->AddTexture(BUILD_DESCRIPTOR(coll, bitmap));
		tex->descriptor(BUILD_DESCRIPTOR(coll, bitmap));
		
		int16 opac_type = 0;
		ttree.read_indexed("opac_type", opac_type, 4);
		tex->opac_type(opac_type);
		
		float opac_scale = 1.0;
		ttree.read_attr("opac_scale", opac_scale);
		tex->opac_scale(opac_scale);
		
		float opac_shift = 0.0;
		ttree.read_attr("opac_shift", opac_shift);
		tex->opac_shift(opac_shift);
	}
}

