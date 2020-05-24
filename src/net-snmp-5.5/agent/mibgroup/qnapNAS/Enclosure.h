#ifndef SNMP_ENCLOSURE_H
#define SNMP_ENCLOSURE_H


extern int enclosure_list_callback(int enc_id, void *contextP);
int handle_EnclosureNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
extern void init_Enclosure(void);
void Init_Enclosuresetting(void);
int Get_Next_Enclosure(void);
int header_enclosure(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_enclosure(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
