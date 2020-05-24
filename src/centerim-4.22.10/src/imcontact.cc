/*
*
* centerim IM contact basic info class
* $Id: imcontact.cc,v 1.1.1.1 2011/04/18 03:47:16 johnsonlee Exp $
*
* Copyright (C) 2002 by Konstantin Klyagin <k@thekonst.net>
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

#include "imcontact.h"
#include "icqcontact.h"
#include "icqconf.h"

imcontact contactroot(0, icq);

imcontact::imcontact() {
}

imcontact::imcontact(unsigned long auin, protocolname apname) {
    uin = auin;
    pname = apname;
}

imcontact::imcontact(const string &anick, protocolname apname) {
    nickname = anick;
    pname = apname;
    uin = 0;
}

imcontact::imcontact(const icqcontact *c) {
    if(c) *this = c->getdesc();
}

bool imcontact::operator == (const imcontact &ainfo) const {
    bool r = (ainfo.uin == uin) && (ainfo.pname == pname);

    if(r)
    switch(pname) {
	case aim:
	    r = r & (up(stripspaces(ainfo.nickname)) == up(stripspaces(nickname)));
	    break;
	
	case irc:
	case yahoo:
	case jabber:
	    r = r & (up(ainfo.nickname) == up(nickname));
	    break;

	default:
	    r = r & (ainfo.nickname == nickname);
	    break;
    }

    return r;
}

bool imcontact::operator != (const imcontact &ainfo) const {
    return !(*this == ainfo);
}

bool imcontact::operator < (const imcontact &ainfo) const {
    return false; /*TODO: fix this to something meaningfull! */
}

bool imcontact::operator == (protocolname apname) const {
    return apname == pname;
}

bool imcontact::operator != (protocolname apname) const {
    return !(*this == apname);
}

bool imcontact::operator < (protocolname apname) const {
    return (*this < apname);
}

bool imcontact::empty() const {
    return !uin && nickname.empty();
}

string imcontact::totext() const {
    string r;

    if(!uin && nickname.empty()) r = "...";
    else if(uin) r = "[" + conf->getprotocolname(pname) + "] " + i2str(uin);
    else r = "[" + conf->getprotocolname(pname) + "] " + nickname;

    return r;
}

string imstatus2str(imstatus st) {
    static map<imstatus, string> mst;

    if(mst.empty()) {
	mst[offline] = _("Offline");
	mst[available] = _("Online");
	mst[invisible] = _("Invisible");
	mst[freeforchat] = _("Free for chat");
	mst[dontdisturb] = _("Do not disturb");
	mst[occupied] = _("Occupied");
	mst[notavail] = _("Not available");
	mst[away] = _("Away");
	mst[outforlunch] = _("Out for Lunch");
	mst[imstatus_size] = "";
    }

    return mst[st];
}
