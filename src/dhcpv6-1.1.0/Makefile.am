#
# Makefile.am for dhcpv6
# Author: David Cantrell <dcantrell@redhat.com>
#

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = include src man etc

EXTRA_DIST = AUTHORS LICENSE README TODO ChangeLog \
             RFC3315 RFC3633 RFC3646 RFC4242

MAINTAINERCLEANFILES = Makefile.in config.guess config.h.in config.sub \
                       depcomp install-sh ltmain.sh missing ABOUT-NLS \
                       INSTALL aclocal.m4 configure ylwrap $(DIST_ARCHIVES)

CLEANFILES = *~ ChangeLog

MOSTLYCLEANDIRS = m4

dist-hook:
	rm -rf `find $(distdir) -type f -name .gitignore`

ChangeLog:
	(GIT_DIR=.git git log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty ChangeLog.' >&2)

tag:
	@if [ -z "$(GPGKEY)" ]; then \
		echo "GPGKEY environment variable missing, please set this to the key ID" ; \
		echo "you want to use to tag the repository." ; \
		exit 1 ; \
	fi
	@git tag -u $(GPGKEY) -m "Tag as $(PACKAGE)-$(VERSION)" -f $(PACKAGE)-$(VERSION)
	@echo "Tagged as $(PACKAGE)-$(VERSION) (GPG signed)"

release: dist-gzip
	rm -rf $(PACKAGE)-$(VERSION)
	gzip -dc $(PACKAGE)-$(VERSION).tar.gz | tar -xvf -
	( cd $(PACKAGE)-$(VERSION) && ./configure && make ) || exit 1
	@echo
	@echo "$(PACKAGE)-$(VERSION).tar.gz is now ready to upload."
	@echo "Don't forget to tag this release with 'make tag' and push."
	@echo
