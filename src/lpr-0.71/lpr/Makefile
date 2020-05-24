
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lpr

lpr:	lpr.o

install: lpr
	install -s -m6555 lpr $(DESTDIR)/usr/bin
	install -m644 lpr.1 $(DESTDIR)$(MANDIR)/man1

clean:
	rm -f lpr *.o

