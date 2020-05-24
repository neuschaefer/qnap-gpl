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

# LIBLBER
# $OpenLDAP: pkg/ldap/libraries/liblber/Makefile.in,v 1.37.2.6 2010/04/13 20:22:53 kurt Exp $
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

LIBRARY = liblber.la

NT_SRCS = nt_err.c
NT_OBJS = nt_err.lo

UNIX_SRCS = stdio.c
UNIX_OBJS = stdio.lo

LIB_DEFS = -DLBER_LIBRARY

SRCS= assert.c decode.c encode.c io.c bprint.c debug.c \
	memory.c options.c sockbuf.c $(UNIX_SRCS)
OBJS= assert.lo decode.lo encode.lo io.lo bprint.lo debug.lo \
	memory.lo options.lo sockbuf.lo $(UNIX_OBJS)
XSRCS= version.c

PROGRAMS= dtest etest idtest

LDAP_INCDIR= ../../include
LDAP_LIBDIR= ../../libraries

XLIBS = $(LIBRARY) $(LDAP_LIBLUTIL_A)
XXLIBS = 
NT_LINK_LIBS = $(AC_LIBS)
UNIX_LINK_LIBS = $(AC_LIBS)

dtest:    $(XLIBS) dtest.o
	$(LTLINK) -o $@ dtest.o $(LIBS)
etest:  $(XLIBS) etest.o
	$(LTLINK) -o $@ etest.o $(LIBS)
idtest:  $(XLIBS) idtest.o
	$(LTLINK) -o $@ idtest.o $(LIBS)

install-local: FORCE
	-$(MKDIR) $(DESTDIR)$(libdir)
	$(LTINSTALL) $(INSTALLFLAGS) -m 644 $(LIBRARY) $(DESTDIR)$(libdir)
	$(LTFINISH) $(DESTDIR)$(libdir)

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

# $OpenLDAP: pkg/ldap/build/lib-shared.mk,v 1.22.2.5 2010/04/13 20:22:20 kurt Exp $
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
# Makefile Template for Shared Libraries
#

MKDEPFLAG = -l

.SUFFIXES: .c .o .lo

.c.lo:
	$(LTCOMPILE_LIB) $<

$(LIBRARY): version.lo
	$(LTLINK_LIB) -o $@ $(OBJS) version.lo $(LINK_LIBS)

Makefile: $(top_srcdir)/build/lib-shared.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include assert.c decode.c encode.c io.c bprint.c debug.c memory.c options.c sockbuf.c stdio.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include assert.c decode.c encode.c io.c bprint.c debug.c memory.c options.c sockbuf.c stdio.c
#

