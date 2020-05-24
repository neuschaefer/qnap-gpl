.include "macros"
				@@ ************ BEGIN including e3.h ************
.equ WS,1
.equ EM,2
.equ PI,4
.equ VI,8
.equ NE,16
@@ ***Change DEFAULT_MODE grom 1 to 8, KenChen ***
.equ DEFAULT_MODE,8
.equ MAKE_BACKUP,1 @(no value known)
.equ LESSWRITEOPS,1 @(no value known)
.equ UTF8,1 @(no value known)
.equ UTF8RTS,1 @(no value known)
.equ BEEP_IN_VI,1 @(no value known)
.equ USE_BUILTINHELP,1 @(no value known)
.ifndef LINUX 
.endif  
.ifndef UTF8 
.endif  
.ifdef NETBSD 
.endif  
.ifdef OPENBSD 
.endif  
.ifdef BEOS 
.else  
.ifdef QNX 
.else  
.ifdef ATHEOS 
.else  
.ifdef LINUX 
.equ TERMIOS_SET,21506
.equ TERMIOS_GET,21505
.equ TERMIOS_WSIZE,21523
.equ NCCS,19
.equ VMIN,6
.ifndef AMD64 
.equ UIDGID_WORD,1 @(no value known)
.struct 0
stat_struc.st_dev:
.struct 4
stat_struc.st_ino:
.struct 8
stat_struc.st_mode:
.struct 10
stat_struc.st_nlink:
.struct 12
stat_struc.st_uid:
.struct 14
stat_struc.st_gid:
.struct 16
stat_struc.st_rdev:
.struct 20
stat_struc.st_size:
.struct 24
stat_struc.st_blksize:
.struct 28
stat_struc.st_blocks:
.struct 32
stat_struc.st_atime:
.struct 36
stat_struc.__unused1:
.struct 40
stat_struc.st_mtime:
.struct 44
stat_struc.__unused2:
.struct 48
stat_struc.st_ctime:
.struct 52
stat_struc.__unused3:
.struct 56
stat_struc.__unused4:
.struct 60
stat_struc.__unused5:
.struct 64
.equ stat_struc_size,64
.equ SYS_exit,1
.equ SYS_fork,2
.equ SYS_read,3
.equ SYS_write,4
.equ SYS_open,5
.equ SYS_close,6
.equ SYS_unlink,10
.equ SYS_execve,11
.equ SYS_lseek,19
.equ SYS_utime,30
.equ SYS_kill,37
.equ SYS_rename,38
.equ SYS_pipe,42
.equ SYS_brk,45
.equ SYS_ioctl,54
.equ SYS_dup2,63
.equ SYS_sigaction,67
.equ SYS_rt_sigaction,174
.equ SYS_readlink,85
.equ SYS_fchmod,94
.equ SYS_fchown,95
.equ SYS_fstat,108
.equ SYS_wait4,114
.equ SYS_select,142
.ifndef ARMCPU 
.endif  
.else  
.endif  
.ifndef ARMCPU 
.else  
.struct 0
utimbuf_struc.actime:
.struct 4
utimbuf_struc.modtime:
.struct 8
.equ utimbuf_struc_size,8
.endif  
.ifdef CRIPLED_ELF 
.endif  
.ifdef UTF8 
.equ NEW_CURSOR_MGNT,1 @(no value known)
.purgem CURSORMGNT 
.else  
.ifdef AMD64 
.endif  
.endif  
.equ SIGCONT,18
.equ SIGSTOP,19
.equ CAPTURE_STDERR,1 @(no value known)
.ifdef EX 
.else  
.purgem USE_EX_NO_SED 
.ifndef PERLPIPE 
.equ SEDPATH,0
.else  
.endif  
.endif  
.equ MAXERRNO,32
.equ ERRNOMEM,12
.equ ERRNOIO,5
.equ ERRNOEXEC,31
.macro errortext 
.ascii	"Op not permitted"
.byte	10
.ascii	"No such file|directory"
.byte	10
.byte	10
.byte	10
.ascii	"Input/output"
.byte	10
.ascii	"No such device"
.byte	10
.byte	10
.byte	10
.ascii	"Bad file descriptor"
.byte	10
.ascii	"No child processes"
.byte	10
.byte	10
.ascii	"Memory exhausted"
.byte	10
.ascii	"Permission denied"
.byte	10
.byte	10
.byte	10
.ascii	"Device|resource busy"
.byte	10
.ascii	"File exists"
.byte	10
.byte	10
.ascii	"No such device"
.byte	10
.byte	10
.ascii	"Is a directory"
.byte	10
.ascii	"Invalid argument"
.byte	10
.ascii	"Too many open files"
.byte	10
.ascii	"Too many open files"
.byte	10
.ascii	"Inappropriate ioctl"
.byte	10
.ascii	"Text file busy"
.byte	10
.ascii	"File too large"
.byte	10
.ascii	"No space on device"
.byte	10
.ascii	"Illegal seek"
.byte	10
.ascii	"R/O file system"
.byte	10
.ascii	"Can't exec "
.ifdef USE_EX_NO_SED 
.else  
.byte	'/','b','i','n','/','s','e','d',10
.endif  
.ascii	"Broken pipe"
.byte	10
.endm  
.ifdef LIBC 
.endif  
.ifdef ARMCPU 
.purgem USE_MATH 
.purgem USE_UNDO 
.purgem USE_PIPE 
.endif  
.else  
.ifdef FREEBSD 
.ifdef OPENBSD 
.else  
.endif  
.ifdef OPENBSD 
.endif  
.ifdef NETBSD 
.endif  
.ifdef EX 
.else  
.ifndef PERLPIPE 
.else  
.endif  
.endif  
.ifdef USE_EX_NO_SED 
.else  
.endif  
.ifdef LIBC 
.endif  
.else  
.ifdef W32 
.else  
.endif  
.endif  
.endif  
.endif  
.endif  
.endif  
.ifdef TERMIOS_SET 
.ifdef ARMCPU 
.struct 0
termios_struc.c_iflag:
.struct 4
termios_struc.c_oflag:
.struct 8
termios_struc.c_cflag:
.struct 12
termios_struc.c_lflag:
.struct 16
termios_struc.c_line:
.struct 17
termios_struc.c_cc:
.struct 36
.equ termios_struc_size,36
.else  
.ifdef speed_t1 
.endif  
.ifdef speed_t2 
.endif  
.ifdef speed_t3 
.endif  
.endif  
.struct 0
winsize_struc.ws_row:
.struct 2
winsize_struc.ws_col:
.struct 4
winsize_struc.ws_xpixel:
.struct 6
winsize_struc.ws_ypixel:
.struct 8
.equ winsize_struc_size,8
.ifndef IXON 
.endif  
.endif  
.ifndef LINUX 
.endif  
.equ stdtxtlen,10
.ifdef FREEBSD 
.else  
.ifdef QNX 
.else  
.equ O_WRONLY_CREAT_TRUNC,577
.endif  
.endif  
.equ O_RDONLY,0
.equ PERMS,420
.equ stdin,0
.equ stdout,1
.equ optslen,124
.equ TAB,8
.equ TABCHAR,9
.equ SPACECHAR,32
.equ CHANGED,42
.equ UNCHANGED,SPACECHAR
.equ LINEFEED,10
.equ NEWLINE,LINEFEED
.equ RETURN,13
.equ SEDBLOCK,4096
.ifdef CRIPLED_ELF 
.endif  
.ifndef ARMCPU 
.ifdef USE_SPECIAL_HEADER 
.ifdef TINLINK 
.else  
.endif  
.else  
.endif  
.endif  
.ifdef DYN 
.endif  
.ifdef AMD64 
.else  
.macro PUSH_ALL 
	stmfd r13!,{r0-r6}
.endm  
.macro POP_ALL 
	ldmfd r13!,{r0-r6}
.endm  
.endif  
				@@ ************ END include ************
.text
.code 32
ORGheader:

.global _start
_start:	CALL SetTermStruc
.ifdef SELFTEST 
.else  
.ifdef W32 
.else  
.ifdef BEOS 
.else  
.ifdef DYN 
.else  
.ifdef SYS_rt_sigaction 
	CALL SetSigHandler
.else  
.ifdef SYS_sigaction 
.endif  
.endif  
.ifdef ATHEOS 
.endif  
	ldmfd r13!,{r3}
	ldmfd r13!,{r5}
	mov r7,#+1
prog:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne prog
.ifdef AMD64 
.else  
	mov r9,r5
	mov r10,#0x5
	sub r9,r9,r10
	mov r5,R9
.endif  
	CALL SetEditMode
	beq prog1
	ldr r11,= DEFAULT_MODE
	strB r11,[r0]
prog1:
	ldmfd r13!,{r5}
.ifdef NEW_CURSOR_MGNT 
	CALL SetCursorBlock
.endif  
.endif  
.endif  
.endif  
.endif  
.ifdef CURSORMGNT 
.ifndef ARMCPU 
.else  
.endif  
.endif  
.ifdef UTF8RTS 
	A_DR r2,getPos
	ldr r12,= gPlen
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL WriteFile0
	A_DR r2,screenbuffer
	mov r12,#0xa
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL ReadFile0
	mov r9,r2
	add r9,r9,r0
	mov r10,#0x2
	sub r9,r9,r10
	ldrB r12,[R9]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0x33
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	A_DR r12,isUTF8
	strB r0,[r12]
.endif  
ReStart:
	CALL NewFile
	blo E3exit
MainCharLoop:
	CALL ChkCursPos
	CALL IsViMode
	bne MCL
	A_DR r12,blockbegin
	ldr r2,[r12]
	cmp r2,#0
	beq MCL
	stmfd r13!,{r4}
	mov r4,r2
	CALL KeyEnd
	A_DR r12,blockende
	str r4,[r12]
	ldmfd r13!,{r4}
	CALL ShowBl1
MCL:	CALL DispNewScreen
	CALL RestoreStatusLine
	CALL HandleChar
.ifdef W32LF 
.endif  
	A_DR r1,endeedit
	mov r11,#0x0
	ldrB r10,[r1]
	cmp r10,r11
	beq MainCharLoop
	eors r5,r5,r5
	mov r11,#0x2
	ldrB r10,[r1]
	cmp r10,r11
	beq ReStart
E3exit:	CALL KursorStatusLine
.ifdef W32 
.endif  
	A_DR r2,text
	CALL WriteFile00
.ifdef NEW_CURSOR_MGNT 
	CALL SetCursorNormal
.endif  
	A_DR r1,tempfile2
	CALL Unlink
.ifdef W32 
.else  
	ldr r2,= TERMIOS_SET

	CALL IOctlTerminal0
	b Exit
.endif  
HandleChar:
	CALL ReadChar
	mov r12,#0xff
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne ExtAscii
	A_DR r5,mode
	mov r11,#0x6
	ldrB r10,[r5]
	tst r10,r11
	beq NO_EM01
	mov r12,#0xb
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq IsCtrlK
	A_DR r12,EmaCtrlK
	mov r11,#0x0
	strB r11,[r12]
IsCtrlK:
	mov r12,#0x13
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq IsCtrlS
	mov r12,#0x12
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq IsCtrlS
	A_DR r12,EmaCtrlS
	mov r11,#0x0
	strB r11,[r12]
IsCtrlS:

NO_EM01:
	ldr r11,= VI
	ldrB r10,[r5]
	cmp r10,r11
	beq ISVI1
	mov r12,#0x20
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs NormChar
	mov r12,r0
	and r12,r12,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r12
	ldr r12,= jumps1
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	ldr r11,= WS
	ldrB r10,[r5]
	cmp r10,r11
	beq CJump
	mov r12,#0x20
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	ldr r11,= EM
	ldrB r10,[r5]
	cmp r10,r11
	beq CJump
	mov r12,#0x20
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	ldr r11,= PI
	ldrB r10,[r5]
	cmp r10,r11
	beq CJump
	mov r12,#0x20
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
CJump:	b CompJump2
ISVI1:
	mov r12,#0x7
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyDel
	mov r12,#0x8
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyDell
	ldr r12,= RETURN
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyRet
NormChar:
	A_DR r12,mode
	mov r11,#0x6
	ldrB r10,[r12]
	tst r10,r11
	beq NOEM0
	CALL ShowBl0
NOEM0:
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	mov r12,r0
	and r12,r12,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq NormCh2
noUTF_A:

.endif  
	CALL CheckMode
.ifdef USE_UNDO 
.else  
	bne OverWriteChar
.endif  
NormCh2:
	stmfd r13!,{r0}
.ifdef W32LF 
.endif  
	CALL Insert1Byte
	ldmfd r13!,{r0}
	blo InsWriteEnd
OverWriteChar:
	mov r7,#+1
	strb r0,[r4]
	add r4,r4,r7
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	eors r0,r0,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
OWCloopUTF8:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r9,r4
	add r9,r9,r0
	ldrB r12,[R9]
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq OWCloopUTF8
	CALL DeleteByte
noUTF_B:

.endif  
SetChg:	A_DR r12,changed
	ldr r11,= CHANGED
	strB r11,[r12]
InsWriteEnd:
	RET
KeyVICmdr:
	CALL ReadOneChar
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq InsWriteEnd
	ldr r12,= RETURN
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne KeyVICmdr1
	ldr r12,= NEWLINE
	bic r0,r0,#0xFF
	orr r0,r0,r12
KeyVICmdr1:

.ifdef USE_UNDO 
.endif  
KeyVICmdr2:
	strB r0,[r4]
	b SetChg
KeyEmaCtrlQ:
	A_DR r5,asknumber
	CALL GetOctalToInteger
	bls InsWriteEnd
	mov r12,r2
	mov r2,r0
	mov r0,r12
	mov r11,#0x0
	mov r8,#0x1
	add r11,r11,r8,lsl #8
	cmp r0,r11
	blo NormCh2
	RET
CtrlKMenu:
	A_DR r1,Ktable
	mov r12,#0x4b
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b Menu
CtrlQMenu:
	A_DR r1,Qtable
	b PicoQM
PicoJMenu:
	A_DR r1,PicoJtable
	mov r12,#0x4a
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b Menu
PicoQMenu:
	A_DR r1,PicoQtable
PicoQM:	mov r12,#0x51
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b Menu
CtrlXMenu:
	A_DR r1,Xtable
	mov r12,#0x58
	bic r0,r0,#0xFF
	orr r0,r0,r12
Menu:	mov r2,#0x5e
	mov r8,#0x20
	add r2,r2,r8,lsl #8
	mov r8,#0x20
	add r2,r2,r8,lsl #16
	mov r8,#0x20
	add r2,r2,r8,lsl #24

	mov r12,r0
	and r12,r12,#0xFF
	bic r2,r2,#0xFF00
	orr r2,r2,r12,lsl #8
MakeScanCode:
	CALL WriteTwo
	stmfd r13!,{r1}
	CALL GetChar
	ldmfd r13!,{r1}
	mov r12,#0x1f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	ldr r12,= Ktable_size
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs InsWriteEnd
	and r12,r0,#0xFF
	ldrB r12,[r1,r12]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,r0
	and r12,r12,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
ExtAscii:
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r12
	ldr r12,= jumps1
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs InsWriteEnd
	eors r0,r0,r0
	A_DR r12,EmaCtrl
	str r0,[r12]
CompJump2:
	mov r12,#0x0
	bic r1,r1,#0xFF00
	orr r1,r1,r12,lsl #8
.ifdef YASM 
.else  
	mov r1,r1
	mov r1,r1,lsl #16
	mov r1,r1,lsr #16
.endif  
	mov r9,r1
	mov r10,#0x2
	mov r8,r9
	mul r9,r8,r10
	A_DR r10,jumptab1
	add r9,r9,r10
	ldrH r1,[R9]
.ifdef YASM 
.ifdef AMD64 
.else  
.endif  
.else  
	A_DR r11,_start
	adds r1,r1,r11

.endif  
	stmfd r13!,{r14}
	add r14,pc,#4
	mov pc,r1
	mov r0,r0
	mov r1,r1
	mov r2,r2
	mov r3,r3
	ldmfd r13!,{r14}
	A_DR r12,numeriere
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne BZNret
	stmfd r13!,{r4}
	A_DR r5,sot
	mov r12,r4
	mov r4,r5
	mov r5,r12
	eors r3,r3,r3
BZNLoop:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL LookForward
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	cmp r4,r5
	bls BZNLoop
	A_DR r12,linenr
	str r3,[r12]
	ldmfd r13!,{r4}
	A_DR r12,numeriere
	mov r11,#0x0
	strB r11,[r12]
BZNret:	RET
KeyRetNoInd:
	eors r0,r0,r0
	b KeyRetNInd
KeyRet:
.ifndef NO_AUTO_INDENT 
.ifdef SELFTEST 
.else  
	CALL CheckMode
	bne OvrRet
	CALL CountToLineBegin
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	orrs r0,r0,r0
	beq KeyRetNInd
	mov r1,r0
	eors r0,r0,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
KeyRetSrch:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	cmp r0,r1
	bhs KeyRetNInd
	mov r9,r5
	add r9,r9,r0
	ldr r11,= SPACECHAR
	ldrB r10,[R9]
	cmp r10,r11
	beq KeyRetSrch
	mov r9,r5
	add r9,r9,r0
	ldr r11,= TABCHAR
	ldrB r10,[R9]
	cmp r10,r11
	beq KeyRetSrch
.endif  
.else  
.endif  
KeyRetNInd:
	stmfd r13!,{r5}
	stmfd r13!,{r0}
	CALL GoDown
	ldmfd r13!,{r0}
	stmfd r13!,{r0}
.ifdef W32LF 
.endif  
	CALL InsertByte0
	ldmfd r13!,{r2}
	ldmfd r13!,{r5}
	blo SimpleRet
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,linenr
	mov r11,#0x1
	ldr r10,[r12]
	adds r10,r10,r11
	str r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r7,#+1
.ifdef W32LF 
.else  
	ldr r12,= NEWLINE
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
.endif  
	cmp r2,#0
	beq SimpleRet
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
SimpleRet:
	RET
OvrRet:	eors r0,r0,r0
	A_DR r12,ch2linebeg
	str r0,[r12]
	b DownRet
KeyDown:
	CALL CountColToLineBeginVis
DownRet:
	CALL GoDown
	CALL LookLineDown
	b JmpSC
KeyUp:	CALL GoUp
	CALL CountColToLineBeginVis
	CALL LookLineUp
	b JmpSC
KeyHalfPgUp:
	CALL CountColToLineBeginVis
	CALL LookHalfPgUp
	b SetColumn
KeyHalfPgDn:
	CALL CountColToLineBeginVis
	CALL LookHalfPgDn
	b SetColumn
KeyScrollUp:
	CALL CountColToLineBeginVis
	CALL LookScrUp
	b SetColumn
KeyScrollDn:
	CALL CountColToLineBeginVis
	CALL LookScrDn
	b SetColumn
KeyPgUp:
	CALL CountColToLineBeginVis
	CALL LookPageUp
JmpSC:	b SetColumn
KeyPgDn:
	CALL CountColToLineBeginVis
	CALL LookPgDown
SetColumn:
	A_DR r12,ch2linebeg
	ldr r2,[r12]
	eors r3,r3,r3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
SCloop:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8 
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq noUTF_C
.endif  
	ldrB r12,[r4]
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq SCloop
noUTF_C:

.endif  
	cmp r3,r2
	bhs SCret
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq SCret
	ldr r11,= TABCHAR
	ldrB r10,[r4]
	cmp r10,r11
	beq SCtab
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b SCloop
SCtab:	CALL SpacesForTab
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	mov r11,r3,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r3,r3,#0xFF
	orr r3,r3,r11
	cmp r3,r2
	bls SCloop
SCret:	RET
VIsetMarker:
	cmp r4,r2
	bhi Marker_above_cursor
	A_DR r12,blockende
	ldr r2,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	cmp r2,r6
	blo Mbel
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
Mbel:	A_DR r12,EmaMark
	str r2,[r12]
KeyHome:
	CALL CountToLineBegin
	subs r4,r4,r0
	RET
Marker_above_cursor:

	A_DR r12,EmaMark
	str r2,[r12]
	CALL KeyEnd
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	cmp r4,r6
	blo Mret
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
Mret:	RET
KeyIns:	A_DR r12,insstat
	ldrB r11,[r12]
	mvns r11,r11
	strB r11,[r12]
	eors r0,r0,r0
	CALL IsViMode
	bne KeyIns2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,VICmdMode
	ldrB r10,[r12]
	mov r11,r0
	and r11,r11,#0xFF
	cmp r10,r11
	bne KeyIns2
	A_DR r12,insstat
	strB r0,[r12]
	CALL KeyVImode0
KeyIns2:
	CALL IsEmMode
	bne KeyIns3
	A_DR r12,showblock
	strB r0,[r12]
KeyIns3:

.ifdef NEW_CURSOR_MGNT 
	A_DR r12,insstat
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne SetCursorNormal
	b SetCursorBlock
.endif  
	RET
KeyVICmdJ:
	CALL KeyEnd
	b KeyDel
KeyDell:
	CALL KeyLeft
	beq KeyDell2
KeyDel:	cmp r4,r6
	bhs KeyIns3
	eors r0,r0,r0
KDloopUTF8:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	mov r9,r4
	add r9,r9,r0
	ldrB r12,[R9]
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KDloopUTF8
noUTF_D:

.endif  
.ifdef W32LF 
.endif  
	CALL IsViMode
	bne DeleteByte
	mov r5,r4
	A_DR r12,VInolinebased
	mov r11,#0x1
	strB r11,[r12]
	CALL KeyEmaAltW2
	b DeleteByte
KeyDell2:
	A_DR r11,sot
	cmp r4,r11
	bls KeyIns3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,linenr
	mov r11,#0x1
	ldr r10,[r12]
	subs r10,r10,r11
	str r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b KeyCtrlT1
KeyEmaCtrlT:

.ifdef UTF8 
	RET
.else  
.ifdef USE_UNDO 
.endif  
.endif  
KeyRight:

.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyRight
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
noUTF_E:

.endif  
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	bne KeyRNoMargin
	CALL CheckEof
	bhs KeyRightEnd
	CALL IsViMode
	beq KeyRightEnd
	CALL CheckENum
	CALL GoDown
KeyRNoMargin:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
KeyRightEnd:
	RET
KeyCLeft3:
	A_DR r11,sot
	cmp r4,r11
	bls KeyCLEnd
	CALL CheckENum
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
KeyCtrlQW:
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldr r11,= NEWLINE
	ldrB r10,[R9]
	cmp r10,r11
	beq KeyCLeft3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r11,#0x2f
	ldrB r10,[r4]
	cmp r10,r11
	bls KeyCtrlQW
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	mov r11,#0x2f
	ldrB r10,[R9]
	cmp r10,r11
	bhi KeyCtrlQW
KeyCLEnd:
	RET
KeyCRight3:
	CALL CheckEof
	bhs KeyCREnd
	CALL CheckENum
	b KQZ1
KeyCtrlQZ:
	mov r12,#0x2f
	bic r0,r0,#0xFF
	orr r0,r0,r12
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq KeyCRight3
KQZ1:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL IsEmMode
	beq ISEM2
	ldrB r10,[r4]
	mov r11,r0
	and r11,r11,#0xFF
	cmp r10,r11
	bls KeyCtrlQZ
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldrB r10,[R9]
	mov r11,r0
	and r11,r11,#0xFF
	cmp r10,r11
	b ISEM22
ISEM2:	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldrB r10,[R9]
	mov r11,r0
	and r11,r11,#0xFF
	cmp r10,r11
	bls KeyCtrlQZ
	ldrB r10,[r4]
	mov r11,r0
	and r11,r11,#0xFF
	cmp r10,r11
ISEM22:	bhi KeyCtrlQZ
KeyCREnd:
	RET
KeyVIcmde3:
	CALL CheckEof
	bhs KeyCREnd
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
KeyVIcmde:
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq KeyVIcmde3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r11,#0x2f
	ldrB r10,[r4]
	cmp r10,r11
	bls KeyVIcmde
	mov r9,r4
	mov r10,#0x1
	add r9,r9,r10
	mov r11,#0x2f
	ldrB r10,[R9]
	cmp r10,r11
	bhi KeyVIcmde
	RET
KeyEmaCtrlO:
	CALL Insert1Byte
	blo KeyRightEnd
	ldr r12,= NEWLINE
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strB r0,[r4]
	RET
KeyCtrlQE:
	CALL LookPgBegin
	CALL KursorFirstLine
	b KCtKV1
KeyCtrlQX:
	CALL LookPgEnd
	CALL KeyEnd
	CALL KursorLastLine
	b KCtKV1
KeyCtrlQV:
	A_DR r12,bereitsges
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq KeyCREnd
	A_DR r12,oldQFpos
	ldr r4,[r12]
KCtKV1:	b CQFNum
KeyVIbsearch:
	mov r12,#0xff
	mov r8,#0xff
	add r12,r12,r8,lsl #8
	mov r8,#0xff
	add r12,r12,r8,lsl #16
	mov r8,#0xff
	add r12,r12,r8,lsl #24
	stmfd r13!,{r12}
	b KVIf
KeyVIfsearch:
	mov r12,#0x1
	stmfd r13!,{r12}
KVIf:	A_DR r12,grossklein
	mov r11,#0xff
	strB r11,[r12]
	b KeyECtS1
PicoCtrlTpico:
	A_DR r12,PicoSearch
	str r4,[r12]
KeyEmaAltPer:
	mov r12,#0x1
	stmfd r13!,{r12}
	ldmfd r13!,{r12}
	A_DR r11,vorwarts
	str r12,[r11]
	A_DR r12,grossklein
	mov r11,#0xdf
	strB r11,[r12]
KeyCtrlQA:
	A_DR r12,bereitsges
	mov r11,#0x2
	strB r11,[r12]
	CALL AskForReplace
	blo SimpleRet9
CQACtrlL:
	stmfd r13!,{r4}
	CALL FindText
	bhs CQACL2
	ldmfd r13!,{r4}
SimpleRet9:
	RET
CQACL2:	A_DR r12,suchlaenge
	ldr r0,[r12]
	CALL DeleteByte
	A_DR r12,repllaenge
	ldr r0,[r12]
	CALL InsertByte
	A_DR r5,replacetext
	CALL MoveBlock
	b CQFFound
KeyPiCtrlJT:
	A_DR r12,bereitsges
	mov r11,#0x2
	ldrB r10,[r12]
	cmp r10,r11
	beq CQACtrlL
	RET
KeyEmaCtrlR:
	mov r12,#0xff
	mov r8,#0xff
	add r12,r12,r8,lsl #8
	mov r8,#0xff
	add r12,r12,r8,lsl #16
	mov r8,#0xff
	add r12,r12,r8,lsl #24
	stmfd r13!,{r12}
	b KECS
KeyEmaCtrlS:
	mov r12,#0x1
	stmfd r13!,{r12}
KECS:	A_DR r12,grossklein
	mov r11,#0xdf
	strB r11,[r12]
KeyECtS1:
	ldmfd r13!,{r12}
	A_DR r11,vorwarts
	str r12,[r11]
	A_DR r12,EmaMark
	str r4,[r12]
	CALL ShowBl0
KeyCtrlQF:
	CALL IsEmMode
	bne NO_EM04
	A_DR r12,EmaCtrlS
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq KeyCtrlL
NO_EM04:
	A_DR r12,suchtext
	ldr r12,[r12]
	stmfd r13!,{r12}
	A_DR r12,bereitsges
	mov r11,#0x1
	strB r11,[r12]
	CALL AskForFind
	ldmfd r13!,{r1}
	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	A_DR r12,mode
	mov r11,#0xc
	ldrB r10,[r12]
	tst r10,r11
	beq NO_VIPI01
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	orrs r0,r0,r0
	bne QFpico
	mov r12,r1
	and r12,r12,#0xFF
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	beq CtrlQFEnd
	A_DR r12,suchtext
	strB r1,[r12]
QFpico:	A_DR r12,PicoSearch
	str r4,[r12]
	b CQFCtrlL
NO_VIPI01:
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	blo CtrlQFEnd
CQFCtrlL:
	stmfd r13!,{r4}
	CALL FindText
	A_DR r12,EmaCtrlS
	mov r11,#0x1
	strB r11,[r12]
	blo CtrlQFNotFound
CQFFound:
	A_DR r12,oldQFpos
	str r4,[r12]
	ldmfd r13!,{r5}
CQFNum:	b CheckENum
CtrlQFNotFound:
	ldmfd r13!,{r4}
CtrlQFEnd:
	RET
KeyCtrlL:
	A_DR r12,bereitsges
	ldr r0,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	beq CQFCtrlL
	A_DR r12,mode
	mov r11,#0x15
	ldrB r10,[r12]
	tst r10,r11
	beq SimpleRet4
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	beq CQACtrlL
SimpleRet4:
	RET
KeyVIcmd1:
	CALL ReadOneChar
	mov r12,#0x47
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyCtrlQR
	RET
ViSpecial:
	cmp r2,#0
	beq KeyCtrlQR
	b KCQI
KeyNedCtrlA:
	A_DR r12,EmaMark
	str r6,[r12]
	CALL ShowBl1
KeyCtrlQR:
	A_DR r4,sot
	b CQFNum
KeyCtrlQP:
	A_DR r12,veryold
	ldr r2,[r12]
	cmp r2,r6
	bhi SimpleRet4
	mov r4,r2
JmpCQFN3:
	b CQFNum
KeyCtrlQB:
	mov r12,r4
	mov r4,r0
	mov r0,r12
	A_DR r12,blockbegin
	ldr r4,[r12]
CtrlQB2:
	orrs r4,r4,r4
	bne CQFNum
	mov r12,r0
	mov r0,r4
	mov r4,r12
	RET
KeyCtrlQK:
	mov r12,r4
	mov r4,r0
	mov r0,r12
	A_DR r12,blockende
	ldr r4,[r12]
	b CtrlQB2
KeyCtrlQI:
	A_DR r5,asklineno
	CALL GetAsciiToInteger
	bls CtrlQFEnd
KCQI:	A_DR r4,sot
	CALL LookPD2
JmpCQFN:
	b JmpCQFN3
KeyCtrlQDel:
	CALL KeyLeft
	CALL CountToLineBegin
	subs r4,r4,r0
	b KCY
KeyVICmdD:
	A_DR r12,VInolinebased
	mov r11,#0x1
	strB r11,[r12]
KeyCtrlQY:
	CALL CountToLineEnd
.ifdef W32LF 
.endif  
	CALL IsViMode
	bne CtrlTEnd1
	CALL CtrlTEnd1
	b KeyLeft
KeyCmddw:
	CALL CountToWordBeginVIstyle
	b NO_EM05
KeyCtrlY:
	CALL KeyHome
	CALL CountToLineEnd
	A_DR r12,mode
	ldr r11,= WS
	ldrB r10,[r12]
	cmp r10,r11
	bne NO_WS01
KCY:	CALL DeleteByteCheckMarker
	b KeyCtrlT1
NO_WS01:
	A_DR r12,mode
	mov r11,#0xc
	ldrB r10,[r12]
	tst r10,r11
	beq KeyCtrlT
	mov r9,r4
	add r9,r9,r0
	mov r2,R9
	cmp r2,r6
	beq CtrlTEnd1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b CtrlTEnd1
KeyCtrlT:
	CALL CountToWordBegin
	CALL IsEmMode
	bne NO_EM05
KeyEmaCtrlK:
	CALL CountToLineEnd
NO_EM05:
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	bne CtrlTEnd1
KeyCtrlT1:
	eors r0,r0,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef W32LF 
.endif  
CtrlTEnd1:
	CALL CheckEof
	beq SimpleRet3
	A_DR r12,mode
	ldr r11,= WS
	ldrB r10,[r12]
	cmp r10,r11
	beq DeleteByteCheckMarker
	mov r5,r4
	CALL KeyEmaAltW2
	b DelBjmp
KeyEmaCtrlW:
	A_DR r12,showblock
	ldr r2,[r12]
	A_DR r12,mode
	ldr r11,= PI
	ldrB r10,[r12]
	cmp r10,r11
	bne NOPI1
KECW:
.ifndef YASM 
	cmp r2,#0
	beq KeyCtrlY
.else  
.endif  
	A_DR r12,EmaMark
	ldr r2,[r12]
	cmp r2,#0
	beq KECW
	b NOPI2
NOPI1:	cmp r2,#0
	beq SimpleRet3
	A_DR r12,EmaMark
	ldr r2,[r12]
	cmp r2,#0
	beq SimpleRet3
NOPI2:	CALL KeyEmaAltW
	A_DR r12,EmaKiSrc
	ldr r4,[r12]
	A_DR r12,EmaKiSize
	ldr r0,[r12]
DelBjmp:
	b DeleteByte
KeyCtrlKY:
	CALL CheckBlock
	blo SimpleRet3
	A_DR r12,blockende
	ldr r0,[r12]
	mov r4,r5
	subs r0,r0,r5
	CALL DeleteByte
	mov r12,r2
	mov r2,r0
	mov r0,r12
	CALL InitSV2
JmpCQFN2:
	b JmpCQFN
KeyCtrlKH:
	A_DR r12,showblock
	mov r11,#0x1
	ldrB r10,[r12]
	eors r10,r10,r11
	strB r10,[r12]
SimpleRet3:
	RET
KeyCtrlKK:

	A_DR r12,blockende
	str r4,[r12]
	b ShowBl1
KeyCtrlKC:
	CALL CopyBlock
	blo SimpleRet2
CtrlKC2:
	A_DR r12,blockbegin
	str r4,[r12]
	adds r0,r0,r4
	b InitSV3
KeyCtrlXX:
	A_DR r12,EmaMark
	ldr r2,[r12]
	cmp r2,#0
	beq SimpleRet3
	CALL KeyEmaMark
	mov r4,r2
	CALL KeyEmaCtrlL
KeyCXX:	b JmpCQFN2
KeyCtrlKV:
	CALL CopyBlock
	blo SimpleRet2
	stmfd r13!,{r4}
	A_DR r12,blockbegin
	ldr r10,[r12]
	cmp r4,r10
	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	A_DR r12,blockbegin
	ldr r4,[r12]
	CALL DeleteByte
	rsbs r0,r0,#0
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	ldmfd r13!,{r4}
	blo CtrlKC2
	A_DR r12,blockende
	str r4,[r12]
	subs r4,r4,r0
KeyCtrlKB:
	A_DR r12,blockbegin
	str r4,[r12]
ShowBl1:
	A_DR r12,showblock
	mov r11,#0x1
	strB r11,[r12]
SimpleRet2:
	RET
ShowBl0:
	A_DR r12,showblock
	mov r11,#0x0
	strB r11,[r12]
	RET
KeyVICmdm:
	CALL ReadOneChar
	mov r12,#0x61
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne SimpleRet2
	stmfd r13!,{r4}
	CALL KeyHome
	A_DR r12,blockbegin
	str r4,[r12]
	ldmfd r13!,{r4}
	RET
KeyVICmdJmpM:
	CALL ReadOneChar
	mov r12,#0x61
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne SimpleRet2
	A_DR r12,blockbegin
	ldr r2,[r12]
	cmp r2,#0
	beq SimpleRet2
	mov r4,r2
	b KeyCXX
KeyEmaMark:
	A_DR r12,EmaMark
	str r4,[r12]
	b ShowBl1
KeyCtrlKR:
	CALL ReadBlock
	blo CtrlKREnd
	CALL KeyCtrlKB
	adds r2,r2,r4
	A_DR r12,blockende
	str r2,[r12]
	A_DR r12,mode
	mov r11,#0x12
	ldrB r10,[r12]
	tst r10,r11
	beq NO_EM03
	A_DR r12,EmaMark
	str r2,[r12]
	CALL ShowBl0
NO_EM03:
	A_DR r12,mode
	ldr r11,= PI
	ldrB r10,[r12]
	cmp r10,r11
	bne CtrlKREnd
	mov r4,r2
CtrlKREnd:
	b RestKursPos
KeyCtrlKW:
	CALL CheckBlock
	blo CtrlKSEnd
	CALL SaveBlock
	b CtrlKREnd
KeyEmaCtrlXF:
	A_DR r12,changed
	ldr r11,= UNCHANGED
	ldrB r10,[r12]
	cmp r10,r11
	beq KECF
	A_DR r5,asksave2
	CALL DE1
	CALL RestKursPos
	CALL CheckUserAbort
	beq CtrlKSEnd
	mov r12,#0xdf
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x4e
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
KECF:	beq KCKD2
	b KeyCtrlKD
KeyEmaCtrlXW:
	CALL GetBlockName
	blo CtrlKSEnd
	A_DR r5,blockpath
XW1:	mov r7,#+1
 PUSH_ALL @(is a macro)
	A_DR r4,filepath
XW0:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne XW0
	strb r0,[r4]
	add r4,r4,r7
 POP_ALL @(is a macro)
KeyCtrlKS0:
	CALL SetChg
KeyCtrlKS:
	CALL SaveFile
	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	CALL RestKursPos
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	blo CtrlKSEnd
Unchg:	A_DR r12,changed
	ldr r11,= UNCHANGED
	strB r11,[r12]
CtrlKSEnd:
	RET
KeyCtrlKD:
	CALL KeyCtrlKS
	blo KeyKXend
KCKD2:	A_DR r12,endeedit
	mov r11,#0x2
	strB r11,[r12]
	RET
KeyCtrlKQ:
	A_DR r12,changed
	ldr r11,= UNCHANGED
	ldrB r10,[r12]
	cmp r10,r11
	beq KCKXend
	A_DR r5,asksave
	CALL DE1
	CALL RestKursPos
	CALL CheckUserAbort
	beq CtrlKSEnd
	mov r12,#0xdf
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x4e
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KCKXend
	mov r12,#0x4c
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne KeyCtrlKX
	CALL KCKXend
KeyCtrlKX:
	CALL KeyCtrlKS
	blo CtrlKSEnd
KCKXend:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,endeedit
	mov r11,#0x1
	ldrB r10,[r12]
	adds r10,r10,r11
	strB r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
KeyKXend:
	RET
