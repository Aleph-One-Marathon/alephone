Aleph One/Carbon MacOSX Installation Instructions
==============================================

The supplied project files are for Apple's Project Builder (I'm using version
1.0.1, but an earlier version might work). If the development tools weren't
included in your OSX installation, you can download them from
http://connect.apple.com. You must be a registered Apple Developer Connection
(ADC) member, but a free registration is available which gives you access to a
version of the developer tools.

This is not the MacOS X SDL version, but the Carbonized version based on the
MacOS 7.x-9.x/Classic UI code.

Required Libraries
------------------

Aleph One/Carbon with SDL networking for MacOSX requires the Simple DirectMedia
Layer (SDL) library, available from the official SDL site:

  http://www.libsdl.org/

Aleph One expects SDL to be installed as a Framework, which is MacOSX's fancy
version of a shared library. If you build SDL from source use Project Builder
rather than command line tools, because (as far as I can tell) the command line
tools do not build framework versions. The developer package available at the
SDL webpage does install as a framework.

The SDL_net library, available from the "Libraries" section of the SDL site, is
also required.


Building and Installing the Program
-----------------------------------

Go to the aleph/PBProjects directory and open the AlephOne-OSX.pbproj within
Project Builder.

Select AlephOne-Carbon from the menu of targets. (By default it opens to building the
AlephOne/SDL version).

Now, build the program (click on the hammer icon or choose "Build" from the Build menu).

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
resources as the MacOS Classic version of AlephOne does. You'll need to
download either original AlephOne data files or copy the data files from
a copy of Marathon 2, Marathon Infinity, or Marathon Trilogy Box Set CD.

The MacOSX/Carbon version of AlephOne expects the data files to be
in the same directory as the AlephOne application.

Issues, Bugs, and annoyances
----------------------------

Choice of Frequencies for full screen display is funky. Also leads to
AlephOne getting confused about screen capabilities. (Fires off the warning
that AlephOne requires a screen capable of 256 colors or more and is 13" or
bigger) and to a corruption of the preferences file.

Screen fading is operational. But there appears to be a race condition
between when screen updates occur and when the screen gamma is changed. (The screen
can be updated, but not yet flushed to the window buffer when you start changing
screen gamma. Lots of short term artifacts)

After suspending and resuming AlephOne it is possible the cursor won't disappear.

Marathon 2's starting music doesn't play because it's an AIFF with
a different file type (mus2) so Quicktime doesn't recognize it.
It seems more trouble than it's worth to implement. Map makers who
want startup audio can use a real file type AIFF or a movie.

Opening and closing the OpenGL preferences results in the word 'ground'
as showing up funky or AlephOne crashing due to a System called TE Event
Handler (There's no editable text items in the dialog....)

When a preferences sheet is open, the preferences dialog isn't draggable.

The Customize Keyboard dialog is not a sheet. It's seperator lines
aren't drawn by proper user procs. And it wasn't quite made a sheet as I
was worried about the dialog's size and small resolutions.

The mouse cannot currently be inverted.

There's a very strong likelihood that the dialog box to choose which monitor
on a Mac with multiple monitors doesn't actually do anything.

The Gather dialog often has the ok button active even if it should be deactivated.
For instance, when no other users than the host have been gathered.

Network play is not necessarilly full features, or even stable. There isn't
even a way for the gatherer to send messages to the player as the SDL version
purports. It is included in the build because otherwise there's no networking.
Hopefully with partial networking either things will come together as networking
in general comes together or as someone focuses on the Carbon build details.

There is currently no way to build without Networking. (And without requireing the
SDL libraries either)

The resulting executable currently requires SDL and SDL_Net frameworks to be
installed. In the future we hope to install these as private frameworks of the
application. In order to do this yourself, download the source for both these
libraries and follow Project Builder's documentation on preparing a framework
for inclusion within an application. Once you have the runtime lib all prepared
you can have Project Builder install that into the private frameworks directory
of the AlephOne Carbon application.