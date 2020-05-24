#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Cacheacceleration.h"
#include "common.h"

#include "../../../NasLib/include/cfg_iscsi_lio.h"
#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 

static struct counter64 counter64_return;
static char string[256]={};

typedef struct _vol_info_ref 
{
    VOL_INFO vol_info;
    struct _vol_info_ref *nextP;
} VOL_INFO_REF;

int dm_vol_scan_callback(int vol_id, void* contextP)
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

    SM_VOLUME_TYPE vol_type;
    vol_type = Volume_Get_Type(vol_id);
    if (vol_type != SM_VT_CACHE_LV)  
    {
		if(tmpP){
			free(tmpP);
			tmpP=NULL;
		}
        return -2;
    }
    
    ret = Volume_Get_Info(vol_id, &tmpP->vol_info);

  
    
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


int cachevolenabled() 
{
    int volumeid = getCacheVolumeID();
    
    if(volumeid == -1)
    {
        return 0;
    }
    else
    {
        return NAS_CacheVolume_Is_Enabled(volumeid);
    }
}

int checkcachevolstatus(int cachevolid)
{
    VOL_INFO volinfo;
    VOL_CONF *vol_confP = NULL;    
    int ret = Volume_Get_Info(cachevolid, &volinfo);
    vol_confP = &(volinfo.vol_conf); 

    if(vol_confP->status != SS_READY && vol_confP->status != SS_REMOVING_VOLUME && vol_confP->status != SS_UNINITIALIZE && vol_confP->status != SS_DEGRADED)
    {
        return 0;
    }
    return 1;
}

oid Cacheaccelerationservice_oid[] =             { 1,3,6,1,4,1,24681,1,4,1,1,1,3,1 };
oid Cacheaccelerationavailablepercent_oid[] =            { 1,3,6,1,4,1,24681,1,4,1,1,1,3,2 };
oid Cacheaccelerationreadhitrate_oid[] =         { 1,3,6,1,4,1,24681,1,4,1,1,1,3,3 };
oid Cacheaccelerationwritehitrate_oid[] =        { 1,3,6,1,4,1,24681,1,4,1,1,1,3,4 };
oid Cacheaccelerationstatus_oid[] =              { 1,3,6,1,4,1,24681,1,4,1,1,1,3,5 };

int getCacheVolumeID()
{
    int cachevolumeid = -1;
    VOL_INFO_REF *vol_list = NULL;
    VOL_INFO_REF *vol_next = NULL;
    int count = Volume_Enumerate(NULL, 0, dm_vol_scan_callback, &vol_list);

    //volume_id_aryP = calloc(MAX_VOLUME_NUMBER, sizeof(int));
    //memset(volume_id_aryP, 0, MAX_VOLUME_NUMBER * sizeof(int));

    int idx = 0;
    while (vol_list) 
    {
        cachevolumeid = vol_list->vol_info.vol_conf.vol_id;
        //volume_id_aryP[idx++] = vol_list->vol_info.vol_conf.vol_id;
        vol_next = vol_list->nextP;
        free(vol_list);
        vol_list = vol_next;
    } 
    return cachevolumeid;
}
int handle_Cacheaccelerationservice(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int value = cachevolenabled();
    
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

int handle_Cacheaccelerationavailablepercent(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int availablepercent = 0,percent;
    int cachevolid = getCacheVolumeID();
    if(cachevolid != -1 && checkcachevolstatus(cachevolid))
    {  
        unsigned long long capacity;
        unsigned long long used_size;
        unsigned long long dirty_size;
        unsigned long long capacity_block;
        
        int lv_id = Volume_Get_LV_ID(cachevolid);
        if (LV_Get_Size(lv_id, &capacity)) 
           capacity = 0ULL;
        
        if (NAS_CacheVolume_Is_Enabled(cachevolid) && !NAS_CacheVolume_Get_Block(cachevolid, &capacity_block, &used_size, &dirty_size)) 
        {
             if (used_size == 0ULL) 
             {
                availablepercent = 100;
             }
             else
             {
                if (capacity_block > 0)
                {
                    percent = used_size * 100 / capacity_block;
                }
                else
                {
                    if (capacity != 0ULL)
                    {
                        percent = used_size * DEFAULT_CACHE_BLOCK_SIZE * 100 / capacity;
                    }
                    else
                    {
                        percent = 100;//if capacity is 0 , set used all
                    }
                }
                availablepercent = 100 - percent;
             }
             
        }
        else
        {
            availablepercent = 0;
        }
    }
    else//if cache is no enabled 
    {
        availablepercent = 0;
    }


    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &availablepercent, sizeof(availablepercent));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskperformanceNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


int handle_Cacheaccelerationreadhitrate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int value = -1;
   
    int hit_read = 0;
    unsigned long long hit_count;
    int cachevolid = getCacheVolumeID();
    if (cachevolid != -1 && checkcachevolstatus(cachevolid) && NAS_CacheVolume_Is_Enabled(cachevolid) && !NAS_CacheVolume_Get_Hit(cachevolid, 1, &hit_count, &hit_read)) 
    {
        value = hit_read;
    }


    
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

int handle_Cacheaccelerationwritehitrate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int value = -1;
     
    int hit_write = 0; 
    unsigned long long hit_count;
    int cachevolid = getCacheVolumeID();   
    if (cachevolid != -1 && checkcachevolstatus(cachevolid) && NAS_CacheVolume_Is_Enabled(cachevolid) && !NAS_CacheVolume_Get_Hit(cachevolid, 0, &hit_count, &hit_write)) 
    {
        int cache_mode = 0;
        if(!NAS_CacheVolume_Get_Cache_Mode(cachevolid, &cache_mode) && cache_mode == 1)
        {
            value = hit_write;
        }
    }

  
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

int handle_Cacheaccelerationstatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{                    
	int str_len=sizeof(string)-1;
	string[str_len] = '\0';

    VOL_INFO volinfo;
    RAID_CONF* raid_confP = NULL;
    VOL_CONF *vol_confP = NULL;
    double raid_progress = 0.0; 
    STORAGE_STATUS status = SS_NONE_STATUS;
    int cachevolid = getCacheVolumeID();  
    if(cachevolid == -1)
    {
        snprintf(string, sizeof(string), "--");
    }
    else
    {    
        int ret = Volume_Get_Info(cachevolid, &volinfo);
        vol_confP = &(volinfo.vol_conf);
        raid_confP = &vol_confP->raid_conf;
        if (!RAID_Get_Status(raid_confP->raid_id, &status, &raid_progress) && status==SS_REBUILDING_RAID) 
        {
            snprintf(string, sizeof(string), "Rebuilding RAID...");
        } 
        else 
        {        
            Volume_Get_Status_String(cachevolid, string, sizeof(string));  
        }
    }
    
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) &string, strlen(string));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskperformanceNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

void init_Cacheacceleration(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("service", handle_Cacheaccelerationservice, Cacheaccelerationservice_oid, OID_LENGTH(Cacheaccelerationservice_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("availablePercent", handle_Cacheaccelerationavailablepercent, Cacheaccelerationavailablepercent_oid, OID_LENGTH(Cacheaccelerationavailablepercent_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("readHitRate", handle_Cacheaccelerationreadhitrate, Cacheaccelerationreadhitrate_oid, OID_LENGTH(Cacheaccelerationreadhitrate_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("writeHitRate", handle_Cacheaccelerationwritehitrate, Cacheaccelerationwritehitrate_oid, OID_LENGTH(Cacheaccelerationwritehitrate_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("status", handle_Cacheaccelerationstatus, Cacheaccelerationstatus_oid, OID_LENGTH(Cacheaccelerationstatus_oid), HANDLER_CAN_RONLY));

                 
    DEBUGMSGTL(("qnapNAS/Volume:init", "initializing table Iscsi Storage\n"));
}
