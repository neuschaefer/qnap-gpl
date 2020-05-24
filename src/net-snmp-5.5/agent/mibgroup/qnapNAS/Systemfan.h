#ifndef SNMP_SYSTEMFAN_H
#define SNMP_SYSTEMFAN_H



int handle_SystemfanNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Systemfan(void);
void Init_Systemfansetting(void);
int Get_Next_Systemfan(void);
int header_Systemfan(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Systemfan(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
