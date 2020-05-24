;--------------------------------------------------------------------
;
;  e3.asm v2.7.1 Copyright (C) 2000-2007 Albrecht Kleine <kleine@ak.sax.de>
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;----------------------------------------------------------------------
;
%include "e3.h"
;
section .text
bits 32
	ORGheader
global _start
;
; start with OUTER editor loop
;
_start:	call SetTermStruc
%ifdef SELFTEST
	mov eax,mode
	mov byte [eax],WS		;store current editor mode:  WS only..
	mov esi,pipein			;...because the test file expects WS "syntax"
%else
%ifdef W32
	push byte 0
	push dword 8192			;initial size (to be extended)
	push byte 0
	call HeapCreate
	mov [heap],eax
;------
	mov eax,mode
	mov byte [eax],DEFAULT_MODE	;store default editor mode
	call GetCommandLineA		;eax points to  either	D:\PATH\E3.EXE args1 args2 args3...
	xchg eax,esi			;               or	"d:\path\e3.exe"
	;-- for debug only --		;		or	e3.exe "args"
	;PUSH_ALL
	;push dword 0			;single OK button
	;push dword esi
	;push dword esi
	;push dword 0
	;call MessageBoxA		;show cmd line
	;POP_ALL
	;--------------------
	cld
	xor ebx,ebx
	cmp byte [esi],'"'		;due above mentined 2 different cmd line ways
	jnz prog
	dec ebx				;ebx -1 due pending '"'
prog:	lodsb
	or al,al
	jz prog9
	cmp al,SPACECHAR		;TODO chk TABs
	ja prog
	inc esi
prog9:	dec esi
prog0:	push esi
	lea esi,[esi+ebx-4-5]		;-4 due suffix ".exe"
	call SetEditMode
	jz prog1
	mov byte [eax],DEFAULT_MODE	;store current editor mode
prog1:	pop esi
	xor edx,edx			;no args
prog2:	lodsb
	or al,al
	jz prog3
	cmp al,SPACECHAR
	jbe prog2
	dec esi
	jmp short prog5
prog3:	mov esi,edx
prog5:
%else
%ifdef BEOS ;-----------------------------------------------------------
	pop eax
	pop ebx				;args counter (1....x)
	pop esi
	mov esi,[esi]			;points to our_name0[args]0env.....
	cld
prog:	lodsb
	or al,al
	jne prog
	mov edx,esi			;store arg ptr
	lea esi,[esi-5]
	call SetEditMode
	jz prog1
	mov byte [eax],DEFAULT_MODE	;store current editor mode
prog1:	xor esi,esi			;init for 'no args'
	dec ebx
	jz noarg
	mov esi,edx
noarg:
%else
%ifdef DYN ;------------------------------------------------------------
; This is to be called from dynamic linked libc startup code 
; just like you would expect:  int main(int argc,char**argv)
; (for experimental purpose only)
;
	xor esi,esi			;init to "no args"
	mov ecx,[esp+4]			;"int argc"
	cmp ecx,1
	je NoArg
	mov esi,[esp+8]			;"char**argv"
	mov esi,[esi]
	cld
Argl:	lodsb
	or al,al
	jne Argl
NoArg:	mov eax,mode
	mov byte [eax],DEFAULT_MODE	;store current editor mode (WS only)
%undef CURSORMGNT
%else	;-------------- i.e. Linux, FreeBSD, QNX, Ath ------------------
%ifdef SYS_rt_sigaction
	call SetSigHandler
%else
%ifdef SYS_sigaction
	call SetSigHandler
%endif
%endif
;-------
%ifdef ATHEOS
	pop edx
	pop edx
	pop edx
%endif
	pop edx				;Linux: arguments #
	pop esi				;Linux: argv[0]
	cld
prog:	lodsb
	or al,al
	jne prog			;get program name
%ifdef AMD64
	lea rsi,[rsi-5]
%else
	lea esi,[esi-5]
%endif
	call SetEditMode
	jz prog1
	mov byte [eax],DEFAULT_MODE	;store current editor mode
prog1:
;-------
	pop esi				;Linux: esi points to first arg (filename)
%ifdef NEW_CURSOR_MGNT
	call SetCursorBlock
%endif
%endif
%endif
%endif
%endif
;-------
%ifdef CURSORMGNT
	or esi,esi
	jz moreenv
morearg:pop ecx				;arguments until NULL
	or ecx,ecx
        jnz morearg
;-------
moreenv:pop ecx
	jecxz ReStart
%ifndef ARMCPU
	cmp dword[ecx],'TERM'		;a short test for "TERM=linux"
	jnz moreenv
	cmp dword[ecx+5],'linu'
%else
	cmp byte[ecx],'T'
	jnz moreenv
	cmp byte[ecx+1],'E'
	jnz moreenv
	cmp byte[ecx+2],'R'
	jnz moreenv
	cmp byte[ecx+3],'M'
	jnz moreenv
	cmp byte[ecx+5],'l'
	jnz ReStart
	cmp byte[ecx+6],'i'
	jnz ReStart
	cmp byte[ecx+7],'n'
	jnz ReStart
	cmp byte[ecx+8],'u'
%endif
	jnz ReStart
	add byte[revvoff],boldlen	;special inverse cursor on linux terminals
%endif
%ifdef UTF8RTS
	mov ecx,getPos 			;second argument: pointer to message to write
	push byte gPlen			;third argument: message length
	pop edx
	call WriteFile0
	mov ecx,screenbuffer		;pointer to buf
	push byte 10
	pop edx
	call ReadFile0			;get cursor pos
	mov al,[ecx+eax-2]		;al == '2' @ UTF8 terminal,  else al == '3'
	sub al,'3'
	mov byte [isUTF8],al
%endif
;-------
ReStart:call NewFile
	jc E3exit
MainCharLoop:call ChkCursPos
	call IsViMode
	jnz MCL
	mov ecx,[blockbegin]
	jecxz MCL
	push edi
	mov edi,ecx			;for vi only: keep Begin/End-line marker together
	call KeyEnd
	; <------prev line------------>
	; BegM......marker line........EndM
	; <-------next line----------->
	mov [blockende],edi		;set WS's "blockende" to one after EOL for VI marker
	pop edi
	call ShowBl1			;i.e. "mov byte [showblock],1"
;-------
MCL:	call DispNewScreen
	call RestoreStatusLine
	call HandleChar
%ifdef W32LF
	cmp byte [edi],RETURN		;never stay at character 0dh
	jnz MCL2
	inc edi				;(rather stay at following 0ah)
MCL2:
%endif
	mov ebx,endeedit
	cmp byte [ebx],0
	je MainCharLoop
	xor esi,esi			;just like if no arg is present
	cmp byte [ebx],2
	je ReStart			;^KD repeat edit using another file
E3exit:	call KursorStatusLine
%ifdef W32
	push dword w32result		;reset all to standard colors
	push byte 0
	mov eax,[lines]
	inc eax
	mov ebx,[columns]
	mul bl
	push eax
	push byte DARKWHITE		;equ 7
	push dword [hout]
	call FillConsoleOutputAttribute
%endif
	mov ecx,text			;enter next line on terminal NEWLINE is @ byte [text]
	call WriteFile00
%ifdef NEW_CURSOR_MGNT
	call SetCursorNormal
%endif
;-------
	mov ebx,tempfile2		;undo info (if exist)
	call Unlink
%ifdef W32
	push dword [heap]
	call HeapDestroy
	push byte 0			;return code
	call ExitProcess		;Ready&Exit
%else
	mov ecx,TERMIOS_SET 
	call IOctlTerminal0		;restore termios settings
	jmp Exit
%endif
;----------------------------------------------------------------------
;
; MAIN function for processing keys
;
HandleChar:call ReadChar
	cmp ah,0xFF			;normal chars get 0xFF in ah
	jne near ExtAscii		;go handling Cursor-Keys
	mov esi,mode
	test byte [esi], EM | PI
	jz NO_EM01
	cmp al,11
	je IsCtrlK
	mov byte [EmaCtrlK],0
IsCtrlK:cmp al,13h			;^S
	je IsCtrlS
	cmp al,12h			;^R
	je IsCtrlS
	mov byte [EmaCtrlS],0
IsCtrlS:
NO_EM01:cmp byte [esi],VI
	jz ISVI1
	cmp al,32			;in WS,EM,PI,NE: handle control chars
	jae NormChar
	mov bl,al
	add bl,jumps1
	cmp byte [esi],WS
	je CJump
	add bl,32
	cmp byte [esi],EM
	je CJump
	add bl,32
	cmp byte [esi],PI
	je CJump
	add bl,32
CJump:	jmp CompJump2
ISVI1:					;in VI: most control is done in command mode...
	cmp al,7			;... so maintaining another table for <Return>...
	je near KeyDel			;... <Del> and <DelLeft> is useless
	cmp al,8
	je near KeyDell
	cmp al,RETURN
	je near KeyRet
;-------
NormChar:test byte [mode], EM | PI
	jz NOEM0
	call ShowBl0			;i.e. "mov byte [showblock],0"
NOEM0:	
%ifdef UTF8
%ifdef UTF8RTSx_wont			;won't overwrite more than one single ASCII byte at once...
	cmp byte [isUTF8],0		;...with a 2- or 3-byte UTF-8 character entered from keyboard.
	je noUTF_A			;So @ non-UTF8 consoles byte 2,3,... are always inserted.
%endif
	mov bl,al
	and bl,0C0h
	cmp bl,080h			;byte 2,3,4.. always insert
	je NormCh2
noUTF_A:
%endif
	call CheckMode
%ifdef USE_UNDO
	jz NormCh2
	call DataForUndoOverwrite
	jmp short OverWriteChar
%else
	jnz OverWriteChar
%endif	
NormCh2:push eax
%ifdef W32LF
	call CheckEof
	jz noEOL
	cmp word [edi-1],RETURN|(NEWLINE<<8)
	jnz noEOL
	dec edi				;move back to 0Dh
noEOL:
%endif
	call Insert1Byte
	pop eax
	jc InsWriteEnd			;error: text buffer full
OverWriteChar:cld
	stosb
%ifdef UTF8
%ifdef UTF8RTSx_wont			;won't produce incomplete UTF8 characters: ....
	cmp byte [isUTF8],0		;...so one single ASCII will overwrite a complete...
	je noUTF_B			;...UTF8 byte sequence at once.
%endif
	xor eax,eax
	dec eax
OWCloopUTF8:inc eax
	mov bl,byte [edi+eax]
	and bl,0C0h
	cmp bl,080h			;delete byte 2,3,4,....
	je OWCloopUTF8
	call DeleteByte
noUTF_B:
%endif
SetChg:	mov byte [changed],CHANGED
InsWriteEnd:ret
;-------
KeyVICmdr:call ReadOneChar		;repl one char (except newline)
	cmp byte [edi],NEWLINE
	je InsWriteEnd
	cmp al,RETURN
	jnz KeyVICmdr1
	mov al,NEWLINE
KeyVICmdr1:
%ifdef USE_UNDO
	call DataForUndoOverwrite
%endif	
KeyVICmdr2:mov byte [edi],al
	jmp short SetChg
;-------
KeyEmaCtrlQ:mov esi,asknumber
	call GetOctalToInteger
	jbe InsWriteEnd
	xchg eax,ecx			;using decimal input for ASCII value
	cmp eax,256
	jb NormCh2
	ret
;-------
;
; helper for HandleChar
;
CtrlKMenu:mov ebx,Ktable
	mov al,'K'
	jmp short Menu
CtrlQMenu:mov ebx,Qtable
	jmp short PicoQM
PicoJMenu:mov ebx,PicoJtable
	mov al,'J'
	jmp short Menu
PicoQMenu:mov ebx,PicoQtable
PicoQM:	mov al,'Q'
	jmp short Menu
CtrlXMenu:mov ebx,Xtable
	mov al,'X'
Menu:	mov ecx,2020205eh
	mov ch,al
;-------
MakeScanCode:call WriteTwo		;ebx expects xlat-table
	push ebx
	call GetChar
	pop ebx
	and al,01fh
	cmp al,Ktable_size
	jnb InsWriteEnd			;if no valid scancode
	xlatb
	mov ah,al			;=pseudo "scancode"
;------- cont
ExtAscii:mov bl,ah			;don't use al (carries char e.g. TAB)
	cmp bl,jumps1
	jae InsWriteEnd
	xor eax,eax
	mov [EmaCtrl],eax
CompJump2:mov bh,0
%ifdef YASM
	and ebx,0ffh
%else
	lea ebx,[bx]			;1 byte shorter than 'and ebx,0ffh'
%endif
	movzx ebx,word [2*ebx+jumptab1] ;2*ebx is due 2 byte per entry
;;;%ifdef YASM
;;;%ifdef AMD64
;;;	add rbx,0x400000b0
;;;%else
;;;	add ebx,0x08048080		;most ugly work around ever written
;;;%endif
;;;%else
	add ebx,_start			;offset inside code
;;;%endif
;-------
	call ebx			;the general code jump dispatcher
;-------
	cmp byte [numeriere],1		;after return from functions...
	jnz BZNret			;...decide whether count current line number
	push edi
	mov esi,sot
	xchg esi,edi
	xor edx,edx
BZNLoop:inc edx				;edx=linenr counter
	call LookForward
	inc edi				;point to start of next line
	cmp edi,esi
	jbe BZNLoop
	mov [linenr],edx
	pop edi
	mov byte [numeriere],0
BZNret:	ret
;----------------------------------------------------------------------
;
; processing special keys: cursor, ins, del
;
KeyRetNoInd:xor eax,eax
	jmp short KeyRetNInd
KeyRet:	
;;; %define NO_AUTO_INDENT		;for Izzy
%ifndef NO_AUTO_INDENT
%ifdef SELFTEST
	xor eax,eax
%else
	call CheckMode
	jnz  OvrRet
	call CountToLineBegin		;set esi / returns eax
	inc esi
	inc esi
	or eax,eax
	jz KeyRetNInd
	mov ebx,eax	
	xor eax,eax
	dec eax
KeyRetSrch:inc eax			;search non (SPACE or TABCHAR)
	cmp eax,ebx
	jae KeyRetNInd
	cmp byte [esi+eax],SPACECHAR
	je KeyRetSrch
	cmp byte [esi+eax],TABCHAR
	je KeyRetSrch
%endif
%else
	xor eax,eax
%endif
KeyRetNInd:push esi
	push eax			;eax is 0 or =indented chars
	call GoDown
	pop eax
	push eax
%ifdef W32LF
	inc eax				;1 extra for RETURN
	call CheckEof
	jz noEOL2
	cmp word [edi-1],RETURN|(NEWLINE<<8)
	jnz noEOL2
	dec edi				;move back to 0Dh
noEOL2:
%endif
	call InsertByte0		;1 extra for NEWLINE
	pop ecx				;# blanks
	pop esi				;where to copy
	jc SimpleRet
	inc dword [linenr]
	cld
%ifdef W32LF
	mov ax,RETURN|(NEWLINE<<8)	;insert 0d0ah combination
	stosw
%else
	mov al,NEWLINE
	stosb
%endif
	jecxz SimpleRet
	rep movsb			;copy upper line i.e. SPACES,TABS into next
SimpleRet:ret
OvrRet:	xor eax,eax
	mov [ch2linebeg],eax
	jmp short DownRet
;-------
KeyDown:call CountColToLineBeginVis
DownRet:call GoDown
	call LookLineDown
	jmp short JmpSC
;-------
KeyUp:	call GoUp
	call CountColToLineBeginVis
	call LookLineUp
	jmp short JmpSC
;-------
KeyHalfPgUp:call CountColToLineBeginVis
	call LookHalfPgUp
	jmp short SetColumn
;-------
KeyHalfPgDn:call CountColToLineBeginVis
	call LookHalfPgDn
	jmp short SetColumn
;-------
KeyScrollUp:call CountColToLineBeginVis
	call LookScrUp
	jmp short SetColumn
KeyScrollDn:call CountColToLineBeginVis
	call LookScrDn
	jmp short SetColumn
;-------
KeyPgUp:call CountColToLineBeginVis
	call LookPageUp
JmpSC:	jmp short SetColumn
;-------
KeyPgDn:call CountColToLineBeginVis
	call LookPgDown			;1st char last line
SetColumn:mov ecx,[ch2linebeg]		;=maximal columns
	xor edx,edx			;counts visible columns i.e. expand TABs
	dec edi
SCloop:	inc edi
%ifdef UTF8
%ifdef UTF8RTS
	cmp byte [isUTF8],0		;if the tty can't handle UTF8..
	je noUTF_C			;..each byte is one column
%endif
	mov bl,byte [edi]
	and bl,0C0h
	cmp bl,080h			;do not count byte 2,3,4,.. for columns in UTF8 chars
	jz SCloop
noUTF_C:
%endif
	cmp edx,ecx			;from CountColToLineBeginVis
	jae SCret
	cmp byte [edi],NEWLINE		;don't go beyond line earlier line end
	jz SCret
	cmp byte [edi],TABCHAR
	jz SCtab
	inc edx				;count columns
	jmp short SCloop
SCtab:	call SpacesForTab
	add dl,ah
	cmp edx,ecx			;this tab to far away right?
	jna SCloop			;no
SCret:	ret
;----------------------------------------------------------------------
;
; a helper for d'a and y'a vi commands
; have to differ whether cursor is below or above the marked line
; (at all this line based concept does not fit very well into e3)
; expects:
;	ecx valid begin of marked line
;	edi cursor
VIsetMarker:cmp edi,ecx
	ja Marker_above_cursor
	; X........cursor line.......
	; ...........................
	; .........marker line.......
	; Y
	mov ecx,[blockende]
	inc ecx
	cmp ecx,ebp
	jb Mbel
	dec ecx
Mbel:	mov dword [EmaMark],ecx		;i.e. store point Y
;------- cont
KeyHome:call CountToLineBegin		;i.e. goto point X
	sub edi,eax
	ret
;-------
Marker_above_cursor:
	; Y.......marker line .......	
	; ...........................
	; ........cursor line........
	; X				;								
	mov dword [EmaMark],ecx		;i.e. store point Y
	call KeyEnd
	inc edi				;i.e. goto point X
	cmp edi,ebp
	jb Mret
	dec edi
Mret:	ret
;----------------------------------------------------------------------
KeyIns:	not byte [insstat]
	xor eax,eax
	call IsViMode
	jnz KeyIns2
	inc eax
	cmp byte [VICmdMode],al
	jne KeyIns2
	mov byte [insstat],al
	call KeyVImode0
KeyIns2:call IsEmMode
	jnz KeyIns3
	mov byte [showblock],al
KeyIns3:
%ifdef NEW_CURSOR_MGNT
	cmp byte [insstat],1
	jne near SetCursorNormal
	jmp SetCursorBlock
%endif
	ret
;-------
KeyVICmdJ:call KeyEnd
	jmp short KeyDel
;-------
KeyDell:call KeyLeft
	jz KeyDell2
KeyDel:	cmp edi,ebp
	jnb KeyIns3
	xor eax,eax			;delete one @ cursor
KDloopUTF8:inc eax
%ifdef UTF8
%ifdef UTF8RTSx_wont			;won't produce incomplete UTF8 characters: ....
	cmp byte [isUTF8],0		;...so pressing DEL single will delete a complete..
	je noUTF_D			...UTF8 byte sequence at once.
%endif
	mov bl,byte [edi+eax]
	and bl,0C0h
	cmp bl,080h
	je KDloopUTF8			;delete one more at UTF-8 byte 2,3,4,....
noUTF_D:
%endif
%ifdef W32LF
	cmp byte [edi-1],RETURN
	jnz KD2
	dec edi				;delete one more
	inc eax
KD2:
%endif
;-------
	call IsViMode
	jne near DeleteByte
	mov esi,edi			;make vi's x and X pasteable
	mov byte [VInolinebased],1
	call KeyEmaAltW2
	jmp DeleteByte
;-------	
KeyDell2:cmp edi,sot			;delete newline char
	jbe KeyIns3
	dec dword [linenr]
	dec edi
	jmp KeyCtrlT1
;-------
KeyEmaCtrlT:
%ifdef UTF8
	ret				;FIXME!!
%else
	cmp edi,sot			;xchg 2 chars
	jbe KeyRightEnd
	cmp byte [edi],NEWLINE
	jnz KECT
	dec edi
KECT:	
%ifdef USE_UNDO
	call DataForUndoXchange
%endif	
	mov al,byte [edi]
	xchg al,byte [edi-1]
	call KeyVICmdr2			;mov byte [edi],al / mov byte [changed],CHANGED
%endif
;-------
KeyRight:
%ifdef UTF8
%ifdef UTF8RTSx_wont			;try to keep UTF8 bytes together..
	cmp byte [isUTF8],0		;...also if the console can not display the UTF8 character
	je noUTF_E
%endif
	inc edi
	mov al,byte [edi]		;check for UTF byte 2,3,4,..
	and al,0c0h
	cmp al,080h
	je KeyRight
	dec edi				;due inc edi above
noUTF_E:
%endif
	cmp byte [edi],NEWLINE
	jnz KeyRNoMargin
	call CheckEof
	jae KeyRightEnd
	call IsViMode
	je KeyRightEnd			;no more line wrap around in vi mode
	call CheckENum			;Sun Feb 20 2005
	call GoDown
KeyRNoMargin:inc edi
KeyRightEnd:ret
;-------
KeyCLeft3:cmp edi,sot
	jbe KeyCLEnd
	call CheckENum			;Sun Feb 20 2005
	dec edi
KeyCtrlQW:cmp byte [edi-1],NEWLINE
	jz KeyCLeft3
	dec edi
	cmp byte [edi],2fh
	jbe KeyCtrlQW
	cmp byte [edi-1],2fh
	ja KeyCtrlQW
KeyCLEnd:ret
;-------
KeyCRight3:call CheckEof
	jae KeyCREnd
	call CheckENum			;Sun Feb 20 2005
	jmp short KQZ1
KeyCtrlQZ:mov al,2fh
	cmp byte [edi],NEWLINE
	jz KeyCRight3
KQZ1:	inc edi
	call IsEmMode
	jz ISEM2
	cmp byte [edi],al		;ws stops at word begin
	jbe KeyCtrlQZ
	cmp byte [edi-1],al
	jmp short ISEM22
ISEM2:	cmp byte [edi-1],al		;em stops after end
	jbe KeyCtrlQZ
	cmp byte [edi],al
ISEM22:	ja KeyCtrlQZ
KeyCREnd:ret
;-------
KeyVIcmde3:call CheckEof		;end of word (vi only)
	jae KeyCREnd
	inc edi
KeyVIcmde:cmp byte [edi],NEWLINE
	jz KeyVIcmde3
	inc edi
	cmp byte [edi],2fh
	jbe KeyVIcmde
	cmp byte [edi+1],2fh
	ja KeyVIcmde
	ret
;-------
KeyEmaCtrlO:call Insert1Byte
	jc KeyRightEnd
	mov al,NEWLINE
	mov byte [edi],al
	ret
;----------------------------------------------------------------------
;
; processing special keys from the WS's Ctrl-Q menu
;
KeyCtrlQE:call LookPgBegin		;goto top left on screen
	call KursorFirstLine
	jmp short KCtKV1
;-------
KeyCtrlQX:call LookPgEnd		;1st goto last line on screen
	call KeyEnd			;2nd goto line end
	call KursorLastLine
	jmp short KCtKV1
;-------
KeyCtrlQV:cmp byte [bereitsges],0	;goto last ^QA,^QF pos
	jz KeyCREnd
	mov edi,[oldQFpos]
KCtKV1:	jmp CQFNum
;-------
KeyVIbsearch:push byte -1
	jmp short KVIf
KeyVIfsearch:push byte 1
KVIf:	mov byte[grossklein],0ffh
	jmp short KeyECtS1
;-------
PicoCtrlTpico:mov [PicoSearch],edi	;store begin of search (because wrap around EOF)
KeyEmaAltPer:push byte 1		;s&repl
	pop dword[vorwarts]
	mov byte[grossklein],0dfh
;-------
KeyCtrlQA:mov byte [bereitsges],2
	call AskForReplace
	jc SimpleRet9
CQACtrlL:push edi
	call FindText
	jnc CQACL2
	pop edi
SimpleRet9:ret
CQACL2:	mov eax,[suchlaenge]
	call DeleteByte
	mov eax,[repllaenge]
	call InsertByte
	mov esi,replacetext
	call MoveBlock
	jmp short CQFFound
;-------
KeyPiCtrlJT:cmp byte [bereitsges],2
	jz CQACtrlL
	ret
;-------
KeyEmaCtrlR:push byte -1
	jmp short KECS
;-------
KeyEmaCtrlS:push byte 1
KECS:	mov byte[grossklein],0dfh
KeyECtS1:pop dword[vorwarts]
	mov [EmaMark],edi
	call ShowBl0			;i.e. "mov byte [showblock],0"
;------- cont
KeyCtrlQF:call IsEmMode
	jnz NO_EM04
	cmp byte [EmaCtrlS],1
	jz KeyCtrlL
