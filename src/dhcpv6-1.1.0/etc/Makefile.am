#
# Makefile.am for dhcpv6 etc subdirectory
# Author: David Cantrell <dcantrell@redhat.com>
#

SUBDIRS = sysconfig init.d

EXTRA_DIST = dhcp6c.conf dhcp6s.conf

MAINTAINERCLEANFILES = Makefile.in

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)
	$(install_sh_DATA) dhcp6s.conf $(DESTDIR)$(sysconfdir)/dhcp6s.conf
	$(install_sh_DATA) dhcp6c.conf $(DESTDIR)$(sysconfdir)/dhcp6c.conf
