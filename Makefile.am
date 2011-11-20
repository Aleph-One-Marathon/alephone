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
	zip -r Marathon-$(VERSION)-Win.zip "$(WINDISTDIR_M1)"
	rm -rf "$(WINDISTDIR_M1)"

win1zip: Marathon-$(VERSION)-Win.zip

$(PACKAGE)-$(VERSION)-Durandal.zip: Source_Files/Marathon2.exe
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
	cp Source_Files/Marathon2.exe "$(WINDISTDIR)-Durandal/Marathon 2.exe"
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

$(PACKAGE)-$(VERSION)-Infinity.zip: Source_Files/MarathonInfinity.exe
	cp -r "$(srcdir)/data/Scenarios/Marathon Infinity" $(WINDISTDIR)-Infinity
	mkdir $(WINDISTDIR)-Infinity/Plugins
	cp -r $(srcdir)/data/XBLA_HUD $(WINDISTDIR)-Infinity/Plugins/XBLA_HUD
	rm -rf "$(WINDISTDIR)-Infinity/.svn"
	rm -rf "$(WINDISTDIR)-Infinity/Physics Models/.svn"
	rm -rf $(WINDISTDIR)-Infinity/Plugins/*/.svn
	rm -rf $(WINDISTDIR)-Infinity/Plugins/*/*/.svn
	rm -rf "$(WINDISTDIR)-Infinity/Scripts/.svn"
	cp Source_Files/MarathonInfinity.exe "$(WINDISTDIR)-Infinity/Marathon Infinity.exe"
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
