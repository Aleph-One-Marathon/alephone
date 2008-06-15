## Process this file with automake to produce Makefile.in 

SUBDIRS = Source_Files tools data

# Requires automake 1.5
AUTOMAKE_OPTIONS = 1.5 foreign

EXTRA_DIST = COPYING.SDL INSTALL.BeOS INSTALL.Unix INSTALL.Windows Makefile.BeOS \
  docs/MML.html docs/Cheat_Codes data/Fonts Resources/Windows/alephone.ico Resources/Windows/alephone.rc PBProjects/AlephOne-OSX10.4.xcodeproj/project.pbxproj PBProjects/English.lproj/InfoPlist.strings PBProjects/FilmIcon.icns PBProjects/ImagesIcon.icns PBProjects/MapIcon.icns PBProjects/MMLIcon.icns PBProjects/MusakIcon.icns PBProjects/PFIcon.icns PBProjects/PhysIcon.icns PBProjects/SaveIcon.icns PBProjects/ShapesIcon.icns PBProjects/SoundsIcon.icns PBProjects/Info-AlephOne_SDL.plist PBProjects/precompiled_headers.h PBProjects/SDLMain.h PBProjects/SDLMain.m PBProjects/AlephOne.icns Expat/xmltok/nametab.h PBProjects/config.h PBProjects/confpaths.h data/AlephSansMono-Bold.ttf data/AlephSansMonoLicense.txt

man_MANS = docs/alephone.6

WINDISTDIR=$(PACKAGE)-$(VERSION)

dist-hook: AlephOne.spec
	cp AlephOne.spec $(distdir)
	cp "Aleph One Classic SDL.mcp" $(distdir)
	cp "Resources/Aleph One Classic SDL.r" $(distdir)/Resources
	mkdir -p ${srcdir}/Libraries
	cp -r ${srcdir}/Libraries ${distdir}/Libraries
	rmdir --ignore-fail-on-non-empty ${srcdir}/Libraries
	rmdir --ignore-fail-on-non-empty ${distdir}/Libraries

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta --clean $(PACKAGE)-$(VERSION).tar.gz

ChangeLog:
	cvs2cl --show-dead -S

# Windows version
$(PACKAGE)-$(VERSION).zip: Source_Files/AlephOne.exe
	mkdir $(WINDISTDIR)
	cp Source_Files/AlephOne.exe $(WINDISTDIR)/
	unix2dos -n AUTHORS $(WINDISTDIR)/AUTHORS.txt
	unix2dos -n COPYING $(WINDISTDIR)/COPYING.txt
	unix2dos -n INSTALL.Windows $(WINDISTDIR)/INSTALL.Windows.txt
	unix2dos -n README $(WINDISTDIR)/README.txt
	mkdir $(WINDISTDIR)/docs
	cp docs/Lua.html $(WINDISTDIR)/docs/Lua.html
	cp docs/MML.html $(WINDISTDIR)/docs/MML.html
	unix2dos -n docs/Troubleshooting.txt $(WINDISTDIR)/docs/Troubleshooting.txt
	mkdir $(WINDISTDIR)/Themes
	cp -r data/default_theme $(WINDISTDIR)/Themes/Default
	mkdir $(WINDISTDIR)/Extras
	cp data/Software_Transparent_Liquids.mml $(WINDISTDIR)/Extras
	cp data/Transparent_Liquids.mml $(WINDISTDIR)/Extras
	cp data/Transparent_Sprites.mml $(WINDISTDIR)/Extras
	cp data/Fonts $(WINDISTDIR)/
	cp -r "Resources/Library Licenses" $(WINDISTDIR)/
	zip -r $(PACKAGE)-$(VERSION).zip $(WINDISTDIR)
	rm -rf $(WINDISTDIR)

winzip: $(PACKAGE)-$(VERSION).zip

FORCE: