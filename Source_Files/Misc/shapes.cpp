/*
SHAPES.C
Saturday, September 4, 1993 9:26:41 AM

Thursday, May 19, 1994 9:06:28 AM
	unification of wall and object shapes complete, new shading table builder.
Wednesday, June 22, 1994 11:55:07 PM
	we now read data from alain’s shape extractor.
Saturday, July 9, 1994 3:22:11 PM
	lightening_table removed; we now build darkening tables on a collection-by-collection basis
	(one 8k darkening table per clut permutation of the given collection)
Monday, October 3, 1994 4:17:15 PM (Jason)
	compressed or uncompressed collection resources
Friday, June 16, 1995 11:34:08 AM  (Jason)
	self-luminescent colors

Jan 30, 2000 (Loren Petrich):
	Changed "new" to "_new" to make data structures more C++-friendly
	Did some typecasts

Feb 3, 2000 (Loren Petrich):
	Changed _collection_madd to _collection_vacbob (later changed all "vacbob"'s to "civilian_fusion"'s)

Feb 4, 2000 (Loren Petrich):
	Changed halt() to assert(false) for better debugging

Feb 12, 2000 (Loren Petrich):
	Set up a fallback strategy for the colors;
	when there are more colors in all the tables than there are in the general color table,
	then look for the nearest one.

Feb 24, 2000 (Loren Petrich):
	Added get_number_of_collection_frames(), so as to assist in wall-texture error checking

Mar 14, 2000 (Loren Petrich):
	Added accessors for number of bitmaps and which bitmap index for a frame index;
	these will be useful for OpenGL rendering

Mar 23, 2000 (Loren Petrich):
	Made infravision tinting more generic and reassignable

Aug 12, 2000 (Loren Petrich):
	Using object-oriented file handler
	
Aug 14, 2000 (Loren Petrich):
	Turned collection and shading-table handles into pointers,
	because handles are needlessly MacOS-specific,
	and because these are variable-format objects.

Aug 26, 2000 (Loren Petrich):
	Moved get_default_shapes_spec() to preprocess_map_mac.c

Sept 2, 2000 (Loren Petrich):
	Added shapes-file unpacking.
*/

/*
//gracefully handle out-of-memory conditions when loading shapes.  it will happen.
//get_shape_descriptors() needs to look at high-level instead of low-level shapes when fetching scenery instead of walls/ceilings/floors
//get_shape_information() is called often, and is quite slow
//it is possible to have more than 255 low-level shapes in a collection, which means the existing shape_descriptor is too small
//must build different shading tables for each collection (even in 8-bit, for alternate color tables)
*/

#include "cseries.h"

#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "render.h"
#include "interface.h"
#include "collection_definition.h"
#include "screen.h"
#include "FileHandler.h"

#include "map.h"

// LP addition: OpenGL support
#include "OGL_Render.h"

// LP addition: infravision XML setup needs colors
#include "ColorParser.h"

#include "Packing.h"

#ifdef env68k
#pragma segment shell
#endif

/* ---------- constants */

#define iWHITE 1
#ifdef SCREAMING_METAL
#define iBLACK 255
#else
#define iBLACK 18
#endif

/* each collection has a tint table which (fully) tints the clut of that collection to whatever it
	looks like through the light enhancement goggles */
#define NUMBER_OF_TINT_TABLES 1

// Moved from shapes_macintosh.c:

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

/* ---------- macros */

// LP: fake portable-files stuff
#ifdef mac
inline short memory_error() {return MemError();}
#else
inline short memory_error() {return 0;}
#endif

/* ---------- structures */

/* ---------- globals */

#include "shape_definitions.h"

static pixel16 *global_shading_table16= (pixel16 *) NULL;
static pixel32 *global_shading_table32= (pixel32 *) NULL;

short number_of_shading_tables, shading_table_fractional_bits, shading_table_size;

// LP addition: opened-shapes-file object
static OpenedFile ShapesFile;

/* ---------- private prototypes */

static void update_color_environment(void);
static short find_or_add_color(struct rgb_color_value *color, struct rgb_color_value *colors, short *color_count);
static void build_shading_tables(struct rgb_color_value *colors, short count);
static pixel8 find_closest_match(struct rgb_color *match, struct rgb_color_value *colors, short count);
static void _change_clut(void (*change_clut_proc)(struct color_table *color_table), struct rgb_color_value *colors, short color_count);

static void build_shading_tables8(struct rgb_color_value *colors, short color_count, pixel8 *shading_tables);
static void build_shading_tables16(struct rgb_color_value *colors, short color_count, pixel16 *shading_tables, byte *remapping_table);
static void build_shading_tables32(struct rgb_color_value *colors, short color_count, pixel32 *shading_tables, byte *remapping_table);
static void build_global_shading_table16(void);
static void build_global_shading_table32(void);

static boolean get_next_color_run(struct rgb_color_value *colors, short color_count, short *start, short *count);
static boolean new_color_run(struct rgb_color_value *_new, struct rgb_color_value *last);

static long get_shading_table_size(short collection_code);

static void build_collection_tinting_table(struct rgb_color_value *colors, short color_count, short collection_index);
static void build_tinting_table8(struct rgb_color_value *colors, short color_count, pixel8 *tint_table, short tint_start, short tint_count);
static void build_tinting_table16(struct rgb_color_value *colors, short color_count, pixel16 *tint_table, struct rgb_color *tint_color);
static void build_tinting_table32(struct rgb_color_value *colors, short color_count, pixel32 *tint_table, struct rgb_color *tint_color);

static void precalculate_bit_depth_constants(void);

static boolean collection_loaded(struct collection_header *header);
static void unload_collection(struct collection_header *header);
static void unlock_collection(struct collection_header *header);
static void lock_collection(struct collection_header *header);
static boolean load_collection(short collection_index, boolean strip);

static void debug_shapes_memory(void);

static void shutdown_shape_handler(void);
static void close_shapes_file(void);

static byte *read_object_from_file(OpenedFile& OFile, long offset, long length);

// static byte *make_stripped_collection(byte *collection);

/* --------- collection accessor prototypes */

// Modified to return NULL for unloaded collections and out-of-range indices for collection contents.
// This is to allow for more graceful degradation.

static struct collection_header *get_collection_header(short collection_index);
static struct collection_definition *get_collection_definition(short collection_index);
static void *get_collection_shading_tables(short collection_index, short clut_index);
static void *get_collection_tint_tables(short collection_index, short tint_index);
// static void *collection_offset(struct collection_definition *definition, long offset);
static struct rgb_color_value *get_collection_colors(short collection_index, short clut_number);
static struct high_level_shape_definition *get_high_level_shape_definition(short collection_index, short high_level_shape_index);
static struct low_level_shape_definition *get_low_level_shape_definition(short collection_index, short low_level_shape_index);
static struct bitmap_definition *get_bitmap_definition(short collection_index, short bitmap_index);

/* ---------- machine-specific code */

#ifdef mac
#include "shapes_macintosh.cpp"
#elif defined(SDL)
#include "shapes_sdl.cpp"
#endif

/* ---------- code */

/* --------- private code */

void initialize_shape_handler()
{
	// M1 uses the resource fork, but M2 and Moo use the data fork

	FileSpecifier File;
	get_default_shapes_spec(File);
	open_shapes_file(File);
	if (!ShapesFile.IsOpen())
		alert_user(fatalError, strERRORS, badExtraFileLocations, ShapesFile.GetError());
	else
		atexit(shutdown_shape_handler);
	
	initialize_pixmap_handler();
}