KeyVICmdW:
	mov r9,r2
	mov r10,#0x2
	add r9,r9,r10
	mov r5,R9
	ldr r11,= SPACECHAR
	ldrB r10,[r5]
	cmp r10,r11
	bhi XW1
	RET
VINoLineCmd:
	ldr r0,[r2]
	mov r12,#0x77
	mov r8,#0x21
	add r12,r12,r8,lsl #8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyCtrlKS0
	mov r12,#0x77
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyCtrlKS
	mov r12,#0x78
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyCtrlKX
	mov r12,#0x24
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	bne KVI_KX0
KeyCtrlQC:
	mov r4,r6
	b CQFNum
KVI_KX0:
	mov r12,#0x77
	mov r8,#0x71
	add r12,r12,r8,lsl #8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
KVI_KX:	beq KeyCtrlKX
	mov r12,#0x77
	mov r8,#0x20
	add r12,r12,r8,lsl #8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyVICmdW
	mov r12,#0x71
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyCtrlKQ
	mov r12,#0x71
	mov r8,#0x21
	add r12,r12,r8,lsl #8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KCKXend
	mov r12,#0x65
	mov r8,#0x20
	add r12,r12,r8,lsl #8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyVICmdE
	mov r12,#0x68
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyHelp
.ifdef UTF8RTS 
	mov r12,#0x75
	mov r11,r0,lsl #16
	mov r11,r11,lsr #16
	cmp r11,r12
	beq KeyUTF8switch
.endif  
.ifndef USE_PIPE 
	RET
.else  
.endif  
KeyVICmdZ:
	CALL ReadOneChar
	mov r12,#0x5a
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KVI_KX
	RET
KeyVI1Char:
	CALL KeyHome
	ldr r11,= SPACECHAR
	ldrB r10,[r4]
	cmp r10,r11
	bhi KFC2
KFC1:	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq KFC2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	ldr r11,= SPACECHAR
	ldrB r10,[r4]
	cmp r10,r11
	bls KFC1
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldr r11,= SPACECHAR
	ldrB r10,[R9]
	cmp r10,r11
	bhi KFC1
KFC2:	RET
KeyVICmdS:
	CALL KeyHome
	CALL KeyEmaCtrlK
	A_DR r12,VInolinebased
	mov r11,#0x1
	strB r11,[r12]
	b KeyVICmdI
KeyVICmdd:
	CALL ReadOneChar
	mov r12,#0x77
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	A_DR r12,VInolinebased
	mov r11,#0x1
	strB r11,[r12]
	beq KeyCmddw
	mov r12,#0x64
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	A_DR r12,VInolinebased
	mov r11,#0x0
	strB r11,[r12]
	beq KeyCtrlY
	mov r12,#0x27
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne KFC2
	CALL ReadOneChar
	mov r12,#0x61
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne KFC2
	A_DR r12,blockbegin
	ldr r2,[r12]
	cmp r2,#0
	beq KFC2
	CALL VIsetMarker
callKECW:
	CALL KeyEmaCtrlW
	eors r0,r0,r0
	A_DR r12,blockbegin
	str r0,[r12]
	b JmpCQFn
KeyVICmdI:
	CALL KeyVI1Char
	b KeyVImode0
KeyVICmdp:
	A_DR r12,EmaKiSize
	ldr r2,[r12]
jmpKFC2:

.ifdef YASM 
.else  
	cmp r2,#0
	beq KFC2
.endif  
	A_DR r12,VInolinebased
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq KeyVICmdpnLB
	CALL OvrRet
KeyVICmdP:
	A_DR r12,EmaKiSize
	ldr r2,[r12]
	cmp r2,#0
	beq jmpKFC2
	A_DR r12,VInolinebased
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq KeyVICmdPnLB
	CALL KeyHome
KeyVICP2:
	stmfd r13!,{r4}
	CALL KeyEmaCtrlY
	ldmfd r13!,{r4}
JmpCQFn:
	b CQFNum
KeyVICmdR:
	A_DR r12,insstat
	mov r11,#0xfe
	strB r11,[r12]
	b KeyVImode0
KeyVICmdO:
	CALL KeyHome
	CALL KeyRet
	CALL KeyUp
	b KeyVImode0
KeyVICmdo:
	CALL KeyEnd
	CALL KeyRet
	b KeyVImode0
KeyVICmdA:
	CALL KeyEnd
	b KeyVImode0
KeyVIcmda:
	CALL KeyRight
KeyVIcmdi:
	A_DR r12,insstat
	mov r11,#0x1
	strB r11,[r12]
KeyVImode0:
	mov r12,#0x0
	stmfd r13!,{r12}
	b KVim1
KeyVICmdC:
	CALL KeyEmaCtrlK
	A_DR r12,VInolinebased
	mov r11,#0x1
	strB r11,[r12]
	b KeyVImode0
KeyVICmdpnLB:
	CALL KeyRight
KeyVICmdPnLB:
	CALL KeyVICP2
	A_DR r12,EmaKiSize
	ldr r10,[r12]
	adds r4,r4,r10
KeyLeft:

.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyLeft
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
noUTF_F:

.endif  
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldr r11,= NEWLINE
	ldrB r10,[R9]
	cmp r10,r11
	bne KeyLNoMargin
	A_DR r11,sot
	cmp r4,r11
	beq KeyLeftEnd
	CALL IsViMode
	beq KeyLeftEnd
	CALL CheckENum
	CALL GoUp
KeyLNoMargin:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef W32LF 
.endif  
KeyLeftEnd:
	RET
KeyEnd:	CALL CountToLineEnd
	adds r4,r4,r0
	RET
KeyVImode1:
	mov r12,#0x1
	stmfd r13!,{r12}
KVim1:	ldmfd r13!,{r0}
	A_DR r12,VICmdMode
	strB r0,[r12]
	RET
KeyVIex:
	CALL InputStringWithMessage0
	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	CALL RestKursPos
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	blo Kviex
	A_DR r5,optbuffer
	eors r3,r3,r3
	mov r2,r0
	cmp r2,#0
	beq Kviex
	stmfd r13!,{r5}
	mov r7,#+1
CheckDig:
	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0x30
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs CD1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
CD1:	mov r12,#0x3a
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo CD2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
CD2:	subs r2,r2,#1
	bne CheckDig
	ldmfd r13!,{r2}
	orrs r3,r3,r3
	bne VINoLineCmd
	CALL GetAsciiToInteger
	b ViSpecial
IsViMode:
	A_DR r12,mode
	ldr r11,= VI
	ldrB r10,[r12]
	cmp r10,r11
	RET
IsEmMode:
	A_DR r12,mode
	ldr r11,= EM
	ldrB r10,[r12]
	cmp r10,r11
Kviex:	RET
DispNewScreen:
	A_DR r12,mode
	mov r11,#0x16
	ldrB r10,[r12]
	tst r10,r11
	beq NoEmBlock
	A_DR r12,showblock
	ldr r2,[r12]
	cmp r2,#0
	beq NoEmBlock
	A_DR r12,EmaMark
	ldr r2,[r12]
	cmp r2,#0
	beq NoEmBlock
	mov r0,r4
	cmp r2,r0
	blo EmBlock
	mov r12,r2
	mov r2,r0
	mov r0,r12
EmBlock:
	A_DR r12,blockbegin
	str r2,[r12]
	A_DR r12,blockende
	str r0,[r12]
NoEmBlock:
	CALL GetEditScreenSize
	eors r0,r0,r0
	A_DR r12,isbold
	strB r0,[r12]
	A_DR r12,inverse
	strB r0,[r12]
	A_DR r12,zloffst
	str r0,[r12]
	A_DR r12,columne
	str r0,[r12]
	A_DR r12,fileptr
	str r4,[r12]
	stmfd r13!,{r4}
	CALL CountColToLineBeginVis
	A_DR r12,columns
	ldr r1,[r12]
	mov r9,r1
	mov r10,#0x4
	sub r9,r9,r10
	mov r1,R9
	cmp r0,r1
	blo DispShortLine
	subs r0,r0,r1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,zloffst
	str r0,[r12]
DispShortLine:
	CALL LookPgBegin
	mov r5,r4
	A_DR r12,lines
	ldr r2,[r12]
.ifndef YASM 
	cmp r2,#0
	beq Kviex
.else  
.endif  
	mov r7,#+1
	mov r12,#0xff
	bic r1,r1,#0xFF00
	orr r1,r1,r12,lsl #8
DispNewLine:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r12,#0x1
	mov r11,r1,lsl #16
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF00
	orr r1,r1,r11,lsl #8
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r4,screenline
	eors r3,r3,r3
	mov r12,#0x0
	bic r1,r1,#0xFF
	orr r1,r1,r12
.ifdef LESSWRITEOPS 
	CALL SetColor2
.endif  
DispCharLoop:

	A_DR r12,fileptr
	ldr r10,[r12]
	cmp r5,r10
	bne DispCharL1
	A_DR r12,tabcnt
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	bne DispCharL1
	A_DR r12,kurspos
	str r1,[r12]
	A_DR r12,zloffst
	ldr r0,[r12]
	mov r12,r1
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	A_DR r12,columne
	ldr r10,[r12]
	adds r10,r10,r0
	str r10,[r12]
.ifdef CURSORMGNT 
.endif  
DispCharL1:
	CALL SetColor
DispEndLine:
	cmp r5,r6
	bhi FillLine
	A_DR r12,tabcnt
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq ELZ
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,tabcnt
	mov r11,#0x1
	ldrB r10,[r12]
	subs r10,r10,r11
	strB r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b ELZ2
ELZ:	cmp r5,r6
	bne ELZ6
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b ELZ2
ELZ6:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	ldr r12,= TABCHAR
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne ELZ3
	CALL SpacesForTab
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r12,#0x1
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r11,lsl #8
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,tabcnt
	mov r11,r0,lsr #8
	strB r11,[r12]
ELZ2:	ldr r12,= SPACECHAR
	bic r0,r0,#0xFF
	orr r0,r0,r12
ELZ3:	ldr r12,= NEWLINE
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq FillLine
.ifdef W32LF 
.endif  
	ldr r12,= SPACECHAR
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs ELZ9
ELZ99:	mov r12,#0x2e
	bic r0,r0,#0xFF
	orr r0,r0,r12
ELZ9:
.ifndef W32 
	mov r12,#0x7f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo ELZ7
	beq ELZ99
.ifndef UTF8 
.else  
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	bne YXCVB
	mov r12,#0x2e
	bic r0,r0,#0xFF
	orr r0,r0,r12
YXCVB:
.endif  
.endif  
ELZ7:
.endif  
	A_DR r10,columns
	ldrB r12,[r10]
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhs DispEndLine
.ifdef UTF8 
	mov r12,#0x0
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq CountByte
.endif  
	stmfd r13!,{r0}
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	ldmfd r13!,{r0}
	beq UByte234
	blo CountByte
	stmfd r13!,{r0}
	ldrB r12,[r5]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	ldmfd r13!,{r0}
	bne UByte234
CountByte:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r12,#0x1
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r11,lsl #8
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
UByte234:
	A_DR r12,zloffst
	ldr r10,[r12]
	cmp r3,r10
	bls ELZ5
	strb r0,[r4]
	add r4,r4,r7
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
.else  
.endif  
.ifdef CURSORMGNT 
.endif  
ELZ5:	b DispCharLoop
FillLine:
	stmfd r13!,{r2}
	A_DR r12,columns
	ldr r2,[r12]
	mov r12,r1
	and r12,r12,#0xFF
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r2,r2,#0xFF
	orr r2,r2,r11
	ldr r12,= SPACECHAR
	bic r0,r0,#0xFF
	orr r0,r0,r12
	cmp r2,#0
	beq FillLine2
	A_DR r12,inverse
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne FillLine1
	strb r0,[r4]
	add r4,r4,r7
.ifdef CURSORMGNT 
.endif  
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	beq FillLine2
FillLine1:

@reploop:
	strb r0,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#20		@rep stosb
FillLine2:
	ldmfd r13!,{r2}
	mov r11,#0x0
	strB r11,[r4]
	CALL ScreenLineShow
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	bne DispNewLine
	ldmfd r13!,{r4}
	b RestKursPos
.ifdef CURSORMGNT 
.endif  
.ifdef W32 
.else  
SIS6:	A_DR r12,isbold
	mov r11,#0x0
	strB r11,[r12]
SIS5:	A_DR r5,bold0
SIS2:	ldr r12,= boldlen
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
SIS3:	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
SIS4:	ldmfd r13!,{r5}
	ldmfd r13!,{r2}
	RET
SetColor:
	stmfd r13!,{r2}
	stmfd r13!,{r5}
	CALL IsShowBlock
	bhs SCEsc1
	A_DR r12,isbold
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq SIS4
	A_DR r12,isbold
	mov r11,#0x1
	strB r11,[r12]
SCEsc2:	A_DR r5,bold1
	b SIS2
SCEsc1:	A_DR r12,isbold
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq SIS4
	b SIS6
.ifdef LESSWRITEOPS 
SetColor2:

	stmfd r13!,{r2}
	stmfd r13!,{r5}
	CALL IsShowBlock
	bhs SIS5
	b SCEsc2
.endif  
.endif  
IsShowBlock:
	A_DR r12,showblock
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq SBlock
	A_DR r12,blockbegin
	mov r11,#0x0
	ldr r10,[r12]
	cmp r10,r11
	beq SBlock
	A_DR r12,blockbegin
	ldr r10,[r12]
	cmp r10,r5
	bhi SBlock
	A_DR r12,blockende
	ldr r10,[r12]
	cmp r5,r10
	blo SB_ret
SBlock:	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
SB_ret:	RET
GetEditScreenSize:

.ifdef W32 
.else  
	ldr r2,= TERMIOS_WSIZE

	A_DR r3,winsize
	CALL IOctlTerminal
	ldr r0,[r3]
	mov r11,#0xff
	mov r8,#0xff
	add r11,r11,r8,lsl #8
	cmp r0,r11
	blo iserr
	orrs r0,r0,r0
	bne noerr
iserr:	mov r0,#0x18
	mov r8,#0x0
	add r0,r0,r8,lsl #8
	mov r8,#0x50
	add r0,r0,r8,lsl #16

noerr:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,lines
	strB r0,[r12]
	mov r0,r0,lsr #16
	A_DR r12,columns
	strB r0,[r12]
	RET
.endif  
WriteTwo:
	A_DR r12,screenline
	str r2,[r12]
StatusLineShow:

.ifdef W32 
.endif  
	eors r2,r2,r2
ScreenLineShow:
 PUSH_ALL @(is a macro)
.ifdef LESSWRITEOPS 
.ifdef W32 
.else  
	eors r1,r1,r1
.endif  
	A_DR r12,columns
	ldr r0,[r12]
	mov r9,r0
	mov r10,#0x20
	add r9,r9,r10
	mov r0,R9
	mul r12,r0,r2
	mov r0,r12
	mov r3,#0
	mov r9,r0
	A_DR r10,screenbuffer
	add r9,r9,r10
	mov r4,R9
.else  
.endif  
	mov r7,#+1
	A_DR r5,screenline
sl3:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef LESSWRITEOPS 
	A_DR r11,screenbuffer_end
	cmp r4,r11
	bhs sl5
	ldrB r12,[r4]
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq sl4
	strB r0,[r4]
sl5:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
sl4:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.endif  
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne sl3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef LESSWRITEOPS 
	orrs r1,r1,r1
	beq NoWrite
.endif  
	stmfd r13!,{r3}
	eors r3,r3,r3
	A_DR r10,lines
	ldrB r12,[r10]
	bic r3,r3,#0xFF00
	orr r3,r3,r12,lsl #8
	mov r12,r2
	and r12,r12,#0xFF
	mov r11,r3,lsl #16
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r3,r3,#0xFF00
	orr r3,r3,r11,lsl #8
.ifdef W32_EXTENDED_IO 
.else  
	CALL sys_writeKP
	ldmfd r13!,{r3}
	stmfd r13!,{r2}
	A_DR r0,screencolors1
	CALL sys_writeSLColors
	A_DR r2,screenline
	CALL WriteFile0
	ldmfd r13!,{r2}
	A_DR r0,screencolors0
	CALL sys_writeSLColors
	A_DR r12,kurspos2
	ldr r3,[r12]
	CALL sys_writeKP
.endif  
NoWrite:
 POP_ALL @(is a macro)
	RET
sys_writeSLColors:

.ifndef W32 
	cmp r2,#0
	beq syswSL
	RET
syswSL: PUSH_ALL @(is a macro)
	mov r12,r2
	mov r2,r0
	mov r0,r12
	ldr r12,= scolorslen
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL WriteFile0
 POP_ALL @(is a macro)
.endif  
	RET
InputStringWithMessage0:
	A_DR r5,extext
InputStringWithMessage:
	CALL WriteMess9MakeLine
	A_DR r2,optbuffer
	ldr r12,= optslen
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	b InputString
InputString00:
	A_DR r2,suchtext
InputString0:
	CALL WriteMess9MakeLine
	ldr r3,= maxfilenamelen

InputString:
	stmfd r13!,{r2}
	stmfd r13!,{r4}
	mov r12,#0x2
	stmfd r13!,{r12}
	ldmfd r13!,{r0}
	A_DR r12,VICmdMode
	ldr r10,[r12]
	str r0,[r12]
	mov r0,r10
	stmfd r13!,{r0}
	A_DR r12,kurspos2
	ldr r12,[r12]
	stmfd r13!,{r12}
	A_DR r12,columns
	ldr r1,[r12]
.ifndef LINUX 
.endif  
	mov r9,r1
	ldr r10,= stdtxtlen
	sub r9,r9,r10
	mov r1,R9
	cmp r3,r1
	blo IS8
	mov r3,r1
IS8:	eors r1,r1,r1
	mov r4,r2
IS0:	stmfd r13!,{r1}
	stmfd r13!,{r3}
	stmfd r13!,{r2}
 PUSH_ALL @(is a macro)
	mov r5,r2
	A_DR r4,screenline+stdtxtlen
	mov r2,r1
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	mov r2,r3
	subs r2,r2,r1
	mov r12,#0x20
	bic r0,r0,#0xFF
	orr r0,r0,r12
@reploop:
	strb r0,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#20		@rep stosb
 POP_ALL @(is a macro)
	mov r1,r4
	subs r1,r1,r2
	ldr r12,= stdtxtlen
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
.ifdef UTF8 
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq noUTF_I
.endif  
	mov r5,r2
ISloopUTF8:
	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne ISdncUTF8
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r12,#0x1
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
ISdncUTF8:
	cmp r5,r4
	blo ISloopUTF8
noUTF_I:

.endif  
	A_DR r10,lines
	ldrB r12,[r10]
	bic r1,r1,#0xFF00
	orr r1,r1,r12,lsl #8
	A_DR r12,kurspos2
	str r1,[r12]
.ifdef LESSWRITEOPS 
	A_DR r12,screenbuffer
	mov r11,#0x0
	strB r11,[r12]
.endif  
	CALL StatusLineShow
	CALL GetChar
	ldmfd r13!,{r2}
	ldmfd r13!,{r3}
	ldmfd r13!,{r1}
	mov r7,#+1
	CALL IsViMode
	bne NO_VI01
	mov r12,#0x0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq ISA
NO_VI01:
	CALL CheckUserAbort
	bne IS9
