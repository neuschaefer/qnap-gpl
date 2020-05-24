CC=${TARGET}-gcc
CFLAGS=-Wall -W -Wmissing-prototypes -D_REENTRANT -DQNAPNAS -D_FILE_OFFSET_BITS=64
INSTLIBDIR=/usr/local/lib
SYSCONFFILE=/etc/libtrash.conf

ifeq (${RECYCLE_EX},yes)
CFLAGS += -DRECYCLE_EX
else
	ifeq (${IMAGE_SHARE},yes)
		CFLAGS += -DIMAGE_SHARE
	endif
	INCLUDES = -I${NAS_LIB_PATH}/include
	LIBS = -luLinux_Util -luLinux_config -luLinux_PDC -luLinux_NAS -luLinux_quota -luLinux_Storage -luLinux_naslog -lsqlite3 -L${ROOT_PATH}/usr/lib

	ifeq (${LDAP},yes)
	LIBS += -L${ROOT_PATH}/lib -lssl -lcrypt -liconv -lcrypto -llber -lldap -luLinux_cgi
	endif

	ifeq (${QNAP_HAL_SUPPORT},yes)
	LIBS += -L${ROOT_PATH}/lib -luLinux_ini -luLinux_hal
	CFLAGS += -DQNAP_HAL_SUPPORT
	endif
endif

MAJOR =2
VERSION =2.4

all: libtrash

install: install-libtrash 

*.o: Makefile

SRC= main.c helpers.c unlink.c ## rename.c ## open-funs.c
OBJ= main.o helpers.c unlink.o ## rename.o ## open-funs.o

libtrash: libtrash.so.${VERSION}

ifeq (${RECYCLE_EX},yes)
libtrash.so.${VERSION}: $(SRC) ../libtrash.conf
	cp ../libtrash.conf.ex ../libtrash.conf
	perl -w ../scripts/trimheader.pl
	perl -w ../scripts/genheader.pl
	perl -w ../scripts/genconf.pl
	$(CC) $(CFLAGS) $(SRC) -nostartfiles -shared -fPIC -Wl,-soname,libtrash.so.${MAJOR} \
	-o libtrash.so.${VERSION}  -ldl -lm
else
libtrash.so.${VERSION}: $(SRC) ../libtrash.conf
	perl -w ../scripts/trimheader.pl
	perl -w ../scripts/genheader.pl
	perl -w ../scripts/genconf.pl
	$(CC) $(CFLAGS) $(SRC) -nostartfiles -shared -fPIC -Wl,-soname,libtrash.so.${MAJOR} \
        ${INCLUDES} -o libtrash.so.${VERSION} ${LIBS} -ldl -lm
endif

install-libtrash: 
	TRASH_OFF=YES install libtrash.so.${VERSION} ${INSTLIBDIR}/libtrash.so.${VERSION}
	TRASH_OFF=YES ln -sf libtrash.so.${VERSION} ${INSTLIBDIR}/libtrash.so
	TRASH_OFF=YES install libtrash.conf.sys --mode a=r ${SYSCONFFILE}
	ldconfig	

clean:
	rm -f *~
	rm -f ../*~
	rm -f ../scripts/*~
	rm -f ../cleanTrash/*~
	rm -f *.o
	rm -f libtrash.so.${VERSION}	
	perl -w ../scripts/trimheader.pl
	rm -f libtrash.conf.sys

