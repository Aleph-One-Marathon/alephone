Aleph One/Carbon MacOSX Installation Instructions
=================================================

The supplied project files are for Apple's Project Builder (Version 2.1 for
MacOS X 10.2.x.) If the development tools weren't included in your MacOS X
installation, you can download them from http://connect.apple.com. You must be a
registered Apple Developer Connection (ADC) member, but a free registration is
available which gives you access to a version of the developer tools.

This is not the MacOS X SDL version, but the Carbonized version based on the
MacOS 7.x-9.x/Classic UI code.

Required Tools
--------------

	* Apple's OSX Developer release (Project Builder, et al.)
	* MacCVS Pro (optional)

As mentioned above, if the development tools weren't included in your MacOS X
installation, you can download them from http://connect.apple.com. You must be a
registered Apple Developer Connection (ADC) member, but a free registration is
available which gives you access to a version of the developer tools.

The recommended way of dealing with the A1 CVS respository under MacOS X is by
using the Command-Line CVS tools, which should be installed automatically on
your machine when you install the Apple Developer Tools. For more information
about Command-Line CVS check out the CVS homepage at http://www.cvshome.org/ and
their manual at http://www.cvshome.org/docs/manual/.

For people who would like to continue using a GUI interface for CVS, then you
can use MacCVS Pro to check out the files.

The Carbon version of MacCVS Pro 2.7d3 or later can be downloaded from
http://www.maccvs.org. Unfortunately if you download AlephOne through one CVS
tool, then the other can't work with the files and vice versa. To use a
different CVS tool, then you will need to check the files out into an entirely
new directory.

After setting up a session file for AlephOne within MacCVS Pro you may wish to
go to Session Settings and choose the File Translation and Encoding settings. It
can be a little more convenient if you set the Code Warrior creators (CWIE) to
something nonsensical like ' ' (a single space) and setting linefeeds to Unix.
Thus, letting you use FileMerge to compare files and to have all the files open
by default within ProjectBuilder. This isn't necessary, just potentially useful.

More Information on configuring MacCVS Pro for Aleph can be found at:
http://source.bungie.org/_enginedevelopment/cvs/aboutcvs.html

Required Libraries
------------------

	* SDL
	* SDL_net
	* Lua
	* Speex

AlephOne/Carbon with SDL networking for MacOSX requires the Simple DirectMedia
Layer (SDL) library, available from the official SDL site:

  http://www.libsdl.org/

While one can build their own SDL and SDL_net frameworks, the recommended way
is to download the latest release version of AlephOne. 

	Control-Click on AlephOne and select Show Package Contents.
	Navigate to Contents/Frameworks
	Copy the SDL.framework and SDL_net.framework directories to
		/Library/Frameworks
	
Even if you do build the SDL Frameworks yourself, AlephOne's ProjectBuilder
project expects to find them in /Library/Frameworks


AlephOne requires Lua 5.0 for Lua scripting support. Lua is available from the
official Lua site:

	http://www.lua.org/

Installing Lua:

	Download http://www.lua.org/ftp/lua-5.0.tar.gz
	Uncompress the source (Stuffit works)
	Open Terminal
	cd into the lua-5.0 directory
	Edit the file 'config' so that install root is
		/Library/Application\ Support/Lua/lua-5.0
	make
	make install
	make clean
	cd /Library/Application\ Support/Lua/lua-5.0/lib
	ranlib *


AlephOne requires Speex 1.0 for handling network microphone encoding/decoding.
Speex is available from the official site:

	http://www.speex.org/
	
Installing Speex:

	Download http://www.speex.org/download/speex-1.0.tar.gz
		(1.0 Source Code)
	Uncompress
	Open Terminal
	cd into speex directory
	setenv CFLAGS -O3
	./configure --enable-shared=no
	make
	Make directory /Library/Application Support/Speex
	Move speex-1.0 directory into /Library/Application Support/Speex

Building and Installing the Program
-----------------------------------

Beginning after using the Command-Line CVS Tools:

	Go to the aleph/PBProjects directory
	sh fix-cvs-resources.sh
	
This decodes all of the files that had been encoded within CVS to preserve their
resource forks.

To Compile AlephOne-Carbon:

Go to the aleph/PBProjects directory and open the AlephOne-OSX.pbproj within
Project Builder.

Select AlephOne-Carbon from the menu of targets. (By default it opens to
building the AlephOne/SDL version).

Now, build the program (click on the hammer icon or choose "Build" from the
Build menu).

If all goes well, you'll have an executable called "AlephOne" in
aleph/PBProjects/build. You can run this from within Project Builder (probably a
good way to test the program, as Project Builder will give you console output),
or move this wherever you want it installed.

For the curious the build flags currently are:
	-Dmac -DTARGET_API_MAC_CARBON  -DUSE_CARBON_ACCESSORS -DSUPPRESS_MACOS_CLASSIC -DUSE_SHEETS=1 -DHAVE_SDL_NET=1 -Wno-deprecated -DHAVE_UNISTD_H -x c++  -DHAVE_CORE_GRAPHICS -DAPPLICATION_IS_BUNDLED -DEXPLICIT_CARBON_HEADER -DHAVE_LUA -DSPEEX


	mac
		Compile the Macintosh UI code
	TARGET_API_MAC_CARBON
		When defined this means we are targeting the Carbon APIs
	USE_CARBON_ACCESSORS
		When defined this means accessors for certain structure fields should be
		used in a prelinary step towards full Carbon usage
	SUPPRESS_MACOS_CLASSIC
		When defined this prevents older MacOS Features that are unavailable
		under OSX from being compiled
	USE_SHEETS=1
		When defined as true (1) this specifies that MacOSX sheets should be
		used in dialogs, where applicable.
	HAVE_SDL_NET=1
		When defined as true (1) thisspecifies that SDL style networking should
		be used.
	APPLICATION_IS_BUNDLED
		When defined this means the application should expect to find itself
		arranged in a standalone bundle (As opposed to a single executable under
		prior MacOS versions)
	EXPLICIT_CARBON_HEADER
		Distinguishes between whether files should be including Carbon.h or
		Carbon/Carbon.h like when using the Carbon frameworks in ProjectBuilder.
	HAVE_LUA
		When defined, Lua support is compiled in.
	SPEEX
		When defined, Speex support for network microphone is compiled in.

To Compile AlephOne-Carbon Standalone:

Follow the directions for compiling AlephOne-Carbon, however select Carbon
Standalone for the compile target.
		
Alas, this step is necessary at least once for the ProjectBuilder debugger
utilities to work with the AlephOne App. But after the first Standalone
build you can switch back to the basic AlephOne-Carbon target.

Installing the data files
-------------------------

The MacOSX/Carbon version of AlephOne uses the exact same data files with
resources as the MacOS Classic version of AlephOne does. You'll need to download
either original AlephOne data files or copy the data files from a copy of
Marathon 2, Marathon Infinity, or Marathon Trilogy Box Set CD.

The MacOSX/Carbon version of AlephOne expects the data files to be in the same
directory as the AlephOne application.

Issues, Bugs, and annoyances
----------------------------

Resource forks from must be expanded manually with
aleph/PBProjects/fix-cvs-resources.sh when using the command line tools.

NIBS Support is available, but isn't currently compiled in the default setup.

For reporting or checking up on bugs check the bug tracking for AlephOne on
SourceForge.net

	https://sourceforge.net/tracker/?group_id=1997&atid=101997