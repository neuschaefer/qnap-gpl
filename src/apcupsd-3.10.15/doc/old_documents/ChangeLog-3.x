/***************************************************************************/
/*                     ChangeLog of apcupsd                                */
/*               Riccardo Facchetti <riccardo@master.oasi.gpa.it>          */
/*               http://www.oasi.gpa.it/riccardo/linux/apcupsd             */
/*               http://www.sibbald.com/apcupsd                            */
/*               ftp://ftp.oasi.gpa.it/pub/apcupsd                         */
/***************************************************************************/

----> Released apcupsd-3.8.1
        See techlogs/kes08Dec00 
        Corrected depend creation in config.status.
        See techologs/kes05Dec00 for details
        Fixed Win32 to work with simple signalling UPSes.
        Upgraded Win32 to CGYWIN 1.1.5.
        Fixed an Alpha Tru64 64/32 bit problem in the networking code.
        Fixed the random incorrect on battery timers (I think).
        Upgraded to latest version of gcc 
        Do much better argument checking in the .cgi programs. This
           improves security (at least psychologically).


----> Released apcupsd-3.8.0
        Additional updates to apctest.c, it now tests   
          Smart UPSes as well as Dumb UPSes
        Increase getline buffer size
        Quick documentation of apctest.c

----> Released apcupsd-3.8.0-pre7
        See techologs/kes27Oct00
        Win32 make binary_tar_file
        Rewrote the 940-0119A cable code based on Jason A. Smith's testing.
        Improved apcupsd version print out.
        Win32 and Signaling UPS documentation.

----> Released apcupsd-3.8.0-pre6
        See techlogs/kes21Oct00
        A number of Win32 fixes including an email program for Win95/98.
        Put Win32 system binaries in source code.
        Fixed WinNT problem when running as a master or slave.
        Win32 make install.
        Minor corrections to scripts.
        Fixed a Solaris xlib link problem in the cgi directory.

----> Released apcupsd-3.8.0-pre5 for testing 
        See techlogs/kes17Oct00
        Applied Andr�'s fixes to support the 0119A cable used by the
          BackUPS Office 500.
        Applied Riccardo's fix for Solaris xlib.

----> Released apcupsd-3.8.0-pre4
        Applied Kern's diff, see techlogs/kes06Oct00
        Key points: apcaction.c fix for timer starting during power failure,
          install key event handlers by default that email the 
          following events: changeme, commfailure, commok, mainsback,
          and onbattery.

----> Released apcupsd-3.8.0-pre3
        Applied Kern's diff, see techlogs/kes05Oct00
        Key points: fixes to Win32 binary release, manual updates,
          TIMEOUT bug fix, handle pre-1994 UPSes correctly,
          portability issues.

----> Released winapcupsd-3.8.0-pre2 Win32 binarys only
        Applied Kern's corrections to the Win32 binary files.
        Fix to <src>/win32/apccontrol.in
        Add sh.exe to \apcupsd\bin directory.

----> Released apcupsd-3.8.0-pre-1.
        Applied Kern diffs, see techlogs/kes27Sep00
        Key points: Alpha Tru64 port, documentation updates,
          additional retries in networking, new STATUS variables,
          tighten permission of files, RedHat RPM spec, corrections
          to event scripts.

----> Snapshot released 20000911.
        Applied Kern diffs, see techlogs/kes10Sep00
        Key points: Win32 enhancements, buffer overrun
          fix, tightning of file permissions, manual updates.

----> Snapshot released 20000910.
        WIN32 (cygwin) enhancements (KES): win'ization and use of system tray.
        Documentation update.
        autoconf reorganization.

----> apcupsd-3.7.2 released.
        true and false programs are no more hardcoded into the configure script.
        Reorganized some of the distributions/ Makefiles.
        Added FreeBSD installation support.

----> Snapshot released 20000528
        Now apcaccess.c::stat_print() output status to stdout instead of stderr.
        Reorganized the kill_* code in apcupsd.c::main()
        Removed the kill_ups_power argument from apcnet.c::kill_net()
        Moved all save_dumb_status() calls to apcserial.c::setup_serial()
        Applied kern's patch for kill_net in apcupsd.c and some other
        beautifications of the code.
        Changed scripts/autoregen.sh to compile with all the options switched on.
        Added a web page of thanks to the html manual.
        Moved apcproctitle.c to lib/proctitle.c.  Use that source only if
        there is not a system setproctitle(3).
        Don't use 's' subflag to ar(1).  It's unnecessary since ranlib is
        being called, and more importantly not all ar implementations have it.
        Install distribution-specific apcupsd.conf files if they exist.
        Fixed some calls to open(2) that didn't specify file create modes.
        Added OpenBSD distribution and minor source patches.
        Modified Slackware distribution:  Update README, patch instead of
        replacing rc files, delete obsolete distribution files.
        Updated "makediff" developers' tool.
        Minor manual updates.
        Updated again Developers file: we gained more *BSD help.
        Added the '-p' option to cmdline: was missing.
        Abort on killpower for slaves instead of simply exit, in apcupsd.c
        Removed nologin information from apcnet.c because we don't need. All the
        nologin file is managed by apcaction.c

----> Snapshot released 20000511
        New mantainer for Slackware and OpenBSD ports: updated Developers file.
        Applied David patch for slackware 
        Applied kern diffs, see techlogs/kes26Jan00
        po/POTFILES file is now removed by main Makefile since is always built.
        Corrected Slackware distribution Makefile.in: it was tied to
        /usr/src. Thanks to John McSwain.
        Corrected a bug in configure.in and include/apc_defines.h in which
        the nologin file was created in /etc/apcupsd/nologin instead of
        /etc/nologin.
        Moved README.solaris to doc/.
        Moved the patch "./1" to techlogs/kern-patch-17-Feb-2000

----> apcupsd-3.7.1 released 17 February 2000
        Bumped version to 3.7.1 and released by Kern since Riccardo
        is on vacation.
        Update to upsbible.html to include additional credits.
        Fixed dumb UPSes, which did not work in version 3.7.0,
        see Kern's fixes techlogs/kes12Feb00

----> apcupsd-3.7.0 released.
        Bumped version to 3.7.0 and released.
        Powerflute: last 8 events are loaded from events file (if present).
        Revamped powerflute: now curses work correctly.
        Small fix to include/apc_config.h to clean Solaris compile.
        Clean intl/ a bit better.
        Fixed apcnetlib.c and apcnetd.c: now apcupsd compile under FreeBSD too.
        Applied kern diffs, see techlogs/kes26Jan00
        Fixed compile for Solaris 2.5.1.
        Added check for snprintf when CGI is configured in configure.in.
        Fixed internationalization inclusion when msgfmt is not found.
        Fixed warning in lib/getopt.c.
        Fixed AC_PATH_PROGS in configure.in.
        Verified clean compile under Linux and HP-UX.
        Added libintl and nls in information at the end of configure.
        Corrected stpcpy warning in intl/dcgettext.c.
        Changed all `make -C', this fixes HP-UX compilation of additional modules
        like internationalization and cgi support.
        Fixed detection of UP-UX version.
        In apcnetlib.c use memset instead of bzero.
        In apcupsd.c don't use TIOCNOTTY if not defined.
        Corrected a macro bug in arguments of setpgrp() call in apcupsd.

----> apcupsd-3.7.0-rc1 released.
        Updated my e-mail and (C) signatures: if you want to make it simple,
        use vim and the scripts you can find in scripts/.
        Restructured include/apc.h inclusion order: now make more sense.
        Corrected setpgrp() call in apcupsd.c for BSD/non-BSD.
        Added FreeBSD to configure autodetection and placeholder in distributions/.
        Other minor fixes to Makefile.in and cgi/Makefile.in
        Fixed make clean for cgi.
        Applied kern diffs, see techlogs/kes17Jan00
        Added slackware scripts from John McSwain.
        Added Kern's fixes to distributions/redhat/awkhaltprog.in
        Applied kern diffs, see techlogs/kes16Jan00
        Added handling of `exit 0' at the end of halt.local as in suse 5.x.
        Added autodetection for SuSE 5.x.
        SuSE installation: written a shell script for installing apcupsd
        directives in /etc/rc.config.

----> apcupsd-3.7.0-beta4 released.
        Bumped version to beta4 and released.
        Fixed a minor po/ problem.
        Applied kern diffs, see techlogs/kes13Jan00
        Applied kern diffs, see techlogs/kes12Jan00
        Fixed CYGWIN detection in configure.in.
        Added clean and distclean for distributions/.
        Added slackware detection but still manual installation (scripts
        generated but no actual installation performed: too dangerous doing
        the code without being able to test).

