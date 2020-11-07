/*
LOW_LEVEL_TEXTURES.C

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

Friday, August 19, 1994 2:05:54 PM

Monday, February 27, 1995 11:40:47 PM  (Jason')
	rob suggests that the PPC might not write-allocate cache lines so we might be faster if we
	read from a location we’re about to write to.  he also suggested a rowbytes of 704 instead
	of 640 off-screen for better cache performance.

Jan 30, 2000 (Loren Petrich):
	Added some typecasts
	Removed some "static" declarations that conflict with "extern"
*/

#include "cseries.h"
#include "preferences.h"
#include "textures.h"
#include "scottish_textures.h"

/* ---------- global state */

inline uint16 & texture_random_seed()
{
	static uint16 seed = 6906;
	return seed;
}

/* ---------- texture horizontal polygon */

#define HORIZONTAL_WIDTH_SHIFT 7 /* 128 (8 for 256) */
#define HORIZONTAL_HEIGHT_SHIFT 7 /* 128 */
#define HORIZONTAL_FREE_BITS (32-TRIG_SHIFT-WORLD_FRACTIONAL_BITS)
#define HORIZONTAL_WIDTH_DOWNSHIFT (32-HORIZONTAL_WIDTH_SHIFT)
#define HORIZONTAL_HEIGHT_DOWNSHIFT (32-HORIZONTAL_HEIGHT_SHIFT)

struct _horizontal_polygon_line_header
{
	int32 y_downshift;
};

struct _horizontal_polygon_line_data
{
	uint32 source_x, source_y;
	uint32 source_dx, source_dy;
	
	void *shading_table;
};

/* ---------- texture vertical polygon */

#define VERTICAL_TEXTURE_WIDTH 128
#define VERTICAL_TEXTURE_WIDTH_BITS 7
#define VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS (FIXED_FRACTIONAL_BITS-VERTICAL_TEXTURE_WIDTH_BITS)
#define VERTICAL_TEXTURE_ONE (1<<VERTICAL_TEXTURE_WIDTH_FRACTIONAL_BITS)
#define VERTICAL_TEXTURE_FREE_BITS FIXED_FRACTIONAL_BITS
#define VERTICAL_TEXTURE_DOWNSHIFT (32-VERTICAL_TEXTURE_WIDTH_BITS)

struct _vertical_polygon_data
{
	int16 downshift;
	int16 x0;
	int16 width;
	
	int16 pad;
};

struct _vertical_polygon_line_data
{
	void *shading_table;
	pixel8 *texture;
	int32 texture_y, texture_dy;
};

/* ---------- code */

// Find the next lower power of 2, and return the exponent
inline int NextLowerExponent(int n)
{
	int xp = 0;
	while(n > 1) {n >>= 1; xp++;}
	return xp;
}

template <typename T>
inline T average(T fg, T bg)
{
	return fg;
}

template <>
inline pixel32 average(pixel32 fg, pixel32 bg)
{
	// badly assume that the pixel format is ARGB
	return ( ((((fg) ^ (bg)) & 0xfffefefeL) >> 1) + ((fg) & (bg)) );
}

template <>
inline pixel16 average(pixel16 fg, pixel16 bg)
{
	// badly assume that the pixel format is 565
	return ( ((((fg) ^ (bg)) & 0xf7deU) >> 1) + ((fg) & (bg)) );
}

template <typename T>
inline T alpha_blend(T fg, T bg, pixel8 alpha, pixel32 rmask, pixel32 bmask, pixel32 gmask)
{
	return (
		(rmask & ((bg & rmask) + ((int)(1LL*((int)(fg & rmask) - (int)(bg & rmask)) * alpha) >> 8))) |
		(gmask & ((bg & gmask) + ((int)(1LL*((int)(fg & gmask) - (int)(bg & gmask)) * alpha) >> 8))) |
		(bmask & ((bg & bmask) + ((int)(1LL*((int)(fg & bmask) - (int)(bg & bmask)) * alpha) >> 8)))
		);
}

template <typename T, int sw_alpha_blend, bool check_transparency>
void inline write_pixel(T *dst, pixel8 pixel, T *shading_table, uint8 *opacity_table, pixel32 rmask, pixel32 gmask, pixel32 bmask)
{
	if (!check_transparency || pixel != 0) 
	{
		if (sw_alpha_blend == _sw_alpha_off)
		{
			*dst = shading_table[pixel];
		} 
		else if (sw_alpha_blend == _sw_alpha_fast)
		{
			*dst = average(shading_table[pixel], *dst);
		}
		else if (sw_alpha_blend == _sw_alpha_nice)
		{
			*dst = alpha_blend(shading_table[pixel], *dst, opacity_table[pixel], rmask, gmask, bmask);
		}
	}	
}

