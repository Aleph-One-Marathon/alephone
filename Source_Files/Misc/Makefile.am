## Process this file with automake to produce Makefile.in 

bin_PROGRAMS = alephone

alephone_SOURCES = \
  ActionQueues.h AnimatedTextures.h ChaseCam.h CircularQueue.h ColorParser.h \
  Crosshairs.h DamageParser.h FileHandler.h FontHandler.h HUDRenderer.h \
  HUDRenderer_SW.h HUDRenderer_OGL.h ImageLoader.h ISp_Support.h OGL_Faders.h \
  OGL_Render.h OGL_Setup.h OGL_Textures.h OverheadMap_OGL.h OverheadMap_SDL.h \
  OverheadMapRenderer.h PlayerImage_sdl.h PlayerName.h Random.h Rasterizer.h \
  Rasterizer_OGL.h Rasterizer_SW.h RenderPlaceObjs.h RenderRasterize.h \
  RenderSortPoly.h RenderVisTree.h ShapesParser.h TextLayoutHelper.h \
  TextStrings.h VectorOps.h ViewControl.h XML_Configure.h XML_DataBlock.h \
  XML_ElementParser.h XML_LevelScript.h XML_ParseTreeRoot.h XML_Loader_SDL.h \
  collection_definition.h computer_interface.h computer_interface_sdl.h crc.h \
  dynamic_limits.h editor.h effect_definitions.h effects.h extensions.h \
  fades.h find_files.h flood_map.h game_errors.h game_wad.h game_window.h \
  images.h images_sdl.h interface.h interface_menus.h item_definitions.h \
  items.h key_definitions.h lightsource.h low_level_textures.h map.h media.h \
  media_definitions.h monster_definitions.h monsters.h motion_sensor.h mouse.h \
  music.h mysound.h overhead_map.h physics_models.h platform_definitions.h \
  platforms.h player.h preferences.h preferences_private.h preferences_sdl.h \
  preferences_widgets_sdl.h progress.h projectile_definitions.h projectiles.h \
  render.h resource_manager.h scenery.h scenery_definitions.h \
  scottish_textures.h screen.h screen_definitions.h screen_drawing.h \
  screen_drawing_sdl.h screen_shared.h sdl_dialogs.h sdl_fonts.h sdl_network.h \
  sdl_widgets.h shape_definitions.h shape_descriptors.h shapes_sdl.h shell.h \
  shell_sdl.h song_definitions.h sound_definitions.h sound_sdl.h tags.h \
  textures.h thread_priority_sdl.h vbl.h vbl_definitions.h wad.h wad_prefs.h \
  weapon_definitions.h weapons.h world.h \
  \
  ActionQueues.cpp AnimatedTextures.cpp ChaseCam.cpp ColorParser.cpp \
  Crosshairs_SDL.cpp DamageParser.cpp FileHandler_SDL.cpp FontHandler.cpp \
  HUDRenderer.cpp HUDRenderer_SW.cpp HUDRenderer_OGL.cpp ImageLoader_SDL.cpp \
  OGL_Faders.cpp OGL_Render.cpp OGL_Setup.cpp OGL_Textures.cpp \
  OverheadMap_OGL.cpp OverheadMap_SDL.cpp OverheadMapRenderer.cpp Packing.h \
  PlayerImage_sdl.cpp PlayerName.cpp RenderPlaceObjs.cpp RenderRasterize.cpp \
  RenderSortPoly.cpp RenderVisTree.cpp ShapesParser.cpp TextLayoutHelper.cpp \
  TextStrings.cpp ViewControl.cpp XML_Configure.cpp XML_DataBlock.cpp \
  XML_ElementParser.cpp XML_LevelScript.cpp XML_MakeRoot.cpp XML_Loader_SDL.cpp \
  computer_interface.cpp crc.cpp devices.cpp dynamic_limits.cpp effects.cpp \
  fades.cpp find_files_sdl.cpp flood_map.cpp game_errors.cpp game_wad.cpp \
  game_window.cpp game_window_sdl.cpp images.cpp import_definitions.cpp \
  interface.cpp interface_sdl.cpp items.cpp lightsource.cpp map.cpp \
  map_constructors.cpp marathon2.cpp media.cpp monsters.cpp motion_sensor.cpp \
  mouse_sdl.cpp mysound.cpp overhead_map.cpp pathfinding.cpp physics.cpp \
  placement.cpp platforms.cpp player.cpp preferences.cpp \
  preferences_widgets_sdl.cpp preprocess_map_sdl.cpp projectiles.cpp render.cpp \
  resource_manager.cpp scenery.cpp scottish_textures.cpp screen_drawing.cpp \
  screen_sdl.cpp sdl_dialogs.cpp sdl_fonts.cpp sdl_widgets.cpp shapes.cpp \
  shell.cpp shell_misc.cpp textures.cpp thread_priority_sdl_dummy.cpp vbl.cpp \
  vbl_sdl.cpp wad.cpp wad_sdl.cpp wad_prefs.cpp weapons.cpp world.cpp

alephone_LDADD = ../Expat/libexpat.a ../ModelView/libmodelview.a \
  ../Network/libnetwork.a ../Pfhortran/libpfhortran.a ../CSeries/libcseries.a \
  @SYS_GL_LIBS@

EXTRA_alephone_SOURCES = thread_priority_sdl_macosx.cpp thread_priority_sdl_win32.cpp

INCLUDES = -I$(top_srcdir)/Source_Files/CSeries \
  -I$(top_srcdir)/Source_Files/Expat -I$(top_srcdir)/Source_Files/Network \
  -I$(top_srcdir)/Source_Files/Pfhortran -I$(top_srcdir)/Source_Files/ModelView

# Data directories
confpaths.h: Makefile
	echo "#define PKGDATADIR \"$(pkgdatadir)\"" > $@

DISTCLEANFILES = confpaths.h

shell.o: confpaths.h
