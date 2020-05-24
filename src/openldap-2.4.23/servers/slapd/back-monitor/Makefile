# $OpenLDAP: pkg/ldap/build/top.mk,v 1.103.2.11 2010/04/13 20:22:22 kurt Exp $
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2010 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
##---------------------------------------------------------------------------
#
# Top-level Makefile template
#

PACKAGE= OpenLDAP
VERSION= 2.4.23
RELEASEDATE= 2010/06/30


SHELL = /bin/sh

top_builddir = ../../..

srcdir = .
top_srcdir = ../../..

prefix = /
exec_prefix = ${prefix}
ldap_subdir = /openldap

bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}$(ldap_subdir)
includedir = ${prefix}/include
infodir = ${datarootdir}/info
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localstatedir = ${prefix}/var
mandir = ${datarootdir}/man
moduledir = ${exec_prefix}/libexec$(ldap_subdir)
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
sysconfdir = ${prefix}/etc$(ldap_subdir)
schemadir = $(sysconfdir)/schema

PLAT = UNIX
EXEEXT = 
OBJEXT = o

BUILD_LIBS_DYNAMIC = static

SHTOOL = $(top_srcdir)/build/shtool

INSTALL = $(SHTOOL) install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_SCRIPT = $(INSTALL)

STRIP = -s

LINT = lint
5LINT = 5lint

MKDEP = $(top_srcdir)/build/mkdep $(MKDEPFLAG) \
	-d "$(srcdir)" -c "$(MKDEP_CC)" -m "$(MKDEP_CFLAGS)"
MKDEP_CC	= x86_64-QNAP-linux-gnu-gcc
MKDEP_CFLAGS = -M

MKVERSION = $(top_srcdir)/build/mkversion -v "$(VERSION)"

LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBRELEASE = 2.4
LIBVERSION = 7:6:5
LTVERSION = -release $(LIBRELEASE) -version-info $(LIBVERSION)

# libtool --only flag for libraries: platform specific
NT_LTONLY_LIB = # --only-$(BUILD_LIBS_DYNAMIC)
LTONLY_LIB = $(UNIX_LTONLY_LIB)

# libtool --only flag for modules: depends on linkage of module
# The BUILD_MOD macro is defined in each backend Makefile.in file
LTONLY_yes = --tag=disable-shared
LTONLY_mod = --tag=disable-static
LTONLY_MOD = $(LTONLY_$(BUILD_MOD))

# platform-specific libtool flags
NT_LTFLAGS_LIB = -no-undefined -avoid-version -rpath $(libdir)
NT_LTFLAGS_MOD = -no-undefined -avoid-version -rpath $(moduledir)
UNIX_LTFLAGS_LIB = $(LTVERSION) -rpath $(libdir)
UNIX_LTFLAGS_MOD = $(LTVERSION) -rpath $(moduledir)

# libtool flags
LTFLAGS     = $(UNIX_LTFLAGS)
LTFLAGS_LIB = $(UNIX_LTFLAGS_LIB)
LTFLAGS_MOD = $(UNIX_LTFLAGS_MOD)

# LIB_DEFS defined in liblber and libldap Makefile.in files.
# MOD_DEFS defined in backend Makefile.in files.

# platform-specific LINK_LIBS defined in various Makefile.in files.
# LINK_LIBS referenced in library and module link commands.
LINK_LIBS = $(MOD_LIBS) $(UNIX_LINK_LIBS)

LTSTATIC = -static

LTLINK   = $(LIBTOOL) --mode=link \
	$(CC) $(LTSTATIC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS)

LTCOMPILE_LIB = $(LIBTOOL) $(LTONLY_LIB) --mode=compile \
	$(CC) $(LT_CFLAGS) $(LT_CPPFLAGS) $(LIB_DEFS) -c

LTLINK_LIB = $(LIBTOOL) $(LTONLY_LIB) --mode=link \
	$(CC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS_LIB)

LTCOMPILE_MOD = $(LIBTOOL) $(LTONLY_MOD) --mode=compile \
	$(CC) $(LT_CFLAGS) $(LT_CPPFLAGS) $(MOD_DEFS) -c

LTLINK_MOD = $(LIBTOOL) $(LTONLY_MOD) --mode=link \
	$(CC) $(LT_CFLAGS) $(LDFLAGS) $(LTFLAGS_MOD)

LTINSTALL = $(LIBTOOL) --mode=install $(INSTALL) 
LTFINISH = $(LIBTOOL) --mode=finish

# Misc UNIX commands used in build environment
AR = ar
BASENAME = basename
CAT = cat
CHMOD = chmod
DATE = date
ECHO = $(SHTOOL) echo
HOSTNAME = $(SHTOOL) echo -e "%h%d"
LN = $(SHTOOL) mkln
LN_H = $(LN)
LN_S = $(LN) -s
MAKEINFO = ${SHELL} /root/daily_build/64_01/4.4.1/SysUtil/openldap-2.4.23/build/missing --run makeinfo
MKDIR = $(SHTOOL) mkdir -p
MV = $(SHTOOL) move
PWD = pwd
RANLIB = x86_64-QNAP-linux-gnu-ranlib
RM = rm -f
SED = sed
SUBST = $(SHTOOL) subst

# For manual pages
# MANCOMPRESS=@MANCOMPRESS@
# MANCOMPRESSSUFFIX=@MANCOMPRESSSUFFIX@
MANCOMPRESS=$(CAT)
MANCOMPRESSSUFFIX=

SOELIM=soelim

INCLUDEDIR= $(top_srcdir)/include
LDAP_INCPATH= -I$(LDAP_INCDIR) -I$(INCLUDEDIR)
LDAP_LIBDIR= $(top_builddir)/libraries

LUTIL_LIBS =  -lcrypt
LTHREAD_LIBS =  -pthread

BDB_LIBS =  -ldb-4.8
SLAPD_NDB_LIBS = 

LDAP_LIBLBER_LA = $(LDAP_LIBDIR)/liblber/liblber.la
LDAP_LIBLDAP_LA = $(LDAP_LIBDIR)/libldap/libldap.la
LDAP_LIBLDAP_R_LA = $(LDAP_LIBDIR)/libldap_r/libldap_r.la

