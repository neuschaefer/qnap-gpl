#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Volume.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 


            
#define	SNMP_VOLUMEINDEX		    1
#define	SNMP_VOLUMEID		        2
#define	SNMP_VOLUMECAPACITY		3
#define	SNMP_VOLUMEFREESIZE		4
#define	SNMP_VOLUMESTATUS		    5
#define SNMP_VOLUMESSDCACHE            6
#define SNMP_VOLUMETHIN                7       
#define SNMP_VOLUMENAME                8
#define MAX_VOLUME_NUMBER 128

static int vol_enumerate_general_volumes_callback(int vol_id, void *contextP)
{
    SM_RAID_LEVEL raid_level;
    int ret, raid_id = -1;
    int child_vol_id, parent_vol_id;
    
    Volume_Get_RAID_Level(vol_id, &raid_level, &raid_id);
    
    ret = Volume_Is_Aggregate_Member(raid_id, &child_vol_id, &parent_vol_id) ? 
          -2 : 0;
    
    return ret;
}

static int vol_enumerate_volumelist_callback(int vol_id, void *contextP)
{
    SM_VOLUME_TYPE volType;
    int ret=0;
    
    if(vol_id == 0)
    {
        ret =  -2;       
        goto END_VOL_EMULATE_VOLUMELIST_CALLBACK;
    }
    volType = Volume_Get_Type(vol_id);

    if (volType != SM_VT_DATA_LV && volType != SM_VT_MD )
    {
        ret =  -2;
        goto END_VOL_EMULATE_VOLUMELIST_CALLBACK;
    }
    else if(vol_enumerate_general_volumes_callback( vol_id, contextP) == -2)
    {
        ret = -2;
        goto END_VOL_EMULATE_VOLUMELIST_CALLBACK;
    }

    //check if global spare
    STORAGE_STATUS status = SS_UNINITIALIZE;
    double progress = 0.0;
    Volume_Get_Status(vol_id, &status, &progress);
    if(status == 12)
    {
        ret = -2;  
        goto END_VOL_EMULATE_VOLUMELIST_CALLBACK;
    }
    
END_VOL_EMULATE_VOLUMELIST_CALLBACK:
    return ret;
}

int *volume_id_aryP = NULL;
int volumenum = 0;
int curret_volumeid = -1;
int curret_volumeindex = 0;
static struct counter64 counter64_return;
static char string[256]={};

struct variable4 volume_variables[] = {
    {SNMP_VOLUMEINDEX, ASN_INTEGER,RONLY,
     var_Volume, 2, {1,1}},
    {SNMP_VOLUMEID, ASN_INTEGER, RONLY,
     var_Volume, 2, {1,2}},
    {SNMP_VOLUMECAPACITY, ASN_COUNTER64,RONLY,
     var_Volume, 2, {1,3}},
    {SNMP_VOLUMEFREESIZE, ASN_COUNTER64, RONLY,
     var_Volume, 2, {1,4}},
    {SNMP_VOLUMESTATUS, ASN_OCTET_STR, RONLY,
     var_Volume, 2, {1,5}},
    {SNMP_VOLUMESSDCACHE, ASN_INTEGER,RONLY,
     var_Volume, 2, {1,6}},
    {SNMP_VOLUMETHIN, ASN_INTEGER, RONLY,
     var_Volume, 2, {1,7}},
    {SNMP_VOLUMENAME, ASN_OCTET_STR,RONLY,
     var_Volume, 2, {1,8}}     
};

oid VolumeNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,3,1 };
oid volume_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,3,2 };

int handle_VolumeNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    int *local_volume_id_aryP = NULL;
    // if(local_volume_id_aryP != NULL)
    // {
        // free(local_volume_id_aryP);
    // }
    local_volume_id_aryP = calloc(MAX_VOLUME_NUMBER, sizeof(int));
    memset(local_volume_id_aryP, 0, MAX_VOLUME_NUMBER * sizeof(int));
    // value = Volume_Enumerate_Data_Volumes(local_volume_id_aryP, MAX_VOLUME_NUMBER);
    int value = Volume_Enumerate(local_volume_id_aryP, MAX_VOLUME_NUMBER, vol_enumerate_volumelist_callback, NULL);        
    free(local_volume_id_aryP);
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_VolumeNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Volume(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("VolumeNumber", handle_VolumeNumber, VolumeNumber_oid, OID_LENGTH(VolumeNumber_oid), HANDLER_CAN_RONLY));
    volumenum = 0;
    curret_volumeid = -1;
    curret_volumeindex = 0;
    REGISTER_MIB("qnapNAS/Volume", volume_variables, variable4, volume_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Volume:init", "initializing table VolumeTable\n"));
}

