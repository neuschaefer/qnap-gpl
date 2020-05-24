# $Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $

include ../Make.inc

all:

radwatch:
	$(INSTALL) -m 755 radwatch		$(R)$(sbindir)

install:
	$(INSTALL) -m 755 rc.radiusd		$(R)$(sbindir)
	$(INSTALL) -m 755 radsqlrelay		$(R)$(bindir)
	$(INSTALL) -m 755 radwatch		$(R)$(sbindir)
	$(INSTALL) -m 755 raddebug		$(R)$(sbindir)

clean:
