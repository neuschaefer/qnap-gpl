
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

all:	lpf

lpf:	lpf.o

install: lpf
	install -s lpf $(DESTDIR)/usr/sbin

clean:
	rm -f lpf *.o