LDAP_LIBREWRITE_A = $(LDAP_LIBDIR)/librewrite/librewrite.a
LDAP_LIBLUNICODE_A = $(LDAP_LIBDIR)/liblunicode/liblunicode.a
LDAP_LIBLUTIL_A = $(LDAP_LIBDIR)/liblutil/liblutil.a

LDAP_L = $(LDAP_LIBLUTIL_A) \
	$(LDAP_LIBLDAP_LA) $(LDAP_LIBLBER_LA)
SLAPD_L = $(LDAP_LIBLUNICODE_A) $(LDAP_LIBREWRITE_A) \
	$(LDAP_LIBLUTIL_A) $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)

WRAP_LIBS = 
# AutoConfig generated 
AC_CC	= x86_64-QNAP-linux-gnu-gcc
AC_CFLAGS = -g -O2
AC_DEFS = -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include # -DHAVE_CONFIG_H
AC_LDFLAGS = -L/root/daily_build/64_01/4.4.1/LinkFS/lib -L/root/daily_build/64_01/4.4.1/LinkFS/usr/lib
AC_LIBS = -lresolv 

KRB4_LIBS = 
KRB5_LIBS = 
KRB_LIBS =  
SASL_LIBS = 
TLS_LIBS = -lssl -lcrypto
AUTH_LIBS = 
SECURITY_LIBS = $(SASL_LIBS) $(KRB_LIBS) $(TLS_LIBS) $(AUTH_LIBS)
ICU_LIBS = 

MODULES_CPPFLAGS = 
MODULES_LDFLAGS = 
MODULES_LIBS = 
SLAPD_PERL_LDFLAGS = 

SLAPD_SQL_LDFLAGS = 
SLAPD_SQL_INCLUDES = 
SLAPD_SQL_LIBS = 

SLAPD_LIBS =  -luuid $(BDB_LIBS)      $(ICU_LIBS)

# Our Defaults
CC = $(AC_CC)
DEFS = $(LDAP_INCPATH) $(XINCPATH) $(XDEFS) $(AC_DEFS) $(DEFINES)
CFLAGS = $(AC_CFLAGS) $(DEFS)
LDFLAGS = $(LDAP_LIBPATH) $(AC_LDFLAGS) $(XLDFLAGS)
LIBS = $(XLIBS) $(XXLIBS) $(AC_LIBS) $(XXXLIBS)

LT_CFLAGS = $(AC_CFLAGS)
LT_CPPFLAGS = $(DEFS)

all:		all-common all-local FORCE
install:	install-common install-local FORCE
clean:		clean-common clean-local FORCE
veryclean:	veryclean-common veryclean-local FORCE
depend:		depend-common depend-local FORCE

# empty common rules
all-common:
install-common:
clean-common:
veryclean-common:	clean-common FORCE
depend-common:
lint-common:
lint5-common:

# empty local rules
all-local:
install-local:
clean-local:
veryclean-local:	clean-local FORCE
depend-local:
lint-local:
lint5-local:

veryclean: FORCE
	$(RM) Makefile
	$(RM) -r .libs

Makefile: Makefile.in $(top_srcdir)/build/top.mk

pathtest:
	$(SHTOOL) --version

# empty rule for forcing rules
FORCE:

##---------------------------------------------------------------------------

# Makefile.in for back-monitor
# $OpenLDAP: pkg/ldap/servers/slapd/back-monitor/Makefile.in,v 1.20.2.5 2010/04/13 20:23:32 kurt Exp $
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2010 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.

SRCS = init.c search.c compare.c modify.c bind.c \
	operational.c \
	cache.c entry.c \
	backend.c database.c thread.c conn.c rww.c log.c \
	operation.c sent.c listener.c time.c overlay.c
OBJS = init.lo search.lo compare.lo modify.lo bind.lo \
	operational.lo \
	cache.lo entry.lo \
	backend.lo database.lo thread.lo conn.lo rww.lo log.lo \
	operation.lo sent.lo listener.lo time.lo overlay.lo

LDAP_INCDIR= ../../../include
LDAP_LIBDIR= ../../../libraries

BUILD_OPT = "--enable-monitor"
BUILD_MOD = yes

mod_DEFS = -DSLAPD_IMPORT
MOD_DEFS = $(yes_DEFS)

shared_LDAP_LIBS = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
NT_LINK_LIBS = -L.. -lslapd $(static_LDAP_LIBS)
UNIX_LINK_LIBS = $(static_LDAP_LIBS)

LIBBASE = back_monitor

XINCPATH = -I.. -I$(srcdir)/.. -I$(srcdir)/../slapi
XDEFS = $(MODULES_CPPFLAGS)

all-local-lib:	../.backend

../.backend: lib$(LIBBASE).a
	@touch $@

# $OpenLDAP: pkg/ldap/build/mod.mk,v 1.25.2.5 2010/04/13 20:22:21 kurt Exp $
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2010 The OpenLDAP Foundation.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.
##---------------------------------------------------------------------------
#
# Makefile Template for Server Modules
#

LIBRARY = $(LIBBASE).la
LIBSTAT = lib$(LIBBASE).a

MKDEPFLAG = -l

.SUFFIXES: .c .o .lo

.c.lo:
	$(LTCOMPILE_MOD) $<

all-no lint-no 5lint-no depend-no install-no: FORCE
	@echo "run configure with $(BUILD_OPT) to make $(LIBBASE)"

all-common: all-$(BUILD_MOD)

version.c: Makefile
	$(RM) $@
	$(MKVERSION) $(LIBBASE) > $@

version.lo: version.c $(OBJS)

$(LIBRARY): version.lo
	$(LTLINK_MOD) -module -o $@ $(OBJS) version.lo $(LINK_LIBS)

$(LIBSTAT): version.lo
	$(AR) ruv $@ `echo $(OBJS) | sed 's/\.lo/.o/g'` version.o
	@$(RANLIB) $@

clean-common: clean-lib FORCE
veryclean-common: veryclean-lib FORCE


lint-common: lint-$(BUILD_MOD)

5lint-common: 5lint-$(BUILD_MOD)

depend-common: depend-$(BUILD_MOD)

install-common: install-$(BUILD_MOD)

all-local-mod:
all-mod: $(LIBRARY) all-local-mod FORCE

