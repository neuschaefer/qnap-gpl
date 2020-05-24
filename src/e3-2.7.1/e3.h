;--------------------------------------------------------------------------
;  e3.asm v2.7.0 Copyright (C) 2000-06 Albrecht Kleine <kleine@ak.sax.de>
;
;  see GNU copyright details in e3.asm
;--------------------------------------------------------------------------

%define WS 1
%define EM 2
%define PI 4
%define VI 8
%define NE 16
;---change DEFAULT_MODE from WS to VI---
%define DEFAULT_MODE VI			;<---- select one of WS, EM, NE, PI, VI
%define MAKE_BACKUP
%define LESSWRITEOPS
;-------
%define UTF8				;for UTF8 console or xterm e.g. @ Suse 9.1
;%define UTF8RTS				;runtime detection of UTF8 console display
;-------
%define BEEP_IN_VI			;undef if you hate beeping computers
%define USE_MATH			;undef if you don't use the numerics
%define USE_PIPE			;undef if you don't use piping through sed/ex
%define USE_BUILTINHELP			;undef if you really don't need help (saves some space)
%define USE_UNDO			;undef if there is low memory
;;;%define USE_EXT_MOVE			;smart move mode for Home,End,BOF,EOF keys
;
;-------
;
;	D O   N O T   C H A N G E   B E L O W   L I N E
;----------------------------------------------------------------------
%ifndef LINUX
 %undef UTF8
%endif

%ifndef UTF8
  %undef UTF8RTS
%endif

%ifdef NETBSD
%define FREEBSD
%endif
%ifdef OPENBSD
%define FREEBSD
%endif
;
;beware of SYS_... constants > 255! (see NetBSD)
;
%ifdef BEOS
	;posix/termios.h				;termios eq termio
	%define TERMIOS_SET 8001h			;TCSETA
	%define TERMIOS_GET 8000h			;TCGETA
	%define TERMIOS_WSIZE 800Ch			;TIOCGWINSZ
	%define NCCS 11
	%define VMIN 4
	%define speed_t1		resb
	%undef USE_PIPE
%define SYS_exit	63
%define SYS_read	2
%define SYS_write	3
%define SYS_open	0
%define SYS_close	1
%define SYS_unlink	39
%define SYS_lseek	5
%define SYS_rename	38
%define SYS_ioctl	4

MAXERRNO	equ 30
ERRNOMEM	equ 12
ERRNOIO		equ 5

	%macro errortext 0
db "Op not permitted",10		;1
db "No such file|directory",10		;2
db 10					;3
db 10					;4
db "Input/output",10			;5
db "No such device",10			;6
db 10					;7
db 10					;8
db "Bad file descriptor",10		;9
db "No child processes",10		;10
db 10					;11		
db "Memory exhausted",10		;12
db "Permission denied",10		;13
db 10					;14
db 10					;15
db "Device|resource busy",10		;16
db "File exists",10			;17
db 10					;18
db "No such device",10			;19
db 10					;20
db "Is a directory",10			;21
db "Invalid argument",10		;22
db "Too many open files",10		;23
db "Too many open files",10		;24
db "Inappropriate ioctl",10		;25
db "Text file busy",10			;26
db "File too large",10			;27
db "No space on device",10		;28
db "Illegal seek",10			;29
db "R/O file system",10			;30
	%endmacro

%else
%ifdef QNX
	;termios.h
	;sys/ioctl.h
	%define TERMIOS_SET   804c7414h	;TIOCSETA
	%define TERMIOS_GET   404c7413h	;TIOCGETA
	%define TERMIOS_WSIZE 40087468h	;TIOCGWINSZ
	;		      rw	
	;		        size
	;			  't'
	;			    nr.
	%define NCCS 40
	%define VMIN 6
	%define speed_t2	resd
