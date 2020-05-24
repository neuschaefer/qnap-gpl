
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	pac

pac:	pac.o

install: pac
	install -s -m744 pac $(DESTDIR)/usr/sbin
	install -m644 pac.8 $(DESTDIR)$(MANDIR)/man8

clean:
	rm -f pac *.o

