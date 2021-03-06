/* user.c - supporting elements of user handling functions for upsd

   Copyright (C) 2001  Russell Kroll <rkroll@exploits.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

void user_load(void);

#ifndef	HAVE_IPV6
int user_checkinstcmd(const struct sockaddr_in *addr,
#else
int user_checkinstcmd(const struct sockaddr_storage *addr,
#endif
	const char *un, const char *pw, const char *cmd);

#ifndef	HAVE_IPV6
int user_checkaction(const struct sockaddr_in *addr,
#else
int user_checkaction(const struct sockaddr_storage *addr,
#endif
	const char *un, const char *pw, const char *action);

void user_flush(void);

/* cheat - we don't want the full upsd.h included here */
void check_perms(const char *fn);
