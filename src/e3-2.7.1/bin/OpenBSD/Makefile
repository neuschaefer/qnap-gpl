# Makefile for OpenBSD 
# (does not use GNU make extensions)
#

# 1. edit you OS if you want....

#OS=LINUX
#OS=BEOS
#OS=FREEBSD
OS=OPENBSD
#OS=QNX
#OS=W32 	** please use a separate make.bat for W9x **

# 2. edit dest dir prefix if you want....

PREFIX='/usr/local'


# 3. for vi friends only (else leave as is):
#    choice between '/bin/ex' or default '/bin/sed'
EXMODE=SED
#EXMODE=EX



BINDIR='$(PREFIX)/bin'
MANSEC='1'
MANDIR='$(PREFIX)/man/man$(MANSEC)'


#______________________do not edit below line________________________


ASOURCES=e3.asm
AFLAGS = -w+orphan-labels -f elf


all:	$(ASOURCES) Makefile
	nasm $(AFLAGS) -o e3.o $(ASOURCES) -l e3.lst -D$(OS) -D$(EXMODE)
	ld -s -o e3 e3.o
	strip --remove-section .comment e3
	ln -sf e3 e3ws
	ln -sf e3 e3em
	ln -sf e3 e3pi
	ln -sf e3 e3vi
	ln -sf e3 e3ne

install:	e3
	install -d $(PREFIX) $(BINDIR) $(MANDIR)
	install -m 755 e3 $(BINDIR)
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ws
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3em
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3pi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3vi
	ln -sf $(BINDIR)/e3 $(BINDIR)/e3ne
	install -m 644 e3.man $(MANDIR)/e3.$(MANSEC)

clean:
	rm -f e3*.o e3*.lst e3 e3em e3pi e3vi e3ws e3ne
