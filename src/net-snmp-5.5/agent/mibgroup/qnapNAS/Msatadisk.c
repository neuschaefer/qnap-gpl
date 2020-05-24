#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Msatadisk.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 



#define	SNMP_MSATADISKINDEX		    1
#define	SNMP_MSATADISKID		        2
#define	SNMP_MSATADISKENCLOSUREID	3
#define	SNMP_MSATADISKSUMMARY         4
#define	SNMP_MSATADISKSMARTINFO     	5
#define	SNMP_MSATADISKTEMP     	    6
#define	SNMP_MSATADISKGLOBALSPARE    7
#define	SNMP_MSATADISKMODEL       	8
#define	SNMP_MSATADISKCAPACITY 	    9

int msatadisknum = 0;
int curret_msatadiskid = -1;
int curret_msatadiskindex = 0;
static char string[256]={};
static struct counter64 counter64_return;

int msata_pd_id_ary[MAX_PD_NUM];
int enc_id = 0;

struct variable4 msatadisk_variables[] = {
    {SNMP_MSATADISKINDEX, ASN_INTEGER,RONLY,
     var_Msatadisk, 2, {1,1}},
    {SNMP_MSATADISKID, ASN_INTEGER, RONLY,
     var_Msatadisk, 2, {1,2}},
    {SNMP_MSATADISKENCLOSUREID, ASN_INTEGER,RONLY,
     var_Msatadisk, 2, {1,3}},
    {SNMP_MSATADISKSUMMARY, ASN_OCTET_STR,RONLY,
     var_Msatadisk, 2, {1,4}},
    {SNMP_MSATADISKSMARTINFO, ASN_INTEGER,RONLY,
     var_Msatadisk, 2, {1,5}},
    {SNMP_MSATADISKTEMP, ASN_INTEGER,RONLY,
     var_Msatadisk, 2, {1,6}},
    {SNMP_MSATADISKGLOBALSPARE, ASN_INTEGER,RONLY,
     var_Msatadisk, 2, {1,7}},
    {SNMP_MSATADISKMODEL, ASN_OCTET_STR,RONLY,
     var_Msatadisk, 2, {1,8}},
    {SNMP_MSATADISKCAPACITY, ASN_COUNTER64,RONLY,
     var_Msatadisk, 2, {1,9}}       
};

oid MsatadiskNumber_oid[] =     { 1,3,6,1,4,1,24681,1,4,1,1,1,1,6,1 };
oid msatadisk_variables_oid[] =  { 1,3,6,1,4,1,24681,1,4,1,1,1,1,6,2 };

int handle_MsatadiskNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
	enc_id = SE_Get_Cache_Enc_Id();
	msatadisknum = 0;
    msatadisknum = PD_Enumerate(enc_id, NULL, 0, NULL, NULL);
       
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &msatadisknum, sizeof(msatadisknum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}



void init_Msatadisk(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("MsatadiskNumber", handle_MsatadiskNumber, MsatadiskNumber_oid, OID_LENGTH(MsatadiskNumber_oid), HANDLER_CAN_RONLY));


    msatadisknum = 0;
    curret_msatadiskid = -1;
    curret_msatadiskindex = 0;
    REGISTER_MIB("qnapNAS/Msatadisk", msatadisk_variables, variable4, msatadisk_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Msatadisk:init", "initializing table MsatadiskTable\n"));
}

