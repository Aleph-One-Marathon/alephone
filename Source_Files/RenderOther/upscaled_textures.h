#ifndef __UPSCALED_TEXTURE_H
#define __UPSCALED_TEXTURE_H

#include <list>

#include "textures.h"

class UpscaledTextureManager {
	struct Cached {
		bitmap_definition *original, *scaled;
		Ptr disposer;
		bool touched;
		int memUsage;
		Cached(bitmap_definition *orig, bitmap_definition *scale, Ptr disp, int size)
			{original = orig; scaled = scale; disposer = disp; memUsage=size; touched = true;}
		void Cleanup();
	};
	
	int memUsage, maxMemUsage;
	
	std::list<Cached> cached;
	
	void DoUpscale(bitmap_definition *src, bitmap_definition*dst, bool tiled);
	
public:
	UpscaledTextureManager(int maxMem=2*1024*1024) {memUsage = 0; maxMemUsage = maxMem;}
	~UpscaledTextureManager();
	
	bitmap_definition* GetUpscale(bitmap_definition* original, bool tiled);
	
	void NextFrame(); // marks cached images as purgable
	void Flush();
};

#endif