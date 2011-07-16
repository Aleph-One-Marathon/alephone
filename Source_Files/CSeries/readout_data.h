#ifndef __READOUT_DATA_H
#define __READOUT_DATA_H

/*
USAGE_TIMERS.H
Sunday, September 30, 2001

Sept 30, 2001 (Ian Rickard):
	Created so I can add stuff without recompiling the whole app. (shell.h is in my precomp header)
*/

#include "timer.h"

struct time_usage_stats
{
	Timer total;
	Timer totalUsed;
	Timer world, worldAI, worldCollision;
	Timer vis, visTree, visFlat;
	Timer render, renderCalc, renderObj, renderClip, renderDraw;
	Timer blit;
};

struct prerender_stats
{
	int totalNodes, totalWindows, sortedNodes, combinedWindows;
	int objects, objectWindows;
};

struct render_stats
{
	int vertical, verticalWindows, horizontal, horizontalWindows, sprites, spriteWindows;
	int surfacePixels, spritePixels;
	int surfaces, polys;
};

struct OGL_TexturesStats {
	int inUse;
	int binds, totalBind, minBind, maxBind;
	int longNormalSetups, longGlowSetups;
	int totalAge;
	OGL_TexturesStats() {
		inUse=binds=totalBind=0;
		minBind=500000;
		maxBind=longNormalSetups=longGlowSetups=totalAge=0;
	}
};

extern prerender_stats		gPrerenderStats;
extern render_stats			gRenderStats;
extern time_usage_stats		gUsageTimers;
extern OGL_TexturesStats	gGLTxStats;

#endif