u_char* var_Msatadisk(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int disk_idx;
    DEBUGMSGTL(("qnapNAS/Msatadisk:var_Msatadisk", "var_Msatadisk start\n"));
    
    disk_idx = header_Msatadisk(vp, name, length, exact, var_len, write_method);
    if (disk_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_MSATADISKINDEX:
            long_return = disk_idx;
            return (u_char *) & long_return;
        case SNMP_MSATADISKID:
            long_return = msata_pd_id_ary[disk_idx-1];
            return (u_char *) & long_return;
        case SNMP_MSATADISKENCLOSUREID:
        {           
            long_return = enc_id;
            return (u_char *) & long_return;
        }
        case SNMP_MSATADISKSUMMARY:
        { 
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            //get hd_status
            PD_DEV_ID dev_id; 
            dev_id = PD_MAKE_DEV_ID(enc_id, msata_pd_id_ary[disk_idx-1]);
            STORAGE_STATUS disk_status;  
            NAS_Disk_Get_Status(dev_id, &disk_status); 
 
            
            //get smart status
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(enc_id, msata_pd_id_ary[disk_idx-1], &summary);            
            int smartresult = !result ? summary.health : -1;
            
            //blockscan
           // int bd_status = Get_Badblocks_Status(dev_id);
            
            if(    disk_status             <  SS_READY
                    ||  smartresult               ==  2 /*SMART_ABNORMAL*/
                    ||  smartresult               ==  -1 /*SMART_UNKNOW*/)
            {
                snprintf(string, sizeof(string), "Abnormal");
            }
            else if(    smartresult               == 1 /*me.SMART_NORMAL*/ )
            {
                snprintf(string, sizeof(string), "Warning");
            }
            // else if(    bd_status             == 1 /*SCAN_SCANNING*/ )
            // {
                // snprintf(string, sizeof(string),  "Scanning");
            // }
            // else if(    disk_status             == SS_REBUILDING_RAID )
            // {
                // snprintf(string, sizeof(string), "Rebuilding");
            // }
            else
            {
                snprintf(string, sizeof(string), "Good");
            }            
            
            *var_len = strlen(string);
            return (unsigned char *) string; 
        }
        case SNMP_MSATADISKSMARTINFO:
        {   
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(enc_id, msata_pd_id_ary[disk_idx-1], &summary);
            long_return = !result ? summary.health : -1;      
            return (u_char *) & long_return;
        }
        case SNMP_MSATADISKTEMP:
        {   
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(enc_id, msata_pd_id_ary[disk_idx-1], &summary);
			if(summary.temperature == 0)
			{
				long_return =-1;				
			}
			else
			{
				long_return = summary.temperature;
			}				
            return (u_char *) & long_return;
        }  
        case SNMP_MSATADISKGLOBALSPARE:
        {           
            long_return = RAID_Is_Global_Spare(enc_id, msata_pd_id_ary[disk_idx-1]);            ; 
            return (u_char *) & long_return;
        }  
        case SNMP_MSATADISKMODEL:
        {      
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';

            PD_INFO pd_info;
            PD_Get_Info(enc_id, msata_pd_id_ary[disk_idx-1], &pd_info);
            snprintf(string, sizeof(string), "%s %s (mSATA)",pd_info.vendor, pd_info.model);

            
            *var_len = strlen(string);
            return (unsigned char *) string; 
        }  
        case SNMP_MSATADISKCAPACITY:
        { 
            PD_INFO pd_info;
            PD_Get_Info(enc_id, msata_pd_id_ary[disk_idx-1], &pd_info);       
            unsigned long long uCapacity = pd_info.capacity * pd_info.sector_size;        

            counter64_return.low = uCapacity & 0xffffffff;
            counter64_return.high = uCapacity >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }          
        default:
            DEBUGMSGTL(("var_Msatadisk", "unknown sub-id %d in var_Msatadisk\n",
                        vp->magic));
    }
    return NULL;
}

int header_Msatadisk(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define MSATADISK_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             disk_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Msatadisk", "start header_Msatadisk: "));
    DEBUGMSGOID(("header_Msatadisk", name, *length));
    DEBUGMSG(("header_Msatadisk", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Msatadisksetting();
    for (;;) 
    {
        disk_idx = Get_Next_Msatadisk();
        DEBUGMSGTL(("header_Msatadisk", "... index %d\n", disk_idx));
        if (disk_idx == -1)
            break;
        newname[MSATADISK_ENTRY_NAME_LENGTH] = disk_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = disk_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || disk_idx < LowIndex)*/) 
        {
            LowIndex = disk_idx;
            break;
        }
    }
 
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Msatadisk", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[MSATADISK_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Msatadisk", "... get disk stats "));
    DEBUGMSGOID(("header_Msatadisk", name, *length));
    DEBUGMSG(("header_Msatadisk", "\n"));

    return LowIndex;
}
void Init_Msatadisksetting(void)
{
    //init parameter
    msatadisknum = 0;
    curret_msatadiskid = -1;
    curret_msatadiskindex = 0;

	memset(msata_pd_id_ary, 0, sizeof(msata_pd_id_ary));
	enc_id = SE_Get_Cache_Enc_Id();
    msatadisknum = PD_Enumerate(enc_id, msata_pd_id_ary, MAX_PD_NUM, NULL, NULL);

}


int Get_Next_Msatadisk(void)
{  
    if (msatadisknum > 0 && curret_msatadiskindex < msatadisknum)
    {
        curret_msatadiskindex++;              
        return curret_msatadiskindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
