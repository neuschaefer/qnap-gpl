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

# Makefile.in for back-relay
# $OpenLDAP: pkg/ldap/servers/slapd/back-relay/Makefile.in,v 1.5.2.5 2010/04/13 20:23:38 kurt Exp $
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

SRCS	= init.c op.c
OBJS	= init.lo op.lo

LDAP_INCDIR= ../../../include       
LDAP_LIBDIR= ../../../libraries

BUILD_OPT = "--enable-relay"
BUILD_MOD = yes

mod_DEFS = -DSLAPD_IMPORT
MOD_DEFS = $(yes_DEFS)

shared_LDAP_LIBS = $(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
NT_LINK_LIBS = -L.. -lslapd $(static_LDAP_LIBS) $(REWRITE)
UNIX_LINK_LIBS = $(static_LDAP_LIBS) $(REWRITE)

LIBBASE = back_relay

XINCPATH = -I.. -I$(srcdir)/..
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
# files: -I../../../include -I../../../include -I.. -I./.. -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include init.c op.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../../include -I../../../include -I.. -I./.. -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include init.c op.c
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
init.lo: ../../../include/ldap_cdefs.h
init.lo: ../../../include/ldap_log.h
init.lo: ../../../include/ldap.h
init.lo: ../../../include/lber.h
init.lo: ../../../include/lber_types.h
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
init.lo: back-relay.h
init.lo: proto-back-relay.h
op.lo: op.c
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
op.lo: ../../../include/portable.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
op.lo: ../../../include/ac/fdset.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
op.lo: ../../../include/ldap_cdefs.h
op.lo: ../../../include/ldap_features.h
op.lo: ../../../include/ac/assert.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
op.lo: ../../../include/ac/localize.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
op.lo: ../slap.h
op.lo: ../../../include/ldap_defaults.h
op.lo: ../../../include/ldap_config.h
op.lo: ../../../include/ac/stdlib.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
op.lo: ../../../include/ac/syslog.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
op.lo: ../../../include/ac/regex.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
op.lo: ../../../include/ac/signal.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
op.lo: ../../../include/ac/socket.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
op.lo: ../../../include/ac/time.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
op.lo: ../../../include/ac/param.h
op.lo: ../../../include/ac/unistd.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
op.lo: ../../../include/lutil_lockf.h
op.lo: ../../../include/avl.h
op.lo: ../../../include/ldap_cdefs.h
op.lo: ../../../include/ldap_log.h
op.lo: ../../../include/ldap.h
op.lo: ../../../include/lber.h
op.lo: ../../../include/lber_types.h
op.lo: ../../../include/ldap_features.h
op.lo: ../../../include/ldap_schema.h
op.lo: ../../../include/lber_pvt.h
op.lo: ../../../include/ldap_pvt.h
op.lo: ../../../include/ldap_pvt_uc.h
op.lo: ../../../include/ac/bytes.h
op.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
op.lo: ../../../include/ldap_pvt_thread.h
op.lo: ../../../include/ldap_int_thread.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
op.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
op.lo: ../../../include/ldap_queue.h
op.lo: ../proto-slap.h
op.lo: back-relay.h
op.lo: proto-back-relay.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
