## Process this file with automake to produce Makefile.in 

bin_PROGRAMS = alephone

alephone_SOURCES = \
  AnimatedTextures.h ChaseCam.h ColorParser.h Crosshairs.h DamageParser.h \
  GrowableList.h FileHandler.h ISp_Support.h LocalEvents.h MacCheckbox.h \
  OGL_Faders.h OGL_Render.h OGL_Setup.h OGL_Textures.h OverheadMap_OGL.h \
  OverheadMap_SDL.h OverheadMapRenderer.h PlayerName.h Random.h Rasterizer.h \
  Rasterizer_OGL.h Rasterizer_SW.h RenderPlaceObjs.h RenderRasterize.h \
  RenderSortPoly.h RenderVisTree.h ResizableList.h ShapesParser.h \
  TextStrings.h VectorOps.h ViewControl.h XML_Configure.h XML_ElementParser.h \
  XML_ParseTreeRoot.h XML_Resources_SDL.h collection_definition.h \
  computer_interface.h crc.h dynamic_limits.h editor.h effect_definitions.h \
  effects.h extensions.h fades.h find_files.h flood_map.h game_errors.h \
  game_wad.h game_window.h images.h interface.h interface_menus.h \
  item_definitions.h items.h key_definitions.h lightsource.h \
  map.h media.h media_definitions.h monster_definitions.h \
  monsters.h motion_sensor.h mouse.h music.h mysound.h overhead_map.h \
  physics_models.h platform_definitions.h platforms.h player.h \
  preferences.h progress.h projectile_definitions.h projectiles.h render.h \
  resource_manager.h scenery.h scenery_definitions.h scottish_textures.h \
  screen.h screen_definitions.h screen_drawing.h shape_definitions.h \
  shape_descriptors.h shell.h song_definitions.h sound_definitions.h tags.h \
  textures.h vbl.h vbl_definitions.h wad.h wad_prefs.h weapon_definitions.h \
  weapons.h world.h \
  \
  AnimatedTextures.cpp ChaseCam.cpp ColorParser.cpp Crosshairs_SDL.cpp \
  DamageParser.cpp FileHandler_SDL.cpp OGL_Setup.cpp OverheadMap_SDL.cpp \
  OverheadMapRenderer.cpp PlayerName.cpp RenderPlaceObjs.cpp \
  RenderRasterize.cpp RenderSortPoly.cpp RenderVisTree.cpp ShapesParser.cpp \
  TextStrings.cpp ViewControl.cpp XML_Configure.cpp XML_ElementParser.cpp \
  XML_MakeRoot.cpp XML_Resources_SDL.cpp computer_interface.cpp \
  crc.cpp devices.cpp dynamic_limits.cpp effects.cpp fades.cpp \
  filetypes_sdl.cpp flood_map.cpp game_errors.cpp game_wad.cpp game_window.cpp \
  game_window_sdl.cpp images.cpp import_definitions.cpp interface.cpp \
  interface_sdl.cpp items.cpp lightsource.cpp map.cpp map_constructors.cpp \
  marathon2.cpp media.cpp monsters.cpp motion_sensor.cpp \
  mysound.cpp overhead_map.cpp pathfinding.cpp physics.cpp placement.cpp \
  platforms.cpp player.cpp preferences.cpp preprocess_map_sdl.cpp \
  projectiles.cpp render.cpp resource_manager.cpp scenery.cpp \
  scottish_textures.cpp screen_drawing_sdl.cpp screen_sdl.cpp shapes.cpp \
  shell_sdl.cpp textures.cpp vbl.cpp vbl_sdl.cpp wad.cpp wad_sdl.cpp \
  wad_prefs.cpp wad_prefs_sdl.cpp weapons.cpp world.cpp

alephone_LDADD = ../CSeries/libcseries.a ../Expat/libexpat.a \
  ../Network/libnetwork.a ../Pfhortran/libpfhortran.a

EXTRA_alephone_SOURCES = computer_interface_sdl.cpp images_sdl.cpp \
  low_level_textures.cpp shapes_sdl.cpp shell_beos.cpp sound_sdl.cpp

INCLUDES = -I../CSeries -I../Expat -I../Network -I../Pfhortran

# Data directories
confpaths.h: Makefile
	echo "#define PKGDATADIR \"$(pkgdatadir)\"" > $@

DISTCLEANFILES = confpaths.h

shell_sdl.o: confpaths.h
