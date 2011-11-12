## Process this file with automake to produce Makefile.in 

SUBDIRS = Source_Files tools data

# Requires automake 1.5
AUTOMAKE_OPTIONS = 1.5 foreign

EXTRA_DIST = COPYING.SDL INSTALL.BeOS INSTALL.Unix INSTALL.Windows	\
  Makefile.BeOS docs/MML.html docs/Lua.html docs/Lua_HUD.html		\
  data/Fonts Resources/Windows/alephone.ico				\
  Resources/Windows/alephone.rc						\
  PBProjects/AlephOne.xcodeproj/project.pbxproj			\
  PBProjects/English.lproj/InfoPlist.strings PBProjects/FilmIcon.icns	\
  PBProjects/ImagesIcon.icns PBProjects/MapIcon.icns			\
  PBProjects/MMLIcon.icns PBProjects/MusakIcon.icns			\
  PBProjects/PFIcon.icns PBProjects/PhysIcon.icns			\
  PBProjects/SaveIcon.icns PBProjects/ShapesIcon.icns			\
  PBProjects/SoundsIcon.icns PBProjects/Info-AlephOne_SDL.plist		\
  PBProjects/precompiled_headers.h PBProjects/SDLMain.h			\
  PBProjects/SDLMain.m PBProjects/AlephOne.icns Expat/xmltok/nametab.h	\
  PBProjects/config.h PBProjects/confpaths.h				\
  data/AlephSansMono-Bold.ttf data/AlephSansMonoLicense.txt		\
  docs/alephone.6 examples/lua/Cheats.lua

man_MANS = docs/alephone.6

WINDISTDIR=$(PACKAGE)-$(VERSION)

dist-hook: AlephOne.spec
	cp AlephOne.spec $(distdir)
	cp "$(srcdir)/Aleph One Classic SDL.mcp" $(distdir)
	cp "$(srcdir)/Resources/Aleph One Classic SDL.r" $(distdir)/Resources

$(PACKAGE)-$(VERSION)-MacWinSrc.7z: distdir
	cp -r ${srcdir}/Libraries ${distdir}/
	tar cf - $(distdir) --exclude-vcs | xz > $(PACKAGE)-$(VERSION)-MacWinSrc.xz 
	$(am__remove_distdir)

libsrc: $(PACKAGE)-$(VERSION)-MacWinSrc.7z

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta --clean $(PACKAGE)-$(VERSION).tar.gz

ChangeLog:
	svn2cl --break-before-msg

# Windows version
$(PACKAGE)-$(VERSION).zip: Source_Files/AlephOne.exe
	mkdir $(WINDISTDIR)
	cp Source_Files/AlephOne.exe $(WINDISTDIR)/
	unix2dos -n $(srcdir)/AUTHORS $(WINDISTDIR)/AUTHORS.txt
	unix2dos -n $(srcdir)/COPYING $(WINDISTDIR)/COPYING.txt
	unix2dos -n $(srcdir)/INSTALL.Windows $(WINDISTDIR)/INSTALL.Windows.txt
	unix2dos -n $(srcdir)/README $(WINDISTDIR)/README.txt
	mkdir $(WINDISTDIR)/docs
	cp $(srcdir)/docs/Lua.html $(WINDISTDIR)/docs/Lua.html
	cp $(srcdir)/docs/Lua_HUD.html $(WINDISTDIR)/docs/Lua_HUD.html
	cp $(srcdir)/docs/MML.html $(WINDISTDIR)/docs/MML.html
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt $(WINDISTDIR)/docs/Troubleshooting.txt
	mkdir $(WINDISTDIR)/Themes
	cp -r $(srcdir)/data/default_theme $(WINDISTDIR)/Themes/Default
	rm -rf "$(WINDISTDIR)/Themes/Default/.svn"
	mkdir $(WINDISTDIR)/Extras
	cp $(srcdir)/data/Software_Transparent_Liquids.mml $(WINDISTDIR)/Extras
	cp $(srcdir)/data/Transparent_Liquids.mml $(WINDISTDIR)/Extras
	cp $(srcdir)/data/Transparent_Sprites.mml $(WINDISTDIR)/Extras
	cp $(srcdir)/data/Carnage_Messages.mml $(WINDISTDIR)/Extras
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua $(WINDISTDIR)/Extras/Cheats.lua
	cp $(srcdir)/data/Fonts $(WINDISTDIR)/
	cp -r "$(srcdir)/Resources/Library Licenses" $(WINDISTDIR)/
	rm -rf "$(WINDISTDIR)/Library Licenses/.svn"
	zip -r $(PACKAGE)-$(VERSION).zip $(WINDISTDIR)
	rm -rf "$(WINDISTDIR)"

