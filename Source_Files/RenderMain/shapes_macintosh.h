/*
SHAPES_MACINTOSH.C - Shapes handling, MacOS specific stuff (included by shapes.c)

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

Monday, August 28, 1995 1:28:48 PM  (Jason)

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 2, 2000 (Loren Petrich):
	Added alias resolution to opening of file

Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.

Oct 19, 2000 (Loren Petrich):
	Added graceful degradation in the case of frames or bitmaps not being found;
	get_shape_pixmap() returns NULL when that happens

Jan 25, 2002 (Br'fin (Jeremy Parsons)):
	Removed access to pmReserved of the pixmap under Carbon
*/

/* ---------- constants */

#define HOLLOW_PIXMAP_BUFFER_SIZE 0
//was (80*1024)

/* ---------- globals */

// static short shapes_file_refnum= -1;

/* the dummy pixmap we point to a shape we want to CopyBits */
static CTabHandle hollow_pixmap_color_table= (CTabHandle) NULL;
static PixMapHandle hollow_pixmap;
static pixel8 *hollow_data;

/* --------- private prototypes */

// LP: separating out initing the hollow pixmap, used for reading in PICT resources
static void initialize_pixmap_handler();

/* --------- code */

static void initialize_pixmap_handler()
{
	hollow_pixmap= NewPixMap();
	assert(hollow_pixmap);
	if (HOLLOW_PIXMAP_BUFFER_SIZE)
	{
		hollow_data= (unsigned char *)NewPtr(HOLLOW_PIXMAP_BUFFER_SIZE);
		assert(hollow_data);
	}

	/* bounds and rowBytes are deliberately unset! */
	(*hollow_pixmap)->pixelType= 0;
	(*hollow_pixmap)->pixelSize= 8;
	(*hollow_pixmap)->cmpCount= 1;
	(*hollow_pixmap)->cmpSize= (*hollow_pixmap)->pixelSize;
/*
#if !defined(USE_CARBON_ACCESSORS)
	(*hollow_pixmap)->pmReserved= 0;
#endif
*/
}


PixMapHandle get_shape_pixmap(
	short shape,
	bool force_copy)
{
	OSErr error;
	struct collection_definition *collection;
	struct low_level_shape_definition *low_level_shape;
	struct bitmap_definition *bitmap;
	short collection_index, low_level_shape_index, clut_index;

	collection_index= GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(shape));
	clut_index= GET_COLLECTION_CLUT(GET_DESCRIPTOR_COLLECTION(shape));
	low_level_shape_index= GET_DESCRIPTOR_SHAPE(shape);
 	collection= get_collection_definition(collection_index);

	switch (interface_bit_depth)
	{
		case 8:
			/* if the ctSeed of our offscreen pixmap is different from the ctSeed of the world
				device then the color environment has changed since the last call to our routine,
				and we just HandToHand the deviceÕs ctTable and throw away our old one. */
			if ((*(*(*GetWorldDevice())->gdPMap)->pmTable)->ctSeed!=(*(*hollow_pixmap)->pmTable)->ctSeed)
			{
				DisposeHandle((Handle)(*hollow_pixmap)->pmTable);
				
				(*hollow_pixmap)->pmTable= (*(*GetWorldDevice())->gdPMap)->pmTable;	
				HLock((Handle)hollow_pixmap);
				error= HandToHand((Handle *)&(*hollow_pixmap)->pmTable);
				HUnlock((Handle)hollow_pixmap);
				
				assert(error==noErr);
				
				/* this is a device color table so we donÕt clear ctFlags (well, it isnÕt a device
					color table anymore, but itÕs formatted like one */
			}
			break;
		
		case 16:
		case 32:
			if (!hollow_pixmap_color_table)
			{
				hollow_pixmap_color_table= (CTabHandle) NewHandle(sizeof(ColorTable)+PIXEL8_MAXIMUM_COLORS*sizeof(ColorSpec));
				MoveHHi((Handle)hollow_pixmap_color_table);
				HLock((Handle)hollow_pixmap_color_table);
				assert(hollow_pixmap_color_table);
			}
			
			(*hollow_pixmap_color_table)->ctSeed= GetCTSeed();
			(*hollow_pixmap_color_table)->ctSize= collection->color_count-NUMBER_OF_PRIVATE_COLORS-1;
			(*hollow_pixmap_color_table)->ctFlags= 0;
			
			BlockMove(get_collection_colors(collection_index, clut_index)+NUMBER_OF_PRIVATE_COLORS, &(*hollow_pixmap_color_table)->ctTable,
				(collection->color_count-NUMBER_OF_PRIVATE_COLORS)*sizeof(ColorSpec));
			
			(*hollow_pixmap)->pmTable= hollow_pixmap_color_table;
			
			break;
		
		default:
			assert(false);
			break;
	}

	low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	if (!low_level_shape) return NULL;
	bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
	if (!bitmap) return NULL;
	
	/* setup the pixmap (canÕt wait to change this for Copland) */
	(*hollow_pixmap)->bounds.top= 0;
	(*hollow_pixmap)->bounds.left= 0;
	(*hollow_pixmap)->bounds.bottom= bitmap->height;
	(*hollow_pixmap)->bounds.right= bitmap->width;
	(*hollow_pixmap)->rowBytes= bitmap->width|0x8000;
	(*hollow_pixmap)->baseAddr= (Ptr)bitmap->row_addresses[0];
	
	if (bitmap->bytes_per_row==NONE) /* is this a compressed shape? */
	{
		register pixel8 *read, *write;
		register short run_count;
		short x;

		/* for now all RLE shapes are in column-order */
		assert(bitmap->flags&_COLUMN_ORDER_BIT);
		
		/* donÕt overflow the buffer */
		assert(bitmap->width*bitmap->height<=HOLLOW_PIXMAP_BUFFER_SIZE);
		
		/* decompress column-order shape into row-order buffer */
		for (x=0;x<bitmap->width;x+=1)
		{
			short bytes_per_row= bitmap->width;
			
			write= hollow_data+x;
			read= bitmap->row_addresses[x];
			while ((run_count= *((short*)read)++)!=0)
			{
				if (run_count<0) while ((run_count+=1)<=0) *write= iBLACK, write+= bytes_per_row; /* fill transparent areas with black */
					else while ((run_count-=1)>=0) *write= *read++, write+= bytes_per_row; /* copy shape data */
			}
		}

		(*hollow_pixmap)->baseAddr= (Ptr)hollow_data;
	}
	else
	{
		/* if this is a raw, row-order shape then only copy it if weÕve been asked to */
		if (force_copy)
		{
			assert(bitmap->width*bitmap->height<=HOLLOW_PIXMAP_BUFFER_SIZE);
			BlockMove(bitmap->row_addresses[0], hollow_data, bitmap->width*bitmap->height);
			(*hollow_pixmap)->baseAddr= (Ptr)hollow_data;
		}
	}
	
	return hollow_pixmap;
}