void open_shapes_file(FileSpecifier& File)
{
	if (File.Open(ShapesFile))
	{
		// Load the collection headers;
		// need a buffer for the packed data
		int Size = MAXIMUM_COLLECTIONS*SIZEOF_collection_header;
		byte *CollHdrStream = new byte[Size];
		if (!ShapesFile.Read(Size,CollHdrStream))
		{
			ShapesFile.Close();
			delete []CollHdrStream;
			return;
		}
		
		// Unpack them
		uint8 *S = CollHdrStream;
		int Count = MAXIMUM_COLLECTIONS;
		collection_header* ObjPtr = collection_headers;
		
		for (int k = 0; k < Count; k++, ObjPtr++)
		{
			StreamToValue(S,ObjPtr->status);
			StreamToValue(S,ObjPtr->flags);
			
			StreamToValue(S,ObjPtr->offset);
			StreamToValue(S,ObjPtr->length);
			StreamToValue(S,ObjPtr->offset16);
			StreamToValue(S,ObjPtr->length16);
			
			S += 6*2;
			
			ObjPtr->Definition = NULL;	// so unloading can work properly
		}
		
		assert((S - CollHdrStream) == Count*SIZEOF_collection_header);
		
		delete []CollHdrStream;
	}
}

static void close_shapes_file(void)
{
	ShapesFile.Close();
}

static void shutdown_shape_handler(void)
{
	close_shapes_file();
}


