## $Id: Makefile.am,v 1.1.1.1 2014/03/25 08:01:38 aquavitwu Exp $

CLEANFILES = *~

EXTRA_DIST = cgicc.m4

cgicc_libdir=@cgicc_libdir@
cgicc_includedir=@cgicc_includedir@

bin_PROGRAMS = example.cgi

INCLUDES = -I$(cgicc_includedir)

example_cgi_SOURCES = example.cpp
example_cgi_LDADD = -L$(cgicc_libdir) -lcgicc