ISA:	eors r1,r1,r1
IS1j:	b IS1
IS9:	ldr r12,= RETURN
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq IS1j
	mov r12,#0x8
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne IS2
DNHloopUTF8:

	cmp r4,r2
	beq IS0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq DNHloopUTF8
noUTF_J:

	b Delete1
.else  
.endif  
IS2:	mov r12,#0x0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NoSpecialKey
	mov r12,#0x5
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NotEnd
	mov r9,r2
	add r9,r9,r1
	mov r4,R9
IS0j:
.ifdef UTF8 
	b IS0
.else  
.endif  
NotEnd:	mov r12,#0x0
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NotHome
	mov r4,r2
	b IS0j
NotHome:
	mov r12,#0x3
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NotLeft
NHloopUTF8:

	cmp r4,r2
	beq IS0j
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq NHloopUTF8
noUTF_K:

.endif  
	b IS0j
NotLeft:
	mov r12,#0x4
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NotRight
	mov r9,r2
	add r9,r9,r1
	mov r5,R9
NLloopUTF8:

	cmp r4,r5
	beq IS0j
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq NLloopUTF8
noUTF_L:

.endif  
IS0jj:	b IS0j
NotRight:
	mov r12,#0x8
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NotIns
	A_DR r12,insstat
	ldrB r11,[r12]
	mvns r11,r11
	strB r11,[r12]
.ifdef NEW_CURSOR_MGNT 
	A_DR r12,insstat
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne NCM
	CALL SetCursorBlock
	b IS0j
NCM:	CALL SetCursorNormal
.endif  
	b IS0j
NotIns:	mov r12,#0x9
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NoSpecialKey
Delete1:
	mov r9,r2
	add r9,r9,r1
	mov r5,R9
	cmp r4,r5
	beq IS0jj
.ifdef UTF8 
	stmfd r13!,{r2}
	stmfd r13!,{r4}
	stmfd r13!,{r5}
	mov r2,r1
	mov r9,r4
	mov r10,#0x1
	add r9,r9,r10
	mov r5,R9
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
DeloopUTF8:

.ifdef UTF8RTSx_wont 
.endif  
	ldrB r12,[r5]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne DeUTF8
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b DeloopUTF8
noUTF_M:

DeUTF8:	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	ldmfd r13!,{r5}
	ldmfd r13!,{r4}
	ldmfd r13!,{r2}
.else  
.endif  
	b IS0jj
NoSpecialKey:

	ldr r12,= SPACECHAR
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo IS0jj
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	stmfd r13!,{r1}
	mov r12,r0
	and r12,r12,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	ldmfd r13!,{r1}
	beq INSrt
noUTF_N:

.endif  
	A_DR r12,insstat
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq INSrt
	mov r9,r2
	add r9,r9,r1
	mov r5,R9
	cmp r4,r5
	bne NO_INSERT
INSrt: PUSH_ALL @(is a macro)
	mov r0,r4
	mov r9,r2
	add r9,r9,r3
	mov r10,#0x1
	add r9,r9,r10
	mov r4,R9
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	mov r5,R9
	mov r2,r4
	subs r2,r2,r0
	mov r7,#-1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
 POP_ALL @(is a macro)
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
NO_INSERT:
	mov r7,#+1
	strb r0,[r4]
	add r4,r4,r7
.ifdef UTF8 
.ifdef UTF8RTSx_wont 
.endif  
	mov r5,r4
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
NI_loopUTF8:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	ldrB r12,[r5]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq NI_loopUTF8
	cmp r5,r4
	beq NI_UTF8rdy
 PUSH_ALL @(is a macro)
	mov r2,r1
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
 POP_ALL @(is a macro)
noUTF_O:

NI_UTF8rdy:

.endif  
	cmp r1,r3
	blo IS0j
IS1:	eors r0,r0,r0
	mov r9,r2
	add r9,r9,r1
	strB r0,[R9]
	ldmfd r13!,{r12}
	A_DR r11,kurspos2
	str r12,[r11]
	ldmfd r13!,{r12}
	A_DR r11,VICmdMode
	str r12,[r11]
	ldmfd r13!,{r4}
	ldmfd r13!,{r2}
	mov r12,r1
	mov r1,r0
	mov r0,r12
	mov r12,#0x1
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
ISready:
	RET
ReadChar:
	mov r0,r4
	A_DR r12,old
	ldr r10,[r12]
	str r0,[r12]
	mov r0,r10
	A_DR r12,veryold
	str r0,[r12]
GetChar:
	CALL ReadOneChar
.ifdef W32 
.endif  
	mov r12,#0x7f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne RC_No7F
.ifndef FREEBSD 
	mov r12,#0x8
	bic r0,r0,#0xFF
	orr r0,r0,r12
.else  
.endif  
RC_No7F:

.equ DoNo,10
	CALL IsViMode
	beq ISVI7
	mov r12,#0x1b
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne ISready
	CALL ReadOneChar
	b NOVI7
ISVI7:	A_DR r12,VICmdMode
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne NoCMDmode
	mov r12,#0x1b
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq ESCpressed
	ldr r12,= VIsize
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhi Other
	A_DR r1,VIcmdTable
	b RCready_0
ESCpressed:
	CALL ReadOneChar
	mov r12,#0x5b
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq Other
	b NoCursorKey
NoCMDmode:
	mov r12,#0x1b
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne ISready
	CALL KeyVImode1
.ifdef BEOS 
.else  
.ifdef SYS_select 
 PUSH_ALL @(is a macro)
	CALL Select
 POP_ALL @(is a macro)
	beq isSingleEscape
.endif  
.endif  
	CALL ReadOneChar
	mov r12,#0x5b
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq IsCursorKey
NoCursorKey:
	A_DR r12,VIbufch
	strB r0,[r12]
	ldr r12,= DoNo
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b JmpRCready
isSingleEscape:
	mov r12,#0x3
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b JmpRCready
IsCursorKey:
	CALL KeyVImode0
NOVI7:	A_DR r12,mode
	ldr r11,= NE
	ldrB r10,[r12]
	cmp r10,r11
	bne NONE7
	mov r12,#0x69
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NOi
	mov r12,#0x10
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b JmpRCready
NOi:	mov r12,#0x49
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NONE7
	mov r12,#0x10
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b JmpRCready
NONE7:	CALL IsEmMode
	bne NOEM7
	mov r12,#0x25
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NoAltPer
	mov r12,#0x28
	bic r0,r0,#0xFF
	orr r0,r0,r12
JmpRCready:
	b RCready_1
NoAltPer:
	mov r12,#0x3c
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NoAltLt
	mov r12,#0xe
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b RCready_1
NoAltLt:
	mov r12,#0x3e
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne NoAltGt
	mov r12,#0xf
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b RCready_1
NoAltGt:
	mov r12,#0x5f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x42
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bmi Other
	ldr r12,= ATsize
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhi Other
	A_DR r1,EmaAltTable
	b RCready_0
NOEM7:	mov r12,#0x5f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x48
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne Other
	mov r12,#0x3d
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b RCready_1
Other:
.ifdef W32 
.else  
	CALL ReadOneChar
	mov r12,#0x38
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhi NoNumber
	stmfd r13!,{r0}
	CALL ReadOneChar
	mov r12,r1
	mov r1,r0
	mov r0,r12
	ldmfd r13!,{r0}
	mov r12,#0x7e
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne GetCharJmp
NoNumber:
	mov r12,#0x30
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x9
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo IsNumber
.ifdef QNX 
.else  
	mov r12,#0x8
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
.endif  
	mov r12,#0x9
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo GetCharJmp
	ldr r12,= STsize
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhi GetCharJmp
IsNumber:
	A_DR r1,ScanTable
.endif  
RCready_0:
	and r12,r0,#0xFF
	ldrB r12,[r1,r12]
	bic r0,r0,#0xFF
	orr r0,r0,r12
RCready_1:
	mov r0,r0,lsl #8
	RET
GetCharJmp:
	b GetChar
ReadOneChar:
	CALL IsViMode
	bne NOVI4
	eors r0,r0,r0
	A_DR r12,VIbufch
	ldr r10,[r12]
	str r0,[r12]
	mov r0,r10
	orrs r0,r0,r0
	bne RoneC
NOVI4:	A_DR r2,read_b
	eors r3,r3,r3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL ReadFile0
.ifdef SELFTEST 
.endif  
	ldr r0,[r2]
.ifdef W32_EXTENDED_IO 
.endif  
RoneC:	mov r12,#0xff
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
	RET
LookBackward:

	stmfd r13!,{r2}
	stmfd r13!,{r1}
	eors r1,r1,r1
	mov r9,r4
	mov r10,#0x1
	sub r9,r9,r10
	ldr r11,= NEWLINE
	ldrB r10,[R9]
	cmp r10,r11
	beq LBa3
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	bne LBa1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LBa1:	mov r2,#0x9f
	mov r8,#0x86
	add r2,r2,r8,lsl #8
	mov r8,#0x1
	add r2,r2,r8,lsl #16

	ldr r12,= NEWLINE
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r7,#-1
	and r11,r0,#0xFF
@reploop:
	ldrb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	cmpne r12,r11
	subne pc,pc,#24		@rep scasb
	mov r9,r1
	mov r10,#0x9d
	mov r8,#0x86
	add r10,r10,r8,lsl #8
	mov r8,#0x1
	add r10,r10,r8,lsl #16
	add r9,r9,r10
	mov r0,R9
	subs r0,r0,r2
LBa5:	ldmfd r13!,{r1}
	ldmfd r13!,{r2}
	b CheckBof
LBa3:	eors r0,r0,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LBa5
LookForward:

	stmfd r13!,{r2}
	mov r2,#0x9f
	mov r8,#0x86
	add r2,r2,r8,lsl #8
	mov r8,#0x1
	add r2,r2,r8,lsl #16

	ldr r12,= NEWLINE
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r7,#+1
	and r11,r0,#0xFF
@reploop:
	ldrb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	cmpne r12,r11
	subne pc,pc,#24		@rep scasb
	mov r0,#0x9e
	mov r8,#0x86
	add r0,r0,r8,lsl #8
	mov r8,#0x1
	add r0,r0,r8,lsl #16

	subs r0,r0,r2
	ldmfd r13!,{r2}
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
CheckEof:
	cmp r4,r6
	bne CheckEnd
	b CheckENum
CheckBof:
	A_DR r11,sot-1
	cmp r4,r11
	bhi CheckEnd
CheckENum:
	A_DR r12,numeriere
	mov r11,#0x1
	strB r11,[r12]
CheckEnd:
	RET
LookPgBegin:
	A_DR r12,kurspos2
	ldr r3,[r12]
	mov r2,r3,lsl #16
	mov r2,r2,lsr #24
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LookPU2
LookPgEnd:
	A_DR r12,kurspos2
	ldr r3,[r12]
	A_DR r12,lines
	ldr r2,[r12]
	mov r12,r3,lsr #8
	and r12,r12,#0xFF
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r2,r2,#0xFF
	orr r2,r2,r11
	b LookPD2
LookLineUp:
	mov r12,#0x2
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,linenr
	mov r11,#0x1
	ldr r10,[r12]
	subs r10,r10,r11
	str r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LookPU2
LookLineDown:
	mov r12,#0x2
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	mrs r8,CPSR
	and r8,r8,#0x20000000
	A_DR r12,linenr
	mov r11,#0x1
	ldr r10,[r12]
	adds r10,r10,r11
	str r10,[r12]
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LookPD2
LookPageUp:
	A_DR r12,lines
	ldr r2,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LookPU1:
	A_DR r12,linenr
	ldr r10,[r12]
	subs r10,r10,r2
	str r10,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LookPU2:
	CALL LookBackward
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	blo LookPUEnd
	subs r2,r2,#1
	bne LookPU2
LookPUEnd:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	RET
LookScrDn:
	eors r2,r2,r2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LookPD1
LookScrUp:
	eors r2,r2,r2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b LookPU1
LookHalfPgUp:
	A_DR r12,lines
	ldr r2,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r2,r2,lsr #1
	b LookPU1
LookHalfPgDn:
	A_DR r12,lines
	ldr r2,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r2,r2,lsr #1
	b LookPD1
LookPgDown:
	A_DR r12,lines
	ldr r2,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LookPD1:
	A_DR r12,linenr
	ldr r10,[r12]
	adds r10,r10,r2
	str r10,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LookPD2:
	CALL LookForward
	beq LookPDEnd
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	subs r2,r2,#1
	bne LookPD2
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
LookPDEnd:
	subs r4,r4,r0
	RET
CheckBlock:
	A_DR r12,showblock
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	blo CheckBlockEnd
	A_DR r12,blockende
	ldr r5,[r12]
	A_DR r11,sot
	cmp r5,r11
	blo CheckBlockEnd
	A_DR r12,blockbegin
	ldr r5,[r12]
	A_DR r11,sot
	cmp r5,r11
	blo CheckBlockEnd
	A_DR r12,blockende
	ldr r10,[r12]
	cmp r10,r5
CheckBlockEnd:
	RET
CheckImBlock:
	A_DR r12,blockbegin
	ldr r10,[r12]
	cmp r10,r4
	bhi CImBlockEnd
	A_DR r12,blockende
	ldr r10,[r12]
	cmp r4,r10
CImBlockEnd:
	RET
CheckMode:
	ldr r11,= NEWLINE
	ldrB r10,[r4]
	cmp r10,r11
	beq ChModeEnd
	A_DR r12,insstat
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
ChModeEnd:
	RET
CheckMarker:

	cmp r4,r3
	bhi CMEnd
	cmp r1,r3
	blo CMEnd
	mov r3,r4
CMEnd:	RET
CountToLineEnd:
	stmfd r13!,{r4}
	CALL LookForward
	ldmfd r13!,{r4}
	RET
CountColToLineBeginVis:

	CALL CountToLineBegin
	stmfd r13!,{r5}
	eors r3,r3,r3
	mov r5,r4
	subs r5,r5,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
CCV1:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	cmp r5,r4
	bhs CCVend
.ifdef UTF8 
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq noUTF_P
.endif  
	ldrB r12,[r5]
	bic r1,r1,#0xFF
	orr r1,r1,r12
	mov r12,#0xc0
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r1,r1,#0xFF
	orr r1,r1,r11
	mov r12,#0x80
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq CCV1
noUTF_P:

.endif  
	ldr r11,= TABCHAR
	ldrB r10,[r5]
	cmp r10,r11
	beq CCVTab
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	b CCV1
CCVTab:	CALL SpacesForTab
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	mov r11,r3,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r3,r3,#0xFF
	orr r3,r3,r11
	b CCV1
CCVend:	A_DR r12,ch2linebeg
	str r3,[r12]
	mov r0,r3
	ldmfd r13!,{r5}
.ifdef W32LF 
.endif  
	RET
CountToLineBegin:
	stmfd r13!,{r4}
	CALL LookBackward
	mov r5,r4
	ldmfd r13!,{r4}
	RET
CountToWordBeginVIstyle:

	mov r5,r4
	ldr r11,= SPACECHAR
	ldrB r10,[r5]
	cmp r10,r11
	bhi CtWviStyle
CountToWordBegin:

	mov r5,r4
CountNLoop:
	cmp r5,r6
	beq CTWend
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef W32LF 
.else  
	ldr r11,= NEWLINE
	ldrB r10,[r5]
	cmp r10,r11
.endif  
	beq CTWend
	ldr r11,= SPACECHAR
	ldrB r10,[r5]
	cmp r10,r11
	bls CountNLoop
	mov r9,r5
	mov r10,#0x1
	sub r9,r9,r10
	mov r11,#0x2f
	ldrB r10,[R9]
	cmp r10,r11
	bhi CountNLoop
CTWend:	mov r0,r5
	subs r0,r0,r4
Goret:	RET
CtWviStyle:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef W32LF 
.else  
	ldr r11,= NEWLINE
	ldrB r10,[r5]
	cmp r10,r11
.endif  
	beq CTWend
	mov r11,#0x2f
	ldrB r10,[r5]
	cmp r10,r11
	bhi CtWviStyle
	b CountNLoop
KeyHelp:

.ifdef USE_BUILTINHELP 
	A_DR r12,kurspos
	ldr r12,[r12]
	stmfd r13!,{r12}
 PUSH_ALL @(is a macro)
	eors r0,r0,r0
	A_DR r12,showblock
	ldr r10,[r12]
	str r0,[r12]
	mov r0,r10
	stmfd r13!,{r0}
	mov r7,#+1
	A_DR r5,sot
	A_DR r4,buffercopy
	ldr r2,= buffercopysize

	stmfd r13!,{r4}
	stmfd r13!,{r2}
	stmfd r13!,{r5}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	CALL GetHelpText
	ldmfd r13!,{r4}
	stmfd r13!,{r4}
	stmfd r13!,{r4}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	A_DR r5,helpfoot
	ldr r12,= helpfootsize
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	mov r6,r4
	ldmfd r13!,{r4}
	CALL DispNewScreen
	CALL ReadOneChar
	ldmfd r13!,{r4}
	ldmfd r13!,{r2}
	ldmfd r13!,{r5}
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	ldmfd r13!,{r12}
	A_DR r11,showblock
	str r12,[r11]
 POP_ALL @(is a macro)
	ldmfd r13!,{r3}
	b SetKursPos
.else  
.endif  
GoUp:	eors r0,r0,r0
	b UpDown
GoDown:	A_DR r10,lines
	ldrB r12,[r10]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r12,#0xff
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
UpDown:	A_DR r12,kurspos2
	ldr r3,[r12]
	mov r12,r3,lsr #8
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq Goret
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	mov r11,r3,lsl #16
	mov r11,r11,lsr #24
	sbcs r11,r11,r12
	and r11,r11,#0xFF
	bic r3,r3,#0xFF00
	orr r3,r3,r11,lsl #8
	b SetKursPos
KeyVICmdz:
	CALL ReadOneChar
	mov r12,#0x2e
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyEmaCtrlL
	RET
KeyVI_M:
	CALL LookPgBegin
	CALL LookHalfPgDn
	A_DR r12,lines
	mov r11,#0x1
	ldrB r10,[r12]
	tst r10,r11
	bne KeyEmaCtrlL
	CALL LookLineDown
KeyEmaCtrlL:
	CALL CountToLineBegin
	A_DR r10,lines
	ldrB r12,[r10]
	bic r3,r3,#0xFF00
	orr r3,r3,r12,lsl #8
	mov r12,r3
	and r12,r12,#0xFF
	mov r12,r12,lsr #1
	and r12,r12,#0xFF
	bic r3,r3,#0xFF00
	orr r3,r3,r12,lsl #8
	mov r12,r0
	and r12,r12,#0xFF
	bic r3,r3,#0xFF
	orr r3,r3,r12
	b SetKursPos
KursorFirstLine:
	eors r3,r3,r3
	b SetKursPos
KursorLastLine:
	A_DR r10,lines
	ldrB r12,[r10]
	bic r3,r3,#0xFF00
	orr r3,r3,r12,lsl #8
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r12,#0x1
	mov r11,r3,lsl #16
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r3,r3,#0xFF00
	orr r3,r3,r11,lsl #8
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r12,#0x0
	bic r3,r3,#0xFF
	orr r3,r3,r12
	b SetKursPos
