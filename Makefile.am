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
	unix2dos -n $(srcdir)/THANKS $(WINDISTDIR)/THANKS.txt
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

WINDISTDIR_M1=Marathon (A1)

Marathon-$(VERSION)-Win.zip: Source_Files/Marathon.exe
	mkdir "$(WINDISTDIR_M1)"
	tar -c --exclude='.svn' -C $(srcdir)/data/Scenarios/M1A1 . | tar -x -C "$(WINDISTDIR_M1)"
	cp Source_Files/Marathon.exe "$(WINDISTDIR_M1)/Marathon.exe"
	unix2dos -n $(srcdir)/COPYING "$(WINDISTDIR_M1)/COPYING.txt"
	unix2dos -n $(srcdir)/README "$(WINDISTDIR_M1)/README.txt"
	mkdir "$(WINDISTDIR_M1)/docs"
	cp $(srcdir)/docs/Lua.html "$(WINDISTDIR_M1)/docs/Lua.html"
	cp $(srcdir)/docs/Lua_HUD.html "$(WINDISTDIR_M1)/docs/Lua_HUD.html"
	cp $(srcdir)/docs/MML.html "$(WINDISTDIR_M1)/docs/MML.html"
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt "$(WINDISTDIR_M1)/docs/Troubleshooting.txt"
	mkdir -p "$(WINDISTDIR_M1)/Themes/Default"
	tar -c --exclude='.svn' -C $(srcdir)/data/default_theme . | tar -x -C "$(WINDISTDIR_M1)/Themes/Default"
	mkdir "$(WINDISTDIR_M1)/Extras"
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua "$(WINDISTDIR_M1)/Extras/Cheats.lua"
	cp $(srcdir)/data/Fonts "$(WINDISTDIR_M1)/Fonts.fntA"
	mkdir "$(WINDISTDIR_M1)/docs/Library Licenses"
	tar -c --exclude='.svn' -C "$(srcdir)/Resources/Library Licenses" . | tar -x -C "$(WINDISTDIR_M1)/docs/Library Licenses"
	rm -f Marathon-$(VERSION)-Win.zip
	zip -r Marathon-$(VERSION)-Win.zip "$(WINDISTDIR_M1)"
	rm -rf "$(WINDISTDIR_M1)"

win1zip: Marathon-$(VERSION)-Win.zip

WINDISTDIR_M2=Marathon 2

Marathon2-$(VERSION)-Win.zip: Source_Files/Marathon2.exe
	mkdir "$(WINDISTDIR_M2)"
	tar -c --exclude='.svn' -C "$(srcdir)/data/Scenarios/Marathon 2" . | tar -x -C "$(WINDISTDIR_M2)"
	mkdir -p "$(WINDISTDIR_M2)/Plugins/XBLA_HUD"
	tar -c --exclude='.svn' -C "$(srcdir)/data/XBLA_HUD" . | tar -x -C "$(WINDISTDIR_M2)/Plugins/XBLA_HUD"
	cp Source_Files/Marathon2.exe "$(WINDISTDIR_M2)/Marathon 2.exe"
	unix2dos -n $(srcdir)/COPYING "$(WINDISTDIR_M2)/COPYING.txt"
	unix2dos -n $(srcdir)/README "$(WINDISTDIR_M2)/README.txt"
	mkdir "$(WINDISTDIR_M2)/docs"
	cp $(srcdir)/docs/Lua.html "$(WINDISTDIR_M2)/docs/Lua.html"
	cp $(srcdir)/docs/Lua_HUD.html "$(WINDISTDIR_M2)/docs/Lua_HUD.html"
	cp $(srcdir)/docs/MML.html "$(WINDISTDIR_M2)/docs/MML.html"
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt "$(WINDISTDIR_M2)/docs/Troubleshooting.txt"
	mkdir "$(WINDISTDIR_M2)/Extras"
	cp $(srcdir)/data/Software_Transparent_Liquids.mml "$(WINDISTDIR_M2)/Extras"
	cp $(srcdir)/data/Carnage_Messages.mml "$(WINDISTDIR_M2)/Extras"
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua "$(WINDISTDIR_M2)/Extras/Cheats.lua"
	mkdir -p "$(WINDISTDIR_M2)/docs/Library Licenses"
	tar -c --exclude='.svn' -C "$(srcdir)/Resources/Library Licenses" . | tar -x -C "$(WINDISTDIR_M2)/docs/Library Licenses"
	rm -f Marathon2-$(VERSION)-Win.zip
	zip -r Marathon2-$(VERSION)-Win.zip "$(WINDISTDIR_M2)"
	rm -rf "$(WINDISTDIR_M2)"

win2zip: Marathon2-$(VERSION)-Win.zip

WINDISTDIR_MINF=Marathon Infinity

MarathonInfinity-$(VERSION)-Win.zip: Source_Files/Marathon2.exe
	mkdir "$(WINDISTDIR_MINF)"
	tar -c --exclude='.svn' -C "$(srcdir)/data/Scenarios/Marathon Infinity" . | tar -x -C "$(WINDISTDIR_MINF)"
	mkdir -p "$(WINDISTDIR_MINF)/Plugins/XBLA_HUD"
	tar -c --exclude='.svn' -C "$(srcdir)/data/XBLA_HUD" . | tar -x -C "$(WINDISTDIR_MINF)/Plugins/XBLA_HUD"
	cp Source_Files/MarathonInfinity.exe "$(WINDISTDIR_MINF)/Marathon Infinity.exe"
	mkdir -p "$(WINDISTDIR_MINF)/Themes/Default"
	tar -c --exclude='.svn' -C "$(srcdir)/data/default_theme" . | tar -x -C "$(WINDISTDIR_MINF)/Themes/Default"
	unix2dos -n $(srcdir)/COPYING "$(WINDISTDIR_MINF)/COPYING.txt"
	unix2dos -n $(srcdir)/README "$(WINDISTDIR_MINF)/README.txt"
	mkdir "$(WINDISTDIR_MINF)/docs"
	cp $(srcdir)/docs/Lua.html "$(WINDISTDIR_MINF)/docs/Lua.html"
	cp $(srcdir)/docs/Lua_HUD.html "$(WINDISTDIR_MINF)/docs/Lua_HUD.html"
	cp $(srcdir)/docs/MML.html "$(WINDISTDIR_MINF)/docs/MML.html"
	unix2dos -n $(srcdir)/docs/Troubleshooting.txt "$(WINDISTDIR_MINF)/docs/Troubleshooting.txt"
	mkdir "$(WINDISTDIR_MINF)/Extras"
	cp $(srcdir)/data/Software_Transparent_Liquids.mml "$(WINDISTDIR_MINF)/Extras"
	cp $(srcdir)/data/Carnage_Messages.mml "$(WINDISTDIR_MINF)/Extras"
	unix2dos -n $(srcdir)/examples/lua/Cheats.lua "$(WINDISTDIR_MINF)/Extras/Cheats.lua"
	mkdir -p "$(WINDISTDIR_MINF)/docs/Library Licenses"
	tar -c --exclude='.svn' -C "$(srcdir)/Resources/Library Licenses" . | tar -x -C "$(WINDISTDIR_MINF)/docs/Library Licenses"
	rm -f MarathonInfinity-$(VERSION)-Win.zip
	zip -r MarathonInfinity-$(VERSION)-Win.zip "$(WINDISTDIR_MINF)"
	rm -rf "$(WINDISTDIR_MINF)"

win3zip: MarathonInfinity-$(VERSION)-Win.zip

FORCE:
