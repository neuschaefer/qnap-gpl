# Generated automatically from Makefile.in by configure.
# Makefile.in for thttpd
#
# Copyright ?1995,1998 by Jef Poskanzer <jef@acme.com>.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

# Various configurable paths (remember to edit Makefile.in, not Makefile)

# Pathname of directory to install the binary.
BINDIR = /usr/local/sbin
# Pathname of directory to install the man page.
MANDIR = ${ROOT_PATH}/man
# Pathname of directory to install the CGI programs.
WEBDIR = $(ROOT_PATH)/www

# CONFIGURE: The group that the web directory belongs to.  This is so that
# the makeweb program can be installed set-group-id to that group, and make
# subdirectories.  If you're not going to use makeweb, ignore this.
WEBGROUP =	www

# CONFIGURE: Directory for CGI executables.
CGIBINDIR =	$(WEBDIR)/cgi-bin

# You shouldn't need to edit anything below here.
NAS_LIB_PATH = ${shell pwd}/../../../NasLib
COM_LIB_PATH = ${shell pwd}/../../../ComLib
#INCLUDE = -I$(LIB_PATH)/NAS -I$(LIB_PATH)/config -I$(LIB_PATH)/include
INCLUDE = -I$(NAS_LIB_PATH)/include -I${NAS_LIB_PATH}/libms-1.0.0 -I${NAS_LIB_PATH}/libnaslog-2.0.0 -I$(NAS_LIB_PATH)/../Include -I$(COM_LIB_PATH)/include
CC =		${CROSS_COMPILE}gcc
CCOPT =		-O
DEFS =		-DHAVE__PROGNAME=1 -DHAVE_FCNTL_H=1 -DHAVE_GRP_H=1 -DHAVE_MEMORY_H=1 -DHAVE_PATHS_H=1 -DHAVE_POLL_H=1 -DHAVE_SYS_POLL_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_DIRENT_H=1 -DHAVE_LIBCRYPT=1 -DHAVE_STRERROR=1 -DHAVE_WAITPID=1 -DHAVE_VSNPRINTF=1 -DHAVE_DAEMON=1 -DHAVE_SETSID=1 -DHAVE_GETADDRINFO=1 -DHAVE_GETNAMEINFO=1 -DHAVE_GAI_STRERROR=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_POLL=1 -DHAVE_TM_GMTOFF=1 -DHAVE_SOCKADDR_STORAGE=1 -DHAVE_SOCKADDR_IN6=1 -DHTTP_LIVE_STREAMING -DDYNAMIC_HTTP_STREAM 
INCLS =		-I. $(INCLUDE)

DEFS  += -DQNAP_SERVICE_BINDING

ifeq ($(NSS_V2),yes)
	DEFS  += -DNSS_V2
	DEFS  += -DMAX_NVR_HTTP_STREAM_NUM=$(MAX_NVR_HTTP_STREAM_NUM)
else
	ifeq ($(VIOSTOR_SUPPORT),yes)
		DEF_BY_MODEL= -DMAX_NVR_HTTP_STREAM_NUM=$(MAX_NVR_HTTP_STREAM_NUM)
		DEFS  += ${DEF_BY_MODEL} -D_VIOSTOR_
	else
		DEFS  += -DMAX_NVR_HTTP_STREAM_NUM=8
	endif
endif

ifeq ($(NSS_UI3),yes)
	DEFS += -DNSS_UI3
endif

ifeq ($(NVR_DEBUG),yes)
	DEFS += -DNVR_DEBUG
endif

ifeq ($(QTS_HA),yes)
	DEFS += -DQTS_HA
endif

DEFS += -D$(TARGET_PLATFORM)

ifeq ($(UIV2),yes)
	CFLAGS =        $(CCOPT) $(DEFS) -D_UIV2_ $(INCLS)
else
	CFLAGS =	$(CCOPT) $(DEFS) $(INCLS)
endif


LDFLAGS =	
LIBS =		-L${ROOT_PATH}/usr/lib  -L${TARGET_PREFIX}/lib -lcrypt -L${TARGET_PREFIX}/usr/lib -luLinux_NAS -luLinux_config -luLinux_Util -luLinux_Storage -luLinux_quota -luLinux_PDC -luLinux_statistics -luLinux_ms -luLinux_naslog -lsqlite3 -L$(COM_LIB_PATH)/lib -ldl -lpthread

ifeq ($(VIOSTOR_SUPPORT),yes)
LIBS += -lutils -lsmd -lmsq -lvos -lgmd
endif

ifeq (${TARGET_PLATFORM},AMCC_PPC_4XXFP)
LIBS += -luLinux_Platform
endif

ifeq (${LDAP},yes)
LIBS += -L${ROOT_PATH}/lib -lssl -lcrypt -lcrypto -luLinux_cgi
endif

ifeq (${RECYCLE_EX},yes)
CFLAGS += -DRECYCLE_EX
endif

ifeq (${QNAP_HAL_SUPPORT},yes)
LIBS += -luLinux_ini -luLinux_hal
CFLAGS += -DQNAP_HAL_SUPPORT
endif

ifeq ($(QLICENSE_KEY),yes)
LIBS += -luLinux_qlicense -luLinux_nvrlc -lxml2 -lz -liconv
endif

#LIBS =		-lcrypt
NETLIBS =	
INSTALL =	/usr/bin/install -c

HTTPD_PATH = /home/httpd
ERROR_PATH = $(HTTPD_PATH)/errors
ifeq ($(UIV2),yes)
ERROR_SOURCE = errors2/error_thttpd.html
LIBS += -luLinux_nasauth
else
ERROR_SOURCE = errors/error_thttpd.html
endif
ifeq ($(QOS4),yes)
ERROR_SOURCE = errors3/error_thttpd.html
endif

ifeq (${TARGET_PLATFORM}, ARM_ST)
LIBS += -lpthread	
endif

ifeq (${TARGET_PLATFORM}, ARM_AL)
LIBS += -lpthread	
endif

ifeq (${TARGET_PLATFORM}, ARMADA_370)
LIBS += -lpthread
endif

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

SRC =		thttpd.c libhttpd.c fdwatch.c mmc.c timers.c match.c tdate_parse.c syslog.c

OBJ =		$(SRC:.c=.o)

ALL =		thttpd

GENHDR =	mime_encodings.h mime_types.h

CLEANFILES =	$(ALL) $(OBJ) $(GENSRC) $(GENHDR)

SUBDIRS =	cgi-src extras

all:		this subdirs
this:		$(ALL)

thttpd: $(OBJ)
	@rm -f $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(NETLIBS)

libhttpd.o:		mime_encodings.h
mime_encodings.h:	mime_encodings.txt
	rm -f mime_encodings.h
	sed < mime_encodings.txt > mime_encodings.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", "/' -e 's/^/{ "/' -e 's/$$/" },/'

libhttpd.o:	mime_types.h
mime_types.h:	mime_types.txt
	rm -f mime_types.h
	sed < mime_types.txt > mime_types.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", "/' -e 's/^/{ "/' -e 's/$$/" },/'


subdirs:
	for i in $(SUBDIRS) ; do ( \
	    cd $$i ; \
	    pwd ; \
	    $(MAKE) $(MFLAGS) \
		WEBDIR=$(WEBDIR) \
		CGIBINDIR=$(CGIBINDIR) \
		MANDIR=$(MANDIR) \
		WEBGROUP=$(WEBGROUP) \
	) ; done


install:
#installthis install-man installsubdirs
	./install.sh $(ROOT_PATH)$(ERROR_PATH)
#${CROSS_COMPILE}strip thttpd
	install -o root -g root -m 644 thttpd.conf ${ROOT_PATH}/etc
	install -o root -g root -m 644 ${ERROR_SOURCE} ${ROOT_PATH}${ERROR_PATH}
#	install -o root -g root -m 755 thttpd $(ROOT_PATH)$(BINDIR)
	sudo cp thttpd ${ROOT_PATH}${BINDIR}/_thttpd_
	sudo ${CROSS_COMPILE}strip ${ROOT_PATH}${BINDIR}/_thttpd_
	sudo chown root.root ${ROOT_PATH}${BINDIR}/_thttpd_
	cd ${ROOT_PATH}${BINDIR}; \
	sudo ln -sf _thttpd_ wfm_thttpd


installthis:
	$(INSTALL) -m 555 -o bin -g bin thttpd $(DESTDIR)$(BINDIR)

install-man:
	$(INSTALL) -m 444 -o bin -g bin thttpd.8 $(DESTDIR)$(MANDIR)/man8

installsubdirs:
	for i in $(SUBDIRS) ; do ( \
	    cd $$i ; \
	    pwd ; \
	    $(MAKE) $(MFLAGS) \
		WEBDIR=$(WEBDIR) \
		CGIBINDIR=$(CGIBINDIR) \
		MANDIR=$(MANDIR) \
		WEBGROUP=$(WEBGROUP) \
		install \
	) ; done


clean:		cleansubdirs
	rm -f $(CLEANFILES)

distclean:	distcleansubdirs
	rm -f $(CLEANFILES) Makefile config.cache config.log config.status tags

cleansubdirs:
	for i in $(SUBDIRS) ; do ( \
	    cd $$i ; \
	    pwd ; \
	    $(MAKE) $(MFLAGS) clean \
	) ; done

distcleansubdirs:
	for i in $(SUBDIRS) ; do ( \
	    cd $$i ; \
	    pwd ; \
	    $(MAKE) $(MFLAGS) distclean \
	) ; done

tags:
	ctags -wtd *.c *.h

tar:
	@name=`sed -n -e '/SERVER_SOFTWARE/!d' -e 's,.*thttpd/,thttpd-,' -e 's, .*,,p' version.h` ; \
	  rm -rf $$name ; \
	  mkdir $$name ; \
	  tar cf - `cat FILES` | ( cd $$name ; tar xfBp - ) ; \
	  chmod 644 $$name/Makefile.in $$name/config.h $$name/mime_types.txt ; \
	  chmod 755 $$name/cgi-bin $$name/cgi-src $$name/contrib $$name/contrib/redhat-rpm $$name/extras $$name/scripts ; \
	  tar cf $$name.tar $$name ; \
	  rm -rf $$name ; \
	  gzip $$name.tar

thttpd.o:	config.h version.h libhttpd.h fdwatch.h mmc.h timers.h match.h
libhttpd.o:	config.h version.h libhttpd.h mime_encodings.h mime_types.h \
		mmc.h timers.h match.h tdate_parse.h
fdwatch.o:	fdwatch.h
mmc.o:		mmc.h
timers.o:	timers.h
match.o:	match.h
tdate_parse.o:	tdate_parse.h


test:
	/bin/cp thttpd /usr/local/sbin -f
	echo -n "Restart thttpd services:" 
	./thttpd
	kill -9 `pidof thttpd`
	rm -f /var/lock/subsys/thttpd
	/usr/local/sbin/thttpd -nor -nos -u root -l /var/log/thttpd.log -d /home/httpd -c '**.*'
	/bin/touch /var/lock/subsys/thttpd
	echo "."
		
