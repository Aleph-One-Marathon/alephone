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

As of the current time, no libraries are required. In the future SDL_net may
become a requirement as networking code is synced between the Macintosh and SDL
interfaces.

Building and Installing the Program
-----------------------------------

Go to the aleph/PBProjects directory and open the AlephOne-OSX.pbproj within
Project Builder.

Select AlephOne-Carbon from the menu of targets. (It open defaults to just AlephOne
for the SDL version).

Now, build the program (click on the hammer icon or choose "Build" from the Build menu).

If all goes well, you'll have an executable called "AlephOne" in
aleph/PBProjects/build. You can run this from within Project Builder (probably a
good way to test the program, as Project Builder will give you console output),
or move this wherever you want it installed.

For the curious the build flags currently are:
	-Dmac -DTARGET_API_MAC_CARBON -DUSE_CARBON_ACCESSORS -DSUPPRESS_MACOS_CLASSIC -DUSE_SHEETS=1

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

The software renderer seems extremely slow.

Choice of Frequencies for full screen display is funky. Also leads to
AlephOne getting confused about screen capabilities. (Fires off the warning
that AlephOne requires a screen capable of 256 colors or more and is 13" or
bigger) and to a corruption of the preferences file.

Network play is not hooked up in this build.

Screen fading is not working in this build.

After suspending and resuming AlephOne it is possible the cursor won't disappear.

Marathon 2's starting music doesn't play because it's an AIFF with
a different file type (mus2) so Quicktime doesn't recognize it.
It seems more trouble than it's worth to implement. Map makers who
want startup audio can use a real file type AIFF or a movie.