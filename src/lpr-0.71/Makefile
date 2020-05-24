.EXPORT_ALL_VARIABLES:

VERSION=$(shell awk '/^Version:/ { print $$2 }' lpr.spec)
CVSTAG = r$(subst .,-,$(VERSION))

CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

SUBDIRS	= common_source filters lpc lpd lpq lpr lprm lptest pac

all:
	for i in $(SUBDIRS); do make -C $$i all; done

install:
	for i in $(SUBDIRS); do make -C $$i install; done
	install -m644 printcap.5 $(DESTDIR)$(MANDIR)/man5/

clean:
	for i in $(SUBDIRS); do make -C $$i clean; done
	find . -name "*~" -exec rm \{\} \;

archive:
	@rm -rf /tmp/lpr-$(VERSION) /tmp/lpr
	@cd /tmp; cvs -d $(CVSROOT) export -r$(CVSTAG) lpr || :
	@mv /tmp/lpr /tmp/lpr-$(VERSION)
	@dir=$$PWD; cd /tmp; tar cvzf $$dir/lpr-$(VERSION).tar.gz lpr-$(VERSION)
	@rm -rf /tmp/lpr-$(VERSION)
	@echo "The archive is in lpr-$(VERSION)"
