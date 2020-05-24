#ifndef SNMP_ISCSITARGET_H
#define SNMP_ISCSITARGET_H

int handle_IscsitargetNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Iscsitarget(void);
void Init_Iscsitargetsetting(void);
int Get_Next_Iscsitarget(void);
int header_Iscsitarget(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Iscsitarget(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
