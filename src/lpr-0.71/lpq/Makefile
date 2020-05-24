
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lpq

lpq:	lpq.o

install: lpq
	install -s -m6555 lpq $(DESTDIR)$(bindir)/usr/bin
	install lpq.1 $(DESTDIR)$(MANDIR)/man1

clean:
	rm -f lpq *.o

