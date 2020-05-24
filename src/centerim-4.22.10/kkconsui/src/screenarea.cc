/*
*
* a class for saving and restoring screen areas
* $Id: screenarea.cc,v 1.1.1.1 2011/04/18 03:47:16 johnsonlee Exp $
*
* Copyright (C) 1999-2001 by Konstantin Klyagin <k@thekonst.net>
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

#include "screenarea.h"

screenarea::screenarea() {
}

screenarea::screenarea(int fx1, int fy1, int fx2, int fy2) {
    save(fx1, fy1, fx2, fy2);
}

screenarea::~screenarea() {
    freebuffer();
}

void screenarea::save() {
    save(0, 0, COLS, LINES);
}

void screenarea::save(int fx1, int fy1, int fx2, int fy2) {
    int i;
    chtype *line;
#ifdef HAVE_NCURSESW
    wchar_t *line2;
#endif

    freebuffer();

    for(i = 0; i <= fy2-fy1; i++) {
	line = new chtype[fx2-fx1+2];
#ifdef HAVE_NCURSESW
	line2 = new wchar_t[fx2-fx1+2];
#endif
	mvinchnstr(fy1+i, fx1, line, fx2-fx1+1);
#ifdef HAVE_NCURSESW
	mvinnwstr(fy1+i, fx1, line2, fx2-fx1+1);
#endif
	buffer.push_back(line);
#ifdef HAVE_NCURSESW
	buffer2.push_back(line2);
#endif
    }

    x1 = fx1;
    y1 = fy1;
    x2 = fx2;
    y2 = fy2;
}

void screenarea::restore() {
    restore(x1, y1, x2, y2);
}

void screenarea::restore(int fx1, int fy1, int fx2, int fy2) {
    vector<chtype *>::iterator i;
#ifdef HAVE_NCURSESW
    vector<wchar_t *>::iterator j;
#endif
    int k = fy1;
    chtype *line;
#ifdef HAVE_NCURSESW
    wchar_t *line2;
#endif
    int l;
    if(!buffer.empty()) {
#ifdef HAVE_NCURSESW
	for(i = buffer.begin(), j = buffer2.begin(); i != buffer.end(); i++, j++) {
#else
	for(i = buffer.begin(); i != buffer.end(); i++) {
#endif
	    line = *i;
#ifdef HAVE_NCURSESW
	    line2 = *j;
	    const chtype *line_ptr = line;
	    const wchar_t *line2_ptr = line2;
	    for(l = 0; l < fx2-fx1+1; l++, line_ptr++, line2_ptr++ )
	    {
		    attrset( (*line_ptr & A_COLOR) | (*line_ptr & A_ATTRIBUTES) );
		    mvaddnwstr(k, fx1+l, line2_ptr, 1); 
	    }
	    k++;
#else
	    mvaddchnstr(k++, fx1, line, fx2-fx1+1);
#endif
	    
	}

	refresh();
    }

    freebuffer();
}

void screenarea::freebuffer() {
    while(!buffer.empty()) {
	delete[] buffer.front();
	buffer.erase(buffer.begin());
    }
#ifdef HAVE_NCURSESW
    while(!buffer2.empty()) {
	delete[] buffer2.front();
	buffer2.erase(buffer2.begin());
    }
#endif
}

bool screenarea::empty() {
    return buffer.empty();
}