static boolean load_collection(
	short collection_index,
	boolean strip)
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
	
	// Set all the memory to unused
	header->Definition = NULL;
	header->Colors = NULL;
	header->Sequences = NULL;
	header->Frames = NULL;
	header->Bitmaps = NULL;
	
	if (collection)
	{
		// Unpack the collection header first, of course
		assert(SIZEOF_collection_definition <= length);
		
		header->Definition = new collection_definition;
		collection_definition& Definition = *header->Definition;
		
		uint8 *SBase = collection;
		uint8 *S = collection;
		
		StreamToValue(S,Definition.version);
		
		StreamToValue(S,Definition.type);
		StreamToValue(S,Definition.flags);
		
		StreamToValue(S,Definition.color_count);
		StreamToValue(S,Definition.clut_count);
		StreamToValue(S,Definition.color_table_offset);
		
		StreamToValue(S,Definition.high_level_shape_count);
		StreamToValue(S,Definition.high_level_shape_offset_table_offset);
		
		StreamToValue(S,Definition.low_level_shape_count);
		StreamToValue(S,Definition.low_level_shape_offset_table_offset);
		
		StreamToValue(S,Definition.bitmap_count);
		StreamToValue(S,Definition.bitmap_offset_table_offset);
		
		StreamToValue(S,Definition.pixels_to_world);
		
		StreamToValue(S,Definition.size);
		
		S += 253*2;
		assert((S - SBase) == SIZEOF_collection_definition);
		
		// Get the colors		
		assert(Definition.color_count >= 0);
		assert(Definition.clut_count >= 0);
		int TotalColors = Definition.color_count * Definition.clut_count;
		header->Colors = new rgb_color_value[TotalColors];
		
		assert(Definition.color_table_offset >= 0);
		assert(Definition.color_table_offset + TotalColors*SIZEOF_rgb_color_value <= length);
		rgb_color_value *Colors = header->Colors;
		SBase = S = collection + Definition.color_table_offset;
		for (int k = 0; k < TotalColors; k++, Colors++)
		{
			Colors->flags = *(S++);
			Colors->value = *(S++);
			StreamToValue(S,Colors->red);
			StreamToValue(S,Colors->green);
			StreamToValue(S,Colors->blue);
		}
		assert((S - SBase) == TotalColors*SIZEOF_rgb_color_value);
		
		// Pointers will be cleared unless allocated 
		assert(Definition.high_level_shape_count >= 0);
		header->Sequences = new high_level_shape_definition *[Definition.high_level_shape_count];
		objlist_clear(header->Sequences,Definition.high_level_shape_count);
		
		// Need the sequence offsets here;
		// these will be used to indicate how much to read per sequence,
		// so put the next-data offset at the end
		assert(Definition.high_level_shape_offset_table_offset >= 0);
		assert(Definition.high_level_shape_offset_table_offset +
			Definition.high_level_shape_count*sizeof(int32) <= length);
		int32 *SeqOffsets = new int32[Definition.high_level_shape_count + 1];
		S = collection + Definition.high_level_shape_offset_table_offset;
		StreamToList(S,SeqOffsets,Definition.high_level_shape_count);
		SeqOffsets[Definition.high_level_shape_count] = Definition.low_level_shape_offset_table_offset;
		
		assert(SeqOffsets[0] >= 0);
		for (int k=0; k<Definition.high_level_shape_count; k++)
			assert(SeqOffsets[k+1] - SeqOffsets[k] >= SIZEOF_high_level_shape_definition);
		assert(SeqOffsets[Definition.high_level_shape_count] <= length);
		
		for (int k=0; k<Definition.high_level_shape_count; k++)
		{
			SBase = S = collection + SeqOffsets[k];
			uint8 *SNext = collection + SeqOffsets[k+1];
			
			long NewSize = (SNext - S) +
				(sizeof(high_level_shape_definition) - SIZEOF_high_level_shape_definition);
			header->Sequences[k] = (high_level_shape_definition *)(new byte[NewSize]);
			
			high_level_shape_definition& Sequence = *header->Sequences[k];
			
			StreamToValue(S,Sequence.type);
			StreamToValue(S,Sequence.flags);
			
			StreamToBytes(S,Sequence.name,HIGH_LEVEL_SHAPE_NAME_LENGTH+2);
			
			StreamToValue(S,Sequence.number_of_views);
			
			StreamToValue(S,Sequence.frames_per_view);
			StreamToValue(S,Sequence.ticks_per_frame);
			StreamToValue(S,Sequence.key_frame);
			
			StreamToValue(S,Sequence.transfer_mode);
			StreamToValue(S,Sequence.transfer_mode_period);
			
			StreamToValue(S,Sequence.first_frame_sound);
			StreamToValue(S,Sequence.key_frame_sound);
			StreamToValue(S,Sequence.last_frame_sound);
			
			StreamToValue(S,Sequence.pixels_to_world);
			
			StreamToValue(S,Sequence.loop_frame);
			
			S += 14*2;
			
			StreamToValue(S,Sequence.low_level_shape_indexes[0]);
			assert((S - SBase) == SIZEOF_high_level_shape_definition);
			
			// Do the remaining frame indices
			long NumFrameIndxs = (SNext - S)/2;
			StreamToList(S,Sequence.low_level_shape_indexes+1,NumFrameIndxs);
			
			// Check if it is necessary to allocate some extra space;
			// number_of_views type 2 (_animated2to8) is rather screwy.
			// In some shapes files, it has two views associated with it,
			// while the code says it ought to have 8 views
			int TrueNumViews;
			switch (Sequence.number_of_views)
			{
				case _unanimated:
				case _animated1:
					TrueNumViews = 1;
					break;
				
				case _animated3to4: /* front, quarter and side views only */
				case _animated4:
					TrueNumViews = 4;
					break;
					
				case _animated3to5:
				case _animated5:
					TrueNumViews = 5;
					break;
				
				case _animated2to8:
				case _animated5to8:
				case _animated8:
					TrueNumViews = 8;
					break;
					
				default:
					vassert(false,csprintf(temporary,"Unrecognized view-count type: %d",Sequence.number_of_views));
			}
			
			int TrueNumFrameIndxs = TrueNumViews*Sequence.frames_per_view;
			if (NumFrameIndxs < TrueNumFrameIndxs)
			{
				int OldSize = (S - SBase);
				int NewSize = OldSize + sizeof(short)*(TrueNumFrameIndxs - NumFrameIndxs);
				byte *NewSequence = new byte[NewSize];
				memcpy(NewSequence,header->Sequences[k],OldSize);
				memset(NewSequence+OldSize,0,NewSize-OldSize);
				delete []header->Sequences[k];
				header->Sequences[k] = (high_level_shape_definition *)NewSequence;
			}
		}
		
		delete []SeqOffsets;
		
		if (strip)
		{
			Definition.low_level_shape_count = 0;
			Definition.bitmap_count = 0;
		}
		else
		{
			// Get the frame offsets
			assert(Definition.low_level_shape_count >= 0);
			assert(Definition.low_level_shape_offset_table_offset >= 0);
			assert(Definition.low_level_shape_offset_table_offset +
				Definition.low_level_shape_count*sizeof(int32) <= length);
			int32 *FrmOffsets = new int32[Definition.low_level_shape_count + 1];
			S = collection + Definition.low_level_shape_offset_table_offset;
			StreamToList(S,FrmOffsets,Definition.low_level_shape_count);
			FrmOffsets[Definition.low_level_shape_count] = Definition.bitmap_offset_table_offset;
			header->Frames = new low_level_shape_definition[Definition.low_level_shape_count];
			
			assert(FrmOffsets[0] >= 0);
			for (int k=0; k<Definition.low_level_shape_count; k++)
				assert(FrmOffsets[k+1] - FrmOffsets[k] >= SIZEOF_low_level_shape_definition);
			assert(FrmOffsets[Definition.low_level_shape_count] <= length);
			
			for (int k=0; k<Definition.low_level_shape_count; k++)
			{
				SBase = S = collection + FrmOffsets[k];
				uint8 *SNext = collection + FrmOffsets[k+1];
						
				low_level_shape_definition& Frame = header->Frames[k];
				
				StreamToValue(S,Frame.flags);
				
				StreamToValue(S,Frame.minimum_light_intensity);
				
				StreamToValue(S,Frame.bitmap_index);
				
				StreamToValue(S,Frame.origin_x);
				StreamToValue(S,Frame.origin_y);
				
				StreamToValue(S,Frame.key_x);
				StreamToValue(S,Frame.key_y);
				
				StreamToValue(S,Frame.world_left);
				StreamToValue(S,Frame.world_right);
				StreamToValue(S,Frame.world_top);
				StreamToValue(S,Frame.world_bottom);
				StreamToValue(S,Frame.world_x0);
				StreamToValue(S,Frame.world_y0);
				
				S += 4*2;
				
				assert((S - SBase) == SIZEOF_low_level_shape_definition);
			}
			
			delete []FrmOffsets;
			
			// Need the bitmap offsets here
			assert(Definition.bitmap_count >= 0);
			header->Bitmaps = new bitmap_definition *[Definition.bitmap_count];
			objlist_clear(header->Bitmaps,Definition.bitmap_count);
			
			assert(Definition.bitmap_offset_table_offset >= 0);
			assert(Definition.bitmap_offset_table_offset +
				Definition.bitmap_count*sizeof(int32) <= length);
			long *BtmpOffsets = new long[Definition.bitmap_count + 1];
			S = collection + Definition.bitmap_offset_table_offset;
			StreamToList(S,BtmpOffsets,Definition.bitmap_count);
			BtmpOffsets[Definition.bitmap_count] = length;
			
			assert(BtmpOffsets[0] >= 0);
			for (int k=0; k<Definition.bitmap_count; k++)
				assert(BtmpOffsets[k+1] - BtmpOffsets[k] >= SIZEOF_bitmap_definition);
			assert(BtmpOffsets[Definition.bitmap_count] <= length);
			
			for (int k=0; k<Definition.bitmap_count; k++)
			{
				SBase = S = collection + BtmpOffsets[k];
				uint8 *SNext = collection + BtmpOffsets[k+1];
				
				long NewSize = (SNext - S) +
					(sizeof(bitmap_definition) - SIZEOF_bitmap_definition);
				header->Bitmaps[k] = (bitmap_definition *)(new byte[NewSize]);
				assert(header->Bitmaps[k]);
				
				bitmap_definition& Bitmap = *header->Bitmaps[k];
				
				StreamToValue(S,Bitmap.width);
				StreamToValue(S,Bitmap.height);
				StreamToValue(S,Bitmap.bytes_per_row);
				
				StreamToValue(S,Bitmap.flags);
				StreamToValue(S,Bitmap.bit_depth);
				
				S += 8*2;
				
				// Code assumes 32-bit pointers!
				S += sizeof(pixel8 *);
				
				assert((S - SBase) == SIZEOF_bitmap_definition);
				
				// Do all the other stuff; don't bother to try to process it
				long RemainingBytes = (SNext - S);
				StreamToBytes(S,Bitmap.row_addresses+1,RemainingBytes);
			}
			
			delete []BtmpOffsets;
			
			// No longer needed!
			delete []collection;
		}
		
		// header->collection= (collection_definition *) collection;
		
		/* allocate enough space for this collection’s shading tables */
		if (strip)
		{
			shading_tables = NULL;
		}
		else
		{
			shading_tables = new byte[get_shading_table_size(collection_index)*Definition.clut_count +
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
//		vhalt(csprintf(temporary, "couldn’t load collection #%d (error==#%d)", collection_index, error));
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
	// if (header->collection) delete []header->collection;
	// header->collection = NULL;
	if (header->Definition)
	{
		collection_definition& Definition = *header->Definition;
		
		// Delete the other stuff first, since it needs the header size
		if (header->Colors)
		{
			delete []header->Colors;
			header->Colors = NULL;
		}
		if (header->Sequences)
		{
			for (int k=0; k<Definition.high_level_shape_count; k++)
				if (header->Sequences[k]) delete []header->Sequences[k];
			delete []header->Sequences;
			header->Sequences = NULL;
		}
		if (header->Frames)
		{
			delete []header->Frames;
			header->Frames = NULL;
		}
		if (header->Bitmaps)
		{
			for (int k=0; k<Definition.bitmap_count; k++)
				if (header->Bitmaps[k]) delete []header->Bitmaps[k];
			delete []header->Bitmaps;
			header->Bitmaps = NULL;
		}
		
		delete header->Definition;
		header->Definition = NULL;
	}
	if (header->shading_tables) delete []header->shading_tables;
	header->shading_tables = NULL;
	
	return;
}

/*
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
*/

static boolean collection_loaded(
	struct collection_header *header)
{
	return header->Definition ? TRUE : FALSE;
	// return header->collection ? TRUE : FALSE;
}


static void lock_collection(
	struct collection_header *header)
{
	// nothing to do
}

static void unlock_collection(
	struct collection_header *header)
{
	// nothing to do
}

static byte *read_object_from_file(
	OpenedFile& OFile,
	long offset,
	long length)
{
	if (!OFile.IsOpen()) return NULL;
	
	byte *data = NULL;
	
	if (length <= 0) return NULL;
	if (!(data = new byte[length])) return NULL;
	
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
	
	return data;
}


void unload_all_collections(
	void)
{
	struct collection_header *header;
	short collection_index;
	
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if (collection_loaded(header))
		{
			unload_collection(header);
		}
	}
	
	return;
}

void mark_collection(
	short collection_code,
	boolean loading)
{
	if (collection_code!=NONE)
	{
		short clut_index= GET_COLLECTION_CLUT(collection_code);
		short collection_index= GET_COLLECTION(collection_code);
	
		assert(collection_index>=0&&collection_index<MAXIMUM_COLLECTIONS);
		collection_headers[collection_index].status|= loading ? markLOAD : markUNLOAD;
	}
	
	return;
}

void strip_collection(
	short collection_code)
{
	if (collection_code!=NONE)
	{
		short collection_index= GET_COLLECTION(collection_code);
	
		assert(collection_index>=0&&collection_index<MAXIMUM_COLLECTIONS);
		collection_headers[collection_index].status|= markSTRIP;
	}
	
	return;
}

/* returns count, doesn’t fill NULL buffer */
short get_shape_descriptors(
	short shape_type,
	shape_descriptor *buffer)
{
	short collection_index, low_level_shape_index;
	short appropriate_type;
	short count;
	
	switch (shape_type)
	{
		case _wall_shape: appropriate_type= _wall_collection; break;
		case _floor_or_ceiling_shape: appropriate_type= _wall_collection; break;
		default:
			// LP change:
			assert(false);
			// halt();
	}

	count= 0;
	for (collection_index=0;collection_index<MAXIMUM_COLLECTIONS;++collection_index)
	{
		struct collection_definition *collection= get_collection_definition(collection_index);
		// Skip over nonexistent collections, frames, and bitmaps.
		if (!collection) continue;
		
		if (collection&&collection->type==appropriate_type)
		{
			for (low_level_shape_index=0;low_level_shape_index<collection->low_level_shape_count;++low_level_shape_index)
			{
				struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
				if (!low_level_shape) continue;
				struct bitmap_definition *bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
				if (!bitmap) continue;
				
				count+= collection->clut_count;
				if (buffer)
				{
					short clut;
				
					for (clut=0;clut<collection->clut_count;++clut)
					{
						*buffer++= BUILD_DESCRIPTOR(BUILD_COLLECTION(collection_index, clut), low_level_shape_index);
					}
				}
			}
		}
	}
	
	return count;
}

void extended_get_shape_bitmap_and_shading_table(
	short collection_code,
	short low_level_shape_index,
	struct bitmap_definition **bitmap,
	void **shading_tables,
	short shading_mode)
{
//	if (collection_code==_collection_marathon_control_panels) collection_code= 30, low_level_shape_index= 0;
	short collection_index= GET_COLLECTION(collection_code);
	short clut_index= GET_COLLECTION_CLUT(collection_code);
	struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	// Return NULL pointers for bitmap and shading table if the frame does not exist
	if (!low_level_shape)
	{
		*bitmap = NULL;
		if (shading_tables) *shading_tables = NULL;
		return;
	}
	
	if (bitmap) *bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);
	if (shading_tables)
	{
		switch (shading_mode)
		{
			case _shading_normal:
				*shading_tables= get_collection_shading_tables(collection_index, clut_index);
				break;
			case _shading_infravision:
				*shading_tables= get_collection_tint_tables(collection_index, 0);
				break;
			
			default:
				// LP change:
				assert(false);
				// halt();
		}
	}

	return;
}