KursorStatusLine:
	A_DR r10,lines
	ldrB r12,[r10]
	bic r3,r3,#0xFF00
	orr r3,r3,r12,lsl #8
	ldr r12,= stdtxtlen
	bic r3,r3,#0xFF
	orr r3,r3,r12
	b SetKursPos
RestKursPos:
	A_DR r12,kurspos
	ldr r3,[r12]
SetKursPos:
	A_DR r12,kurspos2
	str r3,[r12]
sys_writeKP:
 PUSH_ALL @(is a macro)
.ifdef W32 
.else  
	CALL make_KPstr
	A_DR r2,setkp
	ldr r12,= setkplen
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL WriteFile0
.endif  
 POP_ALL @(is a macro)
	RET
.ifndef W32 
make_KPstr:
	mov r7,#+1
	A_DR r4,setkp
	mov r12,#0x1b
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
.ifndef ARMCPU 
.else  
	mov r12,#0x5b
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	mov r12,#0x30
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	strb r0,[r4]
	add r4,r4,r7
	strb r0,[r4]
	add r4,r4,r7
	mov r12,#0x3b
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	mov r12,#0x30
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	strb r0,[r4]
	add r4,r4,r7
	strb r0,[r4]
	add r4,r4,r7
.endif  
	mov r12,#0x48
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	mov r9,r4
	mov r10,#0x6
	sub r9,r9,r10
	mov r4,R9
	mov r0,r3,lsl #16
	mov r0,r0,lsr #24
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	stmfd r13!,{r3}
	CALL IntegerToAscii
	ldmfd r13!,{r3}
	A_DR r4,setkp+1+3+4
	mov r0,r3,lsl #24
	mov r0,r0,lsr #24
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.endif  
IntegerToAscii:

	orrs r0,r0,r0
	bpl ItoA1
	eors r0,r0,r0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
ItoA1:	mov r12,#0xa
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	mov r7,#-1
	mov r12,r1
	mov r1,r0
	mov r0,r12
Connum1:
	mov r12,r1
	mov r1,r0
	mov r0,r12
	orrs r0,r0,r0
	movpl r3,#0
	submi r3,r0,#1
	mov r9,r0
	mov r10,r2
	stmfd r13!,{r14}
	bl _DIV
	ldmfd r13!,{r14}
	mov r3,r9
	mov r0,r12

	mov r12,r1
	mov r1,r0
	mov r0,r12
	mov r12,r3
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xf
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x30
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	strb r0,[r4]
	add r4,r4,r7
	orrs r1,r1,r1
	bne Connum1
	mov r7,#+1
ITAret:	RET
DeleteByteCheckMarker:

	A_DR r12,mode
	mov r11,#0x9
	ldrB r10,[r12]
	tst r10,r11
	beq DeleteByte
	mov r9,r4
	add r9,r9,r0
	mov r1,R9
	A_DR r12,blockbegin
	ldr r3,[r12]
	CALL CheckMarker
	A_DR r12,blockbegin
	str r3,[r12]
	A_DR r12,blockende
	ldr r3,[r12]
	CALL CheckMarker
	A_DR r12,blockende
	str r3,[r12]
DeleteByte:
	orrs r0,r0,r0
	beq ITAret
.ifdef USE_UNDO 
.endif  
	stmfd r13!,{r4}
	mov r2,r6
	subs r2,r2,r4
	mov r9,r4
	add r9,r9,r0
	mov r5,R9
	subs r2,r2,r0
	A_DR r12,mode
	ldr r11,= WS
	ldrB r10,[r12]
	cmp r10,r11
	beq No_WS8
	A_DR r12,EmaKiSize
	ldr r10,[r12]
	adds r2,r2,r10
No_WS8:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r2,r2,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	rsbs r0,r0,#0
	b Ins0
Insert1Byte:
	eors r0,r0,r0
InsertByte0:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
InsertByte:
	orrs r0,r0,r0
	beq ITAret
	A_DR r12,maxlen
	ldr r2,[r12]
	A_DR r11,sot
	adds r2,r2,r11

	subs r2,r2,r6
	A_DR r12,EmaKiSize
	ldr r3,[r12]
	subs r2,r2,r3
	cmp r2,r0
	bhs SpaceAva
	ldr r12,= ERRNOMEM
	stmfd r13!,{r12}
	ldmfd r13!,{r12}
	A_DR r11,ErrNr
	str r12,[r11]
	CALL OSerror
	CALL RestKursPos
	mrs r12, CPSR
	and r12,r12,#0xDFFFFFFF
	msr CPSR_f,r12
	RET
SpaceAva:
	stmfd r13!,{r4}
.ifdef USE_UNDO 
.endif  
	mov r5,r6
	mov r9,r6
	mov r10,#0x1
	add r9,r9,r10
	mov r2,R9
	subs r2,r2,r4
	mov r9,r6
	add r9,r9,r0
	mov r4,R9
	A_DR r12,mode
	ldr r11,= WS
	ldrB r10,[r12]
	cmp r10,r11
	beq ISWS8
	adds r2,r2,r3
	adds r4,r4,r3
	adds r5,r5,r3
ISWS8:	mov r7,#-1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
Ins0:	ldmfd r13!,{r4}
	CALL SetChg
	adds r6,r6,r0
	A_DR r12,mode
	mov r11,#0x9
	ldrB r10,[r12]
	tst r10,r11
	beq NOWS8
	A_DR r12,blockende
	ldr r10,[r12]
	cmp r4,r10
	bhs Ins1
	A_DR r12,blockende
	ldr r10,[r12]
	adds r10,r10,r0
	str r10,[r12]
Ins1:	A_DR r12,blockbegin
	ldr r10,[r12]
	cmp r4,r10
	bhs Ins2
	A_DR r12,blockbegin
	ldr r10,[r12]
	adds r10,r10,r0
	str r10,[r12]
NOWS8:
	A_DR r12,mode
	mov r11,#0x6
	ldrB r10,[r12]
	tst r10,r11
	beq NO_EM02
	A_DR r12,EmaMark
	ldr r10,[r12]
	cmp r4,r10
	bhs Ins2
	A_DR r12,EmaMark
	ldr r10,[r12]
	adds r10,r10,r0
	str r10,[r12]
NO_EM02:

Ins2:	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
	RET
CopyBlock:
	CALL CheckBlock
	blo MoveBlEnd
	CALL CheckImBlock
	blo MoveBlEnd
	A_DR r12,blockende
	ldr r0,[r12]
	subs r0,r0,r5
	CALL InsertByte
	blo MoveBlEnd
	A_DR r12,blockbegin
	ldr r5,[r12]
MoveBlock:
	stmfd r13!,{r4}
	mov r2,r0
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	ldmfd r13!,{r4}
	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
MoveBlEnd:
	RET
KeyVICmdyy:
	stmfd r13!,{r4}
	CALL KeyHome
	A_DR r12,EmaMark
	str r4,[r12]
	CALL KeyEnd
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL KeyEmaAltW
	ldmfd r13!,{r4}
KviRet:	RET
KeyVICmdy:
	CALL ReadOneChar
	mov r12,#0x79
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq KeyVICmdyy
	mov r12,#0x27
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne MoveBlEnd
	CALL ReadOneChar
	mov r12,#0x61
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne MoveBlEnd
	A_DR r12,blockbegin
	ldr r2,[r12]
	cmp r2,#0
	beq MoveBlEnd
	CALL VIsetMarker
	CALL KeyEmaAltW
	A_DR r12,blockbegin
	ldr r4,[r12]
.ifdef W32 
.else  
	b ISVI9
.endif  
KeyEmaCtrlY:

.ifdef W32 
.endif  
	A_DR r12,EmaKiSize
	ldr r2,[r12]
.ifdef YASM 
.else  
	cmp r2,#0
	beq KeawRet
.endif  
	mov r12,r2
	mov r2,r0
	mov r0,r12
	stmfd r13!,{r0}
	CALL InsertByte
	ldmfd r13!,{r2}
	blo KeawRet
	mov r5,r6
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r5,r5,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,EmaMark
	str r4,[r12]
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	CALL ShowBl0
	CALL IsViMode
	beq ISVI9
	CALL KeyEmaCtrlL
ISVI9:	b CQFNum
KeyEmaAltW2:
 PUSH_ALL @(is a macro)
	mov r4,r6
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL IsViMode
	beq KEW
	A_DR r12,EmaCtrlK
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne KEW
	A_DR r12,EmaKiSize
	ldr r10,[r12]
	adds r4,r4,r10
	A_DR r12,EmaKiSize
	ldr r10,[r12]
	adds r10,r10,r0
	str r10,[r12]
	b KE2
KEW:	A_DR r12,EmaKiSize
	str r0,[r12]
	A_DR r12,EmaKiSrc
	str r5,[r12]
KE2:	mov r2,r0
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	CALL ShowBl0
Keaw2:	A_DR r12,EmaCtrlK
	mov r11,#0x1
	strB r11,[r12]
 POP_ALL @(is a macro)
KeawRet:
	RET
KeyEmaAltW:
	A_DR r12,VInolinebased
	mov r11,#0x0
	strB r11,[r12]
 PUSH_ALL @(is a macro)
	A_DR r12,showblock
	ldr r2,[r12]
	cmp r2,#0
	beq Keaw2
	A_DR r12,EmaMark
	ldr r2,[r12]
	cmp r2,#0
	beq Keaw2
	mov r0,r4
	cmp r2,r0
	blo KEAW
	mov r12,r2
	mov r2,r0
	mov r0,r12
KEAW:	subs r0,r0,r2
	mov r5,r2
	mov r4,r6
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,EmaKiSize
	str r0,[r12]
	A_DR r12,EmaKiSrc
	str r5,[r12]
	mov r12,r2
	mov r2,r0
	mov r0,r12
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	CALL IsViMode
	beq KEAW3
	CALL ShowBl0
KEAW3:
.ifdef W32 
.endif  
 POP_ALL @(is a macro)
KeaWRet:
	RET
NFnoarg:
	A_DR r5,helptext
	A_DR r4,sot
	ldr r12,= helptextsize
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	stmfd r13!,{r4}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	CALL GetHelpText
	mov r9,r2
	A_DR r10,sot
	add r9,r9,r10
	ldr r10,= helptextsize
	add r9,r9,r10
	mov r6,R9
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
	ldmfd r13!,{r4}
	CALL DispNewScreen
	A_DR r5,filename
	A_DR r2,filepath
	CALL InputString0
	bhs GetFile
	RET
KeyVICmdE:
	mov r9,r2
	mov r10,#0x2
	add r9,r9,r10
	mov r5,R9
	ldr r11,= SPACECHAR
	ldrB r10,[r5]
	cmp r10,r11
	beq KeaWRet
 PUSH_ALL @(is a macro)
	CALL SaveFile
 POP_ALL @(is a macro)
NewFile:
	mov r7,#+1
	CALL InitVars
.ifdef AMD64 
.else  
	orrs r5,r5,r5
	beq NFnoarg
	mov r11,#0x0
	ldrB r10,[r5]
	cmp r10,r11
.endif  
	beq NFnoarg
	A_DR r4,filepath
NF1:
.ifdef W32 
.else  
	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
.endif  
NF2:	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne NF1
GetFile:

.ifdef BEOS 
.else  
	A_DR r1,filepath
.endif  
	CALL OpenFile0
	A_DR r4,sot
	mov r6,r4
	bmi NewFileEnd
.ifdef SYS_brk 
	CALL Seek
 PUSH_ALL @(is a macro)
	mov r9,r0
	add r9,r9,r0
	ldr r10,= max
	add r9,r9,r10
	mov r1,R9
	A_DR r12,maxlen
	str r1,[r12]
	A_DR r11,text
	adds r1,r1,r11

	CALL SysBrk
 POP_ALL @(is a macro)
	bmi OSejmp1
.else  
.endif  
.ifdef SYS_fstat 
	CALL Fstat
	bmi OSejmp1
	A_DR r12,fstatbuf+stat_struc.st_mode
	ldr r0,[r12]
.ifdef FREEBSD 
.endif  
	mov r11,#0xff
	mov r8,#0x1
	add r11,r11,r8,lsl #8
	ands r0,r0,r11
	A_DR r12,perms
	str r0,[r12]
.ifdef SYS_utime 
	A_DR r12,fstatbuf+stat_struc.st_mtime
	ldr r0,[r12]
	A_DR r12,accesstime+utimbuf_struc.modtime
	str r0,[r12]
.endif  
.endif  
OldFile1:
	A_DR r12,maxlen
	ldr r3,[r12]
	mov r2,r4
	CALL Read_File
	mov r12,r0
	mov r0,r3
	mov r3,r12
	bmi OSejmp0
	CALL CloseFile
OSejmp1:
	bmi OSejmp0
	mov r9,r3
	A_DR r10,sot
	add r9,r9,r10
	mov r6,R9
NewFileEnd:
	ldr r11,= NEWLINE
	strB r11,[r6]
	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
NFEnd2:	RET
SaveFile:
	A_DR r12,changed
	ldr r11,= UNCHANGED
	ldrB r10,[r12]
	cmp r10,r11
	beq NFEnd2
	A_DR r5,filesave
	CALL WriteMess9
	A_DR r5,filepath
	stmfd r13!,{r4}
	A_DR r4,bakpath
	mov r1,r5
	mov r2,r4
	mov r7,#+1
SF0:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne SF0
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mov r12,#0x7e
	stmfd r13!,{r12}
	ldmfd r13!,{r0}
.ifdef ARMCPU 
	strb r0,[r4]
	add r4,r4,r7
	mov r0,r0,lsr #8
	strb r0,[r4]
	add r4,r4,r7
	mov r0,r0,lsr #8
	strb r0,[r4]
	add r4,r4,r7
	mov r0,r0,lsr #8
	strb r0,[r4]
	add r4,r4,r7
.else  
.endif  
	ldmfd r13!,{r4}
.ifdef BEOS 
.endif  
.ifdef MAKE_BACKUP 
	mov r11,#0x2f
	mov r8,#0x74
	add r11,r11,r8,lsl #8
	mov r8,#0x6d
	add r11,r11,r8,lsl #16
	mov r8,#0x70
	add r11,r11,r8,lsl #24
	ldr r10,[r2]
	cmp r10,r11
	beq no_ren
.ifdef SYS_readlink 
 PUSH_ALL @(is a macro)
	A_DR r2,linkbuffer
	ldr r12,= linkbuffersize
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL ReadLink
 POP_ALL @(is a macro)
	bpl CopyBAK
	CALL RenameFile
	b no_ren
CopyBAK:
	CALL CopyToBackup
.else  
.endif  
no_ren:
.endif  
.ifdef BEOS 
.endif  
.ifdef W32 
.else  
.ifdef BEOS 
.else  
	ldr r2,= O_WRONLY_CREAT_TRUNC

	A_DR r12,perms
	ldr r3,[r12]
.endif  
.endif  
	CALL OpenFile
OSejmp0:
	bmi OSejmp9
	mov r12,r0
	mov r0,r1
	mov r1,r12
.ifdef SYS_fchown 
	A_DR r12,perms
	ldr r2,[r12]
	CALL Fchmod
.endif  
.ifdef SYS_fstat 
	A_DR r12,fstatbuf+stat_struc.st_uid
	ldr r2,[r12]
.ifdef UIDGID_WORD 
	mov r3,r2
	mov r3,r3,lsr #16
	mov r2,r2,lsl #16
	mov r2,r2,lsr #16
.else  
.endif  
	CALL ChownFile
.endif  
	A_DR r2,sot
	mov r3,r6
SaveFile2:
	subs r3,r3,r2
	CALL IsViMode
	bne NoAddNL
	mov r9,r6
	mov r10,#0x1
	sub r9,r9,r10
	ldr r11,= NEWLINE
	ldrB r10,[R9]
	cmp r10,r11
	beq NoAddNL
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
NoAddNL:
	CALL Write_File
OSejmp9:
	bmi OSejmp
	ldr r12,= ERRNOIO
	stmfd r13!,{r12}
	ldmfd r13!,{r12}
	A_DR r11,ErrNr
	str r12,[r11]
	cmp r0,r3
.ifdef BEOS 
.else  
	bne OSerror
.endif  
	CALL CloseFile
	bmi OSejmp
SaveFile3:
	RET
SaveBlock:
	CALL GetBlockName
	blo jcDE2
SaveBl2:

.ifdef W32 
.else  
.ifdef BEOS 
.else  
	A_DR r1,blockpath
SaveBl3:
	ldr r2,= O_WRONLY_CREAT_TRUNC

	ldr r3,= PERMS

.endif  
.endif  
	CALL OpenFile
	bmi OSejmp
	mov r2,r5
	A_DR r12,blockende
	ldr r3,[r12]
	mov r12,r0
	mov r0,r1
	mov r1,r12
	b SaveFile2
ReadBlock:
	CALL GetBlockName
jcDE2:	blo DE2
ReadBlock2:

.ifdef BEOS 
.else  
	A_DR r1,blockpath
.endif  
	CALL OpenFile0
OSejmp:	bmi OSerror
	CALL Seek
	bmi OSerror
	stmfd r13!,{r0}
	CALL InsertByte
	ldmfd r13!,{r3}
	blo SaveFile3
	mov r2,r4
	CALL Read_File
	bmi preOSerror
	mov r2,r0
	CALL CloseFile
	bmi OSerror
	ldr r12,= ERRNOIO
	stmfd r13!,{r12}
	ldmfd r13!,{r12}
	A_DR r11,ErrNr
	str r12,[r11]
	cmp r3,r2
	bne OSerror
	RET
preOSerror:
	mov r0,r3
	CALL DeleteByte
OSerror:
	stmfd r13!,{r4}
	A_DR r4,error+8
	A_DR r12,ErrNr
	ldr r0,[r12]
	stmfd r13!,{r0}
	CALL IntegerToAscii
	ldmfd r13!,{r2}
	ldr r11,= MAXERRNO
	cmp r2,r11
	bhi DE0
	A_DR r4,errmsgs
	CALL LookPD2
	mov r5,r4
	A_DR r4,error+9
	mov r12,#0x20
	mov r8,#0x3a
	add r12,r12,r8,lsl #8
	mov r0,r0,lsr #16
	mov r0,r0,lsl #16
	orr r0,r0,r12
.ifdef ARMCPU 
	strb r0,[r4]
	add r4,r4,r7
.else  
.endif  
	mov r12,#0x50
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
DE0:	A_DR r5,error
	ldmfd r13!,{r4}
DE1:	CALL WriteMess9
	CALL ReadOneChar
DE2:
RestoreStatusLine:
 PUSH_ALL @(is a macro)
	CALL InitStatusLine
	A_DR r5,mode
	A_DR r12,columns
	ldr r2,[r12]
	mov r12,#0x20
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
.ifdef ARMCPU 
	mov r11,#0xfc
	mov r8,#0xff
	add r11,r11,r8,lsl #8
	mov r8,#0xff
	add r11,r11,r8,lsl #16
	mov r8,#0xff
	add r11,r11,r8,lsl #24
	ands r2,r2,r11
.endif  
	blo RSno_lineNr
	A_DR r10,changed
	ldrB r12,[r10]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	A_DR r12,screenline+1
	strB r0,[r12]
