/*
LOW_LEVEL_TEXTURES.C

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
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

Friday, August 19, 1994 2:05:54 PM

Monday, February 27, 1995 11:40:47 PM  (Jason')
	rob suggests that the PPC might not write-allocate cache lines so we might be faster if we
	read from a location weÕre about to write to.  he also suggested a rowbytes of 704 instead
	of 640 off-screen for better cache performance.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"
*/

/*
we donÕt include anything here because we are included in SCOTTISH_TEXTURES.C, with
BIT_DEPTH==8 and BIT_DEPTH==16
*/

#undef PEL
#undef TEXTURE_HORIZONTAL_POLYGON_LINES
#undef TEXTURE_VERTICAL_POLYGON_LINES
#undef TRANSPARENT_TEXTURE_VERTICAL_POLYGON_LINES
#undef RANDOMIZE_VERTICAL_POLYGON_LINES
#undef TINT_VERTICAL_POLYGON_LINES
#undef LANDSCAPE_HORIZONTAL_POLYGON_LINES

#if BIT_DEPTH==32
#define PEL pixel32
#define TEXTURE_HORIZONTAL_POLYGON_LINES _texture_horizontal_polygon_lines32
#define TEXTURE_VERTICAL_POLYGON_LINES _texture_vertical_polygon_lines32
#define TRANSPARENT_TEXTURE_VERTICAL_POLYGON_LINES _transparent_texture_vertical_polygon_lines32
#define RANDOMIZE_VERTICAL_POLYGON_LINES _randomize_vertical_polygon_lines32
#define TINT_VERTICAL_POLYGON_LINES _tint_vertical_polygon_lines32
#define LANDSCAPE_HORIZONTAL_POLYGON_LINES _landscape_horizontal_polygon_lines32
#endif

#if BIT_DEPTH==16
#define PEL pixel16
#define TEXTURE_HORIZONTAL_POLYGON_LINES _texture_horizontal_polygon_lines16
#define TEXTURE_VERTICAL_POLYGON_LINES _texture_vertical_polygon_lines16
#define TRANSPARENT_TEXTURE_VERTICAL_POLYGON_LINES _transparent_texture_vertical_polygon_lines16
#define RANDOMIZE_VERTICAL_POLYGON_LINES _randomize_vertical_polygon_lines16
#define TINT_VERTICAL_POLYGON_LINES _tint_vertical_polygon_lines16
#define LANDSCAPE_HORIZONTAL_POLYGON_LINES _landscape_horizontal_polygon_lines16
#endif

#if BIT_DEPTH==8
#define PEL pixel8
#define TEXTURE_HORIZONTAL_POLYGON_LINES _texture_horizontal_polygon_lines8
#define TEXTURE_VERTICAL_POLYGON_LINES _texture_vertical_polygon_lines8
#define TRANSPARENT_TEXTURE_VERTICAL_POLYGON_LINES _transparent_texture_vertical_polygon_lines8
#define RANDOMIZE_VERTICAL_POLYGON_LINES _randomize_vertical_polygon_lines8
#define TINT_VERTICAL_POLYGON_LINES _tint_vertical_polygon_lines8
#define LANDSCAPE_HORIZONTAL_POLYGON_LINES _landscape_horizontal_polygon_lines8
#endif

#if !defined(EXTERNAL) || BIT_DEPTH==32
void TEXTURE_HORIZONTAL_POLYGON_LINES(
	struct bitmap_definition *texture,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short *x0_table,
	short *x1_table,
	short line_count)
{
	(void) (view);

	while ((line_count-= 1)>=0)
	{
		short x0= *x0_table++, x1= *x1_table++;
		
		// LP: changed "unsigned char *" to "PEL *"
		register PEL *shading_table= (PEL *)data->shading_table;
		register PEL *write= (PEL *) screen->row_addresses[y0] + x0;
		register pixel8 *base_address= texture->row_addresses[0];
		register uint32 source_x= data->source_x;
		register uint32 source_y= data->source_y;
		register uint32 source_dx= data->source_dx;
		register uint32 source_dy= data->source_dy;
		register short count= x1-x0;
		
		while ((count-= 1)>=0)
		{
			*write++= shading_table[base_address[((source_y>>(HORIZONTAL_HEIGHT_DOWNSHIFT-7))&(0x7f<<7))+(source_x>>HORIZONTAL_WIDTH_DOWNSHIFT)]];
			source_x+= source_dx, source_y+= source_dy;
		}
		
		data+= 1;
		y0+= 1;
	}
}
#endif

