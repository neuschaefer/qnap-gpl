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

top_builddir = ../..

srcdir = .
top_srcdir = ../..

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

# LIBREWRITE
# $OpenLDAP: pkg/ldap/libraries/librewrite/Makefile.in,v 1.14.2.5 2010/04/13 20:23:08 kurt Exp $
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
##
## Copyright 2000-2001 Pierangelo Masarati <ando@sys-net.it>
##

SRCS = config.c context.c info.c ldapmap.c map.c params.c rule.c \
	session.c subst.c var.c xmap.c \
	parse.c rewrite.c
XSRCS = version.c
OBJS = config.o context.o info.o ldapmap.o map.o params.o rule.o \
	session.o subst.o var.o xmap.o

LDAP_INCDIR= ../../include       
LDAP_LIBDIR= ../../libraries

LIBRARY = librewrite.a
PROGRAMS	= rewrite
XLIBS = $(LIBRARY) $(LDAP_LIBLUTIL_A) \
	$(LDAP_LIBLDAP_R_LA) $(LDAP_LIBLBER_LA)
XXLIBS  = $(SECURITY_LIBS) $(LUTIL_LIBS)
XXXLIBS = $(LTHREAD_LIBS)

rewrite:	$(XLIBS) rewrite.o parse.o
	$(LTLINK) -o $@ rewrite.o parse.o $(LIBS)
# $OpenLDAP: pkg/ldap/build/lib.mk,v 1.23.2.6 2010/04/13 20:22:20 kurt Exp $
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
# Makefile Template for Libraries
#

all-common: $(LIBRARY) $(PROGRAMS)

version.c: Makefile
	$(RM) $@
	$(MKVERSION) $(LIBRARY) > $@

version.o version.lo: version.c $(OBJS)

install-common: FORCE

lint: lint-local FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

lint5: lint5-local FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

