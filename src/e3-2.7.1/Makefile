# 1. edit you OS if you want....

OS=LINUX
#OS=BEOS
#OS=FREEBSD
#OS=OPENBSD
#OS=QNX
#OS=ATHEOS
#OS=W32 	** please use a separate make.bat for W9x **
#OS=NETBSD

# Set this to gzexe or upx if you want compression
# COMPRESS=gzexe

# 2. edit dest dir prefix if you want....

PREFIX='./'
#PREFIX=/boot/home


# 3. for vi friends only (else leave as is):
#    choice between '/bin/ex' or default '/bin/sed'
EXMODE=SED
#EXMODE=EX



BINDIR='$(PREFIX)/bin'
MANSEC='1'
MANDIR='$(PREFIX)/man/man$(MANSEC)'


#______________________do not edit below line________________________


ASOURCES=e3.asm e3.h
AFLAGS = -w+orphan-labels -f elf
NASM=nasm -O2


all:	e3

e3:	$(ASOURCES) Makefile
ifeq	($(OS),LINUX)
	$(NASM) -f bin -l e3.lst -o e3 e3.asm -DCRIPLED_ELF=1 -D$(OS) -D$(EXMODE) -DNASM
	chmod +x e3
ifeq	($(COMPRESS),upx)
	if which upx > /dev/null 2>&1 ; then \
		upx -q -q -q -k -9 e3 ; \
	fi ;
endif
ifeq	($(COMPRESS),gzexe)
	if which gzexe > /dev/null 2>&1 ; then gzexe e3; fi;
endif
else
	echo $(ASVER)
	$(NASM) $(AFLAGS) -o e3.o e3.asm -l e3.lst -D$(OS) -D$(EXMODE) -DNASM
ifeq	($(OS),QNX)
	ld -s -o e3 e3.o -lc
else
	ld -s -o e3 e3.o
endif
	strip --remove-section .comment e3
endif
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne


# next for running in gnu debugger
debug:	$(ASOURCES) Makefile
	$(NASM) $(AFLAGS) -g -o e3.o e3.asm -l e3.lst -D$(OS) -D$(EXMODE) -DNASM
	ld  -b elf32-i386  --oformat elf32-i386 -s -o e3 e3.o
	strip --remove-section .comment e3
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne

# L I N U X   only
# Experimental stuff for using YASM assembler
# needs some sed changes (jmp vs jmp near) due heavy bugs in YASM optimizer
# (how to switch off that optimizer ?)
yasm:	$(ASOURCES) Makefile
	cat e3.asm | sed -f e3_nasm_yasm.sed > e3.tmp
	yasm -a x86 -m x86 -f elf32 -p nasm $(AFLAGS) -l e3.lstyasm -o e3.o -DLINUX -DSED -DYASM e3.tmp
	rm e3.tmp
	ld -b elf32-i386  --oformat elf32-i386 -s -o e3 e3.o
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne

# L I N U X / AMD-64  only
# ditto YASM stuff
yasm64:	$(ASOURCES) Makefile
	cat e3.asm 	| sed -f e3_nasm_yasm.sed \
			| sed -f e3_yasm_yasm64.sed >e3.tmp
	yasm  -a x86 -m amd64 -f elf64 -p nasm -L nasm $(AFLAGS) \
			-l e3.lstyasm64 -o e3.o -DLINUX -DSED -DYASM -DAMD64 e3.tmp
	rm e3.tmp
	ld -b elf64-x86-64 --oformat elf64-x86-64 -s -o e3 e3.o
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne

# L I N U X   only
# Experimental stuff for using NASM assembler
nasm64:	$(ASOURCES) Makefile
	cat e3.asm 	| sed -f e3_nasm_yasm.sed \
			| sed -f e3_yasm_yasm64.sed >e3.tmp
	$(NASM) -w+orphan-labels -f elf64  -o e3.o e3.tmp -l e3.lst64 -DLINUX -DSED -DYASM -DAMD64 -DNASM
	rm e3.tmp
	ld -b elf64-x86-64 --oformat elf64-x86-64 -s -o e3 e3.o
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne

# added for compatibility reason:
nasm32:	e3

# next for cross asm for the ELKS people
elks:
	nasm -w+orphan-labels -f as86 -o e3-16.o e3-16.asm -l e3-16.lst -D AS86 -D ELKS -DNASM
	ld86 -0 -s -i -H 0xF800 -o e3-16 e3-16.o


# next two for cross asm testing
w32lst:
	$(NASM) -f coff -o e3.oW32 e3.asm -l e3.lstW32 -DW32 -DNASM
	rm e3.oW32

qnxlst:
	$(NASM) -f elf -o e3.oQNX e3.asm -l e3.lstQNX -DQNX -DNASM
	rm e3.oQNX

# next for release maintainance
man2html:
	rman -f HTML e3.man >e3.html

install:	e3
ifeq	($(OS),QNX)
	cp ./e3 $(BINDIR)/e3
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ws
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3em
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3pi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3vi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ne
else
	install -d $(PREFIX) $(BINDIR) $(MANDIR)
	install -m 755 e3 $(BINDIR)
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ws
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3em
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3pi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3vi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ne
	install -m 644 e3.man $(MANDIR)/e3.$(MANSEC)
endif


armlinux:	e3
	@cd armlinux && make all


clean:
	rm -f e3*.o e3*.lst e3 e3em e3pi e3vi e3ws e3ne *~ \
		PIPE_IN e3test~ e3test e3dync e3dync2 e3statc \
		armlinux/*.o armlinux/e3arm