----> apcupsd-3.7.0-beta3 released.
        Bumped version to beta3 and released.
        Applied kern diffs, see techlogs/kes09Jan00
        Changed clean_threads() in apcexec.c not hang on waitpid, and make
        sure we do all we can to kill the childs.
        Updated INSTALL file to make clear that prior to install a new version
        of apcupsd over an old version, is advisable to make uninstall.
        Corrected a syntax error in distributions/*/apccontrol.sh.in.
        Changed suse halt.local script to:
         . do something only if a powerdown is detected.
         . kill the processes before remounting read only the filesystems.
        Corrected a minor cosmetic (-Wall) in apcnetd.c
        Added -Wall to default CFLAGS.
        Applied kern diffs, see techlogs/kes06Jan00
        Kern minor changes to configure.in.
        Thanks to Tom Schroll, corrected an execv nasty error in apcexec.c.
        Applied kern diffs, see techlogs/kes30Dec99

----> apcupsd-3.7.0-beta2 released.
        Bumped version to beta2 and released.
        Changed install-apcupsd target in main Makefile.in not to overwrite
        old apcupsd.conf file and instead create an apcupsd.conf.new file.
        Applied Kern diffs, see techlogs/kes19Dec99
        Changed the install to warn the user that if an old apcupsd.conf is in
        place, it is saved to apcupsd.conf.old.
        Applied Kern diffs, see techlogs/kes18Dec99.
        Reorganized the path construction in include/apc_defines.h so that now
        file paths are built with autoconf variables and not hardcoded.
        Applied Carl Erhorn patches for Solaris, see techlogs/cpe16Dec99.
        Now SuSE 5.2 is correctly detected (don't know previous versions).
        Cleanups of configure.in.
        Corrected a problem with SuSE and halt scripts where killpower where
        not issued correctly.
        Applied Helmut Messerer corrections.
        Applied Kern diffs, see techlogs/kes09Dec99
        Applied Kern diffs, see techlogs/kes08Dec99
        Added a new contrib/ directory where user contributed files are put.
        New user contribution for sending sms messages on UPS troubles.

----> apcupsd-3.7.0-beta1 released.
        Applied Kern diffs, see techlogs/kes30Nov99
        Applied Kern diffs, see techlogs/kes28Nov99
        Applied Kern diffs, see techlogs/kes20Nov99
        Applied Kern diffs, see techlogs/kes18Nov99
        Applied Kern diffs, see techlogs/kes15Nov99
        More internationalization.
        Applied Kern diffs, see techlogs/kes13Nov99
        Moved TIOCM_LE HP-UX define and friends to include/apc_config.h
        Internationalization: internationalized and translated in Italian all the
        messages that are printed with printf and fprintf: more need to be done
        for error_* functions.
        Removed #ifdef wrapping around debug code and substituted with debug_level
        checks.
        Cleaned a remaining spit of `killpower' in apcupsd.c
        Help screen output to stdout.
        Now (C) and Brian's Support Center are visible in help screen output.
        Updates to the cgi files from Kern.
        Added a vimrc for TABs.
        --debug argument changed meaning. Now is meant to be set to a number that
        range from 0 to N where increasing numbers means increasing debug output.
        Now we use getopt_long [see getopt(3)] for parsing command line.
        Documentation updates and reorganization: now `developers' documentation
        is in doc/developers/.
        Added support for cable 940-0095B.
        Applied Kern patches: syslog, getline, code cleanups, new cgi
        interface. Read his technical log in techlogs/kes03Nov99
        Added doc/CodingStyle file.
        Added Developers file.
        Some minor code documentation.
        Removed the "failed to reacquire the lockfile" problem. Now we release
        the lockfile just before fork() and reacquire it just after.
        More duplicated code cleanups and nasty bugs fixed in apcaction.c.
        SuSE's apcupsd start script now return green "done" and red "failed"
        (SuSE 6.2)
        apccontrol is installed in /etc/apcupsd/ because we need it when
        filesystems may be umounted.
        Removed powersc script: now it is all done in apccontrol and
        /etc/rc.d/apcupsd scripts.
        Moved _all_ the scripts into scripts/ directory (where they belong).
        apccontrol script installation dir is now sysconfdir
        ${prefix}/etc/apcupsd
        Simplified apcaction.c (removed duplicate code).
        Attempt to document some features of apcupsd.conf.
        Deleted two alarm()s related to apcreports.c from apcaction.c: this
        was a bug.
        Added setproctitle for setting forked procs's argv[0]. Now we have
        apcmain -> waiting for other tasks to exit, generic watchdog.
        apcser -> serial task
        apcact -> actions task
        apcnet -> network task
        apcslv -> netslave task
        so that now we can tell with a `ps' which task is doing what.
        apcserial+apcreports are now one single thread.
        Better locking scheme.
        Removed another nasty bug in Old* variables into apcaction.c. Now
        there is a local structure for these values.
        Removed a nasty bug in apcsetup.c.
        Updated man pages and apcupsd.conf.
        RedHat installation scripts.
        Removed 10 seconds sleep() from terminate().
        SHM ID for sanity checks on SHM accesses.
        Obsolete config options now generate only warnings.
        New error handling routines.
        doc/README.developers updated.
        Syslogging functions are bracketed with HAVE_GCC so that with gcc we
        can use macros and with other compilers we use functions for
        compatibility.
        Syslog functions are now real functions. There's no point in having
        incompatible preprocessor macros when a set of little functions can do
        the job.
        Fixed a next_slave label in apcnet.c with a semicolon for HPUX
        compiler.
        OS detection in configure: now Makefiles and sources know about which
        OS they are supposed to compile for.
        Better integration of lib/ sources into the configure mechanism.
        configure cleanups.
        Added cflags and ldflags selection in Makefile.in and configure.
        Fixed getopt_long detection in configure. Now if not found it compile
        the lib/ version.
        Now configure.in have a hardcoded PATH in which search the system
        programs needed.
        Lot of cleanups in global variables and code partitioning and
        duplication.
        More configure cleanups.
        Updated RH 6.0 halt script.
        Removed apchttp.
        Cleaned THREADS. Now forked processes are the only option.
        Various cleanups.
        SuSE-specific install/uninstall.
        All distribution specific directories are now in distributions/.
        All scripts are now in scripts/.
        Moved default apcupsd.conf in etc/.
        Added distribution-specific scripts installation.
        Install /etc/apcupsd.conf if not alredy present.
        Removed apc(un)install.sh: no more needed.
        Rewritten external scripts: no more system(), better customization
        support.
        Removed Makefile.in.in from po/: it's not needed.
        Fixed bogus --enable and --disable behavior of configure.
        Cleaned up po/ and intl/ autoconf.
        Only powerflute is linked with ncurses libraries.
        The program makedepend is not any more vital for compilation.

----> apcupsd-3.6.2
        Fixed the "apcupsd -c" configure command.
        Fixed two thread bugs in alarm handling.
        Fixed two potential security exploits.
        More cleaned up autoconf.
        Random documentation cleanups.

----> apcupsd-3.6.1
        Cleaned up new autoconf stuff from version 3.6.0
        Undefined a test flag for testing networks wierdness.
                #undef WACKY_NETWORK_ATTEMPS
        Network is fully functional under non-threaded compile.
        Possible fix for "pipe_master_status" calls on slaves.
        Added 940-1524C smart signal cable support.

----> apcupsd-3.6.0
        Added autoconf.
        Added internationalization support. There is _only_ the support but
        no current code is written for the intl package. It can be compiled
        in, but intl strings have still to be translated (to be done in the
        future).
        Reorganized documentation.
        Reorganized support for distributions. Now we have a directory for
        every possible distribution (suse, unifix, debian etc etc) so that the
        job for the package-men can be easier.

----> apcupsd-3.5.9
        Added new configuration options to reduce init time of daemon.
        powersc CONFIG
        powersc NAME
        powersc BATTERY

----> apcupsd-3.5.8.patch

        Fixes a FIFO error that I forgot to include in the rush
        to release the code

----> apcupsd-3.5.8

        GPL2 source code status finally...........April 7, 1999

        Threaded code is stable but requires glibc2 or libc6
        Finished SELFTEST setup
        Fixed naming of UPS if allowed.
        Redesigned "apcsetup.c" to allow for ease of parameter setup.

----> apcupsd-3.5.7

        Added 940-0024G cable.
        "newbackupspro" to be replaced by "backupspropnp"

        If you have a BackUPS Pro UPS that is identified as
        BP(SIZE)(EXTRAS) examples BP420SC/BP650SC/BP650PRO-PnP,
        then you have a PnP BackUPS Pro that is very near a SmartUPS.
        Else anything with the identifier BK(SIZE)PRO or BKPRO(SIZE)
        is the very early version of the "dumbed down smartups".
        Unfortunately, it has very little to say.  It is superior to the
        classic BackUPS (simple signals) in that it can at least tell event
        histories and if the batteries are faulted. 

        now powerflute is compiling and working, needs THREADS

----> apcupsd-3.5.6

        See Makefile for enabling flags.

        # New multi-threading code BETA
        #
        # THREADS   = 1

        # New multi-threading code with http BETA functional needs THREADS
        #
        # HTTP      = 1

        # New/Old powerflute tool
        #
        # MUSIC     = 1

        dual building model possible complete.
        intergrated beta http data
        breakages in logs and procfs are still present in shm.
        code sorting and corrections.
        return of ncurses powerflute tool.
        fixed Makefile to find two possible locations of
        ncurses package; however, there are three methods for this animal.

        apcconfig.c: removed check for existance of lock directory. It's not
        needed and dangerous when -killpower !
        apcconfig.c: new configuration checks
        added semun definition to apc_struct.h since it is needed for glibc6

----> apcupsd-3.5.5

        dual building model begun.
        intergrated beta shared memory mapping.
        breakages in logs and procfs are present.

----> apcupsd-3.5.4

        Preparation for GPL status and transfer to GNU.
        Fixed Network bug.
        Fixed IPC PIPE bug.

----> apcupsd-3.5.3

        Fixed UPS killpower bug.

----> apcupsd-3.5.2

        Source wide reformat to conform to standard C programing format

----> apcupsd-3.5.1


----> apcupsd-3.5.0

        fixes "NO" reports in setup loops for UPSes that do not allow for
        changing parameters at initialization.  Previously it was assumed that
        UPSes did not report anything if the feature was not pollable and/or
        changeble.

----> apcupsd-3.4.9

        fixes a long missed error that was incorrectly fixed in the past.

----> apcupsd-3.4.8

        solved more mystery functions and the UPSlink language may
        be completely decoded.

----> apcupsd-3.4.7

        now auto-learns features based on UPS's answer to questions.
        initial porting to Solaris for i386 is complete.

----> apcupsd-3.4.6 45493 Jul 15 13:14 apcupsd.c
                    22326 Jul  9 12:50 apcnet.c
                    12129 Jul  9 12:50 apcpipe.c
                    25634 Jul  9 12:50 apcconfig.c
                     8565 Jul 15 13:07 apcsetup.c
                    11039 Jul 15 13:09 apcreports.c
                    18137 Jul 15 13:19 apcsmart.c

        all logging functions will be moved to "apcreports.c"
        all smart mode calls will be moved to "apcsmart.c"
        fix ups model reporting in "apcsetup.c"

----> apcupsd-3.4.5 77096 Jul  7 17:18 apcupsd.c
                    22326 Jul  7 03:12 apcnet.c
                    12129 Jun 22 15:12 apcpipe.c
                    25634 Jul  7 16:39 apcconfig.c

                    18477 Jul  7 02:08 apcaccess.c

        Added NOLOGON delay for systems with large Matrix UPSes.
        Short information polling, with constant values being set
        in the extended setup functions.

----> apcupsd-3.4.4
        Fixed a missing and needed wait delay for netmaster systems.
        This was discovered when mixing flavors of Linux.
        Since SuSE, RHS, and Debain require extra time for shutdowns.
        man-page is closer to date..........

----> apcupsd-3.4.3
        Added new limit features by polling a new command for
        internal calculated remaining time on line.
        Fixed naming UPS if allowed.

----> apcupsd-3.4.2
        Fixed an unknown error for the forced ups-kill for the
        backupspro models. This was discovered with a new
        BackUPS Pro 1000.

----> apcupsd-3.4.1
        Changed ./includes to have an "apc_" prefix.
        This is for initial port requirements to FreeBSD.

----> apcupsd-3.4.0 72305 May 19 14:18 apcupsd.c
                    21597 Apr 16 17:59 apcnet.c
                    12117 Apr 16 15:44 apcpipe.c
                    22832 Apr 16 15:42 apcconfig.c

                    17687 Apr 16 13:47 apcaccess.c

Slave management disconnect/reconnect added to apcaccess.
Fixed excessive loss of UPS communications loggings of ::
        UPSlink Comm. Error, SM != SM
        UPSlink Comm. reestablished, SM == SM

Fixed Signal/Cable Combination errors.
        A "SmartUPS" with a "Simple Cable" with report as a "BackUPS".

----> apcupsd-3.3.0 72450 Mar 20 01:09 apcupsd.c
                    15321 Mar 20 01:08 apcnet.c
                    11093 Mar 20 01:09 apcpipe.c
                    22465 Mar 20 01:09 config.c

                    17300 Mar 20 01:08 apcaccess.c

Network death solved (hopefully)
PowerFlute removed due to unexplainable daemon kills under 2.0.X. and
some cases of 2.1.X. (TCPIP)

This is replaced bye the original tool that now works "apcaccess".
"apcaccess" is functional under both 2.0.X and 2.1.X kernels without
killing the damon "apcupsd".

EPROM programming of many UPS models is now functional.

Example::

apcaccess : polling apcupsd for status.

APC      : Mar 20 01:24:56
CABLE    : APC Cable 940-0024B
UPSMODEL : SmartUPS
UPSMODE  : Stand Alone
ULINE    : 124.0 Volts
MLINE    : 125.2 Volts
NLINE    : 124.0 Volts
FLINE    : 60.0 Hz
VOUTP    : 124.0 Volts
LOUTP    : 028.0
BOUTP    : 27.3 Volts
BCHAR    : 100.0
BFAIL    : 0x08
SENSE    : HIGH
WAKEUP   : 060 Cycles
SLEEP    : 020 Cycles
LOTRANS  : 103.0 Volts
HITRANS  : 129.0 Volts
CHARGE   : 025.0 Percent
UTEMP    : 49.5 C Internal
ALARM    : Low Batt + 30
DIPSW    : 0x0000

root@Orion% cat apcupsd.status
APC      : Mar 20 01:27:31
CABLE    : APC Cable 940-0024B
UPSMODEL : SmartUPS
UPSMODE  : Stand Alone
UPSNAME  : UPS_IDEN
ULINE    : 124.0 Volts
MLINE    : 124.6 Volts
NLINE    : 124.0 Volts
FLINE    : 60.0 Hz
VOUTP    : 124.0 Volts
LOUTP    : 028.0 Load Capacity
BOUTP    : 27.3 Volts
BCHAR    : 100.0 Batt. Charge
BFAIL    : 0x08 Status Flag
SENSE    : HIGH
WAKEUP   : 060 Cycles
SLEEP    : 020 Cycles
LOTRANS  : 103.0 Volts
HITRANS  : 129.0 Volts
CHARGE   : 025.0 Percent
UTEMP    : 49.5 C Internal
ALARM    : Low Batt
DIPSW    : 0x0000

----> apcupsd-3.2.2 66556 Jan 27 15:24 apcupsd.c

Fixed a reporting error in /etc/apcupsd.status for CUSTOM SIMPLE cable.
Fixed a reporting error in /var/log/apcupsd.log for CUSTOM SIMPLE cable.

----> apcupsd-3.2.1 67450 Jan  8 13:41 apcupsd.c

Fixed INSTALL from "readhat" to "redhat"
Fixed INSTALL errors of "slackware"

Bug search that causes network death, lost slaves under 2.0.X only??
Bug search that causes text-powerflute to kill daemon only under 2.0.X.

----> apcupsd-3.2.0 66096 Dec 18 16:11 apcupsd.c

apcflute.c all code for reconfiguration complete.

----> apcupsd-3.1.0 65925 Dec  8 21:18 apcupsd.c

----> apcupsd-3.0.0

Now binaries only...........

----> apcupsd-2.9.9 63431 Dec  3 13:05 apcupsd.c

Fixed "for (killcount=0;killcount>3;killcount++)" bug
                                 ^ killcount<3;

Found by Piotr Kasprzyk <kwadrat@zeus.polsl.gliwice.pl>.

----> apcupsd-2.9.8 62158 Nov 25 15:13 apcupsd.c

Added user defined magic and security timeouts.
Started powerflute update running master or standalone daemon.
Added another field for SmartUPS procfs file.

----> apcupsd-2.9.7 60945 Nov 18 19:19 apcupsd.c

Added second alternate method for killing power for non SU SmartUPS.
I have one of these older models, and it needs a bigger kick.

Disable lockfiles if slave is a netslave with (ups->cable == APC_NET),
or ethernet.

----> apcupsd-2.9.6 60156 Nov 10 22:34 apcupsd.c

split source files...........

10885 Nov 10 20:40 apcnet.c         :: NetUPS
18805 Nov 10 22:37 config.c         :: configuration

5924 Nov 10 19:54 apcflute.c.src    :: powerflute interface to apcupsd
5264 Nov 10 12:43 powerflute.c.src  :: powerflute tcp management of apcupsd

Brian Schau <bsc@fleggaard.dk> added lockfiles for serial ports.

NetUPS or EtherUPS now works.
Needs more security likely, you can set your own TCP port number.
Will add user MAGIC at compile in the future.

----> apcupsd-2.9.5 72431 Oct 28 00:27 apcupsd.c

Listing my hack of "powerflute".  To enable it edit your /etc/apcupsd.conf
and set NETTIME to any value <60> seconds is something to play around.

Reformat subroutines to get apcupsd to handle networks.

----> apcupsd-2.9.4 70707 Oct 25 13:51 apcupsd.c

Smart V/S is in the same class as a BackUPS Pro.

Riccardo Facchetti <riccardo@master.oasi.gpa.it> has been volunteered to solve the
network part for the project. YEAH!!!!!!!!!!!!

Hello Andre,
I own a SmartUPS v/s and I have played a bit with your apcupsd. I have
found that the SmartUPS v/s command set is similar (if not equal) to the
one of BackUPS PRO, so I have changed the apcupsd.c code to behave this
way and seems to work well.

Another thing: I would like to have an application like powerchute. Of
course I am willing to write it, so I will call it powerflute.

My idea is:

apcupsd -> daemon
        - controls power status
        - listen on a TCP socket for incoming connections
                - get a command from TCP line
                - send the response to the command

powerflute -> application
        - connect to apcupsd
                - sends a command to apcupsd
                - get the response from apcupsd

apcupsd is listening to the socket with a select so it don't block. After
one or more connections are established, it read non-blocking from the TCP
file descriptor(s) and when a char is available, send it to UPS and
readline(), then send the buffer (terminated with '\n' to the TCP client
over the connection file descriptor.

You can note that I have protected apcupsd UPS monitoring from TCP
servicing by doing only non-blocking calls to the TCP layer, so there
should be no problem for it in detecting power problems without delay
caused by TCP connections.

A patch is enclosed for v/s == BKpro and for TCP servicing.
Of course this patch is still very preliminar, but I think it is a good
idea.

You can try the idea running the apcupsd with this patch applied,
telnetting to the port 6669 from localhost (no external connections
allowed now for security) and sending to the daemon some commands for the
UPS. The daemon will redirect them to the UPS and send the answer to you.
You can connect more than one client to the daemon (max 64 connections).

Please send me comments about it.

Ciao,
        Riccardo.
--
Riccardo Facchetti          | e-mail: riccardo@master.oasi.gpa.it


The TCP patch is not there but my hack job is present.
The package will have an addition called "powerflute".

----> apcupsd-2.9.3 67655 Oct 23 14:57 apcupsd.c

Fixed spelling of "dissable" to "disable"
Forgot to add this function for ShareUPS.

int fillShareUPS(int sharefd, UPSINFO *ups);

Brian Schau <bsc@fleggaard.dk> with needed install changes in Makefile
for UNIFIX.

----> apcupsd-2.9.2 67410 Oct 22 19:34 apcupsd.c

Changed "case" names in /etc/apcupsd.conf, again.....
This should be the last time...........

ShareUPS project maybe nearing an end.......

make install is almost done.

----> apcupsd-2.9.1 65886 Oct 19 00:30 apcupsd.c

Changed "case" names in /etc/apcupsd.conf to lower case.
UPSTYPE SMARTUPS is now UPSTYPE smartups
Changed and add two new UPSMODE "cases" for ShareUPSes
Added Two new external call command for TIMEOUT and LOADLIMIT.
Shutdown types are now called by event not by a general shutdown.
Added CHANGEME to /sbin/powersc for SmartUPSes to announce that
its needs a batteries changed.

Tests are now run on (2) BackUPS 600's, SmartUPS Net 1400RM & 700RM,
and SmartUPS 1250.

SmartUPSes now can call for shutdown based on percent of remaining
battery life or capacity.

Cleaned up manpage........

----> apcupsd-2.9 58603 Oct 16 22:17 apcupsd.c

Changed to POSIX Style of control.
Now uses an external CONFIG file -> /etc/apcupsd.conf
All events are handled in the /sbin/powersc file, no more
sysvinit flavor hassels....................
Added ManPage, well an attempt.
Other things begun............

----> apcupsd-2.8 42800 Oct  6 14:36 apcupsd.c

Finally solved the PLUG-N-PLAY Cable........
Tested on a BackUPS 600 and SmartUPS 700RM

Someone needs to test a BackUPS Pro........

If main power returns during a shutdown series, "apcupsd" will now reboot.

----> <----

Pavel Alex <pavel@petrolbank.mldnet.com>
Better fix and support for #940-0023A cable, but still broke, drat.

----> <----

Added a delay repeating broadcasts for DELAY_ANNOY seconds
Default is 300 seconds or 5 minutes

DELAY_ANNOY=60

Fixed @222  Soft shutdown and suspend for SmartUPS Only.
            by Werner Panocha, wpanocha@t-online.de
            UPS will go back in 22 hours and 12 minutes!!
            This is a suspend command, and every digit
            counts for 1/10 hour (6 minutes)

Started Debian project

----> <----

Added support for 940-0023A cable.
USE only as a last resort.
There is no way to kill the UPS, nor detect battery status.

TRANSLATION:

YOU CAN BREAK YOUR SYSTEM IF THERE ARE PROBLEMS WITH YOUR UPS's BATTERY.
ALSO, IF YOUR TIMEOUT IS TO LONG.

Please use another cable if possible!!!!!!!!!!!!!!

Full Support for UNIFIX Linux 2.0

----> <----

More UNIFIX additions

----> <----

Brian Schau <bsc@fleggaard.dk>
Adds new code for UNIFIX Linux

----> apcupsd-2.7.1 35902 Sep  3 21:38 apcupsd.c

By Chris:

More changes and fixes with Pro Series tested.
Better method of "walling" the console.
Minor debugging added.

ShareUPS Project Started, BETA.

----> apcupsd-2.7 35413 Aug 14 15:50 apcupsd.c

TESTED on the following setups:

SLACKWARE SMP/NON-SMP (2.1.49) SmartUPS 700 & BackUPS 600
REDHAT 4.1 SMP/NON-SMP (2.0.30) BackUPS 600
SUSE 5.0 SMP/NON-SMP (2.0.30) BackUPS 600

NOTE BackUPS Pro Series Untested at this time of release.

All UPSes can be time limited shutdown.

Fixed all possible killpower problems.....

Changed rc.power to include a NOW command.
Changed inittab file and added 

# What to do when battery power fails.
pn::powerfailnow:/etc/rc.d/rc.power now

Made changes for SuSE distribution.
Change /sbin/init.d/powerfail file

Chanages to CONFIG File

USE_SLACKWARE=yes
USE_REDHAT=no
USE_SUSE=no

Auto Updater

USE_MAKE_UPDATES=yes
USE_MAKE_INITTAB=yes

RedHat version for "powerstatus" file
REDHAT_VERSION=42

----> apcupsd-2.6 35718 Jul 10 08:10 apcupsd.c

Added more info to apcupsd.maunal about special external controls.
SEE "6) OTHER" in apcupsd.maunal

----> apcupsd-2.6.pre6 35718 Jul 10 08:10 apcupsd.c

Made changes in rc.power to become more universial.
Made N-th attempt to make use of APC cable #940-0095A "DRAT"
in "dumb mode".  Will now try "smart mode".
"DOUBLE DRAT" APC has a new cable #940-0095C.
They just want to make things hard for us........
More Pro Fixes

S.u.S.e work around.  This is a major DANGER!!!!
If you call /usr/sbin/apcupsd /dev/apcups killpower,
in a non-power problem situation.
"YOU WILL SCREW YOURSELF" this safe guard is a direct override
of the daemon.

----> apcupsd-2.6.pre5a 36279 Jun 23 16:39 apcupsd.c

OOPS......forgot an "endif" in the distribution Makefile

----> apcupsd-2.6.pre5 36279 Jun 23 16:39 apcupsd.c

Added Linux Flavor Dependencies in CONFIG file.
Fixed "nologin", so only root can login during a powerfailure.
All other attempts are defeated at the login prompt.

----> apcupsd-2.6.pre4 35616 Jun 19 15:40 apcupsd.c

Possible RedHat 4.2 fix.

----> apcupsd-2.6.pre3 35282 Jun 17 01:41 apcupsd.c

Fixes RedHat install and Makefile to make backup of files
to be changed during installation.  Sorry about the mistake
RedHat users.

----> apcupsd-2.6.pre2 35282 Jun 17 01:41 apcupsd.c

More Pro Fixes and stuff for RedHat install and Makefile

----> apcupsd-2.6.pre1 35279 Jun 16 23:04 apcupsd.c

More Pro Fixes and stuff for RedHat install

----> apcupsd-2.5 36558 Jun  1 19:41 apcupsd.c

More Pro Fixes and stuff for RedHat install

----> apcupsd-2.5.pre4 35240 May 23 11:10 apcupsd.c

Can now re-init logfile at boot time with:
/{path}/apcupsd /dev/apcups newlog
This will help keep the log file from eating your root partition

940-0020B retested, fine
940-0095A untested ------!!BETA CODE!!--------

----> apcupsd-2.5.pre3 34829 May 22 17:56 apcupsd.c

major bug check....sorry
tested on smart and back ups, not pro yet
940-0020B not retested yet

----> apcupsd-2.5.pre2 28712 May 21 20:39 apcupsd.c

New logging features.

----> apcupsd-2.5.pre

RedHat Support and Back UPS Pro Support via third party.
Christopher J. Reimer <reimer@doe.carleton.ca>

----> apcupsd-2.4 24409 May 19 14:17 apcupsd.c

Back UPS Pro BETA Support

Karsten Wiborg <4wiborg@informatik.uni-hamburg.de>
Christopher J. Reimer <reimer@doe.carleton.ca>

----> apcupsd-2.3 21278 May 13 10:11 apcupsd.c

I made a patch to your apcupsd that allows it to be configured for
external commands to run (instead of the built-in wall).  This allows me
to set it up to send out a call to my pager when the power fails, among
other things.

Anyway, here it is.
--
Chris Adams - cadams@ro.com

not public release, yet.

----> apcupsd-2.2 21275 May  8 12:53 apcupsd.c

Fixed psuedo procfs type status file in the /etc directory.
Now works for both SMART and DUMB MODE.

Eric Raymond
Redit of apcupsd.manual. 

----> apcupsd-2.1 20926 May  6 23:08 apcupsd.c

Jason Orendorf

"After building and installing the daemon & init files I found
out why you #define the term _ANNOY_ME...it is rather annoying
to get 'wall'ed every 3 seconds - though I do think the
feature is important.  I made a trivial modification to a few
files to make the time delay ('wall' frequency) configurable
at build time.  If you haven't made this change yourself, feel
free to incorporate these changes into your development source
if you think it would be useful to others."

<snip from email>

Eric Raymond

Edited APC_UPSD.README.1ST to apcupsd.manual.
Many thanks.

Finally added psuedo procfs type status file in the /etc directory.
Broken for DUMB MODE

----> apcupsd-2.0 20119 May  2 17:36 apcupsd.c

Name change to help sunsite.unc.edu "keeper" daemon.

Black Cables #940-0024B and #940-0024C are now supported for APC SmartUPS.
They have been tested on an APC SmartUPS SU700/1400RM.  I don't know if
it will work on APC BackUPS Pro Series.......
Tested SmartUPS SU700RM with SMNP PowerNet(tm) card and SmartMode i/o
through DB-9 port with 940-0024B and #940-0024C.  This means that you
can SMNP manage your SmartUPS with SMNP PowerNet(tm) card.  You must have
your own SMNP software..........
Added /proc/apcupsd file for pseudo procfs info.
Deleted announce aka -D_ANNOY_ME, for pseudo procfs info file.

Borrowed a bunch of code for Smart-UPS from Pavel Korensky's apcd.c & apcd.h.

----> Enhanced-APC-UPS v1.9 April 17, 1997

Name change to help sunsite.unc.edu "keeper" daemon.

----> Enhanced_APC_UPS v1.9 Apr 14 15:02 apc_upsd.c

bug found by Tom Kunicki <kunicki@surgery.wisc.edu>
"I still can cause the UPS to shutoff with a power disturbance by
 typing 'apc_upsd /dev/apc_ups killpower' before the disturbance." ... "
 Maybe you could disable the 'killpower' switch except for when the UPS
 is in the condition where it is supposed to be used..."

I have tested my BackUPS cable on the following and it works as designed:
  APC BackUPS 400, APC BackUPS 600, and APC SmartUPS SU1400RM.
I have tested APC's cable #940-0020B on the following and it works as designed:
  APC BackUPS 400, APC BackUPS 600, and APC SmartUPS SU1400RM.
I have NOT tested a BackUPS Pro, since I don't have one yet.

NOTE: My SmartUPS CABLE has broken code and more likely a broken cable design.

Black Cables #940-0024B and #940-0024C are still in the works but not high
priority, yet.

----> Enhanced_APC_UPS v1.8 Apr  8 00:06 apc_upsd.c

Corrected defines in code for 940-0020B, 940-0023A, and 940-0095A.
-D_SMARTUPS can no longer be define if you select USE_APC=yes
There is no SmartUPS "Smart Mode" support, it has never been available.
I am waiting to hear from APC about pinouts for the following cables
#940-0023A their Unix OS cable and
#940-0095A their Win95OS cable.
If you want to use and APC cable, you must have cable #940-0020B ONLY....

Deleted Enhanced_APC_BackUPS.tar.gz from distribution.
Deleted rc.apc_power, all cables use rc.power.

Not publically released at sunsite.unc.edu

----> Enhanced_APC_UPS v1.7g-fix Apr  3 17:42 Makefile

Error in Makefile, rc.apc_power is for testing APC SmartUPS "black" cable.
All installs should use rc.power only,  I forgot to make this change in the
distribution Makefile from the Master Makefile.

----> Enhanced_APC_UPS v1.7g Apr  1 15:31 apc_upsd.c

Possible FULL support for Grey cable APC# 940-0020B for BackUPS only
There still may be a bug like the original Enhanced_APC_BackUPS v1.0,
but has not shown it face yet after FIVE (5) forced power outages.

NO SUPPORT YET: Black cable APC# 940-0024B for SmartUPS & BackUPS PRO only
This is take MUCH longer.......I don't have either SmartUPS & BackUPS PRO to
test with, but can simulate the effect of PIN#8 on a BackUPS 400 and 600.

----> Enhanced_APC_UPS v1.6b Mar 31 22:03 apc_upsd.c

Added USE_APC in CONFIG for using APC cables
Added for using APC cables rc.apc_power
Basic support like original three-wire daemon.

Black cable   APC# 940-0024B for SmartUPS and BackUPS PRO only
Grey cable    APC# 940-0020B for BackUPS only

VERY BETA......REPEAT, VERY BETA!!!!, ALMOST ALPHA........

Not publically released at sunsite.unc.edu

----> Enhanced_APC_UPS v1.5 Mar 31 12:07 apc_upsd.c

Fixed a possible bug on the SmartUPS side of the daemon.
Retested cable design for "Andreas vasquez Mack" <mack@sofo.uni-stuttgart.de>
Soon to test "black APC cable" for "Kofi N. Agyemang" <kna@safenet.net> and
                                   "Pete Rylko" <petese@inwave.com>
APC Black Cable Support soon v1.6?

----> Enhanced_APC_UPS v1.4 Mar  1 17:18 apc_upsd.c

Added ANNOUNCE defeat in CONFIG

----> Enhanced_APC_UPS v1.3 Mar  1 16:41 apc_upsd.c

Fixed bug with momentary power outage less than 10 seconds with announce.
That darn announce.......
Added original Enhanced_APC_BackUPS package to distribution.
Never released........

----> Enhanced_APC_UPS v1.2 Mar  1 15:46 apc_upsd.c

Fixed a loop order bug.
It now release the power problems message after power returns.

----> Enhanced_APC_UPS v1.1 Feb 28 16:25 apc_upsd.c

Copied new dowall.c from Sysvinit-2.69.
Added support for SmartUPS (non-Smart Accessories Port).
Fixed the above problem of repeated power failures.
Added cable design for SmartUPS (non-Smart Accessories Port).

----> Enhanced_APC_BackUPS v1.0 Sep 20 01:52 backupsd.c

Known Bug in special power situations.  IFF (if and only if) the following
happens will this daemon fail to do its job.

If your power fails long enough to cause "backupsd" to activate and cleanly
shutdown your Linux-Box, and returns later only breifly.

( That is long enough for the UPS to clear the KILL_POWER_BIT and causes )
( your Linux-Box to reboot.  But not long enough to recharge you UPS's   )
( battery above the low-battery latch state, and power fails again.  The )
( daemon cannot currently catch the status change.                       )

I have never had this happen to me ever, but I can force this to happen
under laboratory test conditions.  You can too, but it is not advised.
Your filesystem will say "!@#$%#$*&#$@" and much more to you.

Thanks for the support and use of this daemon.

hedrick@astro.dyer.vanderbilt.edu
Andre Hedrick

http://www.brisse.dk/site/apcupsd/
