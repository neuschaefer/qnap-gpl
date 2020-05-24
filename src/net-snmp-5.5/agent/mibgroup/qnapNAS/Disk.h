#ifndef SNMP_DISK_H
#define SNMP_DISK_H



int handle_SystempowerNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Disk(void);
void Init_Disksetting(void);
int Get_Next_Disk(void);
int header_Disk(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Disk(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
