/************************************************************************/
/* File Name            : gengetstatus.c                                */
/* Program Name         : gengetstatus                Version: 1.0.0    */
/* Author               : Tiger Fu <tigerfu@iei.com.tw>                 */
/* Created              : 2001/04/10                                    */
/*                                                                      */
/* Usage                : gengetstatus [-r] [-d] <serial device>        */
/************************************************************************/
 
#include <sys/types.h>
#include <sys/ioctl.h>
//#include <sys/io.h> /* for glibc */
#include <sys/time.h>
#include <sys/select.h>
#include <linux/serial.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <termios.h>



// Tiger
#include <Util.h>
#include "cfg_ups.h"
#include "genpowerd.h"
#include "gen_common.h"

#define MAXSTRING 			10
#define MAXROW	   			6
#define MAXCOL	   			2

/* Global Array */
int statarray[MAXROW][MAXCOL];

int linestat(int, int);
int tester(int, int);
void stater(char *stat, int linestat, int oldstat);
void stater2(char *stat, int linestat);

/* Main program. */
int main(int argc, char **argv) {
	int fd;
	int rts_bit = TIOCM_RTS;
	int dtr_bit = TIOCM_DTR;
	int rts_set   = 0;
	int dtr_set   = 0;
        int status    = 1;
	int current   = 0;
	int next      = 1;
	int x	      = 0;
	int cable_ok  = 1;
	int pstatus   = 0;   
	int bstatus   = 0;
	
	char result    = 0;
	char *program_name;
	//char smart_status[128];
	char stat[MAXSTRING];
	//struct termios options;
	/*char tags[MAXSTRING][MAXROW] = { "DTR",
				   	 "RTS",
				  	 "CAR",
			  	  	 "CTS",
				  	 "DSR",
				  	 "RNG" };*/
	int lines[MAXROW] = { TIOCM_DTR,
			TIOCM_RTS,
			TIOCM_CAR,
			TIOCM_CTS,
			TIOCM_DSR,
			TIOCM_RNG };
			
	/* Parse the command line */
	program_name = argv[0];
	while ((argc > 1) && (argv[1][0] == '-')) {
		switch (argv[1][1]) {
			case 'r':
				rts_set = 1;
				break;
			case 'd':
				dtr_set = 1;
				break;

			default:
				fprintf(stderr, "Usage: %s [-r] [-d] {device} {ups type}\n", program_name);
				exit(ERROR_PARAMETER);
		} 					/* switch (argv[1][1]) */
		argv++;
		argc--;
	}						/* while ((argc > 1) && (argv[1][0] == '-')) */
	
	/* Done with options, make sure that one port is specified */
	if (argc < 3) {
		fprintf(stderr, "Usage: %s [-r] [-d] {device} {ups type}\n", program_name);
                exit(ERROR_PARAMETER);
	}						/* if (argc != 2) */
	
	for (pups=&ups[0]; pups->tag > 0; pups++){
                if (strcmp(pups->tag,argv[2])==0) break;
        }						/* for (pups=&ups[0]; pups->tag; pups++) */
        if (!pups->tag){
                fprintf(stderr, "Error: %s: UPS <%s> unknown\n", program_name,argv[2]);
                exit(ERROR_PARAMETER);
        }	

	/*********************/
	/* Monitor the line. */
	/*********************/

  	/* Open monitor device. */
  	if ((fd = open(argv[1], O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
		fprintf(stderr, "%s: %s", argv[1], sys_errlist[errno]);
		exit(1);
  	}
//  Add by Tiger
  	if (pups->smart_ups.smart)
  	{
  		struct termios options;
  		char smart_status[128];

  		// setting baud_rate = 2400 8N1
		tcgetattr(fd, &options);
		cfsetispeed (&options, B2400);
		cfsetospeed( &options, B2400);

		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		options.c_iflag |= (IXON | IXOFF | IXANY);

		options.c_cflag     |= (CLOCAL | CREAD);
		options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
		options.c_oflag     &= ~OPOST;
		options.c_cc[VMIN]  = 0;
		options.c_cc[VTIME] = 10;

		tcsetattr(fd, TCSANOW, &options);                
		fcntl(fd, F_SETFL, 0);

		if (pups->id == 8) // APC smart UPS
		{
			int status = 0;

			write (fd, pups->smart_ups.init_command, strlen (pups->smart_ups.init_command));
			read (fd, smart_status, sizeof(smart_status));
			bzero (smart_status, sizeof (smart_status));
			write (fd, pups->smart_ups.detect_command, strlen (pups->smart_ups.detect_command));
			read (fd, smart_status, sizeof(smart_status));
			status = str_to_hex (smart_status);

			if (status != -1)
			{
				if (status & 0x08)
				{
					printf ("AC Power OK (gengetstatus)\n");
					result = GET_STATUS_POWER_OK;
				}
				else if (status == 0x50)
				{
					printf ("Battery Low (gengetstatus)\n");
					result = GET_STATUS_BATTERY_LOW;
				}
				else if (status == 0x10)
				{
					printf ("AC Power Fail (gengetstatus)\n");
					result = GET_STATUS_POWER_FAIL;
				}
			}
			else
			{
				printf ("can't get the status\n");
				result = GET_STATUS_CONNECTION_FAIL;
			}
		}
	}
	else
	{
  	  	/* Line is opened, so DTR and RTS are high. Clear them, can  */
  	  	/* cause problems with some UPSs if they remain set.         */
  	  	ioctl(fd, TIOCMBIC, &rts_bit);
  	  	ioctl(fd, TIOCMBIC, &dtr_bit);

  	  	/* Set line(s) to provide power high to enable monitoring of line. */
		if (rts_set)
  	  		ioctl(fd, TIOCMBIS, &rts_bit);  //  if (rts_set)

		if (dtr_set)
  	  		ioctl(fd, TIOCMBIS, &dtr_bit);  // if (dtr_set)

		status = linestat(fd, current);		//  clear old status
		status = linestat(fd, current);		//  clear old status
		status = linestat(fd, current);		//  clear old status
                
		for ( x=2; x < MAXROW; x++) 
		{
			stater(stat, statarray[x][current], statarray[x][next]);
			if (pups->id == 7) //  for PhoenixTec A1000
			{
				if (lines[x] == pups->reserve.line)
				{
					if (pups->reserve.inverted == !statarray[x][current])
						cable_ok = 1;
					else
						cable_ok = 0;
					break;
				}
			}
			else
			{
				if (lines[x] == pups->cableok.line)
				{
					if (pups->cableok.inverted == !statarray[x][current])
						cable_ok = 1;
					else
						cable_ok = 0;
					break;
				}
			}
			
                        if (lines[x] == pups->powerok.line)
				{
					//return statrray[x][current];
					pstatus = (pups->powerok.inverted == !statarray[x][current]);
				}
        
				if (lines[x] == pups->battok.line)
				{
					//return starry[x][current];
					bstatus =  (pups->battok.inverted == !statarray[x][current]);
				}			
		}

		if (!cable_ok && !pstatus && !bstatus)
		{
			result = GET_STATUS_CONNECTION_FAIL;
		}
		else
		{
		    if(pstatus)
		      result |= GET_STATUS_POWER_OK;
		    else
		      result |= GET_STATUS_POWER_FAIL;
		      
		    if(bstatus)
		      result |= GET_STATUS_BATTERY_OK;
		    else
		      result |= GET_STATUS_BATTERY_LOW;		    		    
		}
	}
	if (fd)
		close (fd);
  	/* Never happens */
  	exit (result);
} 							/* main */

/************************************************************************/
/* End of Function main                                                 */
/************************************************************************/
 
/************************************************************************/
/* Function             : linestat                                      */
/* Author               : Tom Webster <webster@kaiwan.com>              */
/* Created              : 1995/02/05                                    */
/* Last Modified By     : Tiger Fu                    Date:2002/04/0    */
/*                                                                      */
/* Takes                : File handle of open serial port.              */
/*                      : Layer of global to alter.                     */                                                
/*                                                                      */
/* Returns              : int containing result of test.                */
/*                      :                                               */                                                
/*                      : >= 1 == Line status has changed               */
/*                      : 0 == Line status has not changed.             */
/*                                                                      */
/* Purpose              : Tests the serial lines to see if there have   */
/*                      : been any changes, alters global array and     */
/*                      : returns a positive int if changed.            */                                                
/*                                                                      */
/************************************************************************/
int linestat(int fd, int layer)
{
	int flags;
	int x = 0;
	int otherlayer;
	int status = 0;
	int lines[MAXROW] = { TIOCM_DTR,
			TIOCM_RTS,
			TIOCM_CAR,
			TIOCM_CTS,
			TIOCM_DSR,
			TIOCM_RNG };
	
	/* Compute the last layer from the current */
	if (layer == 1) {
		otherlayer = 0;
	} else {
		otherlayer = 1;
	}
	
        /* Get the status. */
        ioctl(fd, TIOCMGET, &flags);
	for(x=0; x < MAXROW; x++) {
		statarray[x][layer] = tester(lines[x], flags); 
//printf ("statarray[%d][%d] = %d\n", x, layer, statarray[x][layer]);
		if (statarray[x][layer] != statarray[x][otherlayer]) {
			status++;
		}
	}	
	return(status);

} /* EOF linestat */
/************************************************************************/
/* End of Function linestat                                             */
/************************************************************************/


/************************************************************************/
/* Function             : tester                                        */
/* Author               : Tom Webster <webster@kaiwan.com>              */
/* Created              : 1995/01/22                                    */
/* Last Modified By     :                             Date:             */
/*                                                                      */
/* Takes                : Line of serial connection to test (int).      */
/*                      :                                               */
/*                      : The current status of the serial connection   */
/*                      : as an int.                                    */
/*                                                                      */
/* Returns              : int containing result of test.                */
/*                      :                                               */
/*                      : 1 == Tested line is in normal mode.           */
/*                      : 0 == Tested line is in failure mode.          */
/*                                                                      */
/* Purpose              : Tests the condition of a serial control line  */
/*                      : to see if it is in normal or failure mode.    */
/*                                                                      */
/************************************************************************/                                                
int tester(int testline, int testflags) 
{
        int genreturn;
        if ((testflags & testline)) {
        	/* ON Value Returned */
                genreturn = 1;
        }else{
        	/* Off Value Returned */
                genreturn = 0;
        }
        return(genreturn);
} /* EOF tester */
/************************************************************************/
/* End of Function tester                                               */
/************************************************************************/
 
/************************************************************************/
/* Function             : stater                                        */
/* Author               : Tom Webster <webster@kaiwan.com>              */
/* Created              : 1995/01/22                                    */
/* Last Modified By     :                             Date:             */
/*                                                                      */
/* Takes                : Pointer to character array.                   */
/*                      :                                               */
/*                      : The current status of the serial connection   */
/*                      : as an int.  (zero or non-zero values)         */
/*                                                                      */
/* Returns              : Places text string in provided array.         */
/*                      : If line status is non-zero, "Low" is          */
/*                      : returned.  Otherwise, "High" is returned.     */
/*                                                                      */
/* Purpose              : Converts the int status of serial control     */
/*                      : line to human readable tags.                  */
/*                                                                      */
/************************************************************************/                                                
void stater(char *stat, int linestat, int oldstat)
{
	if (linestat == oldstat) {
		if (linestat) {
			strcpy(stat, "High  ( )");
		} else {
			strcpy(stat, "Low   ( )");
		}
	} else {
		if (linestat) {
			strcpy(stat, "High  (*)");
		} else {
			strcpy(stat, "Low   (*)");
		}
	}
}
/************************************************************************/
/* End of Function stater                                               */
/************************************************************************/

/************************************************************************/
/* Function             : stater2                                       */
/* Author               : Tom Webster <webster@kaiwan.com>              */
/* Created              : 1995/01/22                                    */
/* Last Modified By     :                             Date:             */
/*                                                                      */
/* Takes                : Pointer to character array.                   */
/*                      :                                               */
/*                      : The current status of the serial connection   */
/*                      : as an int.  (zero or non-zero values)         */
/*                                                                      */
/* Returns              : Places text string in provided array.         */
/*                      : If line status is non-zero, "Set" is          */
/*                      : returned.  Otherwise, "Cleared" is returned.  */
/*                                                                      */
/* Purpose              : Converts the int status of serial control     */
/*                      : line to human readable tags.                  */
/*                                                                      */
/************************************************************************/                                                
void stater2(char *stat, int linestat)
{
	if (linestat) {
		strcpy(stat, "Set");
	} else {
		strcpy(stat, "Cleared");
	}
}
/************************************************************************/
/* End of Function stater2                                              */
/************************************************************************/
