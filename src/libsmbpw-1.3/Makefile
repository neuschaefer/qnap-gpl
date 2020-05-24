CC=gcc
#CFLAGS=-O2 -mpentium -fomit-frame-pointer	# pentium cpu (gcc 2.9.x)
CFLAGS=-O2		# anything else
#CFLAGS=-g		# debugging
VERSION=1.3

OS=LINUX # SOLARIS

OBJS = smbpwd.o md4.o smbdes.o smbencrypt.o 
SH_OBJS = smbpwd.po md4.po smbdes.po smbencrypt.po

.SUFFIXES: .c .o .po

.c.o:
	${CC} -Wall ${CFLAGS} -D${OS} -c $< -o $@

.c.po:
	${CC} -Wall ${CFLAGS} -fPIC -D${OS} -c $< -o $@

all:  libsmbpw.a libsmbpw.so.${VERSION} testsmb smbcrypt

testsmb:  libsmbpw.a testsmb.c 
	${CC} -g -o testsmb testsmb.c -L. -lsmbpw

smbcrypt:  libsmbpw.a smbcrypt.c 
	${CC} -g -o smbcrypt smbcrypt.c -L. -lsmbpw
       
libsmbpw.a:	 ${OBJS}
	ar -rs $@ ${OBJS}

#libsmbpw.a:
#	${CC} -Wall ${CFLAGS} -D${OS} -c smbpwd.c -o smbpwd.o
#	ar -rs $@ smbpwd.o

libsmbpw.so.${VERSION}:	${SH_OBJS}
	${CC} -shared -o libsmbpw.${VERSION}.so ${SH_OBJS}
	ln -fs libsmbpw.${VERSION}.so libsmbpw.so

#libsmbpw.so.${VERSION}:
#	${CC} -Wall ${CFLAGS} -fPIC -D${OS} -c smbpwd.c -o smbpwd.po
#	${CC} -shared smbpwd.po -o $@

clean: 
	rm -f *.o *.po *~ libsmbpw.a libsmbpw.${VERSION}.so libsmbpw.so core testsmb smbcrypt