struct shape_information_data *extended_get_shape_information(
	short collection_code,
	short low_level_shape_index)
{
	short collection_index= GET_COLLECTION(collection_code);
	struct low_level_shape_definition *low_level_shape;
	struct collection_definition *collection;

	// collection= get_collection_definition(collection_index);
	low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);

	/* this will be removed when it’s calculated in the extractor */
#ifdef OBSOLETE
	{
		struct bitmap_definition *bitmap= get_bitmap_definition(collection_index, low_level_shape->bitmap_index);

		low_level_shape->world_left= (-low_level_shape->origin_x)*collection->pixels_to_world;
		low_level_shape->world_top= - (-low_level_shape->origin_y)*collection->pixels_to_world;
		low_level_shape->world_right= (bitmap->width-low_level_shape->origin_x)*collection->pixels_to_world;
		low_level_shape->world_bottom= - (bitmap->height-low_level_shape->origin_y)*collection->pixels_to_world;
		low_level_shape->world_x0= (low_level_shape->key_x-low_level_shape->origin_x)*collection->pixels_to_world;
		low_level_shape->world_y0= - (low_level_shape->key_y-low_level_shape->origin_y)*collection->pixels_to_world;
	}
#endif

	return (struct shape_information_data *) low_level_shape;
}

void process_collection_sounds(
	short collection_code,
	void (*process_sound)(short sound_index))
{
	short collection_index= GET_COLLECTION(collection_code);
	struct collection_definition *collection= get_collection_definition(collection_index);
	// Skip over processing unloaded collections and sequences
	if (!collection) return;
	
	short high_level_shape_index;
	
	for (high_level_shape_index= 0; high_level_shape_index<collection->high_level_shape_count; ++high_level_shape_index)
	{
		struct high_level_shape_definition *high_level_shape= get_high_level_shape_definition(collection_index, high_level_shape_index);
		if (!high_level_shape) return;
		
		process_sound(high_level_shape->first_frame_sound);
		process_sound(high_level_shape->key_frame_sound);
		process_sound(high_level_shape->last_frame_sound);
	}
	
	return;
}

struct shape_animation_data *get_shape_animation_data(
	shape_descriptor shape)
{
	short collection_index, high_level_shape_index;
	struct high_level_shape_definition *high_level_shape;

	collection_index= GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(shape));
	high_level_shape_index= GET_DESCRIPTOR_SHAPE(shape);
	high_level_shape= get_high_level_shape_definition(collection_index, high_level_shape_index);
	if (!high_level_shape) return NULL;
	
	return (struct shape_animation_data *) &high_level_shape->number_of_views;
}

void *get_global_shading_table(
	void)
{
	void *shading_table= (void *) NULL;

	switch (bit_depth)
	{
		case 8:
		{
			/* return the last shading_table calculated */
			short collection_index;
		
			for (collection_index=MAXIMUM_COLLECTIONS-1;collection_index>=0;--collection_index)
			{
				struct collection_definition *collection= get_collection_definition(collection_index);
				
				if (collection)
				{
					shading_table= get_collection_shading_tables(collection_index, 0);
					break;
				}
			}
			
			break;
		}
		
		case 16:
			build_global_shading_table16();
			shading_table= global_shading_table16;
			break;
		
		case 32:
			build_global_shading_table32();
			shading_table= global_shading_table32;
			break;
		
		default:
			// LP change:
			assert(false);
			// halt();
	}
	assert(shading_table);
	
	return shading_table;
}

void load_collections(
	void)
{
	struct collection_header *header;
	short collection_index;
	
	precalculate_bit_depth_constants();
	
	free_and_unlock_memory(); /* do our best to get a big, unfragmented heap */
	
	/* first go through our list of shape collections and dispose of any collections which
		were marked for unloading.  at the same time, unlock all those collections which
		will be staying (so the heap can move around) */
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if ((header->status&markUNLOAD) && !(header->status&markLOAD))
		{
			if (collection_loaded(header))
			{
				unload_collection(header);
			}
		}
		else
		{
			/* if this collection is already loaded, unlock it to tenderize the heap */
			if (collection_loaded(header))
			{
				unlock_collection(header);
			}
		}
	}
	
	/* ... then go back through the list of collections and load any that we were asked to */
	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		/* don’t reload collections which are already in memory, but do lock them */
		if (collection_loaded(header))
		{
			lock_collection(header);
		}
		else
		{
			if (header->status&markLOAD)
			{
				/* load and decompress collection */
				if (!load_collection(collection_index, (header->status&markSTRIP) ? TRUE : FALSE))
				{
					alert_user(fatalError, strERRORS, outOfMemory, -1);
				}
			}
		}
		
		/* clear action flags */
		header->status= markNONE;
		header->flags= 0;
	}
	
	/* remap the shapes, recalculate row base addresses, build our new world color table and
		(finally) update the screen to reflect our changes */
	update_color_environment();

#ifdef DEBUG
//	debug_shapes_memory();
#endif

	return;
}

/* ---------- private code */

