/*
texturers.cpp
Aug 22, 2001

Aug 22, 2001 (Ian Rickard):
	Created.  This is where the code for the surface texturers actually lives.
*/

#include "texturers.h"

inline int NextLowerExponent(int n)
{
	int p = n;
	int xp = 0;
	while(p > 1) {p >>= 1; xp++;}
	return xp;
}


#define BIT_DEPTH 8
	#define TRANSPARENT_TEXTURERS 0
		#define BIG_TEXTURERS 0
			#include "low_level_textures.cpp"
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#include "low_level_textures.cpp"
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
	#define TRANSPARENT_TEXTURERS 1
		#define BIG_TEXTURERS 0
			#include "low_level_textures.cpp"
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#include "low_level_textures.cpp"
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
#undef BIT_DEPTH

#define BIT_DEPTH 16
	#define TRANSPARENT_TEXTURERS 0
		#define BIG_TEXTURERS 0
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
	#define TRANSPARENT_TEXTURERS 1
		#define BIG_TEXTURERS 0
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
#undef BIT_DEPTH

#define BIT_DEPTH 32
	#define TRANSPARENT_TEXTURERS 0
		#define BIG_TEXTURERS 0
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
	#define TRANSPARENT_TEXTURERS 1
		#define BIG_TEXTURERS 0
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
		#define BIG_TEXTURERS 1
			#define TRANSLUCENT_TEXTURERS 0
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
			#define TRANSLUCENT_TEXTURERS 1
				#include "low_level_textures.cpp"
			#undef TRANSLUCENT_TEXTURERS
		#undef BIG_TEXTURERS
	#undef TRANSPARENT_TEXTURERS
#undef BIT_DEPTH