#define LANDSCAPE_WIDTH_BITS 9
#define LANDSCAPE_TEXTURE_WIDTH_DOWNSHIFT (32-LANDSCAPE_WIDTH_BITS)
void LANDSCAPE_HORIZONTAL_POLYGON_LINES(
	struct bitmap_definition *texture,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short *x0_table,
	short *x1_table,
	short line_count)
{
	register short landscape_texture_width_downshift= 32 - NextLowerExponent(texture->height);

	(void) (view);

	while ((line_count-= 1)>=0)
	{
		short x0= *x0_table++, x1= *x1_table++;
		
		register PEL *shading_table= (PEL *)data->shading_table;
		register PEL *write= (PEL *)screen->row_addresses[y0] + x0;
		register pixel8 *read= texture->row_addresses[data->source_y];
		register uint32 source_x= data->source_x;
		register uint32 source_dx= data->source_dx;
		register short count= x1-x0;
		
		while ((count-= 1)>=0)
		{
			*write++= shading_table[read[source_x>>landscape_texture_width_downshift]];
			source_x+= source_dx;
		}
		
		data+= 1;
		y0+= 1;
	}
}

#if !defined(EXTERNAL) || BIT_DEPTH==32
// LP: "static" removed
void TEXTURE_VERTICAL_POLYGON_LINES(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table)
{
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	int bytes_per_row= screen->bytes_per_row;
	int downshift= data->downshift;
	int line_count= data->width;
	bool aborted= false;
	int x= data->x0;
	int count;
	
	(void) (view);

	while (line_count>0)	
	{
		if (line_count<4 || (x&3) || aborted)
		{
			int y0= *y0_table++, y1= *y1_table++;
			uint32 texture_y= line->texture_y;
			uint32 texture_dy= line->texture_dy;
			PEL *write, *shading_table;
			pixel8 *read;

			shading_table= (PEL *)line->shading_table;
			read= line->texture;
			write= (PEL *)screen->row_addresses[y0] + x;

			for (count= y1-y0; count>0; --count)
			{
				*write= shading_table[read[texture_y>>downshift]], write = (PEL *)((byte *)write + bytes_per_row);
				texture_y+= texture_dy;
			}
			
			x+= 1;
			line+= 1;
			line_count-= 1;

			aborted= false;
		}
		else
		{
			uint32 texture_y0= line[0].texture_y, texture_dy0= line[0].texture_dy;
			pixel8 *read0= line[0].texture;
			PEL *shading_table0= (PEL *)line[0].shading_table;
			
			uint32 texture_y1= line[1].texture_y, texture_dy1= line[1].texture_dy;
			pixel8 *read1= line[1].texture;
			PEL *shading_table1= (PEL *)line[1].shading_table;
			
			uint32 texture_y2= line[2].texture_y, texture_dy2= line[2].texture_dy;
			pixel8 *read2= line[2].texture;
			PEL *shading_table2= (PEL *)line[2].shading_table;
			
			uint32 texture_y3= line[3].texture_y, texture_dy3= line[3].texture_dy;
			pixel8 *read3= line[3].texture;
			PEL *shading_table3= (PEL *)line[3].shading_table;
			
			PEL *write;

			int ymax;
			
			/* sync */	
			{
				int y0= y0_table[0], y1= y0_table[1], y2= y0_table[2], y3= y0_table[3];
				PEL *temp_write;
				
				ymax= MAX(y0, y1), ymax= MAX(ymax, y2), ymax= MAX(ymax, y3);
				write= (PEL *)screen->row_addresses[ymax] + x;
				
				{
					int ymin= MIN(y1_table[0], y1_table[1]);
					
					ymin= MIN(ymin, y1_table[2]);
					ymin= MIN(ymin, y1_table[3]);
					
					if (ymin<=ymax)
					{
						aborted= true;
						continue;
					}
				}

				for (count= ymax-y0, temp_write= (PEL *)screen->row_addresses[y0] + x; count>0; --count)
				{
					temp_write[0]= shading_table0[read0[texture_y0>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= ymax-y1, temp_write= (PEL *)screen->row_addresses[y1] + x; count>0; --count)
				{
					temp_write[1]= shading_table1[read1[texture_y1>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= ymax-y2, temp_write= (PEL *)screen->row_addresses[y2] + x; count>0; --count)
				{
					temp_write[2]= shading_table2[read2[texture_y2>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= ymax-y3, temp_write= (PEL *)screen->row_addresses[y3] + x; count>0; --count)
				{
					temp_write[3]= shading_table3[read3[texture_y3>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y3+= texture_dy3;
				}
			}

			/* parallel map (x4) */
			{
				int dy0= y1_table[0] - ymax;
				int dy1= y1_table[1] - ymax;
				int dy2= y1_table[2] - ymax;
				int dy3= y1_table[3] - ymax;
				
				count= MIN(dy0, dy1), count= MIN(count, dy2), count= MIN(count, dy3);
				ymax+= count;
				
				for (; count>0; --count)
				{
					write[0]= shading_table0[read0[texture_y0>>downshift]];
					texture_y0+= texture_dy0;
		
					write[1]= shading_table1[read1[texture_y1>>downshift]];
					texture_y1+= texture_dy1;
		
					write[2]= shading_table2[read2[texture_y2>>downshift]];
					texture_y2+= texture_dy2;
		
					write[3]= shading_table3[read3[texture_y3>>downshift]];
					texture_y3+= texture_dy3;
					
					write = (PEL *)((byte *)write + bytes_per_row);
				}
			}

			/* desync */	
			{
				PEL *temp_write;
				
				for (count= y1_table[0] - ymax, temp_write= write; count>0; --count)
				{
					temp_write[0]= shading_table0[read0[texture_y0>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= y1_table[1] - ymax, temp_write= write; count>0; --count)
				{
					temp_write[1]= shading_table1[read1[texture_y1>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= y1_table[2] - ymax, temp_write= write; count>0; --count)
				{
					temp_write[2]= shading_table2[read2[texture_y2>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= y1_table[3] - ymax, temp_write= write; count>0; --count)
				{
					temp_write[3]= shading_table3[read3[texture_y3>>downshift]], temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y3+= texture_dy3;
				}
			}

			y0_table+= 4, y1_table+= 4;
			line_count-= 4;
			line+= 4;
			x+= 4;
		}
	}
}
#endif

#if !defined(EXTERNAL) || BIT_DEPTH==32
// LP: "static" removed
void TRANSPARENT_TEXTURE_VERTICAL_POLYGON_LINES(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table)
{
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	int bytes_per_row= screen->bytes_per_row;
	int downshift= data->downshift;
	int line_count= data->width;
	bool aborted= false;
	int x= data->x0;
	pixel8 pixel;
	int count;
	
	(void) (view);

	while (line_count>0)	
	{
		if (line_count<4 || (x&3) || aborted)
		{
			int y0= *y0_table++, y1= *y1_table++;
			uint32 texture_y= line->texture_y;
			uint32 texture_dy= line->texture_dy;
			PEL *write, *shading_table;
			pixel8 *read;

			shading_table= (PEL *)line->shading_table;
			read= line->texture;
			write= (PEL *)screen->row_addresses[y0] + x;

			for (count= y1-y0; count>0; --count)
			{
				if ((pixel= read[texture_y>>downshift])!=0)
					*write= shading_table[pixel];
				write = (PEL *)((byte *)write + bytes_per_row);
				texture_y+= texture_dy;
			}
			
			x+= 1;
			line+= 1;
			line_count-= 1;
			
			aborted= false;
		}
		else
		{
			uint32 texture_y0= line[0].texture_y, texture_dy0= line[0].texture_dy;
			pixel8 *read0= line[0].texture;
			PEL *shading_table0= (PEL *)line[0].shading_table;
			
			uint32 texture_y1= line[1].texture_y, texture_dy1= line[1].texture_dy;
			pixel8 *read1= line[1].texture;
			PEL *shading_table1= (PEL *)line[1].shading_table;
			
			uint32 texture_y2= line[2].texture_y, texture_dy2= line[2].texture_dy;
			pixel8 *read2= line[2].texture;
			PEL *shading_table2= (PEL *)line[2].shading_table;
			
			uint32 texture_y3= line[3].texture_y, texture_dy3= line[3].texture_dy;
			pixel8 *read3= line[3].texture;
			PEL *shading_table3= (PEL *)line[3].shading_table;
			
			PEL *write;

			int ymax;
			
			/* sync */	
			{
				int y0= y0_table[0], y1= y0_table[1], y2= y0_table[2], y3= y0_table[3];
				PEL *temp_write;
				
				ymax= MAX(y0, y1), ymax= MAX(ymax, y2), ymax= MAX(ymax, y3);
				write= (PEL *)screen->row_addresses[ymax] + x;

				{
					int ymin= MIN(y1_table[0], y1_table[1]);
					
					ymin= MIN(ymin, y1_table[2]);
					ymin= MIN(ymin, y1_table[3]);
					
					if (ymin<=ymax)
					{
						aborted= true;
						continue;
					}
				}
				
				for (count= ymax-y0, temp_write= (PEL *)screen->row_addresses[y0] + x; count>0; --count)
				{
					if ((pixel= read0[texture_y0>>downshift])!=0)
						temp_write[0]= shading_table0[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= ymax-y1, temp_write= (PEL *)screen->row_addresses[y1] + x; count>0; --count)
				{
					if ((pixel= read1[texture_y1>>downshift])!=0)
						temp_write[1]= shading_table1[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= ymax-y2, temp_write= (PEL *)screen->row_addresses[y2] + x; count>0; --count)
				{
					if ((pixel= read2[texture_y2>>downshift])!=0)
						temp_write[2]= shading_table2[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= ymax-y3, temp_write= (PEL *)screen->row_addresses[y3] + x; count>0; --count)
				{
					if ((pixel= read3[texture_y3>>downshift])!=0)
						temp_write[3]= shading_table3[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y3+= texture_dy3;
				}
			}

			/* parallel map (x4) */
			{
				int dy0= y1_table[0] - ymax;
				int dy1= y1_table[1] - ymax;
				int dy2= y1_table[2] - ymax;
				int dy3= y1_table[3] - ymax;
				
				count= MIN(dy0, dy1), count= MIN(count, dy2), count= MIN(count, dy3);
				ymax+= count;
				
				for (; count>0; --count)
				{
					if ((pixel= read0[texture_y0>>downshift])!=0)
						write[0]= shading_table0[pixel];
					texture_y0+= texture_dy0;
		
					if ((pixel= read1[texture_y1>>downshift])!=0)
						write[1]= shading_table1[pixel];
					texture_y1+= texture_dy1;
		
					if ((pixel= read2[texture_y2>>downshift])!=0)
						write[2]= shading_table2[pixel];
					texture_y2+= texture_dy2;
		
					if ((pixel= read3[texture_y3>>downshift])!=0)
						write[3]= shading_table3[pixel];
					texture_y3+= texture_dy3;
					
					write = (PEL *)((byte *)write + bytes_per_row);
				}
			}

			/* desync */	
			{
				PEL *temp_write;
				
				for (count= y1_table[0] - ymax, temp_write= write; count>0; --count)
				{
					if ((pixel= read0[texture_y0>>downshift])!=0)
						temp_write[0]= shading_table0[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= y1_table[1] - ymax, temp_write= write; count>0; --count)
				{
					if ((pixel= read1[texture_y1>>downshift])!=0)
						temp_write[1]= shading_table1[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= y1_table[2] - ymax, temp_write= write; count>0; --count)
				{
					if ((pixel= read2[texture_y2>>downshift])!=0)
						temp_write[2]= shading_table2[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= y1_table[3] - ymax, temp_write= write; count>0; --count)
				{
					if ((pixel= read3[texture_y3>>downshift])!=0)
						temp_write[3]= shading_table3[pixel];
					temp_write = (PEL *)((byte *)temp_write + bytes_per_row);
					texture_y3+= texture_dy3;
				}
			}

			y0_table+= 4, y1_table+= 4;
			line_count-= 4;
			line+= 4;
			x+= 4;
		}
	}
}
#endif

// LP: "static" removed
void TINT_VERTICAL_POLYGON_LINES(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table,
	uint16 transfer_data)
{
	short tint_table_index= transfer_data&0xff;
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	register short bytes_per_row= screen->bytes_per_row;
	int line_count= data->width;
	int x= data->x0;

#if BIT_DEPTH==8
	register PEL *tint_tables= (PEL *)line->shading_table + tint_table_index*sizeof(struct tint_table8);
#endif

#if BIT_DEPTH==16
	register struct tint_table16 *tint_tables= (struct tint_table16 *)line->shading_table + (tint_table_index<<1);
#endif

#if BIT_DEPTH==32
	register struct tint_table32 *tint_tables= (struct tint_table32 *)line->shading_table + (tint_table_index<<3);
#endif

	(void) (view);

#if defined(SDL) && BIT_DEPTH != 8
	extern SDL_Surface *world_pixels;
	SDL_PixelFormat *fmt = world_pixels->format;
#endif
	
	assert(tint_table_index>=0 && tint_table_index<number_of_shading_tables);

	while ((line_count-= 1)>=0)
	{
		short y0= *y0_table++, y1= *y1_table++;
		register PEL *write= (PEL *) screen->row_addresses[y0] + x;
		register pixel8 *read= line->texture;
		register _fixed texture_y= line->texture_y, texture_dy= line->texture_dy;
		register short count= y1-y0;

		while ((count-=1)>=0)
		{
			if (read[FIXED_INTEGERAL_PART(texture_y)])
			{
#if BIT_DEPTH==8			
				// In color index mode, this is easy
				*write= tint_tables[*write];
#else
				register PEL pixel= *write;

#ifdef SDL
				// Under SDL, the pixel format is not fixed, so we need to be more flexible
				uint8 r = (((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
				uint8 g = (((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
				uint8 b = (((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);
#if BIT_DEPTH==16
				*write = tint_tables->red[r >> 3] | tint_tables->green[g >> 3] | tint_tables->blue[b >> 3];
#elif BIT_DEPTH==32
				*write = tint_tables->red[r] | tint_tables->green[g] | tint_tables->blue[b];
#endif
#else
				// Under MacOS, there is only one 16 and 32 bit pixel format, so this is easier (and faster)
#if BIT_DEPTH==16
				*write= tint_tables->red[RED16(pixel)] | tint_tables->green[GREEN16(pixel)] |
					tint_tables->blue[BLUE16(pixel)];
#elif BIT_DEPTH==32
				*write= tint_tables->red[RED32(pixel)] | tint_tables->green[GREEN32(pixel)] |
					tint_tables->blue[BLUE32(pixel)];
#endif
#endif // def SDL
#endif // BIT_DEPTH==8
			}

			write = (PEL *)((byte *)write + bytes_per_row);
			texture_y+= texture_dy;
		}

		line+= 1;
		x+= 1;
	}
}

// LP: "static" removed
void RANDOMIZE_VERTICAL_POLYGON_LINES(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table,
	uint16 transfer_data)
{
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	register short bytes_per_row= screen->bytes_per_row;
	int line_count= data->width;
	int x= data->x0;
	register uint16 seed= texture_random_seed;
	register uint16 drop_less_than= transfer_data;

	(void) (view);

	while ((line_count-= 1)>=0)
	{
		short y0= *y0_table++, y1= *y1_table++;
		register PEL *write= (PEL *) screen->row_addresses[y0] + x;
		register pixel8 *read= line->texture;
		register _fixed texture_y= line->texture_y, texture_dy= line->texture_dy;
		register short count= y1-y0;

		while ((count-=1)>=0)
		{
			if (read[FIXED_INTEGERAL_PART(texture_y)])
			{
#if BIT_DEPTH==32
				if (seed>=drop_less_than) *write= (pixel32)seed^(((pixel32)seed)<<8);
#else
				if (seed>=drop_less_than) *write= static_cast<PEL>(seed);
#endif
				if (seed&1) seed= (seed>>1)^0xb400; else seed= seed>>1;
			}

			write = (PEL *)((byte *)write + bytes_per_row);
			texture_y+= texture_dy;
		}

		line+= 1;
		x+= 1;
	}
	
	texture_random_seed= seed;
}
