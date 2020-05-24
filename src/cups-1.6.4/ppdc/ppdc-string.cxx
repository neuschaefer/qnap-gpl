//
// "$Id: ppdc-string.cxx,v 1.1.1.1 2013/10/15 06:58:46 aixchou Exp $"
//
//   Shared string class for the CUPS PPD Compiler.
//
//   Copyright 2007-2009 by Apple Inc.
//   Copyright 2002-2005 by Easy Software Products.
//
//   These coded instructions, statements, and computer programs are the
//   property of Apple Inc. and are protected by Federal copyright
//   law.  Distribution and use rights are outlined in the file "LICENSE.txt"
//   which should have been included with this file.  If this file is
//   file is missing or damaged, see the license at "http://www.cups.org/".
//
// Contents:
//
//   ppdcString::ppdcString()  - Create a shared string.
//   ppdcString::~ppdcString() - Destroy a shared string.
//

//
// Include necessary headers...
//

#include "ppdc-private.h"


//
// 'ppdcString::ppdcString()' - Create a shared string.
//

ppdcString::ppdcString(const char *v)	// I - String
  : ppdcShared()
{
  PPDC_NEWVAL(v);

  if (v)
  {
    value = new char[strlen(v) + 1];
    strcpy(value, v);
  }
  else
    value = 0;
}


//
// 'ppdcString::~ppdcString()' - Destroy a shared string.
//

ppdcString::~ppdcString()
{
  PPDC_DELETEVAL(value);

  if (value)
    delete[] value;
}


//
// End of "$Id: ppdc-string.cxx,v 1.1.1.1 2013/10/15 06:58:46 aixchou Exp $".
//