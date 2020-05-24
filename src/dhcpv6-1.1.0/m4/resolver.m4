dnl resolver.m4 - autoconf checks for dn_expand() and dn_comp()
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
dnl Author(s): David Cantrell <dcantrell@redhat.com>

AC_DEFUN([AM_CHECK_RESOLVER],[
AC_SUBST(LIBS)

saved_LIBS="$LIBS"
LIBS="-lresolv"

AC_CHECK_DECL(
    [dn_comp],
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
            [#include <resolv.h>],
            [int i = dn_comp(NULL, NULL, 0, NULL, NULL);]
        )],
        [],
        [AC_MSG_FAILURE([*** Unable to find dn_comp() in libresolv])]
    ),
    [AC_MSG_FAILURE([*** Symbol dn_comp is not declared])],
    [#include <resolv.h>]
)

AC_CHECK_DECL(
    [dn_expand],
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
            [#include <resolv.h>],
            [int i = dn_expand(NULL, NULL, 0, NULL, NULL);]
        )],
        [],
        [AC_MSG_FAILURE([*** Unable to find dn_expand() in libresolv])]
    ),
    [AC_MSG_FAILURE([*** Symbol dn_expand is not declared])],
    [#include <resolv.h>]
)

LIBS="$saved_LIBS -lresolv"
])
