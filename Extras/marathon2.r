/*
MARATHON2.R
Thursday, August 20, 1992 7:15:23 PM

Thursday, February 17, 1994 9:04:12 AM
	changes to build fat applications.
Monday, March 28, 1994 3:08:23 PM
	the file CODE resources are pulled from when building a fat application is now
	an environment variable, CODE_FILE, instead of being hardcoded.
*/

#include "Types.r"
#include "SysTypes.r"
#include "CodeFragmentTypes.r"

#define CREATOR '52.4'
#define VERSION "1.0"
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define RELEASE_STAGE development
#define PRE_RELEASE_REVISION 0

#ifdef DEMO
	#define SHORT_VERSION_STRING "v" VERSION
	#define LONG_VERSION_STRING "v" VERSION " DEMO © 1995 Bungie Software Products Corp."
#else
	#define SHORT_VERSION_STRING "v" VERSION
	#define LONG_VERSION_STRING "v" VERSION " © 1995 Bungie Software Products Corp."
#endif

#ifndef fat

include ":binaries:marathon2.resource";
include ":demos:demos.resource";
/* include ":texts:texts.resource"; */
/* include ":graphics:screens"; */

#ifdef DEMO
include ":binaries:demo.resource"; /* overrides resources in marathon.resource */
include ":graphics:demo.screens";
#else
include ":binaries:game.resource";
include ":graphics:game.screens";
#endif
#endif /* fat */

#ifdef envppc
	#ifdef fat
		include CODE_FILE 'CODE';
		#ifndef DEMO
			/* include whatever resource we need to get the 68k checksum from */
		#endif
	#else
		include ":binaries:notppc";
	#endif
	resource 'cfrg' (0)
	{
		{
			kPowerPC,
			kFullLib,
			kNoVersionNum,
			kNoVersionNum,
			kDefaultStackSize,
			kNoAppSubFolder,
			kIsApp,
			kOnDiskFlat,
			kZeroOffset,
			kWholeFork,
			"Marathon 2"
		}
	};
#endif

#ifdef DEBUG
type 'dbug' as 'STR ';
resource 'dbug' (128)
{
	"i’m sleeping my day away"
};
#endif

resource 'vers' (1)
{
	MAJOR_VERSION, MINOR_VERSION, RELEASE_STAGE, PRE_RELEASE_REVISION,
	verUS, /* US version */
	SHORT_VERSION_STRING,
	LONG_VERSION_STRING
};

type CREATOR as 'STR ';
resource CREATOR (0)
{
	LONG_VERSION_STRING
};