#if 0
static bool load_collection(
	short collection_index,
	bool strip)
{
	struct collection_header *header= get_collection_header(collection_index);
	byte *collection = NULL, *shading_tables = NULL;
	OSErr error= noErr;
	
	if (bit_depth==8 || header->offset16==-1)
	{
		vassert(header->offset!=-1, csprintf(temporary, "collection #%d does not exist.", collection_index));
		collection= read_object_from_file(ShapesFile, header->offset, header->length);
	}
	else
	{
		collection= read_object_from_file(ShapesFile, header->offset16, header->length16);
	}
	error = ShapesFile.GetError();
	if (!collection && error==noErr) error = MemError();
	vwarn(error==noErr, csprintf(temporary, "read_handle_from_file() got error #%d", error));

	if (collection)
	{
		if (strip) {
			byte *new_collection = make_stripped_collection(collection);
			delete []collection;
			collection = new_collection;
		}
		header->collection= (collection_definition *) collection;
	
		/* allocate enough space for this collectionÕs shading tables */
		if (strip)
		{
			shading_tables = NULL;
		}
		else
		{
			struct collection_definition *definition= get_collection_definition(collection_index);
			
			shading_tables = new byte[get_shading_table_size(collection_index)*definition->clut_count +
				shading_table_size*NUMBER_OF_TINT_TABLES];
			if ((error= MemError())==noErr)
			{
				assert(shading_tables);
			}
		}
		
		header->shading_tables= shading_tables;
	}
	else
	{
		error= MemError();
//		vhalt(csprintf(temporary, "couldnÕt load collection #%d (error==#%d)", collection_index, error));
	}

	/* if any errors ocurred, free whatever memory we used */
	if (error!=noErr)
	{
		if (collection) delete []collection;
		if (shading_tables) delete []shading_tables;
	}
	
	return error==noErr ? true : false;
}
#endif

static bool load_collection(
	short collection_index,
	bool strip)
{
	struct collection_header *header= get_collection_header(collection_index);
	byte *collection = NULL, *shading_tables = NULL;
	long length;	// How many bytes in the collection chunk
	OSErr error= noErr;
	
	if (bit_depth==8 || header->offset16==-1)
	{
		vassert(header->offset!=-1, csprintf(temporary, "Collection #%d does not exist.", collection_index));
		collection= read_object_from_file(ShapesFile, header->offset, (length = header->length));
	}
	else
	{
		collection= read_object_from_file(ShapesFile, header->offset16, (length = header->length16));
	}
	error = ShapesFile.GetError();
	if (!collection && error==noErr) error = memory_error();
	vwarn(error==noErr, csprintf(temporary, "read_object_from_file() got error #%d", error));
	
	if (collection)
	{
		header->collection = (collection_definition *)unpack_collection(collection,length,strip);
		
		// No longer needed!
		delete []collection;
		
		// header->collection= (collection_definition *) collection;
		
		/* allocate enough space for this collectionÕs shading tables */
		if (strip)
		{
			shading_tables = NULL;
		}
		else
		{
			collection_definition *definition = header->collection;
			shading_tables = new byte[get_shading_table_size(collection_index)*definition->clut_count +
				shading_table_size*NUMBER_OF_TINT_TABLES];
			if ((error= memory_error())==noErr)
			{
				assert(shading_tables);
			}
		}
		
		header->shading_tables= shading_tables;
	}
	else
	{
		error= memory_error();
//		vhalt(csprintf(temporary, "couldnÕt load collection #%d (error==#%d)", collection_index, error));
	}

	/* if any errors ocurred, free whatever memory we used */
	if (error!=noErr)
	{
		unload_collection(header);
	}
	
	return error==noErr ? TRUE : FALSE;
}

static void unload_collection(
	struct collection_header *header)
{
	// assert(header->collection);
	
	/* unload collection */
	if (header->collection) delete []header->collection;
	header->collection = NULL;
	
	if (header->shading_tables) delete []header->shading_tables;
	header->shading_tables = NULL;
}