.ifndef LINUX 
.else  
	mov r9,r2
	mov r10,#0xc
	sub r9,r9,r10
	A_DR r10,screenline
	add r9,r9,r10
	mov r0,R9
.endif  
	mov r9,r0
	mov r10,#0x8
	add r9,r9,r10
	mov r11,#0x76
	mov r8,#0x69
	add r11,r11,r8,lsl #8
	strH r11,[R9]
	ldr r11,= VI
	ldrB r10,[r5]
	cmp r10,r11
	bne RSL0
	A_DR r12,VICmdMode
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	bne NOVI0
	mov r1,#0x43
	mov r8,#0x4d
	add r1,r1,r8,lsl #8
	mov r8,#0x44
	add r1,r1,r8,lsl #16
	mov r8,#0x20
	add r1,r1,r8,lsl #24

	b RSL1
RSL0:	mov r11,#0x61
	mov r8,#0x6c
	add r11,r11,r8,lsl #8
	mov r8,#0x74
	add r11,r11,r8,lsl #16
	mov r8,#0x48
	add r11,r11,r8,lsl #24
	str r11,[r0]
	mov r9,r0
	mov r10,#0x4
	add r9,r9,r10
	mov r11,#0x3d
	mov r8,#0x68
	add r11,r11,r8,lsl #8
	mov r8,#0x65
	add r11,r11,r8,lsl #16
	mov r8,#0x6c
	add r11,r11,r8,lsl #24
	str r11,[R9]
	A_DR r1,editmode
	ldr r3,[r1]
	ldr r11,= PI
	ldrB r10,[r5]
	cmp r10,r11
	bne No_PI1
	mov r9,r1
	mov r10,#0x4
	add r9,r9,r10
	ldr r3,[R9]
No_PI1:	ldr r11,= EM
	ldrB r10,[r5]
	cmp r10,r11
	bne No_Em1
	mov r9,r1
	mov r10,#0x8
	add r9,r9,r10
	ldr r3,[R9]
No_Em1:	ldr r11,= NE
	ldrB r10,[r5]
	cmp r10,r11
	bne No_Ne1
	mov r9,r1
	mov r10,#0xc
	add r9,r9,r10
	ldr r3,[R9]
No_Ne1:	mov r9,r0
	mov r10,#0x8
	add r9,r9,r10
	str r3,[R9]
NOVI0:	mov r0,#0x20
	mov r8,#0x49
	add r0,r0,r8,lsl #8
	mov r8,#0x4e
	add r0,r0,r8,lsl #16
	mov r8,#0x53
	add r0,r0,r8,lsl #24

	A_DR r12,insstat
	mov r11,#0x1
	ldrB r10,[r12]
	cmp r10,r11
	beq RSL1
	mov r0,#0x20
	mov r8,#0x4f
	add r0,r0,r8,lsl #8
	mov r8,#0x56
	add r0,r0,r8,lsl #16
	mov r8,#0x52
	add r0,r0,r8,lsl #24

RSL1:	A_DR r12,screenline+4
	str r0,[r12]
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	bne RSL1a
	mov r12,#0x37
	bic r0,r0,#0xFF
	orr r0,r0,r12
RSL1a:	A_DR r12,screenline
	strB r0,[r12]
.endif  
	A_DR r4,screenline+stdtxtlen
	mov r9,r2
	mov r10,#0x1e
	sub r9,r9,r10
	mov r2,R9
	A_DR r5,filepath
RSL2:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	beq RSL4
	strb r0,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	bne RSL2
RSL4:	A_DR r4,screenline-15
	A_DR r12,columns
	ldr r10,[r12]
	adds r4,r4,r10
	A_DR r12,columne
	ldr r0,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	CALL IntegerToAscii
	mov r11,#0x3a
	strB r11,[r4]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,linenr
	ldr r0,[r12]
	CALL IntegerToAscii
RSno_lineNr:
	CALL StatusLineShow
 POP_ALL @(is a macro)
	mrs r12, CPSR
	and r12,r12,#0xDFFFFFFF
	msr CPSR_f,r12
	RET
WriteMess9MakeLine:
	CALL InitStatusLine
WriteMess9:
 PUSH_ALL @(is a macro)
	A_DR r4,screenline
	mov r7,#+1
WriteMLoop:
	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	ldr r12,= LINEFEED
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bls WriteMEnd
	strb r0,[r4]
	add r4,r4,r7
	b WriteMLoop
WriteMEnd:
	CALL StatusLineShow
 POP_ALL @(is a macro)
	b KursorStatusLine
InitStatusLine:
 PUSH_ALL @(is a macro)
	A_DR r4,screenline
	ldr r12,= SPACECHAR
	bic r0,r0,#0xFF
	orr r0,r0,r12
	A_DR r12,columns
	ldr r2,[r12]
.ifndef LINUX 
.endif  
	mov r7,#+1
@reploop:
	strb r0,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#20		@rep stosb
	mov r12,#0x0
	bic r0,r0,#0xFF
	orr r0,r0,r12
	strb r0,[r4]
	add r4,r4,r7
ISL: POP_ALL @(is a macro)
	RET
GetBlockName:
 PUSH_ALL @(is a macro)
	A_DR r5,block
	A_DR r2,blockpath
	CALL InputString0
	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	CALL RestKursPos
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
 POP_ALL @(is a macro)
	RET
InitVars:
	A_DR r12,text
	ldr r11,= NEWLINE
	strB r11,[r12]
	CALL Unchg
	CALL InitSomeVars
	A_DR r12,old
	A_DR r11,sot
	str r11,[r12]
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r12,VICmdMode
	strB r0,[r12]
	A_DR r12,linenr
	str r0,[r12]
	A_DR r12,insstat
	strB r0,[r12]
	A_DR r12,maxlen
	ldr r11,= max
	str r11,[r12]
	A_DR r12,error
	mov r11,#0x45
	mov r8,#0x52
	add r11,r11,r8,lsl #8
	mov r8,#0x52
	add r11,r11,r8,lsl #16
	mov r8,#0x4f
	add r11,r11,r8,lsl #24
	str r11,[r12]
	A_DR r12,error+4
	mov r11,#0x52
	mov r8,#0x20
	add r11,r11,r8,lsl #8
	mov r8,#0x20
	add r11,r11,r8,lsl #16
	mov r8,#0x20
	add r11,r11,r8,lsl #24
	str r11,[r12]
	A_DR r12,perms
	ldr r11,= PERMS
	str r11,[r12]
.ifdef SYS_fstat 
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r0,r0,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UIDGID_WORD 
	A_DR r12,fstatbuf+stat_struc.st_uid
	str r0,[r12]
.else  
.endif  
.endif  
	b ShowBl1
InitSomeVars:

	eors r0,r0,r0
.ifdef USE_UNDO 
.endif  
	A_DR r12,EmaMark
	str r0,[r12]
	A_DR r12,oldQFpos
	str r0,[r12]
	A_DR r12,bereitsges
	strB r0,[r12]
	A_DR r12,endeedit
	strB r0,[r12]
InitSV1:
	A_DR r12,EmaKiSize
	str r0,[r12]
InitSV2:
	A_DR r12,blockbegin
	str r0,[r12]
InitSV3:
	A_DR r12,blockende
	str r0,[r12]
	RET
Seek:	mov r12,r0
	mov r0,r1
	mov r1,r12
	mov r12,#0x2
	stmfd r13!,{r12}
	ldmfd r13!,{r3}
	CALL SeekFile
	bmi SeekRet
	eors r3,r3,r3
	stmfd r13!,{r0}
	CALL SeekFile
	ldmfd r13!,{r0}
SeekRet:
	RET
AskForReplace:
	A_DR r5,askreplace1
	CALL InputString00
	blo AskFor_Ex
	A_DR r12,suchlaenge
	str r0,[r12]
	A_DR r5,askreplace2
	A_DR r2,replacetext
	CALL InputString0
	b GetOptions
AskForFind:
	A_DR r5,askfind
	CALL InputString00
	blo AskFor_Ex
GetOptions:
	A_DR r12,repllaenge
	str r0,[r12]
	A_DR r12,mode
	mov r11,#0x11
	ldrB r10,[r12]
	tst r10,r11
	beq GetOpt2
	A_DR r5,optiontext
	CALL InputStringWithMessage
	CALL ParseOptions
GetOpt2:
	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
AskFor_Ex:
	bhs AFE2
	A_DR r12,bereitsges
	mov r11,#0x0
	strB r11,[r12]
AFE2:	mrs r12, CPSR
	stmfd r13!,{r7,r12}
	CALL RestKursPos
	ldmfd r13!,{r7,r12}
	msr CPSR_f,r12
	RET
ParseOptions:
	stmfd r13!,{r5}
	mov r7,#+1
	A_DR r5,optbuffer
	mov r12,#0x1
	stmfd r13!,{r12}
	ldmfd r13!,{r12}
	A_DR r11,vorwarts
	str r12,[r11]
	A_DR r12,grossklein
	mov r11,#0xdf
	strB r11,[r12]
Scan1:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0x5f
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x43
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne notCopt
	A_DR r12,grossklein
	mov r11,#0x20
	ldrB r10,[r12]
	eors r10,r10,r11
	strB r10,[r12]
notCopt:
	mov r12,#0x42
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bne notBopt
	A_DR r12,vorwarts
	ldr r11,[r12]
	rsbs r11,r11,#0
	str r11,[r12]
notBopt:
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bne Scan1
	ldmfd r13!,{r5}
	RET
find2:	mov r1,r4
find3:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,r0
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	orrs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	beq found
	mov r12,#0x41
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo find7
	mov r12,r2,lsr #8
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
find7:	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	ldrB r12,[r4]
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r12,#0x41
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo find10
	mov r12,r2,lsr #8
	and r12,r12,#0xFF
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r2,r2,#0xFF
	orr r2,r2,r11
find10:	mov r12,r2
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq find3
	mov r4,r1
FindText:
	A_DR r10,grossklein
	ldrB r12,[r10]
	bic r2,r2,#0xFF00
	orr r2,r2,r12,lsl #8
	A_DR r5,suchtext
	mov r7,#+1
	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0x41
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo find1
	mov r12,r2,lsr #8
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
find1:	A_DR r12,vorwarts
	ldr r10,[r12]
	adds r4,r4,r10
	ldrB r12,[r4]
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r12,#0x41
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	blo find6
	mov r12,r2,lsr #8
	and r12,r12,#0xFF
	mov r11,r2,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r2,r2,#0xFF
	orr r2,r2,r11
find6:	mov r12,r2
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq find2
	A_DR r12,mode
	ldr r11,= PI
	ldrB r10,[r12]
	cmp r10,r11
	bne find_WS
	A_DR r12,PicoSearch
	ldr r10,[r12]
	cmp r4,r10
	beq notfound
	cmp r4,r6
	blo find1
	A_DR r4,sot-1
	b find1
find_WS:
	cmp r4,r6
	bhi notfound
find9:	A_DR r11,sot
	cmp r4,r11
	bhs find1
notfound:
	mrs r12, CPSR
	and r12,r12,#0xDFFFFFFF
	msr CPSR_f,r12
	RET
found:	mov r4,r1
	mrs r12, CPSR
	orr r12,r12,#0x20000000
	msr CPSR_f,r12
	RET
GetOctalToInteger:
	mov r12,#0x7
	stmfd r13!,{r12}
	b GATI2
GetAsciiToInteger:
	mov r12,#0x9
	stmfd r13!,{r12}
GATI2:	CALL IsViMode
	beq ISVI8
	CALL InputStringWithMessage
	CALL AskFor_Ex
ISVI8:	mov r12,#0x0
	stmfd r13!,{r12}
	ldmfd r13!,{r5}
	ldmfd r13!,{r1}
	mov r12,r5
	mov r5,r2
	mov r2,r12
	blo AIexit2
	mov r7,#+1
AIload:	ldrb r12,[r5]
	add r5,r5,r7
	and r12,r12,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0x30
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	subs r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	bmi AIexit
	mov r12,r1
	and r12,r12,#0xFF
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	bhi AIexit
	mov r12,#0x7
	mov r11,r1,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq GATI3
	mov r9,r2
	mov r10,#0x4
	mov r8,r9
	mul r9,r8,r10
	add r9,r9,r2
	mov r2,R9
	mov r9,r2
	mov r10,#0x2
	mov r8,r9
	mul r9,r8,r10
	add r9,r9,r0
	mov r2,R9
	b AIload
GATI3:	mov r9,r2
	mov r10,#0x8
	mov r8,r9
	mul r9,r8,r10
	add r9,r9,r0
	mov r2,R9
	b AIload
.ifdef ARMCPU 
AIexit:	mov r11,#0x0
	cmp r2,r11
.else  
.endif  
AIexit2:
	RET
SpacesForTab:
	stmfd r13!,{r2}
	mov r0,r3
	ldr r12,= TAB
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	eors r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r11,lsl #8
	mov r9,r0
	and r10,r2,#0xFF
	stmfd r13!,{r14}
	bl _DIV
	ldmfd r13!,{r14}
	and r0,r12,#0xFF
	and r9,r9,#0xFF
	orr r0,r0,r9,lsl #8

	mov r12,r0,lsr #8
	and r12,r12,#0xFF
	rsbs r12,r12,#0
	mov r12,r12,asl #24
	mov r12,r12,asr #24
	and r12,r12,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
	ldr r12,= TAB
	mov r11,r0,lsl #16
	mov r11,r11,lsr #24
	adds r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF00
	orr r0,r0,r11,lsl #8
	ldmfd r13!,{r2}
	RET
GetHelpText:

	A_DR r5,help_ne
	ldr r2,= help_ws_size

.ifdef USE_BUILTINHELP 
	A_DR r0,mode
	ldr r11,= NE
	ldrB r10,[r0]
	cmp r10,r11
	bne NoNe1
	ldr r2,= help_ne_size

	RET
NoNe1:	subs r5,r5,r2
	ldr r11,= VI
	ldrB r10,[r0]
	cmp r10,r11
	beq GHT
	subs r5,r5,r2
	ldr r11,= EM
	ldrB r10,[r0]
	cmp r10,r11
	beq GHT
	subs r5,r5,r2
	ldr r11,= PI
	ldrB r10,[r0]
	cmp r10,r11
	beq GHT
	subs r5,r5,r2
.endif  
GHT:	RET
CheckUserAbort:
	A_DR r5,mode
	ldr r11,= WS
	ldrB r10,[r5]
	cmp r10,r11
	beq CUAWS
	ldr r11,= EM
	ldrB r10,[r5]
	cmp r10,r11
	beq CUAEM
	mov r12,#0x3
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	RET
CUAWS:	mov r12,#0x15
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	RET
CUAEM:	mov r12,#0x7
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	RET
KeyEditMode:

	A_DR r5,modetxt
	CALL InputStringWithMessage
	CALL RestKursPos
	A_DR r5,optbuffer
	CALL InitSomeVars
SetEditMode:
	A_DR r0,mode
.ifndef ARMCPU 
.ifdef AMD64 
.else  
.endif  
.else  
	mov r9,r5
	mov r10,#0x3
	add r9,r9,r10
	ldrB r12,[R9]
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r2,r2,lsl #8
	mov r9,r5
	mov r10,#0x2
	add r9,r9,r10
	ldrB r12,[R9]
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r2,r2,lsl #8
	mov r9,r5
	mov r10,#0x1
	add r9,r9,r10
	ldrB r12,[R9]
	bic r2,r2,#0xFF
	orr r2,r2,r12
	mov r2,r2,lsl #8
	ldrB r12,[r5]
	bic r2,r2,#0xFF
	orr r2,r2,r12
.endif  
.ifdef W32 
.endif  
	mov r11,#0x65
	mov r8,#0x33
	add r11,r11,r8,lsl #8
	mov r8,#0x6e
	add r11,r11,r8,lsl #16
	mov r8,#0x65
	add r11,r11,r8,lsl #24
	cmp r2,r11
	bne NoNe
	ldr r11,= NE
	strB r11,[r0]
	RET
NoNe:	mov r11,#0x65
	mov r8,#0x33
	add r11,r11,r8,lsl #8
	mov r8,#0x65
	add r11,r11,r8,lsl #16
	mov r8,#0x6d
	add r11,r11,r8,lsl #24
	cmp r2,r11
	bne NoEm
	ldr r11,= EM
	strB r11,[r0]
	RET
NoEm:	mov r11,#0x65
	mov r8,#0x33
	add r11,r11,r8,lsl #8
	mov r8,#0x70
	add r11,r11,r8,lsl #16
	mov r8,#0x69
	add r11,r11,r8,lsl #24
	cmp r2,r11
	bne NoPi
	ldr r11,= PI
	strB r11,[r0]
	RET
NoPi:	mov r11,#0x65
	mov r8,#0x33
	add r11,r11,r8,lsl #8
	mov r8,#0x76
	add r11,r11,r8,lsl #16
	mov r8,#0x69
	add r11,r11,r8,lsl #24
	cmp r2,r11
	bne NoVi
	ldr r11,= VI
	strB r11,[r0]
	RET
NoVi:	mov r11,#0x65
	mov r8,#0x33
	add r11,r11,r8,lsl #8
	mov r8,#0x77
	add r11,r11,r8,lsl #16
	mov r8,#0x73
	add r11,r11,r8,lsl #24
	cmp r2,r11
	bne modeOK
	ldr r11,= WS
	strB r11,[r0]
modeOK:	RET
.ifdef USE_EXT_MOVE 
.endif  
.ifdef SYS_kill 
SigHandler:
	CALL RestKursPos
	A_DR r4,screenbuffer
	ldr r2,= screenbuffer_dwords

	mov r7,#+1
@reploop:
	str r0,[r4]
	add r4,r4,r7, lsl #2
	subs r2,r2,#1
	subne pc,pc,#20		@rep stosd
.endif  
SetTermStruc:

.ifdef W32 
.else  
	ldr r2,= TERMIOS_GET

	CALL IOctlTerminal0
	mov r5,r3
	A_DR r4,termios
	mov r3,r4
	ldr r12,= termios_struc_size
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	mov r7,#+1
	cmps r2,#0
	addeq pc,pc,#16
@reploop:
	ldrb r12,[r5]
	add r5,r5,r7
	strb r12,[r4]
	add r4,r4,r7
	subs r2,r2,#1
	subne pc,pc,#28		@rep movsb
.ifdef LINUX 
	mov r9,r3
	A_DR r10,termios_struc.c_cc
	add r9,r9,r10
	ldr r10,= VMIN
	add r9,r9,r10
	mov r11,#0x1
	strB r11,[R9]
.endif  
.ifdef ARMCPU 
	mov r9,r3
	A_DR r10,termios_struc.c_lflag
	add r9,r9,r10
	mov r10,#0x0
	add r9,r9,r10
	mov r11,#0xf4
	mov r8,#0xff
	add r11,r11,r8,lsl #8
	mov r8,#0xff
	add r11,r11,r8,lsl #16
	mov r8,#0xff
	add r11,r11,r8,lsl #24
	ldrB r10,[R9]
	ands r10,r10,r11
	strB r10,[R9]
