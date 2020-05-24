/*
 * $Id: xml-rpc.h,v 1.1.1.1 2011/11/03 12:26:52 ken Exp $
 */

#ifndef _XMLRPC_H_
#define _XMLRPC_H_

#include "webserver.h"

extern void xml_handle(WS_CONNINFO *pwsc);

struct tag_xmlstruct;
typedef struct tag_xmlstruct XMLSTRUCT;

extern XMLSTRUCT *xml_init(WS_CONNINFO *pwsc, int emit_header);
extern void xml_push(XMLSTRUCT *pxml, char *term);
extern void xml_pop(XMLSTRUCT *pxml);
extern void xml_output(XMLSTRUCT *pxml, char *section, char *fmt, ...);
extern void xml_deinit(XMLSTRUCT *pxml);


#endif /* _XMLRPC_H_ */
