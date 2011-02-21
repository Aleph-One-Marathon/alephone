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

FORCE: