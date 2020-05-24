/*

  Based on testprogram coming with libsmbpw.
  Copyright (C) Andrew Tridgell 1998

  New code Copyright SLU (Swedens Agricultural University)
  author: Jens Låås.

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */
#include <stdio.h>
#include "smbpwd.h"


/*
 * reads one line from fd
 * keeps only l number of bytes of the line.
 * always reads until end of line (or file)
 */
int readln(int fd, char *b, int l)
{
  int got = 0, i;
  
  while( (i = read(fd, &b[got], 1)) > 0)
    {
      if(b[got] == '\n')
	{
	  b[got] = 0;
	  return 1;
	}
      
      if(got < (l-1))
	got += i;
    }
  
  b[got] = 0;
  return 0;
}

main(int argc, char *argv[])
{
   unsigned char nt_p16[16];
   unsigned char p16[16];
   char *p, buf[256];
   int i, col = 0;
   int quiet = 0;
   int lm = 1; /* default print lanmanager pw first */
   
   if(argc >1)
     {
       p = argv[1];
       while(*p)
	 {
	   if(*p == 'l')
	     lm = 1;
	   if(*p == 'n')
	     lm = 0;	   
	   if(*p == 'q')
	     quiet = 1;
	   if(*p == ':')
	     col = 1;
	   if((*p == '-')||(*p == 'h'))
	     {
	       printf("Usage: %s [ln:qh]\n"
		      " l = lanmgr pw first\n"
		      " n = nt pw first\n"
		      " : = use : as delimiter\n"
		      " q = quiet, dont prompt\n"
		      " h = this help\n",
		      argv[0]);
	       exit(0);
	     }
	   p++;
	 }
     }
   
   if(!quiet)
     {
       fprintf(stdout, "Password: ");
       fflush(stdout);
     }
   
   readln(0, buf, 255);
   smbcrypt(buf, nt_p16, p16 );   

   if(lm)
     {
       for(i=0;i<16;i++) printf( "%2.2X" , p16[i]);
       if(col)
	 printf (":");
       else
	 printf (" ");
       for(i=0;i<16;i++) printf( "%2.2X" , nt_p16[i]);
     }
   else
     {
       for(i=0;i<16;i++) printf( "%2.2X" , nt_p16[i]);
       if(col)
	 printf (":");
       else
	 printf (" ");
       for(i=0;i<16;i++) printf( "%2.2X" , p16[i]);
     }

   printf ("\n");
   fflush(stdout);
}
     
