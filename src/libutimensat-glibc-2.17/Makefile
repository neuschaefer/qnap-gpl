# toolchain
CC        = $(CROSS_COMPILE)gcc
AR        = $(CROSS_COMPILE)ar
OBJCOPY   = $(CROSS_COMPILE)objcopy

# path
prefix      ?= $(shell pwd)
KERNEL_PATH ?= /usr/src/linux

# header
INCLUDES += -I.
INCLUDES += -I$(KERNEL_PATH)/include

# C
CFLAGS = -g -O0 -Wall -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS += -DQNAPNAS
CFLAGS += $(INCLUDES)

# eventfd
NAME       = utimensat
SHARE_LIB  = lib$(NAME).so
STATIC_LIB = lib$(NAME).a
OBJS       = ${NAME}.o
ALL        = $(SHARE_LIB) $(STATIC_LIB)

# rule
all : $(ALL)

$(SHARE_LIB) : $(OBJS)
	$(CC) -shared -fPIC -o $(SHARE_LIB) $(OBJS)
	
$(STATIC_LIB) : $(OBJS)
	$(AR) -cru  $(STATIC_LIB) $(OBJS)
	
install : $(ALL)
	# libeventfd library
	$(OBJCOPY) --strip-all $(SHARE_LIB)
	sudo cp $(SHARE_LIB) $(prefix)/lib/

.PHONY : clean distclean all

clean distclean:
	rm -f core *~
	rm -f $(SHARE_LIB) $(STATIC_LIB) $(OBJS)

