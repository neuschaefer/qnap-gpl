/*
 * "$Id: cgi-private.h,v 1.1.1.1 2013/10/15 06:58:46 aixchou Exp $"
 *
 *   Private CGI definitions for CUPS.
 *
 *   Copyright 2007-2011 by Apple Inc.
 *   Copyright 1997-2006 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Apple Inc. and are protected by Federal copyright
 *   law.  Distribution and use rights are outlined in the file "LICENSE.txt"
 *   which should have been included with this file.  If this file is
 *   file is missing or damaged, see the license at "http://www.cups.org/".
 */

/*
 * Include necessary headers...
 */

#include "cgi.h"
#include <cups/debug-private.h>
#include <cups/language-private.h>
#include <cups/string-private.h>
#include <cups/ipp-private.h>	/* TODO: Update so we don't need this */


/*
 * Limits...
 */

#define CUPS_PAGE_MAX	100		/* Maximum items per page */


/*
 * End of "$Id: cgi-private.h,v 1.1.1.1 2013/10/15 06:58:46 aixchou Exp $".
 */