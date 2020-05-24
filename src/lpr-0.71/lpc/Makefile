
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lpc

lpc:	lpc.o cmds.o cmdtab.o

install: lpc
	install -s -m2555 lpc $(DESTDIR)/usr/sbin/lpc
	install -m644 lpc.8 $(DESTDIR)$(MANDIR)/man8

clean:
	rm -f lpc *.o