.else  
.endif  
	mov r9,r3
	A_DR r10,termios_struc.c_iflag
	add r9,r9,r10
	mov r10,#0x1
	add r9,r9,r10
	mov r11,#0xfa
	mov r8,#0xff
	add r11,r11,r8,lsl #8
	mov r8,#0xff
	add r11,r11,r8,lsl #16
	mov r8,#0xff
	add r11,r11,r8,lsl #24
	ldrB r10,[R9]
	ands r10,r10,r11
	strB r10,[R9]
	ldr r2,= TERMIOS_SET

	b IOctlTerminal
.endif  
.ifdef NEW_CURSOR_MGNT 
SetCursorNormal:
 PUSH_ALL @(is a macro)
	A_DR r2,normcurs
	ldr r12,= normcurslen
	stmfd r13!,{r12}
	b SCB
SetCursorBlock:
 PUSH_ALL @(is a macro)
	A_DR r2,blockcurs
	ldr r12,= blockcurslen
	stmfd r13!,{r12}
SCB:	ldmfd r13!,{r3}
	CALL WriteFile0
 POP_ALL @(is a macro)
	RET
.endif  
.ifndef W32 
IOctlTerminal0:
	A_DR r3,termios_orig
IOctlTerminal:
	ldr r1,= stdin

.ifdef LIBC 
.else  
	ldr r12,= SYS_ioctl
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
ReadFile0:

.ifdef W32 
.ifdef W32_EXTENDED_IO 
.else  
.endif  
.else  
	eors r1,r1,r1
.endif  
Read_File:

.ifdef W32 
.else  
.ifdef BEOS 
.else  
.ifdef LIBC 
.else  
	ldr r12,= SYS_read
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.endif  
WriteFile00:
	eors r3,r3,r3
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r3,r3,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
WriteFile0:

.ifdef W32 
.else  
	eors r1,r1,r1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.endif  
Write_File:

.ifdef W32 
.else  
.ifdef BEOS 
.else  
.ifdef LIBC 
.else  
	ldr r12,= SYS_write
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.endif  
OpenFile0:

.ifndef BEOS 
.ifdef W32 
.else  
	eors r2,r2,r2
.endif  
.endif  
OpenFile:

.ifdef W32 
.else  
.ifdef BEOS 
.else  
.ifdef LIBC 
.else  
	ldr r12,= SYS_open
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.endif  
CloseFile:

.ifdef W32 
.else  
.ifdef LIBC 
.else  
	ldr r12,= SYS_close
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.ifdef SYS_readlink 
ReadLink:
	ldr r12,= SYS_readlink
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.ifdef SYS_fchmod 
Fchmod:
.ifdef LIBC 
.else  
	ldr r12,= SYS_fchmod
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.ifdef SYS_fstat 
Fstat:	A_DR r2,fstatbuf
.ifdef LIBC 
.else  
.ifdef FREEBSD 
.else  
	ldr r12,= SYS_fstat
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
ChownFile:

.ifdef LIBC 
.else  
	ldr r12,= SYS_fchown
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
RenameFile:

.ifdef W32 
.else  
.ifdef LIBC 
.else  
	ldr r12,= SYS_rename
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.ifdef SYS_brk 
SysBrk:	ldr r12,= SYS_brk
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.ifndef W32 
Exit:	eors r1,r1,r1
Exit2:
.ifdef LIBC 
.else  
	ldr r12,= SYS_exit
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
SeekFile:
	eors r2,r2,r2
.ifdef W32 
.else  
.ifdef FREEBSD 
.else  
.ifdef BEOS 
.else  
.ifdef LIBC 
.ifdef OPENBSD 
.endif  
.ifdef OPENBSD 
.endif  
.else  
	ldr r12,= SYS_lseek
	bic r0,r0,#0xFF
	orr r0,r0,r12
.endif  
.endif  
.endif  
.endif  
.ifndef LIBC 
.ifndef W32 
IntCall:
	mov r12,#0x0
	bic r0,r0,#0xFF00
	orr r0,r0,r12,lsl #8
IntCall2:
	mov r0,r0,lsl #16
	mov r0,r0,asr #16

.ifdef BEOS 
.else  
.ifdef ATHEOS 
.else  
.ifdef LINUX 
.ifdef AMD64 
.else  
	stmfd r13!,{r1-r6,r14}
	bl _INT
	ldmfd r13!,{r1-r6,r14}
.endif  
.else  
.ifdef NETBSD 
.else  
.endif  
.endif  
AfterInt:
	rsbs r0,r0,#0
err:	A_DR r12,ErrNr
	str r0,[r12]
	rsbs r0,r0,#0
.endif  
.endif  
	RET
.endif  
.endif  
.ifdef BEOS 
.endif  
.ifdef SYS_select 
.ifdef LIBC 
.else  
Select:	eors r1,r1,r1
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r1,r1,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
	A_DR r2,readfds
	strB r1,[r2]
	eors r3,r3,r3
	eors r5,r5,r5
	A_DR r4,timevalsec
	ldr r12,= SYS_select
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.ifdef SYS_readlink 
Utime:	ldr r12,= SYS_utime
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.ifdef USE_PIPE 
.ifdef FREEBSD 
.else  
.endif  
.ifdef AMD64 
.else  
.endif  
.endif  
Unlink:
.ifdef W32 
.else  
.ifdef LIBC 
.else  
.ifdef BEOS 
.endif  
	ldr r12,= SYS_unlink
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b IntCall
.endif  
.endif  
.ifdef SYS_kill 
KeySuspend:
	CALL KursorStatusLine
	ldr r12,= SIGSTOP
	stmfd r13!,{r12}
	ldmfd r13!,{r2}
	eors r1,r1,r1
Kill:	ldr r12,= SYS_kill
	bic r0,r0,#0xFF
	orr r0,r0,r12
ICjmp:	b IntCall
SetSigHandler:

	ldr r12,= SIGCONT
	stmfd r13!,{r12}
	ldmfd r13!,{r1}
	A_DR r2,sigaction
.ifdef AMD64 
.else  
	A_DR r11,SigHandler
	str r11,[r2]
	eors r3,r3,r3
.ifdef SIGREST32 
.endif  
Sigaction:
	ldr r12,= SYS_sigaction
	bic r0,r0,#0xFF
	orr r0,r0,r12
	b ICjmp
.ifdef SIGREST32 
.endif  
.endif  
.endif  
.ifdef USE_PIPE 
.ifdef USE_UNDO 
.endif  
.ifdef CAPTURE_STDERR 
.endif  
.ifdef USE_EX_NO_SED 
.else  
.ifdef CAPTURE_STDERR 
.endif  
.ifdef CAPTURE_STDERR 
.ifdef BEEP_IN_VI 
.endif  
.endif  
.endif  
.ifdef USE_EX_NO_SED 
.else  
.ifdef CAPTURE_STDERR 
.endif  
.endif  
.endif  
ChkCursPos:

.ifdef UTF8 
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
CCloopUTF8:
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	subs r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
.ifdef UTF8RTS 
	A_DR r12,isUTF8
	mov r11,#0x0
	ldrB r10,[r12]
	cmp r10,r11
	beq noUTF_Z
.endif  
	ldrB r12,[r4]
	bic r0,r0,#0xFF
	orr r0,r0,r12
	mov r12,#0xc0
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	ands r11,r11,r12
	and r11,r11,#0xFF
	bic r0,r0,#0xFF
	orr r0,r0,r11
	mov r12,#0x80
	mov r11,r0,lsl #24
	mov r11,r11,lsr #24
	cmp r11,r12
	beq CCloopUTF8
noUTF_Z:

.endif  
	cmp r4,r6
	bls CCP
	mov r4,r6
CCP:	A_DR r11,sot
	cmp r4,r11
	bhs CCP2
	A_DR r4,sot
CCP2:	RET
.ifdef SYS_readlink 
CopyToBackup:
 PUSH_ALL @(is a macro)
	stmfd r13!,{r2}
	stmfd r13!,{r2}
	CALL OpenFile0
	mov r12,r0
	mov r0,r5
	mov r5,r12
	ldr r2,= O_WRONLY_CREAT_TRUNC

	A_DR r12,perms
	ldr r3,[r12]
	ldmfd r13!,{r1}
	CALL OpenFile
	mov r12,r0
	mov r0,r1
	mov r1,r12
	A_DR r12,fstatbuf+stat_struc.st_uid
	ldr r2,[r12]
.ifdef UIDGID_WORD 
	mov r3,r2
	mov r3,r3,lsr #16
	mov r2,r2,lsl #16
	mov r2,r2,lsr #16
.else  
.endif  
	CALL ChownFile
	eors r4,r4,r4
copylop:
	stmfd r13!,{r1}
	mov r1,r5
	A_DR r2,screenbuffer
	mov r3,#0x0
	mov r8,#0x10
	add r3,r3,r8,lsl #8

	CALL Read_File
	ldmfd r13!,{r1}
	cmp r0,r3
	beq notready
	mrs r8,CPSR
	and r8,r8,#0x20000000
	mov r11,#0x1
	adds r4,r4,r11
	mrs r12,CPSR
	orrcc r12,r12,r8
	msr CPSR_f,r12
notready:
	A_DR r2,screenbuffer
	mov r3,r0
	CALL Write_File
	orrs r4,r4,r4
	beq copylop
	CALL CloseFile
	mov r1,r5
	CALL CloseFile
	ldmfd r13!,{r1}
	A_DR r2,accesstime
	CALL Utime
 POP_ALL @(is a macro)
	RET
.endif  
KeyCtrlKN:

.ifdef USE_MATH 
.endif  
.ifdef BEEP_IN_VI 
VIBeepForD:
 PUSH_ALL @(is a macro)
.ifdef W32 
.else  
	A_DR r2,BeepChar
	CALL WriteFile00
.endif  
 POP_ALL @(is a macro)
.endif  
	RET
.ifdef USE_MATH 
.endif  
OCret:	RET
.ifdef USE_UNDO 
.ifdef ROLLBACK 
.endif  
.ifdef ROLLBACK 
.endif  
.endif  
.ifdef UTF8RTS 
KeyUTF8switch:
	A_DR r12,isUTF8
	ldrB r11,[r12]
	mvns r11,r11
	strB r11,[r12]
	RET
.endif  
.ifdef LINUX 
.ifndef CRIPLED_ELF 
.data
.code 32
.endif  
.endif  
.ifdef USE_MATH 
.endif  
tempfile2:
.byte	'e','3','#','#',0
.ifdef USE_PIPE 
.ifdef USE_EX_NO_SED 
.ifndef AMD64 
.else  
.endif  
.else  
.ifndef AMD64 
.else  
.endif  
.ifndef PERLPIPE 
.else  
.endif  
.endif  
.endif  
optiontext:
.byte	'O','P','T','?',' ','C','/','B',0
filename:
.byte	'F','I','L','E','N','A','M','E',':',0
block:.byte	' ',' ',' ','N','A','M','E',':',0
saveas:.byte	'S','A','V','E',' ','A','S',':',0
filesave:
.byte	' ',' ',' ','S','A','V','E',':',0
asksave:
.byte	'S','A','V','E','?',' ','Y','n','l',0
asksave2:
.byte	'S','A','V','E','?',' ','Y','n',0
askreplace1:
.byte	'R','E','P','L','A','C','E',':',0
askreplace2:
.byte	'R','E',' ','W','I','T','H',':',0
asklineno:
.byte	'G','O',' ','L','I','N','E',':',0
askfind:
.byte	' ','S','E','A','R','C','H',':',0
asknumber:
.byte	'^','Q',' ','O','C','T','A','L',':',0
extext:.byte	'm','o','d','e',' ','E','X',':',0
modetxt:
.byte	'S','E','T',' ','M','O','D','E',0
.equ DoNo,10
ScanTable:

.ifdef W32 
.else  
.byte	DoNo
.byte	0
.byte	8
.byte	9
.byte	5
.byte	2
.byte	7
.byte	0
.byte	5
.ifdef QNX 
.endif  
.byte	1
.byte	6
.byte	4
.byte	3
.byte	DoNo
.byte	5
.byte	7
.byte	0
.ifndef LINUX 
.endif  
.ifdef QNX 
.endif  
.equ STsize,( . -ScanTable)
.endif  
EmaAltTable:

.byte	18
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	19
.byte	17
.byte	61
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	2
.byte	39
.byte	62
.equ ATsize,( . -EmaAltTable)
.equ Beep,78
VIcmdTable:
.byte	Beep
.byte	Beep
.byte	2
.byte	Beep
.byte	54
.byte	Beep
.byte	7
.byte	Beep
.byte	3
.byte	Beep
.byte	6
.byte	Beep
.byte	Beep
.byte	6
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	55
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	81
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	4
.byte	Beep
.byte	Beep
.byte	79
.byte	5
.byte	Beep
.byte	Beep
.byte	80
.byte	Beep
.byte	Beep
.byte	Beep
.byte	6
.byte	Beep
.byte	1
.byte	Beep
.byte	57
.byte	0
.byte	68
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	Beep
.byte	44
.byte	62
.byte	Beep
.byte	Beep
.byte	Beep
.byte	58
.byte	Beep
.byte	46
.byte	18
.byte	75
.byte	76
.byte	Beep
.byte	Beep
.byte	15
.byte	48
.byte	51
.byte	77
.byte	Beep
.byte	49
.byte	82
.byte	Beep
.byte	50
.byte	60
.byte	Beep
.byte	52
.byte	64
.byte	Beep
.byte	Beep
.byte	Beep
.byte	19
.byte	63
.byte	Beep
.byte	65
.byte	Beep
.byte	Beep
.byte	Beep
.byte	56
.byte	Beep
.byte	Beep
.byte	45
.byte	18
.byte	Beep
.byte	53
.byte	67
.byte	Beep
.byte	Beep
.byte	3
.byte	43
.byte	6
.byte	1
.byte	4
.byte	69
.byte	Beep
.byte	47
.byte	59
.byte	Beep
.byte	74
.byte	Beep
.byte	Beep
.ifdef USE_UNDO 
.else  
.byte	Beep
.endif  
.byte	Beep
.byte	19
.byte	9
.byte	70
.byte	66
.equ VIsize,( . -VIcmdTable)
Ktable:.byte	DoNo
.byte	DoNo
.byte	36
.byte	21
.byte	13
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	29
.byte	DoNo
.byte	DoNo
.byte	20
.byte	DoNo
.byte	62
.byte	79
.byte	DoNo
.byte	72
.byte	11
.byte	16
.byte	12
.byte	DoNo
.ifdef UTF8RTS 
.byte	84
.else  
.endif  
.byte	35
.byte	37
.byte	22
.byte	25
.byte	81
.equ Ktable_size, . -Ktable
Qtable:.byte	DoNo
.byte	26
.byte	32
.byte	15
.byte	5
.byte	30
.byte	27
.byte	DoNo
.byte	23
.byte	17
.byte	DoNo
.byte	33
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	24
.byte	DoNo
.byte	14
.byte	0
.byte	DoNo
.byte	DoNo
.byte	28
.byte	18
.byte	31
.byte	34
.byte	19
Xtable:.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	11
.byte	DoNo
.byte	DoNo
.byte	71
.byte	DoNo
.byte	61
.byte	16
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	79
.byte	DoNo
.byte	72
.byte	DoNo
.byte	DoNo
.byte	12
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	41
.byte	38
.byte	DoNo
.byte	DoNo
PicoJtable:
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	23
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	34
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	72
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	73
.byte	DoNo
.byte	DoNo
.byte	42
.byte	DoNo
.byte	DoNo
.byte	DoNo
PicoQtable:
.byte	DoNo
.byte	DoNo
.byte	31
.byte	79
.byte	DoNo
.byte	15
.byte	28
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	17
.byte	62
.byte	19
.byte	DoNo
.byte	18
.byte	DoNo
.byte	DoNo
.byte	14
.byte	30
.ifdef USE_UNDO 
.else  
.byte	DoNo
.endif  
.ifdef UTF8RTS 
.short	84
.else  
.endif  
.byte	DoNo
.byte	DoNo
.byte	DoNo
.byte	DoNo
.equ esize,2
.ifdef ARMCPU 
.align 2
.endif  
jumptab1:

