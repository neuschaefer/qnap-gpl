/*
 File        : genpowerd.c
 Project     : genpower-1.0.2
               Observatorio Sismologico del SurOccidente O.S.S.O
 Author(s)   : Jhon H. Caicedo O. <jhcaiced@osso.org.co>
 Description : Header file for genpowerd

 History     :
 1.0.2
 ------
 Feb/05/2001    Modified from version 1.0.1, to add more UPSs (jhcaiced)
 Nov/12/2002    Modified by YC Lin to fix various bugs that caused
                APC-Smart UPS to not work.
 ------------------------------------------------------------------------
*/
/************************************************************************/
/* File Name            : genpowerd.c                                   */
/* Program Name         : genpowerd                   Version: 1.0.1    */
/* Author               : Tom Webster <webster@kaiwan.com>              */
/* Created              : 1994/04/20                                    */
/* Last Modified By     : Tom Webster                 Date: 1995/07/05  */
/*                                                                      */
/* Compiler (created)   : GCC 2.6.3                                     */
/* Compiler (env)       : Linux 1.2.5                                   */
/* ANSI C Compatable    : No                                            */
/* POSIX Compatable     : Yes?                                          */
/*                                                                      */
/* Purpose              : Monitor the serial port connected to a UPS.   */
/*                      : Functions performed depend on the values in   */
/*                      : the genpowerd.h file and command line         */
/*      : configuration.  As a minimum, the power       */
/*                      : status will be monitored.  Low battery        */
/*                      : sensing and physical cable conection checks   */
/*                      : are also supported.                           */
/*                      :                                               */
/*                      : If one of the monitored lines of the serial   */
/*                      : connection changes state, genpowerd will      */
/*                      : notify init.                                  */
/*                      :                                               */
/*                      : If sent the "-k" option (and configured to    */
/*                      : support shutting the inverter down), will     */
/*                      : shut the inverter down (killing power to      */
/*                      : the system while in powerfail mode).  If      */
/*      : not configured to shutdown the inverter, or   */
/*      : if unable to, genpowerd will return a         */
/*      : non-zero exit code.                           */
/*                                                                      */
/* Usage                : genpowerd [-k] <serial device> <ups-type>     */
/*                      : i.e. genpowerd /dev/cua4 tripp-nt             */
/*                      : or:  genpowerd  -k /dev/cua4 tripp-nt         */
/*                                                                      */
/* History/Credits      : genpowerd was previously known as unipowerd.  */
/*                      :                                               */
/*                      : genpowerd is a hack to make the powerd        */
/*                      : deamon easier to configure and to add         */
/*                      : additional features.                          */
/*                      :                                               */
/*                      : powerd was written by Miquel van Smoorenburg  */
/*                      : <miquels@drinkel.nl.mugnet.org> for his       */
/*                      : employer Cistron Electronics and released     */
/*                      : into the public domain.                       */
/*                                                                      */
/* Copyright            : GNU Copyleft                                  */
/************************************************************************/
 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <sys/time.h>
#include <linux/serial.h>
#include <termios.h>

#include "genpowerd.h"
#include "gen_common.h"

// Tiger
#include <Util.h>
#include "naslog.h"
#include "cfg_ups.h"

int g_smart_ups_status = -1;

//#define  FORK_DISABLE

char *str_line(int l) {
  switch(l){  
  case TIOCM_RTS: return "RTS";
  case TIOCM_CTS: return "CTS";
  case TIOCM_DTR: return "DTR";
  case TIOCM_CAR: return "CAR";
  case TIOCM_RNG: return "RNG";
  case TIOCM_DSR: return "DSR";
  case TIOCM_ST:  return "ST ";
  default:        return "---";
  }
}

char *str_neg(int s) { return s?"*":" "; }
/* make a table of all supported UPS types */
void list_ups() {

  fprintf(stderr,"\n    <ups-type>    cablep. kill  t powerok battok cableok\n");
  fprintf(stderr,"    ----------------------------------------------------\n");
  for (pups=&ups[0]; pups->tag; pups++){
      fprintf(stderr,"    %-12s  %s%s    %s%s %2d %s%s    %s%s   %s%s\n",pups->tag,
        str_neg(pups->cablepower.inverted),str_line(pups->cablepower.line),
        str_neg(pups->kill.inverted),str_line(pups->kill.line),
        pups->killtime,
        str_neg(pups->powerok.inverted),str_line(pups->powerok.line),
        str_neg(pups->battok.inverted),str_line(pups->battok.line),
        str_neg(pups->cableok.inverted),str_line(pups->cableok.line)
      );
  }
  fprintf(stderr,"           (*=active low, ---=unused)\n\n");
}


