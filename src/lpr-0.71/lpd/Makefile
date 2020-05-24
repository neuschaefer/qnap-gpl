
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd -I../../../System/lib/include
LDFLAGS	= 
LDLIBS = -L ../common_source -lcommon 

all:	lpd

lpd:	lpd.o lpdchar.o printjob.o recvjob.o

install: lpd
	install -s -m744 lpd $(DESTDIR)/usr/sbin
	install -m644 lpd.8 $(DESTDIR)$(MANDIR)/man8

clean:
	rm -f lpd *.o

