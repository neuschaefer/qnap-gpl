dnl darwin.m4 - autoconf checks for Darwin-specific header files
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

AC_DEFUN([AM_CHECK_DARWIN_HEADERS],[
    if test x"`uname -s`" = x"Darwin" ; then
        AC_CHECK_HEADERS([net/if_var.h netinet6/in6_var.h],
                         [],
                         [AC_MSG_FAILURE([*** Header file $ac_header not found.])],
                         [[#ifdef HAVE_NETINET_IN_H
                           # include <netinet/in.h>
                           #endif
                           #ifdef HAVE_NET_IF_VAR_H
                           # include <net/if_var.h>
                           #endif
                         ]])
    fi
])
