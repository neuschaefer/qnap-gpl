/*
 *  apcupsd.c -- Simple Daemon to catch power failure signals from a
 *		   BackUPS, BackUPS Pro, or SmartUPS (from APCC).
 *		-- Now SmartMode support for SmartUPS and BackUPS Pro.
 *
 *  Copyright (C) 1996-99 Andre M. Hedrick <andre@suse.com>
 *  All rights reserved.
 *
 */

/*
   Copyright (C) 1999-2004 Kern Sibbald

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

 */

/* The big overall flow of apcupsd is as follows
 *
 *   Assuming no networking features are turned on.
 *
 * Main Process:
 *   process configuration file
 *   establish contact with the UPS
 *   read the state of the UPS
 *   Fork to become a daemon
 *   write the shared memory
 *   start child process apcdev -- subroutine do_device()
 *   start child process apcnis, if configured -- subroutine do_server()
 *   wait for child processes to complete
 *   exit
 *
 * Child process apcdev -- subroutine do_device()
 *   wait for activity on serial port or timeout 
 *	  from select() (5 seconds) -- check_serial()
 *   call do_action()
 *	  read and lock the shared memory
 *	  check to see if UPS state has changed and if we should take any action. I.e.
 *		are we on batteries, should we shutdown, ...
 *	  write and unlock shared memory
 *   call fillUPS()
 *	  read and lock shared memory
 *	  Poll the UPS for each state variable
 *	  enable the UPS
 *	  enable the UPS again
 *	  write and unlock shared memory
 *   call do_reports()
 *	  if DATA report time expired, write DATA report -- log_data()
 *	  if STATUS report time expired, write STATUS report
 *   loop
 *
 * 
 */ 

#include "apc.h"

/*
 * myUPS is a structure that need to be defined in _all_ the forked processes.
 * The syncronization of data into this structure is done with the shared
 * memory area so this is made reentrant by the shm mechanics.
 */
UPSINFO *core_ups = NULL;
char argvalue[MAXSTRING];

static void daemon_start(void);

int shm_OK = 0;

/**********************************************************************
 * the terminate function and trapping signals allows apcupsd
 * to exit and cleanly close logfiles, and reset the tty back
 * to its original settings. You may want to add this
 * to all configurations.  Of course, the file descriptors
 * must be global for this to work, I also set them to
 * NULL initially to allow terminate to determine whether
 * it should close them.
 *********************************************************************/
void apcupsd_terminate(int sig)
{
    UPSINFO *ups = core_ups;

    restore_signals();

    if (sig != 0) {
        log_event(ups, LOG_WARNING, _("apcupsd exiting, signal %u\n"), sig);
    }

    clear_files();

    device_close(ups);

    delete_lockfile(ups);

    clean_threads();
    log_event(ups, LOG_WARNING, _("apcupsd shutdown succeeded"));
    destroy_ups(ups);
    closelog();
    _exit(0);
}

void apcupsd_error_cleanup(UPSINFO *ups)
{
    device_close(ups);
    delete_lockfile(ups);
    clean_threads();
    log_event(ups, LOG_ERR, _("apcupsd error shutdown completed"));
    destroy_ups(ups);
    closelog();
    exit(1);
}

/*********************************************************************
 * subroutine error_out prints FATAL ERROR with file,
 *  line number, and the error message then cleans up 
 *  and exits. It is normally called from the Error_abort
 *  define, which inserts the file and line number.
 */
void apcupsd_error_out(const char *file, int line, const char *fmt,...)
{
    char      buf[256];
    va_list   arg_ptr;
    int       i;

    asnprintf(buf, sizeof(buf), _("apcupsd FATAL ERROR in %s at line %d\n"), 
	      file, line);
    i = strlen(buf);
    va_start(arg_ptr, fmt);
    avsnprintf((char *)&buf[i], sizeof(buf)-i, (char *) fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "%s", buf);
    log_event(core_ups, LOG_ERR, "%s", buf);
    apcupsd_error_cleanup(core_ups);		    /* finish the work */
}

/* subroutine error_exit simply prints the supplied error
 * message, cleans up, and exits
 */
void apcupsd_error_exit(const char *fmt,...)
{
    char      buf[256];
    va_list   arg_ptr;

    va_start(arg_ptr, fmt);
    avsnprintf(buf, sizeof(buf), (char *) fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(stderr, "%s", buf);
    log_event(core_ups, LOG_ERR, "%s", buf);
    apcupsd_error_cleanup(core_ups);		    /* finish the work */
}


/*********************************************************************/
/*			 Main program.					   */
/*********************************************************************/

int main(int argc, char *argv[]) {
#ifndef HAVE_PTHREADS
    int serial_pid = 0;
#endif
    UPSINFO  *ups;
    int tmp_fd, i;

    /*
     * Set specific error_* handlers.
     */
    error_out = apcupsd_error_out;
    error_exit = apcupsd_error_exit;

    /*
     * Default config file. If we set a config file in startup switches, it
     * will be re-filled by parse_options()
     */
    cfgfile = APCCONF;

    /*
     * Create fake stdout in order to circumvent problems
     * which occur if apcupsd doesn't have a valid stdout
     * Fix by Alexander Schremmer <alex at alexanderweb dot de>
     */
    tmp_fd = open("/dev/null", O_RDONLY, 644);
    if (tmp_fd > 2) {
	close(tmp_fd);
    } else {
	for (i=1; tmp_fd+i <= 2; i++) {
	    dup2(tmp_fd, tmp_fd+i);
	}
    }
    /*
     * If NLS is compiled in, enable NLS messages translation.
     */
    textdomain("apcupsd");

    /* If there's not one in libc, then we have to use our own version
     * which requires initialization.
     */
    init_proctitle(argv[0]);
    astrncpy(argvalue, argv[0], sizeof(argvalue));
      
    ups = new_ups();		      /* get new ups */
    if (!ups) {
        Error_abort1(_("%s: init_ipc failed.\n"), argv[0]);
    }
    init_ups_struct(ups);
    core_ups = ups;		      /* this is our core ups structure */

    /*
     * parse_options is self messaging on errors, so we need only to exit()
     */
    if (parse_options(argc, argv)) {
	exit(1);
    }
    Dmsg0(10, "Options parsed.\n");

    if (show_version) {
        printf("apcupsd " APCUPSD_RELEASE " (" ADATE ") " APCUPSD_HOST"\n");
	exit(0);
    }

    /* We open the log file early to be able to write to it
     * it at any time. However, please note that if the user
     * specifies a different facility in the config file
     * the log will be closed and reopened (see match_facility())
     * in apcconfig.c.	Any changes here should also be made
     * to the code in match_facility().
     */
    openlog("apcupsd", LOG_CONS|LOG_PID, ups->sysfac);   

#ifndef DEBUG
    if ((getuid() != 0) && (geteuid() != 0)) {
        Error_abort0(_("Needs super user privileges to run.\n"));
    }
#endif

    check_for_config(ups, cfgfile);
    Dmsg1(10, "Config file %s processed.\n", cfgfile);

    /*
     * Attach the correct driver.
     */
    attach_driver(ups);

    if (ups->driver == NULL) {
        Error_abort0(_("Apcupsd cannot continue without a valid driver.\n"));
    }

    Dmsg1(10, "Attached to driver: %s\n", ups->driver->driver_name);

    insertUps(ups);
    ups->start_time = time(NULL);
    delete_lockfile(ups);

    if (!kill_ups_power && go_background) {
       daemon_start();
       /* Reopen log file, closed during becoming daemon */
       openlog("apcupsd", LOG_CONS|LOG_PID, ups->sysfac);   
    }

    setproctitle("apcmain");
    make_pid_file();
    init_signals(apcupsd_terminate);

    /* Create temp events file if we are not doing a killpower */
    if (!kill_ups_power && ups->eventfile[0] != 0) {
	ups->event_fd = open(ups->eventfile, O_RDWR|O_CREAT|O_APPEND, 0644);
	if (ups->event_fd < 0) {
            log_event(ups, LOG_WARNING, "Could not open events file %s: %s\n",
		      ups->eventfile, strerror(errno));
	}
    }

    switch(ups->sharenet.type) {
    case DISABLE:
    case SHARE:
	setup_device(ups);
	break;
    case NET:
	switch(ups->upsclass.type) {
	case NO_CLASS:
	case STANDALONE:
	case SHARESLAVE:
	case SHAREMASTER:
	case SHARENETMASTER:
	    break;
	case NETSLAVE:
	    if (kill_ups_power) {
                Error_abort0(_("Ignoring killpower for slave\n"));
	    }
	    set_ups(UPS_SLAVE);
	    setup_device(ups);
	    if (prepare_slave(ups)) {
                Error_abort0(_("Error setting up slave\n"));
	    }
	    break;
	case NETMASTER:
	    setup_device(ups);
	    if ((kill_ups_power == 0) && (prepare_master(ups))) {
                Error_abort0("Error setting up master\n");
	    }
	    break;
	default:
            Error_abort1(_("NET Class Error %s\n\a"), strerror(errno));
	}
	break;
    case SHARENET:
	setup_device(ups);
	if ((kill_ups_power == 0) && (prepare_master(ups))) {
            Error_abort0("Error setting up master.\n");
	}
	break;
    default:
        Error_abort0(_("Unknown share net type\n"));
    }

    if (kill_ups_power) {
	if (!is_ups_set(UPS_SLAVE)) {
	    kill_power(ups);
	} else {
	    kill_net(ups);
	}
	apcupsd_terminate(0);
    }

    if (create_lockfile(ups) == LCKERROR) {
        Error_abort1(_("Failed to reacquire serial port lock file on device %s\n"), ups->device);
    }

    if (!is_ups_set(UPS_SLAVE)) {
	prep_device(ups);
        /* This isn't a documented option but can be used
	 * for testing dumb mode on a SmartUPS if you have
	 * the proper cable.
	 */
	if (dumb_mode_test) {
	    device_entry_point(ups, DEVICE_CMD_SET_DUMB_MODE, NULL);
	}
    }

    shm_OK = 1;

    /*
     * From now ... we must _only_ start up threads!
     * No more unchecked writes to myUPS because the threads must rely
     * on write locks and up to date data in the shared structure.
     */

    if (slave_count) {
	/* we are the netmaster */
        start_thread(ups, do_slaves, "apcmst", argv[0]);
        Dmsg0(10, "Netmaster thread started.\n");
    }

    /* Network status information server */
    if (ups->netstats) {
        start_thread(ups, do_server, "apcnis", argv[0]);
        Dmsg0(10, "NIS thread started.\n");
    }
   
#ifdef HAVE_PTHREADS
    log_event(ups, LOG_WARNING, "apcupsd " APCUPSD_RELEASE " (" ADATE ") " APCUPSD_HOST " startup succeeded");
    /* If we have threads, we simply go there rather
     * than creating a thread.
     */
    if (!is_ups_set(UPS_SLAVE)) {
	/* serial port reading and report generation -- apcserial.c */
	do_device(ups);
    } else {
        /* we are a slave -- thus the net is our "serial port" */
	do_net(ups);
    }
#else
    if (!is_ups_set(UPS_SLAVE)) {
	/* serial port reading and report generation -- apcserial.c */
        serial_pid = start_thread(ups, do_device, "apcdev", argv[0]); 
    } else {
        /* we are a slave -- thus the net is our "serial port" */
        serial_pid = start_thread(ups, do_net, "apcslv", argv[0]);
    }

    log_event(ups, LOG_WARNING, "apcupsd " APCUPSD_RELEASE " (" ADATE ") " APCUPSD_HOST " startup succeeded");
    wait_for_termination(serial_pid);  /* wait for child processes to terminate */
#endif

    apcupsd_terminate(0);
    return 0;				/* to keep compiler happy */
}

/*
 * Initialize a daemon process completely detaching us from
 * any terminal processes.
 */
static void daemon_start(void)
{
#ifndef HAVE_CYGWIN
    int i;
    pid_t cpid;
    mode_t oldmask;

    if ( (cpid = fork() ) < 0) {
        Error_abort0("Cannot fork to become daemon\n");
    } else if (cpid > 0) {
	exit(0);		  /* parent exits */
    }

    /* Child continues */
    setsid();			       /* become session leader */

    /* Call closelog() to close syslog file descriptor */
    closelog();

    /* In the PRODUCTION system, we close ALL
     * file descriptors except stdin, stdout, and stderr.
     */
    for (i=sysconf(_SC_OPEN_MAX)-1; i > 2; i--) {
       close(i);
    }

    /* move to root directory */
    chdir("/");
   
    /* 
     * Avoid creating files 0666 but don't override a
     * more restrictive umask set by the caller.
     */
    oldmask = umask(022);
    oldmask |= 022;
    umask(oldmask);

#endif /* HAVE_CYGWIN */
}
