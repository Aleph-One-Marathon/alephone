/*
SHAPES_MACINTOSH.C
Monday, August 28, 1995 1:28:48 PM  (Jason)

Feb. 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Mar 2, 2000 (Loren Petrich):
	Added alias resolution to opening of file

Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.
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
void initialize_pixmap_handler();

/* --------- code */

void initialize_shape_handler()
{
	// FSSpec shapes_file;
	// OSErr error;

	// LP: this is an out-of-date comment!
	// M1 uses the resource fork, but M2 and Moo use the data fork
	/* open the resource fork of our shape file for reading */
		
	/*
	error= get_file_spec(&shapes_file, strFILENAMES, filenameSHAPES8, strPATHS);
	if (error==noErr)
	{
		open_shapes_file(&shapes_file);
	}
	*/
	
	FileSpecifier File;
	get_default_shapes_spec(File);
	if (!File.Open(ShapesFile))
	{
		alert_user(fatalError, strERRORS, badExtraFileLocations, ShapesFile.GetError());
	}
	else
	{
		atexit(shutdown_shape_handler);
	}
	
	// This still remains in shapes_macintosh.c
	initialize_pixmap_handler();
}

void initialize_pixmap_handler()
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
	(*hollow_pixmap)->pmReserved= 0;

	return;
}

void open_shapes_file(FileSpecifier& File)
	// FSSpec *spec)
{
	/*
	short refNum;
	OSErr error;
		
	// LP addition: resolving shapes file if it was an alias
	Boolean is_folder, was_aliased;
	ResolveAliasFile((FSSpec *)spec, TRUE, &is_folder, &was_aliased);
	
	error= FSpOpenDF(spec, fsRdPerm, &refNum);
	if (error==noErr)
	*/
	if (File.Open(ShapesFile))
	{
		// long count= MAXIMUM_COLLECTIONS*sizeof(struct collection_header);
		
		if (!ShapesFile.ReadObjectList(MAXIMUM_COLLECTIONS,collection_headers))
			ShapesFile.Close();
		/*
		FSRead(refNum, &count, (void *) &collection_headers);
		if (error==noErr)
		{
		}
		
		if (error!=noErr)
		{
			FSClose(refNum);
			refNum= -1;
		}
		*/

		// close_shapes_file();
		// shapes_file_refnum= refNum;
	}
	
	return;
}

static void close_shapes_file(
	void)
{
	ShapesFile.Close();
	/*
	OSErr error= noErr;
	if (shapes_file_refnum!=-1)
	{
		error= FSClose(shapes_file_refnum);
		if (error!=noErr)
		{
			shapes_file_refnum= -1;
		}
	}
	*/
	
	return;
}

PixMapHandle get_shape_pixmap(
	short shape,
	boolean force_copy)
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
			if ((*(*(*world_device)->gdPMap)->pmTable)->ctSeed!=(*(*hollow_pixmap)->pmTable)->ctSeed)
			{
				DisposeHandle((Handle)(*hollow_pixmap)->pmTable);
				
				(*hollow_pixmap)->pmTable= (*(*world_device)->gdPMap)->pmTable;	
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
			// LP change:
			assert(false);
			// halt();
	}

	low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
	
	/* setup the pixmap (canÕt wait to change this for Copland) */
	SetRect(&(*hollow_pixmap)->bounds, 0, 0, bitmap->width, bitmap->height);
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


#ifdef OBSOLETE
static void build_shading_tables16(
	struct rgb_color *colors,
	short color_count,
	pixel16 *shading_tables,
	byte *remapping_table)
{
	short i;
	short start, count, level;
	
	memset(shading_tables, iBLACK, PIXEL8_MAXIMUM_COLORS*sizeof(pixel8));
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color *color= colors + start + i;
				RGBColor result;
				short result_index;
				
				result.red= (color->red*level)/(number_of_shading_tables-1);
				result.green= (color->green*level)/(number_of_shading_tables-1);
				result.blue= (color->blue*level)/(number_of_shading_tables-1);
				
				result_index= find_closest_color(&result, colors, color_count);
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]=
					RGBCOLOR_TO_PIXEL16(colors[result_index].red, colors[result_index].green,
						colors[result_index].blue);
			}
		}
	}

	return;
}
#endif

#if 0
static void dump_colors(RGBColor *colors, short color_count)
{
	CTabHandle new_table;
	Handle old_bad_clut;
	RGBColor *color;
	short loop;

	new_table= (CTabHandle) NewHandleClear(sizeof(ColorTable)+color_count*sizeof(ColorSpec));
	HLock((Handle) new_table);
	(*new_table)->ctSeed= GetCTSeed();
	(*new_table)->ctFlags= 0;
	(*new_table)->ctSize= color_count-1;
	
	/* Slam the colors.. */
	color= colors;
	for(loop=0; loop<=color_count; ++loop)
	{
		(*new_table)->ctTable[loop].rgb= *color;
		(*new_table)->ctTable[loop].value= loop;
		color++;
	}
	HUnlock((Handle) new_table);

	old_bad_clut= GetResource('clut', 5454);
	if (old_bad_clut)
	{
		RmveResource((Handle) old_bad_clut);
		DisposeHandle((Handle) old_bad_clut);
		UpdateResFile(CurResFile());
	}
	
	AddResource((Handle) new_table, 'clut', 5454, "\pBad Colors");
	WriteResource((Handle) new_table);
	ReleaseResource((Handle) new_table);
}
#endif

