/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf 17337 2009-01-01 14:28:29Z magfr $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "SystemInfoEx.h"
#include "common.h"

/** Initializes the SystemInfo module */
int
handle_SystemCPU_UsageEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	char string[256];
	int val = 0;
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{
		FILE *fp = popen("/usr/bin/top -c 2>/dev/null", "r");
		strcpy(string, "0 %");
		if (fp) {
			while (fgets(string, sizeof(string), fp)) {
				if (strncmp(string, "CPU Usage:", 10) == 0) {
					char *ptr = string+10;
					val = atoi(ptr);
					break;
				}
			}
			fclose(fp);
		}
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &val/* XXX: a pointer to the scalar's data */,
                                     sizeof(val)/* XXX: the length of the data in bytes */);
	}
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemCPU-Usage\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_SystemTotalMemEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	long long val = 0;
    
    switch(reqinfo->mode) {

        case MODE_GET:
	{
		FILE *fp = fopen("/proc/meminfo", "r");
		if (fp) {
			char buff[256];
			while (fgets(buff, sizeof(buff), fp)) {
				char *ptr;
				if ((ptr = strstr(buff, "MemTotal:"))) {
					val = atoll(ptr+10)*1024;
					break;
				}
			}
			fclose(fp);
		}
    	}

			qnap_set_var_counter64(requests->requestvb, val);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemTotalMem\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_SystemFreeMemEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	long long val = 0;
	long long shmem = 0;
    switch(reqinfo->mode) {

        case MODE_GET:
	{
	FILE *fp = fopen("/proc/meminfo", "r");
	if (fp) {
		char buff[256];
		while (fgets(buff, sizeof(buff), fp)) {
			char *ptr;
			if ((ptr = strstr(buff, "MemFree:"))) {
				val += atoll(ptr+8);
			}
			else if ((ptr = strstr(buff, "Buffers:"))) {
				val += atoll(ptr+8);
			}
			else if ((ptr = strstr(buff, "Cached:"))) {
				val += atoll(ptr+7);
			}
			else if ((ptr = strstr(buff, "SReclaimable:"))) {
				val += atoll(ptr+13);
			}
			else if ((ptr = strstr(buff, "Shmem:"))) {
				shmem += atoll(ptr+6);
			}
		}
		fclose(fp);
		if (val >= shmem) {
			val -= shmem;
		}
	}
	}
			qnap_set_var_counter64(requests->requestvb, val);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemFreeMem\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_SystemUptimeEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	long value = get_uptime();
    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS,
                                     (u_char *) &value/* XXX: a pointer to the scalar's data */,
                                     sizeof(value)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemUptime\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_CPU_TemperatureEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	int val = 0;
	char string[256];
    switch(reqinfo->mode) {

        case MODE_GET:
		*string = 0;
		GetOneStringFromCommand("/sbin/getsysinfo cputmp", string, sizeof(string));
		val = atoi(string);
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &val, sizeof(val));
		break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CPU-Temperature\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_SystemTemperatureEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	int val = 0;
	char string[256];
    switch(reqinfo->mode) {

        case MODE_GET:
		*string = 0;
		GetOneStringFromCommand("/sbin/getsysinfo systmp", string, sizeof(string));
		val = atoi(string);
		snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &val, sizeof(val));
		break;
        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemTemperature\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_IfNumberEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	int value = GetNasNetworkNumber();
    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &value/* XXX: a pointer to the scalar's data */,
                                     sizeof(value)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_IfNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_HdNumberEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	int value = GetNasDiskNumber();
    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &value/* XXX: a pointer to the scalar's data */,
                                     sizeof(value)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_HdNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_ModelNameEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	char string[256];
    switch(reqinfo->mode) {

        case MODE_GET:
	*string = 0;
	GetOneStringFromCommand("/sbin/getsysinfo model", string, sizeof(string));
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)string /* XXX: a pointer to the scalar's data */,
                                     strlen(string)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_ModelName\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

    return SNMP_ERR_NOERROR;
}
int
handle_HostNameEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
	char string[256];
    switch(reqinfo->mode) {

        case MODE_GET:
	{
        *string = 0;
	FILE *fp = popen("/sbin/getcfg System \"Server Name\" -d \"\"", "r");
	if (fp) {
		fgets(string, sizeof(string), fp);
		fclose(fp);
	}
    	}
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *)string /* XXX: a pointer to the scalar's data */,
                                     strlen(string)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_HostName\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}
