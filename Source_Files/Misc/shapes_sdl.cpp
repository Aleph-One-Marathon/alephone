/* 
 *  shapes_sdl.cpp - Shapes handling, SDL specific stuff
 *
 *  Written in 2000 by Christian Bauer
 */

#include <SDL/SDL_endian.h>
#include <byte_swapping.h>


// Global variables
static SDL_RWops *shapes_file = NULL;

// From FileHandler_SDL.cpp
extern void get_default_shapes_spec(FileSpecifier &File);

// Prototypes
static void close_shapes_file(void);
static void shutdown_shape_handler(void);


/*
 *  Open shapes file, read collection headers
 */

void open_shapes_file(FileSpecifier &file)
{
	// Open stream to shapes file
	SDL_RWops *p = SDL_RWFromFile(file.GetName(), "rb");
	if (p == NULL)
		return;

	// Read collection headers
	collection_header *h = collection_headers;
	for (int i=0; i<MAXIMUM_COLLECTIONS; i++, h++) {
		h->status = SDL_ReadBE16(p);
		h->flags = SDL_ReadBE16(p);
		h->offset = SDL_ReadBE32(p);
		h->length = SDL_ReadBE32(p);
		h->offset16 = SDL_ReadBE32(p);
		h->length16 = SDL_ReadBE32(p);
		SDL_RWseek(p, 12, SEEK_CUR);
		//printf(" collection %d, status %d, flags %04x, offset %d, length %d, offset16 %d, length16 %d\n", i, h->status, h->flags, h->offset, h->length, h->offset16, h->length16);
		h->collection = NULL;
		h->shading_tables = NULL;
	}

	// Close old shapes file, set new one
	close_shapes_file();
	shapes_file = p;
}


/*
 *  Close shapes file
 */

static void close_shapes_file(void)
{
	if (shapes_file) {
		SDL_FreeRW(shapes_file);
		shapes_file = NULL;
	}
}


/*
 *  Initialize shapes handling
 */

void initialize_shape_handler(void)
{
	FileSpecifier shapes;
	get_default_shapes_spec(shapes);
	open_shapes_file(shapes);

	if (shapes_file == NULL)
		alert_user(fatalError, strERRORS, badExtraFileLocations, -1);
	else
		atexit(shutdown_shape_handler);
}


/*
 *  Convert shape to surface
 */

SDL_Surface *get_shape_surface(int shape)
{
	// Get shape information
	int collection_index = GET_COLLECTION(GET_DESCRIPTOR_COLLECTION(shape));
	int clut_index = GET_COLLECTION_CLUT(GET_DESCRIPTOR_COLLECTION(shape));
	int low_level_shape_index = GET_DESCRIPTOR_SHAPE(shape);
	struct collection_definition *collection = get_collection_definition(collection_index);
	struct low_level_shape_definition *low_level_shape = get_low_level_shape_definition(collection_index, low_level_shape_index);
	struct bitmap_definition *bitmap = get_bitmap_definition(collection_index, low_level_shape->bitmap_index);

	// Extract color table
	SDL_Color colors[256];
	int num_colors = collection->color_count - NUMBER_OF_PRIVATE_COLORS;
	rgb_color_value *src_colors = get_collection_colors(collection_index, clut_index) + NUMBER_OF_PRIVATE_COLORS;
	for (int i=0; i<num_colors; i++) {
		int idx = src_colors[i].value;
		colors[idx].r = src_colors[i].red >> 8;
		colors[idx].g = src_colors[i].green >> 8;
		colors[idx].b = src_colors[i].blue >> 8;
	}

	SDL_Surface *s = NULL;
	if (bitmap->bytes_per_row == NONE) {

		// Column-order shape, needs to be converted to row-order
		fprintf(stderr, "Drawing of column-order shapes not implemented.\n");
		abort();
		//!!

	} else {

		// Row-order shape, we can directly create a surface from it
		s = SDL_CreateRGBSurfaceFrom(bitmap->row_addresses[0], bitmap->width, bitmap->height, 8, bitmap->bytes_per_row, 0xff, 0xff, 0xff, 0xff);
	}
	if (s == NULL)
		return NULL;

	// Set color table
	SDL_SetColors(s, colors, 0, 256);
	return s;
}


/*
 *  Load collection
 */

