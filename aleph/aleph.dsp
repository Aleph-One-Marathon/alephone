# Microsoft Developer Studio Project File - Name="aleph" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=aleph - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aleph.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aleph.mak" CFG="aleph - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aleph - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "aleph - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aleph - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "C:\Program Files\Microsoft SDK\STLport-4.5\include" /I "Source_Files\Pfhortran" /I "Source_Files\Network" /I "Source_Files\CSeries" /I "Source_Files\Expat" /I "Source_Files\Misc" /I "Source_files\ModelView" /I "SDL-1.2.2\include" /I "QT501SDK\SDK\CIncludes" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCSSDL" /D "__WIN32__" /D "__MVCPP__" /D "HAVE_OPENGL" /D "SDL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"C:\Docume~1\Admini~1\Desktop\Alephone\aleph.exe" /libpath:"sdl" /libpath:"C:\Program Files\Microsoft SDK\DirectX\lib"

!ELSEIF  "$(CFG)" == "aleph - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W2 /Gm /Gi /GX /ZI /Od /I "C:\Program Files\Microsoft SDK\STLport-4.5\include" /I "Source_Files\Pfhortran" /I "Source_Files\Network" /I "Source_Files\CSeries" /I "Source_Files\Expat" /I "Source_Files\Misc" /I "Source_files\ModelView" /I "SDL-1.2.2\include" /I "QT501SDK\SDK\CIncludes" /D "_DEBUG" /D "SDL" /D "__WIN32__" /D "__MVCPP__" /D "HAVE_OPENGL" /D "WIN32" /D "_WINDOWS" /D "_MBCSSDL" /Fr /YX /FD /GZ /c# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmiids.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"C:\Docume~1\Admini~1\Desktop\M1A1_2\aleph.exe" /pdbtype:sept /libpath:"sdl" /libpath:"C:\Program Files\Microsoft SDK\DirectX\lib"
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "aleph - Win32 Release"
# Name "aleph - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "CSeries"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\CSeries\byte_swapping.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csalerts_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cscluts_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csmisc_sdl.cpp

!IF  "$(CFG)" == "aleph - Win32 Release"

!ELSEIF  "$(CFG)" == "aleph - Win32 Debug"

# ADD CPP /YX""

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csstrings_sdl.cpp
# End Source File
# End Group
# Begin Group "Pfhortran"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\script_instructions.cpp

!IF  "$(CFG)" == "aleph - Win32 Release"

!ELSEIF  "$(CFG)" == "aleph - Win32 Debug"

# ADD CPP /Gi /I "CSeries\\"
# SUBTRACT CPP /X

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\script_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\scripting.cpp

!IF  "$(CFG)" == "aleph - Win32 Release"

!ELSEIF  "$(CFG)" == "aleph - Win32 Debug"

# SUBTRACT CPP /X

!ENDIF 

# End Source File
# End Group
# Begin Group "Expat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Expat\hashtable.c
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmlparse.c
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmlrole.c
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmltok.c
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Misc\AnimatedTextures.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ChaseCam.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ColorParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\computer_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Crosshairs_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\DamageParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\devices.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\dynamic_limits.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\effects.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\fades.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\FileHandler_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\find_files_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\flood_map.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\FontHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_errors.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_wad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_window.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_window_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\HUDRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\HUDRenderer_OGL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\HUDRenderer_SW.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ImageLoader_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\images.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\import_definitions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\interface.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\interface_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\items.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\lightsource.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\map.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\map_constructors.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\marathon2.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\media.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\ModelView\Model3D.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\ModelView\ModelRenderer.cpp
# End Source File
# Begin Source File


