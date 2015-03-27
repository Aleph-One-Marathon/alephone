/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
*/

#import "SDL.h"
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

/* For some reaon, Apple removed setAppleMenu from the headers in 10.4,
 but the method still is there and works. To avoid warnings, we declare
 it ourselves here. */
@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end

/* Use this flag to determine whether we use SDLMain.nib or not */
#define		SDL_USE_NIB_FILE	0

/* Use this flag to determine whether we use CPS (docking) or not */
#define		SDL_USE_CPS		0
#if SDL_USE_CPS
/* Portions of CPS.h */
typedef struct CPSProcessSerNum
{
	UInt32		lo;
	UInt32		hi;
} CPSProcessSerNum;

extern OSErr	CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern OSErr 	CPSEnableForegroundOperation( CPSProcessSerNum *psn, UInt32 _arg2, UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
extern OSErr	CPSSetFrontProcess( CPSProcessSerNum *psn);

#endif /* SDL_USE_CPS */

/* The name of our bundle (such as AlephOneSDL.app") which we determine at run-time */
char *bundle_name = NULL;
/* The short application name, to present to users (seen in menus, etc.) */
char *application_name = NULL;
/* The application bundle identifier, useful for unique directories */
char *application_identifier = NULL;
/* The bundle's Resources path, for finding bundled data */
char *bundle_resource_path = NULL;
/* OS default directories */
char *app_log_directory = NULL;
char *app_preferences_directory = NULL;
char *app_support_directory = NULL;
char *app_screenshots_directory = NULL;

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;
static BOOL   gCalledAppMainline = FALSE;

static NSString *getApplicationName(void)
{
	NSString *appName = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:(NSString *)kCFBundleNameKey];
    
    if (![appName length])
        appName = [[NSProcessInfo processInfo] processName];

    return appName;
}

/* Helper for directory creation */
static void createDirectory(NSString *path)
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    [fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
#else
    [fileManager createDirectoryAtPath:path attributes:nil];
#endif
}

#if SDL_USE_NIB_FILE
/* A helper category for NSString */
@interface NSString (ReplaceSubString)
- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString;
@end
#endif

@interface SDLApplication : NSApplication
@end

@implementation SDLApplication
/* Invoked from the Quit menu item */
- (void)terminate:(id)sender
{
    /* Post a SDL_QUIT event */
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}
@end

/* The main class of the application, the application's delegate */
@implementation SDLMain

/* Find the name of our bundle, as we'll need this later for finding files. */
/* We also find other application identifiers here. */
- (void) findBundleName
{
	NSBundle *bundle = [NSBundle mainBundle];
	NSDictionary *bundleInfo = [bundle localizedInfoDictionary];

	NSString *bundleName = [[bundle bundlePath] lastPathComponent];
	bundle_name = strdup([bundleName UTF8String]);
	
	NSString *appName = [bundleInfo objectForKey:(NSString *)kCFBundleNameKey];
	application_name = strdup([appName UTF8String]);
	
	NSString *bundleID = [[bundle infoDictionary] objectForKey:(NSString *)kCFBundleIdentifierKey];
	application_identifier = strdup([bundleID UTF8String]);
	
	NSString *bundleRes = [bundle resourcePath];
	bundle_resource_path = strdup([bundleRes UTF8String]);

	/* Find other system directories we need. */
	NSArray *arr = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	NSString *libraryPath = [arr objectAtIndex:0];
	if (libraryPath != nil)
	{
		NSString *logPath = [libraryPath stringByAppendingPathComponent:@"Logs"];
		createDirectory(logPath);
		app_log_directory = strdup([logPath UTF8String]);
		
#ifdef PREFER_APP_NAME_TO_BUNDLE_ID
		NSString *prefsPath = [[libraryPath stringByAppendingPathComponent:@"Preferences"] stringByAppendingPathComponent:appName];
#else
		NSString *prefsPath = [[libraryPath stringByAppendingPathComponent:@"Preferences"] stringByAppendingPathComponent:bundleID];
#endif
		createDirectory(prefsPath);
		app_preferences_directory = strdup([prefsPath UTF8String]);
	}
	
	arr = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *supportPath = [arr objectAtIndex:0];
	if (supportPath != nil)
	{
#ifdef PREFER_APP_NAME_TO_BUNDLE_ID
		NSString *appSupportPath = [supportPath stringByAppendingPathComponent:appName];
#else
		NSString *appSupportPath = [supportPath stringByAppendingPathComponent:@"AlephOne"];
#endif
		createDirectory(appSupportPath);
		app_support_directory = strdup([appSupportPath UTF8String]);
	}
    
#ifdef MAC_APP_STORE
    arr = NSSearchPathForDirectoriesInDomains(NSPicturesDirectory, NSUserDomainMask, YES);
    NSString *picturesPath = [arr objectAtIndex:0];
    if (picturesPath != nil)
    {
#ifdef PREFER_APP_NAME_TO_BUNDLE_ID
		NSString *screenshotsPath = [picturesPath stringByAppendingPathComponent:[appName stringByAppendingString:@" Screenshots"]];
#else
		NSString *screenshotsPath = [picturesPath stringByAppendingPathComponent:@"AlephOne Screenshots"];
#endif
        createDirectory(screenshotsPath);
        app_screenshots_directory = strdup([screenshotsPath UTF8String]);
    }
#endif
}		
		