static void precalculate_bit_depth_constants(
	void)
{
	switch (bit_depth)
	{
		case 8:
			number_of_shading_tables= 32;
			shading_table_fractional_bits= 5;
//			next_shading_table_shift= 8;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel8);
			break;
		case 16:
			number_of_shading_tables= 64;
			shading_table_fractional_bits= 6;
//			next_shading_table_shift= 9;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel16);
			break;
		case 32:
			number_of_shading_tables= 256;
			shading_table_fractional_bits= 8;
//			next_shading_table_shift= 10;
			shading_table_size= PIXEL8_MAXIMUM_COLORS*sizeof(pixel32);
			break;
	}
	
	return;
}

/* given a list of RGBColors, find out which one, if any, match the given color.  if there
	aren’t any matches, add a new entry and return that index. */
static short find_or_add_color(
	struct rgb_color_value *color,
	register struct rgb_color_value *colors,
	short *color_count)
{
	short i;
	
	// LP addition: save initial color-table pointer, just in case we overflow
	register struct rgb_color_value *colors_saved = colors;
	
	// = 1 to skip the transparent color
	for (i= 1, colors+= 1; i<*color_count; ++i, ++colors)
	{
		if (colors->red==color->red && colors->green==color->green && colors->blue==color->blue)
		{
			colors->flags= color->flags;
			return i;
		}
	}
	
	// LP change: added a fallback strategy; if there were too many colors,
	// then find the closest one
	if (*color_count >= PIXEL8_MAXIMUM_COLORS)
	{
		// Set up minimum distance, its index
		// Strictly speaking, the distance squared, since that will be
		// what we will be calculating.
		// The color values are data type "word", which is unsigned short;
		// this explains the initial choice of minimum value --
		// as greater than any possible such value.
		double MinDiffSq = 3*double(65536)*double(65536);
		short MinIndx = 0;
		
		// Rescan
		colors = colors_saved;
		for (i= 1, colors+= 1; i<*color_count; ++i, ++colors)
		{
			double RedDiff = double(color->red) - double(colors->red);
			double GreenDiff = double(color->green) - double(colors->green);
			double BlueDiff = double(color->blue) - double(colors->blue);
			double DiffSq = RedDiff*RedDiff + GreenDiff*GreenDiff + BlueDiff*BlueDiff;
			if (DiffSq < MinDiffSq)
			{
				MinIndx = i;
				MinDiffSq = DiffSq;
			}
		}
		return MinIndx;
	}
	
	// assert(*color_count<PIXEL8_MAXIMUM_COLORS);
	*colors= *color;
	
	return (*color_count)++;
}

static void update_color_environment(
	void)
{
	short color_count;
	short collection_index;
	short bitmap_index;
	
	pixel8 remapping_table[PIXEL8_MAXIMUM_COLORS];
	struct rgb_color_value colors[PIXEL8_MAXIMUM_COLORS];

	memset(remapping_table, 0, PIXEL8_MAXIMUM_COLORS*sizeof(pixel8));

	// dummy color to hold the first index (zero) for transparent pixels
	colors[0].red= colors[0].green= colors[0].blue= 65535;
	colors[0].flags= colors[0].value= 0;
	color_count= 1;

	/* loop through all collections, only paying attention to the loaded ones.  we’re
		depending on finding the gray run (white to black) first; so it’s the responsibility
		of the lowest numbered loaded collection to give us this */
	for (collection_index=0;collection_index<MAXIMUM_COLLECTIONS;++collection_index)
	{
		struct collection_definition *collection= get_collection_definition(collection_index);

//		dprintf("collection #%d", collection_index);
		
		if (collection && collection->bitmap_count)
		{
			struct rgb_color_value *primary_colors= get_collection_colors(collection_index, 0)+NUMBER_OF_PRIVATE_COLORS;
			assert(primary_colors);
			short color_index, clut_index;

//			if (collection_index==15) dprintf("primary clut %p", primary_colors);
//			dprintf("primary clut %d entries;dm #%d #%d", collection->color_count, primary_colors, collection->color_count*sizeof(ColorSpec));

			/* add the colors from this collection’s primary color table to the aggregate color
				table and build the remapping table */
			for (color_index=0;color_index<collection->color_count-NUMBER_OF_PRIVATE_COLORS;++color_index)
			{
				primary_colors[color_index].value= remapping_table[primary_colors[color_index].value]= 
					find_or_add_color(&primary_colors[color_index], colors, &color_count);
			}
			
			/* then remap the collection and recalculate the base addresses of each bitmap */
			for (bitmap_index= 0; bitmap_index<collection->bitmap_count; ++bitmap_index)
			{
				struct bitmap_definition *bitmap= get_bitmap_definition(collection_index, bitmap_index);
				assert(bitmap);
				
				/* calculate row base addresses ... */
				bitmap->row_addresses[0]= calculate_bitmap_origin(bitmap);
				precalculate_bitmap_row_addresses(bitmap);

				/* ... and remap it */
				remap_bitmap(bitmap, remapping_table);
			}
			
			/* build a shading table for each clut in this collection */
			for (clut_index= 0; clut_index<collection->clut_count; ++clut_index)
			{
				void *primary_shading_table= get_collection_shading_tables(collection_index, 0);
				short collection_bit_depth= collection->type==_interface_collection ? 8 : bit_depth;

				if (clut_index)
				{
					struct rgb_color_value *alternate_colors= get_collection_colors(collection_index, clut_index)+NUMBER_OF_PRIVATE_COLORS;
					assert(alternate_colors);
					void *alternate_shading_table= get_collection_shading_tables(collection_index, clut_index);
					pixel8 shading_remapping_table[PIXEL8_MAXIMUM_COLORS];
					
					memset(shading_remapping_table, 0, PIXEL8_MAXIMUM_COLORS*sizeof(pixel8));
					
//					dprintf("alternate clut %d entries;dm #%d #%d", collection->color_count, alternate_colors, collection->color_count*sizeof(ColorSpec));
					
					/* build a remapping table for the primary shading table which we can use to
						calculate this alternate shading table */
					for (color_index= 0; color_index<PIXEL8_MAXIMUM_COLORS; ++color_index) shading_remapping_table[color_index]= color_index;
					for (color_index= 0; color_index<collection->color_count-NUMBER_OF_PRIVATE_COLORS; ++color_index)
					{
						shading_remapping_table[find_or_add_color(&primary_colors[color_index], colors, &color_count)]= 
							find_or_add_color(&alternate_colors[color_index], colors, &color_count);
					}
//					shading_remapping_table[iBLACK]= iBLACK; /* make iBLACK==>iBLACK remapping explicit */

					switch (collection_bit_depth)
					{
						case 8:
							/* duplicate the primary shading table and remap it */
							memcpy(alternate_shading_table, primary_shading_table, get_shading_table_size(collection_index));
							map_bytes((unsigned char *)alternate_shading_table, shading_remapping_table, get_shading_table_size(collection_index));
							break;
						
						case 16:
							build_shading_tables16(colors, color_count, (pixel16 *)alternate_shading_table, shading_remapping_table); break;
							break;
						
						case 32:
							build_shading_tables32(colors, color_count, (pixel32 *)alternate_shading_table, shading_remapping_table); break;
							break;
						
						default:
							// LP change:
							assert(false);
							// halt();
					}
				}
				else
				{
					/* build the primary shading table */
					switch (collection_bit_depth)
					{
						case 8: build_shading_tables8(colors, color_count, (unsigned char *)primary_shading_table); break;
						case 16: build_shading_tables16(colors, color_count, (pixel16 *)primary_shading_table, (byte *) NULL); break;
						case 32: build_shading_tables32(colors, color_count,  (pixel32 *)primary_shading_table, (byte *) NULL); break;
						default:
							// LP change:
							assert(false);
							// halt();
					}
				}
			}
			
			build_collection_tinting_table(colors, color_count, collection_index);
			
			/* 8-bit interface, non-8-bit main window; remember interface CLUT separately */
			if (collection_index==_collection_interface && interface_bit_depth==8 && bit_depth!=interface_bit_depth) _change_clut(change_interface_clut, colors, color_count);
			
			/* if we’re not in 8-bit, we don’t have to carry our colors over into the next collection */
			if (bit_depth!=8) color_count= 1;
		}
	}

#ifdef DEBUG
//	dump_colors(colors, color_count);
#endif

	/* change the screen clut and rebuild our shading tables */
	_change_clut(change_screen_clut, colors, color_count);

	return;
}

