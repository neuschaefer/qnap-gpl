/*
*
* centerim main() function
* $Id: centermain.cc,v 1.1.1.1 2011/04/18 03:47:16 johnsonlee Exp $
*
* Copyright (C) 2001 by Konstantin Klyagin <k@thekonst.net>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or (at
* your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
* USA
*
*/

#include <sys/types.h>
#include <sys/stat.h>

#include "centerim.h"
#include "icqface.h"
#include "icqconf.h"
#include "icqcontacts.h"
#include "icqmlist.h"
#include "icqgroups.h"

#ifdef HAVE_LIBOTR
#include "imotr.h"
#endif

centerim cicq;
icqconf* conf=icqconf::instance();
icqcontacts clist;
icqface face;
icqlist lst;
icqgroups groups;

#ifdef ENABLE_NLS

#include <locale.h>
#include <libintl.h>

#endif

int main(int argc, char **argv) {
    char savedir[1024];

    getcwd(savedir, 1024);

    try {
        srand((unsigned int) time(NULL));

#ifdef ENABLE_NLS
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    conf->commandline(argc, argv);
#ifdef HAVE_LIBOTR
    otr.init();
#endif

    cicq.exec();

    } catch(exception e) {
        cerr << "Caught exception: " << e.what() << endl;
    }

    chdir(savedir);
}
