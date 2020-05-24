#ifndef SNMP_POOL_H
#define SNMP_POOL_H



int handle_PoolNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Pool(void);
void Init_Poolsetting(void);
int Get_Next_Pool(void);
int header_pool(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_pool(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