NO_EM04:push dword [suchtext]		;store begin of old find_text
	mov byte [bereitsges],1
	call AskForFind
	pop ebx
	pushf
	test byte [mode], VI | PI
	jz NO_VIPI01
	popf
	or eax,eax			;jmp if user entered a new find_text
	jnz QFpico
	or bl,bl			;jmp if no old find text available
	jz CtrlQFEnd
	mov byte [suchtext],bl		;restore last find_text
QFpico:	mov [PicoSearch],edi		;store begin of search (because wrap around EOF)
	jmp short CQFCtrlL
;-------
NO_VIPI01:popf
	jc CtrlQFEnd
CQFCtrlL:push edi
	call FindText
	mov byte [EmaCtrlS],1
	jc CtrlQFNotFound
CQFFound:mov [oldQFpos],edi
	pop esi				;dummy
CQFNum:	jmp CheckENum			;i.e. "mov byte [numeriere],1   ret"
CtrlQFNotFound:pop edi
CtrlQFEnd:ret
;-------
KeyCtrlL:mov eax,[bereitsges]		;2^QA   1^QF   0else
	dec eax
	jz CQFCtrlL
	test byte[mode],WS | NE | PI
	jz SimpleRet4
	dec eax
	jz near CQACtrlL
SimpleRet4:ret
;-------
KeyVIcmd1:call ReadOneChar
	cmp al,'G'
	je KeyCtrlQR
	ret
ViSpecial:jecxz KeyCtrlQR
	jmp short KCQI
KeyNedCtrlA:mov [EmaMark],ebp
	call ShowBl1			;i.e.  "mov byte [showblock],1"   but shorter
KeyCtrlQR:mov edi,sot
	jmp short CQFNum
;-------
KeyCtrlQP:mov ecx,[veryold]
	cmp ecx,ebp
	ja SimpleRet4
	mov edi,ecx
JmpCQFN3:jmp short CQFNum
;-------
KeyCtrlQB:xchg eax,edi
	mov edi,[blockbegin]
CtrlQB2:or edi,edi			;exit if no marker set
	jnz CQFNum
	xchg edi,eax
	ret
;-------
KeyCtrlQK:xchg eax,edi
	mov edi,[blockende]
	jmp short CtrlQB2
;-------
KeyCtrlQI:mov esi,asklineno
	call GetAsciiToInteger
	jbe CtrlQFEnd			;CY or ZR set
KCQI:	mov edi,sot
	call LookPD2
JmpCQFN:jmp short JmpCQFN3
;-------
KeyCtrlQDel:call KeyLeft		;delete all left of cursor
	call CountToLineBegin
	sub edi,eax
	jmp short KCY
;-------
KeyVICmdD:mov byte [VInolinebased],1
KeyCtrlQY:call CountToLineEnd
%ifdef W32LF
	or eax,eax
	jz KCQY
	cmp byte [edi+eax-1],0dh
	jnz KCQY
	dec eax				;keep RETURN 0dh char if exist
KCQY:
%endif
	call IsViMode
	jnz CtrlTEnd1
	call CtrlTEnd1
	jmp KeyLeft
;-------
KeyCmddw:call CountToWordBeginVIstyle
	jmp short NO_EM05
;-------
KeyCtrlY:call KeyHome			;edi at begin
	call CountToLineEnd
	cmp byte[mode],WS
	jnz NO_WS01
KCY:	call DeleteByteCheckMarker
	jmp short KeyCtrlT1
NO_WS01:test byte [mode], VI | PI
	jz KeyCtrlT
	lea ecx,[edi+eax]
	cmp ecx,ebp
	jz CtrlTEnd1			;do not delete pending LINEFEED (0Ah)
	inc eax
	jmp short CtrlTEnd1
;-------
KeyCtrlT:call CountToWordBegin
	call IsEmMode
	jnz NO_EM05
KeyEmaCtrlK:call CountToLineEnd
NO_EM05:cmp byte [edi],NEWLINE
	jnz CtrlTEnd1
KeyCtrlT1:xor eax,eax
	inc eax				;1 for LINEFEED (0ah)
%ifdef W32LF
	cmp byte[edi-1],RETURN
	jnz KCT2
	dec edi				;0dh is expected "left" of 0ah
	inc eax				;1 for RETURN   (0dh)
KCT2:
%endif
CtrlTEnd1:call CheckEof
	jz SimpleRet3
	cmp byte[mode],WS
	jz near DeleteByteCheckMarker
	mov esi,edi
	call KeyEmaAltW2
	jmp short DelBjmp
;-------
KeyEmaCtrlW:mov ecx,[showblock]
	cmp byte[mode],PI
	jne NOPI1
KECW:	
%ifndef YASM
	jecxz KeyCtrlY
%else
	or ecx,ecx
	jz near KeyCtrlY
%endif
	mov ecx,[EmaMark]
	jecxz KECW
	jmp short NOPI2
NOPI1:	jecxz SimpleRet3
	mov ecx,[EmaMark]
	jecxz SimpleRet3
NOPI2:	call KeyEmaAltW
	mov edi,[EmaKiSrc]
	mov eax,[EmaKiSize]
DelBjmp:jmp DeleteByte
;----------------------------------------------------------------------
;
; processing special Keys from WS's Ctrl-K menu
;
KeyCtrlKY:call CheckBlock
	jc SimpleRet3			;no block: no action
	mov eax,[blockende]
	mov edi,esi			;esi is blockbegin (side effect in CheckBlock)
	sub eax,esi			;block length
	call DeleteByte			;out ecx:=0
	xchg eax,ecx
	call InitSV2			;block no longer valid
JmpCQFN2:jmp JmpCQFN
;-------
KeyCtrlKH:xor byte [showblock],1 	;flip flop
SimpleRet3:ret
KeyCtrlKK:				;UTF-8 :no special handling needed, because block end...
	mov [blockende],edi		;... points to first byte _after_ block
	jmp short ShowBl1
;-------
KeyCtrlKC:call CopyBlock
	jc SimpleRet2
CtrlKC2:mov [blockbegin],edi
	add eax,edi
	jmp InitSV3			;mov [blockende],eax - ret
;-------
KeyCtrlXX:mov ecx,[EmaMark]
	jecxz SimpleRet3
	call KeyEmaMark
	mov edi,ecx
	call KeyEmaCtrlL
KeyCXX:	jmp short JmpCQFN2
;-------
KeyCtrlKV:call CopyBlock
	jc SimpleRet2
	push edi
	cmp edi,[blockbegin]
	pushf
	mov edi,[blockbegin]
	call DeleteByte
	neg eax				;(for optimizing eax is negated there)
	popf
	pop edi
	jb CtrlKC2
	mov [blockende],edi
	sub edi,eax
KeyCtrlKB:mov [blockbegin],edi		;UTF-8: no special handling needed, because block begin...
ShowBl1:mov byte [showblock],1		;...points to _first_ byte in block
SimpleRet2:ret
ShowBl0:mov byte [showblock],0
	ret
;-------
KeyVICmdm:call ReadOneChar
	cmp al,'a'			;ma (marker a)
	jne SimpleRet2
	push edi
	call KeyHome			;setting WS's "blockbegin" to BOL
	mov [blockbegin],edi
	pop edi
	ret
;-------
KeyVICmdJmpM:call ReadOneChar
	cmp al,'a'
	jne SimpleRet2
	mov ecx,[blockbegin]		;like WStar's Ctrl-QB  [also Sun Oct  7 17:01:37 2001]
	jecxz SimpleRet2
	mov edi,ecx
	jmp short KeyCXX
;-------
KeyEmaMark:mov [EmaMark],edi
	jmp short ShowBl1
;-------
KeyCtrlKR:call ReadBlock
	jc CtrlKREnd
	call KeyCtrlKB
	add ecx,edi
	mov [blockende],ecx
	test byte [mode],EM | NE
	jz NO_EM03
	mov [EmaMark],ecx
	call ShowBl0			;i.e. "mov byte [showblock],0"
NO_EM03:cmp byte [mode],PI
	jnz CtrlKREnd
	mov edi,ecx			;in PI: cursor at end of read file
CtrlKREnd:jmp RestKursPos
;-------
KeyCtrlKW:call CheckBlock
	jc CtrlKSEnd	   		;no action
	call SaveBlock
	jmp short CtrlKREnd
;-------
KeyEmaCtrlXF:cmp byte [changed],UNCHANGED
	jz KECF
	mov esi,asksave2
	call DE1
	call RestKursPos
	call CheckUserAbort
	jz CtrlKSEnd
	and al,0dfh
	cmp al,'N'			;N for request NOT SAVE changes
KECF:	jz KCKD2
	jmp short KeyCtrlKD
;-------
KeyEmaCtrlXW:call GetBlockName
	jc CtrlKSEnd
	mov esi,blockpath
XW1:	cld
	PUSH_ALL
	mov edi,filepath
XW0:	lodsb
	stosb				;copy to blockpath to filepath
	or al,al
	jne XW0
	stosb
	POP_ALL
KeyCtrlKS0:call SetChg			;i.e. "mov byte [changed],CHANGED"  to save it really
;-------
KeyCtrlKS:call SaveFile
	pushf				;(called by ^kd)
	call RestKursPos
	popf
	jc CtrlKSEnd
Unchg:	mov byte [changed],UNCHANGED
CtrlKSEnd:ret
;-------
KeyCtrlKD:call KeyCtrlKS
	jc KeyKXend
KCKD2:	mov byte [endeedit],2
	ret
;-------
KeyCtrlKQ:cmp byte [changed],UNCHANGED
	jz KCKXend
	mov esi,asksave
	call DE1
	call RestKursPos
	call CheckUserAbort
	jz CtrlKSEnd
	and al,0dfh
	cmp al,'N'			;N for request NOT SAVE changes
	jz KCKXend
	cmp al,'L'			;L for SAVE and LOAD a new file
	jnz KeyCtrlKX
	call KCKXend
KeyCtrlKX:call KeyCtrlKS
	jc CtrlKSEnd
KCKXend:inc byte [endeedit]
KeyKXend:ret
;----------------------------------------------------------------------
;
; some minimal limited vi specials in command mode
;
KeyVICmdW:lea esi,[ecx+2]
	cmp byte [esi],SPACECHAR
	ja XW1
	ret
;-------
VINoLineCmd:mov eax,[ecx]
	cmp ax,'w!'			;save
	je KeyCtrlKS0
	cmp ax,'w'			;save
	je KeyCtrlKS
	cmp ax,'x'                      ;save and exit
	je KeyCtrlKX
	cmp ax,'$'			;No line number, but EOF
	jne KVI_KX0
;-------
KeyCtrlQC:mov edi,ebp
	jmp CQFNum
;-------
KVI_KX0:cmp ax,'wq'
KVI_KX:	je KeyCtrlKX
	cmp ax,'w '			;save as ... and continue
	je KeyVICmdW
	cmp ax,'q'
	je KeyCtrlKQ
	cmp ax,'q!'
	je KCKXend
	cmp ax,'e '			;edit another ..
	je near KeyVICmdE
	cmp ax,'h'
	je near KeyHelp
%ifdef UTF8RTS
	cmp ax,'u'
	je near KeyUTF8switch
%endif
%ifndef USE_PIPE
	ret
%else
	jmp KeyVICmdtemp
%endif
;-------
KeyVICmdZ:call ReadOneChar
	cmp al,'Z'
	je KVI_KX
	ret
;-------
KeyVI1Char:call KeyHome
	cmp byte [edi],SPACECHAR
	ja KFC2
KFC1:	cmp byte [edi],NEWLINE
	jz KFC2
	inc edi
	cmp byte [edi],SPACECHAR
	jbe KFC1
	cmp byte [edi-1],SPACECHAR
	ja KFC1
KFC2:	ret
;-------
KeyVICmdS:call KeyHome
	call KeyEmaCtrlK		;not quite ok in 'P'/'p' commands
	mov byte [VInolinebased],1
	jmp short KeyVICmdI
KeyVICmdd:call ReadOneChar
	cmp al,'w'			;word (greetings to ma_ko)
	mov byte [VInolinebased],1
	je near KeyCmddw
	cmp al,'d'			;"delete"
	mov byte [VInolinebased],0
	je near KeyCtrlY
	cmp al,"'"			;only line based mode supported
	jne KFC2
	call ReadOneChar
	cmp al,'a'			;" d'a "    (only marker "a" supported)
	jne KFC2
	mov ecx,[blockbegin]		;don't go further if no mark set
	jecxz KFC2
	call VIsetMarker		;an helper for adjusting begin/end marker line
callKECW:call KeyEmaCtrlW
	xor eax,eax
	mov [blockbegin],eax		;after delete mark is no more set
	jmp short JmpCQFn
;-------
KeyVICmdI:call KeyVI1Char
	jmp short KeyVImode0
;-------
KeyVICmdp:mov ecx,[EmaKiSize]		;check this before call KeyEmaCtrlY
jmpKFC2:
%ifdef YASM
	or ecx,ecx
	jz KFC2
%else
	jecxz KFC2
%endif
	cmp byte [VInolinebased],1
	jz KeyVICmdpnLB
	call OvrRet			;ugly
KeyVICmdP:mov ecx,[EmaKiSize]		;check this before call KeyEmaCtrlY
	jecxz jmpKFC2
	cmp byte [VInolinebased],1
	jz KeyVICmdPnLB
	call KeyHome
KeyVICP2:push edi
	call KeyEmaCtrlY
	pop edi
JmpCQFn:jmp CQFNum
;-------
KeyVICmdR:mov byte [insstat],254	;i.e "not 1"
	jmp short KeyVImode0
KeyVICmdO:call KeyHome
	call KeyRet
	call KeyUp
	jmp short KeyVImode0
KeyVICmdo:call KeyEnd
	call KeyRet
	jmp short KeyVImode0
KeyVICmdA:call KeyEnd
	jmp short KeyVImode0
KeyVIcmda:call KeyRight
KeyVIcmdi:mov byte [insstat],1
KeyVImode0:push byte 0
	jmp short KVim1
KeyVICmdC:call KeyEmaCtrlK
	mov byte [VInolinebased],1
	jmp short KeyVImode0
;-------
KeyVICmdpnLB:call KeyRight		;not_Line_Based mode
KeyVICmdPnLB:call KeyVICP2
	add edi,[EmaKiSize]		;Wed Apr 10 18:11:42 MEST 2002
;------- cont
KeyLeft:
%ifdef UTF8
%ifdef UTF8RTSx_wont			;see KeyRight comment
	cmp byte [isUTF8],0
	je noUTF_F
%endif
	dec edi
	mov al,byte [edi]		;check for UTF byte 2,3,4,..
	and al,0c0h
	cmp al,080h
	je KeyLeft
	inc edi
noUTF_F:
%endif
	cmp byte [edi-1],NEWLINE
	jnz KeyLNoMargin
	cmp edi,sot			;i.e. CheckBof
	je KeyLeftEnd
	call IsViMode
	je KeyLeftEnd			;no more line wrap around in vi mode
	call CheckENum			;Sun Feb 20 2005
	call GoUp
KeyLNoMargin:dec edi
%ifdef W32LF
	cmp byte [edi],RETURN		;do not stay at 0dh
	jnz KeyLeftEnd
	dec edi
%endif
KeyLeftEnd:ret
;-------
KeyEnd:	call CountToLineEnd
	add edi,eax			;points to a LINEFEED (0ah) char
	ret
;-------
KeyVImode1:push byte 1
KVim1:	pop eax
	mov byte [VICmdMode],al
	ret
;-------
KeyVIex:call InputStringWithMessage0
	pushf
	call RestKursPos
	popf
	jc Kviex
	mov esi,optbuffer
	xor edx,edx
	mov ecx,eax			;do not use xchg here
	jecxz Kviex
;-------
	push esi			;save optbuffer
	cld
CheckDig:lodsb				;check for line number entered
	cmp al,'0'
	jnb CD1
	inc edx
CD1:	cmp al,':'
	jb CD2
	inc edx
CD2:	loop CheckDig
	pop ecx				;rest optbuffer
;-------
	or edx,edx
	jnz near VINoLineCmd
	call GetAsciiToInteger
	jmp ViSpecial			;due short jumps there
;-------
IsViMode:cmp byte [mode],VI
	ret
IsEmMode:cmp byte [mode],EM
Kviex:	ret
;---------------------------------------------------------------------
;
; the general PAGE DISPLAY function: called after any pressed key
;
; side effect: sets 'columne' for RestoreStatusLine function (displays column)
; variable kurspos: for placing the cursor at new position
; register bh counts lines
; register bl counts columns visible on screen (w/o left scrolled)
; register edx counts columns in text lines
; register ecx screen line counter and helper for rep stos
; register esi text index
; register edi screen line buffer index
;
DispNewScreen:test byte [mode], EM | PI | NE
	jz NoEmBlock
	mov ecx,[showblock]		;transfering Emacs's mark/point into....
	jecxz NoEmBlock			;....WS's block display system
	mov ecx,[EmaMark]
	jecxz NoEmBlock
	mov eax,edi
	cmp ecx,eax
	jb EmBlock
	xchg eax,ecx
EmBlock:mov [blockbegin],ecx
	mov [blockende ],eax
;-------
NoEmBlock:call GetEditScreenSize	;check changed tty size
	xor eax,eax
	mov byte[isbold],al
	mov byte[inverse],al
	mov [zloffst],eax
	mov [columne],eax
	mov [fileptr],edi		;for seeking current cursor pos
	push edi			;&&**##
	call CountColToLineBeginVis	;i.e. expanding TABs
	mov ebx,[columns]
	lea ebx,[ebx-4]			;03 Jun 2001
	cmp eax,ebx
	jb short DispShortLine
	sub eax,ebx
	inc eax
	mov [zloffst],eax
DispShortLine:call LookPgBegin 		;go on 1st char upper left on screen
	mov esi,edi			;esi for reading chars from text
	mov ecx,[lines]
%ifndef YASM
	jecxz Kviex
%else
	or ecx,ecx
	jz near Kviex
%endif
	cld
	mov bh,-1			;first line
DispNewLine:inc bh			;new line
	mov edi,screenline		;line display buffer
	xor edx,edx			;reset char counter
	mov bl,0 			;reset screen column to 0
%ifdef LESSWRITEOPS
	call SetColor2			;set initial character color per each line
%endif
DispCharLoop:
	cmp esi,[fileptr]		;display char @ cursor postion ?
	jnz DispCharL1
	cmp byte[tabcnt],0
	jnz DispCharL1
	mov [kurspos],ebx
	mov eax,[zloffst]		;chars scrolled left hidden
	add al,bl
	add [columne],eax
%ifdef CURSORMGNT
	stc
	call SetInverseStatus
	jnc DispEndLine
%endif
DispCharL1:call SetColor		;set color if neccessary
;-------
DispEndLine:cmp esi,ebp
	ja FillLine			;we have passed EOF, so now fill rest of screen
	cmp byte[tabcnt],0
	jz ELZ
	dec byte[tabcnt]
	jmp short ELZ2
ELZ:	cmp esi,ebp
	jnz ELZ6
	inc esi				;set esi>ebp will later trigger  "ja FillLine"
	jmp short ELZ2
ELZ6:	lodsb
	cmp al,TABCHAR
	jnz ELZ3
	call SpacesForTab		;ah = space_up_to_next_tab location
	dec ah				;count out the tab char itself
	mov byte[tabcnt],ah
ELZ2:	mov al,SPACECHAR
ELZ3:	cmp al,NEWLINE
	jz FillLine
%ifdef W32LF
	cmp al,RETURN
	jz ELZ5				;keep 0dh "invisible"
%endif
	cmp al,SPACECHAR
	jae ELZ9			;simply ignore chars like carriage_return etc.
ELZ99:	mov al,'.'
ELZ9:	
%ifndef W32
	cmp al,7fh
	jb ELZ7
	je ELZ99
%ifndef UTF8
	mov al,'.'
%else
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	jne YXCVB
	mov al,'.'
YXCVB:
%endif
%endif
ELZ7:	
%endif
	cmp bl,byte [columns]		;screen width
	jae DispEndLine			;continue reading line until end
;-------
%ifdef UTF8
	mov ah,0
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	je CountByte
%endif
	push eax
	and al,0c0h
	cmp al,080h
	pop eax
	jz UByte234			;MSB 10...... =do not count 
	jb CountByte			;MSB 01...... 00...... count valid 7bit ASCII
	push eax
	mov al,byte [esi]		;check next byte for vaild UTF8 follower byte
	and al,0C0h
	cmp al,80h			;is UTF8 byte 2,3,4,..  ?
	pop eax
	jnz UByte234			;no do not count wrong UTF8 starter byte
CountByte:inc edx
	inc ah				;1