static boolean load_collection(short collection_index, boolean strip)
{
	SDL_RWops *p = shapes_file;	// Source stream
	uint32 *t;					// Offset table pointer

	// Get offset and length of data in source file from header
	collection_header *header = get_collection_header(collection_index);
	long src_offset, src_length;
	if (bit_depth == 8 || header->offset16 == -1) {
		vassert(header->offset != -1, csprintf(temporary, "collection #%d does not exist.", collection_index));
		src_offset = header->offset;
		src_length = header->length;
	} else {
		src_offset = header->offset16;
		src_length = header->length16;
	}

	// Read collection definition
	SDL_RWseek(p, src_offset, SEEK_SET);
	int16 version = SDL_ReadBE16(p);
	int16 type = SDL_ReadBE16(p);
	uint16 flags = SDL_ReadBE16(p);
	int16 color_count = SDL_ReadBE16(p);
	int16 clut_count = SDL_ReadBE16(p);
	int32 color_table_offset = SDL_ReadBE32(p);
	int16 high_level_shape_count = SDL_ReadBE16(p);
	int32 high_level_shape_offset_table_offset = SDL_ReadBE32(p);
	int16 low_level_shape_count = SDL_ReadBE16(p);
	int32 low_level_shape_offset_table_offset = SDL_ReadBE32(p);
	int16 bitmap_count = SDL_ReadBE16(p);
	int32 bitmap_offset_table_offset = SDL_ReadBE32(p);
	int16 pixels_to_world = SDL_ReadBE16(p);
	int32 size = SDL_ReadBE32(p);

	// Allocate memory for collection
	int extra_length = 1024 + high_level_shape_count * 4 + low_level_shape_count * 4 + bitmap_count * 4;
	void *c = malloc(src_length + extra_length);
	if (c == NULL)
		return false;

	// Initialize collection definition
	collection_definition *cd = (collection_definition *)c;
	cd->version = version;
	cd->type = type;
	cd->flags = flags;
	cd->color_count = color_count;
	cd->clut_count = clut_count;
	cd->high_level_shape_count = high_level_shape_count;
	cd->low_level_shape_count = low_level_shape_count;
	cd->bitmap_count = bitmap_count;
	cd->pixels_to_world = pixels_to_world;
//	printf(" index %d, version %d, type %d, %d colors, %d cluts, %d hl, %d ll, %d bitmaps\n", collection_index, version, type, color_count, clut_count, high_level_shape_count, low_level_shape_count, bitmap_count);

	// Set up destination pointer
	uint8 *q = (uint8 *)c + 0x220;
#define dst_offset (q - (uint8 *)c)

	// Convert CLUTs
	SDL_RWseek(p, src_offset + color_table_offset, SEEK_SET);
	cd->color_table_offset = dst_offset;
	for (int i=0; i<clut_count*color_count; i++) {
		rgb_color_value *r = (rgb_color_value *)q;
		SDL_RWread(p, r, 1, 2);
		r->red = SDL_ReadBE16(p);
		r->green = SDL_ReadBE16(p);
		r->blue = SDL_ReadBE16(p);
		q += sizeof(rgb_color_value);
	}

	// Convert high-level shape definitions
	SDL_RWseek(p, src_offset + high_level_shape_offset_table_offset, SEEK_SET);
	cd->high_level_shape_offset_table_offset = dst_offset;

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), high_level_shape_count);
	byte_swap_memory(t, _4byte, high_level_shape_count);
	q += high_level_shape_count * sizeof(uint32);

	for (int i=0; i<high_level_shape_count; i++) {

		// Seek to offset in source file, correct destination offset
		SDL_RWseek(p, src_offset + t[i], SEEK_SET);
		t[i] = dst_offset;

		// Convert high-level shape definition
		high_level_shape_definition *d = (high_level_shape_definition *)q;
		d->type = SDL_ReadBE16(p);
		d->flags = SDL_ReadBE16(p);
		SDL_RWread(p, d->name, 1, HIGH_LEVEL_SHAPE_NAME_LENGTH + 2);
		d->number_of_views = SDL_ReadBE16(p);
		d->frames_per_view = SDL_ReadBE16(p);
		d->ticks_per_frame = SDL_ReadBE16(p);
		d->key_frame = SDL_ReadBE16(p);
		d->transfer_mode = SDL_ReadBE16(p);
		d->transfer_mode_period = SDL_ReadBE16(p);
		d->first_frame_sound = SDL_ReadBE16(p);
		d->key_frame_sound = SDL_ReadBE16(p);
		d->last_frame_sound = SDL_ReadBE16(p);
		d->pixels_to_world = SDL_ReadBE16(p);
		d->loop_frame = SDL_ReadBE16(p);
		SDL_RWseek(p, 28, SEEK_CUR);

		// Convert low-level shape index list
		int num_views;
		switch (d->number_of_views) {
			case _unanimated:
			case _animated1:
				num_views = 1;
				break;
			case _animated3to4:
			case _animated4:
				num_views = 4;
				break;
			case _animated3to5:
			case _animated5:
				num_views = 5;
				break;
			case _animated2to8:
			case _animated5to8:
			case _animated8:
				num_views = 8;
				break;
			default:
				num_views = d->number_of_views;
				break;
		}
		for (int j=0; j<num_views*d->frames_per_view; j++)
			d->low_level_shape_indexes[j] = SDL_ReadBE16(p);

		q += sizeof(high_level_shape_definition) + (num_views * d->frames_per_view - 1) * sizeof(int16);
		if (dst_offset & 3)	// Align to 32-bit boundary
			q += 4 - (dst_offset & 3);
	}

	// Convert low-level shape definitions
	SDL_RWseek(p, src_offset + low_level_shape_offset_table_offset, SEEK_SET);
	cd->low_level_shape_offset_table_offset = dst_offset;

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), low_level_shape_count);
	byte_swap_memory(t, _4byte, low_level_shape_count);
	q += low_level_shape_count * sizeof(uint32);

	for (int i=0; i<low_level_shape_count; i++) {

		// Seek to offset in source file, correct destination offset
		SDL_RWseek(p, src_offset + t[i], SEEK_SET);
		t[i] = dst_offset;

		// Convert low-level shape definition
		low_level_shape_definition *d = (low_level_shape_definition *)q;
		d->flags = SDL_ReadBE16(p);
		d->minimum_light_intensity = SDL_ReadBE32(p);
		d->bitmap_index = SDL_ReadBE16(p);
		d->origin_x = SDL_ReadBE16(p);
		d->origin_y = SDL_ReadBE16(p);
		d->key_x = SDL_ReadBE16(p);
		d->key_y = SDL_ReadBE16(p);
		d->world_left = SDL_ReadBE16(p);
		d->world_right = SDL_ReadBE16(p);
		d->world_top = SDL_ReadBE16(p);
		d->world_bottom = SDL_ReadBE16(p);
		d->world_x0 = SDL_ReadBE16(p);
		d->world_y0 = SDL_ReadBE16(p);
		SDL_RWseek(p, 8, SEEK_CUR);
		q += sizeof(low_level_shape_definition);
	}

	// Convert bitmap definitions
	SDL_RWseek(p, src_offset + bitmap_offset_table_offset, SEEK_SET);
	cd->bitmap_offset_table_offset = dst_offset;

	t = (uint32 *)q;	// Offset table
	SDL_RWread(p, t, sizeof(uint32), bitmap_count);
	byte_swap_memory(t, _4byte, bitmap_count);
	q += bitmap_count * sizeof(uint32);

	for (int i=0; i<bitmap_count; i++) {

		// Seek to offset in source file, correct destination offset
		SDL_RWseek(p, src_offset + t[i], SEEK_SET);
		t[i] = dst_offset;

		// Convert bitmap definition
		bitmap_definition *d = (bitmap_definition *)q;
		d->width = SDL_ReadBE16(p);
		d->height = SDL_ReadBE16(p);
		d->bytes_per_row = SDL_ReadBE16(p);
		d->flags = SDL_ReadBE16(p);
		d->bit_depth = SDL_ReadBE16(p);
		SDL_RWseek(p, 16, SEEK_CUR);
		q += sizeof(bitmap_definition);

		// Skip row address pointers
		int rows = (d->flags & _COLUMN_ORDER_BIT) ? d->width : d->height;
		SDL_RWseek(p, (rows + 1) * sizeof(uint32), SEEK_CUR);
		q += rows * sizeof(pixel8 *);

		// Copy bitmap data
		if (d->bytes_per_row == NONE) {
			// RLE format
			for (int j=0; j<rows; j++) {
				int16 first = SDL_ReadBE16(p);
				int16 last = SDL_ReadBE16(p);
#ifdef LITTLE_ENDIAN
				*q++ = first; *q++ = first >> 8;
				*q++ = last; *q++ = last >> 8;
#else
				*q++ = first >> 8; *q++ = first;
				*q++ = last >> 8; *q++ = last;
#endif
				SDL_RWread(p, q, 1, last - first);
				q += last - first;
			}
		} else {
			// Raw format
			SDL_RWread(p, q, d->bytes_per_row, rows);
			q += rows * d->bytes_per_row;
		}
		if (dst_offset & 3)	// Align to 32-bit boundary
			q += 4 - (dst_offset & 3);
	}

	// Set pointer to collection in collection header
	header->collection = cd;
	cd->size = dst_offset;
//	printf(" collection at %p, size %d -> %d\n", cd, src_length, cd->size);
	assert(cd->size <= src_length + extra_length);

	if (strip) {
		//!! don't know what to do
		fprintf(stderr, "Stripped shapes not implemented\n");
		abort();
	}

	// Allocate enough space for this collection's shading tables
	if (strip)
		header->shading_tables = malloc(8);
	else {
		collection_definition *definition = get_collection_definition(collection_index);
		header->shading_tables = malloc(get_shading_table_size(collection_index) * definition->clut_count + shading_table_size * NUMBER_OF_TINT_TABLES);
	}
	if (header->shading_tables == NULL) {
		free(header->collection);
		header->collection = NULL;
		free(c);
		return false;
	}

	// Everything OK
	return true;
}


/*
 *  Unload collection
 */

static void unload_collection(struct collection_header *header)
{
	assert(header->collection);
	free(header->collection);
	free(header->shading_tables);
	header->collection = NULL;
	header->shading_tables = NULL;
}