winzip: $(PACKAGE)-$(VERSION).zip

$(PACKAGE)-$(VERSION)-M1A1.zip: Source_Files/AlephOne.exe
	cp -r "$(srcdir)/data/Scenarios/M1A1" $(WINDISTDIR)-M1A1
	rm -rf "$(WINDISTDIR)-M1A1/.svn"
	rm -rf "$(WINDISTDIR)-M1A1/Physics Models/.svn"
	rm -rf "$(WINDISTDIR)-M1A1/Plugins/.svn"
	rm -rf $(WINDISTDIR)-M1A1/Plugins/*/.svn
	rm -rf $(WINDISTDIR)-M1A1/Plugins/*/*/.svn
	rm -rf $(WINDISTDIR)-M1A1/Plugins/*/*/*/.svn
	rm -rf "$(WINDISTDIR)-M1A1/Scripts/.svn"
	rm -rf "$(WINDISTDIR)-M1A1/Tracks/.svn"
	cp Source_Files/AlephOne.exe $(WINDISTDIR)-M1A1/
	unix2dos -n $(srcdir)/AUTHORS $(WINDISTDIR)-M1A1/AUTHORS.txt
	unix2dos -n $(srcdir)/COPYING $(WINDISTDIR)-M1A1/COPYING.txt
	unix2dos -n $(srcdir)/INSTALL.Windows $(WINDISTDIR)-M1A1/INSTALL.Windows.txt
	unix2dos -n $(srcdir)/README $(WINDISTDIR)-M1A1/README.txt
	mkdir $(WINDISTDIR)-M1A1/docs
	cp $(srcdir)/docs/Lua.html $(WINDISTDIR)-M1A1/docs/Lua.html
	cp $(srcdir)/docs/Lua_HUD.html $(WINDISTDIR)-M1A1/docs/Lua_HUD.html
	cp $(srcdir)/docs/MML.html $(WINDISTDIR)-M1A1/docs/MML.html
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt $(WINDISTDIR)-M1A1/docs/Troubleshooting.txt
	mkdir $(WINDISTDIR)-M1A1/Themes
	cp -r $(srcdir)/data/default_theme $(WINDISTDIR)-M1A1/Themes/Default
	rm -rf "$(WINDISTDIR)-M1A1/Themes/Default/.svn"
	mkdir $(WINDISTDIR)-M1A1/Extras
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua $(WINDISTDIR)-M1A1/Extras/Cheats.lua
	cp $(srcdir)/data/Fonts $(WINDISTDIR)-M1A1/Fonts.fntA
	cp -r "$(srcdir)/Resources/Library Licenses" $(WINDISTDIR)-M1A1/
	rm -rf "$(WINDISTDIR)-M1A1/Library Licenses/.svn"
	zip -r $(PACKAGE)-$(VERSION)-M1A1.zip $(WINDISTDIR)-M1A1
	rm -rf "$(WINDISTDIR)-M1A1"

win1zip: $(PACKAGE)-$(VERSION)-M1A1.zip

