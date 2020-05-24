# Makefile for hdparm

# DESTDIR is for non root installs (eg packages, NFS) only!
DESTDIR =

binprefix = /
manprefix = /usr
exec_prefix = $(binprefix)
sbindir = $(exec_prefix)/sbin
mandir = $(manprefix)/share/man
oldmandir = $(manprefix)/man

#ifndef CC
CC = $(CROSS_COMPILE)gcc
#endif
CFLAGS := -O2 -W -Wall -Wbad-function-cast -Wcast-align -Wpointer-arith -Wcast-qual -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -fkeep-inline-functions -Wwrite-strings -Waggregate-return -Wnested-externs -Wtrigraphs $(CFLAGS)


LDFLAGS = -s
INSTALL = install
INSTALL_DATA = $(INSTALL) -m 644
INSTALL_PROGRAM = $(INSTALL)


ifeq (${TARGET_PLATFORM},X86_SANDYBRIDGE)
MODEL_FLAG := -D$(subst -,,$(MODEL))
MODEL_FLAG += -D$(patsubst %G,%,$(subst -,,$(REAL_MODEL)))
CFLAGS += -D${TARGET_PLATFORM}
endif

ifeq (${TARGET_PLATFORM},X86_CEDAVIEW)
MODEL_FLAG := -D$(subst -,,$(MODEL))
MODEL_FLAG += -D$(patsubst %G,%,$(subst -,,$(REAL_MODEL)))
CFLAGS += -D${TARGET_PLATFORM}
endif

ifeq (${TARGET_PLATFORM},X86_EVANSPORT)
MODEL_FLAG := -D$(subst -,,$(MODEL))
MODEL_FLAG += -D$(patsubst %G,%,$(subst -,,$(REAL_MODEL)))
CFLAGS += -D${TARGET_PLATFORM}
endif

ifeq ($(MODEL),TS-809)
        MODEL_FLAG = -DTS809
endif
ifeq ($(MODEL),TS-609)
        MODEL_FLAG = -DTS609
endif
ifeq ($(MODEL),TS-509)
        MODEL_FLAG = -DTS509
endif
ifeq ($(MODEL),TS-409)
        MODEL_FLAG = -DTS409
endif
ifeq ($(MODEL),TS-201)
        MODEL_FLAG = -DTS201
endif
ifeq ($(MODEL),TS-259)
        MODEL_FLAG = -DTS259
endif
ifeq ($(MODEL),TS-459)
        MODEL_FLAG = -DTS459
endif
ifeq ($(MODEL),TS-559)
        MODEL_FLAG = -DTS559
endif
ifeq ($(MODEL),TS-659)
        MODEL_FLAG = -DTS659
endif
ifeq ($(MODEL),TS-859)
        MODEL_FLAG = -DTS859
endif
ifeq ($(MODEL),TS-419)
	MODEL_FLAG = -DTS419
endif
ifeq ($(MODEL),TS-219)
	MODEL_FLAG = -DTS219
endif
ifeq ($(MODEL),TS-119)
	MODEL_FLAG = -DTS119
endif

ifeq ($(findstring TS-859U,$(REAL_MODEL)),TS-859U)
	MODEL_FLAG += -DTS859U
endif

CFLAGS += ${MODEL_FLAG}
all: hdparm

hdparm: hdparm.o identify.o hdparm.h
	$(CC) $(LDFLAGS) -o hdparm hdparm.o identify.o
 
install: all hdparm.8
	if [ -f $(DESTDIR)$(sbindir)/hdparm ]; then rm -f $(DESTDIR)$(sbindir)/hdparm ; fi
	if [ -f $(DESTDIR)$(mandir)/man8/hdparm.8 ]; then rm -f $(DESTDIR)$(mandir)/man8/hdparm.8 ;\
	elif [ -f $(DESTDIR)$(oldmandir)/man8/hdparm.8 ]; then rm -f $(DESTDIR)$(oldmandir)/man8/hdparm.8 ; fi
	$(INSTALL_PROGRAM) hdparm $(DESTDIR)$(sbindir)/hdparm
	if [ -d $(DESTDIR)$(mandir) ]; then $(INSTALL_DATA) hdparm.8 $(DESTDIR)$(mandir)/man8/hdparm.8 ;\
	elif [ -d $(DESTDIR)$(oldmandir) ]; then $(INSTALL_DATA) hdparm.8 $(DESTDIR)$(oldmandir)/man8/hdparm.8 ; fi

clean:
	rm -f hdparm *.o core

