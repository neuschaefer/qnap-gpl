# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = .
prefix = /opt/cross-project/TS-101/tools
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/gd
pkglibdir = $(libdir)/gd
pkgincludedir = $(includedir)/gd

top_builddir = .

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i686-pc-linux
host_triplet = i686-pc-linux-gnu
AS = @AS@
CC = powerpc-linux-gcc
DLLTOOL = @DLLTOOL@
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LN_S = ln -s
MAKEINFO = makeinfo
OBJDUMP = @OBJDUMP@
PACKAGE = gd
RANLIB = powerpc-linux-ranlib
VERSION = 1.8.4

AUTOMAKE_OPTIONS = 1.3

# library interface 2.1, gd release 1.8.0
libgd_la_LDFLAGS = -version-info 3:0

noinst_HEADERS = gdcache.h 

lib_LTLIBRARIES = libgd.la

include_HEADERS = gd.h gd_io.h gdfontt.h gdfonts.h 	gdfontmb.h gdfontl.h gdfontg.h gdhelpers.h wbmp.h jisx0208.h


libgd_la_SOURCES = gd.c gd_io.c gd_io_file.c gd_io_dp.c gd_io_ss.c 	gd_png.c gd_ss.c gd_gd.c gd_gd2.c gdfontt.c gdfonts.c gdfontmb.c 	gdfontl.c gdfontg.c gdttf.c gdcache.c gdxpm.c gdtables.c gdkanji.c     gd_gif_in.c gd_gif_out.c gd_biggif_out.c gd_lzw_out.c 	gd_jpeg.c gdft.c gdhelpers.c gd_wbmp.c wbmp.c 


bin_PROGRAMS = pngtogd pngtogd2 gdtopng gdtojpeg gd2topng gd2copypal gdparttopng webpng                gd2togif gdcmpgif giftogd2

pngtogd_LDADD = libgd.la

noinst_PROGRAMS = gdtest gddemo gd2time gdtestttf gdtestft

gdtest_SOURCES = gdtest.c gd.h
gdtest_LDADD = libgd.la

gdtestttf_SOURCES = gdtestttf.c gd.h
gdtestttf_LDADD = libgd.la

gdtestft_SOURCES = gdtestft.c gd.h
gdtestft_LDADD = libgd.la

gddemo_SOURCES = gddemo.c gd.h gdfonts.h gdfontg.h
gddemo_LDADD = libgd.la

pngtogd_SOURCES = pngtogd.c gd.h

pngtogd2_SOURCES = pngtogd2.c gd.h
pngtogd2_LDADD = libgd.la

gdtojpeg_SOURCES = gdtojpeg.c gd.h
gdtojpeg_LDADD = libgd.la

gdtopng_SOURCES = gdtopng.c gd.h
gdtopng_LDADD = libgd.la

gd2topng_SOURCES = gd2topng.c gd.h
gd2topng_LDADD = libgd.la

gd2copypal_SOURCES = gd2copypal.c gd.h
gd2copypal_LDADD = libgd.la

gdparttopng_SOURCES = gdparttopng.c gd.h
gdparttopng_LDADD = libgd.la

gd2time_SOURCES = gd2time.c gd.h
gd2time_LDADD = libgd.la

webpng_SOURCES = webpng.c gd.h
webpng_LDADD = libgd.la

gd2togif_SOURCES = gd2togif.c gd.h
gd2togif_LDADD = libgd.la

gdcmpgif_SOURCES = gdcmpgif.c gd.h
gdcmpgif_LDADD = libgd.la

giftogd2_SOURCES = giftogd2.c gd.h
giftogd2_LDADD = libgd.la

