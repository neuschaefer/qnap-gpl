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

# Makefile for LDAP tools
# $OpenLDAP: pkg/ldap/clients/tools/Makefile.in,v 1.45.2.6 2010/04/13 20:22:22 kurt Exp $
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

SRCS	= ldapsearch.c ldapmodify.c ldapdelete.c ldapmodrdn.c \
		ldappasswd.c ldapwhoami.c ldapcompare.c \
		ldapexop.c ldapurl.c common.c
OBJS	= ldapsearch.o ldapmodify.o ldapdelete.o ldapmodrdn.o \
		ldappasswd.o ldapwhoami.o ldapcompare.o \
		ldapexop.o ldapurl.o common.o

LDAP_INCDIR= ../../include       
LDAP_LIBDIR= ../../libraries

MKVOPTS = -s

XLIBS =  $(LDAP_L)
XXLIBS	= $(SECURITY_LIBS) $(LUTIL_LIBS)

XSRCS	= ldsversion.c ldmversion.c lddversion.c ldrversion.c \
	ldpversion.c ldwversion.c ldcversion.c ldeversion.c lduversion.c

PROGRAMS = ldapsearch ldapmodify ldapdelete ldapmodrdn \
	ldappasswd ldapwhoami ldapcompare ldapexop ldapurl


ldapsearch:	ldsversion.o
	$(LTLINK) -o $@ ldapsearch.o common.o ldsversion.o $(LIBS)

ldapmodify:	ldmversion.o
	$(LTLINK) -o $@ ldapmodify.o common.o ldmversion.o $(LIBS)

ldapdelete:	lddversion.o
	$(LTLINK) -o $@ ldapdelete.o common.o lddversion.o $(LIBS)

ldapmodrdn:	ldrversion.o
	$(LTLINK) -o $@ ldapmodrdn.o common.o ldrversion.o $(LIBS)

ldappasswd:	ldpversion.o
	$(LTLINK) -o $@ ldappasswd.o common.o ldpversion.o $(LIBS)

ldapwhoami:	ldwversion.o
	$(LTLINK) -o $@ ldapwhoami.o common.o ldwversion.o $(LIBS)

ldapcompare: ldcversion.o
	$(LTLINK) -o $@ ldapcompare.o common.o ldcversion.o $(LIBS)

ldapexop: ldeversion.o
	$(LTLINK) -o $@ ldapexop.o common.o ldeversion.o $(LIBS)

ldapurl: lduversion.o
	$(LTLINK) -o $@ ldapurl.o lduversion.o $(LIBS)

ldsversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapsearch > $@

ldsversion.o: ldapsearch.o common.o $(XLIBS)

ldmversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapmodify > $@

ldmversion.o: ldapmodify.o common.o $(XLIBS)

lddversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapdelete > $@

lddversion.o: ldapdelete.o common.o $(XLIBS)

ldpversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldappasswd > $@

ldpversion.o: ldappasswd.o common.o $(XLIBS)

ldrversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapmodrdn > $@

ldrversion.o: ldapmodrdn.o common.o $(XLIBS)

ldwversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapwhoami > $@

ldwversion.o: ldapwhoami.o common.o $(XLIBS)

ldcversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapcompare > $@

ldcversion.o: ldapcompare.o common.o $(XLIBS)

ldeversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapexop > $@

ldeversion.o: ldapexop.o common.o $(XLIBS)

lduversion.c: Makefile
	@-$(RM) $@
	$(MKVERSION) $(MKVOPTS) ldapurl > $@

lduversion.o: ldapurl.o $(XLIBS)

install-local:	FORCE
	-$(MKDIR) $(DESTDIR)$(bindir)
	@(								\
	    for prg in $(PROGRAMS); do					\
		$(LTINSTALL) $(INSTALLFLAGS) $(STRIP) -m 755 $$prg$(EXEEXT)	\
		    $(DESTDIR)$(bindir);				\
	    done							\
	)
	$(RM) $(DESTDIR)$(bindir)/ldapadd$(EXEEXT)
	$(LN_S) $(DESTDIR)$(bindir)/ldapmodify$(EXEEXT) $(DESTDIR)$(bindir)/ldapadd$(EXEEXT)