int
handle_SysFanNumberEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	int value = GetSystemFanNumber();
    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &value/* XXX: a pointer to the scalar's data */,
                                     sizeof(value)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysFanNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}
int
handle_SysVolumeNumberEx(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
	int value = GetSystemVolumeNumber();
    switch(reqinfo->mode) {

        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER,
                                     (u_char *) &value/* XXX: a pointer to the scalar's data */,
                                     sizeof(value)/* XXX: the length of the data in bytes */);
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SysVolumeNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

void
init_SystemInfoEx(void)
{
    const oid SystemCPU_Usage_oid[] = { NAS_EX_ROOT_OID,1 };
    const oid SystemTotalMem_oid[] = { NAS_EX_ROOT_OID,2 };
    const oid SystemFreeMem_oid[] = { NAS_EX_ROOT_OID,3 };
    const oid SystemUptime_oid[] = { NAS_EX_ROOT_OID,4 };
    const oid CPU_Temperature_oid[] = { NAS_EX_ROOT_OID,5 };
    const oid SystemTemperature_oid[] = { NAS_EX_ROOT_OID,6 };
    const oid IfNumber_oid[] = { NAS_EX_ROOT_OID,8 };
    const oid HdNumber_oid[] = { NAS_EX_ROOT_OID,10 };
    const oid ModelName_oid[] = { NAS_EX_ROOT_OID,12 };
    const oid HostName_oid[] = { NAS_EX_ROOT_OID,13 };
    const oid SysFanNumber_oid[] = { NAS_EX_ROOT_OID,14 };
    const oid SysVolumeNumber_oid[] = { NAS_EX_ROOT_OID,16 };

  DEBUGMSGTL(("SystemInfo", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SystemCPU-UsageEx", handle_SystemCPU_UsageEx,
                               SystemCPU_Usage_oid, OID_LENGTH(SystemCPU_Usage_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SystemTotalMemEx", handle_SystemTotalMemEx,
                               SystemTotalMem_oid, OID_LENGTH(SystemTotalMem_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SystemFreeMemEx", handle_SystemFreeMemEx,
                               SystemFreeMem_oid, OID_LENGTH(SystemFreeMem_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SystemUptimeEx", handle_SystemUptimeEx,
                               SystemUptime_oid, OID_LENGTH(SystemUptime_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("CPU-TemperatureEx", handle_CPU_TemperatureEx,
                               CPU_Temperature_oid, OID_LENGTH(CPU_Temperature_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SystemTemperatureEx", handle_SystemTemperatureEx,
                               SystemTemperature_oid, OID_LENGTH(SystemTemperature_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("IfNumberEx", handle_IfNumberEx,
                               IfNumber_oid, OID_LENGTH(IfNumber_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("HdNumberEx", handle_HdNumberEx,
                               HdNumber_oid, OID_LENGTH(HdNumber_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("ModelNameEx", handle_ModelNameEx,
                               ModelName_oid, OID_LENGTH(ModelName_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("HostNameEx", handle_HostNameEx,
                               HostName_oid, OID_LENGTH(HostName_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SysFanNumberEx", handle_SysFanNumberEx,
                               SysFanNumber_oid, OID_LENGTH(SysFanNumber_oid),
                               HANDLER_CAN_RONLY
        ));
    netsnmp_register_scalar(
        netsnmp_create_handler_registration("SysVolumeNumberEx", handle_SysVolumeNumberEx,
                               SysVolumeNumber_oid, OID_LENGTH(SysVolumeNumber_oid),
                               HANDLER_CAN_RONLY
        ));
}