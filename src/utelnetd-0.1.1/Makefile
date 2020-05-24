PROGS     = utelnetd

INSTDIR   = $(prefix)/bin/
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS = utelnetd.o

QNAP_LIBPATH = ../../../NasLib
NAS_INCLUDES = -I$(QNAP_LIBPATH)/NAS -I$(QNAP_LIBPATH)/include -I$(QNAP_LIBPATH)/statistics -I${QNAP_LIBPATH}/libnaslog-2.0.0
NAS_LIBS =  -L${ROOT_PATH}/usr/lib -luLinux_Util -luLinux_config -luLinux_PDC -luLinux_NAS -luLinux_quota -luLinux_Storage -luLinux_naslog -lsqlite3 -luLinux_statistics

ifeq (${TARGET_PLATFORM},AMCC_PPC_4XXFP)
NAS_LIBS += -luLinux_Platform
endif

ifeq (${LDAP},yes)
NAS_LIBS += -L${ROOT_PATH}/lib -lssl -lcrypt -lcrypto -luLinux_cgi
endif

ifeq (${QNAP_HAL_SUPPORT},yes)
NAS_LIBS += -luLinux_hal -luLinux_ini
CFLAGS += -DQNAP_HAL_SUPPORT
endif

all: $(PROGS)

CFLAGS   += -DSHELLPATH=\"$(run_prefix)/bin/login\" $(NAS_INCLUDES)
CFLAGS   += -DQNAP_SERVICE_BINDING

ifneq ('',$(DEBUG))
CFLAGS   += -DDEBUG
endif

$(PROGS): $(OBJS) 
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(NAS_LIBS) -o $@

.PHONY: install
install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

.PHONY: clean
clean:
	rm -f $(PROGS) *.o core
