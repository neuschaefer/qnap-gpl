#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Systempower.h"
#include "common.h"
#include "Enclosure.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 


// static int get_enclosure_list_callback(int enc_id, void *contextP)
// {
    // int ret = 0;
    // int is_internal = SE_Is_Internal(enc_id);

    
    // if (0 == is_internal)
        // ret = -2;
    // else
    
    // if (1 == SE_Is_Removing(enc_id))    
        // ret = -2;

    
    // return ret;    
// }

#define	SNMP_SYSTEMPOWERINDEX		    1
#define	SNMP_SYSTEMPOWERID		    2
#define	SNMP_SYSTEMPOWERENCLOSUREID	3
#define	SNMP_SYSTEMPOWERSTATUS        4
#define	SNMP_SYSTEMPOWERSPEED     	5
#define	SNMP_SYSTEMPOWERTEMP     	6

int systempowernum = 0;
int curret_systempowerid = -1;
int curret_systempowerindex = 0;

int enc_id_ary[MAX_SE_NUM];
typedef struct {
	int powerid;	
	int powerencid;	
	int powerstatus;	
	int powerspeed;	
    int powertemp;
} SNMPSYSPOWER_INFO;




SNMPSYSPOWER_INFO * syspower_listP = NULL;


struct variable4 systempower_variables[] = {
    {SNMP_SYSTEMPOWERINDEX, ASN_INTEGER,RONLY,
     var_Systempower, 2, {1,1}},
    {SNMP_SYSTEMPOWERID, ASN_INTEGER, RONLY,
     var_Systempower, 2, {1,2}},
    {SNMP_SYSTEMPOWERENCLOSUREID, ASN_INTEGER,RONLY,
     var_Systempower, 2, {1,3}},
    {SNMP_SYSTEMPOWERSTATUS, ASN_INTEGER,RONLY,
     var_Systempower, 2, {1,4}},
    {SNMP_SYSTEMPOWERSPEED, ASN_INTEGER,RONLY,
     var_Systempower, 2, {1,5}},
    {SNMP_SYSTEMPOWERTEMP, ASN_INTEGER,RONLY,
     var_Systempower, 2, {1,6}}       
};

oid SystempowerNumber_oid[] =     { 1,3,6,1,4,1,24681,1,4,1,1,1,1,3,1 };
oid systempower_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,3,2 };

int handle_SystempowerNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{

    memset(enc_id_ary, 0, sizeof(enc_id_ary));

    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, enclosure_list_callback, NULL);

	systempowernum = 0;
	int i = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
		systempowernum += enc_sys_status.pwr_count;
    }        
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &systempowernum, sizeof(systempowernum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_SystempowerNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}



void init_Systempower(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("SystempowerNumber", handle_SystempowerNumber, SystempowerNumber_oid, OID_LENGTH(SystempowerNumber_oid), HANDLER_CAN_RONLY));


    systempowernum = 0;
    curret_systempowerid = -1;
    curret_systempowerindex = 0;
    REGISTER_MIB("qnapNAS/Systempower", systempower_variables, variable4, systempower_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Systempower:init", "initializing table SystempowerTable\n"));
}

u_char* var_Systempower(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int systempower_idx;
    DEBUGMSGTL(("qnapNAS/Systempower:var_Systempower", "var_Systempower start\n"));
    
    systempower_idx = header_Systempower(vp, name, length, exact, var_len, write_method);
    if (systempower_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_SYSTEMPOWERINDEX:
            long_return = systempower_idx;
            return (u_char *) & long_return;
        case SNMP_SYSTEMPOWERID:
            long_return = syspower_listP[systempower_idx-1].powerid;
            return (u_char *) & long_return;
        case SNMP_SYSTEMPOWERENCLOSUREID:
        {           
            long_return = syspower_listP[systempower_idx-1].powerencid;
            return (u_char *) & long_return;
        }
        case SNMP_SYSTEMPOWERSTATUS:
        {           
            long_return = syspower_listP[systempower_idx-1].powerstatus;
            return (u_char *) & long_return;
        }
        case SNMP_SYSTEMPOWERSPEED:
        {           
            long_return = syspower_listP[systempower_idx-1].powerspeed;
            return (u_char *) & long_return;
        }
        case SNMP_SYSTEMPOWERTEMP:
        {           
            long_return = syspower_listP[systempower_idx-1].powertemp;
            return (u_char *) & long_return;
        }                
        default:
            DEBUGMSGTL(("var_Systempower", "unknown sub-id %d in var_Systempower\n",
                        vp->magic));
    }
    return NULL;
}

int header_Systempower(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define SYSTEMPOWER_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             systempower_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Systempower", "start header_Systempower: "));
    DEBUGMSGOID(("header_Systempower", name, *length));
    DEBUGMSG(("header_Systempower", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Systempowersetting();
    for (;;) 
    {
        systempower_idx = Get_Next_Systempower();
        DEBUGMSGTL(("header_Systempower", "... index %d\n", systempower_idx));
        if (systempower_idx == -1)
            break;
        newname[SYSTEMPOWER_ENTRY_NAME_LENGTH] = systempower_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = systempower_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || systempower_idx < LowIndex)*/) 
        {
            LowIndex = systempower_idx;
            break;
        }
    }
 
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Systempower", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[SYSTEMPOWER_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Systempower", "... get systempower stats "));
    DEBUGMSGOID(("header_Systempower", name, *length));
    DEBUGMSG(("header_Systempower", "\n"));

    return LowIndex;
}
void Init_Systempowersetting(void)
{
    //init parameter
    systempowernum = 0;
    curret_systempowerid = -1;
    curret_systempowerindex = 0;

    if(syspower_listP != NULL)
    {
        free(syspower_listP);
        syspower_listP = NULL;
    }
    
    memset(enc_id_ary, 0, sizeof(enc_id_ary));
    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, enclosure_list_callback, NULL);

	systempowernum = 0;
    int i = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
		systempowernum += enc_sys_status.pwr_count;
    }
    //get sys power total count, allocate all sys power structure
    syspower_listP = calloc(systempowernum, sizeof(SNMPSYSPOWER_INFO));
    
    int poweridx = 0;
    for(i = 0; i < se_count; i++)
    {
        ENCLOSURE_SYS_STATUS enc_sys_status;
        SE_Get_System_Status(enc_id_ary[i], &enc_sys_status);
        int j = 0;
        for(j = 0; j < enc_sys_status.pwr_count; j++)
        {            
            //fill in value
            syspower_listP[poweridx].powerid = j+1;	
            syspower_listP[poweridx].powerencid = enc_id_ary[i];	
            syspower_listP[poweridx].powerstatus = enc_sys_status.pwr_status[j];
            syspower_listP[poweridx].powerspeed = enc_sys_status.pwr_rpm[j];            
            syspower_listP[poweridx].powertemp = enc_sys_status.pwr_temp[j];            
            poweridx++;
        }
    } 
}


int Get_Next_Systempower(void)
{  
    if (systempowernum > 0 && curret_systempowerindex < systempowernum)
    {
        curret_systempowerindex++;
        curret_systempowerid = syspower_listP[curret_systempowerindex - 1].powerid;
              
        return curret_systempowerindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
