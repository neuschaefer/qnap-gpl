
//#ifndef NAS_ROOT_OID
	#define NAS_ROOT_OID	1,3,6,1,4,1,31492
//#endif

#define REGISTER_SNMP_SCALE(ITEM) netsnmp_register_scalar(netsnmp_create_handler_registration(#ITEM, handle_ ## ITEM, ITEM ## _oid, OID_LENGTH(ITEM ## _oid), HANDLER_CAN_RONLY))

int GetOneStringFromCommand(char *cmd, char *buff, int buf_size);
int GetNasDiskNumber();
int GetNasNetworkNumber();
int GetSystemFanNumber();

