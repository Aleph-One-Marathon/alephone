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

// LP: begin no-compile
#if 0

// Possibly of historical interest:
// #define COLLECTIONS_RESOURCE_BASE 128
// #define COLLECTIONS_RESOURCE_BASE16 1128

enum /* collection status */
{
	markNONE,
	markLOAD= 1,
	markUNLOAD= 2,
	markSTRIP= 4 /* we don’t want bitmaps, just high/low-level shape data */
};

enum /* flags */
{
	_collection_is_stripped= 0x0001
};

// LP: end no-compile
#endif

/* ---------- globals */

// static short shapes_file_refnum= -1;

/* the dummy pixmap we point to a shape we want to CopyBits */
static CTabHandle hollow_pixmap_color_table= (CTabHandle) NULL;
static PixMapHandle hollow_pixmap;
static pixel8 *hollow_data;

/* --------- private prototypes */

// LP: begin no-compile
#if 0

static void shutdown_shape_handler(void);
static void close_shapes_file(void);

static byte *read_object_from_file(OpenedFile& OFile, long offset, long length);
// static Handle read_handle_from_file(short refNum, long offset, long length);

static byte *make_stripped_collection(byte *collection);
// static void strip_collection_handle(struct collection_definition **collection);

// LP: end no-compile
#endif

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

// LP: begin no-compile
#if 0
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
	// if (error!=noErr || shapes_file_refnum==-1)
	{
		alert_user(fatalError, strERRORS, badExtraFileLocations, ShapesFile.GetError());
	}
	else
	{
		atexit(shutdown_shape_handler);
	}

// LP: end no-compile
#endif

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
				and we just HandToHand the device’s ctTable and throw away our old one. */
			if ((*(*(*world_device)->gdPMap)->pmTable)->ctSeed!=(*(*hollow_pixmap)->pmTable)->ctSeed)
			{
				DisposeHandle((Handle)(*hollow_pixmap)->pmTable);
				
				(*hollow_pixmap)->pmTable= (*(*world_device)->gdPMap)->pmTable;	
				HLock((Handle)hollow_pixmap);
				error= HandToHand((Handle *)&(*hollow_pixmap)->pmTable);
				HUnlock((Handle)hollow_pixmap);
				
				assert(error==noErr);
				
				/* this is a device color table so we don’t clear ctFlags (well, it isn’t a device
					color table anymore, but it’s formatted like one */
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
	
	/* setup the pixmap (can’t wait to change this for Copland) */
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
		
		/* don’t overflow the buffer */
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
		/* if this is a raw, row-order shape then only copy it if we’ve been asked to */
		if (force_copy)
		{
			assert(bitmap->width*bitmap->height<=HOLLOW_PIXMAP_BUFFER_SIZE);
			BlockMove(bitmap->row_addresses[0], hollow_data, bitmap->width*bitmap->height);
			(*hollow_pixmap)->baseAddr= (Ptr)hollow_data;
		}
	}
	
	return hollow_pixmap;
}

// LP: begin no-compile
#if 0

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

/* --------- private code */

