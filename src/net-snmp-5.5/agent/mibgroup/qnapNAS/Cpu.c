#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Cpu.h"
#include "common.h"


#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 

#include "../../../../../NasLib/include/qnap_pic_user.h"
#include "../../../../../NasLib/include/cfg_system.h" 

#define SRES_CPU_USAGE	2
 

// #define MAX_CPU_NUMBER	9
// #define MAX_PROC_INFO 		10
// #define MAX_PROC_NAME 		80
// #define MAX_USER_NAME 		80
//Support up to 4TiB (2^42) system memory.
// typedef struct {
	// uint mem_total;	//in kB
	// uint mem_used;	//in kB
	// uint mem_free;	//in kB
	// uint swap_total;	//in kB
	// uint swap_used;	//in kB
	// uint swap_free;	//in kB
// } MEM_INFO;
// typedef struct {
	// char proc_name[MAX_PROC_NAME];
	// char user[MAX_USER_NAME];
	// uint pid;
	// double cpu_us;
	// uint mem_us;
	// char mem_us_str[16];
// } PROC_INFO;
//cpu_usage[1~N] recorded CPU 1~N usage.
// typedef struct {
	// int cpu_array_count;
	// double cpu_usage[MAX_CPU_NUMBER];
	// int proc_count;
	// PROC_INFO *proc_info;
	// MEM_INFO mem;
// } SYS_RES;
#define	SNMP_CPUINDEX		    1
#define	SNMP_CPUID		        2
#define	SNMP_CPUUSAGE   		3



int cpunum = 0;
int curret_cpuid = -1;
int curret_cpuindex = 0;

SYS_RES sys_res;

struct variable4 cpu_variables[] = {
    {SNMP_CPUINDEX, ASN_INTEGER,RONLY,
     var_Cpu, 2, {1,1}},
    {SNMP_CPUID, ASN_INTEGER, RONLY,
     var_Cpu, 2, {1,2}},
    {SNMP_CPUUSAGE, ASN_INTEGER,RONLY,
     var_Cpu, 2, {1,3}}
};

oid CpuNumber_oid[] =     { 1,3,6,1,4,1,24681,1,4,1,1,1,1,4,1 };
oid CpuTemp_oid[] =       { 1,3,6,1,4,1,24681,1,4,1,1,1,1,4,2 };
oid cpu_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,4,3 };

int handle_CpuNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
	int rc = 0, i = 0;
	int cpucount = 0;
    memset(&sys_res,0,sizeof(sys_res));
	rc = Get_System_Resource(&sys_res,SRES_CPU_USAGE);
    if(rc == 0)
    {
		cpucount = sys_res.cpu_array_count-1;
    }        
    Free_System_Resource(&sys_res);
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &cpucount, sizeof(cpucount));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CpuNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

int handle_CpuTemp(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int sys_temp = -1; 
#ifdef QNAP_HAL_SUPPORT
    ENCLOSURE_SYS_STATUS enc_sys_status;
    SE_Get_System_Status(0, &enc_sys_status);
    if (enc_sys_status.cpu_temp > -1)
    {   
    	sys_temp = enc_sys_status.cpu_temp;
    }
    
#else
        HWMON_STATUS hw_stat;

        Get_Hwmon_All_Status(&hw_stat);
        sys_temp = (int)hw_stat.cputmp-Get_CPU_Descreased_Temp();
#endif    
		if(sys_temp <= 0)
		{
			sys_temp = -1;
		}
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &sys_temp, sizeof(sys_temp));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_CpuTemp\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

void init_Cpu(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("CpuNumber", handle_CpuNumber, CpuNumber_oid, OID_LENGTH(CpuNumber_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("CpuTemperature", handle_CpuTemp, CpuTemp_oid, OID_LENGTH(CpuTemp_oid), HANDLER_CAN_RONLY));


    cpunum = 0;
    curret_cpuid = -1;
    curret_cpuindex = 0;
    REGISTER_MIB("qnapNAS/Cpu", cpu_variables, variable4, cpu_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Cpu:init", "initializing table CpuTable\n"));
}

u_char* var_Cpu(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int cpu_idx;
    DEBUGMSGTL(("qnapNAS/Cpu:var_Cpu", "var_Cpu start\n"));
    
    cpu_idx = header_Cpu(vp, name, length, exact, var_len, write_method);
    if (cpu_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_CPUINDEX:
            long_return = cpu_idx;
            return (u_char *) & long_return;
        case SNMP_CPUID:
            long_return = curret_cpuid;
            return (u_char *) & long_return;

        case SNMP_CPUUSAGE:
        {           
            long_return = (int)sys_res.cpu_usage[cpu_idx];
            return (u_char *) & long_return;
        }
       
        default:
            DEBUGMSGTL(("var_Cpu", "unknown sub-id %d in var_Cpu\n",
                        vp->magic));
    }
    return NULL;
}

int header_Cpu(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define RAID_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             cpu_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Cpu", "start header_Cpu: "));
    DEBUGMSGOID(("header_Cpu", name, *length));
    DEBUGMSG(("header_Cpu", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Cpusetting();
    for (;;) 
    {
        cpu_idx = Get_Next_Cpu();
        DEBUGMSGTL(("header_Cpu", "... index %d\n", cpu_idx));
        if (cpu_idx == -1)
            break;
        newname[RAID_ENTRY_NAME_LENGTH] = cpu_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = cpu_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || cpu_idx < LowIndex)*/) 
        {
            LowIndex = cpu_idx;
            break;
        }
    }
 
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Cpu", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[RAID_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Cpu", "... get cpu stats "));
    DEBUGMSGOID(("header_Cpu", name, *length));
    DEBUGMSG(("header_Cpu", "\n"));

    return LowIndex;
}
void Init_Cpusetting(void)
{
    //init parameter
    cpunum = 0;
    curret_cpuid = -1;
    curret_cpuindex = 0;
    

	int rc = 0, i = 0;
    memset(&sys_res,0,sizeof(sys_res));
    rc = Get_System_Resource(&sys_res,SRES_CPU_USAGE);
    if(rc == 0)
    {
        cpunum = sys_res.cpu_array_count-1;
    }
    else
    {
        cpunum = 1;
    }  
    Free_System_Resource(&sys_res);
}


int Get_Next_Cpu(void)
{     
    if (cpunum > 0 && curret_cpuindex < cpunum)
    {
        curret_cpuindex++;
        curret_cpuid = curret_cpuindex;
        return curret_cpuindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
