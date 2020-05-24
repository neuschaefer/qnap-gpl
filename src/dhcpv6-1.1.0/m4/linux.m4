dnl linux.m4 - autoconf checks for Linux-specific header files
dnl
dnl Copyright (C) 2008  Red Hat, Inc.
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
dnl Author(s): David Cantrell <dcantrell@redhat.com>

AC_DEFUN([AM_CHECK_LINUX_HEADERS],[
    if test x"`uname -s`" = x"Linux" ; then
        AC_CHECK_HEADERS([linux/ipv6.h linux/sockios.h \
                          linux/netlink.h linux/rtnetlink.h],
                         [],
                         [AC_MSG_FAILURE([*** Header file $ac_header not found.])],
                         [[#ifdef HAVE_SYS_TYPES_H
                           # include <sys/types.h>
                           #endif
                           #ifdef HAVE_SYS_SOCKET_H
                           # include <sys/socket.h>
                           #endif
                         ]])
    fi
])