static void shutdown_shape_handler(
	void)
{
	close_shapes_file();
	
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

static byte *make_stripped_collection(byte *collection)
{
	long StrippedLength =
		((collection_definition *)collection)->low_level_shape_offset_table_offset;
	byte *new_collection = new byte[StrippedLength];
	memcpy(new_collection, collection, StrippedLength);
	((collection_definition *)new_collection)->low_level_shape_count= 0;
	((collection_definition *)new_collection)->bitmap_count= 0;
	
	return new_collection;
}
/*
static void strip_collection_handle(
	struct collection_definition **collection)
{
	SetHandleSize((Handle)collection, (*collection)->low_level_shape_offset_table_offset);
	(*collection)->low_level_shape_count= 0;
	(*collection)->bitmap_count= 0;
	
	return;
}
*/

static boolean collection_loaded(
	struct collection_header *header)
{
	return header->collection ? TRUE : FALSE;
}

static void unload_collection(
	struct collection_header *header)
{
	assert(header->collection);
	
	/* unload collection */
	delete []header->collection;
	delete []header->shading_tables;
	// DisposeHandle((Handle)header->collection);
	// DisposeHandle((Handle)header->shading_tables);
	header->collection= NULL;
	
	return;
}

static void unlock_collection(
	struct collection_header *header)
{
	// assert(header->collection);
	
	// HUnlock((Handle)header->collection);
	// HUnlock((Handle)header->shading_tables);
	
	return;
}

static void lock_collection(
	struct collection_header *header)
{
	// assert(header->collection);
	
	// MoveHHi((Handle)header->collection);
	// HLock((Handle)header->collection);

	// MoveHHi((Handle)header->shading_tables);
	// HLock((Handle)header->shading_tables);
	
	return;
}

static boolean load_collection(
	short collection_index,
	boolean strip)
{
	struct collection_header *header= get_collection_header(collection_index);
	// Handle collection= NULL, shading_tables= NULL;
	byte *collection = NULL, *shading_tables = NULL;
	OSErr error= noErr;
	
	if (bit_depth==8 || header->offset16==-1)
	{
		vassert(header->offset!=-1, csprintf(temporary, "collection #%d does not exist.", collection_index));
		collection= read_object_from_file(ShapesFile, header->offset, header->length);
		// collection= read_handle_from_file(shapes_file_refnum, header->offset, header->length);
	}
	else
	{
		collection= read_object_from_file(ShapesFile, header->offset16, header->length16);
		// collection= read_handle_from_file(shapes_file_refnum, header->offset16, header->length16);
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
		// if (strip) strip_collection_handle((struct collection_definition **) collection);
		// MoveHHi(collection), HLock(collection);
		// header->collection= (struct collection_definition **) collection;
		header->collection= (collection_definition *) collection;
	
		/* allocate enough space for this collection’s shading tables */
		if (strip)
		{
			shading_tables = NULL;
			// shading_tables= NewHandle(0);
		}
		else
		{
			struct collection_definition *definition= get_collection_definition(collection_index);
			
			shading_tables = new byte[get_shading_table_size(collection_index)*definition->clut_count +
				shading_table_size*NUMBER_OF_TINT_TABLES];
			// shading_tables= NewHandle(get_shading_table_size(collection_index)*definition->clut_count +
			// 	shading_table_size*NUMBER_OF_TINT_TABLES);
			if ((error= MemError())==noErr)
			{
				assert(shading_tables);
				// MoveHHi(shading_tables), HLock(shading_tables);
			}
		}
		
		header->shading_tables= shading_tables;
		// header->shading_tables= (void **)shading_tables;
	}
	else
	{
		error= MemError();
//		vhalt(csprintf(temporary, "couldn’t load collection #%d (error==#%d)", collection_index, error));
	}

	/* if any errors ocurred, free whatever memory we used */
	if (error!=noErr)
	{
		if (collection) delete []collection;
		if (shading_tables) delete []shading_tables;
		// if (collection) DisposeHandle(collection);
		// if (shading_tables) DisposeHandle(shading_tables);
	}
	
	return error==noErr ? TRUE : FALSE;
}

// static Handle read_handle_from_file(
static byte *read_object_from_file(
	OpenedFile& OFile,
	// short refNum,
	long offset,
	long length)
{
	if (!OFile.IsOpen()) return NULL;
	
	// OSErr error= noErr;
	// Handle data= NULL;
	byte *data = NULL;
	
	if (length <= 0) return NULL;
	if (!(data = new byte[length])) return NULL;
	// if (!(data= NewHandle(length))) return NULL;
	// HLock(data);
	
	if (!OFile.SetPosition(offset))
	{
		delete []data;
		return NULL;
	}
	if (!OFile.ReadObjectList(length,data))
	{
		delete []data;
		return NULL;
	}
	
	// HUnlock(data);
	return data;
	
	/*
	// if (refNum!=-1)
	{
		if ((data= NewHandle(length)) != NULL)
		// LP: replacing old file-I/O routines
		{
		{
			ParamBlockRec param;
			
			HLock(data);
			
			param.ioParam.ioCompletion= (IOCompletionUPP) NULL;
			param.ioParam.ioRefNum= refNum;
			param.ioParam.ioBuffer= *data;
			param.ioParam.ioReqCount= length;
			param.ioParam.ioPosMode= fsFromStart;
			param.ioParam.ioPosOffset= offset;
			
			error= PBReadSync(&param);
			if (error==noErr)
			{
				HUnlock(data);
			}
			else
			{
				DisposeHandle(data);
				
				data= NULL;
			}
		}
		else
		{
			error= MemError();
		}
	}
	
	vassert(data, "could not load data in read_handle_from_file()");
	// vwarn(error==noErr, csprintf(temporary, "load_sound() got error #%d", error));
	
	return data;
	*/
}

/* --------- collection accessors */

static struct collection_definition *get_collection_definition(
	short collection_index)
{
	struct collection_definition *collection= get_collection_header(collection_index)->collection;

	vassert(collection, csprintf(temporary, "collection #%d isn’t loaded", collection_index));

	return collection; // (struct collection_definition *) StripAddress(*collection);
}

static void *get_collection_shading_tables(
	short collection_index,
	short clut_index)
{
	void *shading_tables= get_collection_header(collection_index)->shading_tables;

	(byte *)shading_tables+= clut_index*get_shading_table_size(collection_index);
	// shading_tables= StripAddress(shading_tables);
	
	return shading_tables;
}

static void *get_collection_tint_tables(
	short collection_index,
	short tint_index)
{
	struct collection_definition *definition= get_collection_definition(collection_index);
	void *tint_table= get_collection_header(collection_index)->shading_tables;

	(byte *)tint_table+= get_shading_table_size(collection_index)*definition->clut_count + shading_table_size*tint_index;
	// tint_table= StripAddress(tint_table);
	
	return tint_table;
}

static struct collection_definition *_get_collection_definition(
	short collection_index)
{
	struct collection_definition *collection= get_collection_header(collection_index)->collection;

	return collection; // ? (struct collection_definition *) StripAddress(*collection) : (struct collection_definition *) NULL;
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

// LP: end no-compile
#endif

