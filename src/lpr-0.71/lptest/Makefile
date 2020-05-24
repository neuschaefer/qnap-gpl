
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lptest

lptest:	lptest.o

install: lptest
	install -s -m755 lptest $(DESTDIR)/usr/bin
	install -m644 lptest.1 $(DESTDIR)$(MANDIR)/man1

clean:
	rm -f lptest *.o

