#ifndef SNMP_MSATADISK_H
#define SNMP_MSATADISK_H




extern void init_Msatadisk(void);
void Init_Msatadisksetting(void);
int Get_Next_Msatadisk(void);
int header_Msatadisk(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);
u_char* var_Msatadisk(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method);
#endif
