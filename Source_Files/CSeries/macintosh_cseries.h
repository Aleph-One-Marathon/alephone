#ifndef _MACINTOSH_CSERIES
#define _MACINTOSH_CSERIES

#define mac
#define DEBUG

#include <Events.h>
#include <AppleEvents.h>
#include <Aliases.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Sound.h>
#include <Gestalt.h>
#include <Devices.h>
#include <Resources.h>
#include <Script.h>
#include <Timer.h>
#include <TextUtils.h>
#include <PictUtils.h>
#include <Lists.h>
//#include <ControlDefinitions.h>

// Integer types with specific bit size
typedef SInt8 int8;
typedef UInt8 uint8;
typedef SInt16 int16;
typedef UInt16 uint16;
typedef SInt32 int32;
typedef UInt32 uint32;

#include "cstypes.h"
#include "csmacros.h"
#include "cscluts.h"
#include "cskeys.h"
#include "csdialogs.h"
#include "csstrings.h"
#include "csfonts.h"
#include "cspixels.h"
#include "csalerts.h"
#include "csfiles.h"
#include "csmisc.h"

#include "gdspec.h"

#endif