#
# In the mingw/cygwin environment, the so and dll files must be
# deleted separately, instead of using the {.so*,*.dll} construct
# that was previously used. It just didn't work.
#
clean-common: 	FORCE
	$(RM) $(LIBRARY) ../$(LIBRARY) $(XLIBRARY) \
		$(PROGRAMS) $(XPROGRAMS) $(XSRCS) $(XXSRCS) \
		*.o *.lo a.out *.exe core version.c .libs/*

depend-common: FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS) $(XXSRCS)

lint-local: FORCE
lint5-local: FORCE

Makefile: $(top_srcdir)/build/lib.mk

# $OpenLDAP: pkg/ldap/build/lib-static.mk,v 1.13.2.5 2010/04/13 20:22:20 kurt Exp $
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
# Makefile Template for Static Libraries
#

$(LIBRARY): version.o
	$(AR) ru $@ $(OBJS) version.o
	@$(RANLIB) $@

Makefile: $(top_srcdir)/build/lib-static.mk
# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include config.c context.c info.c ldapmap.c map.c params.c rule.c session.c subst.c var.c xmap.c parse.c rewrite.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include config.c context.c info.c ldapmap.c map.c params.c rule.c session.c subst.c var.c xmap.c parse.c rewrite.c
#

config.o: config.c
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
config.o: ../../include/portable.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
config.o: ../../include/ac/fdset.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
config.o: ../../include/ldap_cdefs.h
config.o: ../../include/ldap_features.h
config.o: ../../include/ac/assert.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
config.o: ../../include/ac/localize.h
config.o: rewrite-int.h
config.o: ../../include/ac/stdlib.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
config.o: ../../include/ac/string.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
config.o: ../../include/ac/syslog.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
config.o: ../../include/ac/regex.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
config.o: ../../include/ac/socket.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
config.o: ../../include/ac/unistd.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
config.o: ../../include/lutil_lockf.h
config.o: ../../include/ac/ctype.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
config.o: ../../include/lber.h
config.o: ../../include/lber_types.h
config.o: ../../include/ldap_cdefs.h
config.o: ../../include/ldap.h
config.o: ../../include/ldap_features.h
config.o: ../libldap/ldap-int.h
config.o: ../libldap/../liblber/lber-int.h
config.o: ../../include/ldap_log.h
config.o: ../../include/lber_pvt.h
config.o: ../../include/ldap_queue.h
config.o: ../../include/lutil.h
config.o: ../../include/ac/time.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
config.o: ../../include/ldap_pvt.h
config.o: ../../include/ldap_pvt_uc.h
config.o: ../../include/ac/bytes.h
config.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
config.o: ../../include/avl.h
config.o: ../../include/rewrite.h
config.o: ../../include/ldap_pvt_thread.h
config.o: ../../include/ldap_int_thread.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
config.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
config.o: rewrite-map.h
context.o: context.c
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
context.o: ../../include/portable.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
context.o: ../../include/ac/fdset.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
context.o: ../../include/ldap_cdefs.h
context.o: ../../include/ldap_features.h
context.o: ../../include/ac/assert.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
context.o: ../../include/ac/localize.h
context.o: rewrite-int.h
context.o: ../../include/ac/stdlib.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
context.o: ../../include/ac/string.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
context.o: ../../include/ac/syslog.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
context.o: ../../include/ac/regex.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
context.o: ../../include/ac/socket.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
context.o: ../../include/ac/unistd.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
context.o: ../../include/lutil_lockf.h
context.o: ../../include/ac/ctype.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
context.o: ../../include/lber.h
context.o: ../../include/lber_types.h
context.o: ../../include/ldap_cdefs.h
context.o: ../../include/ldap.h
context.o: ../../include/ldap_features.h
context.o: ../libldap/ldap-int.h
context.o: ../libldap/../liblber/lber-int.h
context.o: ../../include/ldap_log.h
context.o: ../../include/lber_pvt.h
context.o: ../../include/ldap_queue.h
context.o: ../../include/lutil.h
context.o: ../../include/ac/time.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
context.o: ../../include/ldap_pvt.h
context.o: ../../include/ldap_pvt_uc.h
context.o: ../../include/ac/bytes.h
context.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
context.o: ../../include/avl.h
context.o: ../../include/rewrite.h
context.o: ../../include/ldap_pvt_thread.h
context.o: ../../include/ldap_int_thread.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
context.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
info.o: info.c
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
info.o: ../../include/portable.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
info.o: ../../include/ac/fdset.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
info.o: ../../include/ldap_cdefs.h
info.o: ../../include/ldap_features.h
info.o: ../../include/ac/assert.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
info.o: ../../include/ac/localize.h
info.o: rewrite-int.h
info.o: ../../include/ac/stdlib.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
info.o: ../../include/ac/string.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
info.o: ../../include/ac/syslog.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
info.o: ../../include/ac/regex.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
info.o: ../../include/ac/socket.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
info.o: ../../include/ac/unistd.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
info.o: ../../include/lutil_lockf.h
info.o: ../../include/ac/ctype.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
info.o: ../../include/lber.h
info.o: ../../include/lber_types.h
info.o: ../../include/ldap_cdefs.h
info.o: ../../include/ldap.h
info.o: ../../include/ldap_features.h
info.o: ../libldap/ldap-int.h
info.o: ../libldap/../liblber/lber-int.h
info.o: ../../include/ldap_log.h
info.o: ../../include/lber_pvt.h
info.o: ../../include/ldap_queue.h
info.o: ../../include/lutil.h
info.o: ../../include/ac/time.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
info.o: ../../include/ldap_pvt.h
info.o: ../../include/ldap_pvt_uc.h
info.o: ../../include/ac/bytes.h
info.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
info.o: ../../include/avl.h
info.o: ../../include/rewrite.h
info.o: ../../include/ldap_pvt_thread.h
info.o: ../../include/ldap_int_thread.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
info.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
ldapmap.o: ldapmap.c
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapmap.o: ../../include/portable.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapmap.o: ../../include/ac/fdset.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapmap.o: ../../include/ldap_cdefs.h
ldapmap.o: ../../include/ldap_features.h
ldapmap.o: ../../include/ac/assert.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapmap.o: ../../include/ac/localize.h
ldapmap.o: rewrite-int.h
ldapmap.o: ../../include/ac/stdlib.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapmap.o: ../../include/ac/string.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapmap.o: ../../include/ac/syslog.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
ldapmap.o: ../../include/ac/regex.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
ldapmap.o: ../../include/ac/socket.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapmap.o: ../../include/ac/unistd.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapmap.o: ../../include/lutil_lockf.h
ldapmap.o: ../../include/ac/ctype.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapmap.o: ../../include/lber.h
ldapmap.o: ../../include/lber_types.h
ldapmap.o: ../../include/ldap_cdefs.h
ldapmap.o: ../../include/ldap.h
ldapmap.o: ../../include/ldap_features.h
ldapmap.o: ../libldap/ldap-int.h
ldapmap.o: ../libldap/../liblber/lber-int.h
ldapmap.o: ../../include/ldap_log.h
ldapmap.o: ../../include/lber_pvt.h
ldapmap.o: ../../include/ldap_queue.h
ldapmap.o: ../../include/lutil.h
ldapmap.o: ../../include/ac/time.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapmap.o: ../../include/ldap_pvt.h
ldapmap.o: ../../include/ldap_pvt_uc.h
ldapmap.o: ../../include/ac/bytes.h
ldapmap.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
ldapmap.o: ../../include/avl.h
ldapmap.o: ../../include/rewrite.h
ldapmap.o: ../../include/ldap_pvt_thread.h
ldapmap.o: ../../include/ldap_int_thread.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
ldapmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
ldapmap.o: rewrite-map.h
map.o: map.c
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
map.o: ../../include/portable.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
map.o: ../../include/ac/fdset.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
map.o: ../../include/ldap_cdefs.h
map.o: ../../include/ldap_features.h
map.o: ../../include/ac/assert.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
map.o: ../../include/ac/localize.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pwd.h
map.o: rewrite-int.h
map.o: ../../include/ac/stdlib.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
map.o: ../../include/ac/string.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
map.o: ../../include/ac/syslog.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
map.o: ../../include/ac/regex.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
map.o: ../../include/ac/socket.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
map.o: ../../include/ac/unistd.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
map.o: ../../include/lutil_lockf.h
map.o: ../../include/ac/ctype.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
map.o: ../../include/lber.h
map.o: ../../include/lber_types.h
map.o: ../../include/ldap_cdefs.h
map.o: ../../include/ldap.h
map.o: ../../include/ldap_features.h
map.o: ../libldap/ldap-int.h
map.o: ../libldap/../liblber/lber-int.h
map.o: ../../include/ldap_log.h
map.o: ../../include/lber_pvt.h
map.o: ../../include/ldap_queue.h
map.o: ../../include/lutil.h
map.o: ../../include/ac/time.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
map.o: ../../include/ldap_pvt.h
map.o: ../../include/ldap_pvt_uc.h
map.o: ../../include/ac/bytes.h
map.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
map.o: ../../include/avl.h
map.o: ../../include/rewrite.h
map.o: ../../include/ldap_pvt_thread.h
map.o: ../../include/ldap_int_thread.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
map.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
map.o: rewrite-map.h
params.o: params.c
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
params.o: ../../include/portable.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
params.o: ../../include/ac/fdset.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
params.o: ../../include/ldap_cdefs.h
params.o: ../../include/ldap_features.h
params.o: ../../include/ac/assert.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
params.o: ../../include/ac/localize.h
params.o: rewrite-int.h
params.o: ../../include/ac/stdlib.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
params.o: ../../include/ac/string.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
params.o: ../../include/ac/syslog.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
params.o: ../../include/ac/regex.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
params.o: ../../include/ac/socket.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
params.o: ../../include/ac/unistd.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
params.o: ../../include/lutil_lockf.h
params.o: ../../include/ac/ctype.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
params.o: ../../include/lber.h
params.o: ../../include/lber_types.h
params.o: ../../include/ldap_cdefs.h
params.o: ../../include/ldap.h
params.o: ../../include/ldap_features.h
params.o: ../libldap/ldap-int.h
params.o: ../libldap/../liblber/lber-int.h
params.o: ../../include/ldap_log.h
params.o: ../../include/lber_pvt.h
params.o: ../../include/ldap_queue.h
params.o: ../../include/lutil.h
params.o: ../../include/ac/time.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
params.o: ../../include/ldap_pvt.h
params.o: ../../include/ldap_pvt_uc.h
params.o: ../../include/ac/bytes.h
params.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
params.o: ../../include/avl.h
params.o: ../../include/rewrite.h
params.o: ../../include/ldap_pvt_thread.h
params.o: ../../include/ldap_int_thread.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
params.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
rule.o: rule.c
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
rule.o: ../../include/portable.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
rule.o: ../../include/ac/fdset.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
rule.o: ../../include/ldap_cdefs.h
rule.o: ../../include/ldap_features.h
rule.o: ../../include/ac/assert.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
rule.o: ../../include/ac/localize.h
rule.o: rewrite-int.h
rule.o: ../../include/ac/stdlib.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
rule.o: ../../include/ac/string.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
rule.o: ../../include/ac/syslog.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
rule.o: ../../include/ac/regex.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
rule.o: ../../include/ac/socket.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
rule.o: ../../include/ac/unistd.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
rule.o: ../../include/lutil_lockf.h
rule.o: ../../include/ac/ctype.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
rule.o: ../../include/lber.h
rule.o: ../../include/lber_types.h
rule.o: ../../include/ldap_cdefs.h
rule.o: ../../include/ldap.h
rule.o: ../../include/ldap_features.h
rule.o: ../libldap/ldap-int.h
rule.o: ../libldap/../liblber/lber-int.h
rule.o: ../../include/ldap_log.h
rule.o: ../../include/lber_pvt.h
rule.o: ../../include/ldap_queue.h
rule.o: ../../include/lutil.h
rule.o: ../../include/ac/time.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
rule.o: ../../include/ldap_pvt.h
rule.o: ../../include/ldap_pvt_uc.h
rule.o: ../../include/ac/bytes.h
rule.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
rule.o: ../../include/avl.h
rule.o: ../../include/rewrite.h
rule.o: ../../include/ldap_pvt_thread.h
rule.o: ../../include/ldap_int_thread.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
rule.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
session.o: session.c
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
session.o: ../../include/portable.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
session.o: ../../include/ac/fdset.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
session.o: ../../include/ldap_cdefs.h
session.o: ../../include/ldap_features.h
session.o: ../../include/ac/assert.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
session.o: ../../include/ac/localize.h
session.o: rewrite-int.h
session.o: ../../include/ac/stdlib.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
session.o: ../../include/ac/string.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
session.o: ../../include/ac/syslog.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
session.o: ../../include/ac/regex.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
session.o: ../../include/ac/socket.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
session.o: ../../include/ac/unistd.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
session.o: ../../include/lutil_lockf.h
session.o: ../../include/ac/ctype.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
session.o: ../../include/lber.h
session.o: ../../include/lber_types.h
session.o: ../../include/ldap_cdefs.h
session.o: ../../include/ldap.h
session.o: ../../include/ldap_features.h
session.o: ../libldap/ldap-int.h
session.o: ../libldap/../liblber/lber-int.h
session.o: ../../include/ldap_log.h
session.o: ../../include/lber_pvt.h
session.o: ../../include/ldap_queue.h
session.o: ../../include/lutil.h
session.o: ../../include/ac/time.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
session.o: ../../include/ldap_pvt.h
session.o: ../../include/ldap_pvt_uc.h
session.o: ../../include/ac/bytes.h
session.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
session.o: ../../include/avl.h
session.o: ../../include/rewrite.h
session.o: ../../include/ldap_pvt_thread.h
session.o: ../../include/ldap_int_thread.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
session.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
subst.o: subst.c
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
subst.o: ../../include/portable.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
subst.o: ../../include/ac/fdset.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
subst.o: ../../include/ldap_cdefs.h
subst.o: ../../include/ldap_features.h
subst.o: ../../include/ac/assert.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
subst.o: ../../include/ac/localize.h
subst.o: rewrite-int.h
subst.o: ../../include/ac/stdlib.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
subst.o: ../../include/ac/string.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
subst.o: ../../include/ac/syslog.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
subst.o: ../../include/ac/regex.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
subst.o: ../../include/ac/socket.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
subst.o: ../../include/ac/unistd.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
subst.o: ../../include/lutil_lockf.h
subst.o: ../../include/ac/ctype.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
subst.o: ../../include/lber.h
subst.o: ../../include/lber_types.h
subst.o: ../../include/ldap_cdefs.h
subst.o: ../../include/ldap.h
subst.o: ../../include/ldap_features.h
subst.o: ../libldap/ldap-int.h
subst.o: ../libldap/../liblber/lber-int.h
subst.o: ../../include/ldap_log.h
subst.o: ../../include/lber_pvt.h
subst.o: ../../include/ldap_queue.h
subst.o: ../../include/lutil.h
subst.o: ../../include/ac/time.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
subst.o: ../../include/ldap_pvt.h
subst.o: ../../include/ldap_pvt_uc.h
subst.o: ../../include/ac/bytes.h
subst.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
subst.o: ../../include/avl.h
subst.o: ../../include/rewrite.h
subst.o: ../../include/ldap_pvt_thread.h
subst.o: ../../include/ldap_int_thread.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
subst.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
var.o: var.c
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
var.o: ../../include/portable.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
var.o: ../../include/ac/fdset.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
var.o: ../../include/ldap_cdefs.h
var.o: ../../include/ldap_features.h
var.o: ../../include/ac/assert.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
var.o: ../../include/ac/localize.h
var.o: rewrite-int.h
var.o: ../../include/ac/stdlib.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
var.o: ../../include/ac/string.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
var.o: ../../include/ac/syslog.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
var.o: ../../include/ac/regex.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
var.o: ../../include/ac/socket.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
var.o: ../../include/ac/unistd.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
var.o: ../../include/lutil_lockf.h
var.o: ../../include/ac/ctype.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
var.o: ../../include/lber.h
var.o: ../../include/lber_types.h
var.o: ../../include/ldap_cdefs.h
var.o: ../../include/ldap.h
var.o: ../../include/ldap_features.h
var.o: ../libldap/ldap-int.h
var.o: ../libldap/../liblber/lber-int.h
var.o: ../../include/ldap_log.h
var.o: ../../include/lber_pvt.h
var.o: ../../include/ldap_queue.h
var.o: ../../include/lutil.h
var.o: ../../include/ac/time.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
var.o: ../../include/ldap_pvt.h
var.o: ../../include/ldap_pvt_uc.h
var.o: ../../include/ac/bytes.h
var.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
var.o: ../../include/avl.h
var.o: ../../include/rewrite.h
var.o: ../../include/ldap_pvt_thread.h
var.o: ../../include/ldap_int_thread.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
var.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
xmap.o: xmap.c
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
xmap.o: ../../include/portable.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
xmap.o: ../../include/ac/fdset.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
xmap.o: ../../include/ldap_cdefs.h
xmap.o: ../../include/ldap_features.h
xmap.o: ../../include/ac/assert.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
xmap.o: ../../include/ac/localize.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pwd.h
xmap.o: rewrite-int.h
xmap.o: ../../include/ac/stdlib.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
xmap.o: ../../include/ac/string.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
xmap.o: ../../include/ac/syslog.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
xmap.o: ../../include/ac/regex.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
xmap.o: ../../include/ac/socket.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
xmap.o: ../../include/ac/unistd.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
xmap.o: ../../include/lutil_lockf.h
xmap.o: ../../include/ac/ctype.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
xmap.o: ../../include/lber.h
xmap.o: ../../include/lber_types.h
xmap.o: ../../include/ldap_cdefs.h
xmap.o: ../../include/ldap.h
xmap.o: ../../include/ldap_features.h
xmap.o: ../libldap/ldap-int.h
xmap.o: ../libldap/../liblber/lber-int.h
xmap.o: ../../include/ldap_log.h
xmap.o: ../../include/lber_pvt.h
xmap.o: ../../include/ldap_queue.h
xmap.o: ../../include/lutil.h
xmap.o: ../../include/ac/time.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
xmap.o: ../../include/ldap_pvt.h
xmap.o: ../../include/ldap_pvt_uc.h
xmap.o: ../../include/ac/bytes.h
xmap.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
xmap.o: ../../include/avl.h
xmap.o: ../../include/rewrite.h
xmap.o: ../../include/ldap_pvt_thread.h
xmap.o: ../../include/ldap_int_thread.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
xmap.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
xmap.o: rewrite-map.h
parse.o: parse.c
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
parse.o: ../../include/portable.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
parse.o: ../../include/ac/fdset.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
parse.o: ../../include/ldap_cdefs.h
parse.o: ../../include/ldap_features.h
parse.o: ../../include/ac/assert.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
parse.o: ../../include/ac/localize.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
parse.o: rewrite-int.h
parse.o: ../../include/ac/stdlib.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
parse.o: ../../include/ac/string.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
parse.o: ../../include/ac/syslog.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
parse.o: ../../include/ac/regex.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
parse.o: ../../include/ac/socket.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
parse.o: ../../include/ac/unistd.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
parse.o: ../../include/lutil_lockf.h
parse.o: ../../include/ac/ctype.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
parse.o: ../../include/lber.h
parse.o: ../../include/lber_types.h
parse.o: ../../include/ldap_cdefs.h
parse.o: ../../include/ldap.h
parse.o: ../../include/ldap_features.h
parse.o: ../libldap/ldap-int.h
parse.o: ../libldap/../liblber/lber-int.h
parse.o: ../../include/ldap_log.h
parse.o: ../../include/lber_pvt.h
parse.o: ../../include/ldap_queue.h
parse.o: ../../include/lutil.h
parse.o: ../../include/ac/time.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
parse.o: ../../include/ldap_pvt.h
parse.o: ../../include/ldap_pvt_uc.h
parse.o: ../../include/ac/bytes.h
parse.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
parse.o: ../../include/avl.h
parse.o: ../../include/rewrite.h
parse.o: ../../include/ldap_pvt_thread.h
parse.o: ../../include/ldap_int_thread.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
parse.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
rewrite.o: rewrite.c
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
rewrite.o: ../../include/portable.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
rewrite.o: ../../include/ac/fdset.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
rewrite.o: ../../include/ldap_cdefs.h
rewrite.o: ../../include/ldap_features.h
rewrite.o: ../../include/ac/assert.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
rewrite.o: ../../include/ac/localize.h
rewrite.o: ../../include/ac/stdlib.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
rewrite.o: ../../include/ac/string.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
rewrite.o: ../../include/ac/syslog.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
rewrite.o: ../../include/ac/regex.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
rewrite.o: ../../include/ac/socket.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
rewrite.o: ../../include/ac/unistd.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
rewrite.o: ../../include/lutil_lockf.h
rewrite.o: ../../include/ac/ctype.h
rewrite.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
rewrite.o: ../../include/rewrite.h
rewrite.o: ../../include/lutil.h
rewrite.o: ../../include/ldap_cdefs.h
rewrite.o: ../../include/lber_types.h
rewrite.o: ../../include/ldap.h
rewrite.o: ../../include/lber.h
rewrite.o: ../../include/ldap_features.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
