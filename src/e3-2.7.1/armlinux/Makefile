AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
STRIP=${CROSS_COMPILE}strip

all:	../e3.asm e3.s
#	$(AS) --defsym LINUX=1 --defsym ARMCPU=1 -a=e3arm.lst -gstabs e3.s -o e3.o
#	$(AS) --defsym LINUX=1 --defsym ARMCPU=1 -a=emulayer.lst -gstabs emulayer.s -o emulayer.o
	$(AS) --defsym LINUX=1 --defsym ARMCPU=1 e3.s -o e3.o
	$(AS) --defsym LINUX=1 --defsym ARMCPU=1 emulayer.s -o emulayer.o
	$(LD) e3.o emulayer.o -o e3arm 
	$(STRIP) e3arm
	