all-local-lib:
all-yes: $(LIBSTAT) all-local-lib FORCE

install-mod: $(LIBRARY)
	@-$(MKDIR) $(DESTDIR)$(moduledir)
	$(LTINSTALL) $(INSTALLFLAGS) -m 755 $(LIBRARY) $(DESTDIR)$(moduledir)

install-local-lib:
install-yes: install-local-lib FORCE

lint-local-lib:
lint-yes lint-mod: lint-local-lib FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

5lint-local-lib:
5lint-yes 5lint-mod: 5lint-local-lib FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

clean-local-lib:
clean-lib: 	clean-local-lib FORCE
	$(RM) $(LIBRARY) $(LIBSTAT) version.c *.o *.lo a.out core .libs/*

depend-local-lib:
depend-yes depend-mod: depend-local-lib FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS)

veryclean-local-lib:
veryclean-lib: 	clean-lib veryclean-local-lib

Makefile: $(top_srcdir)/build/mod.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../../include -I../../../include -I.. -I./.. -I./../slapi -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include init.c search.c compare.c modify.c bind.c operational.c cache.c entry.c backend.c database.c thread.c conn.c rww.c log.c operation.c sent.c listener.c time.c overlay.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../../include -I../../../include -I.. -I./.. -I./../slapi -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include init.c search.c compare.c modify.c bind.c operational.c cache.c entry.c backend.c database.c thread.c conn.c rww.c log.c operation.c sent.c listener.c time.c overlay.c
#

init.lo: init.c
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
init.lo: ../../../include/portable.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
init.lo: ../../../include/ac/fdset.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
init.lo: ../../../include/ldap_cdefs.h
init.lo: ../../../include/ldap_features.h
init.lo: ../../../include/ac/assert.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
init.lo: ../../../include/ac/localize.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
init.lo: ../../../include/ac/string.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
init.lo: ../../../include/lutil.h
init.lo: ../../../include/ldap_cdefs.h
init.lo: ../../../include/lber_types.h
init.lo: ../slap.h
init.lo: ../../../include/ldap_defaults.h
init.lo: ../../../include/ldap_config.h
init.lo: ../../../include/ac/stdlib.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
init.lo: ../../../include/ac/syslog.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
init.lo: ../../../include/ac/regex.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
init.lo: ../../../include/ac/signal.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
init.lo: ../../../include/ac/socket.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
init.lo: ../../../include/ac/time.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
init.lo: ../../../include/ac/param.h
init.lo: ../../../include/ac/unistd.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
init.lo: ../../../include/lutil_lockf.h
init.lo: ../../../include/avl.h
init.lo: ../../../include/ldap_log.h
init.lo: ../../../include/ldap.h
init.lo: ../../../include/lber.h
init.lo: ../../../include/ldap_features.h
init.lo: ../../../include/ldap_schema.h
init.lo: ../../../include/lber_pvt.h
init.lo: ../../../include/ldap_pvt.h
init.lo: ../../../include/ldap_pvt_uc.h
init.lo: ../../../include/ac/bytes.h
init.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
init.lo: ../../../include/ldap_pvt_thread.h
init.lo: ../../../include/ldap_int_thread.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
init.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
init.lo: ../../../include/ldap_queue.h
init.lo: ../proto-slap.h
init.lo: ../config.h
init.lo: back-monitor.h
init.lo: proto-back-monitor.h
search.lo: search.c
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
search.lo: ../../../include/portable.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
search.lo: ../../../include/ac/fdset.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
search.lo: ../../../include/ldap_cdefs.h
search.lo: ../../../include/ldap_features.h
search.lo: ../../../include/ac/assert.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
search.lo: ../../../include/ac/localize.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
search.lo: ../../../include/ac/string.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
search.lo: ../../../include/ac/socket.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
search.lo: ../slap.h
search.lo: ../../../include/ldap_defaults.h
search.lo: ../../../include/ldap_config.h
search.lo: ../../../include/ac/stdlib.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
search.lo: ../../../include/ac/syslog.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
search.lo: ../../../include/ac/regex.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
search.lo: ../../../include/ac/signal.h
search.lo: ../../../include/ac/time.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
search.lo: ../../../include/ac/param.h
search.lo: ../../../include/ac/unistd.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
search.lo: ../../../include/lutil_lockf.h
search.lo: ../../../include/avl.h
search.lo: ../../../include/ldap_cdefs.h
search.lo: ../../../include/ldap_log.h
search.lo: ../../../include/ldap.h
search.lo: ../../../include/lber.h
search.lo: ../../../include/lber_types.h
search.lo: ../../../include/ldap_features.h
search.lo: ../../../include/ldap_schema.h
search.lo: ../../../include/lber_pvt.h
search.lo: ../../../include/ldap_pvt.h
search.lo: ../../../include/ldap_pvt_uc.h
search.lo: ../../../include/ac/bytes.h
search.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
search.lo: ../../../include/ldap_pvt_thread.h
search.lo: ../../../include/ldap_int_thread.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
search.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
search.lo: ../../../include/ldap_queue.h
search.lo: ../proto-slap.h
search.lo: back-monitor.h
search.lo: proto-back-monitor.h
compare.lo: compare.c
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
compare.lo: ../../../include/portable.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
compare.lo: ../../../include/ac/fdset.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
compare.lo: ../../../include/ldap_cdefs.h
compare.lo: ../../../include/ldap_features.h
compare.lo: ../../../include/ac/assert.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
compare.lo: ../../../include/ac/localize.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
compare.lo: ../slap.h
compare.lo: ../../../include/ldap_defaults.h
compare.lo: ../../../include/ldap_config.h
compare.lo: ../../../include/ac/stdlib.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
compare.lo: ../../../include/ac/syslog.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
compare.lo: ../../../include/ac/regex.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
compare.lo: ../../../include/ac/signal.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
compare.lo: ../../../include/ac/socket.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
compare.lo: ../../../include/ac/time.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
compare.lo: ../../../include/ac/param.h
compare.lo: ../../../include/ac/unistd.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
compare.lo: ../../../include/lutil_lockf.h
compare.lo: ../../../include/avl.h
compare.lo: ../../../include/ldap_cdefs.h
compare.lo: ../../../include/ldap_log.h
compare.lo: ../../../include/ldap.h
compare.lo: ../../../include/lber.h
compare.lo: ../../../include/lber_types.h
compare.lo: ../../../include/ldap_features.h
compare.lo: ../../../include/ldap_schema.h
compare.lo: ../../../include/lber_pvt.h
compare.lo: ../../../include/ldap_pvt.h
compare.lo: ../../../include/ldap_pvt_uc.h
compare.lo: ../../../include/ac/bytes.h
compare.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
compare.lo: ../../../include/ldap_pvt_thread.h
compare.lo: ../../../include/ldap_int_thread.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
compare.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
compare.lo: ../../../include/ldap_queue.h
compare.lo: ../proto-slap.h
compare.lo: back-monitor.h
compare.lo: proto-back-monitor.h
modify.lo: modify.c
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
modify.lo: ../../../include/portable.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
modify.lo: ../../../include/ac/fdset.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
modify.lo: ../../../include/ldap_cdefs.h
modify.lo: ../../../include/ldap_features.h
modify.lo: ../../../include/ac/assert.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
modify.lo: ../../../include/ac/localize.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
modify.lo: ../../../include/ac/string.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
modify.lo: ../../../include/ac/socket.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
modify.lo: ../slap.h
modify.lo: ../../../include/ldap_defaults.h
modify.lo: ../../../include/ldap_config.h
modify.lo: ../../../include/ac/stdlib.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
modify.lo: ../../../include/ac/syslog.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
modify.lo: ../../../include/ac/regex.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
modify.lo: ../../../include/ac/signal.h
modify.lo: ../../../include/ac/time.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
modify.lo: ../../../include/ac/param.h
modify.lo: ../../../include/ac/unistd.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
modify.lo: ../../../include/lutil_lockf.h
modify.lo: ../../../include/avl.h
modify.lo: ../../../include/ldap_cdefs.h
modify.lo: ../../../include/ldap_log.h
modify.lo: ../../../include/ldap.h
modify.lo: ../../../include/lber.h
modify.lo: ../../../include/lber_types.h
modify.lo: ../../../include/ldap_features.h
modify.lo: ../../../include/ldap_schema.h
modify.lo: ../../../include/lber_pvt.h
modify.lo: ../../../include/ldap_pvt.h
modify.lo: ../../../include/ldap_pvt_uc.h
modify.lo: ../../../include/ac/bytes.h
modify.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
modify.lo: ../../../include/ldap_pvt_thread.h
modify.lo: ../../../include/ldap_int_thread.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
modify.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
modify.lo: ../../../include/ldap_queue.h
modify.lo: ../proto-slap.h
modify.lo: back-monitor.h
modify.lo: proto-back-monitor.h
bind.lo: bind.c
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
bind.lo: ../../../include/portable.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
bind.lo: ../../../include/ac/fdset.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
bind.lo: ../../../include/ldap_cdefs.h
bind.lo: ../../../include/ldap_features.h
bind.lo: ../../../include/ac/assert.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
bind.lo: ../../../include/ac/localize.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
bind.lo: ../slap.h
bind.lo: ../../../include/ldap_defaults.h
bind.lo: ../../../include/ldap_config.h
bind.lo: ../../../include/ac/stdlib.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
bind.lo: ../../../include/ac/syslog.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
bind.lo: ../../../include/ac/regex.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
bind.lo: ../../../include/ac/signal.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
bind.lo: ../../../include/ac/socket.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
bind.lo: ../../../include/ac/time.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
bind.lo: ../../../include/ac/param.h
bind.lo: ../../../include/ac/unistd.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
bind.lo: ../../../include/lutil_lockf.h
bind.lo: ../../../include/avl.h
bind.lo: ../../../include/ldap_cdefs.h
bind.lo: ../../../include/ldap_log.h
bind.lo: ../../../include/ldap.h
bind.lo: ../../../include/lber.h
bind.lo: ../../../include/lber_types.h
bind.lo: ../../../include/ldap_features.h
bind.lo: ../../../include/ldap_schema.h
bind.lo: ../../../include/lber_pvt.h
bind.lo: ../../../include/ldap_pvt.h
bind.lo: ../../../include/ldap_pvt_uc.h
bind.lo: ../../../include/ac/bytes.h
bind.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
bind.lo: ../../../include/ldap_pvt_thread.h
bind.lo: ../../../include/ldap_int_thread.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
bind.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
bind.lo: ../../../include/ldap_queue.h
bind.lo: ../proto-slap.h
bind.lo: back-monitor.h
bind.lo: proto-back-monitor.h
operational.lo: operational.c
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
operational.lo: ../../../include/portable.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
operational.lo: ../../../include/ac/fdset.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
operational.lo: ../../../include/ldap_cdefs.h
operational.lo: ../../../include/ldap_features.h
operational.lo: ../../../include/ac/assert.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
operational.lo: ../../../include/ac/localize.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
operational.lo: ../../../include/ac/string.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
operational.lo: ../../../include/ac/socket.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
operational.lo: ../slap.h
operational.lo: ../../../include/ldap_defaults.h
operational.lo: ../../../include/ldap_config.h
operational.lo: ../../../include/ac/stdlib.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
operational.lo: ../../../include/ac/syslog.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
operational.lo: ../../../include/ac/regex.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
operational.lo: ../../../include/ac/signal.h
operational.lo: ../../../include/ac/time.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
operational.lo: ../../../include/ac/param.h
operational.lo: ../../../include/ac/unistd.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
operational.lo: ../../../include/lutil_lockf.h
operational.lo: ../../../include/avl.h
operational.lo: ../../../include/ldap_cdefs.h
operational.lo: ../../../include/ldap_log.h
operational.lo: ../../../include/ldap.h
operational.lo: ../../../include/lber.h
operational.lo: ../../../include/lber_types.h
operational.lo: ../../../include/ldap_features.h
operational.lo: ../../../include/ldap_schema.h
operational.lo: ../../../include/lber_pvt.h
operational.lo: ../../../include/ldap_pvt.h
operational.lo: ../../../include/ldap_pvt_uc.h
operational.lo: ../../../include/ac/bytes.h
operational.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
operational.lo: ../../../include/ldap_pvt_thread.h
operational.lo: ../../../include/ldap_int_thread.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
operational.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
operational.lo: ../../../include/ldap_queue.h
operational.lo: ../proto-slap.h
operational.lo: back-monitor.h
operational.lo: proto-back-monitor.h
cache.lo: cache.c
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
cache.lo: ../../../include/portable.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
cache.lo: ../../../include/ac/fdset.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
cache.lo: ../../../include/ldap_cdefs.h
cache.lo: ../../../include/ldap_features.h
cache.lo: ../../../include/ac/assert.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
cache.lo: ../../../include/ac/localize.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
cache.lo: ../../../include/ac/string.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
cache.lo: ../slap.h
cache.lo: ../../../include/ldap_defaults.h
cache.lo: ../../../include/ldap_config.h
cache.lo: ../../../include/ac/stdlib.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
cache.lo: ../../../include/ac/syslog.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
cache.lo: ../../../include/ac/regex.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
cache.lo: ../../../include/ac/signal.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
cache.lo: ../../../include/ac/socket.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
cache.lo: ../../../include/ac/time.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
cache.lo: ../../../include/ac/param.h
cache.lo: ../../../include/ac/unistd.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
cache.lo: ../../../include/lutil_lockf.h
cache.lo: ../../../include/avl.h
cache.lo: ../../../include/ldap_cdefs.h
cache.lo: ../../../include/ldap_log.h
cache.lo: ../../../include/ldap.h
cache.lo: ../../../include/lber.h
cache.lo: ../../../include/lber_types.h
cache.lo: ../../../include/ldap_features.h
cache.lo: ../../../include/ldap_schema.h
cache.lo: ../../../include/lber_pvt.h
cache.lo: ../../../include/ldap_pvt.h
cache.lo: ../../../include/ldap_pvt_uc.h
cache.lo: ../../../include/ac/bytes.h
cache.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
cache.lo: ../../../include/ldap_pvt_thread.h
cache.lo: ../../../include/ldap_int_thread.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
cache.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
cache.lo: ../../../include/ldap_queue.h
cache.lo: ../proto-slap.h
cache.lo: back-monitor.h
cache.lo: proto-back-monitor.h
entry.lo: entry.c
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
entry.lo: ../../../include/portable.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
entry.lo: ../../../include/ac/fdset.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
entry.lo: ../../../include/ldap_cdefs.h
entry.lo: ../../../include/ldap_features.h
entry.lo: ../../../include/ac/assert.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
entry.lo: ../../../include/ac/localize.h
entry.lo: ../slap.h
entry.lo: ../../../include/ldap_defaults.h
entry.lo: ../../../include/ldap_config.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
entry.lo: ../../../include/ac/stdlib.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
entry.lo: ../../../include/ac/syslog.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
entry.lo: ../../../include/ac/regex.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
entry.lo: ../../../include/ac/signal.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
entry.lo: ../../../include/ac/socket.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
entry.lo: ../../../include/ac/time.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
entry.lo: ../../../include/ac/param.h
entry.lo: ../../../include/ac/unistd.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
entry.lo: ../../../include/lutil_lockf.h
entry.lo: ../../../include/avl.h
entry.lo: ../../../include/ldap_cdefs.h
entry.lo: ../../../include/ldap_log.h
entry.lo: ../../../include/ldap.h
entry.lo: ../../../include/lber.h
entry.lo: ../../../include/lber_types.h
entry.lo: ../../../include/ldap_features.h
entry.lo: ../../../include/ldap_schema.h
entry.lo: ../../../include/lber_pvt.h
entry.lo: ../../../include/ldap_pvt.h
entry.lo: ../../../include/ldap_pvt_uc.h
entry.lo: ../../../include/ac/bytes.h
entry.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
entry.lo: ../../../include/ldap_pvt_thread.h
entry.lo: ../../../include/ldap_int_thread.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
entry.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
entry.lo: ../../../include/ldap_queue.h
entry.lo: ../proto-slap.h
entry.lo: back-monitor.h
entry.lo: proto-back-monitor.h
backend.lo: backend.c
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
backend.lo: ../../../include/portable.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
backend.lo: ../../../include/ac/fdset.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
backend.lo: ../../../include/ldap_cdefs.h
backend.lo: ../../../include/ldap_features.h
backend.lo: ../../../include/ac/assert.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
backend.lo: ../../../include/ac/localize.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
backend.lo: ../../../include/ac/string.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
backend.lo: ../slap.h
backend.lo: ../../../include/ldap_defaults.h
backend.lo: ../../../include/ldap_config.h
backend.lo: ../../../include/ac/stdlib.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
backend.lo: ../../../include/ac/syslog.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
backend.lo: ../../../include/ac/regex.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
backend.lo: ../../../include/ac/signal.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
backend.lo: ../../../include/ac/socket.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
backend.lo: ../../../include/ac/time.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
backend.lo: ../../../include/ac/param.h
backend.lo: ../../../include/ac/unistd.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
backend.lo: ../../../include/lutil_lockf.h
backend.lo: ../../../include/avl.h
backend.lo: ../../../include/ldap_cdefs.h
backend.lo: ../../../include/ldap_log.h
backend.lo: ../../../include/ldap.h
backend.lo: ../../../include/lber.h
backend.lo: ../../../include/lber_types.h
backend.lo: ../../../include/ldap_features.h
backend.lo: ../../../include/ldap_schema.h
backend.lo: ../../../include/lber_pvt.h
backend.lo: ../../../include/ldap_pvt.h
backend.lo: ../../../include/ldap_pvt_uc.h
backend.lo: ../../../include/ac/bytes.h
backend.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
backend.lo: ../../../include/ldap_pvt_thread.h
backend.lo: ../../../include/ldap_int_thread.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
backend.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
backend.lo: ../../../include/ldap_queue.h
backend.lo: ../proto-slap.h
backend.lo: back-monitor.h
backend.lo: proto-back-monitor.h
database.lo: database.c
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
database.lo: ../../../include/portable.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
database.lo: ../../../include/ac/fdset.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
database.lo: ../../../include/ldap_cdefs.h
database.lo: ../../../include/ldap_features.h
database.lo: ../../../include/ac/assert.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
database.lo: ../../../include/ac/localize.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
database.lo: ../../../include/ac/string.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
database.lo: ../../../include/ac/unistd.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
database.lo: ../../../include/lutil_lockf.h
database.lo: ../slap.h
database.lo: ../../../include/ldap_defaults.h
database.lo: ../../../include/ldap_config.h
database.lo: ../../../include/ac/stdlib.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
database.lo: ../../../include/ac/syslog.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
database.lo: ../../../include/ac/regex.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
database.lo: ../../../include/ac/signal.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
database.lo: ../../../include/ac/socket.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
database.lo: ../../../include/ac/time.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
database.lo: ../../../include/ac/param.h
database.lo: ../../../include/avl.h
database.lo: ../../../include/ldap_cdefs.h
database.lo: ../../../include/ldap_log.h
database.lo: ../../../include/ldap.h
database.lo: ../../../include/lber.h
database.lo: ../../../include/lber_types.h
database.lo: ../../../include/ldap_features.h
database.lo: ../../../include/ldap_schema.h
database.lo: ../../../include/lber_pvt.h
database.lo: ../../../include/ldap_pvt.h
database.lo: ../../../include/ldap_pvt_uc.h
database.lo: ../../../include/ac/bytes.h
database.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
database.lo: ../../../include/ldap_pvt_thread.h
database.lo: ../../../include/ldap_int_thread.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
database.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
database.lo: ../../../include/ldap_queue.h
database.lo: ../proto-slap.h
database.lo: back-monitor.h
database.lo: proto-back-monitor.h
thread.lo: thread.c
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
thread.lo: ../../../include/portable.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
thread.lo: ../../../include/ac/fdset.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
thread.lo: ../../../include/ldap_cdefs.h
thread.lo: ../../../include/ldap_features.h
thread.lo: ../../../include/ac/assert.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
thread.lo: ../../../include/ac/localize.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
thread.lo: ../../../include/ac/string.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
thread.lo: ../slap.h
thread.lo: ../../../include/ldap_defaults.h
thread.lo: ../../../include/ldap_config.h
thread.lo: ../../../include/ac/stdlib.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
thread.lo: ../../../include/ac/syslog.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
thread.lo: ../../../include/ac/regex.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
thread.lo: ../../../include/ac/signal.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
thread.lo: ../../../include/ac/socket.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
thread.lo: ../../../include/ac/time.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
thread.lo: ../../../include/ac/param.h
thread.lo: ../../../include/ac/unistd.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
thread.lo: ../../../include/lutil_lockf.h
thread.lo: ../../../include/avl.h
thread.lo: ../../../include/ldap_cdefs.h
thread.lo: ../../../include/ldap_log.h
thread.lo: ../../../include/ldap.h
thread.lo: ../../../include/lber.h
thread.lo: ../../../include/lber_types.h
thread.lo: ../../../include/ldap_features.h
thread.lo: ../../../include/ldap_schema.h
thread.lo: ../../../include/lber_pvt.h
thread.lo: ../../../include/ldap_pvt.h
thread.lo: ../../../include/ldap_pvt_uc.h
thread.lo: ../../../include/ac/bytes.h
thread.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
thread.lo: ../../../include/ldap_pvt_thread.h
thread.lo: ../../../include/ldap_int_thread.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
thread.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
thread.lo: ../../../include/ldap_queue.h
thread.lo: ../proto-slap.h
thread.lo: back-monitor.h
thread.lo: proto-back-monitor.h
thread.lo: ../../../include/ldap_rq.h
conn.lo: conn.c
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
conn.lo: ../../../include/portable.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
conn.lo: ../../../include/ac/fdset.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
conn.lo: ../../../include/ldap_cdefs.h
conn.lo: ../../../include/ldap_features.h
conn.lo: ../../../include/ac/assert.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
conn.lo: ../../../include/ac/localize.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
conn.lo: ../../../include/ac/string.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
conn.lo: ../slap.h
conn.lo: ../../../include/ldap_defaults.h
conn.lo: ../../../include/ldap_config.h
conn.lo: ../../../include/ac/stdlib.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
conn.lo: ../../../include/ac/syslog.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
conn.lo: ../../../include/ac/regex.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
conn.lo: ../../../include/ac/signal.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
conn.lo: ../../../include/ac/socket.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
conn.lo: ../../../include/ac/time.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
conn.lo: ../../../include/ac/param.h
conn.lo: ../../../include/ac/unistd.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
conn.lo: ../../../include/lutil_lockf.h
conn.lo: ../../../include/avl.h
conn.lo: ../../../include/ldap_cdefs.h
conn.lo: ../../../include/ldap_log.h
conn.lo: ../../../include/ldap.h
conn.lo: ../../../include/lber.h
conn.lo: ../../../include/lber_types.h
conn.lo: ../../../include/ldap_features.h
conn.lo: ../../../include/ldap_schema.h
conn.lo: ../../../include/lber_pvt.h
conn.lo: ../../../include/ldap_pvt.h
conn.lo: ../../../include/ldap_pvt_uc.h
conn.lo: ../../../include/ac/bytes.h
conn.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
conn.lo: ../../../include/ldap_pvt_thread.h
conn.lo: ../../../include/ldap_int_thread.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
conn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
conn.lo: ../../../include/ldap_queue.h
conn.lo: ../proto-slap.h
conn.lo: ../../../include/lutil.h
conn.lo: back-monitor.h
conn.lo: proto-back-monitor.h
rww.lo: rww.c
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
rww.lo: ../../../include/portable.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
rww.lo: ../../../include/ac/fdset.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
rww.lo: ../../../include/ldap_cdefs.h
rww.lo: ../../../include/ldap_features.h
rww.lo: ../../../include/ac/assert.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
rww.lo: ../../../include/ac/localize.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
rww.lo: ../../../include/ac/string.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
rww.lo: ../slap.h
rww.lo: ../../../include/ldap_defaults.h
rww.lo: ../../../include/ldap_config.h
rww.lo: ../../../include/ac/stdlib.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
rww.lo: ../../../include/ac/syslog.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
rww.lo: ../../../include/ac/regex.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
rww.lo: ../../../include/ac/signal.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
rww.lo: ../../../include/ac/socket.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
rww.lo: ../../../include/ac/time.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
rww.lo: ../../../include/ac/param.h
rww.lo: ../../../include/ac/unistd.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
rww.lo: ../../../include/lutil_lockf.h
rww.lo: ../../../include/avl.h
rww.lo: ../../../include/ldap_cdefs.h
rww.lo: ../../../include/ldap_log.h
rww.lo: ../../../include/ldap.h
rww.lo: ../../../include/lber.h
rww.lo: ../../../include/lber_types.h
rww.lo: ../../../include/ldap_features.h
rww.lo: ../../../include/ldap_schema.h
rww.lo: ../../../include/lber_pvt.h
rww.lo: ../../../include/ldap_pvt.h
rww.lo: ../../../include/ldap_pvt_uc.h
rww.lo: ../../../include/ac/bytes.h
rww.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
rww.lo: ../../../include/ldap_pvt_thread.h
rww.lo: ../../../include/ldap_int_thread.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
rww.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
rww.lo: ../../../include/ldap_queue.h
rww.lo: ../proto-slap.h
rww.lo: ../../../include/lutil.h
rww.lo: back-monitor.h
rww.lo: proto-back-monitor.h
log.lo: log.c
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
log.lo: ../../../include/portable.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
log.lo: ../../../include/ac/fdset.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
log.lo: ../../../include/ldap_cdefs.h
log.lo: ../../../include/ldap_features.h
log.lo: ../../../include/ac/assert.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
log.lo: ../../../include/ac/localize.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
log.lo: ../../../include/ac/string.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
log.lo: ../slap.h
log.lo: ../../../include/ldap_defaults.h
log.lo: ../../../include/ldap_config.h
log.lo: ../../../include/ac/stdlib.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
log.lo: ../../../include/ac/syslog.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
log.lo: ../../../include/ac/regex.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
log.lo: ../../../include/ac/signal.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
log.lo: ../../../include/ac/socket.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
log.lo: ../../../include/ac/time.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
log.lo: ../../../include/ac/param.h
log.lo: ../../../include/ac/unistd.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
log.lo: ../../../include/lutil_lockf.h
log.lo: ../../../include/avl.h
log.lo: ../../../include/ldap_cdefs.h
log.lo: ../../../include/ldap_log.h
log.lo: ../../../include/ldap.h
log.lo: ../../../include/lber.h
log.lo: ../../../include/lber_types.h
log.lo: ../../../include/ldap_features.h
log.lo: ../../../include/ldap_schema.h
log.lo: ../../../include/lber_pvt.h
log.lo: ../../../include/ldap_pvt.h
log.lo: ../../../include/ldap_pvt_uc.h
log.lo: ../../../include/ac/bytes.h
log.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
log.lo: ../../../include/ldap_pvt_thread.h
log.lo: ../../../include/ldap_int_thread.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
log.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
log.lo: ../../../include/ldap_queue.h
log.lo: ../proto-slap.h
log.lo: ../../../include/lutil.h
log.lo: ../../../include/ldif.h
log.lo: back-monitor.h
log.lo: proto-back-monitor.h
operation.lo: operation.c
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
operation.lo: ../../../include/portable.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
operation.lo: ../../../include/ac/fdset.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
operation.lo: ../../../include/ldap_cdefs.h
operation.lo: ../../../include/ldap_features.h
operation.lo: ../../../include/ac/assert.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
operation.lo: ../../../include/ac/localize.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
operation.lo: ../../../include/ac/string.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
operation.lo: ../slap.h
operation.lo: ../../../include/ldap_defaults.h
operation.lo: ../../../include/ldap_config.h
operation.lo: ../../../include/ac/stdlib.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
operation.lo: ../../../include/ac/syslog.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
operation.lo: ../../../include/ac/regex.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
operation.lo: ../../../include/ac/signal.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
operation.lo: ../../../include/ac/socket.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
operation.lo: ../../../include/ac/time.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
operation.lo: ../../../include/ac/param.h
operation.lo: ../../../include/ac/unistd.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
operation.lo: ../../../include/lutil_lockf.h
operation.lo: ../../../include/avl.h
operation.lo: ../../../include/ldap_cdefs.h
operation.lo: ../../../include/ldap_log.h
operation.lo: ../../../include/ldap.h
operation.lo: ../../../include/lber.h
operation.lo: ../../../include/lber_types.h
operation.lo: ../../../include/ldap_features.h
operation.lo: ../../../include/ldap_schema.h
operation.lo: ../../../include/lber_pvt.h
operation.lo: ../../../include/ldap_pvt.h
operation.lo: ../../../include/ldap_pvt_uc.h
operation.lo: ../../../include/ac/bytes.h
operation.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
operation.lo: ../../../include/ldap_pvt_thread.h
operation.lo: ../../../include/ldap_int_thread.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
operation.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
operation.lo: ../../../include/ldap_queue.h
operation.lo: ../proto-slap.h
operation.lo: back-monitor.h
operation.lo: proto-back-monitor.h
sent.lo: sent.c
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
sent.lo: ../../../include/portable.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
sent.lo: ../../../include/ac/fdset.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
sent.lo: ../../../include/ldap_cdefs.h
sent.lo: ../../../include/ldap_features.h
sent.lo: ../../../include/ac/assert.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
sent.lo: ../../../include/ac/localize.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
sent.lo: ../../../include/ac/string.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
sent.lo: ../slap.h
sent.lo: ../../../include/ldap_defaults.h
sent.lo: ../../../include/ldap_config.h
sent.lo: ../../../include/ac/stdlib.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
sent.lo: ../../../include/ac/syslog.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
sent.lo: ../../../include/ac/regex.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
sent.lo: ../../../include/ac/signal.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
sent.lo: ../../../include/ac/socket.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
sent.lo: ../../../include/ac/time.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
sent.lo: ../../../include/ac/param.h
sent.lo: ../../../include/ac/unistd.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
sent.lo: ../../../include/lutil_lockf.h
sent.lo: ../../../include/avl.h
sent.lo: ../../../include/ldap_cdefs.h
sent.lo: ../../../include/ldap_log.h
sent.lo: ../../../include/ldap.h
sent.lo: ../../../include/lber.h
sent.lo: ../../../include/lber_types.h
sent.lo: ../../../include/ldap_features.h
sent.lo: ../../../include/ldap_schema.h
sent.lo: ../../../include/lber_pvt.h
sent.lo: ../../../include/ldap_pvt.h
sent.lo: ../../../include/ldap_pvt_uc.h
sent.lo: ../../../include/ac/bytes.h
sent.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
sent.lo: ../../../include/ldap_pvt_thread.h
sent.lo: ../../../include/ldap_int_thread.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
sent.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
sent.lo: ../../../include/ldap_queue.h
sent.lo: ../proto-slap.h
sent.lo: back-monitor.h
sent.lo: proto-back-monitor.h
listener.lo: listener.c
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
listener.lo: ../../../include/portable.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
listener.lo: ../../../include/ac/fdset.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
listener.lo: ../../../include/ldap_cdefs.h
listener.lo: ../../../include/ldap_features.h
listener.lo: ../../../include/ac/assert.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
listener.lo: ../../../include/ac/localize.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
listener.lo: ../../../include/ac/string.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
listener.lo: ../slap.h
listener.lo: ../../../include/ldap_defaults.h
listener.lo: ../../../include/ldap_config.h
listener.lo: ../../../include/ac/stdlib.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
listener.lo: ../../../include/ac/syslog.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
listener.lo: ../../../include/ac/regex.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
listener.lo: ../../../include/ac/signal.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
listener.lo: ../../../include/ac/socket.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
listener.lo: ../../../include/ac/time.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
listener.lo: ../../../include/ac/param.h
listener.lo: ../../../include/ac/unistd.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
listener.lo: ../../../include/lutil_lockf.h
listener.lo: ../../../include/avl.h
listener.lo: ../../../include/ldap_cdefs.h
listener.lo: ../../../include/ldap_log.h
listener.lo: ../../../include/ldap.h
listener.lo: ../../../include/lber.h
listener.lo: ../../../include/lber_types.h
listener.lo: ../../../include/ldap_features.h
listener.lo: ../../../include/ldap_schema.h
listener.lo: ../../../include/lber_pvt.h
listener.lo: ../../../include/ldap_pvt.h
listener.lo: ../../../include/ldap_pvt_uc.h
listener.lo: ../../../include/ac/bytes.h
listener.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
listener.lo: ../../../include/ldap_pvt_thread.h
listener.lo: ../../../include/ldap_int_thread.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
listener.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
listener.lo: ../../../include/ldap_queue.h
listener.lo: ../proto-slap.h
listener.lo: back-monitor.h
listener.lo: proto-back-monitor.h
time.lo: time.c
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
time.lo: ../../../include/portable.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
time.lo: ../../../include/ac/fdset.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
time.lo: ../../../include/ldap_cdefs.h
time.lo: ../../../include/ldap_features.h
time.lo: ../../../include/ac/assert.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
time.lo: ../../../include/ac/localize.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
time.lo: ../../../include/ac/string.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
time.lo: ../../../include/ac/time.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
time.lo: ../slap.h
time.lo: ../../../include/ldap_defaults.h
time.lo: ../../../include/ldap_config.h
time.lo: ../../../include/ac/stdlib.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
time.lo: ../../../include/ac/syslog.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
time.lo: ../../../include/ac/regex.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
time.lo: ../../../include/ac/signal.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
time.lo: ../../../include/ac/socket.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
time.lo: ../../../include/ac/param.h
time.lo: ../../../include/ac/unistd.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
time.lo: ../../../include/lutil_lockf.h
time.lo: ../../../include/avl.h
time.lo: ../../../include/ldap_cdefs.h
time.lo: ../../../include/ldap_log.h
time.lo: ../../../include/ldap.h
time.lo: ../../../include/lber.h
time.lo: ../../../include/lber_types.h
time.lo: ../../../include/ldap_features.h
time.lo: ../../../include/ldap_schema.h
time.lo: ../../../include/lber_pvt.h
time.lo: ../../../include/ldap_pvt.h
time.lo: ../../../include/ldap_pvt_uc.h
time.lo: ../../../include/ac/bytes.h
time.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
time.lo: ../../../include/ldap_pvt_thread.h
time.lo: ../../../include/ldap_int_thread.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
time.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
time.lo: ../../../include/ldap_queue.h
time.lo: ../proto-slap.h
time.lo: ../../../include/lutil.h
time.lo: ../proto-slap.h
time.lo: back-monitor.h
time.lo: proto-back-monitor.h
overlay.lo: overlay.c
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
overlay.lo: ../../../include/portable.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
overlay.lo: ../../../include/ac/fdset.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
overlay.lo: ../../../include/ldap_cdefs.h
overlay.lo: ../../../include/ldap_features.h
overlay.lo: ../../../include/ac/assert.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
overlay.lo: ../../../include/ac/localize.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
overlay.lo: ../../../include/ac/string.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
overlay.lo: ../slap.h
overlay.lo: ../../../include/ldap_defaults.h
overlay.lo: ../../../include/ldap_config.h
overlay.lo: ../../../include/ac/stdlib.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
overlay.lo: ../../../include/ac/syslog.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
overlay.lo: ../../../include/ac/regex.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
overlay.lo: ../../../include/ac/signal.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
overlay.lo: ../../../include/ac/socket.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
overlay.lo: ../../../include/ac/time.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
overlay.lo: ../../../include/ac/param.h
overlay.lo: ../../../include/ac/unistd.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
overlay.lo: ../../../include/lutil_lockf.h
overlay.lo: ../../../include/avl.h
overlay.lo: ../../../include/ldap_cdefs.h
overlay.lo: ../../../include/ldap_log.h
overlay.lo: ../../../include/ldap.h
overlay.lo: ../../../include/lber.h
overlay.lo: ../../../include/lber_types.h
overlay.lo: ../../../include/ldap_features.h
overlay.lo: ../../../include/ldap_schema.h
overlay.lo: ../../../include/lber_pvt.h
overlay.lo: ../../../include/ldap_pvt.h
overlay.lo: ../../../include/ldap_pvt_uc.h
overlay.lo: ../../../include/ac/bytes.h
overlay.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
overlay.lo: ../../../include/ldap_pvt_thread.h
overlay.lo: ../../../include/ldap_int_thread.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
overlay.lo: ../../../include/ldap_queue.h
overlay.lo: ../proto-slap.h
overlay.lo: back-monitor.h
overlay.lo: proto-back-monitor.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
