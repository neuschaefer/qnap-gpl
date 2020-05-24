
#ifndef NAS_ROOT_OID
	#define NAS_ROOT_OID	1,3,6,1,4,1,24681,1,2
#endif
#define NAS_EX_ROOT_OID	1,3,6,1,4,1,24681,1,3

int GetOneStringFromCommand(char *cmd, char *buff, int buf_size);
int GetNasDiskNumber();
int GetNasNetworkNumber();
int GetSystemFanNumber();
int qnap_set_var_counter64(netsnmp_variable_list *requestvb, unsigned long long value);
