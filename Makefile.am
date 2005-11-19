## Process this file with automake to produce Makefile.in 

SUBDIRS = Source_Files tools data

# Requires automake 1.5
AUTOMAKE_OPTIONS = 1.5 foreign

EXTRA_DIST = COPYING.SDL INSTALL.BeOS INSTALL.Unix INSTALL.Windows Makefile.BeOS \
  docs/MML.html docs/Cheat_Codes

dist-hook: AlephOne.spec
	cp AlephOne.spec $(distdir)
	mkdir -p ${srcdir}/Libraries
	cp -r ${srcdir}/Libraries ${distdir}/Libraries
	rmdir --ignore-fail-on-non-empty ${srcdir}/Libraries
	rmdir --ignore-fail-on-non-empty ${distdir}/Libraries

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta --clean $(PACKAGE)-$(VERSION).tar.gz
