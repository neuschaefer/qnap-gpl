/* $Header: /home/cvsroot/NasX86/DataService/DBMS/mysql-5.1.36/pstack/pstack.h,v 1.1.1.1 2009/08/06 13:22:49 kent Exp $ */

#ifndef	pstack_pstack_h_
#define	pstack_pstack_h_

#include	"pstacktrace.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Install the stack-trace-on-SEGV handler....
 */
extern int
pstack_install_segv_action(	const char*	path_format);
#ifdef __cplusplus
}
#endif

#endif /* pstack_pstack_h_ */