.ifndef USE_EXT_MOVE 
.short	KeyHome-_start
.else  
.endif  
.short	KeyUp-_start
.short	KeyPgUp-_start
.short	KeyLeft-_start
.short	KeyRight-_start
.ifndef USE_EXT_MOVE 
.short	KeyEnd-_start
.else  
.endif  
.short	KeyDown-_start
.short	KeyPgDn-_start
.short	KeyIns-_start
.short	KeyDel-_start
.short	SimpleRet-_start
.short	KeyCtrlKQ-_start
.short	KeyCtrlKS-_start
.short	KeyCtrlKD-_start
.ifndef USE_EXT_MOVE 
.short	KeyCtrlQR-_start
.short	KeyCtrlQC-_start
.else  
.endif  
.short	KeyCtrlKR-_start
.short	KeyCtrlQI-_start
.short	KeyCtrlQW-_start
.short	KeyCtrlQZ-_start
.short	KeyCtrlKK-_start
.short	KeyCtrlKC-_start
.short	KeyCtrlKX-_start
.short	KeyCtrlQDel-_start
.short	KeyCtrlQP-_start
.short	KeyCtrlKY-_start
.short	KeyCtrlQA-_start
.short	KeyCtrlQF-_start
.short	KeyCtrlQV-_start
.short	KeyCtrlKH-_start
.short	KeyCtrlQE-_start
.short	KeyCtrlQX-_start
.short	KeyCtrlQB-_start
.short	KeyCtrlQK-_start
.short	KeyCtrlQY-_start
.short	KeyCtrlKV-_start
.short	KeyCtrlKB-_start
.short	KeyCtrlKW-_start
.short	KeyCtrlXX-_start
.short	KeyEmaAltW-_start
.short	KeyEmaAltPer-_start
.short	KeyEmaCtrlXW-_start
.short	KeyCtrlT-_start
.short	KeyVIcmdi-_start
.short	KeyVIex-_start
.short	KeyVIcmda-_start
.short	KeyVICmdA-_start
.short	KeyVICmdo-_start
.short	KeyCtrlQE-_start
.short	KeyCtrlQX-_start
.short	KeyVICmdO-_start
.short	KeyVICmdI-_start
.short	KeyVICmdR-_start
.short	KeyVICmdd-_start
.short	KeyHalfPgDn-_start
.short	KeyHalfPgUp-_start
.short	KeyVI1Char-_start
.short	KeyVIfsearch-_start
.short	KeyVIbsearch-_start
.short	KeyVICmdp-_start
.short	KeyVICmdP-_start
.short	KeyHelp-_start
.short	KeyEditMode-_start
.short	KeyDell-_start
.short	KeyVICmdS-_start
.short	KeyVICmdZ-_start
.short	KeyVICmdz-_start
.short	KeyVIcmde-_start
.short	KeyVIcmd1-_start
.short	KeyVICmdm-_start
.short	KeyVICmdy-_start
.short	KeyEmaCtrlXF-_start
.ifdef USE_PIPE 
.else  
.short	SimpleRet-_start
.endif  
.short	KeyPiCtrlJT-_start
.short	KeyVICmdr-_start
.short	KeyVICmdC-_start
.short	KeyVICmdD-_start
.short	KeyVICmdJ-_start
.ifdef BEEP_IN_VI 
.short	VIBeepForD-_start
.else  
.endif  
.short	KeyCtrlKN-_start
.short	KeyVICmdJmpM-_start
.ifdef SYS_kill 
.short	KeySuspend-_start
.else  
.endif  
.short	KeyVI_M-_start
.ifdef USE_UNDO 
.else  
.short	SimpleRet-_start
.endif  
.ifdef UTF8RTS 
.short	KeyUTF8switch-_start
.else  
.endif  
.equ jumps1,( . -jumptab1)/esize
.short	SimpleRet-_start
.short	KeyCtrlQW-_start
.short	SimpleRet-_start
.short	KeyPgDn-_start
.short	KeyRight-_start
.short	KeyUp-_start
.short	KeyCtrlQZ-_start
.short	KeyDel-_start
.short	KeyDell-_start
.short	NormChar-_start
.ifdef SELFTEST 
.else  
.short	KeyHelp-_start
.endif  
.short	CtrlKMenu-_start
.short	KeyCtrlL-_start
.short	KeyRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	KeyHelp-_start
.short	CtrlQMenu-_start
.short	KeyPgUp-_start
.short	KeyLeft-_start
.short	KeyCtrlT-_start
.ifdef USE_UNDO 
.else  
.short	SimpleRet-_start
.endif  
.short	KeyIns-_start
.short	KeyScrollUp-_start
.short	KeyDown-_start
.short	KeyCtrlY-_start
.short	KeyScrollDn-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.ifdef ROLLBACK 
.else  
.short	SimpleRet-_start
.endif  
.short	KeyEmaMark-_start
.short	KeyHome-_start
.short	KeyLeft-_start
.short	SimpleRet-_start
.short	KeyDel-_start
.short	KeyEnd-_start
.short	KeyRight-_start
.short	SimpleRet-_start
.short	KeyDell-_start
.short	NormChar-_start
.short	KeyRet-_start
.short	KeyEmaCtrlK-_start
.short	KeyEmaCtrlL-_start
.short	KeyRetNoInd-_start
.short	KeyDown-_start
.short	KeyEmaCtrlO-_start
.short	KeyUp-_start
.short	KeyEmaCtrlQ-_start
.short	KeyEmaCtrlR-_start
.short	KeyEmaCtrlS-_start
.short	KeyEmaCtrlT-_start
.ifdef UTF8RTS 
.short	KeyUTF8switch-_start
.else  
.endif  
.short	KeyPgDn-_start
.short	KeyEmaCtrlW-_start
.short	CtrlXMenu-_start
.short	KeyEmaCtrlY-_start
.ifdef SYS_kill 
.short	KeySuspend-_start
.else  
.endif  
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.ifdef USE_UNDO 
.else  
.short	SimpleRet-_start
.endif  
.short	KeyEmaMark-_start
.short	KeyHome-_start
.short	KeyLeft-_start
.short	SimpleRet-_start
.short	KeyDel-_start
.short	KeyEnd-_start
.short	KeyRight-_start
.short	KeyHelp-_start
.short	KeyDell-_start
.short	NormChar-_start
.short	PicoJMenu-_start
.short	KeyEmaCtrlW-_start
.short	KeyEmaMark-_start
.short	KeyRet-_start
.short	KeyDown-_start
.short	KeyCtrlKS-_start
.short	KeyUp-_start
.short	PicoQMenu-_start
.short	KeyCtrlKR-_start
.short	KeyEmaCtrlXW-_start
.short	PicoCtrlTpico-_start
.short	KeyEmaCtrlY-_start
.short	KeyPgDn-_start
.short	KeyEmaCtrlS-_start
.short	KeyCtrlKQ-_start
.short	KeyPgUp-_start
.ifdef SYS_kill 
.short	KeySuspend-_start
.else  
.endif  
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	KeyEmaMark-_start
.short	SimpleRet-_start
.short	KeyEmaMark-_start
.short	KeyNedCtrlA-_start
.short	KeyIns-_start
.short	KeyEmaAltW-_start
.short	SimpleRet-_start
.short	KeyEditMode-_start
.short	KeyCtrlQF-_start
.short	KeyCtrlL-_start
.short	KeyDell-_start
.short	NormChar-_start
.short	KeyRet-_start
.short	KeyCtrlKN-_start
.short	KeyCtrlQI-_start
.short	KeyRet-_start
.short	KeyEmaCtrlXF-_start
.short	KeyEmaCtrlXF-_start
.short	SimpleRet-_start
.short	KeyCtrlKQ-_start
.short	KeyCtrlQA-_start
.short	KeyCtrlKS-_start
.short	SimpleRet-_start
.ifdef USE_UNDO 
.else  
.short	SimpleRet-_start
.endif  
.short	KeyEmaCtrlY-_start
.short	KeyEmaCtrlXW-_start
.short	KeyEmaCtrlW-_start
.ifdef UTF8RTS 
.short	KeyUTF8switch-_start
.else  
.endif  
.ifdef SYS_kill 
.short	KeySuspend-_start
.else  
.endif  
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.short	SimpleRet-_start
.ifdef W32 
.else  
BeepChar:
.byte	7
screencolors0:
.byte	27,'[','4','0','m',27,'[','3','7','m'
bold0:.byte	27,'[','0','m'
screencolors1:
.byte	27,'[','4','4','m',27,'[','3','3','m'
reversevideoX:

bold1:.byte	27,'[','1','m'
.equ scolorslen, . -screencolors1
.equ boldlen, . -bold1
.ifdef LINUX 
.byte	27,'[','7','m'
.ifdef NEW_CURSOR_MGNT 
blockcurs:
.byte	27,'[','?','1','7',';','0',';','6','4','c'
.equ blockcurslen, . -blockcurs
normcurs:
.byte	27,'[','?','2','c'
.equ normcurslen, . -normcurs
.endif  
.endif  
.endif  
.ifdef SELFTEST 
.endif  
.ifdef UTF8 
.ifdef UTF8RTS 
getPos:.byte	13,195,182,27,'[','6','n',13
.equ gPlen, . -getPos
.endif  
.endif  
.ifdef ARMCPU 
.align 2
.endif  
editmode:
.byte	'p',' ','W','S','p',' ','P','i','p',' ','E','m','p',' ','N','E'
helptext:

.ascii	"MicroEditor e3 v2.7.0"
.ifdef YASM 
.endif  
.ifdef UTF8 
.ascii	"-UTF8 "
.byte	194,169
.else  
.endif  
.ascii	"2000-06 A.Kleine"
.byte	10
.ascii	"Enter filename or leave with RETURN"
.byte	10,10
.ifdef YASM 
.ifdef UTF8 
.else  
.endif  
.else  
.equ helptextsize, . -helptext
.if 0
.endif  
.endif  
helpfoot:
.byte	10,10,10,TABCHAR,TABCHAR,TABCHAR
.ascii	"-= PRESS ANY KEY =-"
.ifdef YASM 
.else  
.equ helpfootsize, . -helpfoot
.if 0
.endif  
.endif  
.ifdef USE_BUILTINHELP 
help_ws:

.ascii	"Key bindings in WS mode:"
.byte	10,10
.ascii	"Files:	^KR Insert	^KS Save	^KX Save&Exit	^KQ Abort&Exit"
.byte	10
.ifndef USE_PIPE 
.ascii	"	^KD Save&Load"
.byte	10
.else  
.ifdef USE_EX_NO_SED 
.else  
.endif  
.endif  
.byte	10
.ascii	"Blocks:	^KB Start	^KK End		^KC Copy	^KY Del"
.byte	10
.ascii	"	^KV Move	^KW Write"
.byte	10
.byte	10
.ascii	"Search:	^QF Find	^L  Repeat	^QA Srch&Repl"
.byte	10
.ascii	"Move:	^E  Up		^X  Down	^S  Left	^D  Right"
.byte	10
.ascii	"	^R  Page Up	^C  Page Dn	^W  Scroll Up	^Z  Scroll Dn"
.byte	10
.ascii	"Quick-	^QE Wnd Top	^QX Wnd Bott	^QS Home	^QD End"
.byte	10
.ascii	"-Move:	^QR BOF		^QC EOF		^QB Blk Begin	^QK Blk End"
.byte	10
.ascii	"	^F  Next Word	^A  Prev Word	^QI Line#	^QV Last Find"
.byte	10
.byte	10
.ascii	"Delete:	^T  Word	^Y  Line	^H  Left	^G  Chr"
.byte	10
.ascii	"	^QY Line End	^QDel,^QH Line Beg"
.byte	10
.ifdef USE_MATH 
.else  
.ascii	"Other:	^KM Set mode"
.endif  
.ifdef SYS_kill 
.ascii	"	^KZ Suspend "
.endif  
.ifdef USE_UNDO 
.endif  
.ifdef UTF8RTS 
.byte	10
.ascii	"	^KU UTF8"
.endif  
.equ help_ws_size, . -help_ws
help_pi:

.ascii	"Key bindings in PICO mode:"
.byte	10,10
.ascii	"Files:	^XN ExitNoSave	^XY Exit+Save	^XL Save+Load New File"
.byte	10
.ascii	"	^O  Save	^S  Save as	^R  Read"
.byte	10
.byte	10
.ascii	"Move:	^P  Up		^N  Down	^B  Left	^F  Right"
.byte	10
.ascii	"	^Y  Page up	^V  Page down	^QN Next word	^QP Previous word"
.byte	10
.ascii	"	^A  Home	^E  End		^QS Start	^QE EOF"
.byte	10
.ascii	"	^QT Top screen	^QB Bottom scr	^QL Line #	^QF last Find"
.byte	10
.byte	10
.ascii	"Search:	^W  Where is	^T  Search&Repl	^JT Repeat Search & Replace"
.byte	10
.byte	10
.ascii	"Delete:	^H  Left char	^D  This char	^K  Kill line/region"
.byte	10
.ascii	"	^JW Word	^JL Line end	^JH Line begin"
.byte	10
.byte	10
.ascii	"Other:	^U  Unkill	^G  Help	^^,^L,^<SPC> Mark region"
.byte	10
.ifndef USE_PIPE 
.ascii	"	^QM Set Edit Mode "
.byte	10
.else  
.ifdef USE_EX_NO_SED 
.else  
.endif  
.endif  
.ifdef USE_MATH 
.else  
.ascii	" "
.endif  
.ifdef SYS_kill 
.ascii	"			^Z Suspend"
.endif  
.ifdef USE_UNDO 
.endif  
.ifdef UTF8RTS 
.byte	10
.ascii	"	^QV UTF8"
.endif  
.equ help_pi_size, . -help_pi
help_em:

.ascii	"Key bindings in EMACS mode:"
.byte	10,10
.ascii	"Files:	^X^C Exit	^XI  Insert	^X^S Save	^X^F Load New"
.byte	10
.ifndef USE_PIPE 
.ascii	"	^X^W Write new	^X^H Help "
.byte	10
.else  
.ifdef USE_EX_NO_SED 
.else  
.endif  
.endif  
.byte	10
.ascii	"Move:	^P   Up		^N  Down	^B   Left	^F   Right"
.byte	10
.ascii	"	altV Pg up	^V  Pg down	altB Left word	altF Right word"
.byte	10
.ascii	"	^A   Home	^E  End		alt< BOF	alt> EOF"
.byte	10
.ascii	"	altG Go line#	^L  Center Pos"
.byte	10
.byte	10
.ascii	"Search:	^S Find fwd	^R Find bwd	alt% Search&Replace like WS"
.byte	10
.byte	10
.ascii	"Buffer:	altW Copy	^Y Yank		^<SPC> Mark	^X^X Xchg Mark/Pt"
.byte	10
.byte	10
.ascii	"Delete:	^K Line		^W Region	^H Left	Chr	^D This Chr"
.byte	10
.byte	10
.ifdef UTF8 
.ascii	"Other:	^O Open line	           	^I Ins Tab	^Q Quoted Ins"
.byte	10
.else  
.endif  
.ascii	"	^M NL		^J NL+indent	altX Set edit mode"
.byte	10
.ifdef USE_MATH 
.else  
.ascii	"		"
.endif  
.ifdef SYS_kill 
.ascii	"			^Z Suspend"
.endif  
.ifdef USE_UNDO 
.endif  
.ifdef UTF8RTS 
.byte	10
.ascii	"	^U UTF-8"
.endif  
.equ help_em_size, . -help_em
help_vi:

.ascii	"Key bindings in vi mode:"
.byte	10
.byte	10
.ascii	"<ESC>			enter cmd mode"
.byte	10
.ascii	"h,j,k,l,+,-,<Ret>,<SPC>	move by chars&lines"
.byte	10
.ascii	"^B,^F,^D,^U		move by [half]page"
.byte	10
.ascii	"$,0,^,w,b,e,H,L,M,z.	move in line/screen"
.byte	10
.ascii	"/,?,G			srch fwd/bwd, go EOF"
.byte	10
.ascii	"ma,'a			set/go marker a"
.byte	10
.ascii	"x,X,<Del>,dw,D		dele chr,word,EOL"
.byte	10
.ascii	"S,C,dd,d'a,yy,y'a	subst,change,dele,yank"
.byte	10
.ascii	"p,P			paste"
.byte	10
.ascii	"A,a,I,i,<Ins>,O,o	enter ins.mode"
.byte	10
.ascii	"R,r			enter ovw.mode,ovw.chr"
.byte	10
.ascii	"J			join lines"
.byte	10
.ifdef USE_UNDO 
.ifdef SYS_kill 
.else  
.endif  
.else  
.ifdef SYS_kill 
.ascii	"ZZ,^Z			save&quit,suspend"
.byte	10
.else  
.endif  
.endif  
.ifdef USE_MATH 
.else  
.ascii	";			E3 SPECIAL:set edit mode"
.byte	10
.endif  
.ascii	":w,:wq,:x,:q,:q!,:e	ex mode:save,quit,save_as,edit other"
.byte	10
.ascii	":0,:$,:<line#>		ex mode:go BOF,EOF,line"
.byte	10
.ifdef UTF8RTS 
.ascii	":h,:u			ex mode:help, UTF-8"
.byte	10
.else  
.endif  
.ifndef USE_PIPE 
.ascii	"         "
.else  
.ifdef USE_EX_NO_SED 
.else  
.endif  
.endif  
.equ help_vi_size, . -help_vi
help_ne:

.ascii	"Key bindings in NEDIT mode:"
.byte	10
.byte	10
.ascii	"Files:		^QN Exit_NoSave	^QY Exit&Save	^QL Save&Load new"
.byte	10
.ascii	"		^S  Save	^W  WriteTo=SaveAs"
.byte	10
.ascii	"Move:		^L  Line#"
.byte	10
.ascii	"		^F  Find	^R Search&Replace (like WS)"
.byte	10
.ascii	"		^G  Go repeat last ^F,^R"
.byte	10
.byte	10
.ascii	"Select:		^<SPACE> begin&extend by cursor keys (like Emacs)"
.byte	10
.ascii	"		^A  All buffer"
.byte	10
.ascii	"		^X  Cut		^C Copy 	^V Paste"
.byte	10
.byte	10
.ascii	"Other:		^E  Set edit mode"
.byte	10
.ifdef UTF8RTS 
.ascii	"		^Y  UTF8 view"
.byte	10
.endif  
.ifdef USE_MATH 
.endif  
.ascii	"		altH Help"
.ifdef SYS_kill 
.ascii	"	^Z Suspend"
.endif  
.ifdef USE_UNDO 
.endif  
.equ help_ne_size, . -help_ne
.ifndef YASM 
.if 0
.endif  
.endif  
.else  
.endif  
errmsgs:
 errortext @(is a macro)
.ifdef CRIPLED_ELF 
.endif  
.ifdef ATHEOS 
.endif  
.ifdef NETBSD 
.endif  
.ifdef OPENBSD 
.endif  
.bss
.code 32
.align 4
.ifdef CRIPLED_ELF 
.endif  
.equ screenbuffer_size,11904
.equ screenbuffer_dwords,screenbuffer_size/4
screenbuffer:
.space	screenbuffer_size
.equ screenbuffer_end, . 
.ifdef W32 
.else  
termios:
.space	termios_struc_size
termios_orig:
.space	termios_struc_size
winsize:
.space	winsize_struc_size
.equ setkplen,10
setkp:.space	setkplen
.space	2
revvoff:
.space	1*4
.endif  
.ifdef USE_UNDO 
.endif  
lines:.space	1*4
columns:
.space	1*4
columne:
.space	1*4
zloffst:
.space	1*4
fileptr:
.space	1*4
kurspos:
.space	1*4
kurspos2:
.space	1*4
tabcnt:.space	1*4
changed:
.space	1*4
oldQFpos:
.space	1*4
bereitsges:
.space	1*4
blockbegin:
.space	1*4
blockende:
.space	1*4
endeedit:
.space	1*4
old:.space	1*4
veryold:
.space	1*4
linenr:.space	1*4
showblock:
.space	1*4
suchlaenge:
.space	1*4
repllaenge:
.space	1*4
vorwarts:
.space	1*4
grossklein:
.space	1*4
ch2linebeg:
.space	1*4
numeriere:
.space	1*4
read_b:.space	1*4
.ifdef W32 
.endif  
isbold:.space	1*4
inverse:
.space	1*4
insstat:
.space	1*4
.ifdef AMD64 
.else  
ErrNr:.space	1*4
.endif  
.equ errlen,100
error:.space	errlen
maxlen:.space	1*4
.equ maxfilenamelen,255
filepath:
.space	maxfilenamelen+1
bakpath:
.space	maxfilenamelen+1
blockpath:
.space	maxfilenamelen+1
replacetext:
.space	maxfilenamelen+1
suchtext:
.space	maxfilenamelen+1
suchtext2:
.space	maxfilenamelen+1
optbuffer:
.space	optslen
.equ linkbuffersize,4
linkbuffer:
.space	linkbuffersize
sigaction:
.space	40*4
perms:.space	1*4
.ifdef SYS_fstat 
fstatbuf:
.space	stat_struc_size
.endif  
.ifdef SYS_utime 
accesstime:
.space	utimbuf_struc_size
.endif  
screenline:
.space	256+4*scolorslen
.ifdef W32 
.endif  
EmaKiSize:
.space	1*4
EmaKiSrc:
.space	1*4
EmaMark:
.space	1*4
EmaCtrl:

EmaCtrlK:
.space	1
EmaCtrlS:
.space	1
.space	2
EmaNumHelper:
.space	1*4
VICmdMode:
.space	1*4
VIbufch:
.space	1*4
VInolinebased:
.space	1*4
PicoSearch:
.space	1*4
.ifdef UTF8RTS 
isUTF8:.space	1*4
.endif  
.ifdef USE_PIPE 
.endif  
mode:.space	1*4
readfds:
.space	1*4
timevalsec:
.space	1*4
timevalusec:
.space	1*4
.equ buffercopysize,1024
buffercopy:
.space	buffercopysize
.ifdef USE_MATH 
.ifdef AMD64 
.else  
.endif  
.endif  
.ifdef W32 
.endif  
.ifdef SYS_brk 
.equ max,1024000
.else  
.endif  
text:.space	max
.equ sot,(text+1)
.ifdef CRIPLED_ELF 
.endif  
