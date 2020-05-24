#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Raid.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 

#define MAX_RAID_NUMBER  128

  

  
#define	SNMP_RAIDINDEX		    1
#define	SNMP_RAIDID		        2
#define	SNMP_RAIDCAPACITY		3
#define	SNMP_RAIDFREESIZE		4
#define	SNMP_RAIDSTATUS		    5
#define	SNMP_RAIDBITMAP		    6
#define	SNMP_RAIDLEVEL		    7

int *raid_id_aryP = NULL;
int raidnum = 0;
int curret_raidid = -1;
int curret_raidindex = 0;
static struct counter64 counter64_return;
static char string[256]={};


struct variable4 raid_variables[] = {
    {SNMP_RAIDINDEX, ASN_INTEGER,RONLY,
     var_Raid, 2, {1,1}},
    {SNMP_RAIDID, ASN_INTEGER, RONLY,
     var_Raid, 2, {1,2}},
    {SNMP_RAIDCAPACITY, ASN_COUNTER64,RONLY,
     var_Raid, 2, {1,3}},
    {SNMP_RAIDFREESIZE, ASN_COUNTER64, RONLY,
     var_Raid, 2, {1,4}},
    {SNMP_RAIDSTATUS, ASN_OCTET_STR, RONLY,
     var_Raid, 2, {1,5}},
    {SNMP_RAIDBITMAP, ASN_INTEGER, RONLY,
     var_Raid, 2, {1,6}},
    {SNMP_RAIDLEVEL, ASN_OCTET_STR, RONLY,
     var_Raid, 2, {1,7}}     
};

oid RaidNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,1,1 };
oid raid_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,1,2 };

int handle_RaidNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    RAID_REF *raid_list = NULL;
    RAID_REF *raid_next = NULL;
    int value = RAID_Enumerate(NULL, 0, dm_raid_callback, &raid_list);

    while (raid_list) 
    {
        raid_next = raid_list->nextP;
        free(raid_list);
        raid_list = raid_next;
    }
    
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_RaidNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Raid(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("RaidNumber", handle_RaidNumber, RaidNumber_oid, OID_LENGTH(RaidNumber_oid), HANDLER_CAN_RONLY));
    raidnum = 0;
    curret_raidid = -1;
    curret_raidindex = 0;
    REGISTER_MIB("qnapNAS/Raid", raid_variables, variable4, raid_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Raid:init", "initializing table RaidTable\n"));
}

u_char* var_Raid(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int raid_idx;
    DEBUGMSGTL(("qnapNAS/Raid:var_Raid", "var_Raid start\n"));
    
    raid_idx = header_Raid(vp, name, length, exact, var_len, write_method);
    if (raid_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_RAIDINDEX:
            long_return = raid_idx;
            return (u_char *) & long_return;
        case SNMP_RAIDID:
            long_return = curret_raidid;
            return (u_char *) & long_return;
        case SNMP_RAIDCAPACITY:
        {
            unsigned long long ulRaidSize = 0;

            int pool_id = 0;
            if (pool_id = Pool_Get_Id_By_Member_Id(curret_raidid) >= 0) 
            { //Pool RAID
                DEV_MEMBER member;
                Pool_Get_Member_Info(curret_raidid, &member);
                unsigned long long tmp_capacity;
                unsigned long long tmp_available_size;
                if (!Pool_Get_Member_Size(member.dev_name, &tmp_capacity, &tmp_available_size)) 
                {
                    tmp_capacity *= 512;
                    //if (status_ok) 
                    //{
                        ulRaidSize = tmp_capacity;
                    //} 
                } 
            } 
            else 
            { //legacy RAID
                int curret_raidid;
                int vol_id;
                if (!Volume_Get_Vol_ID_By_RAID_ID(curret_raidid, &vol_id)) 
                {
                    VOL_INFO* vol_infoP = calloc(1, sizeof(VOL_INFO));
                    if (!Volume_Get_Info(vol_id, vol_infoP)) 
                    {
                        ulRaidSize = (vol_infoP->total_size) * 1024;   
                    } 
                    free(vol_infoP);
                }
            }            
            
            counter64_return.low = ulRaidSize & 0xffffffff;
            counter64_return.high = ulRaidSize >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_RAIDFREESIZE:
        {
            unsigned long long ulRaidFree = 0;

            int pool_id = 0;
            if (pool_id = Pool_Get_Id_By_Member_Id(curret_raidid) >= 0) 
            { //Pool RAID
                DEV_MEMBER member;
                Pool_Get_Member_Info(curret_raidid, &member);
                unsigned long long tmp_capacity;
                unsigned long long tmp_available_size;
                if (!Pool_Get_Member_Size(member.dev_name, &tmp_capacity, &tmp_available_size)) 
                {
                    tmp_available_size *= 512;
                    ulRaidFree = tmp_available_size;
                } 
            } 
            else 
            { //legacy RAID
                int curret_raidid;
                int vol_id;
                if (!Volume_Get_Vol_ID_By_RAID_ID(curret_raidid, &vol_id)) 
                {
                    VOL_INFO* vol_infoP = calloc(1, sizeof(VOL_INFO));
                    if (!Volume_Get_Info(vol_id, vol_infoP)) 
                    {  
                        ulRaidFree = vol_infoP->free_size * 1024;
                    } 
                    free(vol_infoP);
                }
            }            
            counter64_return.low = ulRaidFree & 0xffffffff;
            counter64_return.high = ulRaidFree >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_RAIDSTATUS:
        {
			int str_len=sizeof(string)-1;
            string[str_len] = '\0';
            RAID_Get_Status_String(curret_raidid, string, sizeof(string));
            *var_len = strlen(string);
            return (unsigned char *) string;  
        }
        case SNMP_RAIDBITMAP:
        {           
			int str_len=sizeof(string)-1;
            string[str_len] = '\0';
            RAID_Get_Level_String(curret_raidid, string, sizeof(string));
            if(strcmp(string, "Single") == 0 || strcmp(string, "0") == 0 || strcmp(string, "JBOD") == 0)
            {
                long_return = -1;
            }
            else
            {
                long_return = RAID_Is_Bitmap_Enabled(curret_raidid);
            }
            return (u_char *) & long_return;
        }
        case SNMP_RAIDLEVEL:
        {
			int str_len=sizeof(string)-1;
            string[str_len] = '\0';
            RAID_Get_Level_String(curret_raidid, string, sizeof(string));
            *var_len = strlen(string);
            return (unsigned char *) string; 
        }        
        default:
            DEBUGMSGTL(("var_Raid", "unknown sub-id %d in var_Raid\n",
                        vp->magic));
    }
    return NULL;
}

int header_Raid(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define RAID_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             raid_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Raid", "start header_Raid: "));
    DEBUGMSGOID(("header_Raid", name, *length));
    DEBUGMSG(("header_Raid", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Raidsetting();
    for (;;) 
    {
        raid_idx = Get_Next_Raid();
        DEBUGMSGTL(("header_Raid", "... index %d\n", raid_idx));
        if (raid_idx == -1)
            break;
        newname[RAID_ENTRY_NAME_LENGTH] = raid_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = raid_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || raid_idx < LowIndex)*/) 
        {
            LowIndex = raid_idx;
            break;
        }
    }
    
    if(raid_id_aryP != NULL)
    {
        free(raid_id_aryP);
        raid_id_aryP = NULL;
    }
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Raid", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[RAID_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Raid", "... get raid stats "));
    DEBUGMSGOID(("header_Raid", name, *length));
    DEBUGMSG(("header_Raid", "\n"));

    return LowIndex;
}
void Init_Raidsetting(void)
{
    //init parameter
    raidnum = 0;
    curret_raidid = -1;
    curret_raidindex = 0;
    
    //init raid array and raid num
    if(raid_id_aryP != NULL)
    {
        free(raid_id_aryP);
        raid_id_aryP = NULL;
    }
    raid_id_aryP = calloc(MAX_RAID_NUMBER, sizeof(int));
    memset(raid_id_aryP, 0, MAX_RAID_NUMBER * sizeof(int));

    RAID_REF *raid_list = NULL;
    RAID_REF *raid_next = NULL;
    RAID_CONF *raid_infoP = NULL;    
    raidnum = RAID_Enumerate(NULL, 0, dm_raid_callback, &raid_list);

    int idx = 0;
    while (raid_list) 
    {
        raid_infoP = &raid_list->md_info;
        raid_id_aryP[idx++] = raid_infoP->raid_id;
        raid_next = raid_list->nextP;
        free(raid_list);
        raid_list = raid_next;
    }    
}


int Get_Next_Raid(void)
{     
    if (raidnum > 0 && curret_raidindex < raidnum)
    {
        curret_raidid = raid_id_aryP[curret_raidindex];
        curret_raidindex++;
        return curret_raidindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
