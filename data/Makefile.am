## Process this file with automake to produce Makefile.in 

SUBDIRS = default_theme icons

mmldir = $(pkgdatadir)/MML
mml_DATA = Interface_Rects.mml Transparent_Liquids.mml \
  Transparent_Sprites.mml

EXTRA_DIST = $(mml_DATA) alephone.xml


if INSTALL_MIME

mime_icons = \
  application-x-alephone-fila.png \
  application-x-alephone-imga.png \
  application-x-alephone-musa.png \
  application-x-alephone-phya.png \
  application-x-alephone-scea.png \
  application-x-alephone-sgaa.png \
  application-x-alephone-shpa.png \
  application-x-alephone-snda.png \
  text-x-alephone-mml.png
icon_sizes = 16 22 24 32 48 64 96 128 256
mimedir = $(datarootdir)/mime/packages
mime_DATA = alephone.xml

install-icons:
	for size in $(icon_sizes); do \
  $(MKDIR_P) $(DESTDIR)$(datarootdir)/icons/hicolor/$${size}x$${size}/mimetypes/; \
  for icon in $(mime_icons); do \
    $(INSTALL_DATA) icons/$${size}x$${size}/$$icon \
      $(DESTDIR)$(datarootdir)/icons/hicolor/$${size}x$${size}/mimetypes/; \
  done; \
done

install-data-local: install-icons

endif