# $OpenLDAP: pkg/ldap/build/rules.mk,v 1.15.2.5 2010/04/13 20:22:22 kurt Exp $
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
# Makefile Template for Programs
#

all-common: $(PROGRAMS) FORCE

clean-common: 	FORCE
	$(RM) $(PROGRAMS) $(XPROGRAMS) $(XSRCS) *.o *.lo a.out core *.core \
		    .libs/* *.exe

depend-common: FORCE
	$(MKDEP) $(DEFS) $(DEFINES) $(SRCS)

lint: FORCE
	$(LINT) $(DEFS) $(DEFINES) $(SRCS)

lint5: FORCE
	$(5LINT) $(DEFS) $(DEFINES) $(SRCS)

Makefile: $(top_srcdir)/build/rules.mk

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.


#
# files: -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include ldapsearch.c ldapmodify.c ldapdelete.c ldapmodrdn.c ldappasswd.c ldapwhoami.c ldapcompare.c ldapexop.c ldapurl.c common.c
# command: x86_64-QNAP-linux-gnu-gcc -M -I.  -I../../include -I../../include -D_QNAP_ -I../../include -I../../../include -I/root/daily_build/64_01/4.4.1/LinkFS/include -I/root/daily_build/64_01/4.4.1/LinkFS/usr/include ldapsearch.c ldapmodify.c ldapdelete.c ldapmodrdn.c ldappasswd.c ldapwhoami.c ldapcompare.c ldapexop.c ldapurl.c common.c
#

ldapsearch.o: ldapsearch.c
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapsearch.o: ../../include/portable.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapsearch.o: ../../include/ac/fdset.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapsearch.o: ../../include/ldap_cdefs.h
ldapsearch.o: ../../include/ldap_features.h
ldapsearch.o: ../../include/ac/assert.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapsearch.o: ../../include/ac/localize.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapsearch.o: ../../include/ac/stdlib.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapsearch.o: ../../include/ac/ctype.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapsearch.o: ../../include/ac/string.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapsearch.o: ../../include/ac/unistd.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapsearch.o: ../../include/lutil_lockf.h
ldapsearch.o: ../../include/ac/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/stat.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stat.h
ldapsearch.o: ../../include/ac/signal.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/fcntl.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl.h
ldapsearch.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl-linux.h
ldapsearch.o: ../../include/ldap.h
ldapsearch.o: ../../include/lber.h
ldapsearch.o: ../../include/lber_types.h
ldapsearch.o: ../../include/ldap_cdefs.h
ldapsearch.o: ../../include/ldap_features.h
ldapsearch.o: ../../include/ldif.h
ldapsearch.o: ../../include/lutil.h
ldapsearch.o: ../../include/lutil_ldap.h
ldapsearch.o: ../../include/ldap_defaults.h
ldapsearch.o: ../../include/ldap_config.h
ldapsearch.o: ../../include/ldap_log.h
ldapsearch.o: ../../include/ldap_pvt.h
ldapsearch.o: ../../include/lber_pvt.h
ldapsearch.o: ../../include/ldap_pvt_uc.h
ldapsearch.o: ../../include/ac/bytes.h
ldapsearch.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
ldapsearch.o: common.h
ldapmodify.o: ldapmodify.c
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapmodify.o: ../../include/portable.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapmodify.o: ../../include/ac/fdset.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapmodify.o: ../../include/ldap_cdefs.h
ldapmodify.o: ../../include/ldap_features.h
ldapmodify.o: ../../include/ac/assert.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapmodify.o: ../../include/ac/localize.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapmodify.o: ../../include/ac/stdlib.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapmodify.o: ../../include/ac/ctype.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapmodify.o: ../../include/ac/string.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapmodify.o: ../../include/ac/unistd.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapmodify.o: ../../include/lutil_lockf.h
ldapmodify.o: ../../include/ac/socket.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapmodify.o: ../../include/ac/time.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/stat.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stat.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/file.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/fcntl.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl.h
ldapmodify.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl-linux.h
ldapmodify.o: ../../include/ldap.h
ldapmodify.o: ../../include/lber.h
ldapmodify.o: ../../include/lber_types.h
ldapmodify.o: ../../include/ldap_cdefs.h
ldapmodify.o: ../../include/ldap_features.h
ldapmodify.o: ../../include/lutil.h
ldapmodify.o: ../../include/lutil_ldap.h
ldapmodify.o: ../../include/ldif.h
ldapmodify.o: ../../include/ldap_defaults.h
ldapmodify.o: ../../include/ldap_config.h
ldapmodify.o: ../../include/ldap_log.h
ldapmodify.o: ../../include/ldap_pvt.h
ldapmodify.o: ../../include/lber_pvt.h
ldapmodify.o: ../../include/ldap_pvt_uc.h
ldapmodify.o: ../../include/ac/bytes.h
ldapmodify.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
ldapmodify.o: common.h
ldapdelete.o: ldapdelete.c
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapdelete.o: ../../include/portable.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapdelete.o: ../../include/ac/fdset.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapdelete.o: ../../include/ldap_cdefs.h
ldapdelete.o: ../../include/ldap_features.h
ldapdelete.o: ../../include/ac/assert.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapdelete.o: ../../include/ac/localize.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapdelete.o: ../../include/ac/stdlib.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapdelete.o: ../../include/ac/ctype.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapdelete.o: ../../include/ac/string.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapdelete.o: ../../include/ac/unistd.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapdelete.o: ../../include/lutil_lockf.h
ldapdelete.o: ../../include/ac/socket.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapdelete.o: ../../include/ac/time.h
ldapdelete.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapdelete.o: ../../include/ldap.h
ldapdelete.o: ../../include/lber.h
ldapdelete.o: ../../include/lber_types.h
ldapdelete.o: ../../include/ldap_cdefs.h
ldapdelete.o: ../../include/ldap_features.h
ldapdelete.o: ../../include/lutil.h
ldapdelete.o: ../../include/lutil_ldap.h
ldapdelete.o: ../../include/ldap_defaults.h
ldapdelete.o: ../../include/ldap_config.h
ldapdelete.o: common.h
ldapmodrdn.o: ldapmodrdn.c
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapmodrdn.o: ../../include/portable.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapmodrdn.o: ../../include/ac/fdset.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapmodrdn.o: ../../include/ldap_cdefs.h
ldapmodrdn.o: ../../include/ldap_features.h
ldapmodrdn.o: ../../include/ac/assert.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapmodrdn.o: ../../include/ac/localize.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapmodrdn.o: ../../include/ac/stdlib.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapmodrdn.o: ../../include/ac/ctype.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapmodrdn.o: ../../include/ac/string.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapmodrdn.o: ../../include/ac/unistd.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapmodrdn.o: ../../include/lutil_lockf.h
ldapmodrdn.o: ../../include/ac/socket.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapmodrdn.o: ../../include/ac/time.h
ldapmodrdn.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapmodrdn.o: ../../include/ldap.h
ldapmodrdn.o: ../../include/lber.h
ldapmodrdn.o: ../../include/lber_types.h
ldapmodrdn.o: ../../include/ldap_cdefs.h
ldapmodrdn.o: ../../include/ldap_features.h
ldapmodrdn.o: ../../include/lutil.h
ldapmodrdn.o: ../../include/lutil_ldap.h
ldapmodrdn.o: ../../include/ldap_defaults.h
ldapmodrdn.o: ../../include/ldap_config.h
ldapmodrdn.o: common.h
ldappasswd.o: ldappasswd.c
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldappasswd.o: ../../include/portable.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldappasswd.o: ../../include/ac/fdset.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldappasswd.o: ../../include/ldap_cdefs.h
ldappasswd.o: ../../include/ldap_features.h
ldappasswd.o: ../../include/ac/assert.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldappasswd.o: ../../include/ac/localize.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldappasswd.o: ../../include/ac/stdlib.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldappasswd.o: ../../include/ac/ctype.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldappasswd.o: ../../include/ac/socket.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldappasswd.o: ../../include/ac/string.h
ldappasswd.o: ../../include/ac/time.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldappasswd.o: ../../include/ac/unistd.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldappasswd.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldappasswd.o: ../../include/lutil_lockf.h
ldappasswd.o: ../../include/ldap.h
ldappasswd.o: ../../include/lber.h
ldappasswd.o: ../../include/lber_types.h
ldappasswd.o: ../../include/ldap_cdefs.h
ldappasswd.o: ../../include/ldap_features.h
ldappasswd.o: ../../include/lutil.h
ldappasswd.o: ../../include/lutil_ldap.h
ldappasswd.o: ../../include/ldap_defaults.h
ldappasswd.o: ../../include/ldap_config.h
ldappasswd.o: common.h
ldapwhoami.o: ldapwhoami.c
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapwhoami.o: ../../include/portable.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapwhoami.o: ../../include/ac/fdset.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapwhoami.o: ../../include/ldap_cdefs.h
ldapwhoami.o: ../../include/ldap_features.h
ldapwhoami.o: ../../include/ac/assert.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapwhoami.o: ../../include/ac/localize.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapwhoami.o: ../../include/ac/stdlib.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapwhoami.o: ../../include/ac/ctype.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapwhoami.o: ../../include/ac/socket.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapwhoami.o: ../../include/ac/string.h
ldapwhoami.o: ../../include/ac/time.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapwhoami.o: ../../include/ac/unistd.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapwhoami.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapwhoami.o: ../../include/lutil_lockf.h
ldapwhoami.o: ../../include/ldap.h
ldapwhoami.o: ../../include/lber.h
ldapwhoami.o: ../../include/lber_types.h
ldapwhoami.o: ../../include/ldap_cdefs.h
ldapwhoami.o: ../../include/ldap_features.h
ldapwhoami.o: ../../include/lutil.h
ldapwhoami.o: ../../include/lutil_ldap.h
ldapwhoami.o: ../../include/ldap_defaults.h
ldapwhoami.o: ../../include/ldap_config.h
ldapwhoami.o: common.h
ldapcompare.o: ldapcompare.c
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapcompare.o: ../../include/portable.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapcompare.o: ../../include/ac/fdset.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapcompare.o: ../../include/ldap_cdefs.h
ldapcompare.o: ../../include/ldap_features.h
ldapcompare.o: ../../include/ac/assert.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapcompare.o: ../../include/ac/localize.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapcompare.o: ../../include/ac/stdlib.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapcompare.o: ../../include/ac/ctype.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapcompare.o: ../../include/ac/string.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapcompare.o: ../../include/ac/unistd.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapcompare.o: ../../include/lutil_lockf.h
ldapcompare.o: ../../include/ac/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
ldapcompare.o: ../../include/ac/socket.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapcompare.o: ../../include/ac/time.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/stat.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stat.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/fcntl.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl.h
ldapcompare.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/fcntl-linux.h
ldapcompare.o: ../../include/ldap.h
ldapcompare.o: ../../include/lber.h
ldapcompare.o: ../../include/lber_types.h
ldapcompare.o: ../../include/ldap_cdefs.h
ldapcompare.o: ../../include/ldap_features.h
ldapcompare.o: ../../include/lutil.h
ldapcompare.o: ../../include/lutil_ldap.h
ldapcompare.o: ../../include/ldap_defaults.h
ldapcompare.o: ../../include/ldap_config.h
ldapcompare.o: common.h
ldapexop.o: ldapexop.c
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapexop.o: ../../include/portable.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapexop.o: ../../include/ac/fdset.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapexop.o: ../../include/ldap_cdefs.h
ldapexop.o: ../../include/ldap_features.h
ldapexop.o: ../../include/ac/assert.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapexop.o: ../../include/ac/localize.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapexop.o: ../../include/ac/stdlib.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapexop.o: ../../include/ac/ctype.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapexop.o: ../../include/ac/socket.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
ldapexop.o: ../../include/ac/string.h
ldapexop.o: ../../include/ac/time.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
ldapexop.o: ../../include/ac/unistd.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapexop.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapexop.o: ../../include/lutil_lockf.h
ldapexop.o: ../../include/ldap.h
ldapexop.o: ../../include/lber.h
ldapexop.o: ../../include/lber_types.h
ldapexop.o: ../../include/ldap_cdefs.h
ldapexop.o: ../../include/ldap_features.h
ldapexop.o: ../../include/ldif.h
ldapexop.o: ../../include/lutil.h
ldapexop.o: ../../include/lutil_ldap.h
ldapexop.o: ../../include/ldap_defaults.h
ldapexop.o: ../../include/ldap_config.h
ldapexop.o: common.h
ldapurl.o: ldapurl.c
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
ldapurl.o: ../../include/portable.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
ldapurl.o: ../../include/ac/fdset.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
ldapurl.o: ../../include/ldap_cdefs.h
ldapurl.o: ../../include/ldap_features.h
ldapurl.o: ../../include/ac/assert.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
ldapurl.o: ../../include/ac/localize.h
ldapurl.o: ../../include/ac/stdlib.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
ldapurl.o: ../../include/ac/unistd.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
ldapurl.o: ../../include/lutil_lockf.h
ldapurl.o: ../../include/ldap.h
ldapurl.o: ../../include/lber.h
ldapurl.o: ../../include/lber_types.h
ldapurl.o: ../../include/ldap_cdefs.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
ldapurl.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
ldapurl.o: ../../include/ldap_features.h
ldapurl.o: ../../include/ldap_pvt.h
ldapurl.o: ../../include/lber_pvt.h
ldapurl.o: ../../include/ldap_pvt_uc.h
ldapurl.o: ../../include/ac/bytes.h
ldapurl.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
ldapurl.o: ../../include/lutil.h
common.o: common.c
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdc-predef.h
common.o: ../../include/portable.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stddef.h
common.o: ../../include/ac/fdset.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/types.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/features.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/cdefs.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wordsize.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/gnu/stubs-64.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/typesizes.h
common.o: ../../include/ldap_cdefs.h
common.o: ../../include/ldap_features.h
common.o: ../../include/ac/assert.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/assert.h
common.o: ../../include/ac/localize.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdio.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/libio.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/_G_config.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/wchar.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdarg.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdio_lim.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sys_errlist.h
common.o: ../../include/ac/stdlib.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdlib.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitflags.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/waitstatus.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/endian.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/endian.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/byteswap-16.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/types.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/time.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/select.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/select.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigset.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/time.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/sysmacros.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/pthreadtypes.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/alloca.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/stdlib-float.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/limits.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include-fixed/syslimits.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/limits.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix1_lim.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/local_lim.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/limits.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix2_lim.h
common.o: ../../include/ac/signal.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/signal.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/signum.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/siginfo.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigaction.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigcontext.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigstack.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/ucontext.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sigthread.h
common.o: ../../include/ac/string.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/string.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/xlocale.h
common.o: ../../include/ac/ctype.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/ctype.h
common.o: ../../include/ac/unistd.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/unistd.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/posix_opt.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/environments.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/confname.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/getopt.h
common.o: ../../include/lutil_lockf.h
common.o: ../../include/ac/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/errno-base.h
common.o: ../../include/ac/time.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/time.h
common.o: ../../include/ac/socket.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/poll.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/poll.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/poll.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/socket.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/uio.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/uio.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/socket_type.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/sockaddr.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/socket.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/socket.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/sockios.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/sockios.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/un.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/in.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/cross-tools/lib/gcc/x86_64-QNAP-linux-gnu/4.9.2/include/stdint.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/stdint.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/wchar.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/in.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netinet/tcp.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/inet.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/param.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/param.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/linux/param.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm/param.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/asm-generic/param.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/sys/bitypes.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/arpa/nameser_compat.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/netdb.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/rpc/netdb.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/bits/netdb.h
common.o: /root/CT/x86_64-QNAP-linux-gnu/fs/include/resolv.h
common.o: ../../include/ldap.h
common.o: ../../include/lber.h
common.o: ../../include/lber_types.h
common.o: ../../include/ldap_cdefs.h
common.o: ../../include/ldap_features.h
common.o: ../../include/ldif.h
common.o: ../../include/lutil.h
common.o: ../../include/lutil_ldap.h
common.o: ../../include/ldap_defaults.h
common.o: ../../include/ldap_config.h
common.o: ../../include/ldap_pvt.h
common.o: ../../include/lber_pvt.h
common.o: ../../include/ldap_pvt_uc.h
common.o: ../../include/ac/bytes.h
common.o: ../../include/../libraries/liblunicode/ucdata/ucdata.h
common.o: common.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
