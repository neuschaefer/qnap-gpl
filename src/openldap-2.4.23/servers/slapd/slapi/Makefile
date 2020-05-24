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

# Makefile.in for SLAPI
# $OpenLDAP: pkg/ldap/servers/slapd/slapi/Makefile.in,v 1.18.2.5 2010/04/13 20:23:50 kurt Exp $
## This work is part of OpenLDAP Software <http://www.openldap.org/>.
##
## Copyright 1998-2010 The OpenLDAP Foundation.
## Portions Copyright IBM Corp. 1997,2002,2003
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted only as authorized by the OpenLDAP
## Public License.
##
## A copy of this license is available in the file LICENSE in the
## top-level directory of the distribution or, alternatively, at
## <http://www.OpenLDAP.org/license.html>.

LIBRARY = libslapi.la

#all-common: $(LIBRARY) $(PROGRAMS)
#	@touch plugin.c slapi_pblock.c slapi_utils.c slapi_ops.c slapi_ext.c

NT_SRCS = nt_err.c
NT_OBJS = nt_err.lo

LIB_DEFS = -DSLAPI_LIBRARY

SRCS=  plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c \
	$(UNIX_SRCS)
OBJS=  plugin.lo slapi_pblock.lo slapi_utils.lo printmsg.lo slapi_ops.lo slapi_dn.lo slapi_ext.lo slapi_overlay.lo \
	$(UNIX_SRCS)

XSRCS= version.c

LDAP_INCDIR= ../../../include -I.. -I.       
LDAP_LIBDIR= ../../../libraries       

XLIBS = $(LIBRARY)
XXLIBS = 
NT_LINK_LIBS = $(AC_LIBS)

XINCPATH = -I$(srcdir)/.. -I$(srcdir)
XDEFS = $(MODULES_CPPFLAGS)

BUILD_MOD = no

install-local: FORCE
	if test "$(BUILD_MOD)" = "yes"; then \
		$(MKDIR) $(DESTDIR)$(libdir); \
		$(LTINSTALL) $(INSTALLFLAGS) -m 644 $(LIBRARY) $(DESTDIR)$(libdir); \
	fi

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
# files: -I../../../include -I.. -I. -I../../../include -I./.. -I. -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../../include -I.. -I. -I../../../include -I./.. -I. -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include plugin.c slapi_pblock.c slapi_utils.c printmsg.c slapi_ops.c slapi_dn.c slapi_ext.c slapi_overlay.c
#