assert.lo: assert.c
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
assert.lo: ../../include/portable.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
assert.lo: ../../include/ac/fdset.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
assert.lo: ../../include/ldap_cdefs.h
assert.lo: ../../include/ldap_features.h
assert.lo: ../../include/ac/assert.h
assert.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
assert.lo: ../../include/ac/localize.h
decode.lo: decode.c
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
decode.lo: ../../include/portable.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
decode.lo: ../../include/ac/fdset.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
decode.lo: ../../include/ldap_cdefs.h
decode.lo: ../../include/ldap_features.h
decode.lo: ../../include/ac/assert.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
decode.lo: ../../include/ac/localize.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
decode.lo: ../../include/ac/stdlib.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
decode.lo: ../../include/ac/stdarg.h
decode.lo: ../../include/ac/string.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
decode.lo: ../../include/ac/socket.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
decode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
decode.lo: lber-int.h
decode.lo: ../../include/lber.h
decode.lo: ../../include/lber_types.h
decode.lo: ../../include/ldap_cdefs.h
decode.lo: ../../include/ldap_log.h
decode.lo: ../../include/lber_pvt.h
decode.lo: ../../include/ldap_queue.h
encode.lo: encode.c
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
encode.lo: ../../include/portable.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
encode.lo: ../../include/ac/fdset.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
encode.lo: ../../include/ldap_cdefs.h
encode.lo: ../../include/ldap_features.h
encode.lo: ../../include/ac/assert.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
encode.lo: ../../include/ac/localize.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
encode.lo: ../../include/ac/stdlib.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
encode.lo: ../../include/ac/stdarg.h
encode.lo: ../../include/ac/socket.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
encode.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
encode.lo: ../../include/ac/string.h
encode.lo: lber-int.h
encode.lo: ../../include/lber.h
encode.lo: ../../include/lber_types.h
encode.lo: ../../include/ldap_cdefs.h
encode.lo: ../../include/ldap_log.h
encode.lo: ../../include/lber_pvt.h
encode.lo: ../../include/ldap_queue.h
io.lo: io.c
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
io.lo: ../../include/portable.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
io.lo: ../../include/ac/fdset.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
io.lo: ../../include/ldap_cdefs.h
io.lo: ../../include/ldap_features.h
io.lo: ../../include/ac/assert.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
io.lo: ../../include/ac/localize.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
io.lo: ../../include/ac/stdlib.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
io.lo: ../../include/ac/ctype.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
io.lo: ../../include/ac/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
io.lo: ../../include/ac/socket.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
io.lo: ../../include/ac/string.h
io.lo: ../../include/ac/unistd.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
io.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
io.lo: ../../include/lutil_lockf.h
io.lo: lber-int.h
io.lo: ../../include/lber.h
io.lo: ../../include/lber_types.h
io.lo: ../../include/ldap_cdefs.h
io.lo: ../../include/ldap_log.h
io.lo: ../../include/lber_pvt.h
io.lo: ../../include/ldap_queue.h
bprint.lo: bprint.c
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
bprint.lo: ../../include/portable.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
bprint.lo: ../../include/ac/fdset.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
bprint.lo: ../../include/ldap_cdefs.h
bprint.lo: ../../include/ldap_features.h
bprint.lo: ../../include/ac/assert.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
bprint.lo: ../../include/ac/localize.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
bprint.lo: ../../include/ac/ctype.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
bprint.lo: ../../include/ac/stdarg.h
bprint.lo: ../../include/ac/string.h
bprint.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
bprint.lo: lber-int.h
bprint.lo: ../../include/lber.h
bprint.lo: ../../include/lber_types.h
bprint.lo: ../../include/ldap_cdefs.h
bprint.lo: ../../include/ldap_log.h
bprint.lo: ../../include/lber_pvt.h
bprint.lo: ../../include/ldap_queue.h
debug.lo: debug.c
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
debug.lo: ../../include/portable.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
debug.lo: ../../include/ac/fdset.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
debug.lo: ../../include/ldap_cdefs.h
debug.lo: ../../include/ldap_features.h
debug.lo: ../../include/ac/assert.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
debug.lo: ../../include/ac/localize.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
debug.lo: ../../include/ac/stdarg.h
debug.lo: ../../include/ac/stdlib.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
debug.lo: ../../include/ac/string.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
debug.lo: ../../include/ac/time.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
debug.lo: ../../include/ac/ctype.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
debug.lo: ../../include/ac/syslog.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
debug.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
debug.lo: ../../include/ldap_log.h
debug.lo: ../../include/ldap_cdefs.h
debug.lo: ../../include/ldap_defaults.h
debug.lo: ../../include/ldap_config.h
debug.lo: ../../include/lber.h
debug.lo: ../../include/lber_types.h
debug.lo: ../../include/ldap_pvt.h
debug.lo: ../../include/lber_pvt.h
debug.lo: ../../include/ldap_pvt_uc.h
debug.lo: ../../include/ac/bytes.h
debug.lo: ../../include/../libraries/liblunicode/ucdata/ucdata.h
memory.lo: memory.c
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
memory.lo: ../../include/portable.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
memory.lo: ../../include/ac/fdset.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
memory.lo: ../../include/ldap_cdefs.h
memory.lo: ../../include/ldap_features.h
memory.lo: ../../include/ac/assert.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
memory.lo: ../../include/ac/localize.h
memory.lo: ../../include/ac/stdlib.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
memory.lo: ../../include/ac/string.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
memory.lo: lber-int.h
memory.lo: ../../include/lber.h
memory.lo: ../../include/lber_types.h
memory.lo: ../../include/ldap_cdefs.h
memory.lo: ../../include/ldap_log.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
memory.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
memory.lo: ../../include/lber_pvt.h
memory.lo: ../../include/ldap_queue.h
options.lo: options.c
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
options.lo: ../../include/portable.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
options.lo: ../../include/ac/fdset.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
options.lo: ../../include/ldap_cdefs.h
options.lo: ../../include/ldap_features.h
options.lo: ../../include/ac/assert.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
options.lo: ../../include/ac/localize.h
options.lo: ../../include/ac/stdlib.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
options.lo: ../../include/ac/string.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
options.lo: ../../include/ac/stdarg.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
options.lo: lber-int.h
options.lo: ../../include/lber.h
options.lo: ../../include/lber_types.h
options.lo: ../../include/ldap_cdefs.h
options.lo: ../../include/ldap_log.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
options.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
options.lo: ../../include/lber_pvt.h
options.lo: ../../include/ldap_queue.h
sockbuf.lo: sockbuf.c
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
sockbuf.lo: ../../include/portable.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
sockbuf.lo: ../../include/ac/fdset.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
sockbuf.lo: ../../include/ldap_cdefs.h
sockbuf.lo: ../../include/ldap_features.h
sockbuf.lo: ../../include/ac/assert.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
sockbuf.lo: ../../include/ac/localize.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
sockbuf.lo: ../../include/ac/stdlib.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
sockbuf.lo: ../../include/ac/ctype.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
sockbuf.lo: ../../include/ac/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
sockbuf.lo: ../../include/ac/socket.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
sockbuf.lo: ../../include/ac/string.h
sockbuf.lo: ../../include/ac/unistd.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
sockbuf.lo: ../../include/lutil_lockf.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/fcntl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl-linux.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stat.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ioctl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/ioctls.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/ioctls.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/ioctls.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/ioctl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/ioctl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/ioctl.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/ioctl-types.h
sockbuf.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ttydefaults.h
sockbuf.lo: lber-int.h
sockbuf.lo: ../../include/lber.h
sockbuf.lo: ../../include/lber_types.h
sockbuf.lo: ../../include/ldap_cdefs.h
sockbuf.lo: ../../include/ldap_log.h
sockbuf.lo: ../../include/lber_pvt.h
sockbuf.lo: ../../include/ldap_queue.h
stdio.lo: stdio.c
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
stdio.lo: ../../include/portable.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
stdio.lo: ../../include/ac/fdset.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
stdio.lo: ../../include/ldap_cdefs.h
stdio.lo: ../../include/ldap_features.h
stdio.lo: ../../include/ac/assert.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
stdio.lo: ../../include/ac/localize.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
stdio.lo: ../../include/ac/stdarg.h
stdio.lo: ../../include/ac/string.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
stdio.lo: ../../include/ac/ctype.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
stdio.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
stdio.lo: ../../include/lutil.h
stdio.lo: ../../include/ldap_cdefs.h
stdio.lo: ../../include/lber_types.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
