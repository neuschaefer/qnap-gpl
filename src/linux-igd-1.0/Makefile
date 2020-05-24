PREFIX=/usr
LIBUPNP_PREFIX=/usr
#LIBIPTC_PREFIX=/usr

CC=${CROSS_COMPILE}gcc
INCLUDES= -I${TARGET_PREFIX}/usr/include -I$(LIBUPNP_PREFIX)/include -I../include
LIBS= -lpthread -lupnp -lixml -lthreadutil -L$(LIBUPNP_PREFIX)/lib -L${TARGET_PREFIX}/usr/lib -L../libs
FILES= main.o gatedevice.o pmlist.o util.o config.o

CFLAGS += -Wall -g -O2

ifdef HAVE_LIBIPTC
ifdef LIBIPTC_PREFIX
LIBS += -L$(LIBIPTC_PREFIX)/lib
INCLUDES += -I$(LIBIPTC_PREFIX)/include
endif

LIBS += -liptc
INCLUDES += -DHAVE_LIBIPTC
FILES += iptc.o
endif

all: upnpd

upnpd: $(FILES)
	$(CC) $(CFLAGS) $(FILES) $(LIBS) -o $@
	@echo "make $@ finished on `date`"

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -D_GNU_SOURCE -c $<

clean:
	rm -f *.o upnpd

install: upnpd
	install -d $(PREFIX)/etc/linuxigd
	install etc/gatedesc.xml $(PREFIX)/etc/linuxigd
	install etc/gateconnSCPD.xml  $(PREFIX)/etc/linuxigd
	install etc/gateicfgSCPD.xml $(PREFIX)/etc/linuxigd
	install etc/dummy.xml $(PREFIX)/etc/linuxigd
	install -d $(PREFIX)/usr/sbin
	install upnpd $(PREFIX)/usr/sbin/upnpd
	install upnpd.8 $(PREFIX)/share/man/man8
	if [ ! -f /etc/upnpd.conf ]; then install etc/upnpd.conf $(PREFIX)/etc; fi