static void _change_clut(
	void (*change_clut_proc)(struct color_table *color_table),
	struct rgb_color_value *colors,
	short color_count)
{
	struct color_table color_table;
	struct rgb_color *color;
	short i;
	
	color= color_table.colors;
	color_table.color_count= PIXEL8_MAXIMUM_COLORS;
	for (i= 0; i<color_count; ++i, ++color, ++colors)
	{
		*color= *((struct rgb_color *)&colors->red);
	}
	for (i= color_count; i<PIXEL8_MAXIMUM_COLORS; ++i, ++color)
	{
		color->red= color->green= color->blue= 0;
	}
	change_clut_proc(&color_table);
	
	return;
}

#ifndef SCREAMING_METAL
static void build_shading_tables8(
	struct rgb_color_value *colors,
	short color_count,
	pixel8 *shading_tables)
{
	short i;
	short start, count, level, value;
	
	objlist_set(shading_tables, iBLACK, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			short adjust= start ? 1 : 0;

			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + start + i;
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? (level>>1) : level;

				value= i + (multiplier*(count+adjust-i))/(number_of_shading_tables-1);
				if (value>=count) value= iBLACK; else value= start+value;
				shading_tables[PIXEL8_MAXIMUM_COLORS*(number_of_shading_tables-level-1)+start+i]= value;
			}
		}
	}

	return;
}
#else
short find_closest_color(
	struct rgb_color *color,
	register struct rgb_color *colors,
	short color_count)
{
	short i;
	long closest_delta= LONG_MAX;
	short closest_index= 0;
	
	// = 1 to skip the transparent color
	for (i= 1, colors+= 1; i<color_count; ++i, ++colors)
	{
		long delta= (long)ABS(colors->red-color->red) +
			(long)ABS(colors->green-color->green) +
			(long)ABS(colors->blue-color->blue);
		
		if (delta<closest_delta) closest_index= i, closest_delta= delta;
	}

	return closest_index;
}

static void build_shading_tables8(
	struct rgb_color *colors,
	short color_count,
	pixel8 *shading_tables)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, iBLACK, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color *color= colors + start + i;
				RGBColor result;
				
				result.red= (color->red*level)/(number_of_shading_tables-1);
				result.green= (color->green*level)/(number_of_shading_tables-1);
				result.blue= (color->blue*level)/(number_of_shading_tables-1);
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]=
					find_closest_color(&result, colors, color_count);
			}
		}
	}

	return;
}
#endif

static void build_shading_tables16(
	struct rgb_color_value *colors,
	short color_count,
	pixel16 *shading_tables,
	byte *remapping_table)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, 0, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i=0;i<count;++i)
		{
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + (remapping_table ? remapping_table[start+i] : (start+i));
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? ((number_of_shading_tables>>1)+(level>>1)) : level;
				
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
					RGBCOLOR_TO_PIXEL16((color->red*multiplier)/(number_of_shading_tables-1),
						(color->green*multiplier)/(number_of_shading_tables-1),
						(color->blue*multiplier)/(number_of_shading_tables-1));
			}
		}
	}

	return;
}

static void build_shading_tables32(
	struct rgb_color_value *colors,
	short color_count,
	pixel32 *shading_tables,
	byte *remapping_table)
{
	short i;
	short start, count, level;
	
	objlist_set(shading_tables, 0, PIXEL8_MAXIMUM_COLORS);
	
	start= 0, count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		for (i= 0; i<count; ++i)
		{
			for (level= 0; level<number_of_shading_tables; ++level)
			{
				struct rgb_color_value *color= colors + (remapping_table ? remapping_table[start+i] : (start+i));
				short multiplier= (color->flags&SELF_LUMINESCENT_COLOR_FLAG) ? ((number_of_shading_tables>>1)+(level>>1)) : level;
				
				shading_tables[PIXEL8_MAXIMUM_COLORS*level+start+i]= 
					RGBCOLOR_TO_PIXEL32((color->red*multiplier)/(number_of_shading_tables-1),
						(color->green*multiplier)/(number_of_shading_tables-1),
						(color->blue*multiplier)/(number_of_shading_tables-1));
			}
		}
	}

	return;
}

static void build_global_shading_table16(
	void)
{
	if (!global_shading_table16)
	{
		short component, value, shading_table;
		pixel16 *write;
		
		global_shading_table16= (pixel16 *) malloc(sizeof(pixel16)*number_of_shading_tables*NUMBER_OF_COLOR_COMPONENTS*(PIXEL16_MAXIMUM_COMPONENT+1));
		assert(global_shading_table16);
		
		write= global_shading_table16;
		for (shading_table= 0; shading_table<number_of_shading_tables; ++shading_table)
		{
			for (component=0;component<NUMBER_OF_COLOR_COMPONENTS;++component)
			{
				short shift= 5*(NUMBER_OF_COLOR_COMPONENTS-component-1);

				for (value=0;value<=PIXEL16_MAXIMUM_COMPONENT;++value)
				{
					*write++= ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
				}
			}
		}
	}
	
	return;
}

static void build_global_shading_table32(
	void)
{
	if (!global_shading_table32)
	{
		short component, value, shading_table;
		pixel32 *write;
		
		global_shading_table32= (pixel32 *) malloc(sizeof(pixel32)*number_of_shading_tables*NUMBER_OF_COLOR_COMPONENTS*(PIXEL32_MAXIMUM_COMPONENT+1));
		assert(global_shading_table32);
		
		write= global_shading_table32;
		for (shading_table= 0; shading_table<number_of_shading_tables; ++shading_table)
		{
			for (component= 0; component<NUMBER_OF_COLOR_COMPONENTS; ++component)
			{
				short shift= 8*(NUMBER_OF_COLOR_COMPONENTS-component-1);

				for (value= 0; value<=PIXEL32_MAXIMUM_COMPONENT; ++value)
				{
					*write++= ((value*(shading_table))/(number_of_shading_tables-1))<<shift;
				}
			}
		}
	}
	
	return;
}

static boolean get_next_color_run(
	struct rgb_color_value *colors,
	short color_count,
	short *start,
	short *count)
{
	boolean not_done= FALSE;
	struct rgb_color_value last_color;
	
	if (*start+*count<color_count)
	{
		*start+= *count;
		for (*count=0;*start+*count<color_count;*count+= 1)
		{
			if (*count)
			{
				if (new_color_run(colors+*start+*count, &last_color))
				{
					break;
				}
			}
			last_color= colors[*start+*count];
		}
		
		not_done= TRUE;
	}
	
	return not_done;
}