plugin.lo: plugin.c
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
plugin.lo: ../../../include/portable.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
plugin.lo: ../../../include/ac/fdset.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
plugin.lo: ../../../include/ldap_cdefs.h
plugin.lo: ../../../include/ldap_features.h
plugin.lo: ../../../include/ac/assert.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
plugin.lo: ../../../include/ac/localize.h
plugin.lo: ../../../include/ldap_pvt_thread.h
plugin.lo: ../../../include/ldap_int_thread.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
plugin.lo: ../slap.h
plugin.lo: ../../../include/ldap_defaults.h
plugin.lo: ../../../include/ldap_config.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
plugin.lo: ../../../include/ac/stdlib.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
plugin.lo: ../../../include/ac/syslog.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
plugin.lo: ../../../include/ac/regex.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
plugin.lo: ../../../include/ac/signal.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
plugin.lo: ../../../include/ac/socket.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
plugin.lo: ../../../include/ac/time.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
plugin.lo: ../../../include/ac/param.h
plugin.lo: ../../../include/ac/unistd.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
plugin.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
plugin.lo: ../../../include/lutil_lockf.h
plugin.lo: ../../../include/avl.h
plugin.lo: ../../../include/ldap_cdefs.h
plugin.lo: ../../../include/ldap_log.h
plugin.lo: ../../../include/ldap.h
plugin.lo: ../../../include/lber.h
plugin.lo: ../../../include/lber_types.h
plugin.lo: ../../../include/ldap_features.h
plugin.lo: ../../../include/ldap_schema.h
plugin.lo: ../../../include/lber_pvt.h
plugin.lo: ../../../include/ldap_pvt.h
plugin.lo: ../../../include/ldap_pvt_uc.h
plugin.lo: ../../../include/ac/bytes.h
plugin.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
plugin.lo: ../../../include/ldap_queue.h
plugin.lo: ../proto-slap.h
plugin.lo: ../config.h
plugin.lo: ../../../include/ac/string.h
plugin.lo: slapi.h
plugin.lo: ../../../include/lutil.h
plugin.lo: /root/daily_build/64_01/4.4.1/LinkFS/include/ltdl.h
slapi_pblock.lo: slapi_pblock.c
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_pblock.lo: ../../../include/portable.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_pblock.lo: ../../../include/ac/fdset.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_pblock.lo: ../../../include/ldap_cdefs.h
slapi_pblock.lo: ../../../include/ldap_features.h
slapi_pblock.lo: ../../../include/ac/assert.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_pblock.lo: ../../../include/ac/localize.h
slapi_pblock.lo: ../slap.h
slapi_pblock.lo: ../../../include/ldap_defaults.h
slapi_pblock.lo: ../../../include/ldap_config.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
slapi_pblock.lo: ../../../include/ac/stdlib.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
slapi_pblock.lo: ../../../include/ac/syslog.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
slapi_pblock.lo: ../../../include/ac/regex.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
slapi_pblock.lo: ../../../include/ac/signal.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
slapi_pblock.lo: ../../../include/ac/socket.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
slapi_pblock.lo: ../../../include/ac/time.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
slapi_pblock.lo: ../../../include/ac/param.h
slapi_pblock.lo: ../../../include/ac/unistd.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_pblock.lo: ../../../include/lutil_lockf.h
slapi_pblock.lo: ../../../include/avl.h
slapi_pblock.lo: ../../../include/ldap_cdefs.h
slapi_pblock.lo: ../../../include/ldap_log.h
slapi_pblock.lo: ../../../include/ldap.h
slapi_pblock.lo: ../../../include/lber.h
slapi_pblock.lo: ../../../include/lber_types.h
slapi_pblock.lo: ../../../include/ldap_features.h
slapi_pblock.lo: ../../../include/ldap_schema.h
slapi_pblock.lo: ../../../include/lber_pvt.h
slapi_pblock.lo: ../../../include/ldap_pvt.h
slapi_pblock.lo: ../../../include/ldap_pvt_uc.h
slapi_pblock.lo: ../../../include/ac/bytes.h
slapi_pblock.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_pblock.lo: ../../../include/ldap_pvt_thread.h
slapi_pblock.lo: ../../../include/ldap_int_thread.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
slapi_pblock.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
slapi_pblock.lo: ../../../include/ldap_queue.h
slapi_pblock.lo: ../proto-slap.h
slapi_pblock.lo: slapi.h
slapi_utils.lo: slapi_utils.c
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_utils.lo: ../../../include/portable.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_utils.lo: ../../../include/ac/fdset.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_utils.lo: ../../../include/ldap_cdefs.h
slapi_utils.lo: ../../../include/ldap_features.h
slapi_utils.lo: ../../../include/ac/assert.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_utils.lo: ../../../include/ac/localize.h
slapi_utils.lo: ../../../include/ac/string.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_utils.lo: ../../../include/ac/stdarg.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_utils.lo: ../../../include/ac/ctype.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_utils.lo: ../../../include/ac/unistd.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_utils.lo: ../../../include/lutil_lockf.h
slapi_utils.lo: ../../../include/lutil.h
slapi_utils.lo: ../../../include/ldap_cdefs.h
slapi_utils.lo: ../../../include/lber_types.h
slapi_utils.lo: ../slap.h
slapi_utils.lo: ../../../include/ldap_defaults.h
slapi_utils.lo: ../../../include/ldap_config.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
slapi_utils.lo: ../../../include/ac/stdlib.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
slapi_utils.lo: ../../../include/ac/syslog.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
slapi_utils.lo: ../../../include/ac/regex.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
slapi_utils.lo: ../../../include/ac/signal.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
slapi_utils.lo: ../../../include/ac/socket.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
slapi_utils.lo: ../../../include/ac/time.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
slapi_utils.lo: ../../../include/ac/param.h
slapi_utils.lo: ../../../include/avl.h
slapi_utils.lo: ../../../include/ldap_log.h
slapi_utils.lo: ../../../include/ldap.h
slapi_utils.lo: ../../../include/lber.h
slapi_utils.lo: ../../../include/ldap_features.h
slapi_utils.lo: ../../../include/ldap_schema.h
slapi_utils.lo: ../../../include/lber_pvt.h
slapi_utils.lo: ../../../include/ldap_pvt.h
slapi_utils.lo: ../../../include/ldap_pvt_uc.h
slapi_utils.lo: ../../../include/ac/bytes.h
slapi_utils.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_utils.lo: ../../../include/ldap_pvt_thread.h
slapi_utils.lo: ../../../include/ldap_int_thread.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
slapi_utils.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
slapi_utils.lo: ../../../include/ldap_queue.h
slapi_utils.lo: ../proto-slap.h
slapi_utils.lo: slapi.h
printmsg.lo: printmsg.c
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
printmsg.lo: ../../../include/portable.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
printmsg.lo: ../../../include/ac/fdset.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
printmsg.lo: ../../../include/ldap_cdefs.h
printmsg.lo: ../../../include/ldap_features.h
printmsg.lo: ../../../include/ac/assert.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
printmsg.lo: ../../../include/ac/localize.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
printmsg.lo: ../../../include/ac/string.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
printmsg.lo: ../../../include/ac/stdarg.h
printmsg.lo: ../../../include/ac/unistd.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
printmsg.lo: ../../../include/lutil_lockf.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/fcntl.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl-linux.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stat.h
printmsg.lo: ../../../include/ac/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
printmsg.lo: ../../../include/ldap.h
printmsg.lo: ../../../include/lber.h
printmsg.lo: ../../../include/lber_types.h
printmsg.lo: ../../../include/ldap_cdefs.h
printmsg.lo: ../../../include/ldap_features.h
printmsg.lo: ../../../include/ldap_config.h
printmsg.lo: ../slap.h
printmsg.lo: ../../../include/ldap_defaults.h
printmsg.lo: ../../../include/ac/stdlib.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
printmsg.lo: ../../../include/ac/syslog.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
printmsg.lo: ../../../include/ac/regex.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
printmsg.lo: ../../../include/ac/signal.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
printmsg.lo: ../../../include/ac/socket.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
printmsg.lo: ../../../include/ac/time.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
printmsg.lo: ../../../include/ac/param.h
printmsg.lo: ../../../include/avl.h
printmsg.lo: ../../../include/ldap_log.h
printmsg.lo: ../../../include/ldap_schema.h
printmsg.lo: ../../../include/lber_pvt.h
printmsg.lo: ../../../include/ldap_pvt.h
printmsg.lo: ../../../include/ldap_pvt_uc.h
printmsg.lo: ../../../include/ac/bytes.h
printmsg.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
printmsg.lo: ../../../include/ldap_pvt_thread.h
printmsg.lo: ../../../include/ldap_int_thread.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
printmsg.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
printmsg.lo: ../../../include/ldap_queue.h
printmsg.lo: ../proto-slap.h
printmsg.lo: slapi.h
slapi_ops.lo: slapi_ops.c
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_ops.lo: ../../../include/portable.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_ops.lo: ../../../include/ac/fdset.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_ops.lo: ../../../include/ldap_cdefs.h
slapi_ops.lo: ../../../include/ldap_features.h
slapi_ops.lo: ../../../include/ac/assert.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_ops.lo: ../../../include/ac/localize.h
slapi_ops.lo: ../../../include/ac/string.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_ops.lo: ../../../include/ac/stdarg.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_ops.lo: ../../../include/ac/ctype.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_ops.lo: ../../../include/ac/unistd.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_ops.lo: ../../../include/lutil_lockf.h
slapi_ops.lo: ../slap.h
slapi_ops.lo: ../../../include/ldap_defaults.h
slapi_ops.lo: ../../../include/ldap_config.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
slapi_ops.lo: ../../../include/ac/stdlib.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
slapi_ops.lo: ../../../include/ac/syslog.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
slapi_ops.lo: ../../../include/ac/regex.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
slapi_ops.lo: ../../../include/ac/signal.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
slapi_ops.lo: ../../../include/ac/socket.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
slapi_ops.lo: ../../../include/ac/time.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
slapi_ops.lo: ../../../include/ac/param.h
slapi_ops.lo: ../../../include/avl.h
slapi_ops.lo: ../../../include/ldap_cdefs.h
slapi_ops.lo: ../../../include/ldap_log.h
slapi_ops.lo: ../../../include/ldap.h
slapi_ops.lo: ../../../include/lber.h
slapi_ops.lo: ../../../include/lber_types.h
slapi_ops.lo: ../../../include/ldap_features.h
slapi_ops.lo: ../../../include/ldap_schema.h
slapi_ops.lo: ../../../include/lber_pvt.h
slapi_ops.lo: ../../../include/ldap_pvt.h
slapi_ops.lo: ../../../include/ldap_pvt_uc.h
slapi_ops.lo: ../../../include/ac/bytes.h
slapi_ops.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_ops.lo: ../../../include/ldap_pvt_thread.h
slapi_ops.lo: ../../../include/ldap_int_thread.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
slapi_ops.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
slapi_ops.lo: ../../../include/ldap_queue.h
slapi_ops.lo: ../proto-slap.h
slapi_ops.lo: slapi.h
slapi_dn.lo: slapi_dn.c
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_dn.lo: ../../../include/portable.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_dn.lo: ../../../include/ac/fdset.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_dn.lo: ../../../include/ldap_cdefs.h
slapi_dn.lo: ../../../include/ldap_features.h
slapi_dn.lo: ../../../include/ac/assert.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_dn.lo: ../../../include/ac/localize.h
slapi_dn.lo: ../../../include/ac/string.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_dn.lo: ../../../include/ac/stdarg.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_dn.lo: ../../../include/ac/ctype.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_dn.lo: ../../../include/ac/unistd.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_dn.lo: ../../../include/lutil_lockf.h
slapi_dn.lo: ../../../include/ldap_pvt.h
slapi_dn.lo: ../../../include/lber.h
slapi_dn.lo: ../../../include/lber_types.h
slapi_dn.lo: ../../../include/ldap_cdefs.h
slapi_dn.lo: ../../../include/lber_pvt.h
slapi_dn.lo: ../../../include/ldap_pvt_uc.h
slapi_dn.lo: ../../../include/ac/bytes.h
slapi_dn.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
slapi_dn.lo: ../slap.h
slapi_dn.lo: ../../../include/ldap_defaults.h
slapi_dn.lo: ../../../include/ldap_config.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
slapi_dn.lo: ../../../include/ac/stdlib.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
slapi_dn.lo: ../../../include/ac/syslog.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
slapi_dn.lo: ../../../include/ac/regex.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
slapi_dn.lo: ../../../include/ac/signal.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
slapi_dn.lo: ../../../include/ac/socket.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
slapi_dn.lo: ../../../include/ac/time.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
slapi_dn.lo: ../../../include/ac/param.h
slapi_dn.lo: ../../../include/avl.h
slapi_dn.lo: ../../../include/ldap_log.h
slapi_dn.lo: ../../../include/ldap.h
slapi_dn.lo: ../../../include/ldap_features.h
slapi_dn.lo: ../../../include/ldap_schema.h
slapi_dn.lo: ../../../include/ldap_pvt_thread.h
slapi_dn.lo: ../../../include/ldap_int_thread.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
slapi_dn.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
slapi_dn.lo: ../../../include/ldap_queue.h
slapi_dn.lo: ../proto-slap.h
slapi_dn.lo: slapi.h
slapi_ext.lo: slapi_ext.c
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_ext.lo: ../../../include/portable.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_ext.lo: ../../../include/ac/fdset.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_ext.lo: ../../../include/ldap_cdefs.h
slapi_ext.lo: ../../../include/ldap_features.h
slapi_ext.lo: ../../../include/ac/assert.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_ext.lo: ../../../include/ac/localize.h
slapi_ext.lo: ../../../include/ac/string.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_ext.lo: ../../../include/ac/stdarg.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_ext.lo: ../../../include/ac/ctype.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_ext.lo: ../../../include/ac/unistd.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_ext.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_ext.lo: ../../../include/lutil_lockf.h
slapi_overlay.lo: slapi_overlay.c
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
slapi_overlay.lo: ../../../include/portable.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
slapi_overlay.lo: ../../../include/ac/fdset.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
slapi_overlay.lo: ../../../include/ldap_cdefs.h
slapi_overlay.lo: ../../../include/ldap_features.h
slapi_overlay.lo: ../../../include/ac/assert.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
slapi_overlay.lo: ../../../include/ac/localize.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
slapi_overlay.lo: ../../../include/ac/string.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
slapi_overlay.lo: ../../../include/ac/socket.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
slapi_overlay.lo: ../slap.h
slapi_overlay.lo: ../../../include/ldap_defaults.h
slapi_overlay.lo: ../../../include/ldap_config.h
slapi_overlay.lo: ../../../include/ac/stdlib.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
slapi_overlay.lo: ../../../include/ac/syslog.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/syslog.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/syslog.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/syslog-path.h
slapi_overlay.lo: ../../../include/ac/regex.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/regex.h
slapi_overlay.lo: ../../../include/ac/signal.h
slapi_overlay.lo: ../../../include/ac/time.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
slapi_overlay.lo: ../../../include/ac/param.h
slapi_overlay.lo: ../../../include/ac/unistd.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
slapi_overlay.lo: ../../../include/lutil_lockf.h
slapi_overlay.lo: ../../../include/avl.h
slapi_overlay.lo: ../../../include/ldap_cdefs.h
slapi_overlay.lo: ../../../include/ldap_log.h
slapi_overlay.lo: ../../../include/ldap.h
slapi_overlay.lo: ../../../include/lber.h
slapi_overlay.lo: ../../../include/lber_types.h
slapi_overlay.lo: ../../../include/ldap_features.h
slapi_overlay.lo: ../../../include/ldap_schema.h
slapi_overlay.lo: ../../../include/lber_pvt.h
slapi_overlay.lo: ../../../include/ldap_pvt.h
slapi_overlay.lo: ../../../include/ldap_pvt_uc.h
slapi_overlay.lo: ../../../include/ac/bytes.h
slapi_overlay.lo: ../../../include/../libraries/liblunicode/ucdata/ucdata.h
slapi_overlay.lo: ../../../include/ldap_pvt_thread.h
slapi_overlay.lo: ../../../include/ldap_int_thread.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/pthread.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sched.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sched.h
slapi_overlay.lo: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/setjmp.h
slapi_overlay.lo: ../../../include/ldap_queue.h
slapi_overlay.lo: ../proto-slap.h
slapi_overlay.lo: slapi.h
slapi_overlay.lo: ../config.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
