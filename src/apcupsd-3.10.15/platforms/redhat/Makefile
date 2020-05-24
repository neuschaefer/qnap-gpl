# Makefile template
#
# Copyright (C) 1999-2002 Riccardo Facchetti <riccardo@master.oasi.gpa.it>
# 
# This file is used as the template to create the
# Makefile for the RedHat specific installation.
#
#  25 Sep 1999 -- KES
#

# Default variables

# General rules for Makefile(s) subsystem.
# In this file we will put everything that need to be
# shared betweek all the Makefile(s).
# This file must be included at the beginning of every Makefile
#
# Copyright (C) 1999-2002 Riccardo Facchetti <riccardo@master.oasi.gpa.it>

#
# package version
PACKAGE = apcupsd
DISTNAME = redhat
DISTVER = (Stentz)
VERSION = 3.10.15

#
# programs needed by compilation
CP = /bin/cp
MV = /bin/mv
ECHO = /bin/echo
RM = /bin/rm
RMF = $(RM) -f
LN = /bin/ln
SED = /bin/sed
MAKE = /usr/bin/gmake
SHELL = /bin/sh
RANLIB = /usr/bin/ranlib
AR = /usr/bin/ar
LIBTOOL = /usr/bin/libtool
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
MKINSTALLDIRS = /home/jimmychang/NasARM-main/NasARM/SysUtil/apcupsd-3.10.15/autoconf/mkinstalldirs
ETAGS = /usr/bin/etags
CTAGS = /usr/bin/etags
GENCAT = 
MSGFMT = /usr/bin/msgfmt


# Files and directories (paths)
#
# Commond prefix for machine-independent installed files.
prefix = /usr

# Commond prefix for machine-dependent installed files.
exec_prefix = ${prefix}

# system configuration directory.
sysconfdir = /etc/apcupsd

# cgi-bin install directory.
cgibin = /etc/apcupsd

# Ultrix 2.2 make doesn't expand the value of VPATH.
VPATH = /usr/lib:/usr/local/lib

# source directory where this Makefile is placed.
srcdir = .

# Absolute top srcdir
abssrcdir = /home/jimmychang/NasARM-main/NasARM/SysUtil/apcupsd-3.10.15

# Directory in which to install.
sbindir = /sbin

# Directory for pid files.
piddir = /var/run

# Manual extension
manext = 8

# Manual directory
mandir=${prefix}/man


# Compilation macros.
CC = arm-none-linux-gnueabi-gcc
DEFS =  $(LOCALDEFS)
CPPFLAGS = 

# Libraries
APCLIBS = $(topdir)/src/lib/libapc.a
DRVLIBS = $(topdir)/src/drivers/libdrivers.a
win32 = $(topdir)/src/win32/winmain.o $(topdir)/src/win32/winlib.a $(topdir)/src/win32/winres.res
INTLLIBS = 
WINAPC = $()
# Hack or die: seems that cygwin doesnn't like -lgdi32 and -luser32 in deps.
WINLIBS = 


# Made INCFLAGS relative to the topdir and hardcoded into the Makefiles
# For GDINCLUDE see src/cgi/Makefile.in
INCFLAGS = $(GDINCLUDE) -I$(topdir)/include
CFLAGS = -I/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/include -I/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/usr/local/include -Wall $(CPPFLAGS)  $(INCFLAGS)
LDFLAGS = -L/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/lib -L/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/usr/local/lib
LIBS = $(DRVLIBS) $(APCLIBS) $(INTLLIBS) -lpthread 

# Flags for windows programs
WCFLAGS = 
WLDFLAGS = 
# TOP source directory.
topdir = /home/jimmychang/NasARM-main/NasARM/SysUtil/apcupsd-3.10.15
top_builddir = $(topdir)

# Include the default make targets: to be put before the all-targets: rule.

# General targets for Makefile(s) subsystem.
# In this file we will put everything that need to be
# shared betweek all the Makefile(s).
# This file must be included at the beginning of every Makefile
#
# Copyright (C) 1999-2002 Riccardo Facchetti <riccardo@master.oasi.gpa.it>

