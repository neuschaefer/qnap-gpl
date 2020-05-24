#include <stdio.h>
#include "smbpwd.h"

/* simple test program */

main(int argc, char *argv[])
{
   FILE *fout;
   struct smb_passwd *fred;
   unsigned char nt_p16[16];
   unsigned char p16[16];
   
   int i;
   
   if (geteuid()!=0) printf("May need to be logged on as root to read smbpasswd file\n");
   
   fred = NULL;
   
   fout = fopen("/tmp/smbtest","w");
   
   if (fout==NULL) {
      printf ("Cannot write to file /tmp/smbtest\n");
      perror("reason");
      exit (1);
   }
   
   printf("Setting smbfilepath to /etc/smbpasswd\n");
   setsmbfilepath("/etc/smbpasswd");
   
   while ( (fred = getsmbpwent()) !=NULL){
      printf("smb name, %s, smb_uid, %d, smb_gecos, %s\n",fred->smb_name, fred->smb_userid, fred->smb_gecos);
      
      putsmbpwent(fred, fout);
      
      
   }
   fclose(fout);

   smbcrypt("FredFred", nt_p16, p16 );   
   
   printf ("\nplaintext FredFred\nNT encrypted ");
    for(i=0;i<16;i++) printf( "%2.2X" , nt_p16[i]);
   printf ("\nSMB encrypted ");
    for(i=0;i<16;i++) printf( "%2.2X" , p16[i]);
   printf("\n\n");
   
}
     