;-------
;the QNX version is linked against libc
%define LIBC
extern open,read,write,close,lseek,rename,_exit,ioctl,fstat,fchown,select,unlink
extern errno
%undef USE_PIPE
	%define SYS_fstat		;dummy
	struc stat_struc
.st_ino:	resd 2 
.st_size:	resd 2
.st_dev:	resd 1
.st_rdev:	resd 1
.st_uid:	resd 1;24
.st_gid:	resd 1;28
.st_ctime:	resd 1;
.st_atime:	resd 1;
.st_mtime:	resd 1;40
.st_mode:	resd 1;44
.st_dummy:	resd 20		;who cares?
	endstruc
MAXERRNO	equ 30
ERRNOMEM	equ 12
ERRNOIO		equ 5

	%macro errortext 0
db "Op not permitted",10		;1
db "No such file|directory",10		;2
db 10					;3
db 10					;4
db "Input/output",10			;5
db "No such device",10			;6
db 10					;7
db 10					;8
db "Bad file descriptor",10		;9
db "No child processes",10		;10
db 10					;11		
db "Memory exhausted",10		;12
db "Permission denied",10		;13
db 10					;14
db 10					;15
db "Device|resource busy",10		;16
db "File exists",10			;17
db 10					;18
db "No such device",10			;19
db 10					;20
db "Is a directory",10			;21
db "Invalid argument",10		;22
db "Too many open files",10		;23
db "Too many open files",10		;24
db "Inappropriate ioctl",10		;25
db "Text file busy",10			;26
db "File too large",10			;27
db "No space on device",10		;28
db "Illegal seek",10			;29
db "R/O file system",10			;30
	%endmacro

%else
%ifdef ATHEOS	;--------------------- A T H E O S -----------------------
	;posix/termbits.h
	%define TERMIOS_SET 5406h
	%define TERMIOS_GET 5405h
	%define TERMIOS_WSIZE 5413h
	%define NCCS 19
	%define VMIN 6
	;posix/stat.h
	struc stat_struc
.st_dev:	resd 1
.st_ino:	resd  2
.st_mode:	resd 1
.st_nlink:	resd 1
.st_uid:	resd 1
.st_gid:	resd 1
.st_rdev:	resd 1
.st_size:	resd  2
.st_blksize:	resd 1
.st_blocks:	resd  2
.st_atime:	resd 1
.__unused1:	resd 1
.st_mtime:	resd 1
.__unused2:	resd 1
.st_ctime:	resd 1
.__unused3:	resd 1
.__unused4:	resd 1
.__unused5:	resd 1
	endstruc
	%undef USE_PIPE

%define SYS_exit	6
%define SYS_read	3
%define SYS_write	4
%define SYS_open	1
%define SYS_close	2
%define SYS_unlink	20
%define SYS_lseek	13
%define SYS_kill	92
%define SYS_rename	7
%define SYS_ioctl	116
%define SYS_sigaction	93
%define SYS_fchown	86
%define SYS_fstat	11
%define SYS_select	42

SIGCONT		equ 18
SIGSTOP		equ 19


MAXERRNO	equ 30
ERRNOMEM	equ 12
ERRNOIO		equ 5

	%macro errortext 0
db "Op not permitted",10		;1
db "No such file|directory",10		;2
db 10					;3
db 10					;4
db "Input/output",10			;5
db "No such device",10			;6
db 10					;7
db 10					;8
db "Bad file descriptor",10		;9
db "No child processes",10		;10
db 10					;11		
db "Memory exhausted",10		;12
db "Permission denied",10		;13
db 10					;14
db 10					;15
db "Device|resource busy",10		;16
db "File exists",10			;17
db 10					;18
db "No such device",10			;19
db 10					;20
db "Is a directory",10			;21
db "Invalid argument",10		;22
db "Too many open files",10		;23
db "Too many open files",10		;24
db "Inappropriate ioctl",10		;25
db "Text file busy",10			;26
db "File too large",10			;27
db "No space on device",10		;28
db "Illegal seek",10			;29
db "R/O file system",10			;30
	%endmacro
