#ifndef __ALEPHVERSION_H
#define __ALEPHVERSION_H

	#define MAJOR_VERSION   1
	#define MAJOR_VERSION_S "1"
	#define MINOR_VERSION   0
	#define MINOR_VERSION_S "0"
	#define STEPPING        "a"
	#define BUILDNUM        1
	#define BUILDSTR        "1"
	#define BRANCH          "(trunk)"
	#define FINAL	        0
	
	#if FINAL
		#define VERSION_STRING MAJOR_VERSION_S "." MINOR_VERSION_S STEPPING BUILDSTR BRANCH
	#else
		#define VERSION_STRING MAJOR_VERSION_S "." MINOR_VERSION_S STEPPING BUILDSTR "pre" BRANCH "(" __DATE__ " " __TIME__ ")"
	#endif
#endif
