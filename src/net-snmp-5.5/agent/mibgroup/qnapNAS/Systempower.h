#ifndef SNMP_SYSTEMPOWER_H
#define SNMP_SYSTEMPOWER_H



int handle_SystempowerNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Systempower(void);
void Init_Systempowersetting(void);
int Get_Next_Systempower(void);
int header_Systempower(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Systempower(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
