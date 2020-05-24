#
# Makefile.am for dhcpv6 etc/init.d subdirectory
# Author: David Cantrell <dcantrell@redhat.com>
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = dhcp6r.sh dhcp6s.sh

rc_d_dir = $(DESTDIR)$(sysconfdir)/rc.d/init.d

install-data-local:
	$(MKDIR_P) $(rc_d_dir)
	$(install_sh_SCRIPT) -m 0755 dhcp6s.sh $(rc_d_dir)/dhcp6s
	$(install_sh_SCRIPT) -m 0755 dhcp6r.sh $(rc_d_dir)/dhcp6r
