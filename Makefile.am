## Process this file with automake to produce Makefile.in 

SUBDIRS = Source_Files tools data

# Requires automake 1.5
AUTOMAKE_OPTIONS = 1.5 foreign

EXTRA_DIST = COPYING.SDL INSTALL.BeOS INSTALL.Unix INSTALL.Windows	\
  Makefile.BeOS docs/MML.html docs/Lua.html docs/Lua_HUD.html		\
  data/Fonts Resources/Windows/alephone.ico				\
  Resources/Windows/alephone.rc						\
  Resources/Windows/marathon.rc Resources/Windows/marathon.ico          \
  Resources/Windows/marathon2.rc Resources/Windows/marathon2.ico        \
  Resources/Windows/marathon-infinity.rc                                \
  Resources/Windows/marathon-infinity.ico                               \
  PBProjects/AlephOne.xcodeproj/project.pbxproj				\
  PBProjects/English.lproj/InfoPlist.strings PBProjects/FilmIcon.icns	\
  PBProjects/ImagesIcon.icns PBProjects/MapIcon.icns			\
  PBProjects/MMLIcon.icns PBProjects/MusakIcon.icns			\
  PBProjects/PhysIcon.icns			                        \
  PBProjects/SaveIcon.icns PBProjects/ShapesIcon.icns			\
  PBProjects/SoundsIcon.icns PBProjects/Info-AlephOne_SDL.plist		\
  PBProjects/precompiled_headers.h PBProjects/SDLMain.h			\
  PBProjects/SDLMain.m PBProjects/AlephOne.icns				\
  Expat/xmltok/nametab.h PBProjects/config.h PBProjects/confpaths.h	\
  data/AlephSansMono-Bold.ttf data/AlephSansMonoLicense.txt		\
  docs/alephone.6 examples/lua/Cheats.lua THANKS			\
  data/powered-by-alephone.svg						\
  PBProjects/AlephOne-Xcode4.xcodeproj/project.pbxproj\
  PBProjects/AlephOne-Xcode4.xcodeproj/project.xcworkspace/contents.xcworkspacedata\
  PBProjects/Info-AlephOne-Xcode4.plist\
	PBProjects/AppStore/Marathon/Marathon.plist \
	PBProjects/AppStore/Marathon/Marathon.icns \
	PBProjects/AppStore/Marathon/English.lproj/InfoPlist.strings

man_MANS = docs/alephone.6

WINDISTDIR=$(PACKAGE)-$(VERSION)

dist-hook: AlephOne.spec
	cp AlephOne.spec $(distdir)
	cp "$(srcdir)/Aleph One Classic SDL.mcp" $(distdir)
	cp "$(srcdir)/Resources/Aleph One Classic SDL.r" $(distdir)/Resources
	mkdir -p "$(distdir)/PBProjects/AppStore/Marathon 2/English.lproj"
	cp "$(srcdir)/PBProjects/AppStore/Marathon 2/Marathon 2.plist" "$(distdir)/PBProjects/AppStore/Marathon 2/"
	cp "$(srcdir)/PBProjects/AppStore/Marathon 2/Marathon 2.icns" "$(distdir)/PBProjects/AppStore/Marathon 2/"
	cp "$(srcdir)/PBProjects/AppStore/Marathon 2/English.lproj/InfoPlist.strings" "$(distdir)/PBProjects/AppStore/Marathon 2/English.lproj/"
	mkdir -p "$(distdir)/PBProjects/AppStore/Marathon Infinity/English.lproj"
	cp "$(srcdir)/PBProjects/AppStore/Marathon Infinity/Marathon Infinity.plist" "$(distdir)/PBProjects/AppStore/Marathon Infinity/"
	cp "$(srcdir)/PBProjects/AppStore/Marathon Infinity/Marathon Infinity.icns" "$(distdir)/PBProjects/AppStore/Marathon Infinity/"
	cp "$(srcdir)/PBProjects/AppStore/Marathon Infinity/English.lproj/InfoPlist.strings" "$(distdir)/PBProjects/AppStore/Marathon Infinity/English.lproj/"


$(PACKAGE)-$(VERSION)-MacWinLibSrc.tar.bz2: Libraries
	tar cjf $(PACKAGE)-$(VERSION)-MacWinLibSrc.tar.bz2 -C $(srcdir) --exclude '.svn' Libraries 

libsrc: $(PACKAGE)-$(VERSION)-MacWinLibSrc.tar.bz2

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta --clean $(PACKAGE)-$(VERSION).tar.gz

ChangeLog:
	svn2cl --break-before-msg

define windist_common
unix2dos -n $(srcdir)/THANKS "$(1)/THANKS.txt"
unix2dos -n $(srcdir)/COPYING "$(1)/COPYING.txt"
unix2dos -n $(srcdir)/INSTALL.Windows "$(1)/INSTALL.Windows.txt"
unix2dos -n $(srcdir)/README "$(1)/README.txt"
mkdir "$(1)/docs"
cp $(srcdir)/docs/Lua.html "$(1)/docs/Lua.html"
cp $(srcdir)/docs/Lua_HUD.html "$(1)/docs/Lua_HUD.html"
cp $(srcdir)/docs/MML.html "$(1)/docs/MML.html"
unix2dos -n $(srcdir)/docs/Troubleshooting.txt "$(1)/docs/Troubleshooting.txt"
mkdir "$(1)/Themes"
cp -r $(srcdir)/data/default_theme "$(1)/Themes/Default"
rm -rf "$(1)/Themes/Default/.svn"
mkdir "$(1)/Extras"
cp $(srcdir)/data/Software_Transparent_Liquids.mml "$(1)/Extras"
cp $(srcdir)/data/Transparent_Liquids.mml "$(1)/Extras"
cp $(srcdir)/data/Carnage_Messages.mml "$(1)/Extras"
unix2dos -n $(srcdir)/examples/lua/Cheats.lua "$(1)/Extras/Cheats.lua"
cp $(srcdir)/data/Fonts "$(1)/"
cp -r "$(srcdir)/Resources/Library Licenses" "$(1)/"
rm -rf "$(1)/Library Licenses/.svn"
endef

# Windows version
$(PACKAGE)-$(VERSION).zip: Source_Files/AlephOne.exe
	mkdir "$(WINDISTDIR)"
	$(call windist_common,$(WINDISTDIR))
	cp Source_Files/AlephOne.exe $(WINDISTDIR)/
	cp $(srcdir)/data/Transparent_Sprites.mml $(WINDISTDIR)/Extras
	zip -r $(PACKAGE)-$(VERSION).zip $(WINDISTDIR)
	rm -rf "$(WINDISTDIR)"

winzip: $(PACKAGE)-$(VERSION).zip

WINDISTDIR_M1=Marathon (A1)

Marathon-$(VERSION)-Win.zip: Source_Files/Marathon.exe
	mkdir "$(WINDISTDIR_M1)"
	$(call windist_common,$(WINDISTDIR_M1))
	tar -c --exclude='.svn' -C $(srcdir)/data/Scenarios/M1A1 . | tar -x -C "$(WINDISTDIR_M1)"
	cp Source_Files/Marathon.exe "$(WINDISTDIR_M1)/Marathon.exe"
	rm -f Marathon-$(VERSION)-Win.zip
	zip -r Marathon-$(VERSION)-Win.zip "$(WINDISTDIR_M1)"
	rm -rf "$(WINDISTDIR_M1)"

win1zip: Marathon-$(VERSION)-Win.zip

WINDISTDIR_M2=Marathon 2

Marathon2-$(VERSION)-Win.zip: Source_Files/Marathon2.exe
	mkdir "$(WINDISTDIR_M2)"
	$(call windist_common,$(WINDISTDIR_M2))
	tar -c --exclude='.svn' -C "$(srcdir)/data/Scenarios/Marathon 2" . | tar -x -C "$(WINDISTDIR_M2)"
	mkdir -p "$(WINDISTDIR_M2)/Plugins/XBLA_HUD"
	tar -c --exclude='.svn' -C "$(srcdir)/data/XBLA_HUD" . | tar -x -C "$(WINDISTDIR_M2)/Plugins/XBLA_HUD"
	cp Source_Files/Marathon2.exe "$(WINDISTDIR_M2)/Marathon 2.exe"
	rm -f Marathon2-$(VERSION)-Win.zip
	zip -r Marathon2-$(VERSION)-Win.zip "$(WINDISTDIR_M2)"
	rm -rf "$(WINDISTDIR_M2)"

win2zip: Marathon2-$(VERSION)-Win.zip

WINDISTDIR_MINF=Marathon Infinity

MarathonInfinity-$(VERSION)-Win.zip: Source_Files/Marathon2.exe
	mkdir "$(WINDISTDIR_MINF)"
	$(call windist_common,$(WINDISTDIR_MINF))
	tar -c --exclude='.svn' -C "$(srcdir)/data/Scenarios/Marathon Infinity" . | tar -x -C "$(WINDISTDIR_MINF)"
	mkdir -p "$(WINDISTDIR_MINF)/Plugins/XBLA_HUD"
	tar -c --exclude='.svn' -C "$(srcdir)/data/XBLA_HUD" . | tar -x -C "$(WINDISTDIR_MINF)/Plugins/XBLA_HUD"
	cp Source_Files/MarathonInfinity.exe "$(WINDISTDIR_MINF)/Marathon Infinity.exe"
	rm -f MarathonInfinity-$(VERSION)-Win.zip
	zip -r MarathonInfinity-$(VERSION)-Win.zip "$(WINDISTDIR_MINF)"
	rm -rf "$(WINDISTDIR_MINF)"

win3zip: MarathonInfinity-$(VERSION)-Win.zip

FORCE:
