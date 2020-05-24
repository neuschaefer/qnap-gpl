#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Diskperformance.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 

typedef struct _vol_info_ref 
{
    VOL_INFO vol_info;
    struct _vol_info_ref *nextP;
} VOL_INFO_REF;

int dm_diskperformancevol_scan_callback(int vol_id, void* contextP)
{
    int ret = -1;
    VOL_INFO_REF** vol_listP = (VOL_INFO_REF**) contextP;
    VOL_INFO_REF* curP = NULL;
    VOL_INFO_REF* preP = NULL;
    VOL_INFO_REF* tmpP = NULL;
    SM_RAID_LEVEL cur_level, pre_level;
    PD_DEV_ID cur_first, pre_first;
    //create a new one. 
    tmpP = calloc(1, sizeof(VOL_INFO_REF));
    if (!tmpP) {
        SM_TRACE_MEMORY();
        // Use -1 to break the enumeration
        return -1;
    }    

    ret = Volume_Get_Info(vol_id, &tmpP->vol_info);

    if (tmpP->vol_info.vol_conf.flag & SM_VOLUME_IS_INVISIBLE) 
    {
        // Skip the invisible one.
        free(tmpP);
        return -2;
    }
    if (FALSE == tmpP->vol_info.vol_conf.internal) 
    {
        // Skip the external volumes.
        free(tmpP);
        return -2;
    }
    SM_VOLUME_TYPE vol_type;
    vol_type = Volume_Get_Type(tmpP->vol_info.vol_conf.vol_id);
            if ((tmpP->vol_info.status == SS_UNINITIALIZE && tmpP->vol_info.vol_conf.raid_conf.raid_id == -1 && tmpP->vol_info.vol_conf.raid_conf.raid_level == RL_SINGLE)
                || (tmpP->vol_info.status == SS_INITIALIZING && tmpP->vol_info.vol_conf.raid_conf.raid_id == -1 && tmpP->vol_info.vol_conf.raid_conf.raid_level == RL_SINGLE)
                || tmpP->vol_info.status == SS_GLOBAL_SPARE || vol_type == SM_VT_CACHE_LV)
                {
                    free(tmpP);
                    return -2;   
                }
    
    
    cur_first = tmpP->vol_info.vol_conf.raid_conf.data_drives[0];
    cur_level = tmpP->vol_info.vol_conf.raid_conf.raid_level;
    cur_first |= RAID_IS_ARRGEGATE(cur_level) ? 0x00FF0000 : 0;
    
    preP = curP = *vol_listP;
    
    while (curP) {           
        pre_first = curP->vol_info.vol_conf.raid_conf.data_drives[0];
        pre_level = curP->vol_info.vol_conf.raid_conf.raid_level;
        pre_first |= RAID_IS_ARRGEGATE(pre_level) ? 0x00FF0000 : 0;
    
        if (pre_first > cur_first) {
            if (curP == *vol_listP)
                *vol_listP = tmpP;
            else
                preP->nextP = tmpP;

            tmpP->nextP = curP;
            return 0;
        }

        preP = curP;

        curP = curP->nextP;
    }
    
    // Link to the next or set back for the first time. Note that curP must be NULL after while loop.
    if (preP) 
        preP->nextP = tmpP;
    else 
        *vol_listP = tmpP;          
    
    return 0;
}
            
#define	SNMP_DISKPERFORMANCEINDEX		    1
#define	SNMP_DISKPERFORMANCEID		        2
#define	SNMP_DISKPERFORMANCEIOPS		3
#define	SNMP_DISKPERFORMANCELATENCY		4

#define MAX_VOLUME_NUMBER 128

static int *volume_id_aryP = NULL;
static int volumenum = 0;
static int curret_volumeid = -1;
static int curret_volumeindex = 0;


struct variable4 diskperformance_variables[] = {
    {SNMP_DISKPERFORMANCEINDEX, ASN_INTEGER,RONLY,
     var_Diskperformance, 2, {1,1}},
    {SNMP_DISKPERFORMANCEID, ASN_INTEGER, RONLY,
     var_Diskperformance, 2, {1,2}},
    {SNMP_DISKPERFORMANCEIOPS, ASN_INTEGER,RONLY,
     var_Diskperformance, 2, {1,3}},
    {SNMP_DISKPERFORMANCELATENCY, ASN_INTEGER, RONLY,
     var_Diskperformance, 2, {1,4}}
};

oid DiskperformanceNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,11,5,6,1 };
oid diskperformance_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,11,5,6,2 };

int handle_DiskperformanceNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    VOL_INFO_REF *volume_list = NULL;
    VOL_INFO_REF *volume_next = NULL;
    int value = Volume_Enumerate(NULL, 0, dm_diskperformancevol_scan_callback, &volume_list);    

    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskperformanceNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Diskperformance(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("DiskperformanceNumber", handle_DiskperformanceNumber, DiskperformanceNumber_oid, OID_LENGTH(DiskperformanceNumber_oid), HANDLER_CAN_RONLY));
    volumenum = 0;
    curret_volumeid = -1;
    curret_volumeindex = 0;
    REGISTER_MIB("qnapNAS/Diskperformance", diskperformance_variables, variable4, diskperformance_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Diskperformance:init", "initializing table DiskperformanceTable\n"));
}

u_char* var_Diskperformance(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int volume_idx;
    DEBUGMSGTL(("qnapNAS/Diskperformance:var_Diskperformance", "var_Diskperformance start\n"));
    
    volume_idx = header_Diskperformance(vp, name, length, exact, var_len, write_method);
    if (volume_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_DISKPERFORMANCEINDEX:
            long_return = volume_idx;
            return (u_char *) & long_return;
        case SNMP_DISKPERFORMANCEID:
            long_return = curret_volumeid;
            return (u_char *) & long_return;
        case SNMP_DISKPERFORMANCEIOPS:
        {
            BLK_Perf *vol_performanceP = NULL;
            vol_performanceP = calloc(1, sizeof(BLK_Perf));
            NAS_Volume_Get_Perf(curret_volumeid, vol_performanceP);
            long_return = vol_performanceP->iops;
            free(vol_performanceP);
            return (u_char *) & long_return;
        } 
        case SNMP_DISKPERFORMANCELATENCY:
        {
            BLK_Perf *vol_performanceP = NULL;
            vol_performanceP = calloc(1, sizeof(BLK_Perf));
            NAS_Volume_Get_Perf(curret_volumeid, vol_performanceP);
            long_return = vol_performanceP->latency;
            free(vol_performanceP);
            return (u_char *) & long_return;
        }         
        default:
            DEBUGMSGTL(("var_Diskperformance", "unknown sub-id %d in var_Diskperformance\n",
                        vp->magic));
    }
    return NULL;
}

int header_Diskperformance(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define DISKPERFORMANCE_ENTRY_NAME_LENGTH	16
    oid             newname[MAX_OID_LEN];
    int             volume_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Diskperformance", "start header_Diskperformance: "));
    DEBUGMSGOID(("header_Diskperformance", name, *length));
    DEBUGMSG(("header_Diskperformance", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Diskperformancesetting();
    for (;;) 
    {
        volume_idx = Get_Next_Diskperformance();
        DEBUGMSGTL(("header_Diskperformance", "... index %d\n", volume_idx));
        if (volume_idx == -1)
            break;
        newname[DISKPERFORMANCE_ENTRY_NAME_LENGTH] = volume_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = volume_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || volume_idx < LowIndex)*/) 
        {
            LowIndex = volume_idx;
            break;
        }
    }
    if(volume_id_aryP != NULL)
    {
        free(volume_id_aryP);
        volume_id_aryP = NULL;
    }
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Diskperformance", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[DISKPERFORMANCE_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Diskperformance", "... get diskperformance stats "));
    DEBUGMSGOID(("header_Diskperformance", name, *length));
    DEBUGMSG(("header_Diskperformance", "\n"));

    return LowIndex;
}
void Init_Diskperformancesetting(void)
{
    //init parameter
    volumenum = 0;
    curret_volumeid = -1;
    curret_volumeindex = 0;
    
    //init volume array and volume num
    if(volume_id_aryP != NULL)
    {
        free(volume_id_aryP);
        volume_id_aryP = NULL;
    }
    volume_id_aryP = calloc(MAX_VOLUME_NUMBER, sizeof(int));
    memset(volume_id_aryP, 0, MAX_VOLUME_NUMBER * sizeof(int));

    VOL_INFO_REF *volume_list = NULL;
    VOL_INFO_REF *volume_next = NULL;
    VOL_INFO *volume_infoP = NULL;        
    volumenum = Volume_Enumerate(NULL, 0, dm_diskperformancevol_scan_callback, &volume_list);    

    int idx = 0;
    while (volume_list) 
    {
        volume_infoP = &volume_list->vol_info;
        volume_id_aryP[idx++] = volume_infoP->vol_conf.vol_id;
        volume_next = volume_list->nextP;
        free(volume_list);
        volume_list = volume_next;
    }    
}


int Get_Next_Diskperformance(void)
{     
    if (volumenum > 0 && curret_volumeindex < volumenum)
    {
        curret_volumeid = volume_id_aryP[curret_volumeindex];
        curret_volumeindex++;
        return curret_volumeindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}

