#ifndef SNMP_DISKPERFORMANCE_H
#define SNMP_DISKPERFORMANCE_H

int handle_DiskperformanceNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Diskperformance(void);
void Init_Diskperformancesetting(void);
int Get_Next_Diskperformance(void);
int header_Diskperformance(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Diskperformance(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
