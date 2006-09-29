#ifndef __ALEPHVERSION_H
#define __ALEPHVERSION_H

// ZZZ: screw this stuff - nobody's keeping it up to date.
/*
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
*/


#ifndef A1_VERSION_PLATFORM


# ifdef SDL

#  ifndef A1_VERSION_MINOR_PLATFORM

#   ifdef WIN32
#    define A1_VERSION_MINOR_PLATFORM "Windows"
#   elif defined(__APPLE__)
#    define A1_VERSION_MINOR_PLATFORM "Mac OS X"
#   elif defined(__MACOS__)
#    define A1_VERSION_MINOR_PLATFORM "Mac OS"
#   elif defined(TARGET_PLATFORM)
#    define A1_VERSION_MINOR_PLATFORM TARGET_PLATFORM
// add more minor platforms (and/or a general way of getting GNU-style arch/OS stuff) here
#   endif

#   ifndef A1_VERSION_MINOR_PLATFORM
#    define A1_VERSION_MINOR_PLATFORM "unknown platform"
#   endif

#  endif // !defined(A1_VERSION_MINOR_PLATFORM)

#  define A1_VERSION_PLATFORM "SDL " A1_VERSION_MINOR_PLATFORM


# else // now !defined(SDL)


#  ifndef A1_VERSION_MINOR_PLATFORM

#   ifdef TARGET_API_MAC_CARBON
#    ifdef __APPLE_CC__
#     define A1_VERSION_MINOR_PLATFORM "Carbon (PB)"
#    else
#     ifdef __MWERKS__
#      define A1_VERSION_MINOR_PLATFORM "Carbon (CW)"
#     else
#      define A1_VERSION_MINOR_PLATFORM "Carbon (Other)"
#     endif
#    endif
#   else
#    define A1_VERSION_MINOR_PLATFORM "Classic"
#   endif

#  endif // !defined(A1_VERSION_MINOR_PLATFORM)

#  define A1_VERSION_PLATFORM "Mac OS " A1_VERSION_MINOR_PLATFORM


# endif // !defined(SDL)


#endif // !defined(A1_VERSION_PLATFORM)



#ifndef A1_VERSION_PLATFORM
# define A1_VERSION_PLATFORM "unknown platform"
#endif

#ifndef A1_VERSION_STRING
# define A1_VERSION_STRING A1_VERSION_PLATFORM " " __DATE__
#endif // !defined(A1_VERSION_STRING)



#endif // ALEPHVERSION_H