/* Set the working directory to the .app's parent directory */
- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    if (shouldChdir)
    {
        char parentdir[MAXPATHLEN];
		CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
		CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
		if (CFURLGetFileSystemRepresentation(url2, true, (UInt8 *)parentdir, MAXPATHLEN)) {
	        assert ( chdir (parentdir) == 0 );   /* chdir to the binary app's parent */
		}
		CFRelease(url);
		CFRelease(url2);
	}

}

#if SDL_USE_NIB_FILE

/* Fix menu to contain the real app name instead of "SDL App" */
- (void)fixMenu:(NSMenu *)aMenu withAppName:(NSString *)appName
{
    NSRange aRange;
    NSEnumerator *enumerator;
    NSMenuItem *menuItem;

    aRange = [[aMenu title] rangeOfString:@"SDL App"];
    if (aRange.length != 0)
        [aMenu setTitle: [[aMenu title] stringByReplacingRange:aRange with:appName]];

    enumerator = [[aMenu itemArray] objectEnumerator];
    while ((menuItem = [enumerator nextObject]))
    {
        aRange = [[menuItem title] rangeOfString:@"SDL App"];
        if (aRange.length != 0)
            [menuItem setTitle: [[menuItem title] stringByReplacingRange:aRange with:appName]];
        if ([menuItem hasSubmenu])
            [self fixMenu:[menuItem submenu] withAppName:appName];
    }
    [ aMenu sizeToFit ];
}

#else

static void setApplicationMenu(void)
{
    /* warning: this code is very odd */
    NSMenu *appleMenu;
    NSMenuItem *menuItem;
    NSString *title;
    NSString *appName;
    
    appName = getApplicationName();
    appleMenu = [[NSMenu alloc] initWithTitle:@""];
    
    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];

    
    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appleMenu];
    [[NSApp mainMenu] addItem:menuItem];

    /* Tell the application object that this is now the application menu */
    [NSApp setAppleMenu:appleMenu];

    /* Finally give up our references to the objects */
    [appleMenu release];
    [menuItem release];
}

/* Create a window menu */
static void setupWindowMenu(void)
{
    NSMenu      *windowMenu;
    NSMenuItem  *windowMenuItem;
    NSMenuItem  *menuItem;

    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];
    
    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];

    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}

/* Replacement for NSApplicationMain */
static void CustomApplicationMain (int argc, char **argv)
{
    NSAutoreleasePool	*pool = [[NSAutoreleasePool alloc] init];
    SDLMain				*sdlMain;

    /* Ensure the application object is initialised */
    [SDLApplication sharedApplication];
    
#if SDL_USE_CPS
    {
        CPSProcessSerNum PSN;
        /* Tell the dock about us */
        if (!CPSGetCurrentProcess(&PSN))
            if (!CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103))
                if (!CPSSetFrontProcess(&PSN))
                    [SDLApplication sharedApplication];
    }
#endif /* SDL_USE_CPS */

    /* Set up the menubar */
    [NSApp setMainMenu:[[NSMenu alloc] init]];
    setApplicationMenu();
    setupWindowMenu();

    /* Create SDLMain and make it the app delegate */
    sdlMain = [[SDLMain alloc] init];
    [NSApp setDelegate:sdlMain];
    
    /* Start the main event loop */
    [NSApp run];
    
    [sdlMain release];
    [pool release];
}

#endif