int getlevel(LINE *l,int flags);
void setlevel(int fd,int line,int level);
void powerfail(int);

//  Added by Tiger and YC
int detect_cable(int fd);
int init_smart_ups(int fd);
void get_smart_ups_status (int fd);
void probe_status(int fd);

// global variables
int g_fd = 0;
int cstatus = 0, coldstatus = 1;

void exit_prog (int sig){
  if (sig == SIGUSR1) {
    if (g_fd) close (g_fd);
    exit (1);
  }
}


/* Main program. */
int main(int argc, char **argv) {
  int fd2;
  int flags;
  int pstatus = 0, poldstat = 1;
  int bstatus = 0, boldstat = 1;
  int rts_bit = TIOCM_RTS;
  int dtr_bit = TIOCM_DTR;
  int count = 0;
  int tries = 0;
  int ikill = 0;
  int pid = 0;
  int status = 0;
  int pidfilewritten = 0;
  char pbuf [20];
  char *program_name;
  char killchar = ' ';
  struct termios options;

  program_name = argv[0];
  // Parse the command line
  while ((argc > 1) && (argv[1][0] == '-')) {
    switch (argv[1][1]) {
      case 'k':
        ikill = 1;
        break;
      default:
        fprintf(stderr, "\nUsage: %s [-k] <device> <ups-type>\n", program_name);                                             
        exit(1);
    } // switch (argv[1][1])
    argv++;
    argc--;
  } // while ((argc > 1) && (argv[1][0] == '-'))
        
  // Done with options, make sure that one port is specified
  if (argc != 3) {
    fprintf(stderr, "\nUsage: %s [-k] <device> <ups-type>\n", program_name);
    list_ups();  
     exit(1);
  } // if (argc != 3)

  for (pups=&ups[0]; pups->tag > 0; pups++){
    if (strcmp(pups->tag,argv[2])==0) break;
  } // for (pups=&ups[0]; pups->tag; pups++)
  if (!pups->tag){
    fprintf(stderr, "Error: %s: UPS <%s> unknown\n", program_name,argv[2]);
    exit(1);
  } // if (!pups->tag)

  (void)signal (SIGUSR1, exit_prog);
  // Kill the inverter and close out if inverter kill was selected
  if (ikill) {
    if (pups->killtime) {
      if ((g_fd = open(argv[1], O_RDWR | O_SYNC | O_NOCTTY )) < 0) {
        fprintf(stderr, "%s: %s: %s\n",program_name, argv[1], sys_errlist[errno]);
        exit(1);
      } // if ((fd = open(argv[1], O_RDWR | O_NDELAY)) < 0)

      // Explicitly clear both DTR and RTS as soon as possible 
      ioctl(g_fd, TIOCMBIC, TIOCM_RTS);
      ioctl(g_fd, TIOCMBIC, TIOCM_DTR);

      // clear killpower, apply cablepower to enable monitoring
      setlevel(g_fd,pups->kill.line,!pups->kill.inverted);
      setlevel(g_fd,pups->cablepower.line,!pups->cablepower.inverted);
        
      if (pups->kill.line == TIOCM_ST) {
        // Send BREAK (TX high) for INVERTERTIME seconds to kill the UPS inverter.
        ioctl(g_fd, TCSBRKP, 10*pups->killtime);
      } else {
        // Force high to send the UPS the inverter kill signal.
        setlevel(g_fd,pups->kill.line,pups->kill.inverted);
        sleep(pups->killtime);
      } // if (pups->kill.line == TIOCM_ST)


      ioctl(g_fd, TIOCMGET, &flags);

      // Feb/05/2001 Added support for Tripplite Omnismart
      // 450PNP, this UPS shutdowns inverter when data is 
      // sent over the Tx line (jhcaiced)
     
      if (pups->id == 6) {
        sleep(2);
        write(g_fd, &killchar, 1);
      }

      close(g_fd);

      // ********************************************************* //
      // We never should have gotten here.                         //
      // The inverter kill has failed for one reason or another.   //
      // If still in powerfail mode, exit with an error.           //
      // If power is ok (power has returned) let rc.0 finish the   //
      // reboot.                                                   //
      // ********************************************************* //
      if (getlevel(&pups->powerok,flags) == 0) {
        fprintf(stderr, "%s: UPS inverter kill failed.\n", program_name);
        exit(1);
      } // if (getlevel(&pups->powerok,flags) == 0)

      // Otherwise, exit normaly, power has returned.
      exit(0);
    } else {
      fprintf(stderr, "Error: %s: UPS <%s> has no support for killing the inverter.\n", 
          program_name,pups->tag);
      exit(1);
    }                                       // if (pups->kill)
  }                                         // if (ikill) 

  // *************************************** //
  // If no kill signal, monitor the line.    //
  // *************************************** //

  // Start syslog.
  openlog(program_name, LOG_CONS|LOG_PERROR, LOG_DAEMON);

  // Open monitor device.
  if ((g_fd = open(argv[1], O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
  //if(0) {
    syslog(LOG_ERR, "%s: %s", argv[1], sys_errlist[errno]);
    closelog();
    exit(1);
  } // if ((g_fd = open(argv[1], O_RDWR | O_NDELAY)) < 0) 
 

  // Explicitly clear both DTR and RTS as soon as possible 
  ioctl(g_fd, TIOCMBIC, &rts_bit);
  ioctl(g_fd, TIOCMBIC, &dtr_bit);
 
  // clear killpower, apply cablepower to enable monitoring
  //setlevel(g_fd,pups->kill.line,!pups->kill.inverted);
  //setlevel(g_fd,pups->cablepower.line,!pups->cablepower.inverted); 
  
  ioctl(g_fd, TIOCMBIS, &rts_bit);
  //ioctl(g_fd, TIOCMBIS, &dtr_bit);

  // close(g_fd);
  // Daemonize. 
#ifndef FORK_DISABLE
  switch(fork()) {
    case 0: // Child
      closelog();
      setsid();
      break;
    case -1: // Error
      syslog(LOG_ERR, "can't fork.");
      closelog();
      exit(1);
    default: // Parent
      closelog();
      exit(0);
  } // switch(fork())
#endif

  // Add by Tiger
  if ((pid = open (PID_FILE, O_RDONLY)) >= 0) {
    status = read (pid, pbuf, (sizeof pbuf) - 1);
    close (pid);
    pbuf [status] = 0;
    pid = atoi (pbuf);

    // If the previous server process is not still running,
    //   write a new pid file immediately.
    if (pid && (pid == getpid () || kill (pid, 0) < 0)) {
      unlink (PID_FILE);
      if ((pid = open (PID_FILE, O_WRONLY | O_CREAT, 0640)) >= 0) {
        sprintf (pbuf, "%d\n", (int)getpid ());
        write (pid, pbuf, strlen (pbuf));
        close (pid);
        pidfilewritten = 1;
      }
    } else {
      fprintf(stderr, "There's already a genpowerd running.\n");
      exit(1);
    }
  }     

  if (!pidfilewritten) {
    unlink (PID_FILE);
    if ((pid = open (PID_FILE, O_WRONLY | O_CREAT, 0640)) >= 0) {
      sprintf (pbuf, "%d\n", (int)getpid ());
      write (pid, pbuf, strlen (pbuf));
      close (pid);
    }
  }
  // added by tiger until here
  
  // Restart syslog.
  openlog(program_name, LOG_CONS, LOG_DAEMON);

  // Create an info file for powerfail scripts.
  unlink(UPSSTAT);
  if ((fd2 = open(UPSSTAT, O_CREAT|O_WRONLY, 0644)) >= 0) {
    write(fd2, "OK\n", 3);
    close(fd2);
  } // if ((fd2 = open(UPSSTAT, O_CREAT|O_WRONLY, 0644)) >= 0)
  
  // Add by Tiger  
  // setting baud_rate = 2400 8N1

  //tcgetattr(g_fd,&oldtio); // save current serial port setting 

  tcgetattr(g_fd, &options);  // retrieve default options....

  cfsetispeed (&options, B2400);
  cfsetospeed( &options, B2400);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_iflag |= (IXON | IXOFF | IXANY);
  
  options.c_cflag |= (CLOCAL | CREAD);
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= ~OPOST;
  options.c_cc[VMIN]  = 0;
  options.c_cc[VTIME] = 10;

  tcsetattr(g_fd, TCSANOW, &options); // applied the modifed options...


/* // async IO setting, added by YC
  saio.sa_handler = signal_handler_IO; // install handler
  //saio.sa_mask = (sigset_t)0;
  saio.sa_flags = 0;
  saio.sa_restorer = NULL;
  sigaction(SIGIO,&saio,NULL);

  fcntl(g_fd, F_SETFL, FASYNC);
*/ // end async IO setting


  fcntl(g_fd, F_SETFL, 0);

  // if is a correct smart ups model, probe the status (to get the 1st power ok message)
  if(pups->smart_ups.smart && pups->id == 8) {
    cstatus = !detect_cable(g_fd);
    if(cstatus) probe_status(g_fd);    
    else {
      g_smart_ups_status = 0;
      coldstatus = cstatus;
      syslog(LOG_ALERT, "No power on startup; UPS connection error?");
      powerfail(3);
    }
  } 
  poldstat = Get_Power_Status ();
  boldstat = Get_Battery_Status ();

  // now loop to poll / monitor the line 
  while(1) {
    // Add by Tiger
    // Get the status.
    ioctl(g_fd, TIOCMGET, &flags);

    if (pups->smart_ups.smart) {
      if (pups->id == 8) { //  for APC Smart-UPS
        get_smart_ups_status (g_fd);
      }
    } else {    //  until here
      // Calculate present status.
      int line_rng;

      pstatus = getlevel(&pups->powerok,flags);
      bstatus = getlevel(&pups->battok,flags);
      line_rng = getlevel(&pups->reserve,flags);
      cstatus = pstatus || bstatus || line_rng;
//printf ("pstatus = %d, poldstat = %d\n", pstatus, poldstat);
//printf ("bstatus = %d, boldstat = %d\n", bstatus, boldstat);
      //  Modify by Tiger
      if (pups->cableok.line) {
        // Check the connection.
        if (cstatus != coldstatus) {
          tries = 0;
          while(getlevel(&pups->cableok,flags) == 0) {
            // Keep on trying, and warn every two minutes.
            if ((tries % 60) == 0) syslog(LOG_ALERT, "UPS connection error");
            sleep(2);
            tries++;
            ioctl(g_fd, TIOCMGET, &flags);
          } // while(getlevel(&pups->cableok,flags)
          if (tries > 0) syslog(LOG_ALERT, "UPS connection OK");
        }
      } else {
        count++;
        if (count < 1) {
          //  Wait a little to ride out short brown-outs
          sleep(1);
          continue;
        } // if (count < 4)
//printf ("cstatus = %d, coldstatus = %d\n", cstatus, coldstatus);
        if (cstatus != coldstatus) {
          if (cstatus) { // recovered from a bad connection... do a check on various flags
            syslog(LOG_ALERT, "UPS connection OK a");
            powerfail (4);
            if (!bstatus && !pstatus) {
              // Power is out and Battery is now low, SCRAM!
              syslog(LOG_ALERT, "UPS battery power is low!");
              powerfail(2);
            }
            else if (!pstatus) {
              // Battery OK, normal shutdown
              syslog(LOG_ALERT, "Line power has failed");
              powerfail(1);
            }
          } 
          else { // no connection....
            // Power is out: assume bad cable, but semi-scram to be safe
            syslog(LOG_ALERT, "No power on startup; UPS connection error?");
            // Set status registers to prevent further processing until
            // the status of the cable is changed.                     
            poldstat = pstatus;
            boldstat = bstatus;
            powerfail(3);
          }                                         
        }
      } // if (pups->cableok.line)
      
      // If anything has changed, process the change ( with the pre-requisite that the 
      // connection is okay)
      if ( cstatus && (pstatus != poldstat || bstatus != boldstat) ) {

        if (pstatus != poldstat) {
          if (pstatus) { // Power is OK
            syslog(LOG_ALERT, "Line power restored");
            powerfail(0);
          } else { // Power has FAILED
            if (bstatus) { // Battery OK, normal shutdown
              syslog (LOG_ALERT, "Line power has failed");
              powerfail(1);
            } else { // Battery failing!!!
              if (pups->id == 7) { //  for PhoenixTec A1000 only
                if (cstatus) {
                  syslog(LOG_ALERT, "UPS battery power is low!");
                  powerfail(2);
                } else {
                  syslog(LOG_ALERT, "No power on startup; UPS connection error?");
                  powerfail (3);
                }
              } else { // Low Battery, SCRAM!
                syslog(LOG_ALERT, "UPS battery power is low!");
                powerfail(2);
              }
            }               // if (bstatus)
          }                 // if (pstatus)
        }                   // if (pstatus != poldstat)

        if ( cstatus && (bstatus != boldstat) ) {
          if (pups->id == 7) { //  for PhoenixTec A1000 only
            if (!bstatus && !pstatus) {
              if (cstatus) {
                syslog(LOG_ALERT, "UPS battery power is low!");
                powerfail(2);
              } else {
                syslog(LOG_ALERT, "No power on startup; UPS connection error?");
                powerfail (3);
              }
            }
          } else {
            if (!bstatus && !pstatus) {
              // Power is out and Battery is now low, SCRAM!
              syslog(LOG_ALERT, "UPS battery power is low!");
              powerfail(2);
            } // if (!bstatus && !pstatus)
            else if (bstatus && !pstatus) {
              syslog (LOG_ALERT, "Line power has failed");
              Set_Battery_Status (BATTERY_OK);
              powerfail(1);
            } else {
              syslog(LOG_ALERT, "Line power restored");
              powerfail(0);
            }
          }
        } // if (bstatus != boldstat)
        poldstat = pstatus;
        boldstat = bstatus;

      } // if (pstatus != poldstat || bstatus != boldstat)
    } //if (pups->smart)

    // modify until here
    
    // Reset count, remember status and sleep 1 seconds.
    count = 0;
    coldstatus = cstatus; 
    sleep(1);
  } //  while(1)
  
  // Never happens
  return(0);
}                                                       // main
/************************************************************************/
/* End of Function main                                                 */
/************************************************************************/

void probe_status(int fd) {

  char smart_status[128];
  int status = 0; 

  write (fd, pups->smart_ups.init_command, strlen (pups->smart_ups.init_command));
  read (fd, smart_status, sizeof(smart_status));
    
  write (fd, pups->smart_ups.detect_command, strlen (pups->smart_ups.detect_command));  
  if(read (fd, smart_status, sizeof(smart_status))<=0) return;
  
  smart_status[2] = 0;  
  status = str_to_hex (smart_status);
  
  if (status & 0x08)        powerfail (0);
  else if (status == 0x50)  powerfail(2);
  else if (status == 0x10)  powerfail(1);
}

// return 0 if have a connection to a APC-SMART UPS
// return 1 if DO NOT have a connection
int detect_cable(int fd) {  
  // check for loop-back connection first...
  int cts_bit = TIOCM_CTS;
  int flags = 0;
      
  ioctl(fd, TIOCMBIC, TIOCM_RTS); // pin 7
  ioctl(fd, TIOCMBIC, TIOCM_CTS); // pin 8

  ioctl(fd, TIOCMGET, &flags);
  
  ioctl(fd, TIOCMBIS, &cts_bit);
  ioctl(fd, TIOCMGET, &flags);
  
  // if a loop-back is detected, should have a connection...
  if( (flags & TIOCM_CTS) ) {
    // a last test is to send a smart-ups init command to see if the server responded
    return init_smart_ups(fd);
  }
  // else, loop back failed, don't have a connection....
  return 1;
}

// return 1 if failed
// return 0 if succeeded
int init_smart_ups(int fd) {

  char smart_status[8];
  struct timeval Timeout;  
  fd_set readfs;

  /// initialize data structures...
  bzero (smart_status, sizeof (smart_status));
  
  Timeout.tv_usec = 0;
  Timeout.tv_sec  = 3;
  FD_SET(fd, &readfs);

  // send init command  
  write (fd, pups->smart_ups.init_command, strlen (pups->smart_ups.init_command));

  // wait for response... (for at most Timout seconds)
  if(!select(fd + 1, &readfs, NULL, NULL, &Timeout)) 
    return 1; // timed-out... return fail

  // select tells us we have something to read.... but we do not 
  // want the response, so just flush it out of the source....
  // (and make a last sanity check, if we read nothing, init has failed)
  return (read (fd, smart_status, sizeof(smart_status)) <= 0);
} 

int get_status(int fd) {

  char smart_status[8];  
  bzero (smart_status, sizeof (smart_status));

  /* block until input becomes available */
  while(1) {
    
    // if some thing is read from the fild descriptor
    if(read (fd, smart_status, sizeof(smart_status))) {
      #define MATCH(x) (smart_status[0] == x)
      if( MATCH('%') )                                  return 0x50;
      else if( MATCH('+') || MATCH('$') || MATCH('=') ) return 0x08;
      else if( MATCH('!') || MATCH('#') || MATCH('&') ) return 0x10;
    }
    else { // nothing is read... check connection...  
      cstatus = !detect_cable(fd);
      // if connection status has changed, take appropriate action
      if(coldstatus != cstatus) {
        coldstatus = cstatus;
        if(!cstatus) { // no cable...
          syslog(LOG_ALERT, "No power on startup; UPS connection error?");
          powerfail(3);
        }
        else {
          syslog(LOG_ALERT, "UPS connection OK");
          powerfail(4);
        }
      }      
    }
    // wait for 1 sec before polling again
    sleep(1);
  }
  
  // never gets here.... but be safe... cancel shutdown...
  return 0x08;  
}

//  Add by Tiger Fu
//  Modified by YC lin... 
//  this function sends an init command to fd to initialize the smart-ups, 
//  and waits for a status code to be returned from the ups. 
void get_smart_ups_status (int fd)
{  
  // always do a init command for the UPS might have just recovered from a shutdown,
  // and requires a init_command to enter smart-mode
  if( init_smart_ups(fd) ) {
    cstatus = 0; // bad cable if entered.... 
    if(coldstatus != cstatus) {
      coldstatus = cstatus; // record old connection status to prevent re-logging repeating events
      syslog(LOG_ALERT, "No power on startup; UPS connection error?");
      powerfail(3);
    }
  }
  else { // init succeeded... proceed with get_status...
    int status = get_status(fd);
        
    if ((g_smart_ups_status != status) && (status >= 8)) {
      g_smart_ups_status = status;
      if (status == 0x08)
        powerfail (0);
      else if (status == 0x50)
        powerfail(2);
      else if (status == 0x10) {
        powerfail(1);
      }
    }
  }
}
 
 //  until here
/************************************************************************/
/* Function             : getlevel                                      */
/* Author               : Erwin Authried <erwin@ws1.atv.tuwien.ac.at>   */
/* Created              : 1995/06/19                                    */
/* Last Modified By     :                             Date:             */
/*                                                                      */
/* Takes                : l: pointer to LINE-structure to test          */
/*                      : flags: bits from ioctl-call                   */
/*                                                                      */
/* Returns              : int containing result of test.                */
/*                      :                                               */
/*                      : 1 == Tested line is in normal mode (active)   */
/*                      : 0 == Tested line is in failure mode           */
/*                                                                      */
/* Purpose              : Tests the condition of a serial control line  */
/*                      : to see if it is in normal or failure mode.    */
/*                                                                      */
/************************************************************************/ 
int getlevel(LINE *l,int flags)
{
  int ret;  

  if (!l->line) return 1;               /* normal mode */
  ret = l->line & flags;
  return (l->inverted) ? !ret: ret;
}

/************************************************************************/
/* Function             : setlevel                                      */
/* Author               : Erwin Authried <erwin@ws1.atv.tuwien.ac.at>   */
/* Created              : 1995/06/19                                    */
/* Last Modified By     :                             Date:             */
/*                                                                      */
/* Takes                : fd:   filedescriptor of serial device         */
/*                      : line: line to change                          */
/*                      : level:new level                               */
/*                      :       1==line is set (pos. voltage)           */
/*                      :       0==line is cleared (neg. voltage)       */
/*                                                                      */
/* Returns              : -                                             */
/*                      :                                               */
/*                                                                      */
/* Purpose              :  set/clear the specified output line          */
/*                                                                      */
/************************************************************************/ 
void setlevel(int fd,int line,int level)
{
  int bit;
  if (line==0 || line==TIOCM_ST) return;
  bit = line;
  ioctl(fd, (level) ? TIOCMBIS:TIOCMBIC, &bit);
}


/************************************************************************/
/* Function             : powerfail                                     */
/* Author               : Miquel van Smoorenburg                        */
/*                      :  <miquels@drinkel.nl.mugnet.org>              */
/* Created              : 1993/10/28                                    */
/* Last Modified By     : Tom Webster                 Date: 1995/01/21  */
/*                      :  <webster@kaiwan.com>                         */
/*                                                                      */
/* Takes                : Failure mode as an int.                       */
/* Returns              : None.                                         */
/*                                                                      */
/* Purpose              : Communicates power status to init via the     */
/*                      : SIGPWR signal and status files.               */
/*                                                                      */
/************************************************************************/                                                
void powerfail(int ok)
{   
        int fd;
        char genpowerfail[255];
        char warning_str[255];
	struct naslog_event a_event;

        /* Create an info file for init. */
        unlink(PWRSTAT);
        if ((fd = open(PWRSTAT, O_CREAT | O_WRONLY, 0644)) >= 0) {
                if (ok && ok !=4) {
                        /* Problem */
                        write(fd, "FAIL\n", 5);
                } else {
                        /* No problem */
                        write(fd, "OK\n", 3);
                }                                       /* if (ok) */
                close(fd);
        }

        /* Create an info file for powerfail scripts. */
        unlink(UPSSTAT);
        if ((fd = open(UPSSTAT, O_CREAT | O_WRONLY, 0644)) >= 0) {
                switch(ok) {
                        case 0: /* Power OK */
                        case 4:
                                write(fd, "OK\n", 3);
                                sprintf (genpowerfail, "%s stop", POWERFAIL);
                                Set_Power_Status (POWER_OK);
                                Set_Battery_Status (BATTERY_OK);
                                
                                //  log event
                                strcpy (warning_str, ok==0?
                                  "UPS: The AC power is back!":
                                  "UPS: Connection OK");
                                break;

                        case 1: /* Line Fail */
                                write(fd, "FAIL\n", 5);
                                sprintf (genpowerfail, "%s start", POWERFAIL);
                                Set_Power_Status (POWER_FAIL);
                                
                                //  log event
                                strcpy (warning_str, "UPS: The AC power is down!");
                                
                                break;

                        case 2: /* Line Fail and Low Batt */
                                write(fd, "SCRAM\n", 6);
                                sprintf (genpowerfail, "%s start", POWERFAIL);
                                Set_Battery_Status (BATTERY_LOW);
                                strcpy (warning_str, "UPS: The Bettery power is low!");
                                break;

                        case 3: /* Bad cable? */
                                write(fd, "CABLE\n", 6);
                                sprintf (genpowerfail, "%s start", POWERFAIL);
                                Set_Battery_Status (UNKNOWN_STATUS);
                                Set_Power_Status (UNKNOWN_STATUS);
                                strcpy (warning_str, "UPS: Possible bad cable!");
                                break;

                        default: /* Should never get here */
                                write(fd, "SCRAM\n", 6);
                                strcpy (warning_str, "UPS: Unknow error!");
                }                                       /* Switch(ok) */
                close(fd);

		a_event.type = EVENT_TYPE_WARN;
		strcpy(a_event.event_user, "");
		strcpy(a_event.event_comp, "");
		strncpy(a_event.event_desc, warning_str, DESC_LENGTH);

		naslog_event_add(&a_event);
        }
        system (genpowerfail);

        // modify by Tiger
        //kill(1, SIGPWR);
} /*EOF powerfail */
/************************************************************************/
/* End of Function powerfail                                            */
/************************************************************************/
