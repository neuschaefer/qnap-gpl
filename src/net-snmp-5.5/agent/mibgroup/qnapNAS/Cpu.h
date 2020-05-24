#ifndef SNMP_CPU_H
#define SNMP_CPU_H



int handle_CpuNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Cpu(void);
void Init_Cpusetting(void);
int Get_Next_Cpu(void);
int header_Cpu(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Cpu(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
