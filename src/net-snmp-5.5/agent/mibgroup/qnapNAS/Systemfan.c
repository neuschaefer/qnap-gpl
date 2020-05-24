#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Systemfan.h"
#include "common.h"


#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 


int fan_stat(int fan_speed, int sys_temp)
{
	int fanspeed_threshold = 60;
	int fan_stat = -1; //0: success -1: fail
	if( fan_speed < fanspeed_threshold )
	{
		int is_custom_temp = Get_Profile_Integer("Misc", "Custom Temp", 0);
		int stop_temp = Get_Profile_Integer("Misc", "system stop temp", 25);
		if(is_custom_temp)
		{
			if( sys_temp < stop_temp )
			{
				fan_stat = 0;
			}else{
				fan_stat = -1;
			}
		}
	}else{
		fan_stat = 0;
	}
	return fan_stat;
}
 


#define	SNMP_SYSTEMFANINDEX		    1
#define	SNMP_SYSTEMFANID		    2
#define	SNMP_SYSTEMFANENCLOSUREID	3
#define	SNMP_SYSTEMFANSTATUS        4
#define	SNMP_SYSTEMFANSPEED     	5


int systemfannum = 0;
int curret_systemfanid = -1;
int curret_systemfanindex = 0;

int enc_id_ary[MAX_SE_NUM];
typedef struct {
	int fanid;	
	int fanencid;	
	int fanstatus;	
	int fanspeed;	
} SNMPSYSFAN_INFO;




SNMPSYSFAN_INFO * sysfan_listP = NULL;


struct variable4 systemfan_variables[] = {
    {SNMP_SYSTEMFANINDEX, ASN_INTEGER,RONLY,
     var_Systemfan, 2, {1,1}},
    {SNMP_SYSTEMFANID, ASN_INTEGER, RONLY,
     var_Systemfan, 2, {1,2}},
    {SNMP_SYSTEMFANENCLOSUREID, ASN_INTEGER,RONLY,
     var_Systemfan, 2, {1,3}},
    {SNMP_SYSTEMFANSTATUS, ASN_INTEGER,RONLY,
     var_Systemfan, 2, {1,4}},
    {SNMP_SYSTEMFANSPEED, ASN_INTEGER,RONLY,
     var_Systemfan, 2, {1,5}}     
};

oid SystemfanNumber_oid[] =     { 1,3,6,1,4,1,24681,1,4,1,1,1,1,2,1 };
oid systemfan_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,2,2 };

int handle_SystemfanNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{

    memset(enc_id_ary, 0, sizeof(enc_id_ary));

    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, NULL, NULL);

	systemfannum = 0;
	int i = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
		systemfannum += enc_sys_status.fan_count;
    }        
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &systemfannum, sizeof(systemfannum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystemfanNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}



void init_Systemfan(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("SystemfanNumber", handle_SystemfanNumber, SystemfanNumber_oid, OID_LENGTH(SystemfanNumber_oid), HANDLER_CAN_RONLY));


    systemfannum = 0;
    curret_systemfanid = -1;
    curret_systemfanindex = 0;
    REGISTER_MIB("qnapNAS/Systemfan", systemfan_variables, variable4, systemfan_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Systemfan:init", "initializing table SystemfanTable\n"));
}

u_char* var_Systemfan(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int systemfan_idx;
    DEBUGMSGTL(("qnapNAS/Systemfan:var_Systemfan", "var_Systemfan start\n"));
    
    systemfan_idx = header_Systemfan(vp, name, length, exact, var_len, write_method);
    if (systemfan_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_SYSTEMFANINDEX:
            long_return = systemfan_idx;
            return (u_char *) & long_return;
        case SNMP_SYSTEMFANID:
            long_return = sysfan_listP[systemfan_idx-1].fanid;
            return (u_char *) & long_return;
        case SNMP_SYSTEMFANENCLOSUREID:
        {           
            long_return = sysfan_listP[systemfan_idx-1].fanencid;
            return (u_char *) & long_return;
        }
        case SNMP_SYSTEMFANSTATUS:
        {           
            long_return = sysfan_listP[systemfan_idx-1].fanstatus;
            return (u_char *) & long_return;
        }
        case SNMP_SYSTEMFANSPEED:
        {           
            long_return = sysfan_listP[systemfan_idx-1].fanspeed;
            return (u_char *) & long_return;
        }        
        default:
            DEBUGMSGTL(("var_Systemfan", "unknown sub-id %d in var_Systemfan\n",
                        vp->magic));
    }
    return NULL;
}

int header_Systemfan(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define SYSTEMFAN_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             systemfan_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Systemfan", "start header_Systemfan: "));
    DEBUGMSGOID(("header_Systemfan", name, *length));
    DEBUGMSG(("header_Systemfan", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Systemfansetting();
    for (;;) 
    {
        systemfan_idx = Get_Next_Systemfan();
        DEBUGMSGTL(("header_Systemfan", "... index %d\n", systemfan_idx));
        if (systemfan_idx == -1)
            break;
        newname[SYSTEMFAN_ENTRY_NAME_LENGTH] = systemfan_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = systemfan_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || systemfan_idx < LowIndex)*/) 
        {
            LowIndex = systemfan_idx;
            break;
        }
    }
 
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Systemfan", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[SYSTEMFAN_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Systemfan", "... get systemfan stats "));
    DEBUGMSGOID(("header_Systemfan", name, *length));
    DEBUGMSG(("header_Systemfan", "\n"));

    return LowIndex;
}
void Init_Systemfansetting(void)
{
    //init parameter
    systemfannum = 0;
    curret_systemfanid = -1;
    curret_systemfanindex = 0;

    if(sysfan_listP != NULL)
    {
        free(sysfan_listP);
        sysfan_listP = NULL;
    }
    
    memset(enc_id_ary, 0, sizeof(enc_id_ary));
    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, NULL, NULL);

	systemfannum = 0;
    int i = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
		systemfannum += enc_sys_status.fan_count;
    }
    //get sys fan total count, allocate all sys fan structure
    sysfan_listP = calloc(systemfannum, sizeof(SNMPSYSFAN_INFO));
    
    int fanidx = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
        int j = 0;
        for(j = 0; j < enc_sys_status.fan_count; j++)
        {            
            //fill in value
            sysfan_listP[fanidx].fanid = j+1;	
            sysfan_listP[fanidx].fanencid = enc_id_ary[i];	
            sysfan_listP[fanidx].fanstatus = fan_stat(enc_sys_status.fan_rpm[j] , enc_sys_status.sys_temp);
            sysfan_listP[fanidx].fanspeed = (enc_sys_status.fan_status[j] == 0) ? enc_sys_status.fan_rpm[j] : 0;            
            
            fanidx++;
        }
    } 
}


int Get_Next_Systemfan(void)
{  
    if (systemfannum > 0 && curret_systemfanindex < systemfannum)
    {
        curret_systemfanindex++;
        curret_systemfanid = sysfan_listP[curret_systemfanindex - 1].fanid;
              
        return curret_systemfanindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