template <typename T, int sw_alpha_blend>
void texture_horizontal_polygon_lines
(
	struct bitmap_definition *texture,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short *x0_table,
	short *x1_table,
	short line_count,
	uint8 *opacity_table = 0
)
{
	(void) (view);

	pixel32 rmask = 0;
	pixel32 gmask = 0;
	pixel32 bmask = 0;

	if (sw_alpha_blend == _sw_alpha_nice)
	{
		extern SDL_Surface *world_pixels;
		SDL_PixelFormat *fmt = world_pixels->format;
		
		rmask = fmt->Rmask;
		gmask = fmt->Gmask;
		bmask = fmt->Bmask;
	}

	while ((line_count-= 1)>=0)
	{
		short x0= *x0_table++, x1= *x1_table++;
		
		T *shading_table= (T *)data->shading_table;
		T *write= (T *) screen->row_addresses[y0] + x0;
		pixel8 *base_address= texture->row_addresses[0];
		uint32 source_x= data->source_x;
		uint32 source_y= data->source_y;
		uint32 source_dx= data->source_dx;
		uint32 source_dy= data->source_dy;
		short count= x1-x0;
		
		while ((count-= 1)>=0)
		{
			write_pixel<T, sw_alpha_blend, false>(write++, base_address[((source_y>>(HORIZONTAL_HEIGHT_DOWNSHIFT-7))&(0x7f<<7))+(source_x>>HORIZONTAL_WIDTH_DOWNSHIFT)], shading_table, opacity_table, rmask, gmask, bmask);
			
			source_x+= source_dx, source_y+= source_dy;
		}
		
		data+= 1;
		y0+= 1;
	}
}

#define LANDSCAPE_WIDTH_BITS 9
#define LANDSCAPE_TEXTURE_WIDTH_DOWNSHIFT (32-LANDSCAPE_WIDTH_BITS)
template <typename T>
void landscape_horizontal_polygon_lines(
	struct bitmap_definition *texture,
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _horizontal_polygon_line_data *data,
	short y0,
	short *x0_table,
	short *x1_table,
	short line_count)
{
	short landscape_texture_width_downshift= 32 - NextLowerExponent(texture->height);

	(void) (view);

	while ((line_count-= 1)>=0)
	{
		short x0= *x0_table++, x1= *x1_table++;
		
		T *shading_table= (T *)data->shading_table;
		T *write= (T *)screen->row_addresses[y0] + x0;
		pixel8 *read= texture->row_addresses[data->source_y];
		uint32 source_x= data->source_x;
		uint32 source_dx= data->source_dx;
		short count= x1-x0;
		
		while ((count-= 1)>=0)
		{
			*write++= shading_table[read[source_x>>landscape_texture_width_downshift]];
			source_x+= source_dx;
		}
		
		data+= 1;
		y0+= 1;
	}
}

template <typename T, bool check_transparent>
void inline copy_check_transparent(T *dst, pixel8 read, T *shading_table)
{
	if (!check_transparent || read != 0)
	{
		*dst = shading_table[read];
	}
}


