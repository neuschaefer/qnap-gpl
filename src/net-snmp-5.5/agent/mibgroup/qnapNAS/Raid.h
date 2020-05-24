#ifndef SNMP_RAID_H
#define SNMP_RAID_H



int handle_RaidNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Raid(void);
void Init_Raidsetting(void);
int Get_Next_Raid(void);
int header_Raid(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Raid(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
