Aleph One/Carbon MacOSX Installation Instructions
=================================================

The supplied project files are for Apple's Project Builder (I'm using version
1.0.1, but an earlier version might work). If the development tools weren't
included in your OSX installation, you can download them from
http://connect.apple.com. You must be a registered Apple Developer Connection
(ADC) member, but a free registration is available which gives you access to a
version of the developer tools.

This is not the MacOS X SDL version, but the Carbonized version based on the
MacOS 7.x-9.x/Classic UI code.

Required Tools
--------------

	* Apple's OSX Developer release (Project Builder, et al.)
	* MacCVS Pro

As mentioned above, If the development tools weren't included in your OSX
installation, you can download them from http://connect.apple.com. You must be a
registered Apple Developer Connection (ADC) member, but a free registration is
available which gives you access to a version of the developer tools.

The Carbon version of MacCVS Pro 2.7d3 or later can be downloaded from
http://www.maccvs.org. If you previously downloaded the AlephOne sources through
the command line CVS tools, I'm sorry but you really should grab MacCVS Pro and
download the AlephOne through that. Some of the resource and demo files still have
resource forks and are stored in CVS in applesingle format. You may be able to
decode them using Stuffit Expander, but it does add fiddling.

After setting up a session file for AlephOne within MacCVS Pro you may wish to
go to Session Settings and choose the File Translation and Encoding settings. It
can be a little more convenient if you set the Code Warrior creators (CWIE) to
something nonsensical like ' ' (a single space) and setting linefeeds to Unix.
Thus letting you use FileMerge to compare files and to have all the files open
by default within ProjectBuilder. This isn't necessary, just potentially useful.

More Information on configuring MacCVS Pro for Aleph can be found at:
http://source.bungie.org/_enginedevelopment/cvs/aboutcvs.html

There is a gotcha currently. If you are using MacCVS Pro, one of the required
files has too long. After checking things out you will need to rename it
manually in order to compile.

Go to aleph/Source_Files/Network/
and rename network_microph…e_sdl_dummy.cpp to network_microphone_sdl_dummy.cpp

Hopefully at some point MacCVS Pro will get around this filename limit or the
file will be renamed appropriately. Even without renaming, MacCVS Pro would
half lose track of the files.

Required Libraries
------------------

	* SDL
	* SDL_net

Aleph One/Carbon with SDL networking for MacOSX requires the Simple DirectMedia
Layer (SDL) library, available from the official SDL site:

  http://www.libsdl.org/

Aleph One expects SDL to be installed as a Framework, which is MacOSX's fancy
version of a shared library. If you build SDL from source use Project Builder
rather than command line tools, because (as far as I can tell) the command line
tools do not build frameworks. The developer packages available at the
SDL webpage does install as a framework.

The SDL_net library, available from the "Libraries" section of the SDL site, is
also required.

For the standalone version, instead of downloading the installers for development
libraries and runtime libraries, one must download the source code for both SDL
and SDL_net.

Building and Installing the Program
-----------------------------------

If you aren't building the standalone version, skip down to compiling
AlephOne-Carbon.

Before you can build the standalone version you must build your own copies of
SDL and SDL_net. Why? Because we need to prepare the libraries for inclusion as
private frameworks. By default, if a program can't locate the libraries in the
usual places (User space, shared space and system space Library/Frameworks
directories) then it uses the install path listed in the original library it was
compiled from. Except that for the binary distributions of SDL and SDL_net this
is something useless for us like /Users/henry/... No idea if requesting this
would be a bug request or a feature request for the particular libraries, or if
it would just be pushed back as 'if you want to do it, compile the variance
yourself as we'd rather have everyone actually install the shared libs'

Anyhow.

To Recompile SDL:

	Unarchive the SDL source code
	Unarchive PBProjects.tar.gz within it
	Open PBProjects/SDL/SDL.pbproj
	Open the Targets Tab on the left
	Select Framework
	Select the Build Tab for the Target: Framework
	Under Installation Location
		Select Path for the install location (as opposed to None)
		Changed Path to: @executable_path/../Frameworks
	Under Build Settings
		Add SKIP_INSTALL=YES
	Now select Standard Package as the product to be built. Select Deployment as the
	build style and build it.

Under PBProjects/SDL/build you'll find the installer it created. Run that to
install these libraries into /Library/Framework. Yes they should work fine from
there as SDL runtime libraries for any other SDL projects you may have.

To Recompile SDL_net:

The process is almost exactly the same. And you need to do it *after* you've
already rebuilt SDL otherwise it'll still be looking for SDL in the wrong place.

There is one difference, in addition to Installation Location and Build Setting
changes you need to add:
	Under Other Linker Flags
		Add to the line:  -seg1addr 0x30040000

Otherwise SDL_net will clash with other segments and prevent prebinding of the
AlephOne application. Not fatal, but not optimal either.

Once both SDL and SDL_net are compiled and installed procede on to compile
AlephOne. One difference though, instead of building AlephOne-Carbon, select
Carbon Standalone from the menu of targets. It depends upon and will compile
AlephOne-Carbon for you and then copy the SDL and SDL_net frameworks into
AlephOne's private Frameworks directory for distribution.

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
	-Dmac -DTARGET_API_MAC_CARBON  -DUSE_CARBON_ACCESSORS -DSUPPRESS_MACOS_CLASSIC -DUSE_SHEETS=1 -DHAVE_SDL_NET=1


	mac
		Compile the Macintosh UI code
	TARGET_API_MAC_CARBON
		When defined means we are targeting the Carbon APIs
	USE_CARBON_ACCESSORS
		When defined means accessors for certain structure fields should be
		used in a prelinary step towards full Carbon usage
	SUPPRESS_MACOS_CLASSIC
		When defined it prevents older MacOS Features that are unavailable
		under OSX from being compiled
	USE_SHEETS=1
		When defined as true (1) specifies that MacOSX sheets should be used
		in dialogs, where applicable.
	HAVE_SDL_NET=1
		When defined as true (1) specifies that SDL style networking should
		be used.
		
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

Choice of Frequencies for full screen display is funky. Also leads to AlephOne
getting confused about screen capabilities. (Fires off the warning that AlephOne
requires a screen capable of 256 colors or more and is 13" or bigger) and to a
corruption of the preferences file.

Marathon 2's starting music doesn't play because it's an AIFF with a different
file type (mus2) so Quicktime doesn't recognize it. It seems more trouble than
it's worth to implement. Map makers who want startup audio can use a real file
type AIFF or a movie.

Opening and closing the OpenGL preferences results in the word 'ground' as
showing up funky or AlephOne crashing due to a System called TE Event Handler
(There's no editable text items in the dialog....)

When a preferences sheet is open, the preferences dialog isn't draggable.

The Customize Keyboard dialog is not a sheet. It's seperator lines aren't drawn
by proper user procs. And it wasn't quite made a sheet as I was worried about
the dialog's size and small resolutions.

The mouse cannot currently be inverted.

There's a very strong likelihood that the dialog box to choose which monitor on
a Mac with multiple monitors doesn't actually do anything.

The Gather dialog often has the ok button active even if it should be
deactivated. For instance, when no other users than the host have been gathered.

Network play is not necessarilly full featured, or even stable. There isn't even
a way for the gatherer to send messages to the player as the SDL version
purports. It is included in the build because otherwise there's no networking.
Hopefully with partial networking either things will come together as networking
in general comes together or as someone focuses on the Carbon build details.

There is currently no way to build without Networking. (And without requireing
the SDL libraries either)
