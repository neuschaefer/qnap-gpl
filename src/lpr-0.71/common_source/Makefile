
CFLAGS	= -Wall $(RPM_OPT_FLAGS) -I../common_source -I/usr/include/bsd
LDFLAGS	= 
LDLIBS = -lcommon -L ../common_source # -lbsd

OBJS	= common.o displayq.o printcap.o rmjob.o startdaemon.o

all:	$(OBJS)
	@ar rcv libcommon.a $(OBJS)
	@ranlib libcommon.a

install: all

clean:
	rm -f *.o *.a