/*
 * Catch document open requests...this lets us notice files when the app
 *  was launched by double-clicking a document, or when a document was
 *  dragged/dropped on the app's icon. You need to have a
 *  CFBundleDocumentsType section in your Info.plist to get this message,
 *  apparently.
 *
 * Files are added to gArgv, so to the app, they'll look like command line
 *  arguments. Previously, apps launched from the finder had nothing but
 *  an argv[0].
 *
 * This message may be received multiple times to open several docs on launch.
 *
 * This message is ignored once the app's mainline has been called.
 */
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    const char *temparg;
    size_t arglen;
    char *arg;
    char **newargv;

    if (!gFinderLaunch)  /* MacOS is passing command line args. */
        return TRUE;     /* We'll handle this in usage() instead. */

    if (gCalledAppMainline)  /* app has started, ignore this document. */
        return FALSE;

    temparg = [filename UTF8String];
    arglen = SDL_strlen(temparg) + 1;
    arg = (char *) SDL_malloc(arglen);
    if (arg == NULL)
        return FALSE;

    newargv = (char **) realloc(gArgv, sizeof (char *) * (gArgc + 2));
    if (newargv == NULL)
    {
        SDL_free(arg);
        return FALSE;
    }
    gArgv = newargv;

    SDL_strlcpy(arg, temparg, arglen);
    gArgv[gArgc++] = arg;
    gArgv[gArgc] = NULL;
    return TRUE;
}


/* Called when the internal event loop has just started running */
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

/* Find the bundle name from where we were launched and save for later use */
    [self findBundleName];

    /* Set the working directory to the .app's parent directory */
    [self setupWorkingDirectory:gFinderLaunch];

#if SDL_USE_NIB_FILE
    /* Set the main menu to contain the real app name instead of "SDL App" */
    [self fixMenu:[NSApp mainMenu] withAppName:getApplicationName()];
#endif

    /* Hand off to main application code */
    gCalledAppMainline = TRUE;
    status = SDL_main (gArgc, gArgv);

    /* We're done, thank you for playing */
    free(bundle_name);
    free(application_name);
    free(application_identifier);
    free(bundle_resource_path);
    free(app_log_directory);
    free(app_preferences_directory);
    free(app_support_directory);
    exit(status);
}
@end


@implementation NSString (ReplaceSubString)

- (NSString *)stringByReplacingRange:(NSRange)aRange with:(NSString *)aString
{
    unsigned int bufferSize;
    unsigned int selfLen = [self length];
    unsigned int aStringLen = [aString length];
    unichar *buffer;
    NSRange localRange;
    NSString *result;

    bufferSize = selfLen + aStringLen - aRange.length;
    buffer = NSAllocateMemoryPages(bufferSize*sizeof(unichar));
    
    /* Get first part into buffer */
    localRange.location = 0;
    localRange.length = aRange.location;
    [self getCharacters:buffer range:localRange];
    
    /* Get middle part into buffer */
    localRange.location = 0;
    localRange.length = aStringLen;
    [aString getCharacters:(buffer+aRange.location) range:localRange];
     
    /* Get last part into buffer */
    localRange.location = aRange.location + aRange.length;
    localRange.length = selfLen - localRange.location;
    [self getCharacters:(buffer+aRange.location+aStringLen) range:localRange];
    
    /* Build output string */
    result = [NSString stringWithCharacters:buffer length:bufferSize];
    
    NSDeallocateMemoryPages(buffer, bufferSize);
    
    return result;
}

@end



#ifdef main
#  undef main
#endif


static int IsTenPointNineOrLater()
{
    /* Gestalt() is deprecated in 10.8, but I don't care. Stop using SDL 1.2. */
    SInt32 major, minor;
    Gestalt(gestaltSystemVersionMajor, &major);
    Gestalt(gestaltSystemVersionMinor, &minor);
    return ( ((major << 16) | minor) >= ((10 << 16) | 9) );
}

extern bool force_software_gamma;

/* Main entry point to executable - should *not* be SDL_main! */
int main (int argc, char **argv)
{
    /* Disable gamma under 10.9 */
    if (IsTenPointNineOrLater())
        force_software_gamma = true;
    
    /* Copy the arguments into a global variable */
    if (getenv("ALEPHONE_FINDER_LAUNCH")) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }

#if SDL_USE_NIB_FILE
    [SDLApplication poseAsClass:[NSApplication class]];
    NSApplicationMain (argc, argv);
#else
    CustomApplicationMain (argc, argv);
#endif
    return 0;
}
