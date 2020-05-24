/* $Header: /home/cvsroot/NasX86/DataService/DBMS/mysql-5.1.36/pstack/pstacktrace.h,v 1.1.1.1 2009/08/06 13:22:49 kent Exp $ */

/*
 * Debugging macros.
 */

#ifndef	pstacktrace_h_
#define	pstacktrace_h_

#define	PSTACK_DEBUG 1
#undef PSTACK_DEBUG

#ifdef PSTACK_DEBUG
#	define	TRACE_PUTC(a)		putc a
#	define	TRACE_FPUTS(a)		fputs a
#	define	TRACE_FPRINTF(a)	fprintf a
#else /* PSTACK_DEBUG */
#	define	TRACE_PUTC(a)		(void)0
#	define	TRACE_FPUTS(a)		(void)0
#	define	TRACE_FPRINTF(a)	(void)0
#endif /* !PSTACK_DEBUG */

#endif /* pstacktrace_h_ */