static boolean new_color_run(
	struct rgb_color_value *_new,
	struct rgb_color_value *last)
{
	if ((long)last->red+(long)last->green+(long)last->blue<(long)_new->red+(long)_new->green+(long)_new->blue)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static long get_shading_table_size(
	short collection_code)
{
	short collection_index= GET_COLLECTION(collection_code);
	long size;
	
	switch (bit_depth)
	{
		case 8: size= number_of_shading_tables*shading_table_size; break;
		case 16: size= number_of_shading_tables*shading_table_size; break;
		case 32: size= number_of_shading_tables*shading_table_size; break;
		default:
			// LP change:
			assert(false);
			// halt();
	}
	
	return size;
}

/* --------- light enhancement goggles */

enum /* collection tint colors */
{
	_tint_collection_red,
	_tint_collection_green,
	_tint_collection_blue,
	_tint_collection_yellow,
	NUMBER_OF_TINT_COLORS
};

struct tint_color8_data
{
	short start, count;
};

static struct rgb_color tint_colors16[NUMBER_OF_TINT_COLORS]=
{
	{65535, 0, 0},
	{0, 65535, 0},
	{0, 0, 65535},
	{65535, 65535, 0},
};

static struct tint_color8_data tint_colors8[NUMBER_OF_TINT_COLORS]=
{
	{45, 13},
	{32, 13},
	{96, 13},
	{83, 13},
};


// LP addition to make it more generic;
// the ultimate in this would be to make every collection
// have its own infravision tint.
static short CollectionTints[NUMBER_OF_COLLECTIONS] = 
{
	// Interface
	NONE,
	// Weapons in hand
	_tint_collection_yellow,
	// Juggernaut, tick
	_tint_collection_red,
	_tint_collection_red,
	// Explosion effects
	_tint_collection_yellow,
	// Hunter	
	_tint_collection_red,
	// Player
	_tint_collection_yellow,
	// Items
	_tint_collection_green,
	// Trooper, Pfhor, S'pht'Kr, F'lickta
	_tint_collection_red,
	_tint_collection_red,
	_tint_collection_red,
	_tint_collection_red,
	// Bob and VacBobs
	_tint_collection_yellow,
	_tint_collection_yellow,
	// Enforcer, Drone
	_tint_collection_red,
	_tint_collection_red,
	// S'pht
	_tint_collection_blue,
	// Walls
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Scenery
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Landscape
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	_tint_collection_blue,
	// Cyborg
	_tint_collection_red
};


static void build_collection_tinting_table(
	struct rgb_color_value *colors,
	short color_count,
	short collection_index)
{
	struct collection_definition *collection= get_collection_definition(collection_index);
	if (!collection) return;
	
	void *tint_table= get_collection_tint_tables(collection_index, 0);
	short tint_color;

	/* get the tint color */
	// LP change: look up a table
	tint_color = CollectionTints[collection_index];
	// Idiot-proofing:
	if (tint_color >= NUMBER_OF_TINT_COLORS)
		tint_color = NONE;
	else
		tint_color = MAX(tint_color,NONE);
	/*
	tint_color= NONE;	
	switch (collection->type)
	{
		case _wall_collection: tint_color= _tint_collection_blue; break;
		case _object_collection: tint_color= _tint_collection_red; break;
		case _scenery_collection: tint_color= _tint_collection_green; break;
	}
	switch (collection_index)
	{
		case _collection_weapons_in_hand:
		case _collection_player:
		case _collection_rocket:
		case _collection_civilian:
		// LP change:
		case _collection_civilian_fusion:
		// case _collection_madd:
			tint_color= _tint_collection_yellow;
			break;
		
		case _collection_items:
			tint_color= _tint_collection_green;
			break;
		
		case _collection_compiler:
		case _collection_scenery1:
		case _collection_scenery2:
		case _collection_scenery3:
		case _collection_scenery4:
		case _collection_scenery5:
			tint_color= _tint_collection_blue;
			break;
	}
	*/

	/* build the tint table */	
	if (tint_color!=NONE)
	{
		// LP addition: OpenGL support
		rgb_color &Color = tint_colors16[tint_color];
#ifdef HAVE_OPENGL
		OGL_SetInfravisionTint(collection_index,true,Color.red/65535.0,Color.green/65535.0,Color.blue/65535.0);
#endif
		switch (bit_depth)
		{
			case 8:
				build_tinting_table8(colors, color_count, (unsigned char *)tint_table, tint_colors8[tint_color].start, tint_colors8[tint_color].count);
				break;
			case 16:
				build_tinting_table16(colors, color_count, (pixel16 *)tint_table, tint_colors16+tint_color);
				break;
			case 32:
				build_tinting_table32(colors, color_count, (pixel32 *)tint_table, tint_colors16+tint_color);
				break;
		}
	}
	else
	{
		// LP addition: OpenGL support
#ifdef HAVE_OPENGL
		OGL_SetInfravisionTint(collection_index,false,1,1,1);
#endif
	}
	
	return;
}

static void build_tinting_table8(
	struct rgb_color_value *colors,
	short color_count,
	pixel8 *tint_table,
	short tint_start,
	short tint_count)
{
	short start, count;
	
	start= count= 0;
	while (get_next_color_run(colors, color_count, &start, &count))
	{
		short i;

		for (i=0; i<count; ++i)
		{
			short adjust= start ? 0 : 1;
			short value= (i*(tint_count+adjust))/count;
			
			value= (value>=tint_count) ? iBLACK : tint_start + value;
			tint_table[start+i]= value;
		}
	}
	
	return;
}

static void build_tinting_table16(
	struct rgb_color_value *colors,
	short color_count,
	pixel16 *tint_table,
	struct rgb_color *tint_color)
{
	short i;

	for (i= 0; i<color_count; ++i, ++colors)
	{
		long magnitude= ((long)colors->red + (long)colors->green + (long)colors->blue)/(short)3;
		
		*tint_table++= RGBCOLOR_TO_PIXEL16((magnitude*tint_color->red)/65535,
			(magnitude*tint_color->green)/65535, (magnitude*tint_color->blue)/65535);
	}

	return;
}

static void build_tinting_table32(
	struct rgb_color_value *colors,
	short color_count,
	pixel32 *tint_table,
	struct rgb_color *tint_color)
{
	short i;

	for (i= 0; i<color_count; ++i, ++colors)
	{
		long magnitude= ((long)colors->red + (long)colors->green + (long)colors->blue)/(short)3;
		
		*tint_table++= RGBCOLOR_TO_PIXEL32((magnitude*tint_color->red)/65535,
			(magnitude*tint_color->green)/65535, (magnitude*tint_color->blue)/65535);
	}