UByte234:cmp edx,[zloffst]
	jbe ELZ5			;loaded new char (but won't display)
	stosB
	add bl,ah			;i.e. add 0 or 1 to curser column position counter
%else
	inc edx
	cmp edx,[zloffst]
	jbe ELZ5			;loaded new char (but won't display)
	stosB
	inc bl
%endif
;-------
%ifdef CURSORMGNT
	clc
	call SetInverseStatus
%endif
ELZ5:	jmp DispCharLoop
;-------
FillLine:push ecx			;continue rest of line
	mov ecx,[columns]		;width
	sub cl,bl
	mov al,SPACECHAR		;fill with blanks
	jecxz FillLine2
	cmp byte[inverse],1		;special cursor attribute?
	jnz FillLine1
	stosB				;only 1st char with special attribute
%ifdef CURSORMGNT
	clc
	call SetInverseStatus
%endif
	dec ecx				;one char less
	jz FillLine2
FillLine1:
	rep stosB			;store the rest blanks
FillLine2:pop ecx
	mov byte[edi],0
	call ScreenLineShow
	dec ecx
	jnz near DispNewLine
	pop edi				;&&**##	;OLD: mov edi,[fileptr]	;=restore text pointer
	jmp RestKursPos
;----------------------------------------------------------------------
; three helper subroutines called by DispNewScreen
; dealing ESC sequences for character attributes
; 
%ifdef CURSORMGNT
SetInverseStatus:
	push ecx		;returns zero flag
	push esi
	jnc SIS1
	cmp byte [insstat],1
	stc
	jnz SIS4
	mov byte[inverse],1
	mov esi,reversevideoX
	add esi,[revvoff]		;switch between esc seq for linux or Xterm
	jmp short SIS2
SIS1:	cmp byte[inverse],1
	jnz SIS3
	mov byte[inverse],0
;-------continued...
%endif
;------
; next presented in 2 versions: one for Win32, one for Terminals
;
%ifdef W32 ;------------- this can't be done via ESC seq ----------------
SIS6:	mov byte[isbold],0
SIS5:	mov eax,DARKWHITE
SIS2:	mov ecx,edi
	sub ecx,screenline
	mov edx,ecx			;current pos in columne
	shl ecx,1
	mov edi,attribline
	add edi,ecx
	mov ecx,[columns]
	sub ecx,edx			;only current pos up to line end
	rep stosw
SIS3:	clc
SIS4:	POP_ALL
	ret
SetColor:				;expects cy flag:bold /  nc:normal
	PUSH_ALL
	call IsShowBlock
	jnc SCEsc1
	cmp byte [isbold],1		;never set bold if it is already bold
	jz SIS4
	mov byte [isbold],1
SCEsc2:	mov eax,WHITE
	jmp short SIS2
SCEsc1:	cmp byte [isbold],0		;ditto
	jz SIS4
	jmp short SIS6
;-------
SetColor2:PUSH_ALL
	call IsShowBlock
	jnc SIS5
	jmp short SCEsc2
%else ;---------------------- TERMINAL part -----------------------------
SIS6:	mov byte[isbold],0
SIS5:	mov esi,bold0
SIS2:	push byte boldlen
	pop ecx
	rep movsb
SIS3:	clc
SIS4:	pop esi
	pop ecx
	ret
;-------
SetColor:push ecx			;expects cy flag:bold /  nc:normal
	push esi
	call IsShowBlock
	jnc SCEsc1
	cmp byte [isbold],1		;never set bold if it is already bold
	jz SIS4
	mov byte [isbold],1
SCEsc2:	mov esi,bold1
	jmp short SIS2
SCEsc1:	cmp byte [isbold],0		;ditto
	jz SIS4
	jmp short SIS6
;-------
%ifdef LESSWRITEOPS
SetColor2:
	push ecx
	push esi
	call IsShowBlock
	jnc SIS5
	jmp short SCEsc2
%endif
%endif ;----------------- end of double part -----------------------------
;
;-------
; a little helper for SetColor* functions
;
IsShowBlock:cmp byte [showblock],0
	je SBlock
	cmp dword [blockbegin],0
	je SBlock
	cmp [blockbegin],esi
	ja SBlock
	cmp esi,[blockende]
	jb SB_ret
SBlock:	clc
SB_ret:	ret
;-------
; this helper for DispNewScreen checks screen size before writing on screen
; FIXME: adjusting edit screen resize works with xterm, but not with SVGATextMode
;
GetEditScreenSize:
%ifdef W32
	push dword csbi
	push dword [hout]
	call GetConsoleScreenBufferInfo
	or eax,eax
	mov eax,[csbi]
	jnz noerr
	mov eax,0x00190050		;i.e. (80<<16)+24  (assume 80x25)
noerr:	mov byte [columns],al
	shr eax,16
	dec eax
	mov byte [lines],al		;columns > 255 are ignored...
	ret
%else
	mov ecx,TERMIOS_WSIZE
	mov edx,winsize
	call IOctlTerminal
	mov eax,[edx]			;each 16 bit lines,columns
	cmp eax,0x0000FFFF		;some give no columns info..?
	jb iserr
 	or eax,eax
 	jnz noerr
iserr: 	mov eax,0x00500018		;i.e. (80<<16)+24  (assume 80x24)
noerr:	dec eax				;without status line ('dec al' are 2 byte!)
	mov byte [lines],al
	shr eax,16
	mov byte [columns],al		;columns > 255 are ignored...
	ret
%endif
;----------------------------------------------------------------------
;
; LOWER LEVEL screen acces function (main +2 helpers)
; this function does write the line buffer to screen i.e. terminal
;
; at first 2 special entry points:
WriteTwo:mov [screenline],ecx
StatusLineShow:
%ifdef W32
	push edi
	mov ecx,[columns]
	shr ecx,1
	mov eax,YELLOW_BLUE_TWICE
	mov edi,attribline
	rep stosd
	pop edi
	mov edx,[kurspos2]
	call sys_writeKP		;set cursor pos before reading chars
%endif
	xor ecx,ecx			;0 for bottom line
;-------
ScreenLineShow:PUSH_ALL			;expecting in ecx screen line counted from 0
%ifdef LESSWRITEOPS
%ifdef W32				;screen attrib caching
	mov eax,[columns]
	mul ecx				;setting edx to 0
	mov ebx,edx			;flag
	lea edi,[eax+attribbuffer]
	cld
	mov esi,attribline
Xsl3:	lodsw
	cmp edi,attribbuffer_end	;never read/write beyond buffer
	jnb Xsl5
	cmp ax,[edi]
	jz Xsl4
	mov [edi],ax
Xsl5:	inc ebx				;set flag whether line need redrawing
Xsl4:	inc edi
	inc edi
	or al,al
	jnz Xsl3
%else
	xor ebx,ebx			;flag
%endif
;-------
	mov eax,[columns]
	lea eax,[eax+32]		;estimated max ESC sequences extra bytes (i.e. boldlen*X) (BTW add eax,32 islonger)
	mul ecx				;setting edx to 0
	lea edi,[eax+screenbuffer]
%else
	xor edx,edx			;counter
%endif
	cld
	mov esi,screenline
sl3:	lodsb
	inc edx				;count message length to write
%ifdef LESSWRITEOPS
	cmp edi,screenbuffer_end	;never read/write beyond buffer
	jnb sl5
	cmp al,[edi]
	jz sl4
	mov [edi],al
sl5:	inc ebx				;set flag whether line need redrawing
sl4:	inc edi
%endif
	or al,al
	jnz sl3
	dec edx				;one too much
%ifdef LESSWRITEOPS
	or ebx,ebx			;redraw ?
	jz NoWrite
%endif
	push edx
	xor edx,edx
	mov dh,byte [lines]
	sub dh,cl
%ifdef W32_EXTENDED_IO
	pop ebx				;len
	shl edx,8
	and edx,00ff0000h		;only line# (column is always 0)
	push edx			;cursor data
;-------
	push dword w32result
	push edx			;cursor
	push ebx			;length
	push dword screenline
	push dword [hout]
	call WriteConsoleOutputCharacterA
;-------
	pop edx
	push dword w32result
	push edx			;cursor
	push ebx			;length
	push dword attribline
	push dword [hout]
	call WriteConsoleOutputAttribute
%else
	;this works on both Terminal and W32, ...
	;...but is suboptimal and slow on W32
	call sys_writeKP		;set cursor pos before writing the line
	pop edx
	push ecx
	mov eax,screencolors1		;set bold yellow on blue
	call sys_writeSLColors		;special for status line (ecx==0)
	mov ecx,screenline		;second argument: pointer to message to write
	call WriteFile0
;-------
	pop ecx
	mov eax,screencolors0		;reset to b/w
	call sys_writeSLColors		;special for status line (ecx==0)
	mov edx,[kurspos2]
	call sys_writeKP		;restore old cursor pos
%endif
NoWrite:POP_ALL
	ret
;-------
; a helper for ScreenLineShow
;
sys_writeSLColors:
%ifndef W32
	jecxz syswSL			;do nothing if not in status line
	ret
syswSL:	PUSH_ALL
	xchg eax,ecx			;parameter points to ESC-xxx color string
	push byte scolorslen
	pop edx
	call WriteFile0
	POP_ALL
%endif
	ret
;----------------------------------------------------------------------
;
; getting line INPUT from terminal / UNDER CONSTRUCTION
;
; expecting pointer to message text in esi
;
InputStringWithMessage0:mov esi,extext
InputStringWithMessage:call WriteMess9MakeLine
	mov ecx,optbuffer
	push byte optslen
	pop edx
	jmp short InputString
;-------
InputString00:mov ecx,suchtext
InputString0:call WriteMess9MakeLine
	mov edx,maxfilenamelen
; expecting input line buffer in ecx
; expecting max count byte in edx
; return length in eax, CY for empty string (or user abort)
;
InputString:push ecx
	push edi
	push byte 2
	pop eax
	xchg eax, [VICmdMode]
	push eax			;LONGER: push dword [VICmdMode], mov byte [VICmdMode],2
	push dword [kurspos2]
	mov ebx,[columns]
%ifndef LINUX
	dec ebx				;*BSD do not use lower right screen place...
%endif					;...due some unwanted vertical scrolling
	lea ebx,[ebx-stdtxtlen]
	cmp edx,ebx			;TODO should enable some scrolling:
	jb IS8				;not yet ready, so truncate at end of line
	mov edx,ebx			;edx == max chars
IS8:	xor ebx,ebx			;ebx == chars in buffer
	mov edi,ecx			;edi == pointer on current char
;-------				;ecx == pointer to begin of readline text
;
IS0:	push ebx			;local loop starts here
	push edx
	push ecx
	PUSH_ALL
	mov esi,ecx
	lea edi,[screenline+stdtxtlen]
	mov ecx,ebx
	cld
	rep movsb			;copy line buffer into screen display buffer
	mov ecx,edx
	sub ecx,ebx
	mov al,32			;fill up with blanks
	rep stosb
	POP_ALL
;-------
	mov ebx,edi			;next lines for setting cursor position
	sub ebx,ecx
	add bl,stdtxtlen		;offset+column
%ifdef UTF8
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	je noUTF_I
%endif
	mov esi,ecx
ISloopUTF8:lodsb
	and al,0C0h
	cmp al,080h
	jne ISdncUTF8
	dec bl				;do not count byte 2,3,4,....
ISdncUTF8:cmp esi,edi
	jb ISloopUTF8
noUTF_I:
%endif
	mov bh,byte[lines]		;line#
	mov [kurspos2],ebx
;-------
%ifdef LESSWRITEOPS
	mov byte [screenbuffer],0	;switching off usage of buffer v0.7
%endif
	call StatusLineShow		;show input
	call GetChar			;read next
	pop ecx
	pop edx
	pop ebx
	cld
;-------
	call IsViMode
	jnz NO_VI01
	cmp al,0
	je ISA
NO_VI01:call CheckUserAbort
	jne IS9
ISA:	xor ebx,ebx			;length 0 triggers CY flag
IS1j:	jmp IS1
IS9:	cmp al,RETURN
	je IS1j
	cmp al,8			;^H (translated DEL)
	jne IS2
DNHloopUTF8:
	cmp edi,ecx			;@left border?
	je IS0
;-------
	dec edi				;go 1 left
%ifdef UTF8
%ifdef UTF8RTSx_wont			;Keep UTF8 bytes together even in deleting
	cmp byte [isUTF8],0
	je noUTF_J
%endif
	mov al,byte [edi]		;check for UTF byte 2,3,4,..
	and al,0c0h
	cmp al,080h
	je DNHloopUTF8
noUTF_J:
	jmp Delete1
%else
	jmp short Delete1		;and continue at delete key
%endif
;-------
IS2:	cmp al,0			;marker of cursor keys etc.
	jne NoSpecialKey
	cmp ah,5			;end
	jne NotEnd
	lea edi,[ecx+ebx]
IS0j:	
%ifdef UTF8
	jmp IS0
%else
	jmp short IS0
%endif
NotEnd:	cmp ah,0			;home
	jne NotHome
	mov edi,ecx
	jmp short IS0j
NotHome	cmp ah,3			;left <-
	jne NotLeft
NHloopUTF8:
	cmp edi,ecx
	je IS0j
	dec edi
%ifdef UTF8
%ifdef UTF8RTSx_wont			;see KeyLeft comment
	cmp byte [isUTF8],0
	je noUTF_K
%endif
	mov al,byte [edi]
	and al,0c0h
	cmp al,080h
	je NHloopUTF8
noUTF_K:
%endif
	jmp short IS0j
NotLeft:cmp ah,4			;right ->
	jne NotRight
	lea esi,[ecx+ebx]
NLloopUTF8:
	cmp edi,esi
	je IS0j
	inc edi
%ifdef UTF8
%ifdef UTF8RTSx_wont			;see KeyRight comment
	cmp byte [isUTF8],0
	je near IS0
%endif
	mov al,byte [edi]
	and al,0c0h
	cmp al,080h
	je NLloopUTF8
noUTF_L:
%endif
IS0jj:	jmp short IS0j
NotRight:cmp ah,8			;Insert
	jne NotIns
	not byte [insstat]
%ifdef NEW_CURSOR_MGNT
	cmp byte [insstat],1
	jne short NCM
	call SetCursorBlock
	jmp short IS0j
NCM:	call SetCursorNormal
%endif
	jmp short IS0j
NotIns:	cmp ah,9			;Del
	jne NoSpecialKey
Delete1:lea esi,[ecx+ebx]		;do not delete at last character position
	cmp edi,esi			;...or in empty buffer...
	je IS0jj
%ifdef UTF8
	push ecx
	push edi
	push esi
	mov ecx,ebx			;TODO: check this
	lea esi,[edi+1]
	dec ebx				;decrease char count in buffer	
DeloopUTF8:
%ifdef UTF8RTSx_wont			;Keep UTF8 bytes together even in deleting
	cmp byte [isUTF8],0
	je noUTF_M
%endif
	mov al,byte [esi]
	and al,0C0h
	cmp al,080h
	jne DeUTF8
	inc esi				;delete one more at UTF-8 byte 2,3,4,....
	dec ebx				;decrease char count in buffer
	jmp short DeloopUTF8
noUTF_M:
DeUTF8:	cld
	rep movsb			;move all in buffer 1 char to left
	pop esi
	pop edi
	pop ecx
%else
	PUSH_ALL
	mov ecx,ebx			;TODO: check this
	lea esi,[edi+1]
	cld
	rep movsb			;move all in buffer 1 char to left
	POP_ALL
	dec ebx				;decrease char count in buffer
%endif
	jmp short IS0jj
;-------
NoSpecialKey:
	cmp al,SPACECHAR
	jb short IS0jj

%ifdef UTF8
%ifdef UTF8RTSx_wont			;Keep UTF8 bytes together
	cmp byte [isUTF8],0
	je noUTF_N
%endif
	push ebx
	mov bl,al
	and bl,0C0h
	cmp bl,080h			;byte 2,3,4.. always insert
	pop ebx
	je INSrt
noUTF_N:
%endif
	cmp byte [insstat],1
	jz INSrt
	lea esi,[ecx+ebx]
	cmp edi,esi
	jnz NO_INSERT
INSrt:	PUSH_ALL
	mov eax,edi
	lea edi,[ecx+edx+1]		;end of buffer space
	lea esi,[edi-1]
	mov ecx,edi
	sub ecx,eax
	std
	rep movsb			;move all in buffer 1 char to right
	POP_ALL
	inc ebx
NO_INSERT:cld
	stosb
%ifdef UTF8
%ifdef UTF8RTSx_wont			;Keep UTF8 bytes together
	cmp byte [isUTF8],0
	je noUTF_O
%endif
	mov esi,edi
	dec esi
NI_loopUTF8:inc esi
	mov al,byte [esi]
	and al,0C0h
	cmp al,080h			;delete byte 2,3,4,....
	je NI_loopUTF8
;-------
	cmp esi,edi
	je NI_UTF8rdy
	PUSH_ALL
	mov ecx,ebx			;TODO: check this
	cld
	rep movsb			;move all in buffer 1 char to left
	POP_ALL
noUTF_O:
NI_UTF8rdy:
%endif
	cmp ebx,edx
	jb IS0j
;-------
IS1:	xor eax,eax
	mov byte [ecx+ebx],al		;make asciz string
	pop dword [kurspos2]
	pop dword [VICmdMode]		;restore original vi mode
	pop edi
	pop ecx
	xchg eax,ebx
	cmp al,1			;set cy flag if empty string (len always <256)
ISready:ret				;eax length (but is < 255)
;----------
;
; GetChar (main function for kbd input)
;
ReadChar:mov eax,edi
	xchg eax,[old] 			;for ^QP
	mov [veryold],eax
GetChar:call ReadOneChar		;ah=0xFF for usual keys
%ifdef W32
	cmp ah,0FEh			;cursor key		
	jnz GC33
	shl eax,8
	ret
GC33:	cmp ah,0FDh			;ALT key
	jnz GC34
	and al,5fh			;toupper
	jmp short NOVI7
GC34:
%endif
	cmp al,7Fh
	jne short RC_No7F		;special case: remap DEL to Ctrl-H
%ifndef FREEBSD
	mov al,8
%else
	mov al,7
%endif
RC_No7F:
;-------
%define DoNo 10
;-------
;
; vi needs special handling of the ESC key
;
	call IsViMode
	jz short ISVI7
	cmp al,27 			;ESC ?
	jnz ISready
	call ReadOneChar		;dont care whether '[' or 'O' (should be [ for vt220 family  O for vt100 family)
	jmp short NOVI7
;-------
ISVI7:	cmp byte [VICmdMode],1
	jne NoCMDmode
	cmp al,27
	je ESCpressed
	cmp al,VIsize
	ja near Other
	mov ebx,VIcmdTable		;process command mode keys......
	jmp RCready_0			;....and ready
;-------
ESCpressed:call ReadOneChar
	cmp al,'['			;decide: it's a cursor key?
	je near Other			;yes, contine
	jmp short NoCursorKey		;no push back char into buffer and exit
NoCMDmode:cmp al,27 			;ESC ?
	jnz ISready
	call KeyVImode1			;ESC pressed in EDIT Mode
%ifdef BEOS
	call RestoreStatusLine
%else
%ifdef SYS_select
	PUSH_ALL
	call Select			;differ between ESC and ESC_cursor_keys
	POP_ALL
	jz isSingleEscape
%endif
%endif
	call ReadOneChar
	cmp al,'['			;starting sequence of cursor key found?
	je IsCursorKey			;pressed ESC, but do _NOT_ switch init cmd mode
NoCursorKey:mov byte [VIbufch],al	;push char back into read buffer due it's not a cursor key
	mov al,DoNo			;do nothing
	jmp short JmpRCready
isSingleEscape:mov al,3			;3 is keyLeft (i.e. entry #3 jumptab1)
	jmp short JmpRCready		;keyLeft is what a real vi user expects here ;)
;-------
IsCursorKey:call KeyVImode0		;reset mode to 'no_command' and continue
;-------
NOVI7:	cmp byte [mode],NE		;ALT keys are currently used for nedit mode...
	jnz NONE7
	cmp al,'i'
	jnz NOi
	mov al,0x10
	jmp short JmpRCready
NOi:	cmp al,'I'
	jnz NONE7
	mov al,0x10
	jmp short JmpRCready
NONE7:	call IsEmMode
	jnz NOEM7			;ALT keys are currently used for Emacs mode...
	cmp al,'%'			;...except altH for online Help
	jne NoAltPer
	mov al,0x28
JmpRCready:jmp short RCready_1
NoAltPer:cmp al,'<'
	jne NoAltLt
	mov al,0x0e
	jmp short RCready_1
NoAltLt:cmp al,'>'
	jne NoAltGt
	mov al,0x0f
	jmp short RCready_1
NoAltGt:and al,0x5F			;to upper case
	sub al,'B'			;1at in table
	js Other
	cmp al,ATsize
	ja Other
	mov ebx,EmaAltTable
	jmp short RCready_0
NOEM7:	and al,0x5F
	cmp al,'H'
	jnz Other
	mov al,0x3D
	jmp short RCready_1
;-------
Other:	
%ifdef W32
	ret
%else
	call ReadOneChar
	cmp al,'8'
	ja NoNumber
	push eax			;0,1,2....8  (i.e. 9 keys)
	call ReadOneChar
	xchg eax,ebx
	pop eax
	cmp bl,'~'			;if it's a number we expect following a '~'
	jne GetCharJmp
NoNumber:sub al,'0'
	cmp al,9
	jb IsNumber
%ifdef QNX
	sub al,('@'-'0'-9)		;scantable starts with ESC[@
%else
	sub al,('A'-'0'-9)
%endif
	cmp al,9
	jb GetCharJmp
	cmp al,STsize
	ja GetCharJmp
IsNumber:mov ebx,ScanTable
%endif
RCready_0:xlatb
RCready_1:shl eax,8			;shift into ah (ah always != 0xFF)
	ret
GetCharJmp:jmp near GetChar
;-------
; called by ReadChar/GetChar
;
ReadOneChar:call IsViMode
	jnz NOVI4
	xor eax,eax
	xchg eax,[VIbufch]		;preread char in buf?
	or eax,eax
	jne RoneC
NOVI4:	mov ecx,read_b			;pointer to buf
	xor edx,edx
	inc edx				;mov edx,1  (length)
	call ReadFile0
%ifdef SELFTEST				;for NON_INTERACTIVE mode exit on EOF!
	jnz Cont
	jmp KeyCtrlKX
Cont:
%endif
	mov eax,[ecx]			;[read_b]
%ifdef W32_EXTENDED_IO
	ret
%endif
RoneC:	mov ah,0xFF
	ret
;----------------------------------------------------------------------
;
; L O O K functions
; search special text locations and set register edi to
;
LookBackward:				;set EDI to 1 before LINEFEED (0Ah) i.e., 2 before start of next line
	push ecx
	push ebx
	xor ebx,ebx
	cmp byte[edi-1],NEWLINE		;at BOL ?
	jz LBa3
	cmp byte[edi],NEWLINE		;at EOL ?
	jnz LBa1
	dec edi				;at EOL ? start search 1 char earlier
	inc ebx				;increase counter
;-------
LBa1:	mov ecx,99999
	mov al,NEWLINE
	std
	repne scasb
	lea eax,[ebx+99997]		;mov eax,99997 / add eax,ebx
	sub eax,ecx
LBa5:	pop ebx
	pop ecx
	jmp short CheckBof
;-------
LBa3:	xor eax,eax
	dec edi
	dec edi
	jmp short LBa5
;-------
LookForward:
	push ecx			;don't touch edx (if called by BZNLoop only)
	mov ecx,99999
	mov al,NEWLINE
	cld
	repne scasb
	mov eax,99998
	sub eax,ecx
	pop ecx
	dec edi
CheckEof:cmp edi,ebp			;ptr is eof-ptr?
	jnz CheckEnd			;Z flag if eof
	jmp short CheckENum
CheckBof:cmp edi, sot-1
	ja CheckEnd
CheckENum:mov byte [numeriere],1	;if bof
CheckEnd:ret
;-------
LookPgBegin:mov edx,[kurspos2]		;called by DispNewScreen to get sync with 1st char on screen
	movzx ecx,dh			;called by KeyCtrlQE  (go upper left)   OLD: xor ecx,ecx mov cl,dh
	inc ecx				;'inc cl' are 2 Bytes
	jmp short LookPU2
;-------
LookPgEnd:mov edx,[kurspos2]		;goes 1st char last line on screen
	mov ecx,[lines]
	sub cl,dh
        jmp short LookPD2
;-------
LookLineUp:push byte 2			;2 lines: THIS line and line BEFORE
	pop ecx
	dec dword [linenr]
	jmp short LookPU2
;-------
LookLineDown:push byte 2		;2 lines: THIS and NEXT line
	pop ecx
	inc dword [linenr]
	jmp short LookPD2
;-------
LookPageUp:mov ecx,[lines]
	dec ecx				;PgUp,PgDown one line less
LookPU1:sub [linenr],ecx
	inc ecx
LookPU2:call LookBackward
	inc edi				;inc keeps CY flag!
	jb LookPUEnd			;if BOF
	loop LookPU2			;after loop edi points to char left of LINEFEED (0ah)
LookPUEnd:inc edi			;now points to 1st char on screen or line
	ret
;-------
LookScrDn:xor ecx,ecx
	inc ecx
	jmp short LookPD1
LookScrUp:xor ecx,ecx
	inc ecx
	jmp short LookPU1
LookHalfPgUp:mov ecx,[lines]		;vi special
	dec ecx
	shr ecx,1
	jmp short LookPU1
LookHalfPgDn:mov ecx,[lines]
	dec ecx
	shr ecx,1
	jmp short LookPD1
;-------
LookPgDown:mov ecx,[lines]
	dec ecx				;PgUp,PgDown one line less
LookPD1:add [linenr],ecx
	inc ecx
LookPD2:call LookForward
	jz LookPDEnd			;(jmp if EOF)
	inc edi				;1st char next line
	loop LookPD2
	dec edi				;last char last line
LookPDEnd:sub edi,eax			;1st char last line
	ret
;----------------------------------------------------------------------
;
; some more CHECK functions
;
CheckBlock:cmp byte [showblock],1	;returns CY if error else ok: NC
	jc CheckBlockEnd
	mov esi,[blockende]
	cmp esi, sot
	jb CheckBlockEnd
	mov esi,[blockbegin]		;side effect esi points to block begin
	cmp esi, sot
	jb CheckBlockEnd
	cmp [blockende],esi     	;^KK > ^KB ..OK if above!
CheckBlockEnd:ret
;-------
CheckImBlock:cmp [blockbegin],edi	;^KB mark > edi ?
	ja CImBlockEnd			;OK
	cmp edi,[blockende]		;edi > ^KK
CImBlockEnd:ret	          		;output:cy error / nc ok inside block
;-------
CheckMode:cmp byte [edi],NEWLINE	;checks for INSERT status
	jz ChModeEnd
	cmp byte [insstat],1
ChModeEnd:ret				;Z flag for ins-mode
;-------
; a special case called by DeleteByteCheckMarker
;
CheckMarker:				;edx is blockbegin (^KB)
					;ebx is deleate area end --- edi delete area start
	cmp edi,edx			;delete area start < ^KB marker ?
	ja CMEnd			;no
	cmp ebx,edx			;yes, but delete area end > ^KB ?
	jb CMEnd			;no
	mov edx,edi			;yes so block start (^KB) to delete area start
CMEnd:	ret
;----------------------------------------------------------------------
;
; C O U N T  functions
; to return number of chars up to some place
; (all of them are wrappers of Look....functions anyway)
;
CountToLineEnd:push edi
	call LookForward
	pop edi
	ret				;eax=chars up to line end
;-------
CountColToLineBeginVis:			;counts columns represented by chars in EAX
	call CountToLineBegin		;i.e. EXPAND any TAB chars found
	push esi
	xor edx,edx
	mov esi,edi			;startpoint
	sub esi,eax			;to bol
	dec esi
CCV1:	inc esi
	cmp esi,edi
	jae CCVend
%ifdef UTF8
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	je noUTF_P
%endif
	mov bl,byte [esi]
	and bl,0C0h
	cmp bl,080h			;do not count byte 2,3,4,.. in UTF8 chars
	jz CCV1
noUTF_P:
%endif
	cmp byte [esi],TABCHAR
	jz CCVTab
	inc edx				;count visible chars
	jmp short CCV1
CCVTab:	call SpacesForTab		;return space_up_to_next_tab in ah
	add dl,ah			;FIXME: now using 8 bits only
	jmp short CCV1
CCVend: mov [ch2linebeg],edx		;ch2linebeg: interface to Key... functions
	mov eax,edx			;eax: interface to DispNewScreen
	pop esi
%ifdef W32LF
	cmp byte[edi-1],RETURN
	jnz CCV2
	dec byte [ch2linebeg]		;don't count in RETURN char
CCV2:	
%endif
	ret
;-------
CountToLineBegin:push edi		;output eax=chars up there
	call LookBackward
	mov esi,edi			;side effect: set edi to 1st char in line
	pop edi
	ret
;-------
CountToWordBeginVIstyle:		;output eax=chars up there
	mov esi,edi
	cmp byte [esi],SPACECHAR
	ja CtWviStyle
CountToWordBegin:			;output eax=chars up there
	mov esi,edi
CountNLoop:cmp esi,ebp
	jz CTWend
	inc esi
%ifdef W32LF
	cmp byte [esi],RETURN
%else
	cmp byte [esi],NEWLINE
%endif
	jz CTWend
	cmp byte [esi],SPACECHAR	;below SPACE includes tab chars
	jbe CountNLoop
	cmp byte [esi-1],2fh
	ja CountNLoop
CTWend:	mov eax,esi
	sub eax,edi			;maybe =0
Goret:	ret
;-------
CtWviStyle:inc esi
%ifdef W32LF
	cmp byte [esi],RETURN
%else
	cmp byte [esi],NEWLINE
%endif
	jz CTWend
	cmp byte [esi],2fh
	ja CtWviStyle
	jmp short CountNLoop
;----------------------------------------------------------------------
;
; Online Help: show the message followed by common text
;
KeyHelp:
%ifdef USE_BUILTINHELP
	push dword [kurspos]
	PUSH_ALL
	xor eax,eax
	xchg eax,[showblock]		;hide an blockmarker
	push eax
	cld
	mov esi,sot			;save "buffercopysize" of text
	mov edi,buffercopy
	mov ecx,buffercopysize
	push edi
	push ecx
	push esi
;-------
	rep movsb
	call GetHelpText
	pop edi
	push edi			;i.e. mov edi,sot
;-------
	push edi
	rep movsb			;overwrite saved text with help message
	mov esi,helpfoot
	push byte helpfootsize
	pop ecx
	rep movsb
	mov ebp,edi			;set END_OF_HELP_TEXT pointer
	pop edi
	call DispNewScreen
	call ReadOneChar		;wait for a pressed char
;-------
	pop edi
	pop ecx
	pop esi				;former edi
	cld
	rep movsb			;restore textbuffer with saved patr
	pop dword [showblock]
	POP_ALL
	pop edx				;cursor pos
	jmp short SetKursPos
%else
	ret
%endif
;---------------------------------------------------------------------
;
; some CURSOR control functions
;
GoUp:	xor eax,eax
	jmp short UpDown
GoDown:	mov al,byte [lines]
	dec eax				;'dec al' are 2 byte!
	mov ah,-1
UpDown:	mov edx,[kurspos2]		;former was call getkurspos
	cmp al,dh
	jz Goret
	sbb dh,ah			;ONLY here we change curent line of cursor
	jmp short SetKursPos
;-------
; set cursor to some desired places
;
KeyVICmdz:call ReadOneChar
	cmp al,'.'
	je KeyEmaCtrlL
	ret
;-------
KeyVI_M:call LookPgBegin
	call LookHalfPgDn
	test byte[lines],1
	jnz KeyEmaCtrlL
	call LookLineDown
;------- cont
KeyEmaCtrlL:call CountToLineBegin
	mov dh,byte [lines]		;move cursor to center line (and later redisplay)
	shr dh,1
	mov dl,al
	jmp short SetKursPos
KursorFirstLine:xor edx,edx
	jmp short SetKursPos
KursorLastLine:mov dh,byte [lines]
	dec dh
	mov dl,0
	jmp short SetKursPos
KursorStatusLine:mov dh,byte [lines]
	mov dl,stdtxtlen
	jmp short SetKursPos
RestKursPos:mov edx,[kurspos]
SetKursPos:mov [kurspos2],edx      	;saves reading cursor pos   (0,0)
sys_writeKP:PUSH_ALL
%ifdef W32
	shl edx,8			;linux cursorpos in dh/dl   -   w32 in edx 2*16bit
	mov dl,dh
	and edx,0x00FF00FF
	push dword edx			;xxxxyyyy x=line y=column
	push dword [hout]
	call SetConsoleCursorPosition
%else
	call make_KPstr
	mov ecx,setkp 			;second argument: pointer to message to write
	push byte setkplen		;third argument: message length
	pop edx
	call WriteFile0
%endif
	POP_ALL
	ret
;-------
; make ESC sequence appropriate to most important terminals
;
%ifndef W32
;	;expecting cursor pos in dh/dl (0,0)
make_KPstr:cld
	mov edi,setkp			;build cursor control esc string db 27,'[000;000H'
	mov al,1Bh
	stosb				;init memory
%ifndef ARMCPU
	mov eax,'[000'
	stosd
	mov al,';'			;i.e. load eax with ';000'
	stosd
%else
	mov al,'['
	stosb
	mov al,'0'
	stosb
	stosb
	stosb
	mov al,';'
	stosb
	mov al,'0'
	stosb
	stosb
	stosb
%endif
	mov al,'H'
	stosb				;now we have written 10 chars
	lea edi,[edi-6]			;old was "mov edi,setkp+1+3" now using 1+3 == 10-6
	movzx eax,dh			;DH=line
	inc eax				;now counted from 1
	push edx
	call IntegerToAscii		;make number string
	pop edx
	mov edi,setkp+1+3+4		;column end
	movzx eax,dl			;DL=col
	inc eax				;now counted from 1
%endif
;-------continued...
; a general helper
;   expects integer# in eax
IntegerToAscii:
	or eax,eax
	jns ItoA1
	;int 3				;Assertation
	xor eax,eax			;this should never be
	inc eax
ItoA1:	push byte 10
	pop ecx
	std
	xchg eax,ebx			;ebx helper (xchg eax,.. is only 1 byte!)
Connum1:xchg eax,ebx
	cdq
	div ecx
	xchg eax,ebx			;save quotient (new low word)
	mov al,dl
	and al,0fh
	add al,'0'
	stosb
	or ebx,ebx
	jne Connum1
	cld
ITAret:	ret
;----------------------------------------------------------------------
;
; functions for INSERTING, COPYING and DELETING chars in text
;
DeleteByteCheckMarker:			;edi points to begin
	test byte [mode], WS | VI	;see above note at "jz NOWS8"
	jz DeleteByte
	lea ebx,[edi+eax]		;ebx points to end
	mov edx,[blockbegin]
	call CheckMarker
	mov [blockbegin],edx
	mov edx,[blockende]
	call CheckMarker
	mov [blockende],edx
DeleteByte:or eax,eax			;input in eax
	jz ITAret
%ifdef USE_UNDO
	call DataForUndoDelete
%endif
	push edi
	mov ecx,ebp			;end
	sub ecx,edi
	lea esi,[edi+eax]		;current + x chars
	sub ecx,eax
	cmp byte [mode],WS
	jz No_WS8
	add ecx,[EmaKiSize]
No_WS8:	inc ecx
	cld
	rep movsb
	neg eax				;"neg eax" is for continuing @InsertByte
	jmp short Ins0			;pending "pop edi"
;-------
Insert1Byte:xor eax,eax
InsertByte0:inc eax
;
; do NOT destroy eax
;
InsertByte:or eax,eax			;input: eax = # of bytes  edi = ptr where
	jz ITAret
	mov ecx,[maxlen]		;max_len+offset-eofptr=freespace(ecx)
	add ecx,sot
	sub ecx,ebp
	mov edx,[EmaKiSize]
	sub ecx,edx			;sub size of kill buffer from free space
	cmp ecx,eax			;cmp freespace - newbytes  ;>= 0 ok/ NC  <0 bad / CY
	jnc SpaceAva
	push byte ERRNOMEM
	pop dword [ErrNr]		;(mov dword[ErrNr],..  has 2 byte extra)
	call OSerror
	call RestKursPos
	stc
	ret
SpaceAva:push edi
%ifdef USE_UNDO
	call DataForUndoInsert
%endif
	mov esi,ebp			;end of text movsb-source
	lea ecx,[ebp+1]
	sub ecx,edi			;space count: distance between eof and current position
	lea edi,[ebp+eax]		;movsb-destination
	cmp byte [mode],WS
	jz ISWS8
	add ecx,edx			;add size of kill buffer to distance
	add edi,edx
	add esi,edx
ISWS8:	std
	rep movsB
Ins0:	pop edi				;here is the jmp destination from DeleteByte
;-------
	call SetChg			;i.e. mov byte [changed],CHANGED
	add ebp,eax
	test byte [mode], WS | VI	;for vi mode it would be enough to handle blockbegin
	jz NOWS8			;..because blockende is set at end of marker line..
	cmp edi,[blockende]		;..at HandleVImarker procedure
	jae Ins1
	add [blockende],eax
Ins1:	cmp edi,[blockbegin]
	jae Ins2
	add [blockbegin],eax
NOWS8:
	test byte [mode], EM | PI
	jz NO_EM02
	cmp edi,[EmaMark]
	jae Ins2
	add [EmaMark],eax
NO_EM02:
Ins2:	clc
	ret				;output:nc=ok/cy=bad /ecx=0/ eax inserted / -eax deleted
;-------
CopyBlock:call CheckBlock		;copy block, called by ^KC, ^KV
	jc MoveBlEnd
	call CheckImBlock
	jc MoveBlEnd
	mov eax,[blockende]
	sub eax,esi			;block len
	call InsertByte
	jc MoveBlEnd
	mov esi,[blockbegin]
;-------
MoveBlock:push edi			;input : esi=^KB edi=current
	mov ecx,eax			;don't use xchg here
	cld
	rep movsb
	pop edi
	clc				;nocarry->ok
MoveBlEnd:ret				;return eax=size
;----------------------------------------------------------------------
KeyVICmdyy:push edi
	call KeyHome
	mov [EmaMark],edi
	call KeyEnd
	inc edi				;add a line delimiter
	call KeyEmaAltW
	pop edi
KviRet:	ret
;-------
KeyVICmdy:call ReadOneChar
	cmp al,'y'
	je KeyVICmdyy
	cmp al,"'"
	jne MoveBlEnd
	call ReadOneChar
	cmp al,'a'			;" y'a "    only marker "a" supported
	jne MoveBlEnd
	mov ecx,[blockbegin]		;don't go further if no mark set
	jecxz MoveBlEnd
	call VIsetMarker
	call KeyEmaAltW
	mov edi,[blockbegin]
%ifdef W32
	jmp ISVI9
%else
	jmp short ISVI9
%endif
;
; some of the EM specials
;
KeyEmaCtrlY:
%ifdef W32
	cmp byte[mode],NE		;Nedit ^V
	jnz KECY
	PUSH_ALL
	push byte 0
	call OpenClipboard
	or eax,eax
	jz KECY3
	push byte CF_OEMTEXT
	call IsClipboardFormatAvailable
	or eax,eax
	jz KECY0
	push byte CF_OEMTEXT
	call GetClipboardData
	or eax,eax
	jz KECY0
	mov edi,ebp
	inc edi				;one after eof
	mov ecx,[maxlen]
	add ecx,sot			;the last possible byte
	xor ebx,ebx
	dec ebx				;init counter -1
	xchg esi,eax
	cld
Kloop:	lodsb
	inc ebx
	cmp edi,ecx
	jnb KECY2
	stosb
	or al,al
	jnz Kloop
KECY2:  mov [EmaKiSize],ebx	
KECY0:	call CloseClipboard
KECY3:	POP_ALL
KECY:
%endif
	mov ecx,[EmaKiSize]
%ifdef YASM
	or ecx,ecx
	jmp KeawRet
%else
	jecxz KeawRet
%endif
	xchg eax,ecx			;OLD mov eax,ecx 1 byte longer
	push eax
	call InsertByte
	pop ecx
	jc KeawRet			;no_space_error is handled in InsertByte
	mov esi,ebp
	inc esi
	mov [EmaMark],edi
	cld
	rep movsb
	call ShowBl0			;i.e. "mov byte [showblock],0"
	call IsViMode
	jz ISVI9
	call KeyEmaCtrlL
ISVI9:	jmp CQFNum
;-------
KeyEmaAltW2:PUSH_ALL
	mov edi,ebp
	inc edi
	call IsViMode
	jz KEW
;-------
	cmp byte [EmaCtrlK],1
	jnz KEW
	add edi,[EmaKiSize]
	add [EmaKiSize],eax
	jmp short KE2
KEW:	mov [EmaKiSize],eax
	mov [EmaKiSrc],esi
KE2:	mov ecx,eax
	cld
	rep movsb
	call ShowBl0			;i.e. "mov byte [showblock],0"
Keaw2:	mov byte [EmaCtrlK],1
	POP_ALL
KeawRet:ret
;-------
KeyEmaAltW:mov byte [VInolinebased],0
	PUSH_ALL
	mov ecx,[showblock]
	jecxz Keaw2
	mov ecx,[EmaMark]
	jecxz Keaw2
	mov eax,edi
	cmp ecx,eax
	jb KEAW
	xchg eax,ecx
KEAW:	sub eax,ecx			;eax end / ecx beg
	mov esi,ecx
	mov edi,ebp
	inc edi
	mov [EmaKiSize],eax
	mov [EmaKiSrc],esi
	xchg eax,ecx			;OLD mov ecx,eax 1 byte longer
	cld
	rep movsb
	call IsViMode
	jz KEAW3
	call ShowBl0			;i.e. "mov byte [showblock],0"
KEAW3:	
%ifdef W32
	cmp byte[mode],NE		;Nedit ^C
	jnz KEAW4
	push dword [EmaKiSize]
	push byte 0
	push dword [heap]
	call HeapAlloc
	or eax,eax
	jz KEAW4
	mov esi,[EmaKiSrc]
	mov edi,eax
	mov ecx,[EmaKiSize]
	cld
	rep movsb
	mov byte[edi],0			;ASCIIZ
;-------
	push dword [heap]
	push byte 0
	push eax			;push for later usage in HeapFree
	push eax			;push clip handle for SetClipboardData
	push byte 0
	call OpenClipboard
	or eax,eax
	jz KEAW8
	call EmptyClipboard
	push byte CF_OEMTEXT
	call SetClipboardData
	call CloseClipboard
KEAW8:	call HeapFree
KEAW4:
%endif
	POP_ALL
KeaWRet:ret
;----------------------------------------------------------------------
;
; functions reading/writing  text or blocks  from/into  files
;
NFnoarg:mov esi,helptext		;initial part of help
	mov edi,sot
	push byte helptextsize
	pop ecx
	push edi
	rep movsb
;-------
	call GetHelpText		;second part of help
	lea ebp,[ecx+sot+helptextsize]	;set END_OF_HELP_TEXT pointer
	rep movsb
	pop edi
	call DispNewScreen
;-------
	mov esi, filename
	mov ecx, filepath
	call InputString0
	jnc GetFile			;empty string not allowed here
	ret
;-------
KeyVICmdE:lea esi,[ecx+2]
	cmp byte [esi],SPACECHAR
	je KeaWRet
	PUSH_ALL			;save before load new
	call SaveFile
	POP_ALL
;-------continue
NewFile:cld
	call InitVars
%ifdef AMD64
	or rsi,rsi
	jz NFnoarg
	cmp byte [rsi],0
%else
	or esi,esi
	jz NFnoarg
	cmp byte [esi],0
%endif
	jz NFnoarg
	mov edi,filepath
NF1:
%ifdef W32
	lodsb
	cmp al,'"'
	jz NF1
	stosb
	cmp al,SPACECHAR		;truncate after blanks
	jnz NF3
NF4:	mov byte [edi-1],0
	jmp short GetFile
NF3:	cmp al,TABCHAR
	jz NF4	
%else
	lodsb
	stosb
%endif
NF2:	or al,al
	jnz NF1
;------- cont
GetFile:
%ifdef BEOS
	xor ebx,ebx
	mov edx,ebx
	dec ebx				;edx==0,ebx==-1
	mov ecx,filepath
%else
	mov ebx,filepath
%endif
	call OpenFile0
	mov edi,sot
	mov ebp,edi
	js NewFileEnd
%ifdef SYS_brk
	call Seek
	PUSH_ALL
	lea ebx,[eax+eax+max]		;twice filesize plus reserve = space for inserts
	mov [maxlen],ebx
	add ebx,text
	call SysBrk
	POP_ALL
	js OSejmp1			;OSerror
%else
	mov ebx,eax			;for FreeBSD memory is hard coded by maxlen
%endif
;-------
%ifdef SYS_fstat			;not for W32,BEOS
	call Fstat
	js OSejmp1			;OSerror
	mov eax,[fstatbuf+stat_struc.st_mode]
%ifdef FREEBSD
	mov ecx,eax
	and ecx,0F000h			;see /usr/include/sys/stat.h
	cmp ecx,8000h			;not for special files
	jz regFile
	push byte ERRNOREGFILE
	pop dword [ErrNr]
	jmp OSerror
regFile:
%endif	
	and eax,777q
	mov [perms],eax
%ifdef SYS_utime	
	mov eax,[fstatbuf+stat_struc.st_mtime]
	mov [accesstime+utimbuf_struc.modtime],eax
%endif
%endif					;end of code not for W32,BEOS
;-------
OldFile1:mov edx,[maxlen]		;i.e. either 'max' or filesize twice
	mov ecx,edi			;sot
	call Read_File
	xchg edx,eax			;mov edx,eax	bytes read
	js OSejmp0			;OSerror
	call CloseFile
OSejmp1:js OSejmp0			;OSerror
	lea ebp,[edx+sot]		;eof_ptr=filesize+start_of_text
NewFileEnd:mov byte [ebp],NEWLINE	;eof-marker
	clc
NFEnd2:	ret
;-------
;  save file (called by ^KS,^KX)
;
SaveFile:cmp byte [changed],UNCHANGED
	jz NFEnd2			;no changes: nothing to save
	mov esi, filesave
	call WriteMess9
;-------
	mov esi,filepath
	push edi
	mov edi,bakpath
	mov ebx,esi
	mov ecx,edi
	cld
SF0:	lodsb
	stosb				;copy to BAK file path
	or al,al
	jne SF0
	dec edi
	push byte '~'			;add BAK file extension
	pop eax
%ifdef ARMCPU
	stosb
	shr eax,8
	stosb	
	shr eax,8
	stosb
	shr eax,8
	stosb
%else
	stosd				;not stosb because ascii-ZERO
%endif
	pop edi
%ifdef BEOS
	push edi
	mov ebx,0xFFFFFFFF
	mov edx,ebx
	mov ecx,filepath
	mov edi,bakpath
%endif
%ifdef MAKE_BACKUP
	cmp dword [ecx],'/tmp'
	je no_ren
%ifdef SYS_readlink
	PUSH_ALL
	mov ecx,linkbuffer		;we are only interested whether symlink or not
	push byte linkbuffersize	;=4 byte dummy buffer
	pop edx
	call ReadLink
	POP_ALL
	jns CopyBAK			;no error means it's a symlink...
	call RenameFile                 ;...but plain files are easy to rename (ecx is filepath)
	jmp short no_ren		;...simlilar behave 'xemacs' and 'jed'
CopyBAK:call CopyToBackup		;we can't simply rename the link
%else
	call RenameFile                 ;ecx is filepath
%endif
no_ren:	;...continue here...
%endif
%ifdef BEOS
	pop edi
%endif
;-------
%ifdef W32
	mov ecx,CREATE_ALWAYS
	mov ebx,filepath
	mov edx,GENERIC_WRITE
%else
%ifdef BEOS
	mov ebx,0xFFFFFFFF
	mov ecx,filepath
	mov edx,0x777
%else
	mov ecx,O_WRONLY_CREAT_TRUNC
	mov edx,[perms]
%endif
%endif
	call OpenFile
OSejmp0:js OSejmp9			;OSerror
	xchg ebx,eax			;file descriptor  (xchg is only 1 byte)
;-------
%ifdef SYS_fchown
	mov ecx,[perms]
	call Fchmod
%endif
%ifdef SYS_fstat
	mov ecx,[fstatbuf+stat_struc.st_uid]
%ifdef UIDGID_WORD			;Linux special
	mov edx,ecx
	shr edx,16
	movzx ecx,cx			;OLD and ecx,0xffff
%else
	mov edx,[fstatbuf+stat_struc.st_gid]
%endif
	call ChownFile
%endif
;-------
	mov ecx,sot			;ecx=bof
	mov edx,ebp			;eof
SaveFile2:sub edx,ecx			;edx=fileesize= eof-bof
	call IsViMode
	jnz NoAddNL
	cmp byte [ebp-1],NEWLINE
	jz NoAddNL
	inc edx				;append NewLine char for VI mode
NoAddNL:call Write_File			;ebx=file descriptor
OSejmp9:js OSejmp			;OSerror
	push byte ERRNOIO
	pop dword[ErrNr]		;just in case of....
	cmp eax,edx			;all written?
%ifdef BEOS
	jnz near OSerror
%else
	jnz OSerror
%endif
	call CloseFile
	js OSejmp			;OSerror
SaveFile3:ret
;-------
;  save block (called by ^KW)
;
SaveBlock:call GetBlockName
	jc jcDE2
SaveBl2:
%ifdef W32
	mov ebx,blockpath
SaveBl3:mov ecx,CREATE_ALWAYS
	mov edx,GENERIC_WRITE
%else
%ifdef BEOS
	mov ebx,blockpath
SaveBl3:mov ecx,0xFFFFFFFF
	xchg ebx,ecx
	mov edx,0x241			;was 0x777 upto Dec 01
%else
	mov ebx,blockpath
SaveBl3:mov ecx,O_WRONLY_CREAT_TRUNC
	mov edx,PERMS
%endif
%endif
	call OpenFile
	js OSejmp			;OSerror
	mov ecx,esi			;= block begin
	mov edx,[blockende]
	xchg ebx,eax			;file descriptor  (xchg is only 1 byte)
	jmp short SaveFile2
;-------
; read a block into buffer (by ^KR)
;
ReadBlock:call GetBlockName
jcDE2:	jc near DE2
ReadBlock2:
%ifdef BEOS
	xor ebx,ebx
	mov edx,ebx
	dec ebx				;edx==0,ebx==-1
	mov ecx,blockpath
%else
	mov ebx,blockpath
%endif
	call OpenFile0
OSejmp:	js OSerror
	call Seek
	js OSerror
	push eax			;eax=fileesize
	call InsertByte
	pop edx				;file size
	jc SaveFile3			;ret if cy InsertByte told an error message itself
	mov ecx,edi			;^offset akt ptr
	call Read_File
	js preOSerror			;to delete inserted bytes (# in EDX)
	mov ecx,eax    			;bytes read
	call CloseFile
	js OSerror
	push byte ERRNOIO
	pop dword[ErrNr]		;just in case of....
	cmp edx,ecx			;all read?
	jnz OSerror
	ret
;-------
preOSerror:mov eax,edx			;count bytes
	call DeleteByte			;delete space reserved for insertation
OSerror:push edi
	mov edi,error+8			;where to store ASCII value of ErrNr
	mov eax,[ErrNr]
	push eax
	call IntegerToAscii
	pop ecx				;for getting error text via LookPD2
	cmp ecx,MAXERRNO
	ja DE0
	mov edi,errmsgs
	call LookPD2			;look message # (ecx) in line number #
	mov esi,edi
	mov edi,error+9
	mov ax,' :'
%ifdef ARMCPU
	stosb				;error+9 is not aligned
%else
	stosw
%endif
	push byte 80			;max strlen / compare errlen equ 100
	pop ecx
	rep movsb
DE0:	mov esi,error
	pop edi
DE1:	call WriteMess9
	call ReadOneChar		;was GetChar
DE2:	;continued...
;----------------------------------------------------------------------
;
; more STATUS LINE maintaining subroutines
;
RestoreStatusLine:PUSH_ALL		;important e.g. for asksave
	call InitStatusLine
	mov esi,mode
	mov ecx,[columns]		;width
	cmp cl,stdtxtlen+15+5+2		;this window is too small
%ifdef ARMCPU
	and ecx,0xFFFFFFFC		;get aligned
%endif
	jb near RSno_lineNr
	mov al,byte[changed]
	mov byte[screenline+1],al	;changed status
;-------
%ifndef LINUX
	lea eax,[ecx-13+screenline]	;FreeBSD or Beos ...
%else
	lea eax,[ecx-12+screenline]
%endif
	mov word[eax+8],'vi'		;vi does show mode info only.
	cmp byte [esi],VI		;vi doesn't get altH text because altH won't work...
	jnz RSL0			;...caused by different handling due single ESC keys
	cmp byte [VICmdMode],1
	jnz NOVI0
	mov ebx,'CMD '
	jmp short RSL1
RSL0:	mov dword [eax],'altH'
	mov dword [eax+4],'=hel'	;'p' is stored with editor mode name
	mov ebx,editmode
	mov edx,[ebx]
	cmp byte [esi],PI
	jnz No_PI1
	mov edx,[ebx+4]
No_PI1:	cmp byte [esi],EM
	jnz No_Em1
	mov edx,[ebx+8]
No_Em1:	cmp byte [esi],NE
	jnz No_Ne1
	mov edx,[ebx+12]
No_Ne1:	mov [eax+8],edx
;-------
NOVI0:	mov eax,' INS'			;Insert
	cmp byte [insstat],1
	jz RSL1
	mov eax,' OVR'			;Overwrite
RSL1:	mov [screenline+4],eax		;mode status
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	jnz RSL1a
	mov al,'7'			;"7bit_non_utf8" status
RSL1a:  mov byte [screenline],al
%endif
	mov edi,screenline+stdtxtlen
	lea ecx,[ecx-(stdtxtlen+15+5)]	;space for other than filename
	mov esi,filepath		;lea ... is shorter than sub ecx,stdtxtlen+15+5
RSL2:	lodsb
	or al,al
	jz RSL4
	stosb
	loop RSL2
RSL4:	mov edi,screenline-15
	add edi,[columns]
	mov eax,[columne]
	inc eax				;start with 1
	call IntegerToAscii
	mov byte [edi],':'		;delimiter ROW:COL
	dec edi
	mov eax,[linenr]
	call IntegerToAscii
RSno_lineNr:call StatusLineShow		;now write all at once
	POP_ALL
	stc				;error status only important if called via OSError
	ret
;-------
;
; write an answer prompt into status line
; (with and without re-initialisation)
; expecting esi points to ASCIIZ or 0A terminated string
;
WriteMess9MakeLine:call InitStatusLine
WriteMess9:PUSH_ALL
	mov edi,screenline
	cld
WriteMLoop:lodsb
	cmp al,LINEFEED			;for error messages
	jbe WriteMEnd
	stosb
	jmp short WriteMLoop
WriteMEnd:call StatusLineShow
	POP_ALL
	jmp KursorStatusLine
;-------
; a helper for other status line functions:
; simply init an empty line
;
InitStatusLine:PUSH_ALL			;simply init an empty line
	mov edi,screenline
	mov al,SPACECHAR
	mov ecx,[columns]
%ifndef LINUX
	dec ecx				;? FreeBSD
	js ISL				;should never be = -1
%endif
	cld
	rep stosb
	mov al,0			;prepare ASCIIZ string
	stosb
ISL:	POP_ALL
	ret
;-------
; read a file name for block operations
;
GetBlockName:PUSH_ALL
	mov esi,block
	mov ecx,blockpath
	call InputString0		;cy if empty string
	pushf
	call RestKursPos
	popf
	POP_ALL
	ret
;-------
; helper for NewFile
;
InitVars:mov byte [text],NEWLINE	;don't touch esi!
	call Unchg			;i.e. "mov byte [changed],UNCHANGED"
	call InitSomeVars		;set eax=0
	mov dword[old],sot
	inc eax				;set eax=1
	mov byte [VICmdMode],al
	mov dword [linenr],eax
	mov byte [insstat],al
	mov dword [maxlen],max
	mov dword [error],'ERRO'
	mov dword [error+4],'R   '
	mov dword [perms],PERMS
%ifdef SYS_fstat
	dec eax
	dec eax				;eax == -1 i.e. no changes in fchown
%ifdef UIDGID_WORD			;Linux special
	mov [fstatbuf+stat_struc.st_uid],eax	;both: giduid
%else
	mov [fstatbuf+stat_struc.st_gid],eax
	mov [fstatbuf+stat_struc.st_uid],eax
%endif
%endif
	jmp ShowBl1			;i.e. mov byte [showblock],1
;-------
InitSomeVars:
	xor eax,eax
%ifdef USE_UNDO
	mov [undobuffer],eax		;i.e. invalid pointer
 	mov dword[undoptr],undobuffer+4 ;init to first frame
%endif
	mov [EmaMark],eax
	mov dword [oldQFpos],eax
	mov byte[bereitsges],al
	mov [endeedit],al
InitSV1:mov [EmaKiSize],eax		;do not allow side effects
InitSV2:mov [blockbegin],eax		;i.e. NO block valid now
InitSV3:mov [blockende],eax
	ret
;-------
Seek:	xchg ebx,eax			;mov file_descriptor to ebx (xchg is 1 byte only)
	push byte 2			;FILE_END
	pop edx
	call SeekFile			;end
	js SeekRet
	xor edx,edx			;FILE_BEGIN
	push eax
	call SeekFile			;home
	pop eax
SeekRet:ret
;----------------------------------------------------------------------
;
; FIND/REPLACE related stuff
;
AskForReplace:mov esi,askreplace1
	call InputString00
	jc AskFor_Ex
	mov [suchlaenge],eax
	mov esi,askreplace2
	mov ecx,replacetext
	call InputString0
	jmp short GetOptions		;cy flag is allowed here 18.6.00
AskForFind:mov esi,askfind
	call InputString00
	jc AskFor_Ex
GetOptions:mov [repllaenge],eax
	test byte [mode],WS|NE
	jz GetOpt2
	mov esi,optiontext
	call InputStringWithMessage	;empty string is allowd for std options...
	call ParseOptions		;...(set in ParseOptions)
GetOpt2:clc
AskFor_Ex:jnc AFE2
	mov byte [bereitsges],0
AFE2:	pushf
	call RestKursPos
	popf
	ret
;-------
; check string for 2 possible options: C,c,B,b  (case sensitive & backward)
;
ParseOptions:push esi
	cld
	mov esi,optbuffer
	push byte 1
	pop dword[vorwarts]		;mov dword[vorwarts],1 is longer
	mov byte[grossklein],0dfh
Scan1:	lodsb
	and al,5fh			;upper case
	cmp al,'C'
	jnz notCopt
	xor byte[grossklein],20h	;result 0dfh,   2*C is like not C option
notCopt:cmp al,'B'
	jnz notBopt
	neg dword[vorwarts]		;similar 2*B is backward twice i.e. forward
notBopt:or al,al
	jnz Scan1
	pop esi
	ret
;-------
; the find subroutine itself
;
find2:	mov ebx,edi
find3:	lodsb
	or al,al			;=end?
	jz found
	cmp al,41h
	jb find7
	and al,ch
find7:	inc edi
	mov cl,byte [edi]
	cmp cl,41h
	jb find10
	and cl,ch
find10:	cmp al,cl
	jz find3
	mov edi,ebx
FindText:mov ch,[grossklein]		;ff or df
	mov esi,suchtext
	cld
	lodsb
	cmp al,41h
	jb find1
	and al,ch			;FIXME: check UTF-8 stuff !!
;-------
find1:	add edi,[vorwarts]		;+1 or -1 (increase or decrease pointer) 
	mov cl,byte [edi]
	cmp cl,41h
	jb find6
	and cl,ch
find6:	cmp al,cl
	je find2
	cmp byte[mode],PI		;is it Pico? (always searching forward)
	jnz find_WS			;no, continue
	cmp edi,[PicoSearch]		;yes, but did we search from BOF up to here?
	je notfound			;yes, so we did not found the text
	cmp edi,ebp			;no, but did we touch EOF?
	jb find1			;no, continue from here
	mov edi,sot-1			;yes, let's continue at BOF
	jmp short find1			;loop
find_WS:cmp edi,ebp
	ja notfound
find9:	cmp edi,sot			;this is needed for WStar backward option searching
	jnb find1
notfound:stc
	ret
found:	mov edi,ebx
	clc				;edi points after location
	ret
;----------------------------------------------------------------------
;
; some GENERAL helper functions
;
;
; Get.....ToInteger reads integer value from keyboard (only > 0)
;
GetOctalToInteger:push byte 7		;octal base-1
	jmp short GATI2
GetAsciiToInteger:push byte 9		;decimal base-1
GATI2:	call IsViMode
	jz ISVI8
	call InputStringWithMessage	;eax = al = length
	call AskFor_Ex			;repair status line & set cursor pos / preserve flags
ISVI8:	push byte 0
	pop esi				;preserve flags
	pop ebx				;bl == base-1
	xchg ecx,esi
	jc AIexit2
	cld
AIload:	lodsb				;eax bit 8..31 are 0
	sub al,'0'
	js AIexit
	cmp al,bl
	ja AIexit
	cmp bl,7			;if base==8
	je GATI3
	lea ecx,[ecx+4*ecx]
	lea ecx,[2*ecx+eax]		;mul 10 plus digit
	jmp short AIload
GATI3:	lea ecx,[8*ecx+eax]		;mul 8 plus digit
	jmp short AIload
%ifdef ARMCPU
AIexit:	cmp ecx,0			;ret ecx
%else
AIexit:	or ecx,ecx			;ret ecx
%endif
AIexit2:ret				;CY or ZR if error
;-------
;
; SpacesForTab expects current column in edx
; returns # spaces up to next tabulated location in AH
;
SpacesForTab:push ecx
	mov eax,edx
	mov cl,TAB
	xor ah,ah
	div cl
	neg ah				;ah = modulo division
	add ah,TAB			;TAB - pos % TAB
	pop ecx
	ret
;-------
;
; GetHelpText returns ecx==size of help text / esi==help text for current edit mode
;
GetHelpText:
	mov esi,help_ne			;start with last text block...
	mov ecx,help_ws_size
%ifdef USE_BUILTINHELP
	mov eax,mode
	cmp byte [eax],NE
	jnz NoNe1
	mov ecx,help_ne_size
	ret
NoNe1:	sub esi,ecx			;...and sub block by block until we've found it
	cmp byte [eax],VI
	jz GHT
	sub esi,ecx
	cmp byte [eax],EM
	jz GHT
	sub esi,ecx
	cmp byte [eax],PI
	jz GHT
	sub esi,ecx
%endif
GHT:	ret
;-------
;
; Check whether user discarded his input
;
CheckUserAbort:mov esi,mode
	cmp byte[esi],WS
	jz CUAWS
	cmp byte[esi],EM
	jz CUAEM
	cmp al,3			;^C abort
	ret
CUAWS:	cmp al,15h			;^U abort
	ret
CUAEM:	cmp al,7			;^G abort
	ret
;-------
KeyEditMode:
	mov esi,modetxt
	call InputStringWithMessage	;empty string is allowd for std options...
	call RestKursPos
	mov esi,optbuffer
	call InitSomeVars
;-------
SetEditMode:mov eax,mode		;returns Z flag if the mode was changed / NZ else
%ifndef ARMCPU
%ifdef AMD64
	mov ecx,dword [rsi]
%else
	mov ecx,dword [esi]
%endif
%else					;one never knows how aligned esi will be
	mov cl,byte [esi+3]
	shl ecx,8
	mov cl,byte [esi+2]
	shl ecx,8
	mov cl,byte [esi+1]
	shl ecx,8
	mov cl,byte [esi]
%endif
%ifdef W32
	or ecx,020202020h		;convert to lower case
%endif
	cmp ecx,'e3ne'
	jnz NoNe
	mov byte [eax],NE
	ret
NoNe:	cmp ecx,'e3em'
	jnz NoEm
	mov byte [eax],EM
	ret
NoEm:	cmp ecx,'e3pi'
	jnz NoPi
	mov byte [eax],PI
	ret
NoPi:	cmp ecx,'vi'
	jnz NoVi
	mov byte [eax],VI
	ret
NoVi:	cmp ecx,'e3ws'
	jnz modeOK
	mov byte [eax],WS
modeOK:	ret
;-----------------------------------------------------------------------
;
; Oleg's suggestion / Sat Mar 16 17:58:06
;
%ifdef USE_EXT_MOVE
KeyHome2:cmp byte[edi-1],NEWLINE	
	jz KCQPjmp
	jmp KeyHome
;-------
KeyEnd2:cmp byte[edi],NEWLINE	
	jz KCQPjmp
	jmp KeyEnd	
;-------
KeyCtrlQR2:cmp edi,sot
	jz KCQPjmp
	jmp KeyCtrlQR
;-------
KeyCtrlQC2:cmp edi,ebp
KCQPjmp:jz near KeyCtrlQP
	jmp KeyCtrlQC 
%endif
;-----------------------------------------------------------------------
%ifdef SYS_kill
SigHandler:call RestKursPos
	mov edi,screenbuffer		;make buffer invalid with something
	mov ecx,screenbuffer_dwords	;this will force a complete screen redraw
	cld
	rep stosd
%endif
;------- cont
SetTermStruc:
%ifdef W32
	push dword STD_INPUT_HANDLE
	call GetStdHandle
	mov [hin],eax
	push dword STD_OUTPUT_HANDLE
	call GetStdHandle
	mov [hout],eax
	push byte ENABLE_WINDOW_INPUT	;equ 8
	push dword [hin]
	call SetConsoleMode		;Do not use "jmp SetConsoleMode" here
	ret
%else
	mov ecx,TERMIOS_GET
	call IOctlTerminal0
	mov esi,edx
	mov edi,termios
	mov edx,edi
	push byte termios_struc_size	;prepare a copy of original settings
	pop ecx
	cld
	rep movsb
;-------
%ifdef LINUX
	mov byte [edx+termios_struc.c_cc+VMIN],1				;set min=1 ->needed for gnome-terminal
%endif
%ifdef ARMCPU
	and byte [edx+termios_struc.c_lflag+0],(~ICANON & ~ISIG & ~ECHO)	;icanon off, isig (^C) off, echo off
%else
	and TSize [edx+termios_struc.c_lflag+0],(~ICANON & ~ISIG & ~ECHO)	;icanon off, isig (^C) off, echo off
%endif
	and byte [edx+termios_struc.c_iflag+1],(~(IXON>>8) & ~(ICRNL>>8))	;ixon off,   icrnl off
	mov ecx,TERMIOS_SET
	jmp short IOctlTerminal		;edx is termios pointer
%endif
;----------------------------------------------------------------------
%ifdef NEW_CURSOR_MGNT
SetCursorNormal:PUSH_ALL
	mov ecx,normcurs
	push byte normcurslen
	jmp short SCB
SetCursorBlock:PUSH_ALL
	mov ecx,blockcurs		;second argument: pointer to message to write
	push byte blockcurslen		;third argument: message length
SCB:	pop edx
	call WriteFile0
	POP_ALL
	ret
%endif
;----------------------------------------------------------------------
;
; INTERFACE to OS kernel
; we differ between Linux, and and ...
;
%ifndef W32
IOctlTerminal0:mov edx,termios_orig
IOctlTerminal:mov ebx,stdin		;expects EDX termios or winsize structure ptr
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call ioctl
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_ioctl
	jmp short IntCall		;ECX TCSETS,TCGETS,TIOCGWINSZ
%endif
%endif
;------
ReadFile0:
%ifdef W32
	;all API: 
	;- direction flag is cleared before and after call
	;- preserves ebx,esi,edi,ebp
	PUSH_ALL
	push dword 0xFFFFFFFF
	push dword [hin]
	call WaitForSingleObject
	POP_ALL
%ifdef W32_EXTENDED_IO
	push ecx			;destr
	push edx			;destr
	push dword w32result
	push byte 1
	push ecx
	push dword [hin]
	call ReadConsoleInputA
	pop edx
	pop ecx
;-------
	mov ebx,dword [ecx]
	cmp bl,1			;is it a key_event?
	jnz ReadFile0			;no, read new
	mov ebx,dword [ecx+4]
	cmp ebx,1			;is it a keydown event?
	jnz ReadFile0			;no, read new
	;PUSH_ALL
	;push dword [hin]
	;call FlushConsoleInputBuffer
	;POP_ALL
;-------
	mov ebx,dword [ecx+8]		;virtual key code
	shr ebx,16
;-------
	cmp ebx,dword VK_SPACE
	jnz Normal0
	test dword [ecx+16],CTRL_PRESSED
	jz Normal0
	mov dword[ecx],0FF00h		;return ascii 00 for EMACS ^SPACE key
	ret
Normal0:cmp ebx,dword VK_DELETE
	ja Normal1
	cmp ebx,dword VK_PRIOR
	jb Normal1
;-------
	sub bl,VK_PRIOR			;found a cursor key
	mov al,bl
	mov ebx,ScanTable
	xlatb				;translate to terminal value...
	cmp al,DoNo			;...in results 0..9
ReadF0:	jz ReadFile0
	mov ah,0xFE			;marker for a pre-processed cursor key
	mov [ecx],eax
	ret
;-------
Normal1:mov ebx,dword [ecx+12]		;get ascii char value
	shr ebx,16
	or bl,bl
	jz ReadF0			;no useful ascii char pressed
	mov bh,0xFF
	and dword [ecx+16],LEFT_ALT_PRESSED	;controlkeystate: left ALT key pressed
	jz Normal2
	mov bh,0xFD			;marker for a pre-processed ALT key
Normal2:mov dword[ecx],ebx
	ret
%else					;this way simple input via ReadFile
	mov ebx,[hin]
%endif
%else
	xor ebx,ebx			;mov ebx,stdin		;file desc
%endif
Read_File:
%ifdef W32
	push ecx			;destr
	push edx			;destr
	push byte 0
	push dword w32result
	push edx			;length
	push ecx			;buffer
	push ebx			;handle
	call ReadFile
	pop edx
	pop ecx
;-------
	or eax,eax
	jnz ReadFileOkay
	call GetLastError
	mov [ErrNr],eax
	neg eax
	ret
ReadFileOkay:
	mov eax,[w32result]
	or eax,eax			;clear sign flag
	ret
%else
%ifdef BEOS
	push byte SYS_read		;4+X? stack places
	jmp short WFile
%else
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call read
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_read			;system call number (sys_read) ;return read byte EAX
	jmp short IntCall		;ebx file / ecx buffer / edx count byte
%endif
%endif
%endif
;-------
WriteFile00:xor edx,edx
	inc edx				;mov edx,1	write 1 byte
WriteFile0:
%ifdef W32
	mov ebx,[hout]
%else
	xor ebx,ebx			;mov ebx,stdout		;file desc
	inc ebx				;ditto
%endif
Write_File:
%ifdef W32
	push edx			;destr
	push byte 0
	push dword w32result
	push edx			
	push ecx			;buffer
	push ebx			;handle
	call WriteFile
	pop edx
	or eax,eax
	jnz WriteFileOkay
	call GetLastError
	mov [ErrNr],eax
	neg eax
	ret
WriteFileOkay:
	mov eax,[w32result]
	or eax,eax			;clr sign flag
	ret
%else
%ifdef BEOS
	push byte SYS_write
WFile:	pop eax
	call IntRdWr
	nop
	nop
	nop
	nop
	ret
%else
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call write
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_write
	jmp short IntCall
%endif
%endif
%endif
;-------
OpenFile0:
%ifndef BEOS
%ifdef W32
	mov ecx,OPEN_EXISTING
	mov edx,GENERIC_READ
%else
	xor ecx,ecx			;i.e O_RDONLY
%endif
%endif
OpenFile:
%ifdef W32
	push byte 0
	push dword FILE_ATTRIBUTE_NORMAL
	push ecx			;"CREATE_ALWAYS" or "OPEN_EXISTING"
	push byte 0
	push byte 0
	push edx			;"GENERIC_WRITE" or "GENERIC_READ"
	push ebx			;filename
	call CreateFileA
	cmp eax,INVALID_HANDLE_VALUE
	jnz OpenFileOkay
	call GetLastError
	mov [ErrNr],eax
	neg eax
OpenFileOkay:ret
%else
%ifdef BEOS
	mov al,SYS_open			;5 stack places
	push edi
	mov edi,0x1A4
	call IntCall
	pop edi
	ret
%else
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call open
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_open
	jmp short IntCall		;ecx mode / ebx path / edx permissions (if create)
%endif
%endif
%endif
;-------
CloseFile:
%ifdef W32
	push edx			;destr
	push ecx			;destr
	push ebx			;handle
	call CloseHandle
	pop ecx
	pop edx
	ret
%else
%ifdef LIBC
	push edx			;destr
	push ecx			;destr
	push ebx
	call close
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_close
	jmp short IntCall		;ebx is file desc
%endif
%endif
;-------
%ifdef SYS_readlink
ReadLink:mov al,SYS_readlink
	jmp short IntCall
%endif
;-------
%ifdef SYS_fchmod
Fchmod:	
%ifdef LIBC
	push ecx
	push ebx
	call fchmod
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	or eax,eax
	ret
%else
	mov al,SYS_fchmod
	jmp short IntCall
%endif
%endif
;-------
%ifdef SYS_fstat
Fstat:	mov ecx,fstatbuf
%ifdef LIBC
	push ecx
	push ebx
	call fstat
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	or eax,eax
	ret
%else
%ifdef FREEBSD				;includes NET-BSD
	mov ax,SYS_fstat
	jmp short IntCall2
%else
	mov al,SYS_fstat
	jmp short IntCall
%endif
%endif
;-------
ChownFile:
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call fchown
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_fchown
	jmp short IntCall
%endif
%endif					;endifdef SYS_fstat
;-------
RenameFile:
%ifdef W32
	push ebx			;destr
	push ecx			;destr
	push ecx			;for MoveFile
	push ecx
	call DeleteFileA
	push ebx
	call MoveFileA
	pop ecx
	pop ebx
	or eax,eax
	jnz RenameFileOkay
	call GetLastError
	mov [ErrNr],eax
	neg eax
	ret
RenameFileOkay:	
	xor eax,eax
	ret
%else
%ifdef LIBC
	push ecx
	push ebx
	call rename
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	or eax,eax
	ret
%else
	mov al,SYS_rename
	jmp short IntCall
%endif
%endif
;-------
%ifdef SYS_brk
SysBrk:	mov al,SYS_brk
	jmp short IntCall		;ebx addr
%endif
;-------
%ifndef W32
Exit:	xor ebx,ebx
Exit2:
%ifdef LIBC
	push ebx
	call _exit
%else
	mov al,SYS_exit
	jmp short IntCall
%endif
%endif
;-------
SeekFile:xor ecx,ecx			;ecx offset / ebx file / edx method
%ifdef W32
	push edx
	push byte 0
	push ecx
	push ebx
	call SetFilePointer
	cmp eax,0xFFFFFFFF
	jnz SeekFileOkay
	call GetLastError
	mov [ErrNr],eax
	neg eax
SeekFileOkay:ret
%else
%ifdef FREEBSD				;31 October 2005: 64 bit offset  initial for *BSD
	push edi			;ebx=fh/ecx=dummy/edx,esi=offset/edi=where
	push esi
	xor esi,esi
	xor edi,edi
	xchg edx,edi
	mov al,SYS_lseek
	call IntCall
	neg eax
	mov [ErrNr],eax
	neg eax				;set flags also
	pop esi
	pop edi
	ret
%else
%ifdef BEOS
	mov al,SYS_lseek		;4 stack places (using 64 bit for ptr)
	push edi
	push edx
	mov edi,edx
	xor edx,edx
	call IntCall
	pop edx
	pop edi
	ret
%else
%ifdef LIBC
	push edx
%ifdef OPENBSD
	push byte 0
%endif
	push ecx
	push ebx
	call lseek
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
%ifdef OPENBSD
	pop edx
%endif
	pop edx
	or eax,eax
	ret
%else
	mov al,SYS_lseek		;oldseek =32bit
%endif
%endif
%endif
%endif
;-------
%ifndef LIBC
%ifndef W32
IntCall:mov ah,0
IntCall2:cwde
%ifdef BEOS
	push edi
	push byte 0
	push edi
	push edx
	push ecx
	push ebx
	push dword be_ret
	int 25h
be_ret:	pop ebx
	pop ebx
	pop ecx
	pop edx
	pop edi
	pop edi
	mov [ErrNr],eax
	and dword [ErrNr],7Fh
	or eax,eax			;set flags also
	pop edi
%else
%ifdef ATHEOS
	int 80h
	cmp eax,0xFFFFF001
	jae Fru
	or eax,eax
	ret
Fru:	neg eax
	mov [ErrNr],eax
	and dword [ErrNr],7Fh
	neg eax				;set flags also
	ret
%else
%ifdef LINUX
%ifdef AMD64
	push rbx
	push rcx
	push rsi
	push rdi
	xchg rbx,rdi
	xchg rcx,rsi
	xchg rbx,r8
	mov r10,rcx			;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	syscall
	pop rdi
	pop rsi
	pop rcx
	pop rbx
%else
	int 80h
%endif
%else
	push edi
	push esi
	push edx
	push ecx
	push ebx
%ifdef NETBSD
	push dword nbsdint
%else
	push eax
%endif	
	int 80h
nbsdint:
	pop ebx
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	jc err
%endif
AfterInt:neg eax
err:	mov [ErrNr],eax
	neg eax				;set flags also
%endif
%endif
	ret
%endif
%endif
;-------
%ifdef BEOS
IntRdWr:push edx			;used for Read & Write
	push ecx
	push ebx
	push dword be_ret2
	int 25h
be_ret2:pop ebx
	pop ebx
	pop ecx
	pop edx
	mov [ErrNr],eax
	pop eax
	lea eax,[eax+4]			;add eax,4
	push eax
	mov eax,[ErrNr]
	and dword [ErrNr],7Fh
	or eax,eax			;set flags
	ret
%endif
;--------------------------------------------------------------------------
%ifdef SYS_select
%ifdef LIBC
Select:	push dword timevalsec
	xor ebx,ebx
	push ebx
	push ebx
	mov ecx,readfds
	push ecx
	inc ebx
	mov byte [ecx],bl
	push ebx
	call select
	push ebx
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	add esp,20
	or eax,eax
	ret
%else
Select:	xor ebx,ebx
	inc ebx
	mov ecx,readfds
	mov byte [ecx],bl
	xor edx,edx
	xor esi,esi
	mov edi,timevalsec		;points to [0 sec / 0 usec]
	mov al,SYS_select
	jmp short IntCall
%endif
%endif
;-----------------------------------------------------------------------
%ifdef SYS_readlink
Utime:	mov al,SYS_utime
	jmp short IntCall
%endif
;-------
%ifdef USE_PIPE
Fork:	mov al,SYS_fork
	jmp short IntCall
;-------
Pipe:	
%ifdef FREEBSD
	mov eax,SYS_pipe
	push edi
	push esi
	push ecx
	push ebx
	int 80h
	pop ebx
	pop ecx
	pop esi
	pop edi
	mov [ebx],eax
	mov [ebx+4],edx
	jmp short AfterInt
%else
	mov al,SYS_pipe
	jmp short IntCall
%endif
;-------
Dup2:	mov al,SYS_dup2
	jmp short IntCall
;-------
Execve:	mov al,SYS_execve
	jmp short IntCall
;-------
Wait4:	mov al,SYS_wait4		;set ecx to buffer!
%ifdef AMD64
	xor rbx,rbx
	dec rbx	
%else
	xor ebx,ebx
	dec ebx
%endif
	xor edx,edx
	xor esi,esi
	jmp short IntCall
;-------
%endif
Unlink:
%ifdef W32
	push edx
	push ecx
	push ebx
	call DeleteFileA
	pop ebx
	pop ecx
	pop edx
	ret
%else
%ifdef LIBC
	push edx
	push ecx
	push ebx
	call unlink
	mov ebx,[_errno]
	mov [ErrNr],ebx
	pop ebx
	pop ecx
	pop edx
	or eax,eax
	ret
%else
%ifdef BEOS
	mov ecx,ebx
%endif
	mov al,SYS_unlink
	jmp short IntCall		;ebx is file desc
%endif
%endif
;----------------------------------------------------------------------
%ifdef SYS_kill
KeySuspend:call KursorStatusLine	;simply looks better so
	push byte SIGSTOP
	pop ecx
	xor ebx,ebx
Kill:	mov al,SYS_kill
ICjmp:	jmp short IntCall
;-------
SetSigHandler:
	push byte SIGCONT
	pop ebx
	mov ecx,sigaction
%ifdef AMD64
	mov rdx,qword SigHandler
	mov qword [rcx],rdx
	mov qword [rcx+8],4000000h	;flags @ 8
	mov rdx,qword SigRestorer
	mov qword [rcx+16],rdx		;restorer @ 16
	xor edx,edx
	mov esi,8
	mov al,SYS_rt_sigaction
	jmp short ICjmp
;-------
SigRestorer:mov eax,SYS_rt_sigreturn
	syscall				;thanks to Andi Kleen for info
%else
	mov [ecx],dword SigHandler
	xor edx,edx
%ifdef SIGREST32			;just like on AMD-64 would be:
	mov dword [ecx+8],4000000h
	mov [ecx+12],dword SigRestorer32
%endif
Sigaction:mov al,SYS_sigaction
	jmp short ICjmp
;-------
%ifdef SIGREST32
SigRestorer32:pop eax
	mov eax,119			;sigreturn
	int 80h				;syscall
%endif
%endif
%endif
;-----------------------------------------------------------------------
;
; pipe buffer through child process
;
%ifdef USE_PIPE
KeyCtrlKP:call InputStringWithMessage0	;WS, Emacs's, Pico's access to sed|ex pipe
	pushf
	call RestKursPos
	popf
	jc ExExEx
KeyVICmdtemp:call CheckENum		;i.e. mov byte [numeriere],1
	PUSH_ALL
	mov ebx,tempfile
	mov esi,sot
	mov [blockende],ebp
	call SaveBl3
	POP_ALL
	jnc SaveOK
ExExEx:	ret				;cannot save buffer to tmp file...
SaveOK:					;...this is handled inside SaveBlock, so just return
%ifdef USE_UNDO
	xor eax,eax
	dec eax
	call DataForUndoDelete
%endif
%ifdef CAPTURE_STDERR
	mov ebx,sedpipeC0
	call Pipe
	js OSerrJmp0
%endif
	mov ebx,sedpipeB0
	call Pipe
	js OSerrJmp0
	call Fork
	js OSerrJmp0
	or eax,eax
	jz near ForkChild
;-------
%ifdef USE_EX_NO_SED
;
; This is the UNDEFAULT way using "ex -s" (silent ex).
; We save the buffer into a tempfile and
; WRITE the COMMAND (followed by 'wq' command)
; into a pipe to ex's STDIN.
; Then we truncate the current edit buffer
; and read the tempfile just like a WS block.
;
; "Anything you want, you got it"
; "Anything you need, you got it"
; "Anything at all, you got it, Baby ...." (Jeff Lynne/Roy Orbison/Tom Petty 1989)
;
	mov ebx,[ebx]			;i.e. sedpipeB0
	call CloseFile			;unused pipe direction READ
OSerrJmp0:js OSerrJmp1
	mov ebx,[sedpipeB1]
;-------
	xor edx,edx
	mov ecx,optbuffer
	mov esi,ecx
	cld
	dec edx
Bufloop:inc edx
	lodsb
	or al,al
	jnz Bufloop			;count ex cmd length
	call Write_File			;write to ex program
;-------
	mov ecx,wq_suffix
	mov edx,wq_suffix_len
	call Write_File			;write "wq" string to ex program
	call CloseFile
OSerrJmp1:js OSerrJmp
	mov ecx,optbuffer		;for return status
	push ecx			;new in e3 2.6.1 (needed in Linux 2.6.8-24.10)
	push edi
	call Wait4
	pop edi
	pop ecx
	js OSerrJmp
	mov ecx,[ecx]			;getting return status
	shr ecx,8
	and ecx,0xFF
	mov [ErrNr],ecx
	jnz OSerrJmp
;-------
	mov ebx,tempfile
	push ebp
	push edi
	mov edi,sot			;truncate old file
	mov ebp,sot
	mov al,LINEFEED
	xchg byte [ebp],al		;EOF marker
	push eax
	call ReadBlock2
	pop eax
	pop edi
	jc preEx_Ex
	pop ebx				;dummy (not restore ebp)
;-------
	mov ebx,tempfile
	call Unlink
	jns ChkCursPos			;if no Error
;-------
OSerrJmp:call ChkCursPos		;Error in Wait,Close,...
	jmp near OSerror		;TODO: unlink helper file if exists
preEx_Ex:pop ebp			;Error in ReadBlock
	mov byte[sot],al		;restore pre op values if Read Error
	;continue ChkCursPos
;
%else	;------------------ DEFAULT WAY -------------------
;
; This is the DEFAULT way using stream editor "sed -e".
; The default way is quite opposite: we save the buffer
; into a tempfile, then call sed with the operational command
; given on sed's command line and at last we READ the DATA
; from sed's output via a STDIN READ pipe.
;
	push ebx
	mov ebx,[ebx+4]			;i.e. sedpipeB1
	call CloseFile			;unused pipe direction
%ifdef CAPTURE_STDERR
	mov ebx,[sedpipeC1]
	call CloseFile			;unused pipe direction
%endif
OSerrJmp0:js OSerrJmp1
	pop ebx
	mov esi,[maxlen]
	mov ebx,[ebx]			;pipe read channel
	mov ecx,sot-SEDBLOCK
	add esi,ecx
	push ebp
	mov ebp,sot
	xor eax,eax
	call InitSV1			;forbid any side efects
ReadPipe:mov edx,SEDBLOCK
	add ecx,edx
	call Read_File
	add ebp,eax			;compute eof
	cmp ebp,esi
	jae ReadOK			;sorry, truncating. FIXME: add a message
	cmp eax,edx
	jz ReadPipe
	cmp ebp,sot			;if nothing comes back, keep buffer as is
	jnz ReadOK
	pop edx
	mov ebp,edx
	push esi			;keep stack balanced
ReadOK:	mov byte [ebp],NEWLINE		;EOF marker
	pop esi				;dummy
	call CloseFile
OSerrJmp1:js OSerrJmp
%ifdef CAPTURE_STDERR
	mov ebx,[sedpipeC0]
%ifdef BEEP_IN_VI
	mov ecx,buffercopy
	xor edx,edx
	inc edx
	call Read_File
	or eax,eax
	je NoStdErrMsg
	call VIBeepForD
%endif
NoStdErrMsg:call CloseFile		;stderr pipe
%endif
	call SetChg			;i.e. mov byte [changed],CHANGED  (assumption only)
	mov ecx,optbuffer		;for return status
	push ecx			;new in e3 2.6.1 (needed in Linux 2.6.8-24.10)
	push edi
	call Wait4
	pop edi
	pop ecx				;ditto new in e3 2.6.1
	js OSerrJmp
	mov ebx,tempfile
	call Unlink
	js OSerrJmp
	movzx ecx,byte[ecx+1]		;old shr ecx,8 / and ecx,0xFF
	mov [ErrNr],ecx
	jecxz ChkCursPos
OSerrJmp:call ChkCursPos		;TODO: unlink helper file if exists
	jmp near OSerror
%endif
;----------------------------------------------------------------------
ForkChild:mov ebx,[ebx]			;i.e. sedpipeB0
	xor ecx,ecx
%ifdef USE_EX_NO_SED
	call Dup2			;capturing STDIN
	js FCError
	mov ebx,[sedpipeB1]
	call CloseFile			;unused pipe direction STDOUT
	js FCError
	mov ebx,expath
	mov ecx,exargs
%else
	call CloseFile			;unused pipe direction STDIN
	js FCError
	mov ebx,[sedpipeB1]
	inc ecx
	call Dup2			;capturing STDOUT
	js FCError
%ifdef CAPTURE_STDERR
	mov ebx,[sedpipeC0]
	call CloseFile			;unused pipe direction
	inc ecx
	mov ebx,[sedpipeC1]
	call Dup2			;capturing STDERR
	js FCError
%endif
	mov ebx,sedpath
	mov ecx,sedargs
%endif
	xor edx,edx			;no env
	call Execve
	push byte ERRNOEXEC
	pop ebx				;set error
FCex:	jmp near Exit2			;in case of error
FCError:mov ebx,[ErrNr]
	jmp short FCex
%endif ;USE_PIPE
;----------------------------------------------------------------------
;
; care about cursor pos
;
ChkCursPos:
%ifdef UTF8
	inc edi
CCloopUTF8:dec edi
%ifdef UTF8RTS
	cmp byte [isUTF8],0
	je noUTF_Z
%endif
	mov al,byte [edi]
	and al,0C0h
	cmp al,080h
	je CCloopUTF8
noUTF_Z:
%endif
	cmp edi,ebp			;never let run cursor outside buffer
	jbe CCP
	mov edi,ebp
CCP:	cmp edi,sot
	jae CCP2
	mov edi,sot
CCP2:	ret
;----------------------------------------------------------------------
;
; copy file to a real backup file (for sym linked files only)
;
; expecting ebx==filepath
;	    ecx==bakpath
;
%ifdef SYS_readlink
CopyToBackup:PUSH_ALL
	push ecx			;backup file path later needed for Utime call
	push ecx
	call OpenFile0
	xchg esi,eax			;save handle to copy of original file
	mov ecx,O_WRONLY_CREAT_TRUNC
	mov edx,[perms]
	pop ebx				;handle to backupfile
	call OpenFile
	xchg ebx,eax
	mov ecx,[fstatbuf+stat_struc.st_uid]
%ifdef UIDGID_WORD                      ;Linux special
	mov edx,ecx
	shr edx,16
	movzx ecx,cx			;OLD and ecx,0xffff
%else
	mov edx,[fstatbuf+stat_struc.st_gid]
%endif
	call ChownFile
;-------
	xor edi,edi			;init eof indicator
copylop:push ebx
	mov ebx,esi			;saved orig file handle
	mov ecx,screenbuffer		;used as copy buffer
	mov edx,4096
	call Read_File
	pop ebx				;backup file handle
	;js...
	cmp eax,edx
	jz notready
	inc edi				;eof found
notready:mov ecx,screenbuffer
	mov edx,eax			;write read count of byte
	call Write_File
	;js...
	or edi,edi			;eof ?
	jz copylop
;-------
	call CloseFile			;ready: close backup file
	mov ebx,esi
	call CloseFile			;close original file
	pop ebx				;original file path
	mov ecx,accesstime		;i.e. a data structure of 2* 32 bit
	call Utime			;set change time
	POP_ALL
	ret
%endif
;----------------------------------------------------------------------
; recursive descent parser for very SIMPLE math calc within the text:
;              1234+56*78=
; place cursor ^<--here and press   ^KN  ^QC  ^XN  #   ^K
;			(for one of WS   PI   EM   VI  NE modes)
; 		this should insert the result of 5602 into text.
;
; Use values +-0, 0.000001 ,... up to 999999999999.999999 
; and + - * / 
; and ( )
; and r  (for using the last result in next calculation)
; and p  =3.141593
;
KeyCtrlKN:
%ifdef USE_MATH
	cld				;preserve ebp,edi!
	fninit
	mov esi,edi
	mov [stackptr],esp
	xor eax,eax
	mov [level],eax
	mov [ptlevel],eax
	call Recurs0
	fnstsw [x87]
	and byte[x87],1Fh		;any exception flags?
GErr:	jnz near isErr
	dec dword[level]		;stack balanced?
	jnz GErr
	xor eax,eax
	cmp eax,[ptlevel]		;all parenthesis closed?
	jnz GErr
NoAllgFehl:dec esi
	mov edi,esi			;up to here we have read
	lodsb
	cmp al,'='
	jz EquChar
	mov al,'='
	call OutChar
	dec edi
EquChar:inc edi
	fst qword[lastresult87]		;carry last result for further calc
	fld qword[factor]
	fmulp st1
	fbstp [x87]
	push byte 12			;12 digits
	pop ecx				
	xor dh,dh			;flag for suppressing leading 0
	lea esi,[x87+9]			;9 BCD data byte and sign
	std
	lodsb
	or al,al
	jns plus
	cmp al,0x80
	jnz GErr
	mov al,'-'
	call OutChar
plus:	call OutHlp
	or dh,dh
	jnz dec_dig
	mov al,'0'
	call OutChar
;-------
dec_dig:mov esi,x87
	xor ebx,ebx
	push byte 3
	pop ecx
	cld
dlop:	lodsb
	mov dl,al
	and al,0fh
	jnz nonull
	inc ebx
	mov al,dl
	shr al,4
	jnz nonull
	inc ebx
isnul2:	loop dlop
	ret				;no decimal digits: ready
;-------
nonull:	push byte 6			;6 decimal digits
	pop ecx
	sub ecx,ebx
	mov dh,0xff			;now do not suppress 0
	mov al,'.'
	call OutChar
	lea esi,[x87+2]			;decimal digits pos
OutHlp:	std
	lodsb
	mov dl,al
	shr al,4
	call OutNumber
	dec ecx
	jecxz xret
	mov al,dl
	and al,0fh
	call OutNumber
	loop OutHlp
xret:	ret
;-------
Recurs0:mov al,'+'
RecursPars:push eax			;op code
	mov byte[signctl],0		;last token was an opcode
RecConti:lodsb
	cmp al,LINEFEED			;EOL?
	jz short RRR			;jz RecReturn
	cmp al,')'
	jz short RRet2
	cmp al,'='			;end of task?
RRR:	jz short RecReturn
	cmp al,'!'			;white space?
	jb short RecConti
	cmp byte[signctl],0		;last was opcode?
	jnz short CheckNP		;sign is allowed after opcode only
	cmp al,'+'
	jz short rPlus
	cmp al,'-'
	jnz short CheckNP
	inc byte[signctl]		;2 for minus 
rPlus:	inc byte[signctl]		;1 for plus
	jmp short RecConti		;continue

CheckNP:push dword [signctl]		;we need that for numbers and parenthesis
	cmp al,'('
	jnz short CheckNum
	inc dword[ptlevel]		;increase nesting level
	call Recurs0			;compute term instead of parse number
	jmp short fromP
CheckNum:cmp al,'0'
	jb short noNumber
	cmp al,'r'			;last Result
	jz short isLastRes
	cmp al,'p'			;pi 3.141593
	jz short isPi
	cmp al,'9'
	ja short isErr2
	call Number
fromP:	pop eax				;signctl on stack
	cmp al,2
	jnz short isPlus
	FCHS				;parenthesis or number is negative
isPlus:	mov byte[signctl],1		;last token was a number (or parenthesis)
	mov al,[esp]			;our opcode
	cmp al,'+'
	jz short RecConti
	cmp al,'-'
	jnz short RecReturn
	mov byte[esp],'+'		;adding negative value
	FCHS
RecCon2:jmp short RecConti
;-------
RRet2:	dec dword[ptlevel]
RecReturn:pop eax
	ret
;-------
isLastRes:fld qword[lastresult87]
	jmp short isPi2
isPi:	fldpi
isPi2:	inc dword[level]
	jmp short fromP
isErr2:	jmp short isErr
;-------
noNumber:pop ecx			;due above "push dword [signctl]" (we don't need it here)
	lea esp,[esp-16]
	fstp dword [esp]
	call RecursPars
	fld dword [esp]
	lea esp,[esp+16]
	dec dword[level]
	cmp al,'*'
	jnz noMul
	FMULP st1
	jmp short RecCon2
noMul:	cmp al,'/'
	jnz noDiv
	FDIVRP st1
	jmp short RecCon2
noDiv:	cmp al,'+'
	jnz isErr			;not one of * / +
	FADDP st1
	jmp short RecReturn
;-------
Number:	FLDZ
	FBSTP [x87]			;init buffer
	push byte 12			;read 13 chars  (up to 12 digits)
	pop ecx
num_ctr:call DigitHlp
	jc int_end
	loop num_ctr			;error if >12 digits
isErr:	
	mov esp,[stackptr]		;restore stack pos for math calc
%endif ;USE_MATH
%ifdef BEEP_IN_VI
VIBeepForD:PUSH_ALL
%ifdef W32
	push byte 0
	call MessageBeep
%else
	mov ecx,BeepChar
	call WriteFile00
%endif
	POP_ALL
%endif
	ret
;-------
%ifdef USE_MATH
int_end:push edi			;****
	neg ecx
	lea ecx,[ecx+13]
	push ecx			;stor # of integer digits
	inc dword[level]
	dec esi				;num_ctr loop has read 1 too much
	mov edx,esi			;position we have read so far
	cmp al,('.'-'0')
	jnz integers
	inc esi
	lea edi,[x87+2]			;start of decimal places
	mov cl,3			;6/2 decimal places
dec_ctr:cld
	call DigitHlp
	jc decend
	shl al,4
	mov bl,al
	mov [edi],al			;important if abort at digit 1,3,5
	call DigitHlp
	jc decend	
	add al,bl
	std
	stosb
	loop dec_ctr			;if >6 decimal places it will run into error later
	inc esi
decend:	dec esi
	xchg edx,esi			;edx where later to continue scanning
;-------
integers:pop ecx			;ecx # of integer digits
	dec esi				;esi where integer places are
	lea edi,[x87+3]			;start of integer part
intloop:std
	lodsb
	sub al,'0'
	mov bl,al
	dec ecx
	jecxz h2
	lodsb
	sub al,'0'
	shl al,4
	add al,bl
h2:	cld
	stosb
	jecxz h3
	loop intloop
h3:	FBLD [x87]
	FLD qword [factor]
	fdivp st1
	mov esi,edx			;pointer for continued reading
	pop edi				;****
	ret
;-------
DigitHlp:lodsb
	sub al,'0'
	jb dret
	cmp al,10
	cmc
dret:	ret				;return: al=value / cy if error
;-------
OutNumber:cmp al,dh			;flag set?
	jz OCret
	add al,'0'
	mov dh,0xff			;set flag
OutChar:push esi
	push edx
	push ecx
	call NormChar
	pop ecx
	pop edx
	pop esi
	inc ebx
%endif
OCret:	ret
;----------------------------------------------------------------------
%ifdef USE_UNDO
%define ROLLBACK
%undef ROLLBACK
; Undo is organized in frames on a ringbuffer stack
;
; FRAME_AAAAprevFRAME_BBBBprevCURRENT_EMPTY_FRAME
; ^         v   ^          v  v
; |         |   |          |  |
;  \-------/     \--------/|  |
;                          |  v
;                          v  [undoptr] == next free frame
;                          [undoptr]-4  == begin of previous frame
;
; there are 3 types of frames: DELETE,INSERT,OVERWRITE, see details below:
;
DataForUndoDelete:
;
; Data collector for "delete" by PUSHING undo data into a frame on the undo stack,
; growing to higher addresses, using a variable size (16+X byte) data structure:
; 
; |12345679|--WHERE-|--SIZE--|<data>....X       |PREV-PTR|--NEXT--| ......
; |  =sign |  =edi  |  =eax  |                  |        |        |
; |        |        |        |                  |        |        |
; | edx+0  | edx+4  | edx+8  | edx+12           |edx+16+X|edx+20+X| <----- ADDRESSES
;                                                         ^^^^^^^^
;                                                         NEXT_undo_frame_address stored in [undoptr]
; If the data size is > undobuffer size we have to 
; save the data otherwise, i.e. in a file:
; |1234567B|--WHERE-|--SIZE--|<PREV-PTR|--NEXT--| ......
; |  =sign |  =edi  |  =eax  |         |        |
; |        |        |        |         |        |
; | edx+0  | edx+4  | edx+8  | edx+12  |edx+16  | <----- ADDRESSES
;
	cmp byte [enter_undo],1		;do not collect undo data if within undo operation
	jz OCret
	cmp eax,undobuffer_size-24
	PUSH_ALL
	jb DFok
;-------
	PUSH_ALL
	mov ebx,[last_undo_file]
	or ebx,ebx
	jz noundo_info
	cmp dword [ebx],0x01234567B
	jnz noundo_info
	mov dword [ebx],0		;only ONE external undo info allowed, thus destroy older
noundo_info:mov esi,sot			;i.e. huge undo data
	mov [blockende],ebp
	mov ebx,tempfile2
	call SaveBl3
	POP_ALL
;-------
	call InitUndoFrame0
	mov dword [edx],0x01234567B	;2nd signature for "delete"
	mov [last_undo_file],edx
	mov eax,ebp
	sub eax,sot			;ebp -sot == size of buffer
	jmp short OVWdata
;-------	
DFok:					;i.e. small undo data
	mov ecx,eax			;extra data size / size of copy
	call InitUndoFrame
	mov dword [edx],0x012345679	;signature for "delete", later undo will insert data again
	mov [edx+4],edi			;where
	mov [edx+8],eax			;how much
	mov esi,edi			;source is inside editor text buffer
	lea edi,[edx+12]		;destination of copy
	cld
	rep movsb
	mov eax,[undoptr]		;this frame...
	mov [edi],eax			;... is the prev frame for the next one
	lea eax,[edi+4]			;eax: now the new frame address
	jmp short DFex
;-------
DataForUndoOverwrite:
;
; Data collector for "overwrite" by PUSHING data on a stack, 
; growing to higher addresses, using a data structure like in DataForUndoInsert
;
	PUSH_ALL
	call InitUndoFrame0
	mov dword [edx],0x01234567A	;signature for "overwrite", later undo will restore
	mov eax,[edi]			;fetch overwritten char
	jmp short OVWdata
DataForUndoXchange:
;
; Data collector for "Emacs ^T" by PUSHING data on a stack, 
; growing to higher addresses, using a data structure like in DataForUndoOverWrite
;
	PUSH_ALL
	call InitUndoFrame0
	mov dword [edx],0x012345677	;signature for "xchg", later undo will restore
	jmp short OVWdata
;------
DataForUndoInsert:
;
; Data collector for "insert" by PUSHING data on a stack,
; growing to higher addresses, using a fixed size (16 byte) data structure:
; 
; |12345678|--WHERE-|--SIZE--|PREV-PTR|--NEXT--| .........
; |  =sign |  =edi  |  =eax  |        |        |     
; |        |        |        |        |        |
; | edx+0  | edx+4  | edx+8  | edx+12 |edx+16  |edx+20  <----- ADDRESSES
;                                      ^^^^^^^^
;                                      NEXT_undo_frame_address stored in [undoptr]
;
; An analogue data structure is used for DataForUndoOverwrite:
; |1234567A|--WHERE-|--CHAR--|PREV-PTR|--NEXT--| .........
;
;
	cmp byte [enter_undo],1		;do not collect undo data if within undo operation
	jz DFUI
	PUSH_ALL
	call InitUndoFrame0
	mov dword [edx],0x012345678	;signature for "insert", later undo will delete that data
OVWdata:mov [edx+4],edi			;where
	mov [edx+8],eax			;how much chars (or the character itself)
	mov [edx+12],edx		;this frame is the prev frame for the next one
	lea eax,[edx+16]		;address of next frame
DFex:	mov [undoptr],eax		;let undoptr point to next frame 
	POP_ALL
DFUI:	ret
;----------------------------------------------------------------------
;
; this subroutine is bound to one of the keys like ^KU and
; will POP any UNDO data from the undo stack using 3 types of undo frames
;
KeyUndo:mov byte [enter_undo],1		;do not log dele/insert when in undo mode
	mov ebx,edi			;for case of error
	mov edx,[undoptr]
	mov edx,[edx-4]			;get begin of previos frame
	or edx,edx
	jz NotAv			;no date available
	xor ecx,ecx			;read signature into ecx and destroy sign
	xchg ecx,[edx]			;(destroying is neccessary because it's a ring buffer)
	mov eax,[edx+8]			;data size or character itself
	mov edi,[edx+4]			;position
	sub ecx,0x12345677
	jz UndoOfXchange
	dec ecx
	jz UndoOfInsert
	dec ecx
	jz UndoOfDelete
	dec ecx
	jz UndoOfOverwrite
	dec ecx
	jz ReReadBuffer
NotAv:	mov edi,ebx			;abort UNDO: no valid signature found
%ifdef ROLLBACK
	xor edx,edx
%endif
	jmp short KUret
;-------
UndoOfXchange:mov al,byte [edi]
	xchg byte [edi-1],al
	mov byte [edi],al
	jmp short KUexit
;-------
UndoOfDelete:lea esi,[edx+12]		;source ptr for deleted <data>
	push esi
	call InsertByte			;get some space.....
	pop esi				;source ptr (somewhere inside UNDO frame)
	call MoveBlock			;....and move <data> back into text
	jmp short KUexit
;-------
UndoOfOverwrite:mov byte [edi],al
	jmp short KUexit
;-------
UndoOfInsert:call DeleteByte
	jmp short KUexit
;-------
ReReadBuffer:lea ebp,[eax+sot]		;compute eof pointer
	mov byte [ebp],NEWLINE		;eof-marker
	push eax			;size
	mov ebx,tempfile2
	call OpenFile0
	pop edx				;size
	js KUexit
	xchg ebx,eax			;file handle
	mov ecx,sot
	call Read_File
	js KUexit
	call CloseFile
	mov ebx,tempfile2
	call Unlink
;-------
KUexit:	mov edx,[undoptr]		;switch to undo frame before (i.e. POP)
	mov edx,[edx-4]			;the prev frame....
	mov [undoptr],edx		;...is now current frame
KUret:	mov byte [enter_undo],0		;leave UNDO status
KUjmp:	jmp CheckENum			;renumbering because we have changed the cursor position
;-------
%ifdef ROLLBACK
RollBack:call KeyUndo
	or edx,edx
	jnz RollBack
	jmp short KUjmp	
%endif
;----------------------------------------------------------------------
;
; This inits the frame data pointer into edx.
; If there is not enough space we will wrap around to buffer begin and adjust [undoptr]:
;
;BEFORE WRAP:
;|******any_frame**********any_frame********any_frame*******PREV-PTR---------------| buffer_end
;                                                                   <---too less--->
;                                                                   ^^^^^^^^^
;                                                                   [undoptr]
;NOW AFTER WRAP:
;|PREV-PTR<-space_for_new_frame->ame********any_frame*******PREV-PTR---------------| buffer_end
;         ^^^^^^^^               ^^^^^^^^^^ ^^^^^^^^^         
;        [undoptr]               invalid     last ok frame
;                               frame part
;
InitUndoFrame0:xor ecx,ecx		;no extra data
InitUndoFrame:mov edx,[undoptr]		;get current frame
	mov esi,undobuffer_end-24
	sub esi,ecx			;extra data if exist
	cmp edx,esi			;low memory?
	jb IUFret			;leave if far away from buffer end 
;-------
	mov ebx,[edx-4]			;fetch PREV-PTR frame address @[undoptr-4]
	mov edx,undobuffer		;wrap around: now BACK AT BUFFER BEGIN...
	mov [edx],ebx			;store prev data frame pointer just before new frame
	lea edx,[edx+4]			;=new frame begins here at undobuffer+4
	mov [undoptr],edx
IUFret:	ret
%endif
%ifdef UTF8RTS
KeyUTF8switch:not byte [isUTF8]
	ret
%endif
;----------------------------------------------------------------------
%ifdef LINUX
%ifndef CRIPLED_ELF
section .data
bits 32
%endif
%endif
;
; CONSTANT DATA AREA
;
%ifdef USE_MATH
factor		dq 1000000.0
%endif
tempfile2 	db 'e3##',0		;tempfile (FIXME: use PID for name)
%ifdef USE_PIPE
tempfile 	db 'e3$$',0		;tempfile (FIXME: use PID for name)
%ifdef USE_EX_NO_SED
%ifndef AMD64
exargs		dd expath
		dd minus_s
		dd tempfile
		dd 0
%else
exargs		dq expath
		dq minus_s
		dq tempfile
		dq 0
%endif
expath		db EX_PATH,0
minus_s		db '-s',0
wq_suffix	db LINEFEED,'wq',LINEFEED
wq_suffix_len	equ $-wq_suffix
%else
%ifndef AMD64
sedargs		dd sedpath		;this way default
		dd minus_e
		dd optbuffer
		dd tempfile
		dd 0
%else
sedargs		dq sedpath		;this way default
		dq minus_e
		dq optbuffer
		dq tempfile
		dq 0
%endif
sedpath		db SEDPATH,0
%ifndef PERLPIPE
minus_e		db '-e',0
%else
minus_e		db '-pe',0
%endif
;
%endif
%endif
;
optiontext	db 'OPT? C/B',0
filename	db 'FILENAME:',0
block		db '   NAME:',0
saveas		db 'SAVE AS:',0
filesave	db '   SAVE:',0
asksave		db 'SAVE? Ynl',0
asksave2	db 'SAVE? Yn',0
askreplace1	db 'REPLACE:',0
askreplace2	db 'RE WITH:',0
asklineno	db 'GO LINE:',0
askfind		db ' SEARCH:',0
asknumber	db '^Q OCTAL:',0
extext		db 'mode EX:',0
modetxt		db 'SET MODE',0
%define DoNo 10

ScanTable: 	;another xlat table containing offsets in jumptab1 table
%ifdef W32
	db 2	;VK_PRIOR  = Scan 21h (pgup)
	db 7	;VK_NEXT   = Scan 22h (pgdn)
	db 5	;(end)
	db 0	;(home)
	db 3	;(left)
	db 1	;(up)
	db 4	;(right)
	db 6	;(dn)
	db DoNo	;29h ignored 
	db DoNo	;2ah ditto
	db DoNo	;2bh ditto
	db DoNo	;2ch ditto
	db 8	;VK_INSERT = Scan 2dh (insert)
	db 9	;VK_DELETE = Scan 2eh (del)
%else
	db DoNo	;		esc[0~
	db 0	;keyHome	esc[1~
	db 8	;keyIns		esc[2~
	db 9	;keyDel		esc[3~
	db 5	;keyEnd		esc[4~
	db 2	;keyPgUp	esc[5~
	db 7	;KeyPDn		esc[6~
	db 0	;keyHome	esc[7~
	db 5	;keyEnd		esc[8~
		;---------------------
%ifdef QNX
	db 8	;keyIns		esc[@
%endif
	db 1	;keyUp		esc[A
	db 6	;keyDown	esc[B
	db 4	;keyRight	esc[C
	db 3	;keyLeft	esc[D
	db DoNo	;		esc[E
	db 5	;keyEnd		esc[F
	db 7	;keyPgDn	esc[G
	db 0	;keyHome	esc[H
%ifndef LINUX
	db 2	;keyPUp		esc[I
	db DoNo	;		esc[J
	db DoNo	;		esc[K
	db 8	;keyIns		esc[L
%endif
%ifdef QNX
	db DoNo	;		esc[M
	db DoNo	;		esc[M
	db DoNo	;		esc[O
	db 9	;		esc[P
	db DoNo	;		esc[Q
	db DoNo	;		esc[R
	db DoNo	;		esc[S
	db DoNo	;		esc[T
	db 7	;		esc[U
	db 2	;		esc[V
	db DoNo	;		esc[W
	db DoNo	;		esc[X
	db 5	;		esc[Y
%endif
STsize equ ($-ScanTable)
%endif
;----------------------------------------------------------------------
EmaAltTable: 	;another xlat table containing offsets in jumptab1 table
	db 12h	;'B'
	db DoNo	;'C'
	db DoNo	;'D'
	db DoNo	;'E'
	db 13h	;'F'
	db 11h	;'G'
	db 3Dh	;'H'	Help!
	db DoNo	;'I'
	db DoNo	;'J'
	db DoNo	;'K'
	db DoNo	;'L'
	db DoNo	;'M'
	db DoNo	;'N'
	db DoNo	;'O'
	db DoNo	;'P'
	db DoNo	;'Q'
	db DoNo	;'R'
	db DoNo	;'S'
	db DoNo	;'T'
	db DoNo	;'U'
	db 2 	;'V'
	db 27h	;'W'
	db 3Eh	;'X'
ATsize equ ($-EmaAltTable)
;----------------------------------------------------------------------
%define Beep 0x4E
VIcmdTable:db Beep;0
	db Beep	;1
	db 2	;^B PageUp
	db Beep	;3
	db 36h	;^D half PageUp
	db Beep	;5
	db 7	;^F PageDn
	db Beep	;7
	db 3	;^H KeyLeft
	db Beep	;9
	db 6	;^J KeyDown
	db Beep	;11
	db Beep	;12
	db 6	;^M KeyDown
	db Beep	;^N
	db Beep	;^O
	db Beep	;^P
	db Beep	;^Q
	db Beep	;^R
	db Beep	;^S
	db Beep	;^T
	db 37h 	;^U
	db Beep	;22
	db Beep	;23
	db Beep	;24
	db Beep	;25
	db 51h	;^Z
	db Beep	;27
	db Beep	;28
	db Beep	;29
	db Beep	;30
	db Beep	;31
	db 4	;' ' KeyRight
	db Beep	;33
	db Beep	;34
	db 4fh 	;35  Numerics
	db 5	;'$' KeyEnd
	db Beep	;37
	db Beep	;38
	db 50h	; '
	db Beep	;40
	db Beep	;41
	db Beep	;42
	db 6	;'+' KeyDown
	db Beep	;44
	db 1	;'-' KeyUp
	db Beep	;46
	db 39h	;'/' Search
	db 0	;'0' KeyHome
	db 44h	;'1' 1G BOF
	db Beep	;'2'
	db Beep	;'3'
	db Beep	;'4'
	db Beep	;'5'
	db Beep	;'6'
	db Beep	;'7'
	db Beep	;'8'
	db Beep	;'9'
	db 2Ch	;':' ex mode
	db 3Eh	;';' e3 special command: QUICK leave vi mode :-)  press  e3ws, e3em, e3pi, e3ne
	db Beep	;'<'
	db Beep	;'='
	db Beep	;'>'
	db 3Ah	;'?' search backw
	db Beep	;'@'
	db 2Eh	;'A'
	db 12h	;'B' left word
	db 4Bh	;'C' Change rest of line 
	db 4Ch	;'D' Delete rest of line (not unlike ^QY in WStar)
	db Beep	;'E'
	db Beep	;'F'
	db 0Fh	;'G' EOF
	db 30h	;'H' First LIne
	db 33h	;'I' switch to insert mode
	db 4Dh	;'J' Join lines
	db Beep	;'K'
	db 31h	;'L' Last Line
	db 52h	;'M'
	db Beep	;'N'
	db 32h	;'O' Open Line
	db 3Ch	;'P' Paste
	db Beep	;'Q'
	db 34h	;'R' overwrite
	db 40h	;'S' kill +insmode
	db Beep	;'T'
	db Beep	;'U'
	db Beep	;'V'
	db 13h	;'W' next word
	db 3fh	;'X' del left
	db Beep	;'Y'
	db 41h	;'Z'
	db Beep	;'['
	db Beep	;'\'
	db Beep	;']'
	db 38h	;'^' KeyFirstChar
	db Beep	;'_'
	db Beep	;'`'
	db 2Dh	;'a' append (KeyLeft plus insert mode)
	db 12h	;'b' left word
	db Beep	;'c'
	db 35h	;'d' delete
	db 43h	;'e'
	db Beep	;'f'
	db Beep	;'g'
	db 3	;'h' KeyLeft
	db 2Bh	;'i' switch to insert mode
	db 6	;'j' KeyDown
	db 1	;'k' KeyUp
	db 4	;'l' KeyRight
	db 45h	;'m' set the one and only marker 'a'
	db Beep	;'n'
	db 2Fh	;'o' Open Line
	db 3Bh	;'p' Paste
	db Beep	;'q'
	db 4Ah	;'r' repl one char
	db Beep	;'s'
	db Beep	;'t'
%ifdef USE_UNDO
	db 53h	;'u'	UNDO
%else
	db Beep	;'u'
%endif
	db Beep	;'v'
	db 13h	;'w' next word
	db 9	;'x' KeyDel
	db 46h	;'y'
	db 42h	;'z' center line z.
VIsize equ ($-VIcmdTable)
;----------------------------------------------------------------------
Ktable	db DoNo	;^K@	xlatb table for making pseudo-scancode
	db DoNo	;^ka
	db 24h	;^kb	24h for example points to KeyCtrlKB function offset
	db 15h	;^kc
	db 0dh	;^kd
	db DoNo	;^ke	DoNo means SimpleRet i.e. 'do nothing'
	db DoNo	;^kf
	db DoNo	;^kg
	db 1dh	;^kh
	db DoNo	;^ki
	db DoNo	;^kj
	db 14h	;^kk
	db DoNo	;^kl
	db 3eh	;^km	Set Mode
	db 4Fh	;^kn
	db DoNo	;^ko
	db 48h 	;^kp    Pipe thru sed
	db 0bh	;^kq
	db 10h	;^kr
	db 0ch	;^ks
	db DoNo	;^kt
%ifdef UTF8RTS
	db 54h	;^ku	^KU UTF8 view switcher
%else
	db DoNo	;^ku
%endif
	db 23h	;^kv
	db 25h	;^kw
	db 16h	;^kx
	db 19h	;^ky
	db 51h	;^kz	^KZ suspend (like in joe editor)
Ktable_size equ $-Ktable
Qtable	db DoNo	;^q@	ditto for ^Q menu
	db 1ah	;^qa
	db 20h	;^qb
	db 0fh	;^qc
	db 05h	;^qd
	db 1eh	;^qe
	db 1bh	;^qf
	db DoNo	;^qg
	db 17h	;^qh, ^qDEL
	db 11h	;^qi
	db DoNo	;^qj
	db 21h	;^qk
	db DoNo	;^ql
	db DoNo	;^qm
	db DoNo	;^qn
	db DoNo	;^qo
	db 18h	;^qp
	db DoNo	;^qq
	db 0eh	;^qr
	db 00h	;^qs
	db DoNo	;^qt
	db DoNo	;^qu
	db 1ch	;^qv
	db 12h	;^qw
	db 1fh	;^qx
	db 22h	;^qy
	db 13h	;^qz
;----------------------------------------------------------------------
Xtable	db DoNo	;^x^@
	db DoNo	;^x^a
	db DoNo	;^x^b
	db 0bh	;^x^c	WS: ^KQ
	db DoNo	;^x^d
	db DoNo	;^x^e
	db 47h	;^x^f
	db DoNo	;^x^g
	db 3Dh	;^x^h   		i.e.  "HELP!" (The Beatles, 1965)  ;-)
	db 10h	;^x i	WS: ^KR
	db DoNo	;^x^j
	db DoNo	;^x^k
	db DoNo	;^x^l
	db DoNo	;^x^m
	db 4fh	;^x^n   numerics
	db DoNo	;^x^o
	db 48h	;^x^p   special sed pipe
	db DoNo	;^x^q
	db DoNo	;^x^r
	db 0ch	;^x^s	WS: ^KS
 	db DoNo	;^x^t
	db DoNo	;^x^u
	db DoNo	;^x^v
	db 29h	;^x^w	write to
	db 26h	;^x^x	xchg mark/point
	db DoNo	;^x^y
	db DoNo	;^x^z
;----------------------------------------------------------------------
PicoJtable db DoNo ;^j@	Junk ops for PI mode
	db DoNo	;^ja
	db DoNo	;^jb
	db DoNo	;^jc
	db DoNo	;^jd
	db DoNo	;^je
	db DoNo	;^jf
	db DoNo	;^jg
	db 17h 	;^jh Junk to line Home
	db DoNo	;^ji
	db DoNo	;^jj
	db DoNo	;^jk
	db 22h	;^jl Junk Line rest
	db DoNo	;^jm
	db DoNo	;^jn
	db DoNo	;^jo
	db 48h	;^jp  special sed pipe
	db DoNo	;^jq
	db DoNo	;^jr
	db DoNo	;^js
	db 49h	;^jt repeat last search&replace
	db DoNo	;^ju
	db DoNo	;^jv
	db 2Ah	;^jw Junk Word
	db DoNo	;^jx
	db DoNo	;^jy
	db DoNo	;^jz
PicoQtable db DoNo ;^q@	Quick motions for PI mode: wordstar counterparts on different keys
	db DoNo	;^qa
	db 1Fh	;^qb	Bottom of window
	db 4Fh	;^qc	Calc numerics
	db DoNo	;^qd
	db 0fh	;^qe	End of file
	db 1Ch	;^qf	last Find
	db DoNo	;^qg
	db DoNo	;^qh
	db DoNo	;^qi
	db DoNo	;^qj
	db DoNo	;^qk
	db 11h	;^ql	Line number #
	db 3eh	;^qm	set mode
	db 13h	;^qn	Next word
	db DoNo	;^qo
	db 12h	;^qp	Previous word
	db DoNo	;^qq
	db DoNo	;^qr
	db 0Eh	;^qs	Start of file
	db 1Eh	;^qt	Top of window
%ifdef USE_UNDO
	db 53h	;^qu	UNDO
%else
	db DoNo	;^qu
%endif
%ifdef UTF8RTS
	dw 54h	;^qv	UTF8 switcher
%else
	db DoNo	;^qv
%endif
	db DoNo	;^qw
	db DoNo	;^qx
	db DoNo	;^qy
	db DoNo	;^qz
;----------------------------------------------------------------------
esize equ 2	;(byte per entry)
%ifdef ARMCPU
align 2
%endif

jumptab1:	;Storing 16 bit offsets is valid only for code less size 64 kbyte...
		;  ... but in assembler that should never be a problem   ;)
%ifndef USE_EXT_MOVE
	dw KeyHome	-_start	;0
%else
	dw KeyHome2	-_start	;0
%endif
	dw KeyUp	-_start	;1
	dw KeyPgUp	-_start	;2
	dw KeyLeft	-_start	;3
	dw KeyRight	-_start	;4
%ifndef USE_EXT_MOVE
	dw KeyEnd	-_start	;5
%else
	dw KeyEnd2	-_start	;5
%endif
	dw KeyDown	-_start	;6
	dw KeyPgDn	-_start	;7
	dw KeyIns	-_start	;8
	dw KeyDel	-_start	;9	0..9 are Cursor pad keys
;------------------------
	dw SimpleRet	-_start	;10 	DO_NOTHING == DoNo
	dw KeyCtrlKQ	-_start	;0bh EMA ^X^C
	dw KeyCtrlKS	-_start	;0ch EMA ^X^S
	dw KeyCtrlKD	-_start	;0dh EMA ^X^F
%ifndef USE_EXT_MOVE
	dw KeyCtrlQR	-_start	;0eh EMA Alt<
	dw KeyCtrlQC	-_start	;0fh EMA Alt>
%else
	dw KeyCtrlQR2	-_start	;0eh EMA Alt<
	dw KeyCtrlQC2	-_start	;0fh EMA Alt>
%endif
	dw KeyCtrlKR	-_start	;10h EMA ^XI
	dw KeyCtrlQI	-_start	;11h EMA Alt-G
	dw KeyCtrlQW	-_start	;12h EMA Alt-B
	dw KeyCtrlQZ	-_start	;13h EMA Alt-F
;--------------------------
;up to here this functions are considered common for all (exc vi)
;Of course some use different key names.
;
;now follows special stuff for each editor emulation:
;-------WS and Pico--------
	dw KeyCtrlKK	-_start	;14h
	dw KeyCtrlKC	-_start	;15h
	dw KeyCtrlKX	-_start	;16h
	dw KeyCtrlQDel	-_start	;17h
	dw KeyCtrlQP	-_start	;18h
	dw KeyCtrlKY	-_start	;19h
	dw KeyCtrlQA	-_start	;1ah
	dw KeyCtrlQF	-_start	;1bh
	dw KeyCtrlQV	-_start	;1ch
	dw KeyCtrlKH	-_start	;1dh
	dw KeyCtrlQE	-_start	;1eh
	dw KeyCtrlQX	-_start	;1fh
	dw KeyCtrlQB	-_start	;20h
	dw KeyCtrlQK	-_start	;21h
	dw KeyCtrlQY	-_start	;22h
	dw KeyCtrlKV	-_start	;23h
	dw KeyCtrlKB	-_start	;24h
	dw KeyCtrlKW	-_start	;25h
;-------EM--------
	dw KeyCtrlXX	-_start	;26h
	dw KeyEmaAltW	-_start	;27h
	dw KeyEmaAltPer	-_start ;28h Alt-%
	dw KeyEmaCtrlXW	-_start	;29h
;-------PI--------
	dw KeyCtrlT	-_start	;2Ah
;-------VI--------
	dw KeyVIcmdi	-_start	;2Bh
	dw KeyVIex	-_start	;2Ch
	dw KeyVIcmda	-_start ;2Dh
	dw KeyVICmdA	-_start	;2Eh
	dw KeyVICmdo	-_start	;2Fh
	dw KeyCtrlQE	-_start	;30h
	dw KeyCtrlQX	-_start	;31h
	dw KeyVICmdO	-_start	;32h
	dw KeyVICmdI	-_start	;33h
	dw KeyVICmdR	-_start ;34h
	dw KeyVICmdd	-_start ;35h
	dw KeyHalfPgDn	-_start ;36h
	dw KeyHalfPgUp	-_start ;37h
	dw KeyVI1Char	-_start ;38h
	dw KeyVIfsearch	-_start ;39h
	dw KeyVIbsearch	-_start ;3Ah
	dw KeyVICmdp	-_start ;3Bh
	dw KeyVICmdP	-_start ;3Ch
;------- later added (mostly vi stuff) ------
	dw KeyHelp	-_start ;3Dh	general
	dw KeyEditMode	-_start ;3Eh	general
	dw KeyDell	-_start ;3Fh	vi
	dw KeyVICmdS	-_start ;40h	vi	
	dw KeyVICmdZ	-_start ;41h	vi
	dw KeyVICmdz	-_start ;42h	vi
	dw KeyVIcmde	-_start ;43h	vi
	dw KeyVIcmd1	-_start ;44h	vi
	dw KeyVICmdm	-_start ;45h	vi
	dw KeyVICmdy	-_start ;46h	vi
	dw KeyEmaCtrlXF	-_start ;47h	emacs (extended ^KD from WS)
%ifdef USE_PIPE	
	dw KeyCtrlKP	-_start ;48h    use sed-pipe in WS,Emacs,Pico
%else
	dw SimpleRet	-_start ;48h
%endif
	dw KeyPiCtrlJT	-_start ;49h	Pico
	dw KeyVICmdr	-_start ;4Ah	vi
	dw KeyVICmdC	-_start ;4Bh	vi
	dw KeyVICmdD	-_start ;4Ch	vi
	dw KeyVICmdJ	-_start ;4Dh	vi
%ifdef BEEP_IN_VI
	dw VIBeepForD	-_start ;4Eh	vi
%else
	dw SimpleRet	-_start ;4Eh	vi
%endif
	dw KeyCtrlKN	-_start ;4Fh	general
	dw KeyVICmdJmpM -_start ;50h	vi
%ifdef SYS_kill
	dw KeySuspend	-_start ;51h	general
%else
	dw SimpleRet	-_start	;51h
%endif
	dw KeyVI_M	-_start ;52h	vi
%ifdef USE_UNDO
	dw KeyUndo	-_start ;53h	general
%else
	dw SimpleRet	-_start	;53h
%endif
%ifdef UTF8RTS
	dw KeyUTF8switch-_start ;54h	general
%else
	dw SimpleRet	-_start	;54h
%endif
jumps1 equ ($-jumptab1) / esize
;--- 32 more for WS--------
	dw SimpleRet	-_start	;^Space
	dw KeyCtrlQW	-_start	;^a
	dw SimpleRet	-_start	;^b  ;; TEST dw KeyUndo-_start
	dw KeyPgDn	-_start	;^c
	dw KeyRight	-_start ;^d
	dw KeyUp	-_start	;^e
	dw KeyCtrlQZ	-_start	;^f
	dw KeyDel	-_start	;^g 7
	dw KeyDell	-_start	;^h 8   DEL (7fh is translated)
	dw NormChar	-_start	;^i 9	(TAB)
%ifdef SELFTEST
	dw KeyRet	-_start	;^j 0ah
%else
	dw KeyHelp	-_start	;^j
%endif
	dw CtrlKMenu	-_start	;^k b
	dw KeyCtrlL	-_start	;^l c
	dw KeyRet	-_start	;^m 0dh
	dw SimpleRet	-_start	;^n e
	dw SimpleRet	-_start	;^o f
	dw KeyHelp	-_start	;^p 10	Help!
	dw CtrlQMenu	-_start	;^q 11
	dw KeyPgUp	-_start	;^r 12
	dw KeyLeft	-_start	;^s 13
	dw KeyCtrlT	-_start	;^t 14
%ifdef USE_UNDO
	dw KeyUndo	-_start	;^u 15 (abort in Input)
%else
	dw SimpleRet	-_start	;^u
%endif	
	dw KeyIns	-_start	;^v 16
	dw KeyScrollUp	-_start	;^w 17
	dw KeyDown	-_start	;^x 18
	dw KeyCtrlY	-_start	;^y 19
	dw KeyScrollDn	-_start ;1a
	dw SimpleRet	-_start ;1b
	dw SimpleRet	-_start	;1c
	dw SimpleRet	-_start	;1d
	dw SimpleRet	-_start	;1e
%ifdef ROLLBACK	
	dw RollBack	-_start	;1f  for internal testing of UNDO ring buffer only
%else
	dw SimpleRet	-_start	;1f
%endif
;--- 32 more for EM--------
	dw KeyEmaMark	-_start	;^Space
	dw KeyHome	-_start	;^a
	dw KeyLeft	-_start	;^b
	dw SimpleRet	-_start	;^c (not planned)
	dw KeyDel	-_start ;^d
	dw KeyEnd	-_start	;^e
	dw KeyRight	-_start	;^f
	dw SimpleRet	-_start	;^g (abort in Input)
	dw KeyDell	-_start	;^h
	dw NormChar	-_start	;^i (TAB)
	dw KeyRet	-_start	;^j
	dw KeyEmaCtrlK	-_start	;^k
	dw KeyEmaCtrlL	-_start	;^l
	dw KeyRetNoInd	-_start	;^m 0dh
	dw KeyDown	-_start	;^n
	dw KeyEmaCtrlO	-_start	;^o
	dw KeyUp	-_start	;^p
	dw KeyEmaCtrlQ	-_start	;^q
	dw KeyEmaCtrlR	-_start	;^r
	dw KeyEmaCtrlS	-_start	;^s
	dw KeyEmaCtrlT	-_start	;^t
%ifdef UTF8RTS
	dw KeyUTF8switch-_start	;^u UTF8 mode switcher 
	; (ATTENZIONE: that is NOT emacs stuff like: 'C-u runs the command universal-argument')
%else
	dw SimpleRet	-_start	;^u
%endif
	dw KeyPgDn	-_start	;^v
	dw KeyEmaCtrlW	-_start	;^w
	dw CtrlXMenu	-_start	;^x
	dw KeyEmaCtrlY	-_start	;^y
%ifdef SYS_kill
	dw KeySuspend	-_start ;^z
%else
	dw SimpleRet	-_start	;51h
%endif
	dw SimpleRet	-_start ;1b
	dw SimpleRet	-_start	;1c
	dw SimpleRet	-_start	;1d
	dw SimpleRet	-_start	;1e
%ifdef USE_UNDO
	dw KeyUndo	-_start ;1f
%else
	dw SimpleRet	-_start	;1f
%endif
;--- 32 more for PI------
	dw KeyEmaMark	-_start	;^Space	a redundant marker because ^^ is ugly on some kbds
	dw KeyHome	-_start	;^a 1
	dw KeyLeft	-_start	;^b 2
	dw SimpleRet	-_start	;^c 3
	dw KeyDel	-_start ;^d 4
	dw KeyEnd	-_start	;^e 5
	dw KeyRight	-_start	;^f 6
	dw KeyHelp	-_start	;^g 7
	dw KeyDell	-_start	;^h 8   DEL
	dw NormChar	-_start	;^i 9	(TAB)
	dw PicoJMenu	-_start	;^j a
	dw KeyEmaCtrlW	-_start	;^k b
	dw KeyEmaMark	-_start	;^l c	a redundant marker because ^^ is ugly on some kbds
	dw KeyRet	-_start	;^m d
	dw KeyDown	-_start	;^n e
	dw KeyCtrlKS	-_start	;^o f	SAVE
	dw KeyUp	-_start	;^p 10
	dw PicoQMenu	-_start	;^q 11
	dw KeyCtrlKR	-_start	;^r 12
	dw KeyEmaCtrlXW	-_start	;^s 13	SAVE_AS
	dw PicoCtrlTpico-_start	;^t 14
	dw KeyEmaCtrlY	-_start	;^u 15
	dw KeyPgDn	-_start	;^v 16
	dw KeyEmaCtrlS	-_start	;^w 17
	dw KeyCtrlKQ	-_start	;^x 18
	dw KeyPgUp	-_start	;^y 19
%ifdef SYS_kill	
	dw KeySuspend	-_start ;^z 1A  (not in pico)
%else
	dw SimpleRet	-_start	;^z
%endif
	dw SimpleRet	-_start	;^[ 1B
	dw SimpleRet	-_start	;^\ 1C
	dw SimpleRet	-_start	;^] 1D
	dw KeyEmaMark	-_start	;^^ 1E	see ^L
	dw SimpleRet	-_start ;1F
;--- 32 more for NE------
	dw KeyEmaMark	-_start	;^Space toggle selection mode (no shift cursor keys available!)
	dw KeyNedCtrlA	-_start	;^a 1   Mark all
	dw KeyIns	-_start	;^b 2   toggle Ins mode
	dw KeyEmaAltW	-_start	;^c 3	COPY
	dw SimpleRet	-_start ;^d 4
	dw KeyEditMode	-_start	;^e 5   set EDit mode
	dw KeyCtrlQF	-_start	;^f 6   find
	dw KeyCtrlL	-_start	;^g 7   find again
	dw KeyDell	-_start	;^h 8   DEL
	dw NormChar	-_start	;^i 9	TAB
	dw KeyRet	-_start	;^j a   RETURN
	dw KeyCtrlKN	-_start	;^k b   numerics
	dw KeyCtrlQI	-_start	;^l c   LINE #
	dw KeyRet	-_start	;^m d   RETURN
	dw KeyEmaCtrlXF	-_start	;^n e	OPEN another
	dw KeyEmaCtrlXF	-_start	;^o f   OPEN another
	dw SimpleRet	-_start	;^p 10
	dw KeyCtrlKQ	-_start	;^q 11  EXIT
	dw KeyCtrlQA	-_start	;^r 12  REPLACE
	dw KeyCtrlKS	-_start	;^s 13	SAVE
	dw SimpleRet	-_start	;^t 14
%ifdef USE_UNDO
	dw KeyUndo	-_start ;^u 15	UNDO
%else
	dw SimpleRet	-_start	;^u 15
%endif
	dw KeyEmaCtrlY	-_start	;^v 16	PASTE
	dw KeyEmaCtrlXW	-_start	;^w 13	SAVE_AS/WRITE TO
	dw KeyEmaCtrlW	-_start	;^x 18	CUT
%ifdef UTF8RTS
	dw KeyUTF8switch-_start	;^y 19
%else
	dw SimpleRet	-_start	;^y 19
%endif
%ifdef SYS_kill
	dw KeySuspend	-_start ;^z 1A
%else
	dw SimpleRet	-_start	;^z
%endif	
	dw SimpleRet	-_start	;^[ 1B
	dw SimpleRet	-_start	;^\ 1C
	dw SimpleRet	-_start	;^] 1D
	dw SimpleRet	-_start	;^^ 1E
	dw SimpleRet	-_start ;1F
;----------------------------------------------------------------------
;
%ifdef W32
 scolorslen	equ 0
%else
 BeepChar	db 7
 screencolors0	db 27,'[40m',27,'[37m'
 bold0		db 27,'[0m'		;reset to b/w
 screencolors1	db 27,'[44m',27,'[33m'	;yellow on blue
 reversevideoX:
 bold1:		db 27,'[1m'		;bold
 scolorslen	equ $-screencolors1
 boldlen	equ $-bold1		;take care length of bold0 == length of bold1
%ifdef LINUX
		db 27,'[7m'		;good for "linux" terminal on /dev/tty (but not xterm,kvt)
					;again take care length = length of boldX
					;!! important: store directly after bold1 !!
%ifdef NEW_CURSOR_MGNT
 blockcurs	db 27,'[?17;0;64c'	;see e3.h
 blockcurslen	equ $-blockcurs
 normcurs	db 27,'[?2c'
 normcurslen	equ $-normcurs
%endif
%endif
%endif
%ifdef SELFTEST
 pipein		db 'PIPE_IN',0
%endif
;-------
%ifdef UTF8
%ifdef UTF8RTS
 getPos		db 13, 0c3h, 0B6h,27,'[6n',13	;write carriage_return, followed by 1 UTF8 char
 						;german umlaut oe = ö in UTF-8
 gPlen		equ $-getPos			;and clean up with another carriage_return
						;Terminal should answer ESC[<line>;<column>R
%endif						;on UTF8 terminals should be column==2 else ==3
%endif
;-------------------------------------------------------------------------
%ifdef ARMCPU
align 2
%endif
editmode:db 'p WSp Pip Emp NE'
;
helptext:
db "MicroEditor e3 v2.7.1"
%ifndef NASM
db "Y"
%endif
%ifdef UTF8
db "-UTF8 ",0C2h,0A9h
%else
db " (C)"
%endif
db "2000-07 A.Kleine",10
db "Enter filename or leave with RETURN",10,10
%ifdef YASM
%ifdef UTF8
helptextsize equ 54h
%else
helptextsize equ 50h
%endif
%else
helptextsize equ $-helptext
%if helptextsize>127
 %error helptextsize
%endif
%endif
helpfoot:db 10,10,10,TABCHAR,TABCHAR,TABCHAR,"-= PRESS ANY KEY =-" ;at least 6 wasted byte ;-)
%ifdef YASM
helpfootsize equ 19h
%else
helpfootsize equ $-helpfoot
%if helpfootsize>127
 %error helpfootsize
%endif
%endif
;
%ifdef USE_BUILTINHELP
help_ws:
db "Key bindings in WS mode:",10,10
db "Files:	^KR Insert	^KS Save	^KX Save&Exit	^KQ Abort&Exit",10
%ifndef USE_PIPE
db "	^KD Save&Load",10
%else
%ifdef USE_EX_NO_SED
db "	^KD Save&Load	^KP Pipe buffer thru 'ex' ",10
%else
db "	^KD Save&Load	^KP Pipe buffer thru 'sed'",10
%endif
%endif
db 10
db "Blocks:	^KB Start	^KK End		^KC Copy	^KY Del",10
db "	^KV Move	^KW Write",10
db 10
db "Search:	^QF Find	^L  Repeat	^QA Srch&Repl",10
db "Move:	^E  Up		^X  Down	^S  Left	^D  Right",10
db "	^R  Page Up	^C  Page Dn	^W  Scroll Up	^Z  Scroll Dn",10
db "Quick-	^QE Wnd Top	^QX Wnd Bott	^QS Home	^QD End",10
db "-Move:	^QR BOF		^QC EOF		^QB Blk Begin	^QK Blk End",10
db "	^F  Next Word	^A  Prev Word	^QI Line#	^QV Last Find",10
db 10
db "Delete:	^T  Word	^Y  Line	^H  Left	^G  Chr",10
db "	^QY Line End	^QDel,^QH Line Beg",10
%ifdef USE_MATH
db "Other:	^KM Set mode	^KN Numerics"
%else
db "Other:	^KM Set mode"
%endif
%ifdef SYS_kill
db "	^KZ Suspend "
%endif
%ifdef USE_UNDO
db "	^U  Undo"
%endif
%ifdef UTF8RTS
db 10,"	^KU UTF8"
%endif
help_ws_size equ $-help_ws
;-------------------------
help_pi:
db "Key bindings in PICO mode:",10,10
db "Files:	^XN ExitNoSave	^XY Exit+Save	^XL Save+Load New File",10
db "	^O  Save	^S  Save as	^R  Read",10
db 10
db "Move:	^P  Up		^N  Down	^B  Left	^F  Right",10
db "	^Y  Page up	^V  Page down	^QN Next word	^QP Previous word",10
db "	^A  Home	^E  End		^QS Start	^QE EOF",10
db "	^QT Top screen	^QB Bottom scr	^QL Line #	^QF last Find",10
db 10
db "Search:	^W  Where is	^T  Search&Repl	^JT Repeat Search & Replace",10
db 10
db "Delete:	^H  Left char	^D  This char	^K  Kill line/region",10
db "	^JW Word	^JL Line end	^JH Line begin",10
db 10
db "Other:	^U  Unkill	^G  Help	^^,^L,^<SPC> Mark region",10
%ifndef USE_PIPE
db "	^QM Set Edit Mode ",10
%else
%ifdef USE_EX_NO_SED
db "	^QM Set Edit Mode		^JP Pipe buffer thru 'ex' ",10
%else
db "	^QM Set Edit Mode		^JP Pipe buffer thru 'sed'",10
%endif
%endif
%ifdef USE_MATH
db "	^QC Calculate"
%else
db " "
%endif
%ifdef SYS_kill
db "			^Z Suspend"
%endif
%ifdef USE_UNDO
db "	^QU Undo"
%endif
%ifdef UTF8RTS
db 10,"	^QV UTF8"
%endif
help_pi_size equ $-help_pi
;-------------------------
help_em:
db "Key bindings in EMACS mode:",10,10
db "Files:	^X^C Exit	^XI  Insert	^X^S Save	^X^F Load New",10
%ifndef USE_PIPE
db "	^X^W Write new	^X^H Help ",10
%else
%ifdef USE_EX_NO_SED
db "	^X^W Write new	^X^H Help	^X^P Pipe buffer thru 'ex' ",10
%else
db "	^X^W Write new	^X^H Help	^X^P Pipe buffer thru 'sed'",10
%endif
%endif
db 10
db "Move:	^P   Up		^N  Down	^B   Left	^F   Right",10
db "	altV Pg up	^V  Pg down	altB Left word	altF Right word",10
db "	^A   Home	^E  End		alt< BOF	alt> EOF",10
db "	altG Go line#	^L  Center Pos",10
db 10
db "Search:	^S Find fwd	^R Find bwd	alt% Search&Replace like WS",10
db 10
db "Buffer:	altW Copy	^Y Yank		^<SPC> Mark	^X^X Xchg Mark/Pt",10
db 10
db "Delete:	^K Line		^W Region	^H Left	Chr	^D This Chr",10
db 10
%ifdef UTF8
; FIXME: ^T
db "Other:	^O Open line	           	^I Ins Tab	^Q Quoted Ins",10
%else
db "Other:	^O Open line	^T Xchg Chr	^I Ins Tab	^Q Quoted Ins",10
%endif
db "	^M NL		^J NL+indent	altX Set edit mode",10
%ifdef USE_MATH
db "	^X^N Calculate"
%else
db "		"
%endif
%ifdef SYS_kill
db "			^Z Suspend"
%endif
%ifdef USE_UNDO
db "	^_  Undo"
%endif
%ifdef UTF8RTS
db 10,"	^U UTF-8"
%endif
help_em_size equ $-help_em
;-------------------------
help_vi:
db "Key bindings in vi mode:",10
db 10
db "<ESC>			enter cmd mode",10
db "h,j,k,l,+,-,<Ret>,<SPC>	move by chars&lines",10
db "^B,^F,^D,^U		move by [half]page",10
db "$,0,^,w,b,e,H,L,M,z.	move in line/screen",10
db "/,?,G			srch fwd/bwd, go EOF",10
db "ma,'a			set/go marker a",10
db "x,X,<Del>,dw,D		dele chr,word,EOL",10
db "S,C,dd,d'a,yy,y'a	subst,change,dele,yank",10
db "p,P			paste",10
db "A,a,I,i,<Ins>,O,o	enter ins.mode",10
db "R,r			enter ovw.mode,ovw.chr",10
db "J			join lines",10
%ifdef USE_UNDO
 %ifdef SYS_kill
  db "ZZ,^Z, u		save&quit,suspend, undo!",10
 %else
  db "ZZ, u		save&ex, undo!",10
 %endif
%else
 %ifdef SYS_kill
  db "ZZ,^Z			save&quit,suspend",10
 %else
  db "ZZ			save&ex",10
 %endif
%endif
%ifdef USE_MATH
db ";,#			E3 SPECIAL: set edit mode,calculate",10
%else
db ";			E3 SPECIAL:set edit mode",10
%endif
db ":w,:wq,:x,:q,:q!,:e	ex mode:save,quit,save_as,edit other",10
db ":0,:$,:<line#>		ex mode:go BOF,EOF,line",10
%ifdef UTF8RTS
db ":h,:u			ex mode:help, UTF-8",10
%else
db ":h			ex mode:help",10
%endif
%ifndef USE_PIPE
db "         "
%else
%ifdef USE_EX_NO_SED
db ":<other cmd>		pipe buffer thru 'ex' "
%else
db ":<other cmd>		pipe buffer thru 'sed'"
%endif
%endif
help_vi_size equ $-help_vi
;-------------------------
help_ne:
db "Key bindings in NEDIT mode:",10
db 10
db "Files:		^QN Exit_NoSave	^QY Exit&Save	^QL Save&Load new",10
db "		^S  Save	^W  WriteTo=SaveAs",10
db "Move:		^L  Line#",10
db "		^F  Find	^R Search&Replace (like WS)",10
db "		^G  Go repeat last ^F,^R",10
db 10
db "Select:		^<SPACE> begin&extend by cursor keys (like Emacs)",10
db "		^A  All buffer",10
db "		^X  Cut		^C Copy 	^V Paste",10
db 10
db "Other:		^E  Set edit mode",10
%ifdef UTF8RTS
db "		^Y  UTF8 view",10
%endif
%ifdef USE_MATH
db "		^K  Calculate",10
%endif
db "		altH Help"
%ifdef SYS_kill
db "	^Z Suspend"
%endif
%ifdef USE_UNDO
db "	^U Undo"
%endif
help_ne_size equ $-help_ne
%ifndef YASM
%if help_ws_size != help_pi_size || help_ws_size!= help_em_size || help_ws_size!= help_pi_size || help_ws_size!= help_vi_size
%error Helptext
dw help_vi_size
dw help_ws_size
dw help_pi_size
dw help_em_size
%endif
%endif
%else					;no help texts built in
help_ws:
help_pi:
help_em:
help_vi:
help_ne:
db "This e3 is built w/o help texts."
help_ws_size equ $-help_ws
help_ne_size equ $-help_ws
%endif
;
errmsgs:errortext 			;see e3.h

;-------
;
%ifdef CRIPLED_ELF
 filesize      equ     $ - $$
%endif
;-----------------------------------------------------------------------
%ifdef ATHEOS
section .data				;unused in Linux/FreeBSD/BeOS: save byte in ELF header
bits 32					;unused in W32: save byte in PE header
%endif
;-----------------------------------------------------------------------
%ifdef NETBSD				;added Sun Oct  9 10:10:00 CEST 2005 
section .note.netbsd.ident
align 4
 dd 7
 dd 4
 dd 1
 db "NetBSD",0,0
 dd 0 
%endif 
;-----------------------------------------------------------------------
%ifdef OPENBSD
section .note.openbsd.ident
align 4
 dd 8
 dd 4
 dd 1
 db "OpenBSD",0
 dd 0 
%endif 
;-----------------------------------------------------------------------
section .bss
bits 32
align 4
%ifdef CRIPLED_ELF
 bssstart:
%endif
screenbuffer_size equ 62*(160+32)	;estimated 62 lines 160 columns, 32 byte ESC seq (ca.12k)
screenbuffer_dwords equ screenbuffer_size/4
screenbuffer resb screenbuffer_size
screenbuffer_end equ $			;If you really have higher screen resolution,
					;...no problem, except some useless redrawing happens.
%ifdef W32
 attribbuffer	resw 62*160		;estimated 62 lines 160 columns
 attribbuffer_end equ $
%else
 termios:	resb termios_struc_size
 termios_orig:	resb termios_struc_size
 winsize:	resb winsize_struc_size
 setkplen	equ 10
 setkp		resb setkplen		;to store cursor ESC seq like  db 27,'[000;000H'
 		resb 2			;fill up
 revvoff	resd 1
%endif
%ifdef USE_UNDO
 enter_undo	resd 1			;a status byte: 1 while in a undo process, else 0
 last_undo_file resd 1			;a pointer to undo info stored external in a disk file
 undoptr	resd 1			;points on top frame in undo ringbuffer stack
 undobuffer_size equ 0x10000		;64 k
 undobuffer	resb undobuffer_size
 undobuffer_end equ $
%endif
lines		resd 1			;equ 24 or similar i.e. screen lines-1 (1 for statusline)
columns		resd 1			;equ 80 or similar dword (using only LSB)
columne		resd 1			;helper for display of current column
zloffst		resd 1			;helper: chars scrolled out at left border
fileptr		resd 1			;helper for temp storage of current pos in file
kurspos		resd 1			;cursor position set by DispNewScreen()
kurspos2	resd 1			;cursor position set by other functions

tabcnt		resd 1			;internal helper byte in DispNewScreen() only
changed		resd 1			;status byte: (UN)CHANGED
oldQFpos	resd 1
bereitsges	resd 1			;byte used for ^L

blockbegin	resd 1
blockende	resd 1
endeedit	resd 1			;byte controls program exit
old		resd 1			;helper for ^QP
veryold		resd 1			;ditto
linenr		resd 1			;current line
showblock	resd 1			;helper for ^KH
suchlaenge	resd 1			;helper for ^QA,^QF
repllaenge	resd 1
vorwarts	resd 1
grossklein	resd 1			;helper byte for ^QF,^QA
					; ^ ^ ^ ^ TODO check UTF-8 stuff here
ch2linebeg	resd 1			;helper keeping cursor pos max at EOL (up/dn keys)
numeriere	resd 1			;byte controls re-numeration
read_b		resd 1			;buffer for getchar
%ifdef W32
		resd 4			;4 extra due size INPUT_RECORD in w32
%endif		
isbold		resd 1			;control of bold display of ws-blocks
inverse		resd 1
insstat		resd 1

%ifdef AMD64
ErrNr		resq 1
%else
ErrNr		resd 1			;used similar libc errno, but not excactly equal
%endif
errlen		equ 100
error		resb errlen		;reserved space for string: 'ERROR xxx:tttteeeexxxxtttt',0

maxlen		resd 1
;-------
;
maxfilenamelen	equ 255
filepath	resb maxfilenamelen+1
bakpath		resb maxfilenamelen+1
blockpath	resb maxfilenamelen+1
replacetext	resb maxfilenamelen+1
suchtext	resb maxfilenamelen+1
suchtext2	resb maxfilenamelen+1	;for PICO mode
optbuffer	resb optslen		;buffer for search/replace options and for ^QI
linkbuffersize	equ 4
linkbuffer	resb linkbuffersize
sigaction	resd 40
;------
perms		resd 1
%ifdef SYS_fstat
 fstatbuf:	resb stat_struc_size	
%endif
%ifdef SYS_utime
 accesstime:	resb utimbuf_struc_size
%endif
;-------
screenline	resb 256+4*scolorslen	;max possible columns + 4 color ESC seq per line
					;(buffer for displaying a text line)
%ifdef W32
 attribline	resb 256*2		;attrib is a word
%endif
EmaKiSize	resd 1
EmaKiSrc	resd 1
EmaMark		resd 1
EmaCtrl:
EmaCtrlK	resb 1
EmaCtrlS	resb 1
		resb 2
EmaNumHelper	resd 1
VICmdMode	resd 1
VIbufch		resd 1
VInolinebased	resd 1
PicoSearch	resd 1			;where search started
%ifdef UTF8RTS
isUTF8		resd 1
%endif
%ifdef USE_PIPE
 sedpipeB0	resd 1
 sedpipeB1	resd 1
 sedpipeC0	resd 1
 sedpipeC1	resd 1
%endif

mode		resd 1

readfds		resd 1			;select data struc
timevalsec	resd 1			;lowest
timevalusec	resd 1			;most significant
                                                

buffercopysize	equ 1024
buffercopy	resb buffercopysize
%ifdef USE_MATH
 level		resd 1			;balance
 ptlevel	resd 1			;parenthesis balance
 %ifdef AMD64
 stackptr	resq 1
 %else
 stackptr	resd 1			;escape recursion
 %endif
 x87		resd 3			;12 byte (need 10 byte for 80 bit BCD)
 lastresult87	resq 4			;8 byte
 signctl	resd 1
%endif
%ifdef W32
 heap		resd 1
 hin		resd 1
 hout		resd 1
 w32result	resd 1			;for the w32 API calls
 csbi		resd 6			;screen_buffer_info
%endif

%ifdef SYS_brk
 max		equ 1024000		;valid for NEW created files only
%else
 max		equ 10240000
%endif
;-------
text		resb max
sot 		equ (text+1)		;start-of-text

%ifdef CRIPLED_ELF
 bsssize equ $-bssstart
%endif
