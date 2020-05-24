#
# Makefile
#
# Version:	$Id: Makefile,v 1.3 2014/12/23 07:26:12 richardchang Exp $
#

TARGET		= opentftpd
BINDIR 		= /sbin
SCRIPTDIR 	= /etc/init.d
NAS_LIB_PATH	= $(shell pwd)/../../NasLib

CFLAGS		= -I${TARGET_PREFIX}/include -I${TARGET_PREFIX}usr/local/include -N -Wall -O6 

all:		$(TARGET)

$(TARGET) :	opentftpd.cpp
		${CXX} opentftpd.cpp -o $@

clean:
		rm -f *.o *.bak
		rm opentftpd

install:
		@ echo "Install tftpd-1.62..."
		install --mode=755 opentftpd $(prefix)${BINDIR}