# Tell versions [3.59,3.63) of GNU make not to export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
.SUFFIXES: .o .lo .c .cpp .h .po .gmo .mo .cat .msg .pox
.MAIN: all
.PHONY: all install uninstall install- install-apcupsd install-powerflute \
		install-cgi clean realclean distclean mostlyclean clobber

all: all-subdirs all-targets

all-subdirs:
	@if test ! x"$(subdirs)" = x; then \
	    for file in . ${subdirs}; \
	    do \
		(cd $$file && if test "$$file" != "."; then $(MAKE) DESTDIR=$(DESTDIR) all; fi); \
		  if test "$$?" != "0"; then \
		     break; \
		  fi; \
	    done; \
	fi

# Standard compilation targets
dummy:

.c.o:
	$(CC) -c $(WCFLAGS) $(CFLAGS) $(DEFS) $<

.c.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) $<

.cpp.o:
	$(CC) -c $(WCFLAGS) $(CFLAGS) $(DEFS) $<

.po.pox:
	$(MAKE) DESTDIR=$(DESTDIR) $(PACKAGE).pot
	$(MSGMERGE) $< $(srcdir)/$(PACKAGE).pot -o $*.pox
 
.po.mo:
	$(MSGFMT) -o $@ $<
 
.po.gmo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	  && rm -f $$file && $(GMSGFMT) -o $$file $<
 
.po.cat:
	sed -f $(topdir)/src/intl/po2msg.sed < $< > $*.msg \
	  && rm -f $@ && $(GENCAT) $@ $*.msg					    

