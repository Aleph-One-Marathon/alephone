#include "upscaled_textures.h"

using namespace std;

void UpscaledTextureManager::Cached::Cleanup() {
	DisposePtr(disposer);
}

void UpscaledTextureManager::DoUpscale(bitmap_definition *src, bitmap_definition*dst, bool tiled) {
	assert (src->width*2 == dst->width);
	assert (src->height*2 == dst->height);
	
	int rows= (src->flags&_COLUMN_ORDER_BIT) ? src->width : src->height;
	int columns= (src->flags&_COLUMN_ORDER_BIT) ? src->height : src->width;

	for (int y=0 ; y<rows ; y++) {
		pixel8 *srcp1 = src->row_addresses[y];
		pixel8 *srcp2 = src->row_addresses[(tiled&&y+1==rows)?0:y+1];
		pixel8 *dstp1 = dst->row_addresses[y*2];
		pixel8 *dstp2 = dst->row_addresses[y*2+1];
		
		pixel8 tl, tr, bl, br;
		
		for (int x=0 ; x<columns ; x++) {
			tl = srcp1[0];
			tr = (x+1==columns)?(tiled?srcp2[1-columns]:0):srcp1[1];
			if (tiled || y+1<rows) {
				bl = srcp2[0];
				br = (x+1==columns)?(tiled?srcp2[1-columns]:0):srcp2[1];
			} else {
				bl = br = 0;
			}
			
			dstp1[0] = tl;
			
			if (tl == br) {
				if (bl == tr) {
					dstp1[1] = dstp2[0] = dstp2[1] = tl;
				} else {
					dstp1[1] = dstp2[1] = tl;
					dstp2[0] = bl;
				}
			} else if (bl == tr) {
				dstp1[1] = dstp2[1] = dstp2[0] = tr;
			} else {
					dstp1[1] = dstp2[0] = dstp2[1] = tl;
			}
			
			dstp1+=2;
			dstp2+=2;
			srcp1++;
			srcp2++;
		}
	}
	
}

UpscaledTextureManager::~UpscaledTextureManager() {
	list<Cached>::iterator i;

	for (i = cached.begin(); i !=  cached.end() ; i++) {
		i->Cleanup();
		cached.erase(i);
	}
}

bitmap_definition* UpscaledTextureManager::GetUpscale(bitmap_definition* original, bool tiled) {
	list<Cached>::iterator i;
	
	if (original->width == 256 && original->height == 256) return original;
	
	assert(original->bit_depth == 8);
	assert(original->bytes_per_row!=NONE);
	
	for (i = cached.begin(); i !=  cached.end() ; i++) if (i->original == original) {
		i->touched = true;
		return i->scaled;
	}
	
	int rows= (original->flags&_COLUMN_ORDER_BIT) ? original->width : original->height;
	int columns= (original->flags&_COLUMN_ORDER_BIT) ? original->height : original->width;

	int size = (original->height*original->width*4 + sizeof(bitmap_definition) + 
	                                      sizeof(pixel8 *)*rows*2 + 15) & ~15;
	
	bool cleared=true;
	
	while (cleared && (memUsage + size > maxMemUsage)) {
		cleared = false;
			
		for (i = cached.begin(); i !=  cached.end() ; i++)
			if (!i->touched) {
				memUsage -= i->memUsage;
				dprintf("freeing upscale for map %08X.  Free memory: %dk\n", i->original, (maxMemUsage-memUsage)/1024);
				i->Cleanup();
				cached.erase(i);
				cleared = true;
				break;
			}
	} 
	
	if (memUsage + size > maxMemUsage) return original;
	
	dprintf("allocating upscale for map %08X.  Free memory: %dk\n", original, (maxMemUsage-memUsage-size)/1024);
	
	Ptr thePtr = NewPtr(size);
	if (!thePtr) return original;

	pixel8 *newtex = (pixel8*)thePtr;
	bitmap_definition *newmap = (bitmap_definition*)(newtex + original->height*original->width*4);
	
	newmap->bit_depth = 8;
	newmap->bytes_per_row = original->width*2;
	newmap->width = original->width*2;
	newmap->height = original->height*2;
	newmap->flags = original->flags;
	
	rows= (newmap->flags&_COLUMN_ORDER_BIT) ? newmap->width : newmap->height;
	columns= (newmap->flags&_COLUMN_ORDER_BIT) ? newmap->height : newmap->width;
	
	for (int i=0 ; i<rows ; i++)
		newmap->row_addresses[i] = newtex + i*columns;
	
	DoUpscale(original, newmap, tiled);
	
	cached.push_back(Cached(original, newmap, thePtr, size));
	memUsage += size;
	
	return newmap;
}

void UpscaledTextureManager::NextFrame() {
	list<Cached>::iterator i;
	
	for (i = cached.begin(); i !=  cached.end() ; i++)
		i->touched=false;
}

void UpscaledTextureManager::Flush() {
	list<Cached>::iterator i;
	for (i = cached.begin(); i !=  cached.end() ; i++) {
		i->Cleanup();
	}
	cached.clear();
	memUsage = 0;
}