%else
%ifdef LINUX	;----------------------- L I N U X -----------------------
	;asm/termbits.h
	;asm/ioctls.h
	%define TERMIOS_SET 5402h			;TCSETS
	%define TERMIOS_GET 5401h			;TCGETS
	%define TERMIOS_WSIZE 5413h			;TIOCGWINSZ
	%define NCCS 19
	%define VMIN 6
;-------
%ifndef AMD64
	;asm/stat.h
	%define UIDGID_WORD
	struc stat_struc
.st_dev:	resd 1
.st_ino:	resd 1		;unsigned long  st_ino;
.st_mode:	resw 1		;unsigned short st_mode;
.st_nlink:	resw 1
.st_uid:	resw 1
.st_gid:	resw 1
.st_rdev:	resd 1
.st_size:	resd 1
.st_blksize:	resd 1
.st_blocks:	resd 1
.st_atime:	resd 1
.__unused1:	resd 1
.st_mtime:	resd 1
.__unused2:	resd 1
.st_ctime:	resd 1
.__unused3:	resd 1
.__unused4:	resd 1
.__unused5:	resd 1
	endstruc

%define SYS_exit	1
%define SYS_fork	2
%define SYS_read	3
%define SYS_write	4
%define SYS_open	5
%define SYS_close	6
%define SYS_unlink	10
%define SYS_execve	11
%define SYS_lseek	19
%define SYS_utime	30
%define SYS_kill	37
%define SYS_rename	38
%define SYS_pipe	42
%define SYS_brk		45
%define SYS_ioctl	54
%define SYS_dup2	63
%define SYS_sigaction	67
%define SYS_rt_sigaction 174
%define SYS_readlink	85
%define SYS_fchmod	94
%define SYS_fchown	95
%define SYS_fstat	108
%define SYS_wait4	114
%define SYS_select	142
%ifndef ARMCPU
%define time_t	resd
%endif
%else				;;--------- AMD64
	struc stat_struc
.st_dev:	resq 1
.st_ino:	resq 1
.st_nlink:	resq 1
.st_mode:	resd 1
.st_uid:	resd 1
.st_gid:	resd 1
.__unused0:	resd 1
.st_rdev:	resq 1
.st_size:	resq 1
.st_blksize:	resq 1
.st_blocks:	resq 1
.st_atime:	resq 1
.__unused1:	resq 1
.st_mtime:	resq 1
.__unused2:	resq 1
.st_ctime:	resq 1
.__unused3:	resq 1
.__unused4:	resq 1
.__unused5:	resq 1
	endstruc
%define SYS_exit	60
%define SYS_fork	57
%define SYS_read	0
%define SYS_write	1
%define SYS_open	2
%define SYS_close	3
%define SYS_unlink	87
%define SYS_execve	59
%define SYS_lseek	8
%define SYS_utime	132	;30
%define SYS_kill	62
%define SYS_rename	82
%define SYS_pipe	22
%define SYS_brk		12
%define SYS_ioctl	16
%define SYS_dup2	33
%define SYS_rt_sigaction 13
%define SYS_rt_sigreturn 15
%define SYS_readlink	89
%define SYS_fchmod	91
%define SYS_fchown	93
%define SYS_fstat	5
%define SYS_wait4	61
%define SYS_select	23	;142

%define time_t	resq
%endif

%ifndef ARMCPU
	struc utimbuf_struc
.actime: time_t  1
.modtime:time_t 1
	endstruc
%else
	struc utimbuf_struc
.actime: resd  1
.modtime:resd 1
	endstruc
%endif
%ifdef CRIPLED_ELF
%define USE_SPECIAL_HEADER		;special ELF header etc
%endif