# Library targets
$(topdir)/src/lib/libapc.a: $(topdir)/src/lib/*.[ch]
	@(cd $(topdir)/src/lib && $(MAKE))

$(topdir)/src/drivers/libdrivers.a: $(topdir)/src/drivers/*.[ch]
	@(cd $(topdir)/src/drivers && $(MAKE))

$(topdir)/src/drivers/apcsmart/libapcsmart.a: $(topdir)/src/drivers/apcsmart/*.[ch]
	@(cd $(topdir)/src/drivers/apcsmart && $(MAKE))

$(topdir)/src/drivers/dumb/libdumb.a: $(topdir)/src/drivers/dumb/*.[ch]
	@(cd $(topdir)/src/drivers/dumb && $(MAKE))

$(topdir)/src/drivers/net/libnet.a: $(topdir)/src/drivers/net/*.[ch]
	@(cd $(topdir)/src/drivers/net && $(MAKE))

$(topdir)/src/drivers/usb/libusb.a: $(topdir)/src/drivers/usb/*.[ch]
	@(cd $(topdir)/src/drivers/usb && $(MAKE))

$(topdir)/src/drivers/snmp/libdumb.a: $(topdir)/src/drivers/snmp/*.[ch]
	@(cd $(topdir)/src/drivers/snmp && $(MAKE))

$(topdir)/src/drivers/dumb/libtest.a: $(topdir)/src/drivers/test/*.[ch]
	@(cd $(topdir)/src/drivers/test && $(MAKE))

$(topdir)/src/intl/libintl.a: $(topdir)/src/intl/*.[ch]
	@(cd $(topdir)/src/intl && $(MAKE))

$(topdir)/src/gd1.2/libgd.a: $(topdir)/src/gd1.2/*.[ch]
	@(cd $(topdir)/src/gd1.2 && $(MAKE))

$(topdir)/src/win32/winmain.o: $(topdir)/src/win32/winmain.cpp \
	     $(topdir)/src/win32/winups.h
	@(cd $(topdir)/src/win32 && $(MAKE) DESTDIR=$(DESTDIR) winmain.o)

$(topdir)/src/win32/winlib.a: $(topdir)/src/win32/*.[ch] \
	     $(topdir)/src/win32/*.cpp
	@(cd $(topdir)/src/win32 && $(MAKE) DESTDIR=$(DESTDIR) winlib.a)

$(topdir)/src/win32/winres.res: $(topdir)/src/win32/winres.rc \
	     $(topdir)/src/win32/apcupsd.ico \
	     $(topdir)/src/win32/winres.h \
	     $(topdir)/src/win32/online.ico \
	     $(topdir)/src/win32/onbatt.ico
	@(cd $(topdir)/src/win32 && $(MAKE) winres.res)

# Makefile subsystem targets
$(topdir)/autoconf/variables.mak: $(topdir)/autoconf/variables.mak.in
	@(cd $(topdir) && \
	SINGLE_MAKEFILE=yes \
	CONFIG_FILES=./autoconf/variables.mak \
	CONFIG_HEADERS= $(SHELL) ./config.status)

Makefiles:
	@(cd $(topdir) && \
	$(SHELL) ./config.status)

Makefile: $(srcdir)/Makefile.in $(topdir)/config.status \
			$(topdir)/autoconf/variables.mak $(topdir)/autoconf/targets.mak
	@$(abssrcdir)/autoconf/rebuild-makefile.sh $(abssrcdir)
	@echo "You can ignore any makedepend error messages"
	@$(MAKE) DESTDIR=$(DESTDIR) single-depend

# Configuration targets

configure:  $(topdir)/autoconf/configure.in $(topdir)/autoconf/aclocal.m4 \
			$(topdir)/autoconf/acconfig.h $(topdir)/autoconf/config.h.in
	cd $(topdir);
	$(RMF) config.cache config.log config.out config.status include/config.h
	autoconf --prepend-include=$(topdir)/autoconf \
	autoconf/configure.in > configure
	chmod 755 configure

$(topdir)/autoconf/config.h.in: $(topdir)/autoconf/configure.in \
		$(topdir)/autoconf/acconfig.h
#	cd $(srcdir);
#	autoheader --prepend-include=$(srcdir)/autoconf \
#	autoconf/configure.in > autoconf/config.h.in
#	chmod 644 autoconf/config.h.in

$(topdir)/config.status:
	@if test -x $(topdir)/config.status; then \
		(cd $(topdir) && \
		$(SHELL) ./config.status --recheck); \
	else \
		(cd $(topdir) && \
		$(SHELL) ./configure); \
	fi

clean-subdirs:
	@if test ! x"$(subdirs)" = x; then \
		for file in . ${subdirs}; \
		do \
			(cd $$file && if test "$$file" != "."; then $(MAKE) clean; fi); \
		done; \
	fi

distclean-subdirs:
	@if test ! x"$(subdirs)" = x; then \
		for file in . ${subdirs}; \
		do \
			(cd $$file && if test "$$file" != "."; then $(MAKE) distclean; fi); \
		done; \
	fi

targetclean: clean-subdirs
	$(RMF) *.o *.lo *.a core core.* .*~ *~ *.bak
	$(RMF) *.exe *.res *.cgi
	$(RMF) 1 2 3 4 ID TAGS *.orig $(allexe)

targetdistclean: clean distclean-subdirs

mostlyclean: clean
realclean: distclean
clobber: distclean

# Semi-automatic generation of dependencies:
# Use gcc if possible because X11 `makedepend' doesn't work on all systems
# and it also includes system headers.
# Also test for the presence of source files: if not present, do nothing.
depend:
	@if test "`$(topdir)/autoconf/has-c-files.sh`" = "no"; then \
		$(ECHO) "Nothing to do for depend."; \
	else \
		$(MAKE) real-depend; \
	fi
	@if test ! x"$(subdirs)" = x; then \
		for file in . ${subdirs}; \
		do \
			(cd $$file && if test "$$file" != "."; then $(MAKE) depend; fi); \
		done; \
	fi

single-depend:
	@if test "`$(topdir)/autoconf/has-c-files.sh`" = "no"; then \
		$(ECHO) "Nothing to do for depend."; \
	else \
		$(MAKE) real-depend; \
	fi

real-depend:
	@$(RMF) Makefile.bak
	@if test "$(CC)" = "gcc" ; then \
	   $(MV) Makefile Makefile.bak; \
	   $(SED) "/^# DO NOT DELETE THIS LINE/,$$ d" Makefile.bak > Makefile; \
	   $(ECHO) "# DO NOT DELETE THIS LINE -- make depend depends on it." >> Makefile; \
	   $(CC) -S -M $(CPPFLAGS) $(INCFLAGS) *.c >> Makefile; \
	else \
	   makedepend -- $(CFLAGS) -- $(INCFLAGS) *.c; \
	fi 
	@if test -f Makefile ; then \
	    $(RMF) Makefile.bak; \
	else \
	   $(MV) Makefile.bak Makefile; \
	   echo -e "Something went wrong with the make depend!\n\a\a\a\a"; \
	fi

install-subdirs:
	@if test ! x"$(subdirs)" = x; then \
	   for file in . ${subdirs}; \
	   do \
	       (cd $$file && if test "$$file" != "."; then $(MAKE) DESTDIR=$(DESTDIR) install; fi); \
	   done; \
	fi

uninstall-subdirs:
	@if test ! x"$(subdirs)" = x; then \
	   for file in . ${subdirs}; \
	   do \
	      (cd $$file && if test "$$file" != "."; then $(MAKE) DESTDIR=$(DESTDIR)uninstall; fi); \
	   done; \
	fi

indent:
	(cd $(topdir) && \
		find . \( -name '*.c' -o -name '*.h' -o -name '*.cpp' \) \
			-exec ./scripts/format_code {} \;)

TAGS:
	(cd $(topdir) && $(ETAGS) `find . \( -name \*.c -o -name \*.h \)`)
tags:
	(cd $(topdir) && $(CTAGS) `find . \( -name \*.c -o -name \*.h \)`)

all-targets: Makefile

install:
	@if test x$(DESTDIR) = x ; then \
	   if test -f /etc/rc.d/init.d/apcupsd; then \
	       /sbin/chkconfig --del apcupsd; \
	   fi; \
	fi
	# remove old 3.6.2 stuff
	@rm -f $(DESTDIR)/etc/rc.d/rc0.d/K20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc1.d/S20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc1.d/K20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc2.d/S20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc3.d/S20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc5.d/S20apcups
	@rm -f $(DESTDIR)/etc/rc.d/rc6.d/K20apcups
	@rm -f $(DESTDIR)/etc/rc.d/init.d/apcups
	@$(INSTALL_PROGRAM) -m 744 apcupsd $(DESTDIR)/etc/rc.d/init.d/apcupsd
	# save old halt script
	@cp -f /etc/rc.d/init.d/halt $(DESTDIR)/etc/rc.d/init.d/halt.old
	# insert apcupsd script into halt
	@awk -f awkhaltprog $(DESTDIR)/etc/rc.d/init.d/halt.old >$(DESTDIR)/etc/rc.d/init.d/halt
	@chmod 744 $(DESTDIR)/etc/rc.d/init.d/halt
	# link new halt script for startup and shutdown
	@if test x$(DESTDIR) = x ; then \
	   /sbin/chkconfig --add apcupsd; \
	fi
	@echo "================================================="
	@echo " "
	@echo "apcupsd script installation for RedHat $(DISTVER) complete." 
	@echo " "
	@echo "You should now edit  /etc/apcupsd/apcupsd.conf  to correspond"
	@echo "to your setup then start the apcupsd daemon with:"
	@echo " "
	@echo "/etc/rc.d/init.d/apcupsd start"
	@echo " "
	@echo "thereafter when you reboot, it will be stopped and started"
	@echo "automatically."
	@echo " "
	@echo "Please check that your halt script in:"
	@echo "    /etc/rc.d/init.d/halt"
	@echo "was properly updated (see installation section of manual)"
	@echo " "
	@echo "================================================="

uninstall:
	@echo "Removing symlinks..."
	@if test x$(DESTDIR) = x ; then \
	   /sbin/chkconfig --del apcupsd ; \
	fi
	@echo "Removing boot script..."
	@rm -f $(DESTDIR)/etc/rc.d/init.d/apcupsd
	@echo "Restoring old halt script"
	@cp -f $(DESTDIR)/etc/rc.d/init.d/halt.old /etc/rc.d/init.d/halt

clean:

distclean:
	@rm -f apccontrol.sh apcupsd awkhaltprog Makefile apcupsd.spec
	@rm -rf CVS
