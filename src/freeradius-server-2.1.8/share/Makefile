#
#	Scripts to format dictionary files.
#
#	$Id: Makefile,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
#
.PHONY: format

#
#  This should only be run by hand, and then sanity checked by hand!
#
format: dictionary*
	@for x in dictionary* ; do \
		cat $$x | ./format.pl > tmp; \
		mv tmp $$x; \
	done
