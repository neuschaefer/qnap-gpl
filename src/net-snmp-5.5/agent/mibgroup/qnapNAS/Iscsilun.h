#ifndef SNMP_ISCSILUN_H
#define SNMP_ISCSILUN_H

int handle_IscsilunNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Iscsilun(void);
void Init_Iscsilunsetting(void);
int Get_Next_Iscsilun(void);
int header_Iscsilun(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Iscsilun(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
