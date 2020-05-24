dnl configure.ac for dhcpv6
dnl
dnl Copyright (C) 2007, 2008  Red Hat, Inc.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation; either version 2.1 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.
dnl
dnl Author: David Cantrell <dcantrell@redhat.com>

AC_PREREQ(2.61)
AC_INIT([dhcpv6], [1.1.0], [dcantrell@redhat.com])
AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_SRCDIR([src/dhcp6s.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl Versioning
dnl (from glib's configure.in ...)
dnl Making releases:
dnl     DHCPV6_MICRO_VERSION += 1;
dnl     DHCPV6_INTERFACE_AGE += 1;
dnl     DHCPV6_BINARY_AGE += 1;
dnl if any functions have been added, set DHCPV6_INTERFACE_AGE to 0.
dnl if backwards compatibility has been broken (e.g., functions removed,
dnl function signatures changed),
dnl set DHCPV6_BINARY_AGE and DHCPV6_INTERFACE_AGE to 0.
DHCPV6_MAJOR_VERSION=1
DHCPV6_MINOR_VERSION=1
DHCPV6_MICRO_VERSION=0
DHCPV6_INTERFACE_AGE=0
DHCPV6_BINARY_AGE=0
DHCPV6_VERSION_SUFFIX=
DHCPV6_VERSION=$DHCPV6_MAJOR_VERSION.$DHCPV6_MINOR_VERSION.$DHCPV6_MICRO_VERSION$DHCPV6_VERSION_SUFFIX

LT_RELEASE=$DHCPV6_MAJOR_VERSION.$DHCPV6_MINOR_VERSION
LT_CURRENT=`expr $DHCPV6_MICRO_VERSION - $DHCPV6_INTERFACE_AGE`
LT_REVISION=$DHCPV6_INTERFACE_AGE
AC_SUBST(DHCPV6_VERSION)
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

# Ensure that the DHCPV6_*-specified version is the same as the literal
# in the AC_INIT line, above.
test $PACKAGE_VERSION = $DHCPV6_VERSION ||
  AC_MSG_ERROR(configure.ac: version mismatch: $PACKAGE_VERSION != $DHCPV6_VERSION)

AC_DEFINE_UNQUOTED([BUILD_DATE], ["`date +%m%d%Y`"], [Date of dhcpv6 build])

AC_PROG_CPP
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL

AC_HEADER_RESOLV
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

# These headers may be available.  If so, we need them defined for later header tests.
AC_CHECK_HEADERS([sys/types.h sys/socket.h], [],
                 [AC_MSG_FAILURE([*** Header file $ac_header not found.])],
                 [])

# Check for headers we must have on the system
AC_CHECK_HEADERS([arpa/inet.h err.h errno.h getopt.h fcntl.h ifaddrs.h \
                  libgen.h net/if.h net/if_arp.h netdb.h netinet/in.h \
                  sys/ioctl.h sys/timeb.h syslog.h],
                 [],
                 [AC_MSG_FAILURE([*** Header file $ac_header not found.])],
                 [[#ifdef HAVE_SYS_TYPES_H
                   # include <sys/types.h>
                   #endif
                   #ifdef HAVE_SYS_SOCKET_H
                   # include <sys/socket.h>
                   #endif
                 ]])

# Check for headers that are nice, but we don't absolutely have to have
AC_CHECK_HEADERS([sys/queue.h linux/ipv6.h],
                 [], [],
                 [[#ifdef HAVE_SYS_TYPES_H
                   # include <sys/types.h>
                   #endif
                   #ifdef HAVE_SYS_SOCKET_H
                   # include <sys/socket.h>
                   #endif
                 ]])

AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_CHECK_MEMBERS([struct in6_ifreq.ifr6_addr],,,
    [AC_INCLUDES_DEFAULT()
     [#ifdef HAVE_NETINET_IN_H
      #include <netinet/in.h>
      #endif]])

# check for required library functions
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([bzero ftime gettimeofday memset select socket strchr strdup \
                strerror strrchr strstr strtol strtoul],
               [],
               [AC_MSG_FAILURE([*** Required function $ac_func not found.])])

# We need libnl -- NOTE: this is a Linux-only library, so we'll need something
# else on other platforms.  Will have to create platform-specific tests later.
PKG_CHECK_MODULES(LIBNL, libnl-1 >= 1.1)
AC_SUBST(LIBNL_CFLAGS)
AC_SUBST(LIBNL_LIBS)

# find dn_comp() and dn_expand()
AM_CHECK_RESOLVER

# Check for headers that are platform dependent
AM_CHECK_LINUX_HEADERS
AM_CHECK_DARWIN_HEADERS
AM_PROG_CC_C_O

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile man/Makefile \
                 etc/Makefile etc/sysconfig/Makefile etc/init.d/Makefile])
AC_OUTPUT
