/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * Function prototypes
 */

struct queue {
	        time_t  q_time;                 /* modification time */
	        char    q_name[256];    /* control file name */
};


void fatal(const char *msg, ...);
int getport(char *rhost, int rport);
int getline(FILE *cfp);
int getq(struct queue *(*namelist[]));
char *checkremote(void);
int checkfromremote(char *fromhost);
void displayq(int format);
void warn(void);
void header(void);
void inform(char *cf);
int inlist(char *name, char *file);
void show(register char *nfile, register char *file, int copies);
void blankfill(register int n);
void dump(char *nfile, char *file, int copies);
void ldump(char *nfile, char *file, int copies);
void prank(int n);
int getprent(register char *bp);
void endprent(void);
int pgetent(char *bp, char *name);
int pnchktc(void);
int pnamatch(char *np);
int pgetnum(char *id);
unsigned long pgetusgn(char *id);
int pgetflag(char *id);
char *pgetstr(char *id, char **area);
void rmjob(void);
int lockchk(char *s);
int chk(char *file);
int isowner(char *owner, char *file, char *h);
int dounlink(char *file, char *host, char *user);
int startdaemon(char *printer);
