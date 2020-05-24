
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lprm

lprm:	lprm.o

install: lprm
	install -s -m6555 lprm $(DESTDIR)$(bindir)/usr/bin
	install -m644 lprm.1 $(DESTDIR)$(MANDIR)/man1

clean:
	rm -f lprm *.o