;-------
%ifdef UTF8
 %define NEW_CURSOR_MGNT		;switch cursor depending of 'INSERT'-mode
 %undef CURSORMGNT			;switch cursor depending of 'INSERT'-mode
 ;no more support of CURSORMGNT for UTF-8 thus replaced by:
 ;8.Juni 2004: see /usr/src/linux/Documentation/VGA-softcursor.txt
%else
 %define CURSORMGNT
 ;trad style
 %ifdef AMD64
  %undef CURSORMGNT			;work around January 2006
 %endif
%endif
;--------

SIGCONT		equ 18
SIGSTOP		equ 19

%define CAPTURE_STDERR
%ifdef EX
 %define USE_EX_NO_SED
 %define EX_PATH '/usr/bin/ex'		;(ex is usually a symlink to vi)   [ old was /bin/ex ]
%else
 %undef USE_EX_NO_SED
 %ifndef PERLPIPE
 %define SEDPATH '/bin/sed'		;DEFAULT
 %else
 %define SEDPATH '/usr/bin/perl'
 %endif
%endif

MAXERRNO	equ 32
ERRNOMEM	equ 12
ERRNOIO		equ 5
ERRNOEXEC	equ 31
	%macro errortext 0
db "Op not permitted",10		;1
db "No such file|directory",10		;2
db 10					;3
db 10					;4
db "Input/output",10			;5
db "No such device",10			;6
db 10					;7
db 10					;8
db "Bad file descriptor",10		;9
db "No child processes",10		;10
db 10					;11		
db "Memory exhausted",10		;12
db "Permission denied",10		;13
db 10					;14
db 10					;15
db "Device|resource busy",10		;16
db "File exists",10			;17
db 10					;18
db "No such device",10			;19
db 10					;20
db "Is a directory",10			;21
db "Invalid argument",10		;22
db "Too many open files",10		;23
db "Too many open files",10		;24
db "Inappropriate ioctl",10		;25
db "Text file busy",10			;26
db "File too large",10			;27
db "No space on device",10		;28
db "Illegal seek",10			;29
db "R/O file system",10			;30
db "Can't exec "			;31
%ifdef USE_EX_NO_SED
db EX_PATH,10
%else
db SEDPATH,10
%endif
db "Broken pipe",10			;32
	%endmacro
%ifdef LIBC
 extern open,read,write,close,lseek,rename,_exit,ioctl,fstat,fchown,select,unlink,fchmod
 extern _errno
 %undef SYS_readlink
 %undef SYS_brk
 %undef SYS_kill
 %undef SYS_sigaction
 %undef SYS_rt_sigaction
 %undef USE_PIPE
%endif
%ifdef ARMCPU
 %undef USE_MATH
 %undef USE_UNDO
 %undef USE_PIPE
%endif
%else
%ifdef FREEBSD	;----------------------- FREE B S D -----------------------
	;sys/termios.h
	%define TERMIOS_SET   802c7414h		;TIOCSETA
	%define TERMIOS_GET   402c7413h		;TIOCGETA
	%define TERMIOS_WSIZE 40087468h
	;		      rw	
	;		        size
	;			  't'
	;			    nr.
	%define NCCS 20
	%define VMIN 16
	%define speed_t3	resd
	%define ICRNL	0x100 
	%define IXON	0x200
	%define ICANON	0x100
	%define ISIG	0x80
	%define ECHO	0x8
	%define TSize   word			;due oversized ICANON
;------

	struc stat_struc
.st_dev:	resd 1
.st_ino:	resd 1
%ifdef OPENBSD
.st_mode:	resw 1				;for syscall 279
.st_nlink:	resw 1				;ditto
%else
.st_mode:	resd 1
.st_nlink:	resd 1
%endif
.st_uid:	resd 1
.st_gid:	resd 1
.st_rdev:	resd 1
.st_atime:	resd 1
.st_atimes:	resd 1
.st_mtime:	resd 1
.st_mtimes:	resd 1
.st_ctime:	resd 1
.st_ctimes:	resd 1
.st_size:	resd 2
.st_blocks:	resd 2
.st_blksize:	resd 1
.st_flags:	resd 1
.st_gen:	resd 1
.st_spare:	resd 5
	endstruc