$(PACKAGE)-$(VERSION)-Durandal.zip: Source_Files/AlephOne.exe
	cp -r "$(srcdir)/data/Scenarios/Marathon 2" $(WINDISTDIR)-Durandal
	cp -r $(srcdir)/data/XBLA_HUD $(WINDISTDIR)-Durandal/Plugins/XBLA_HUD
	rm -rf "$(WINDISTDIR)-Durandal/.svn"
	rm -rf "$(WINDISTDIR)-Durandal/Physics Models/.svn"
	rm -rf "$(WINDISTDIR)-Durandal/Plugins/.svn"
	rm -rf $(WINDISTDIR)-Durandal/Plugins/*/.svn
	rm -rf $(WINDISTDIR)-Durandal/Plugins/*/*/.svn
	rm -rf $(WINDISTDIR)-Durandal/Plugins/*/*/*/.svn
	rm -rf $(WINDISTDIR)-Durandal/Plugins/*/*/*/*/.svn
	rm -rf "$(WINDISTDIR)-Durandal/Scripts/.svn"
	cp Source_Files/AlephOne.exe $(WINDISTDIR)-Durandal/
	unix2dos -n $(srcdir)/AUTHORS $(WINDISTDIR)-Durandal/AUTHORS.txt
	unix2dos -n $(srcdir)/COPYING $(WINDISTDIR)-Durandal/COPYING.txt
	unix2dos -n $(srcdir)/INSTALL.Windows $(WINDISTDIR)-Durandal/INSTALL.Windows.txt
	unix2dos -n $(srcdir)/README $(WINDISTDIR)-Durandal/README.txt
	mkdir $(WINDISTDIR)-Durandal/docs
	cp $(srcdir)/docs/Lua.html $(WINDISTDIR)-Durandal/docs/Lua.html
	cp $(srcdir)/docs/Lua_HUD.html $(WINDISTDIR)-Durandal/docs/Lua_HUD.html
	cp $(srcdir)/docs/MML.html $(WINDISTDIR)-Durandal/docs/MML.html
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt $(WINDISTDIR)-Durandal/docs/Troubleshooting.txt
	mkdir $(WINDISTDIR)-Durandal/Themes
	cp -r $(srcdir)/data/default_theme $(WINDISTDIR)-Durandal/Themes/Default
	rm -rf "$(WINDISTDIR)-Durandal/Themes/Default/.svn"
	mkdir $(WINDISTDIR)-Durandal/Extras
	cp $(srcdir)/data/Software_Transparent_Liquids.mml $(WINDISTDIR)-Durandal/Extras
	cp $(srcdir)/data/Transparent_Sprites.mml $(WINDISTDIR)-Durandal/Extras
	cp $(srcdir)/data/Carnage_Messages.mml $(WINDISTDIR)-Durandal/Extras
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua $(WINDISTDIR)-Durandal/Extras/Cheats.lua
	cp -r "$(srcdir)/Resources/Library Licenses" $(WINDISTDIR)-Durandal/
	rm -rf "$(WINDISTDIR)-Durandal/Library Licenses/.svn"
	zip -r $(PACKAGE)-$(VERSION)-Durandal.zip $(WINDISTDIR)-Durandal
	rm -rf "$(WINDISTDIR)-Durandal"

win2zip: $(PACKAGE)-$(VERSION)-Durandal.zip

$(PACKAGE)-$(VERSION)-Infinity.zip: Source_Files/AlephOne.exe
	cp -r "$(srcdir)/data/Scenarios/Marathon Infinity" $(WINDISTDIR)-Infinity
	mkdir $(WINDISTDIR)-Infinity/Plugins
	cp -r $(srcdir)/data/XBLA_HUD $(WINDISTDIR)-Infinity/Plugins/XBLA_HUD
	rm -rf "$(WINDISTDIR)-Infinity/.svn"
	rm -rf "$(WINDISTDIR)-Infinity/Physics Models/.svn"
	rm -rf $(WINDISTDIR)-Infinity/Plugins/*/.svn
	rm -rf $(WINDISTDIR)-Infinity/Plugins/*/*/.svn
	rm -rf "$(WINDISTDIR)-Infinity/Scripts/.svn"
	cp Source_Files/AlephOne.exe $(WINDISTDIR)-Infinity/
	unix2dos -n $(srcdir)/AUTHORS $(WINDISTDIR)-Infinity/AUTHORS.txt
	unix2dos -n $(srcdir)/COPYING $(WINDISTDIR)-Infinity/COPYING.txt
	unix2dos -n $(srcdir)/INSTALL.Windows $(WINDISTDIR)-Infinity/INSTALL.Windows.txt
	unix2dos -n $(srcdir)/README $(WINDISTDIR)-Infinity/README.txt
	mkdir $(WINDISTDIR)-Infinity/docs
	cp $(srcdir)/docs/Lua.html $(WINDISTDIR)-Infinity/docs/Lua.html
	cp $(srcdir)/docs/Lua_HUD.html $(WINDISTDIR)-Infinity/docs/Lua_HUD.html
	cp $(srcdir)/docs/MML.html $(WINDISTDIR)-Infinity/docs/MML.html
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt $(WINDISTDIR)-Infinity/docs/Troubleshooting.txt
	mkdir $(WINDISTDIR)-Infinity/Themes
	cp -r $(srcdir)/data/default_theme $(WINDISTDIR)-Infinity/Themes/Default
	rm -rf "$(WINDISTDIR)-Infinity/Themes/Default/.svn"
	mkdir $(WINDISTDIR)-Infinity/Extras
	cp $(srcdir)/data/Software_Transparent_Liquids.mml $(WINDISTDIR)-Infinity/Extras
	cp $(srcdir)/data/Transparent_Sprites.mml $(WINDISTDIR)-Infinity/Extras
	cp $(srcdir)/data/Carnage_Messages.mml $(WINDISTDIR)-Infinity/Extras
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua $(WINDISTDIR)-Infinity/Extras/Cheats.lua
	cp -r "$(srcdir)/Resources/Library Licenses" $(WINDISTDIR)-Infinity/
	rm -rf "$(WINDISTDIR)-Infinity/Library Licenses/.svn"
	zip -r $(PACKAGE)-$(VERSION)-Infinity.zip $(WINDISTDIR)-Infinity
	rm -rf "$(WINDISTDIR)-Infinity"

win3zip: $(PACKAGE)-$(VERSION)-Infinity.zip

FORCE:
