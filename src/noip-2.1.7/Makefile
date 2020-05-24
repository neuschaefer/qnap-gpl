TGT=noip2
CC=${CROSS_COMPILE}gcc

PREFIX=/usr/local
CONFDIR=${PREFIX}/etc
BINDIR=${PREFIX}/bin

# these defines are for Linux
INCLUDES = -I${NAS_LIB_PATH}/include
LIBS = -luLinux_Util -luLinux_config -luLinux_PDC -luLinux_NAS -luLinux_quota -luLinux_Storage -luLinux_naslog -lsqlite3 -L${ROOT_PATH}/usr/lib
ARCH=linux

# for Mac OS X and BSD systems that have getifaddr(), uncomment the next line
#ARCH=bsd_with_getifaddrs

# for early BSD systems without getifaddrs(), uncomment the next line
#ARCH=bsd
ifeq (${TARGET_PLATFORM},AMCC_PPC_4XXFP)
INCLUDE += -I${KERNEL_PATH}/include/qnap
LIBS += -luLinux_Platform
endif

ifeq (${LDAP},yes)
ifeq ($(TARGET_PLATFORM), ARM_MS)
LIBS += -L/opt/cross-project/arm/mindspeed/target-arm_v7-a_glibc-2.14.1_eabi/usr/lib/ -liconv
LIBS += -L${ROOT_PATH}/lib -lssl -lcrypt -lcrypto -llber -lldap -luLinux_cgi
else
LIBS += -L${ROOT_PATH}/lib -lssl -lcrypt -liconv -lcrypto -llber -lldap -luLinux_cgi
endif
endif

ifeq (${QNAP_HAL_SUPPORT},yes)
LIBS += -luLinux_ini -luLinux_hal
CFLAGS += -DQNAP_HAL_SUPPORT
endif

# for solaris, uncomment the next two lines
# LIBS=-lsocket -lnsl
# ARCH=sun

${TGT}: Makefile ${TGT}.c 
	${CC} -Wall -g -O2 -D${ARCH} -D_QNAP_ -DPREFIX=\"${PREFIX}\" ${TGT}.c ${INCLUDES} -o ${TGT} ${LIBS}

install: ${TGT} 
	if [ ! -d ${BINDIR} ]; then mkdir -p ${BINDIR};fi
	if [ ! -d ${CONFDIR} ]; then mkdir -p ${CONFDIR};fi
	cp ${TGT} ${BINDIR}/${TGT}
	${BINDIR}/${TGT} -C -c /tmp/no-ip2.conf
	mv /tmp/no-ip2.conf ${CONFDIR}/no-ip2.conf

package: ${TGT}
	rm  -f *.bak
	mv ${TGT} binaries/${TGT}-`uname -s`-32bit
	rcp gemini:/local/src/noip-2.0/${TGT} binaries/${TGT}-`uname -s`-64bit
	cd ..; tar zcvf /tmp/noip-2.1.tgz noip-2.0/*

clean: 
	rm -f *o
	rm -f ${TGT}
	rm -f binaries/*
