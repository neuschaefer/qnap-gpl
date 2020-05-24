Summary: A utility that manages print jobs.
Name: lpr
Version: 0.71
Release: 1
Copyright: distributable
Group: System Environment/Daemons
Source: lpr-%{PACKAGE_VERSION}.tar.gz
Prereq: /sbin/chkconfig
BuildRoot: /var/tmp/%{name}-root

%description
The lpr package provides the basic system utility for managing
printing services.  Lpr manages print queues, sends print jobs to
local and remote printers and accepts print jobs from remote clients.

If you will be printing from your system, you'll need to install the
lpr package.

%prep
%setup -q

%build

%ifarch alpha
# we shouldn't need this with more recent glibc's, but lpq kills remote
# lpd w/o it
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS -Dgetline=get_line"
%else
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"
%endif

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/{bin,sbin}
mkdir -p $RPM_BUILD_ROOT%{_mandir}/{man1,man5,man8}

%ifarch alpha
# we shouldn't need this with more recent glibc's, but lpq kills remote
# lpd w/o it and there seems to be a mistake in lpr's makefiles somewhere
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS -Dgetline=get_line" DESTDIR=$RPM_BUILD_ROOT install
%else
make DESTDIR=$RPM_BUILD_ROOT MANDIR=%{_mandir} install
%endif
chmod -x $RPM_BUILD_ROOT%{_mandir}/*/*

mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d
install -m755 lpd.init $RPM_BUILD_ROOT/etc/rc.d/init.d/lpd
( cd $RPM_BUILD_ROOT
mkdir -p ./etc/rc.d/{rc0.d,rc1.d,rc2.d,rc3.d,rc4.d,rc5.d,rc6.d}
  ln -sf ../init.d/lpd ./etc/rc.d/rc0.d/K60lpd
  ln -sf ../init.d/lpd ./etc/rc.d/rc1.d/K60lpd
  ln -sf ../init.d/lpd ./etc/rc.d/rc2.d/S60lpd
  ln -sf ../init.d/lpd ./etc/rc.d/rc3.d/S60lpd
  ln -sf ../init.d/lpd ./etc/rc.d/rc5.d/S60lpd
  ln -sf ../init.d/lpd ./etc/rc.d/rc6.d/K60lpd
  mkdir -p ./var/spool/lpd
)

%clean
rm -rf $RPM_BUILD_ROOT

%post
test -x /sbin/chkconfig && /sbin/chkconfig --add lpd

%preun
if [ $1 = 0 ]; then
   test -x /sbin/chkconfig && /sbin/chkconfig --del lpd
fi

%files
%defattr(-,root,root)
%attr(6555,root,lp) /usr/bin/lpq
%attr(6555,root,lp) /usr/bin/lpr
%attr(6555,root,lp) /usr/bin/lprm
/usr/bin/lptest
%{_mandir}
%attr(2755,root,lp) /usr/sbin/lpc
/usr/sbin/lpd
/usr/sbin/lpf
/usr/sbin/pac
%attr(0775,root,daemon)	%dir /var/spool/lpd
%config /etc/rc.d/init.d/lpd
%config(missingok) /etc/rc.d/rc0.d/K60lpd
%config(missingok) /etc/rc.d/rc1.d/K60lpd
%config(missingok) /etc/rc.d/rc2.d/S60lpd
%config(missingok) /etc/rc.d/rc3.d/S60lpd
%config(missingok) /etc/rc.d/rc5.d/S60lpd
%config(missingok) /etc/rc.d/rc6.d/K60lpd

%changelog
* Thu Jan 11 2001 Ben Woodard <ben@valinux.com>
- Made it install the man pages relative to _mandir so that it would work 
- with other distros

* Mon Oct  2 2000 Crutcher Dunnavant <crutcher@redhat.com>
- Fixed bug #11740
- Which was causing deadlocks in the queue in some situations.
- Still possible, though much less likely.

* Mon Sep 25 2000 Crutcher Dunnavant <crutcher@redhat.com>
- Fixed a format string bug in lpd/printjob.c

* Wed Aug 16 2000 Crutcher Dunnavant <crutcher@redhat.com>
- fixed some evil in lpd/printjob.c that treated unspeced command lines
- in the control file as print job files. (print was the default case)
- this keeps lpd from screaming error messages/email when smatter lpd
- servers talk to it.

* Mon Feb 14 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Fix interoperability with MacOS client (Bug #7593)

* Thu Feb  3 2000 Bernhard Rosenkraenzer <bero@redhat.com>
- Remove executable bit from lpq man page (Bug #9035)
- deal with rpm compressing man pages

* Mon Jan 24 2000 Bill Nottingham <notting@redhat.com>
- you're saying core dumps in the spooldir are a bad idea?

* Tue Jan 18 2000 Bill Nottingham <notting@redhat.com>
- *sigh*

* Thu Jan  6 2000 Bill Nottingham <notting@redhat.com>
- more security fixes

* Tue Nov  7 1999 Bill Nottingham <notting@redhat.com>
- add a 'custom' parameter 'Z' to cf files (GOMEZ@slib.fr)

* Fri Oct 22 1999 Bill Nottingham <notting@redhat.com>
- oops. slightly overzealous in dropping ids.

* Tue Oct 19 1999 Bill Nottingham <notting@redhat.com>
- fix a stupid typo.

* Fri Oct 15 1999 Bill Nottingham <notting@redhat.com>
- fix some security issues.

* Fri Oct  8 1999 Bill Nottingham <notting@redhat.com>
- squash multiple copies bug, hopefully permanently...

* Fri Sep 10 1999 Bill Nottingham <notting@redhat.com>
- chkconfig --del on %preun, not %postun

* Mon Aug 30 1999 Bill Nottingham <notting@redhat.com>
- fix suppress headers on remote printers

* Mon Aug 16 1999 Bill Nottingham <notting@redhat.com>
- initscript munging

* Mon Aug  2 1999 Bill Nottingham <notting@redhat.com>
- fix multiple copy remote printing

* Thu Jul 29 1999 Bill Nottingham <notting@redhat.com>
- add lp group when exec()ing filter, so .config file with passwords
  can be non-world-readable...

* Mon Jul 12 1999 Bill Nottingham <notting@redhat.com>
- fix possible core dumps in lpc (ms@bee.de, #3776)

* Wed Jun 23 1999 Bill Nottingham <notting@redhat.com>
- add lp= capability support (#3683)

* Thu Jun 17 1999 Bill Nottingham <notting@redhat.com>
- fixes for job removal (#2676)

* Wed Jun  2 1999 Jeff Johnson <jbj@redhat.com>
- fix for RS capability (#3217)

* Mon May 10 1999 Bill Nottingham <notting@redhat.com>
- RFC 1179 compliance actually works now...

* Mon Apr 26 1999 Bill Nottingham <notting@redhat.com>
- make lpr RFC 1179 compliant

* Mon Mar 22 1999 Bill Nottingham <notting@redhat.com>
- increase buffer length for filenames (bug #1676)

* Fri Mar 19 1999 Bill Nottingham <notting@redhat.com>
- change man page to show that -r -s is not supported (bug #717)

* Mon Feb 15 1999 Bill Nottingham <notting@redhat.com>
- security patch from Chris Evans
- fix for remote but not local users (originally from Kevin Sochacki)

* Mon Feb  8 1999 Bill Nottingham <notting@redhat.com>
- build for 6.0 tree

* Thu Oct  1 1998 Bill Nottingham <notting@redhat.com>
- don't ignore SIGCHLD in filters

* Mon Aug  3 1998 Jeff Johnson <jbj@redhat.com>
- build root.

* Fri Jun 26 1998 Jeff Johnson <jbj@redhat.com>
- bring printjob up-to-date (fix problem #564)

* Thu Apr 23 1998 Michael K. Johnson <johnsonm@redhat.com>
- enhanced initscript

* Thu Apr 23 1998 Erik Troan <ewt@redhat.com>
- included new rmjob security fix from BSD

* Sat Apr 18 1998 Erik Troan <ewt@redhat.com>
- included rmjob patches from BSD

* Fri Feb 27 1998 Otto Hammersmith <otto@redhat.com>
- increased buffer for hostname from 32 to 1024, plenty big enough now.

* Wed Oct 29 1997 Donnie Barnes <djb@redhat.com>
- added chkconfig support
- changed the initscript name from lpd.init to lpd (all links, too)

* Mon Oct 27 1997 Michael Fulbright <msf@redhat.com>
- Fixed print filters to change to printer's UID so root-squashing wont bite us

* Wed Oct  8 1997 Michael Fulbright <msf@redhat.com>
- Fixed nasty error in getprent() and forked lpd's in startup() which
  caused the printcap file to be read incorrectly.
- added #include <string.h> as needed to make compile cleaner.  

* Thu Jul 10 1997 Erik Troan <ewt@redhat.com>
- changes for glibc 2.0.4

* Tue Apr 22 1997 Michael Fulbright <msf@redhat.com>
- moved to v. 0.17, then 0.18 (!)
- Fixed bug on Alpha/glibc when printing to a remote queue via a filter

* Fri Mar 28 1997 Michael Fulbright <msf@redhat.com>
- Moved version up to 0.16
- Added input filter support for remote queues

* Wed Mar 05 1997 Erik Troan <ewt@redhat.com>
- Incorporated filter patch into main sources
- Removed RCS logs from source tar file
- Added patched from David Mosberger to fix __ivaliduser on Alpha's
- added -Dgetline=get_line for old glibcs (this means alpha)