%define SYS_exit	1
%define SYS_fork	2
%define SYS_read	3
%define SYS_write	4
%define SYS_open	5
%define SYS_close	6
%define SYS_unlink	10
%define SYS_execve	59
%define SYS_lseek	199	;old was 19
%define SYS_utime	138
%define SYS_kill	37
%define SYS_rename	128
%define SYS_pipe	42
%define SYS_ioctl	54
%define SYS_dup2	90
%define SYS_sigaction	46	;also 342 *** take care if >255 cause only register al used
%define SYS_readlink	58
%define SYS_fchown	123
%define SYS_fchmod      124
%define SYS_fstat	279	;old was 189
%define SYS_wait4	7
%define SYS_select	93

%ifdef OPENBSD
%undef SYS_kill
%undef SYS_sigaction
%undef  SYS_fstat	
%define SYS_fstat	189	;could not test 292
%endif

%ifdef NETBSD
%undef SYS_kill		;both
%undef SYS_sigaction	;currently only for old COMPAT layer working
%endif


%define time_t	resd

	struc utimbuf_struc
.actime: time_t  2
.modtime:time_t 2
	endstruc
	
%undef CAPTURE_STDERR
%ifdef EX
 %define USE_EX_NO_SED
 %define EX_PATH '/usr/bin/ex'		;(ex is usually a symlink to vi)   [ old was /bin/ex ]
%else
 %undef USE_EX_NO_SED
 %ifndef PERLPIPE
 %define SEDPATH '/usr/bin/sed'		;DEFAULT
 %else
 %define SEDPATH '/usr/bin/perl'
 %endif
%endif

MAXERRNO	equ 32
ERRNOMEM	equ 12
ERRNOIO		equ 5
ERRNOEXEC	equ 31
ERRNOREGFILE	equ 21
	%macro errortext 0
db "Op not permitted",10		;1
db "No such file|directory",10		;2
db 10					;3
db 10					;4
db "Input/output",10			;5
db "No such device",10			;6
db 10					;7
db 10					;8
db "Bad file descriptor",10		;9
db "No child processes",10		;10
db 10					;11		
db "Memory exhausted",10		;12
db "Permission denied",10		;13
db 10					;14
db 10					;15
db "Device busy",10			;16
db "File exists",10			;17
db 10					;18
db "No such device",10			;19
db 10					;20
db "Is a directory",10			;21
db "Invalid argument",10		;22
db "Too many open files",10		;23
db "Too many open files",10		;24
db "Inappropriate ioctl",10		;25
db "Text file busy",10			;26
db "File too large",10			;27
db "No space on device",10		;28
db "Illegal seek",10			;29
db "R/O file system",10			;30
db "Can't exec "			;31
%ifdef USE_EX_NO_SED
db EX_PATH,10
%else
db SEDPATH,10
%endif
db "Broken pipe",10			;32
	%endmacro
%ifdef LIBC
 extern open,read,write,close,lseek,rename,_exit,ioctl,fstat,fchown,select,unlink,fchmod
 extern errno
 %undef SYS_readlink
 %undef SYS_brk
 %undef SYS_kill
 %undef SYS_sigaction
 %undef USE_PIPE
%endif

%else
%ifdef W32	;----------------------- W I N  32 -----------------------
%define W32LF				;<-- controls linefeed style

