## Process this file with automake to produce Makefile.in 

noinst_LIBRARIES = libgameworld.a

libgameworld_a_SOURCES = dynamic_limits.h editor.h effect_definitions.h		 \
  effects.h flood_map.h item_definitions.h interpolated_world.h items.h		 \
  lightsource.h map.h media.h media_definitions.h monster_definitions.h		 \
  monsters.h physics_models.h platform_definitions.h platforms.h player.h	 \
  projectile_definitions.h projectiles.h scenery_definitions.h scenery.h	 \
  TickBasedCircularQueue.h weapon_definitions.h weapons.h world.h ephemera.h \
																			 \
  devices.cpp dynamic_limits.cpp effects.cpp flood_map.cpp					 \
  interpolated_world.cpp items.cpp lightsource.cpp map_constructors.cpp		 \
  map.cpp marathon2.cpp media.cpp monsters.cpp pathfinding.cpp physics.cpp	 \
  placement.cpp platforms.cpp player.cpp projectiles.cpp scenery.cpp		 \
  weapons.cpp world.cpp ephemera.cpp

AM_CPPFLAGS = -I$(top_srcdir)/Source_Files/CSeries -I$(top_srcdir)/Source_Files/Files \
  -I$(top_srcdir)/Source_Files/Input -I$(top_srcdir)/Source_Files/Lua \
  -I$(top_srcdir)/Source_Files/Misc -I$(top_srcdir)/Source_Files/ModelView \
  -I$(top_srcdir)/Source_Files/Network -I$(top_srcdir)/Source_Files/FFmpeg \
  -I$(top_srcdir)/Source_Files/RenderMain -I$(top_srcdir)/Source_Files/RenderOther \
  -I$(top_srcdir)/Source_Files/Sound -I$(top_srcdir)/Source_Files/XML \
  -I$(top_srcdir)/Source_Files
