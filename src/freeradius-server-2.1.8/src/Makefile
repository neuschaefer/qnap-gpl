#
# Makefile
#
# Version:	$Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
#

include ../Make.inc

SUBDIRS		= include lib modules main
WHAT_TO_MAKE	= all

all: freeradius-devel
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	@rm -f include/*~ *~

install:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

common:
	@for dir in $(SUBDIRS); do \
		echo "Making $(WHAT_TO_MAKE) in $$dir..."; \
		$(MAKE) $(MFLAGS) -C $$dir $(WHAT_TO_MAKE) || exit $$?; \
	done

freeradius-devel:
	ln -s include freeradius-devel