STD_INPUT_HANDLE	equ -10
STD_OUTPUT_HANDLE	equ -11
ENABLE_WINDOW_INPUT	equ 8
FILE_ATTRIBUTE_NORMAL	equ 128
OPEN_EXISTING		equ 3
CREATE_ALWAYS		equ 2
GENERIC_READ		equ $80000000
GENERIC_WRITE		equ $40000000
INVALID_HANDLE_VALUE	equ -1
FOREGROUND_BLUE		equ 1
FOREGROUND_GREEN	equ 2
FOREGROUND_RED		equ 4
FOREGROUND_INTENSITY	equ 8
DARKWHITE		equ (FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED)
WHITE			equ (DARKWHITE|FOREGROUND_INTENSITY)
BACKGROUND_BLUE		equ 16
YELLOW_BLUE		equ FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY|BACKGROUND_BLUE
YELLOW_BLUE_TWICE	equ YELLOW_BLUE| (YELLOW_BLUE<<16)
LEFT_ALT_PRESSED	equ 2
CTRL_PRESSED		equ 12
VK_SPACE		equ 0x0020
VK_PRIOR		equ 0x0021
VK_DELETE		equ 0x002E
CF_OEMTEXT		equ 7

extern MessageBoxA,MessageBeep,ExitProcess
extern GetStdHandle,GetCommandLineA,GetLastError,SetConsoleTextAttribute
extern SetConsoleMode,GetConsoleScreenBufferInfo,SetConsoleCursorPosition
extern WriteFile,ReadFile,CreateFileA,CloseHandle,SetFilePointer,MoveFileA,DeleteFileA
extern FillConsoleOutputAttribute,WaitForSingleObject,ReadConsoleA,ReadConsoleInputA
extern FlushConsoleInputBuffer,WriteConsoleOutputCharacterA,WriteConsoleOutputAttribute
extern HeapCreate,HeapAlloc,HeapFree,HeapDestroy,OpenClipboard,EmptyClipboard
extern SetClipboardData,GetClipboardData,IsClipboardFormatAvailable,CloseClipboard
	%define W32_EXTENDED_IO
	%undef USE_PIPE

MAXERRNO	equ 32
ERRNOMEM	equ 8
ERRNOIO		equ 7

	%macro errortext 0
db "invalid function",10		;1
db "file not found",10			;2
db "path not found",10			;3
db "too many open files",10		;4
db "access denied",10			;5
db "invalid handle",10			;6
db "I/O error",10			;7
db "not enough memory",10		;8
db 10					;9
db 10					;10
db 10					;11
db 10					;12
db 10					;13
db 10					;14
db 10					;15
db 10					;16
db 10					;17
db 10					;18
db 10					;19
db 10					;20
db 10					;21
db 10					;22
db 10					;23
db 10					;24
db 10					;25
db 10					;26
db 10					;27
db 10					;28
db 10					;29
db 10					;30
db 10					;31
db "sharing violation",10  		;32
	%endmacro

%else		;----------------------- END OS -----------------------
%error no OS defined
%endif
%endif
%endif
%endif
%endif
%endif


%ifdef TERMIOS_SET
%ifdef ARMCPU
 	struc termios_struc
.c_iflag:	resd 1
.c_oflag:	resd 1
.c_cflag:	resd 1
.c_lflag:	resd 1
.c_line:	resb 1
.c_cc:		resb NCCS
	endstruc
%else
%define tcflag_t	resd
%define cc_t		resb
 	struc termios_struc
.c_iflag:	tcflag_t 1
.c_oflag:	tcflag_t 1
.c_cflag:	tcflag_t 1
.c_lflag:	tcflag_t 1
.c_line:	cc_t 1
%ifdef speed_t1
c_ixxxxx:	speed_t1 1
c_oxxxxx:	speed_t1 1
%endif
.c_cc:		cc_t NCCS
%ifdef speed_t2
res:		resd 3
c_ixxxxx:	speed_t2 1
c_oxxxxx:	speed_t2 1
%endif
%ifdef speed_t3
c_ispeed speed_t3 1
c_ospeed speed_t3 1
%endif
	endstruc
%endif
	struc winsize_struc
