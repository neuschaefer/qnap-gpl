#COMMIT_REV ?= $(shell git describe --always --abbrev=12)
COMMIT_REV = 1.0-153-g60a143e4686f

obj-m += flashcache.o
flashcache-objs := flashcache_conf.o flashcache_main.o flashcache_subr.o flashcache_ioctl.o flashcache_procfs.o

ifeq (${QTS_SNAPSHOT},yes)
EXTRA_CFLAGS += -DQTS_SNAPSHOT
endif

all:
	$(MAKE) -C $(KERNEL_PATH) M=`pwd` modules V=0

clean:
	#$(MAKE) -C $(KERNEL_PATH) M=`pwd` clean
	rm -rf .*.cmd .tmp_versions *.o *.ko *.mod.c modules.order Module.symvers