u_char* var_Volume(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int volume_idx;
    DEBUGMSGTL(("qnapNAS/Volume:var_Volume", "var_Volume start\n"));
    
    volume_idx = header_Volume(vp, name, length, exact, var_len, write_method);
    if (volume_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_VOLUMEINDEX:
            long_return = volume_idx;
            return (u_char *) & long_return;
        case SNMP_VOLUMEID:
            long_return = curret_volumeid;
            return (u_char *) & long_return;
        case SNMP_VOLUMECAPACITY:
        {
            unsigned long long ulVolumeSize = 0;   
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Status_String(curret_volumeid, string, sizeof(string));
            if(strcmp(string, "Locked") == 0)
            {
                ulVolumeSize = 0;
            }
            else
            {
                VOL_INFO* vol_infoP = calloc(1, sizeof(VOL_INFO));    
                Volume_Get_Info(curret_volumeid, vol_infoP);
                ulVolumeSize = vol_infoP->total_size;              
                free(vol_infoP);

            }            
            counter64_return.low = ulVolumeSize & 0xffffffff;
            counter64_return.high = ulVolumeSize >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_VOLUMEFREESIZE:
        {
            unsigned long long ulVolumeFree = 0;
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Status_String(curret_volumeid, string, sizeof(string));
            if(strcmp(string, "Locked") == 0)
            {
                ulVolumeFree = 0;
            }
            else
            {            
                VOL_INFO* vol_infoP = calloc(1, sizeof(VOL_INFO));    
                Volume_Get_Info(curret_volumeid, vol_infoP);
                ulVolumeFree = vol_infoP->free_size;              
                free(vol_infoP);
            }            
            counter64_return.low = ulVolumeFree & 0xffffffff;
            counter64_return.high = ulVolumeFree >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_VOLUMESTATUS:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Status_String(curret_volumeid, string, sizeof(string));  

            *var_len = strlen(string);
            return (unsigned char *) string;   
        }
        case SNMP_VOLUMESSDCACHE:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Status_String(curret_volumeid, string, sizeof(string));
			 SM_VOLUME_TYPE vol_type = Volume_Get_Type(curret_volumeid);
            if(strcmp(string, "Locked") == 0 ||  vol_type == SM_VT_MD/*legacy volume*/ || vol_type == SM_VT_UNKNOWN)
            {
                long_return = -1;
            }
            else
            {  
                VOL_CONF *vol_confP = calloc(1, sizeof(VOL_CONF));
                Volume_Get_Conf_V2(curret_volumeid, vol_confP, NULL);
                long_return = VOLUME_USE_SSD_CACHE(vol_confP->flag);
                free(vol_confP);  
            }            
            return (u_char *) & long_return;            
        }
        break;
        case SNMP_VOLUMETHIN:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Status_String(curret_volumeid, string, sizeof(string));
            if(strcmp(string, "Locked") == 0)
            {
                long_return = -1;
            }
            else
            { 
                int lv_id = Volume_Get_LV_ID(curret_volumeid);
                LV_TYPE lv_type = LV_Get_Type(lv_id);
                long_return = (lv_type == LT_THIN)? 1 : 0;
            }
            return (u_char *) & long_return;              
        }              
        case SNMP_VOLUMENAME:
        {   
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            Volume_Get_Label(curret_volumeid, string, sizeof(string));  

            *var_len = strlen(string);
            return (unsigned char *) string;            
        }   
        default:
            DEBUGMSGTL(("var_Volume", "unknown sub-id %d in var_Volume\n",
                        vp->magic));
    }
    return NULL;
}

int header_Volume(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define VOLUME_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             volume_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Volume", "start header_Volume: "));
    DEBUGMSGOID(("header_Volume", name, *length));
    DEBUGMSG(("header_Volume", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Volumesetting();
    for (;;) 
    {
        volume_idx = Get_Next_Volume();
        DEBUGMSGTL(("header_Volume", "... index %d\n", volume_idx));
        if (volume_idx == -1)
            break;
        newname[VOLUME_ENTRY_NAME_LENGTH] = volume_idx;
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
        DEBUGMSGTL(("header_Volume", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[VOLUME_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Volume", "... get volume stats "));
    DEBUGMSGOID(("header_Volume", name, *length));
    DEBUGMSG(("header_Volume", "\n"));

    return LowIndex;
}
void Init_Volumesetting(void)
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

    //volumenum = Volume_Enumerate_Data_Volumes(volume_id_aryP, MAX_VOLUME_NUMBER);  
    volumenum = Volume_Enumerate(volume_id_aryP, MAX_VOLUME_NUMBER, vol_enumerate_volumelist_callback, NULL);    
}


int Get_Next_Volume(void)
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

