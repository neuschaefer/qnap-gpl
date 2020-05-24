@--------------------------------------------------------------------
@
@  Copyright (C) 2002 Albrecht Kleine <kleine@ak.sax.de>
@
@  This program is free software; you can redistribute it and/or modify
@  it under the terms of the GNU General Public License as published by
@  the Free Software Foundation; either version 2 of the License, or
@  (at your option) any later version.
@
@  This program is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with this program; if not, write to the Free Software
@  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
@
@----------------------------------------------------------------------
.text
.global _INT
_INT:	adr r12,.table
	cmp r0,#((.endtable-.table)/8)
	movhi pc,r14
	add r12,r12,r0, lsl #3
	mov r0,r1
	mov r1,r2
	mov r2,r3
	mov r3,r4
	mov r4,r5
	mov r5,r6
	mov pc,r12		@ jump
@-------
.table:	swi 0x900000;mov pc,r14
	swi 0x900001;mov pc,r14
	swi 0x900002;mov pc,r14
	swi 0x900003;mov pc,r14
	swi 0x900004;mov pc,r14
	swi 0x900005;mov pc,r14
	swi 0x900006;mov pc,r14
	swi 0x900007;mov pc,r14
	swi 0x900008;mov pc,r14
	swi 0x900009;mov pc,r14
	swi 0x90000a;mov pc,r14
	swi 0x90000b;mov pc,r14
	swi 0x90000c;mov pc,r14
	swi 0x90000d;mov pc,r14
	swi 0x90000e;mov pc,r14
	swi 0x90000f;mov pc,r14
	swi 0x900010;mov pc,r14
	swi 0x900011;mov pc,r14
	swi 0x900012;mov pc,r14
	swi 0x900013;mov pc,r14
	swi 0x900014;mov pc,r14
	swi 0x900015;mov pc,r14
	swi 0x900016;mov pc,r14
	swi 0x900017;mov pc,r14
	swi 0x900018;mov pc,r14
	swi 0x900019;mov pc,r14
	swi 0x90001a;mov pc,r14
	swi 0x90001b;mov pc,r14
	swi 0x90001c;mov pc,r14
	swi 0x90001d;mov pc,r14
	swi 0x90001e;mov pc,r14
	swi 0x90001f;mov pc,r14
	swi 0x900020;mov pc,r14
	swi 0x900021;mov pc,r14
	swi 0x900022;mov pc,r14
	swi 0x900023;mov pc,r14
	swi 0x900024;mov pc,r14
	swi 0x900025;mov pc,r14
	swi 0x900026;mov pc,r14
	swi 0x900027;mov pc,r14
	swi 0x900028;mov pc,r14
	swi 0x900029;mov pc,r14
	swi 0x90002a;mov pc,r14
	swi 0x90002b;mov pc,r14
	swi 0x90002c;mov pc,r14
	swi 0x90002d;mov pc,r14
	swi 0x90002e;mov pc,r14
	swi 0x90002f;mov pc,r14
	swi 0x900030;mov pc,r14
	swi 0x900031;mov pc,r14
	swi 0x900032;mov pc,r14
	swi 0x900033;mov pc,r14
	swi 0x900034;mov pc,r14
	swi 0x900035;mov pc,r14
	swi 0x900036;mov pc,r14
	swi 0x900037;mov pc,r14
	swi 0x900038;mov pc,r14
	swi 0x900039;mov pc,r14
	swi 0x90003a;mov pc,r14
	swi 0x90003b;mov pc,r14
	swi 0x90003c;mov pc,r14
	swi 0x90003d;mov pc,r14
	swi 0x90003e;mov pc,r14
	swi 0x90003f;mov pc,r14
	swi 0x900040;mov pc,r14
	swi 0x900041;mov pc,r14
	swi 0x900042;mov pc,r14
	swi 0x900043;mov pc,r14
	swi 0x900044;mov pc,r14
	swi 0x900045;mov pc,r14
	swi 0x900046;mov pc,r14
	swi 0x900047;mov pc,r14
	swi 0x900048;mov pc,r14
	swi 0x900049;mov pc,r14
	swi 0x90004a;mov pc,r14
	swi 0x90004b;mov pc,r14
	swi 0x90004c;mov pc,r14
	swi 0x90004d;mov pc,r14
	swi 0x90004e;mov pc,r14
	swi 0x90004f;mov pc,r14
	swi 0x900050;mov pc,r14
	swi 0x900051;mov pc,r14
	swi 0x900052;mov pc,r14
	swi 0x900053;mov pc,r14
	swi 0x900054;mov pc,r14
	swi 0x900055;mov pc,r14
	swi 0x900056;mov pc,r14
	swi 0x900057;mov pc,r14
	swi 0x900058;mov pc,r14
	swi 0x900059;mov pc,r14
	swi 0x90005a;mov pc,r14
	swi 0x90005b;mov pc,r14
	swi 0x90005c;mov pc,r14
	swi 0x90005d;mov pc,r14
	swi 0x90005e;mov pc,r14
	swi 0x90005f;mov pc,r14
	swi 0x900060;mov pc,r14
	swi 0x900061;mov pc,r14
	swi 0x900062;mov pc,r14
	swi 0x900063;mov pc,r14
	swi 0x900064;mov pc,r14
	swi 0x900065;mov pc,r14
	swi 0x900066;mov pc,r14
	swi 0x900067;mov pc,r14
	swi 0x900068;mov pc,r14
	swi 0x900069;mov pc,r14
	swi 0x90006a;mov pc,r14
	swi 0x90006b;mov pc,r14
	swi 0x90006c;mov pc,r14
	swi 0x90006d;mov pc,r14
	swi 0x90006e;mov pc,r14
	swi 0x90006f;mov pc,r14
	swi 0x900070;mov pc,r14
	swi 0x900071;mov pc,r14
	swi 0x900072;mov pc,r14
	swi 0x900073;mov pc,r14
	swi 0x900074;mov pc,r14
	swi 0x900075;mov pc,r14
	swi 0x900076;mov pc,r14
	swi 0x900077;mov pc,r14
	swi 0x900078;mov pc,r14
	swi 0x900079;mov pc,r14
	swi 0x90007a;mov pc,r14
	swi 0x90007b;mov pc,r14
	swi 0x90007c;mov pc,r14
	swi 0x90007d;mov pc,r14
	swi 0x90007e;mov pc,r14
	swi 0x90007f;mov pc,r14
	swi 0x900080;mov pc,r14
	swi 0x900081;mov pc,r14
	swi 0x900082;mov pc,r14
	swi 0x900083;mov pc,r14
	swi 0x900084;mov pc,r14
	swi 0x900085;mov pc,r14
	swi 0x900086;mov pc,r14
	swi 0x900087;mov pc,r14
	swi 0x900088;mov pc,r14
	swi 0x900089;mov pc,r14
	swi 0x90008a;mov pc,r14
	swi 0x90008b;mov pc,r14
	swi 0x90008c;mov pc,r14
	swi 0x90008d;mov pc,r14
	swi 0x90008e;mov pc,r14
	swi 0x90008f;mov pc,r14
	swi 0x900090;mov pc,r14
	swi 0x900091;mov pc,r14
	swi 0x900092;mov pc,r14
	swi 0x900093;mov pc,r14
	swi 0x900094;mov pc,r14
	swi 0x900095;mov pc,r14
	swi 0x900096;mov pc,r14
	swi 0x900097;mov pc,r14
	swi 0x900098;mov pc,r14
	swi 0x900099;mov pc,r14
	swi 0x90009a;mov pc,r14
	swi 0x90009b;mov pc,r14
	swi 0x90009c;mov pc,r14
	swi 0x90009d;mov pc,r14
	swi 0x90009e;mov pc,r14
	swi 0x90009f;mov pc,r14
	swi 0x9000a0;mov pc,r14
	swi 0x9000a1;mov pc,r14
	swi 0x9000a2;mov pc,r14
	swi 0x9000a3;mov pc,r14
	swi 0x9000a4;mov pc,r14
	swi 0x9000a5;mov pc,r14
	swi 0x9000a6;mov pc,r14
	swi 0x9000a7;mov pc,r14
	swi 0x9000a8;mov pc,r14
	swi 0x9000a9;mov pc,r14
	swi 0x9000aa;mov pc,r14
	swi 0x9000ab;mov pc,r14
	swi 0x9000ac;mov pc,r14
	swi 0x9000ad;mov pc,r14
	swi 0x9000ae;mov pc,r14
	swi 0x9000af;mov pc,r14
	swi 0x9000b0;mov pc,r14
	swi 0x9000b1;mov pc,r14
	swi 0x9000b2;mov pc,r14
	swi 0x9000b3;mov pc,r14
	swi 0x9000b4;mov pc,r14
	swi 0x9000b5;mov pc,r14
	swi 0x9000b6;mov pc,r14
	swi 0x9000b7;mov pc,r14
	swi 0x9000b8;mov pc,r14
	swi 0x9000b9;mov pc,r14
	swi 0x9000ba;mov pc,r14
	swi 0x9000bb;mov pc,r14
	swi 0x9000bc;mov pc,r14
	swi 0x9000bd;mov pc,r14
	swi 0x9000be;mov pc,r14
	swi 0x9000bf;mov pc,r14
	swi 0x9000c0;mov pc,r14
	swi 0x9000c1;mov pc,r14
	swi 0x9000c2;mov pc,r14
	swi 0x9000c3;mov pc,r14
	swi 0x9000c4;mov pc,r14
	swi 0x9000c5;mov pc,r14
	swi 0x9000c6;mov pc,r14
	swi 0x9000c7;mov pc,r14
	swi 0x9000c8;mov pc,r14
	swi 0x9000c9;mov pc,r14
	swi 0x9000ca;mov pc,r14
	swi 0x9000cb;mov pc,r14
	swi 0x9000cc;mov pc,r14
	swi 0x9000cd;mov pc,r14
	swi 0x9000ce;mov pc,r14
	swi 0x9000cf;mov pc,r14
	swi 0x9000d0;mov pc,r14
	swi 0x9000d1;mov pc,r14
	swi 0x9000d2;mov pc,r14
	swi 0x9000d3;mov pc,r14
	swi 0x9000d4;mov pc,r14
	swi 0x9000d5;mov pc,r14
	swi 0x9000d6;mov pc,r14
	swi 0x9000d7;mov pc,r14
	swi 0x9000d8;mov pc,r14
	swi 0x9000d9;mov pc,r14
	swi 0x9000da;mov pc,r14
	swi 0x9000db;mov pc,r14
	swi 0x9000dc;mov pc,r14
	swi 0x9000dd;mov pc,r14
	swi 0x9000de;mov pc,r14
	swi 0x9000df;mov pc,r14
	swi 0x9000e0;mov pc,r14
	swi 0x9000e1;mov pc,r14
	swi 0x9000e2;mov pc,r14
	swi 0x9000e3;mov pc,r14
	swi 0x9000e4;mov pc,r14
	swi 0x9000e5;mov pc,r14
	swi 0x9000e6;mov pc,r14
	swi 0x9000e7;mov pc,r14
	swi 0x9000e8;mov pc,r14
	swi 0x9000e9;mov pc,r14
	swi 0x9000ea;mov pc,r14
	swi 0x9000eb;mov pc,r14
	swi 0x9000ec;mov pc,r14
	swi 0x9000ed;mov pc,r14
	swi 0x9000ee;mov pc,r14
	swi 0x9000ef;mov pc,r14
	swi 0x9000f0;mov pc,r14
	swi 0x9000f1;mov pc,r14
	swi 0x9000f2;mov pc,r14
	swi 0x9000f3;mov pc,r14
	swi 0x9000f4;mov pc,r14
	swi 0x9000f5;mov pc,r14
	swi 0x9000f6;mov pc,r14
	swi 0x9000f7;mov pc,r14
	swi 0x9000f8;mov pc,r14
	swi 0x9000f9;mov pc,r14
	swi 0x9000fa;mov pc,r14
	swi 0x9000fb;mov pc,r14
	swi 0x9000fc;mov pc,r14
	swi 0x9000fd;mov pc,r14
	swi 0x9000fe;mov pc,r14
	swi 0x9000ff;mov pc,r14
.endtable:
@-------
@
.global _DIV
@ this code is not GPL, but free
@ expects divisor  in r10 and  divident in r9 
@ returns quotient in r12 and remainder in r9
@ uses r11
_DIV:	MOV	r11,#1
.div1:	CMP	r10,#0x80000000	@ shift divisor left until top bit set
	CMPCC	r10,r9		@ ...or divisor>divident
	MOVCC	r10,r10,ASL#1	@ shift divisor left if required
	MOVCC	r11,r11,ASL#1	@ shift r11 left if required
	BCC	.div1		@ repeat whilst more shifting required
	MOV	r12,#0		@ ip used to store result
.div2:	CMP	r9,r10		@ test for possible subtraction
        SUBCS	r9,r9,r10	@ subtract if divident>divisor
        ADDCS	r12,r12,r11	@ put relivant bit into result
        MOVS	r11,r11,LSR#1	@ shift control bit
        MOVNE	r10,r10,LSR#1	@ halve unless finished
        BNE	.div2		@ loop if there is more to do
	@ remainder r9
	@ result in r12
        mov 	pc,r14
@-------