template <typename T, int sw_alpha_blend, bool check_transparent>
void texture_vertical_polygon_lines(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table, 
	uint8 *opacity_table = 0)
{
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	int bytes_per_row= screen->bytes_per_row;
	int downshift= data->downshift;
	int line_count= data->width;
	bool aborted= false;
	int x= data->x0;
	int count;

	(void) (view);

	pixel32 rmask = 0;
	pixel32 gmask = 0;
	pixel32 bmask = 0;

	if (sw_alpha_blend == _sw_alpha_nice) {
		extern SDL_Surface *world_pixels;
		SDL_PixelFormat *fmt = world_pixels->format;
		
		rmask = fmt->Rmask;
		gmask = fmt->Gmask;
		bmask = fmt->Bmask;
	}

	while (line_count>0)	
	{
		if (line_count<4 || (x&3) || aborted)
		{
			int y0= *y0_table++, y1= *y1_table++;
			uint32 texture_y= line->texture_y;
			uint32 texture_dy= line->texture_dy;
			T *write, *shading_table;
			pixel8 *read;

			shading_table= (T *)line->shading_table;
			read= line->texture;
			write = (T *)(screen->row_addresses[0] + bytes_per_row * y0) + x; // invalid but unread if y0 == screen->height

			for (count= y1-y0; count>0; --count)
			{
				write_pixel<T, sw_alpha_blend, check_transparent>(write, read[texture_y>>downshift], shading_table, opacity_table, rmask, gmask, bmask);

				write = (T *)((byte *)write + bytes_per_row);
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
			T *shading_table0= (T *)line[0].shading_table;
			
			uint32 texture_y1= line[1].texture_y, texture_dy1= line[1].texture_dy;
			pixel8 *read1= line[1].texture;
			T *shading_table1= (T *)line[1].shading_table;
			
			uint32 texture_y2= line[2].texture_y, texture_dy2= line[2].texture_dy;
			pixel8 *read2= line[2].texture;
			T *shading_table2= (T *)line[2].shading_table;
			
			uint32 texture_y3= line[3].texture_y, texture_dy3= line[3].texture_dy;
			pixel8 *read3= line[3].texture;
			T *shading_table3= (T *)line[3].shading_table;
			
			T *write;

			int ymax;
			
			/* sync */	
			{
				int y0= y0_table[0], y1= y0_table[1], y2= y0_table[2], y3= y0_table[3];
				T *temp_write;
				
				ymax= MAX(y0, y1), ymax= MAX(ymax, y2), ymax= MAX(ymax, y3);
				
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

				write = (T *)screen->row_addresses[ymax] + x;
				
				for (count= ymax-y0, temp_write= (T *)screen->row_addresses[y0] + x; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write, read0[texture_y0>>downshift], shading_table0);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= ymax-y1, temp_write= (T *)screen->row_addresses[y1] + x; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 1, read1[texture_y1>>downshift], shading_table1);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= ymax-y2, temp_write= (T *)screen->row_addresses[y2] + x; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 2, read2[texture_y2>>downshift], shading_table2);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= ymax-y3, temp_write= (T *)screen->row_addresses[y3] + x; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 3, read3[texture_y3>>downshift], shading_table3);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
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
					write_pixel<T, sw_alpha_blend, check_transparent>(write, read0[texture_y0>>downshift], shading_table0, opacity_table, rmask, gmask, bmask);
					texture_y0+= texture_dy0;
		
					write_pixel<T, sw_alpha_blend, check_transparent>(write+1, read1[texture_y1>>downshift], shading_table1, opacity_table, rmask, gmask, bmask);
					texture_y1+= texture_dy1;

					write_pixel<T, sw_alpha_blend, check_transparent>(write+2, read2[texture_y2>>downshift], shading_table2, opacity_table, rmask, gmask, bmask);
					texture_y2+= texture_dy2;

					write_pixel<T, sw_alpha_blend, check_transparent>(write+3, read3[texture_y3>>downshift], shading_table3, opacity_table, rmask, gmask, bmask);
					texture_y3+= texture_dy3;
					
					write = (T *)((byte *)write + bytes_per_row);
				}
			}

			/* desync */	
			{
				T *temp_write;
				
				for (count= y1_table[0] - ymax, temp_write= write; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write, read0[texture_y0>>downshift], shading_table0);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y0+= texture_dy0;
				}
				
				for (count= y1_table[1] - ymax, temp_write= write; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 1, read1[texture_y1>>downshift], shading_table1);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y1+= texture_dy1;
				}
				
				for (count= y1_table[2] - ymax, temp_write= write; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 2, read2[texture_y2>>downshift], shading_table2);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
					texture_y2+= texture_dy2;
				}
		
				for (count= y1_table[3] - ymax, temp_write= write; count>0; --count)
				{
					copy_check_transparent<T, check_transparent>(temp_write + 3, read3[texture_y3>>downshift], shading_table3);
					temp_write = (T *)((byte *)temp_write + bytes_per_row);
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

template <typename T>
inline void *tint_tables_pointer(_vertical_polygon_line_data *line, short tint_table_index)
{
	return 0;
}

template <>
inline void *tint_tables_pointer<pixel8>(_vertical_polygon_line_data *line, short tint_table_index)
{
	return (void *) ((pixel8 *) line->shading_table + tint_table_index * sizeof(struct tint_table8));
}

template <>
inline void *tint_tables_pointer<pixel16>(_vertical_polygon_line_data *line, short tint_table_index)
{
	return (void *) ((struct tint_table16 *) line->shading_table + (tint_table_index<<1));
}

template <>
inline void *tint_tables_pointer<pixel32>(_vertical_polygon_line_data *line, short tint_table_index)
{
	return (void *) ((struct tint_table32 *) line->shading_table + (tint_table_index<<3));
}

template <typename T>
inline T get_pixel_tint(T, void *, SDL_PixelFormat *)
{
	return 0;
}

template <>
inline pixel8 get_pixel_tint(pixel8 pixel, void *tint_tables, SDL_PixelFormat *)
{
	return ((pixel8 *) tint_tables)[pixel];
}

template<>
inline pixel16 get_pixel_tint(pixel16 pixel, void *tint_tables_pv, SDL_PixelFormat *fmt)
{
	tint_table16 *tint_tables = (tint_table16 *) tint_tables_pv;
	uint8 r = (((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
	uint8 g = (((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
	uint8 b = (((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);

	return tint_tables->red[r >> 3] | tint_tables->green[g >> 3] | tint_tables->blue[b >> 3];
}

template <>
inline pixel32 get_pixel_tint(pixel32 pixel, void *tint_tables_pv, SDL_PixelFormat *fmt)
{
	tint_table32 *tint_tables = (tint_table32 *) tint_tables_pv;
	uint8 r = (((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
	uint8 g = (((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
	uint8 b = (((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);

	return tint_tables->red[r] | tint_tables->green[g] | tint_tables->blue[b];
}

template <typename T>
void tint_vertical_polygon_lines(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table,
	uint16 transfer_data)
{
	short tint_table_index= transfer_data&0xff;
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	short bytes_per_row= screen->bytes_per_row;
	int line_count= data->width;
	int x= data->x0;

	void *tint_tables = tint_tables_pointer<T>(line, tint_table_index);
	
	(void) (view);

	extern SDL_Surface *world_pixels;
	
	fc_assert(tint_table_index>=0 && tint_table_index<number_of_shading_tables);

	while ((line_count-= 1)>=0)
	{
		short y0= *y0_table++, y1= *y1_table++;
		T *write= (T *) screen->row_addresses[y0] + x;
		pixel8 *read= line->texture;
		_fixed texture_y= line->texture_y, texture_dy= line->texture_dy;
		short count= y1-y0;

		while ((count-=1)>=0)
		{
			if (read[FIXED_INTEGERAL_PART(texture_y)])
			{
				*write = get_pixel_tint<T>(*write, tint_tables, world_pixels->format);

			}

			write = (T *)((byte *)write + bytes_per_row);
			texture_y+= texture_dy;
		}

		line+= 1;
		x+= 1;
	}
}


template <typename T>
inline T randomize_vertical_polygon_lines_write(uint16 seed)
{
	return static_cast<T>(seed);
}

template <>
inline pixel32 randomize_vertical_polygon_lines_write<pixel32>(uint16 seed)
{
	return (pixel32)seed^(((pixel32)seed)<<8);
}

template <typename T, bool check_transparent>
void randomize_vertical_polygon_lines(
	struct bitmap_definition *screen,
	struct view_data *view,
	struct _vertical_polygon_data *data,
	short *y0_table,
	short *y1_table,
	uint16 transfer_data)
{
	struct _vertical_polygon_line_data *line= (struct _vertical_polygon_line_data *) (data+1);
	short bytes_per_row= screen->bytes_per_row;
	int line_count= data->width;
	int x= data->x0;
	uint16 seed= texture_random_seed();
	uint16 drop_less_than= transfer_data;

	(void) (view);

	while ((line_count-= 1)>=0)
	{
		short y0= *y0_table++, y1= *y1_table++;
		T *write= (T *) screen->row_addresses[y0] + x;
		pixel8 *read= line->texture;
		_fixed texture_y= line->texture_y, texture_dy= line->texture_dy;
		short count= y1-y0;

		while ((count-=1)>=0)
		{
			if (!check_transparent || read[texture_y>>(data->downshift)])
			{
				if (seed >= drop_less_than) *write = randomize_vertical_polygon_lines_write<T>(seed);
				if (seed&1) seed= (seed>>1)^0xb400; else seed= seed>>1;
			}

			write = (T *)((byte *)write + bytes_per_row);
			texture_y+= texture_dy;
		}

		line+= 1;
		x+= 1;
	}
	
	texture_random_seed() = seed;
}
