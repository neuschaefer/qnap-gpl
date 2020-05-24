#
# Makefile.am for dhcpv6 etc/sysconfig subdirectory
# Author: David Cantrell <dcantrell@redhat.com>
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = dhcp6s.sysconfig dhcp6r.sysconfig

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)/sysconfig
	$(install_sh_DATA) dhcp6s.sysconfig $(DESTDIR)$(sysconfdir)/sysconfig/dhcp6s
	$(install_sh_DATA) dhcp6r.sysconfig $(DESTDIR)$(sysconfdir)/sysconfig/dhcp6r