EXTRA_DIST = demoin.png index.html test/* mathmake.c bdftogd Makefile.nt

CLEANFILES = demoout.png test/fttest.png
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
LTLIBRARIES =  $(lib_LTLIBRARIES)


DEFS =  -DPACKAGE=\"gd\" -DVERSION=\"1.8.4\" -DHAVE_LIBM=1 -DHAVE_LIBZ=1 -DHAVE_LIBPNG=1 -DHAVE_JPEGLIB_H=1 -DHAVE_LIBJPEG=1 -DSTDC_HEADERS=1 -DHAVE_MALLOC_H=1 -DHAVE_UNISTD_H=1 -DHAVE_ZLIB_H=1 -DHAVE_PNG_H=1  -I. -I$(srcdir) 
CPPFLAGS = -I/opt/cross-project/TS-101/tools/include -I${prefix}/include
LDFLAGS = -L/opt/cross-project/TS-101/tools/lib -L${exec_prefix}/lib
LIBS = -ljpeg -lpng -lz -lm 
libgd_la_LIBADD = 
libgd_la_OBJECTS =  gd.lo gd_io.lo gd_io_file.lo gd_io_dp.lo gd_io_ss.lo \
gd_png.lo gd_ss.lo gd_gd.lo gd_gd2.lo gdfontt.lo gdfonts.lo gdfontmb.lo \
gdfontl.lo gdfontg.lo gdttf.lo gdcache.lo gdxpm.lo gdtables.lo \
gdkanji.lo gd_gif_in.lo gd_gif_out.lo gd_biggif_out.lo gd_lzw_out.lo \
gd_jpeg.lo gdft.lo gdhelpers.lo gd_wbmp.lo wbmp.lo
PROGRAMS =  $(bin_PROGRAMS) $(noinst_PROGRAMS)

pngtogd_OBJECTS =  pngtogd.o
pngtogd_DEPENDENCIES =  libgd.la
pngtogd_LDFLAGS = 
pngtogd2_OBJECTS =  pngtogd2.o
pngtogd2_DEPENDENCIES =  libgd.la
pngtogd2_LDFLAGS = 
gdtopng_OBJECTS =  gdtopng.o
gdtopng_DEPENDENCIES =  libgd.la
gdtopng_LDFLAGS = 
gdtojpeg_OBJECTS =  gdtojpeg.o
gdtojpeg_DEPENDENCIES =  libgd.la
gdtojpeg_LDFLAGS = 
gd2topng_OBJECTS =  gd2topng.o
gd2topng_DEPENDENCIES =  libgd.la
gd2topng_LDFLAGS = 
gd2copypal_OBJECTS =  gd2copypal.o
gd2copypal_DEPENDENCIES =  libgd.la
gd2copypal_LDFLAGS = 
gdparttopng_OBJECTS =  gdparttopng.o
gdparttopng_DEPENDENCIES =  libgd.la
gdparttopng_LDFLAGS = 
webpng_OBJECTS =  webpng.o
webpng_DEPENDENCIES =  libgd.la
webpng_LDFLAGS = 
gd2togif_OBJECTS =  gd2togif.o
gd2togif_DEPENDENCIES =  libgd.la
gd2togif_LDFLAGS = 
gdcmpgif_OBJECTS =  gdcmpgif.o
gdcmpgif_DEPENDENCIES =  libgd.la
gdcmpgif_LDFLAGS = 
giftogd2_OBJECTS =  giftogd2.o
giftogd2_DEPENDENCIES =  libgd.la
giftogd2_LDFLAGS = 
gdtest_OBJECTS =  gdtest.o
gdtest_DEPENDENCIES =  libgd.la
gdtest_LDFLAGS = 
gddemo_OBJECTS =  gddemo.o
gddemo_DEPENDENCIES =  libgd.la
gddemo_LDFLAGS = 
gd2time_OBJECTS =  gd2time.o
gd2time_DEPENDENCIES =  libgd.la
gd2time_LDFLAGS = 
gdtestttf_OBJECTS =  gdtestttf.o
gdtestttf_DEPENDENCIES =  libgd.la
gdtestttf_LDFLAGS = 
gdtestft_OBJECTS =  gdtestft.o
gdtestft_DEPENDENCIES =  libgd.la
gdtestft_LDFLAGS = 
CFLAGS = -g -O2
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
LTCOMPILE = $(LIBTOOL) --mode=compile $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(include_HEADERS) $(noinst_HEADERS)

DIST_COMMON =  README AUTHORS COPYING ChangeLog INSTALL Makefile.am \
Makefile.in NEWS aclocal.m4 config.guess config.sub configure \
configure.in install-sh ltconfig ltmain.sh missing mkinstalldirs


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(libgd_la_SOURCES) $(pngtogd_SOURCES) $(pngtogd2_SOURCES) $(gdtopng_SOURCES) $(gdtojpeg_SOURCES) $(gd2topng_SOURCES) $(gd2copypal_SOURCES) $(gdparttopng_SOURCES) $(webpng_SOURCES) $(gd2togif_SOURCES) $(gdcmpgif_SOURCES) $(giftogd2_SOURCES) $(gdtest_SOURCES) $(gddemo_SOURCES) $(gd2time_SOURCES) $(gdtestttf_SOURCES) $(gdtestft_SOURCES)
OBJECTS = $(libgd_la_OBJECTS) $(pngtogd_OBJECTS) $(pngtogd2_OBJECTS) $(gdtopng_OBJECTS) $(gdtojpeg_OBJECTS) $(gd2topng_OBJECTS) $(gd2copypal_OBJECTS) $(gdparttopng_OBJECTS) $(webpng_OBJECTS) $(gd2togif_OBJECTS) $(gdcmpgif_OBJECTS) $(giftogd2_OBJECTS) $(gdtest_OBJECTS) $(gddemo_OBJECTS) $(gd2time_OBJECTS) $(gdtestttf_OBJECTS) $(gdtestft_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .lo .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(ACLOCAL_M4):  configure.in 
	cd $(srcdir) && $(ACLOCAL)

config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck
$(srcdir)/configure: $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

mostlyclean-libLTLIBRARIES:

clean-libLTLIBRARIES:
	-test -z "$(lib_LTLIBRARIES)" || rm -f $(lib_LTLIBRARIES)

distclean-libLTLIBRARIES:

maintainer-clean-libLTLIBRARIES:

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "$(LIBTOOL)  --mode=install $(INSTALL) $$p $(DESTDIR)$(libdir)/$$p"; \
	    $(LIBTOOL)  --mode=install $(INSTALL) $$p $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done

uninstall-libLTLIBRARIES:
	@$(NORMAL_UNINSTALL)
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  $(LIBTOOL)  --mode=uninstall rm -f $(DESTDIR)$(libdir)/$$p; \
	done

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.c.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

libgd.la: $(libgd_la_OBJECTS) $(libgd_la_DEPENDENCIES)
	$(LINK) -rpath $(libdir) $(libgd_la_LDFLAGS) $(libgd_la_OBJECTS) $(libgd_la_LIBADD) $(LIBS)

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

mostlyclean-noinstPROGRAMS:

clean-noinstPROGRAMS:
	-test -z "$(noinst_PROGRAMS)" || rm -f $(noinst_PROGRAMS)

distclean-noinstPROGRAMS:

maintainer-clean-noinstPROGRAMS:

pngtogd: $(pngtogd_OBJECTS) $(pngtogd_DEPENDENCIES)
	@rm -f pngtogd
	$(LINK) $(pngtogd_LDFLAGS) $(pngtogd_OBJECTS) $(pngtogd_LDADD) $(LIBS)

pngtogd2: $(pngtogd2_OBJECTS) $(pngtogd2_DEPENDENCIES)
	@rm -f pngtogd2
	$(LINK) $(pngtogd2_LDFLAGS) $(pngtogd2_OBJECTS) $(pngtogd2_LDADD) $(LIBS)

gdtopng: $(gdtopng_OBJECTS) $(gdtopng_DEPENDENCIES)
	@rm -f gdtopng
	$(LINK) $(gdtopng_LDFLAGS) $(gdtopng_OBJECTS) $(gdtopng_LDADD) $(LIBS)

gdtojpeg: $(gdtojpeg_OBJECTS) $(gdtojpeg_DEPENDENCIES)
	@rm -f gdtojpeg
	$(LINK) $(gdtojpeg_LDFLAGS) $(gdtojpeg_OBJECTS) $(gdtojpeg_LDADD) $(LIBS)

gd2topng: $(gd2topng_OBJECTS) $(gd2topng_DEPENDENCIES)
	@rm -f gd2topng
	$(LINK) $(gd2topng_LDFLAGS) $(gd2topng_OBJECTS) $(gd2topng_LDADD) $(LIBS)

gd2copypal: $(gd2copypal_OBJECTS) $(gd2copypal_DEPENDENCIES)
	@rm -f gd2copypal
	$(LINK) $(gd2copypal_LDFLAGS) $(gd2copypal_OBJECTS) $(gd2copypal_LDADD) $(LIBS)

gdparttopng: $(gdparttopng_OBJECTS) $(gdparttopng_DEPENDENCIES)
	@rm -f gdparttopng
	$(LINK) $(gdparttopng_LDFLAGS) $(gdparttopng_OBJECTS) $(gdparttopng_LDADD) $(LIBS)

webpng: $(webpng_OBJECTS) $(webpng_DEPENDENCIES)
	@rm -f webpng
	$(LINK) $(webpng_LDFLAGS) $(webpng_OBJECTS) $(webpng_LDADD) $(LIBS)

gd2togif: $(gd2togif_OBJECTS) $(gd2togif_DEPENDENCIES)
	@rm -f gd2togif
	$(LINK) $(gd2togif_LDFLAGS) $(gd2togif_OBJECTS) $(gd2togif_LDADD) $(LIBS)

gdcmpgif: $(gdcmpgif_OBJECTS) $(gdcmpgif_DEPENDENCIES)
	@rm -f gdcmpgif
	$(LINK) $(gdcmpgif_LDFLAGS) $(gdcmpgif_OBJECTS) $(gdcmpgif_LDADD) $(LIBS)

giftogd2: $(giftogd2_OBJECTS) $(giftogd2_DEPENDENCIES)
	@rm -f giftogd2
	$(LINK) $(giftogd2_LDFLAGS) $(giftogd2_OBJECTS) $(giftogd2_LDADD) $(LIBS)

gdtest: $(gdtest_OBJECTS) $(gdtest_DEPENDENCIES)
	@rm -f gdtest
	$(LINK) $(gdtest_LDFLAGS) $(gdtest_OBJECTS) $(gdtest_LDADD) $(LIBS)

gddemo: $(gddemo_OBJECTS) $(gddemo_DEPENDENCIES)
	@rm -f gddemo
	$(LINK) $(gddemo_LDFLAGS) $(gddemo_OBJECTS) $(gddemo_LDADD) $(LIBS)

gd2time: $(gd2time_OBJECTS) $(gd2time_DEPENDENCIES)
	@rm -f gd2time
	$(LINK) $(gd2time_LDFLAGS) $(gd2time_OBJECTS) $(gd2time_LDADD) $(LIBS)

gdtestttf: $(gdtestttf_OBJECTS) $(gdtestttf_DEPENDENCIES)
	@rm -f gdtestttf
	$(LINK) $(gdtestttf_LDFLAGS) $(gdtestttf_OBJECTS) $(gdtestttf_LDADD) $(LIBS)

gdtestft: $(gdtestft_OBJECTS) $(gdtestft_DEPENDENCIES)
	@rm -f gdtestft
	$(LINK) $(gdtestft_LDFLAGS) $(gdtestft_OBJECTS) $(gdtestft_LDADD) $(LIBS)

install-includeHEADERS: $(include_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(includedir)
	@list='$(include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p; \
	done

uninstall-includeHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(include_HEADERS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(includedir)/$$p; \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) dist
	-rm -rf $(distdir)
	@banner="$(distdir).tar.gz is ready for distribution"; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	$(mkinstalldirs) $(distdir)/test
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-libLTLIBRARIES install-binPROGRAMS
install-exec: install-exec-am

install-data-am: install-includeHEADERS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-libLTLIBRARIES uninstall-binPROGRAMS \
		uninstall-includeHEADERS
uninstall: uninstall-am
all-am: Makefile $(LTLIBRARIES) $(PROGRAMS) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(libdir) $(DESTDIR)$(bindir) \
		$(DESTDIR)$(includedir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-libLTLIBRARIES mostlyclean-compile \
		mostlyclean-libtool mostlyclean-binPROGRAMS \
		mostlyclean-noinstPROGRAMS mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-libLTLIBRARIES clean-compile clean-libtool \
		clean-binPROGRAMS clean-noinstPROGRAMS clean-tags \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-libLTLIBRARIES distclean-compile \
		distclean-libtool distclean-binPROGRAMS \
		distclean-noinstPROGRAMS distclean-tags \
		distclean-generic clean-am
	-rm -f libtool

distclean: distclean-am
	-rm -f config.status

maintainer-clean-am:  maintainer-clean-libLTLIBRARIES \
		maintainer-clean-compile maintainer-clean-libtool \
		maintainer-clean-binPROGRAMS \
		maintainer-clean-noinstPROGRAMS maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am
	-rm -f config.status

.PHONY: mostlyclean-libLTLIBRARIES distclean-libLTLIBRARIES \
clean-libLTLIBRARIES maintainer-clean-libLTLIBRARIES \
uninstall-libLTLIBRARIES install-libLTLIBRARIES mostlyclean-compile \
distclean-compile clean-compile maintainer-clean-compile \
mostlyclean-libtool distclean-libtool clean-libtool \
maintainer-clean-libtool mostlyclean-binPROGRAMS distclean-binPROGRAMS \
clean-binPROGRAMS maintainer-clean-binPROGRAMS uninstall-binPROGRAMS \
install-binPROGRAMS mostlyclean-noinstPROGRAMS distclean-noinstPROGRAMS \
clean-noinstPROGRAMS maintainer-clean-noinstPROGRAMS \
uninstall-includeHEADERS install-includeHEADERS tags mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
