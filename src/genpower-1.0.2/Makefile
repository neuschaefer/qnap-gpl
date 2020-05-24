########################################
#                                      #
# Makefile for genpowerd.              #
# by Tom Webster <webster@kaiwan.com>  #
# v.1.0.1       1995/07/05             #
# v.1.0.2	2002/03/28             #
#                                      #
######################################## 

BINDIR 		= /sbin
SCRIPTDIR 	= /etc/init.d
NAS_LIB_PATH	= $(shell pwd)/../../NasLib

CC		= ${CROSS_COMPILE}gcc
CFLAGS		= -I/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/include -I/opt/cross-project/ARM/arm-none-linux-gnueabi/arm-none-linux-gnueabi/sys-root/usr/local/include -N -Wall -O6 
INCLUDE 	= -I${NAS_LIB_PATH}/include -I${NAS_LIB_PATH}/uLinux -I${NAS_LIB_PATH}/libnaslog-2.0.0
STARTSHELL	= ups.sh
LIBS 		= -L${ROOT_PATH}/usr/lib -luLinux_NAS -luLinux_cgi -lpthread -luLinux_Storage -luLinux_naslog -lsqlite3 -luLinux_Util -luLinux_config -luLinux_PDC -luLinux_quota

ifeq (${TARGET_PLATFORM},AMCC_PPC_4XXFP)
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

COMMON		= gen_common.o

PROG		= genpowerd gentest gengetstatus

all:		${PROG}

%.o:		%.c
		${CC} -O -Wall -c $(INCLUDE) $<
		
genpowerd:	genpowerd.o ${COMMON}
		${CC} genpowerd.o ${COMMON} ${LIBS} -o $@

gentest:	gentest.o
		${CC} gentest.o -o $@

gengetstatus:	gengetstatus.o ${COMMON}
		${CC} gengetstatus.o ${COMMON} ${LIBS} -o $@

clean:
		rm -f *.o *.bak ${PROG}

install:
		@ echo "Install genpower-1.0.2..."
		@ dos2unix genpowerfail.sh
		@ strip genpowerd
		@ strip gengetstatus
		install --mode=755 genpowerd $(prefix)${BINDIR}
		install --mode=755 gengetstatus $(prefix)${BINDIR}
		install --mode=755 genpowerfail.sh $(prefix)${SCRIPTDIR}