.ws_row:resw 1
.ws_col:resw 1
.ws_xpixel:resw 1
.ws_ypixel:resw 1
	endstruc

%ifndef IXON				;all except *BSD
%define ICRNL	0000400q
%define IXON	0002000q
%define ICANON	0000002q
%define ISIG	0000001q
%define ECHO	0000010q
%define TSize   byte
%endif
%endif

;-------
%ifndef LINUX
 %undef CRIPLED_ELF
 %undef UTF8
%endif
;--------
;
stdtxtlen	equ 10			;do not move this to EOF: code size would increase

%ifdef FREEBSD
SIGCONT		equ 19
SIGSTOP		equ 17
O_WRONLY_CREAT_TRUNC equ 601h		;see fcntl.h
%else
%ifdef QNX
O_WRONLY_CREAT_TRUNC equ 1401q
%else
O_WRONLY_CREAT_TRUNC equ 1101q
%endif
%endif

O_RDONLY	equ 0
PERMS		equ 644q
stdin		equ 0
stdout 		equ 1
optslen		equ 124
TAB		equ 8
TABCHAR		equ 09h
SPACECHAR	equ ' '
CHANGED		equ '*'
UNCHANGED	equ SPACECHAR
LINEFEED	equ 0ah
NEWLINE		equ LINEFEED
RETURN		equ 0dh
SEDBLOCK	equ 4096




;--------------------------------------------------------------------------
%ifdef CRIPLED_ELF
;
; building e3 via "nasm -f bin ...."  using an idea from
;"A Whirlwind Tutorial on Creating Really Teensy ELF Executables for Linux"
;
       %macro ELFheader 0
ehdr:	db 0x7F, "ELF", 1, 1, 1, 0	;Elf32_Ehdr starts here
	dd 0,0
	dw 2				;e_type
	dw 3				;e_machine
	dd 1				;e_version
	dd _start			;e_entry
	dd phdr1- $$			;e_phoff
	dd 0				;e_shoff
	dd 0				;e_flags
	dw ehdrsize			;e_ehsize
	dw phdrsize			;e_phentsize
	dw 2				;e_phnum
phdr1:					;Elf32_Phdr starts here
	dd 1				;both p_type and e_shentsize,e_shnum
	dw 0				;both p_offset and e_shstrndx
ehdrsize equ $ - ehdr
	dw 0
	dd $$				;p_vaddr
	dd $$				;p_paddr
	dd filesize			;p_filesz
	dd filesize			;p_memsz
	dd 5				;p_flags i.e. READ/EXECUTE
	dd 0;  0x1000			;p_align
phdrsize equ $ - phdr1
phdr2:					;another Elf32_Phdr starts here
	dd 1				;p_type
	dd filesize
	dd $$+filesize
	dd $$+filesize
	dd 0				;p_filesz
	dd bsssize			;p_memsz
	dd 6				;p_flags i.e. READ/WRITE
	dd 0;  0x1000			;p_align
	%endmacro
%endif
;-------
%ifndef ARMCPU
	%macro ORGheader 0
%ifdef USE_SPECIAL_HEADER
%ifdef TINLINK
	org 0x800004A			;see file contrib/README.tinlink624
%else
	org 0x8048000
	ELFheader
%endif
%else
	;nothing
%endif
	%endmacro
%endif
;-------
%ifdef DYN				;DYN == "libc dynamic linked"
 %define _start main			;call it "main", libc startup code expects this name
%endif

;-------
%ifdef AMD64
  %macro PUSH_ALL 0
  	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
  %endmacro
  %macro POP_ALL 0
	pop  rbp
	pop  rdi
	pop  rsi
	pop  rdx
	pop  rcx
	pop  rbx
	pop  rax
  %endmacro
%else
  %macro PUSH_ALL 0
	pusha
  %endmacro
  %macro POP_ALL 0
	popa
  %endmacro
%endif
;--------------------------------------------------------------------------