SOURCE=.\Source_Files\Misc\monsters.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\motion_sensor.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\mouse_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\mysound.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Faders.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Render.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Setup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Textures.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\overhead_map.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMap_OGL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMap_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMapRenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\pathfinding.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\physics.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\placement.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\platforms.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\player.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\PlayerName.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\preprocess_map_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\projectiles.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\render.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderPlaceObjs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderRasterize.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderSortPoly.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderVisTree.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\resource_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\scenery.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\scottish_textures.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\screen_drawing.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\screen_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_fonts.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_widgets.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shapes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ShapesParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shell_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shell_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\ModelView\StudioLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\TextStrings.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\textures.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\vbl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\vbl_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ViewControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\wad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\wad_prefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\wad_sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\ModelView\WavefrontLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\world.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_Configure.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_DataBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_ElementParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_LevelScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_Loader_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_MakeRoot.cpp
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Network\network_dummy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Network\network_games.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "CSeries H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\CSeries\byte_swapping.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csalerts.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cscluts.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csdialogs.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cseries.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csfiles.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csfonts.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cskeys.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csmacros.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csmisc.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cspixels.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\csstrings.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\cstypes.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\gdspec.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\mytm.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\CSeries\sdl_cseries.h
# End Source File
# End Group
# Begin Group "Expat H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Expat\asciitab.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\hashtable.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\iasciitab.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\latin1tab.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\nametab.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\utf8tab.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmldef.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmlparse.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmlrole.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmltok.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Expat\xmltok_impl.h
# End Source File
# End Group
# Begin Group "Misc H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Misc\AnimatedTextures.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ChaseCam.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\collection_definition.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ColorParser.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\computer_interface.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\crc.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Crosshairs.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\DamageParser.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\dynamic_limits.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\editor.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\effect_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\effects.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\extensions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\fades.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\FileHandler.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\find_files.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\flood_map.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\FontHandler.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_errors.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_wad.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\game_window.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ImageLoader.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\images.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\interface.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\interface_menus.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ISp_Support.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\item_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\items.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\key_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\lightsource.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\LocalEvents.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\map.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\media.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\media_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\monster_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\monsters.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\MoreFilesExtract.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\motion_sensor.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\mouse.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\music.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\mysound.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Network\network_games.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Network\network_sound.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Faders.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Render.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Setup.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OGL_Textures.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\overhead_map.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMap_OGL.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMap_QD.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMap_SDL.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\OverheadMapRenderer.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Packing.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\physics_models.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\platform_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\platforms.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\player.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\PlayerName.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\preferences.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\progress.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\projectile_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\projectiles.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Random.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Rasterizer.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Rasterizer_OGL.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\Rasterizer_SW.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\render.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderPlaceObjs.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderRasterize.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderSortPoly.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\RenderVisTree.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\resource_manager.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\scenery.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\scenery_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\scottish_textures.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\screen.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\screen_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\screen_drawing.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_fonts.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sdl_widgets.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shape_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shape_descriptors.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ShapesParser.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\shell.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\song_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\sound_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\tags.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\TextStrings.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\textures.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\vbl.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\vbl_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\VectorOps.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\ViewControl.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\wad.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\wad_prefs.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\weapon_definitions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\weapons.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\world.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_Configure.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_DataBlock.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_ElementParser.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_LevelScript.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_Loader_SDL.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_ParseTreeRoot.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Misc\XML_ResourceFork.h
# End Source File
# End Group
# Begin Group "SDL H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SDL\include\begin_code.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\close_code.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_active.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_audio.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_byteorder.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_cdrom.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_copying.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_endian.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_error.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_events.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_joystick.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_keyboard.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_keysym.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_main.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_mouse.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_mutex.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_quit.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_rwops.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_syswm.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_thread.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_timer.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_types.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_version.h
# End Source File
# Begin Source File

SOURCE=.\SDL\include\SDL_video.h
# End Source File
# End Group
# Begin Group "Pfhortran H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\script_instructions.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\script_parser.h
# End Source File
# Begin Source File

SOURCE=.\Source_Files\Pfhortran\scripting.h
# End Source File
# End Group
# Begin Group "Network H"

# PROP Default_Filter ""
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=".\SDL-1.2.2\lib\SDL.lib"
# End Source File
# Begin Source File

SOURCE=".\SDL-1.2.2\lib\SDLmain.lib"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft SDK\lib\GLU32.LIB"
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft SDK\lib\OPENGL32.LIB"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\..\Program Files\Microsoft SDK\STLport-4.5\lib\stlport_vc6_static.lib"
# End Source File
# End Target
# End Project