	return;
}


/* ---------- collection accessors */
// Some originally from shapes_macintosh.c

static struct collection_header *get_collection_header(
	short collection_index)
{
	// This one is intended to bomb because collection indices can only be from 1 to 31,
	// short of drastic changes in how collection indices are specified (a bigger structure
	// than shape_descriptor, for example).
	collection_header *header = GetMemberWithBounds(collection_headers,collection_index,MAXIMUM_COLLECTIONS);
	vassert(header,csprintf(temporary,"Collection index out of range: %d",collection_index));
	
	return header;
	
	/*
	assert(collection_index>=0 && collection_index<MAXIMUM_COLLECTIONS);
	
	return collection_headers + collection_index;
	*/
}

/*
static void *collection_offset(
	struct collection_definition *definition,
	long offset)
{
	vassert(offset>=0 && offset<definition->size,
		csprintf(temporary, "asked for offset #%d/#%d", offset, definition->size));

	return ((byte *)definition) + offset;
}
*/

static struct collection_definition *get_collection_definition(
	short collection_index)
{
	struct collection_definition *collection= get_collection_header(collection_index)->Definition;
	
	return collection;
}

static struct rgb_color_value *get_collection_colors(
	short collection_index,
	short clut_number)
{
	collection_header *header = get_collection_header(collection_index);
	collection_definition *definition = header->Definition;
	if (!definition) return NULL;
	
	return GetMemberWithBounds(header->Colors,
		clut_number*definition->color_count,
			definition->clut_count*definition->color_count);
	
	/*
	struct collection_definition *definition= _get_collection_definition(collection_index);

	assert(clut_number>=0&&clut_number<definition->clut_count);
	
	return (struct rgb_color_value *) collection_offset(definition, definition->color_table_offset+clut_number*sizeof(struct rgb_color_value)*definition->color_count);
	*/
}

static struct high_level_shape_definition *get_high_level_shape_definition(
	short collection_index,
	short high_level_shape_index)
{
	collection_header *header = get_collection_header(collection_index);
	collection_definition *definition = header->Definition;
	if (!definition) return NULL;

	// Watch out for extra indirection!
	high_level_shape_definition** ShapePP = 
		GetMemberWithBounds(header->Sequences,
			high_level_shape_index,
				definition->high_level_shape_count);
	
	if (!ShapePP) return NULL;
	
	return *ShapePP;

/*
	struct collection_definition *definition= get_collection_definition(collection_index);
	long *offset_table;
	
	vassert(high_level_shape_index>=0&&high_level_shape_index<definition->high_level_shape_count,
		csprintf(temporary, "asked for high-level shape %d/%d, collection %d", high_level_shape_index, definition->high_level_shape_count, collection_index));
	
	offset_table= (long *) collection_offset(definition, definition->high_level_shape_offset_table_offset);
	return (struct high_level_shape_definition *) collection_offset(definition, offset_table[high_level_shape_index]);
*/
}

static struct low_level_shape_definition *get_low_level_shape_definition(
	short collection_index,
	short low_level_shape_index)
{
	collection_header *header = get_collection_header(collection_index);
	collection_definition *definition = header->Definition;
	if (!definition) return NULL;
	
	return GetMemberWithBounds(header->Frames,
		low_level_shape_index,
			definition->low_level_shape_count);
	
/*
	struct collection_definition *definition= get_collection_definition(collection_index);
	long *offset_table;

	vassert(low_level_shape_index>=0 && low_level_shape_index<definition->low_level_shape_count,
		csprintf(temporary, "asked for low-level shape %d/%d, collection %d", low_level_shape_index, definition->low_level_shape_count, collection_index));
	
	offset_table= (long *) collection_offset(definition, definition->low_level_shape_offset_table_offset);
	return (struct low_level_shape_definition *) collection_offset(definition, offset_table[low_level_shape_index]);
*/
}

static struct bitmap_definition *get_bitmap_definition(
	short collection_index,
	short bitmap_index)
{
	collection_header *header = get_collection_header(collection_index);
	collection_definition *definition = header->Definition;
	if (!definition) return NULL;	
	
	// Watch out for extra indirection!
	bitmap_definition** BitmapPP = 
		GetMemberWithBounds(header->Bitmaps,
			bitmap_index,
				definition->bitmap_count);
	
	if (!BitmapPP) return NULL;
	
	return *BitmapPP;
	
/*
	struct collection_definition *definition= get_collection_definition(collection_index);
	long *offset_table;

	vassert(bitmap_index>=0 && bitmap_index<definition->bitmap_count,
		csprintf(temporary, "asked for collection #%d bitmap #%d/#%d", collection_index, bitmap_index, definition->bitmap_count));
	
	offset_table= (long *) collection_offset(definition, definition->bitmap_offset_table_offset);
	return (struct bitmap_definition *) collection_offset(definition, offset_table[bitmap_index]);
*/
}

static void *get_collection_shading_tables(
	short collection_index,
	short clut_index)
{
	void *shading_tables= get_collection_header(collection_index)->shading_tables;

	shading_tables = (uint8 *)shading_tables + clut_index*get_shading_table_size(collection_index);
	
	return shading_tables;
}

static void *get_collection_tint_tables(
	short collection_index,
	short tint_index)
{
	struct collection_definition *definition= get_collection_definition(collection_index);
	if (!definition) return NULL;
	
	void *tint_table= get_collection_header(collection_index)->shading_tables;

	tint_table = (uint8 *)tint_table + get_shading_table_size(collection_index)*definition->clut_count + shading_table_size*tint_index;
	
	return tint_table;
}

/*
static void debug_shapes_memory(
	void)
{
	short collection_index;
	struct collection_header *header;
	
	long total_size=0;

	for (collection_index= 0, header= collection_headers; collection_index<MAXIMUM_COLLECTIONS; ++collection_index, ++header)
	{
		if (collection_loaded(header))
		{
			struct collection_definition *definition= get_collection_definition(collection_index);
			
			dprintf("collection #% 2d @ #% 9d bytes", collection_index, definition->size);
			total_size+= definition->size;
		}
	}
	
	dprintf("                  #% 9d bytes total", total_size);
	
	return;
}
*/

// LP additions:

// Whether or not collection is present
bool is_collection_present(short collection_index)
{
	collection_header *CollHeader = get_collection_header(collection_index);
	return collection_loaded(CollHeader);
}

// Number of texture frames in a collection (good for wall-texture error checking)
short get_number_of_collection_frames(short collection_index)
{
	struct collection_definition *Collection = get_collection_definition(collection_index);
	return Collection->low_level_shape_count;
}

// Number of bitmaps in a collection (good for allocating texture information for OpenGL)
short get_number_of_collection_bitmaps(short collection_index)
{
	struct collection_definition *Collection = get_collection_definition(collection_index);
	return Collection->bitmap_count;
}

// Which bitmap index for a frame (good for OpenGL texture rendering)
short get_bitmap_index(short collection_index, short low_level_shape_index)
{
	struct low_level_shape_definition *low_level_shape= get_low_level_shape_definition(collection_index, low_level_shape_index);
	return low_level_shape->bitmap_index;
}


// XML elements for parsing infravision specification;
// this is a specification of a set of infravision colors
// and which collections are to get them

// This assigns an infravision color to a collection
class XML_InfravisionAssignParser: public XML_ElementParser
{
	bool IsPresent[2];
	short Coll, Color;

public:
	bool Start();
	bool HandleAttribute(const char *Tag, const char *Value);
	bool AttributesDone();
		
	XML_InfravisionAssignParser(): XML_ElementParser("assign") {}
};

bool XML_InfravisionAssignParser::Start()
{
	for (int k=0; k<2; k++)
		IsPresent[k] = false;
	return true;
}

bool XML_InfravisionAssignParser::HandleAttribute(const char *Tag, const char *Value)
{
	if (strcmp(Tag,"coll") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Coll,short(0),short(NUMBER_OF_COLLECTIONS-1)))
		{
			IsPresent[0] = true;
			return true;
		}
		else return false;
	}
	else if (strcmp(Tag,"color") == 0)
	{
		if (ReadBoundedNumericalValue(Value,"%hd",Color,short(0),short(NUMBER_OF_TINT_COLORS-1)))
		{
			IsPresent[1] = true;
			return true;
		}
		else return false;
	}
	UnrecognizedTag();
	return false;
}

bool XML_InfravisionAssignParser::AttributesDone()
{
	// Verify...
	bool AllPresent = true;
	for (int k=0; k<2; k++)
		if (!IsPresent[k]) AllPresent = false;
	
	if (!AllPresent)
	{
		AttribsMissing();
		return false;
	}
	
	// Put into place
	CollectionTints[Coll] = Color;
	return true;
}

static XML_InfravisionAssignParser InfravisionAssignParser;


// Subclassed to set the color objects appropriately
class XML_InfravisionParser: public XML_ElementParser
{
public:
	bool Start()
	{
		Color_SetArray(tint_colors16,NUMBER_OF_TINT_COLORS);
		return true;
	}
	bool HandleAttribute(const char *Tag, const char *Value)
	{
		UnrecognizedTag();
		return false;
	}
	
	XML_InfravisionParser(): XML_ElementParser("infravision") {}
};

static XML_InfravisionParser InfravisionParser;

// LP change: added infravision-parser export
XML_ElementParser *Infravision_GetParser()
{
	InfravisionParser.AddChild(&InfravisionAssignParser);
	InfravisionParser.AddChild(Color_GetParser());

	return &InfravisionParser;
}
