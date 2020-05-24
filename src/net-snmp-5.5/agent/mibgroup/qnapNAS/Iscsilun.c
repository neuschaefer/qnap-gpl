#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Iscsilun.h"
#include "common.h"

#include "../../../NasLib/include/cfg_iscsi_lio.h"


#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "storage_man.h"
#endif 

           
#define	SNMP_ISCSILUNINDEX		    1
#define	SNMP_ISCSILUNID		        2
#define	SNMP_ISCSILUNCAPACITY  		3
#define	SNMP_ISCSILUNUSEDPERCENT	4
#define	SNMP_ISCSILUNSTATUS		    5
#define	SNMP_ISCSILUNNAME    		6
#define	SNMP_ISCSILUNBACKUPSTATUS	7
#define	SNMP_ISCSILUNISMAP  		8


/**
 * @fn int CFG_Get_LUN_Backup_Status_By_Index()
 * @brief Retrieve default Initiator index.
 *
 * @param[in] serial_numP buffer of serial number.
 * @retval >=0 Success.
 * @retval < 0 Failure.
 */
int CFG_Get_LUN_Backup_Status_By_Index(int index, LUN_BAK_STATUS *lun_bak_status)
{    
    int ret = -1;
    char tmp_buf[40];
    char lun_sec_name[MAX_NAME_SIZE];

    snprintf(lun_sec_name, sizeof(lun_sec_name), CFG_LUN_INFOI, index);

    if (Ini_Conf_Get_Field(ISCSI_CFG_PATH, lun_sec_name, "LUNBackupStatus", tmp_buf, sizeof(tmp_buf)) >= 0) {
        *lun_bak_status = atoi(tmp_buf);
        ret = 0;
    } else
    {
        *lun_bak_status = 0;
        ret = -1;
    }
    return ret;
}

int *iscsilun_id_aryP = NULL;
int iscsilunnum = 0;
int curret_iscsilunid = -1;
int curret_iscsilunindex = 0;
static struct counter64 counter64_return;
static char string[256]={};

struct variable4 iscsilun_variables[] = {
    {SNMP_ISCSILUNINDEX, ASN_INTEGER,RONLY,
     var_Iscsilun, 2, {1,1}},
    {SNMP_ISCSILUNID, ASN_INTEGER, RONLY,
     var_Iscsilun, 2, {1,2}},
    {SNMP_ISCSILUNCAPACITY, ASN_COUNTER64, RONLY,
     var_Iscsilun, 2, {1,3}},
    {SNMP_ISCSILUNUSEDPERCENT, ASN_INTEGER, RONLY,
     var_Iscsilun, 2, {1,4}},
    {SNMP_ISCSILUNSTATUS, ASN_OCTET_STR, RONLY,
     var_Iscsilun, 2, {1,5}},
    {SNMP_ISCSILUNNAME, ASN_OCTET_STR,RONLY,
     var_Iscsilun, 2, {1,6}}, 
    {SNMP_ISCSILUNBACKUPSTATUS, ASN_INTEGER, RONLY,
     var_Iscsilun, 2, {1,7}},
    {SNMP_ISCSILUNISMAP, ASN_INTEGER,RONLY,
     var_Iscsilun, 2, {1,8}}  
};

oid IscsilunNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,2,1,10,1 };
oid iscsilun_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,2,1,10,2 };

int handle_IscsilunNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    int value = portalP->lun_count;
    free(portalP);
  
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_IscsilunNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Iscsilun(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("IscsilunNumber", handle_IscsilunNumber, IscsilunNumber_oid, OID_LENGTH(IscsilunNumber_oid), HANDLER_CAN_RONLY));
    iscsilunnum = 0;
    curret_iscsilunid = -1;
    curret_iscsilunindex = 0;
    REGISTER_MIB("qnapNAS/Iscsilun", iscsilun_variables, variable4, iscsilun_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Iscsilun:init", "initializing table IscsilunTable\n"));
}

u_char* var_Iscsilun(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int iscsilun_idx;
    DEBUGMSGTL(("qnapNAS/Iscsilun:var_Iscsilun", "var_Iscsilun start\n"));
    
    iscsilun_idx = header_Iscsilun(vp, name, length, exact, var_len, write_method);
    if (iscsilun_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_ISCSILUNINDEX:
            long_return = iscsilun_idx;
            return (u_char *) & long_return;
        case SNMP_ISCSILUNID:
        {
            //long_return = iscsilun_id_aryP[iscsilun_idx-1];
            long_return = curret_iscsilunid;
            return (u_char *) & long_return;
        }
        break;
        case SNMP_ISCSILUNCAPACITY:
        {
            unsigned long long ullunSize = 0;        
            lun_info *lunP = calloc(1, sizeof(lun_info));
            CFG_Get_LUN_Info_By_Index(iscsilun_id_aryP[iscsilun_idx-1], lunP);            
            ullunSize = lunP->lun_capacity  * 1024 * 1024;
            free(lunP); 
            counter64_return.low = ullunSize & 0xffffffff;
            counter64_return.high = ullunSize >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        break;
        case SNMP_ISCSILUNUSEDPERCENT:
        {   
            int lunUsePercent = 0;
            lun_info *lunP = calloc(1, sizeof(lun_info));
            CFG_Get_LUN_Info_By_Index(iscsilun_id_aryP[iscsilun_idx-1], lunP);

            char lun_sec_name[MAX_NAME_SIZE];
            char vol_base[8] = {0},iscsi_service[8] = {0};
            
            //get volume base
            snprintf(lun_sec_name, sizeof(lun_sec_name), CFG_LUN_INFOI, iscsilun_id_aryP[iscsilun_idx-1]);
            Ini_Conf_Get_Field(ISCSI_CFG_PATH, lun_sec_name, "bEnableVolumeBase", vol_base, sizeof(vol_base));

            //get service
            snprintf(lun_sec_name, sizeof(lun_sec_name), CFG_PORTAL_SECTION);
            Ini_Conf_Get_Field(ISCSI_CFG_PATH, lun_sec_name, "bServiceEnable", iscsi_service, sizeof(iscsi_service));
             
            if (!strcasecmp(vol_base, "TRUE")) //block base
            {
                int vol_id = Volume_Get_Id_By_Base_Name(lunP->lun_path);
                int lv_id = Volume_Get_LV_ID(vol_id);
                unsigned long long used_size;
                unsigned long long capacity;
                
                if (!LV_Get_Size(lv_id, &capacity) && !LV_Get_Real_Size(lv_id, &used_size)) 
                {
                    if (used_size == 0ULL)
                    {
                        lunUsePercent = 0;
                    }
                    else if (used_size == capacity)
                    {
                        lunUsePercent = 100;
                    }
                    else
                    {
                        if(used_size >= capacity)
                        {
                            lunUsePercent = 100;
                        }
                        else
                        {
                            lunUsePercent = (int) (used_size * 100 / capacity);
                        }
                    }
                }
                else
                {
                    lunUsePercent = -1;
                }             
            }
            else                                //file base
            {
                if (!strcasecmp(iscsi_service, "TRUE")) 
                {
                    if (lunP->lun_allocate == 0)
                    {
                        lunUsePercent = 0;
                    }                            

                    else if ((lunP->lun_allocate >> 20) == lunP->lun_capacity)
                    {
                        lunUsePercent = 100;
                    }
                    else if ((int)(100 * (lunP->lun_allocate >> 20) / lunP->lun_capacity) > 100)
                    {
                        lunUsePercent = 100;
                    }
                    else
                    {
                        lunUsePercent = (int)(100 * (lunP->lun_allocate >> 20) / lunP->lun_capacity);
                    }
                } 
                else  
                {
                    lunUsePercent = -1;
                }                
            }
        

            free(lunP); 
            long_return = lunUsePercent;
            return (u_char *) & long_return;                
        }
        break;        
        case SNMP_ISCSILUNSTATUS:
        {
            lun_info *lunP = calloc(1, sizeof(lun_info));
            CFG_Get_LUN_Info_By_Index(iscsilun_id_aryP[iscsilun_idx-1], lunP);
                
            int status = lunP->lun_status;
            int enable = 0;
            if(lunP->lun_target_count == 0)
            {
                enable = 0;
            }
            else
            {
                enable = lunP->lun_enable_ary[0];
            }
            
            if(status == -1)
            {
                snprintf(string, sizeof(string), "%s", "Error");
            }
            else if(status == 0)
            {
                snprintf(string, sizeof(string), "Processing(%s)", lunP->lun_op_percent);
            }
            else if(status == 1)
            {
                if(CFG_Get_LUN_Removing(iscsilun_id_aryP[iscsilun_idx-1]) == 1)
                {
#ifdef QTS_SNAPSHOT
                    float vol_removing_percent = 0;
                    int vol_id = Volume_Get_Id_By_Base_Name(lunP->lun_meta_path);
                    if(vol_id >= 0)
                    {
                        NAS_Snapshot_Get_Volume_Remove_Percent(vol_id, &vol_removing_percent);
                    }
                    snprintf(string, sizeof(string), "Removing...%.2f %%", vol_removing_percent);
#else
                    snprintf(string, sizeof(string), "Removing...");
#endif
                }
                else
                {
                    snprintf(string, sizeof(string), "%s", "Ready");
                }                    
            }
            else if(status == 2)
            {
                if(enable == 1)
                {
                    snprintf(string, sizeof(string), "%s", "Enabled");
                }
                else
                {
                    snprintf(string, sizeof(string), "%s", "Disabled");
                }
            }    
            else
            {
                snprintf(string, sizeof(string), "%s", "Unknown Error");                
            }            
            free(lunP);
            *var_len = strlen(string);
            return (unsigned char *) string;       
        }  
        break;        
        case SNMP_ISCSILUNNAME:
        {   
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            lun_info *lunP = calloc(1, sizeof(lun_info));
            CFG_Get_LUN_Info_By_Index(iscsilun_id_aryP[iscsilun_idx-1], lunP);            
            snprintf(string, sizeof(string), "%s",lunP->lun_name);
            free(lunP);
            *var_len = strlen(string);
            return (unsigned char *) string;            
        }
        break;
        case SNMP_ISCSILUNBACKUPSTATUS:
        {   
            int backupstatus = -1;    
            if(0 == CFG_Get_LUN_Backup_Status_By_Index(iscsilun_id_aryP[iscsilun_idx-1], &backupstatus))
            {
                long_return = backupstatus;
            }
            else
            {
                long_return = 0;
            }
            return (u_char *) & long_return;           
        }
        break;
        case SNMP_ISCSILUNISMAP:
        {   
            lun_info *lunP = calloc(1, sizeof(lun_info));
            CFG_Get_LUN_Info_By_Index(iscsilun_id_aryP[iscsilun_idx-1], lunP);            
            long_return = (lunP->lun_target_count > 0)?1:0;
            free(lunP);
            return (u_char *) & long_return;             
        }
        break;      
     
        default:
            DEBUGMSGTL(("var_Iscsilun", "unknown sub-id %d in var_Iscsilun\n",
                        vp->magic));
    }
    return NULL;
}

int header_Iscsilun(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define ISCSILUN_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             iscsilun_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Iscsilun", "start header_Iscsilun: "));
    DEBUGMSGOID(("header_Iscsilun", name, *length));
    DEBUGMSG(("header_Iscsilun", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Iscsilunsetting();
    for (;;) 
    {
        iscsilun_idx = Get_Next_Iscsilun();
        DEBUGMSGTL(("header_Iscsilun", "... index %d\n", iscsilun_idx));
        if (iscsilun_idx == -1)
            break;
        newname[ISCSILUN_ENTRY_NAME_LENGTH] = iscsilun_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = iscsilun_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || iscsilun_idx < LowIndex)*/) 
        {
            LowIndex = iscsilun_idx;
            break;
        }
    }
    // if(iscsilun_id_aryP != NULL)
    // {
        // free(iscsilun_id_aryP);
        // iscsilun_id_aryP = NULL;
    // }
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Iscsilun", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[ISCSILUN_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Iscsilun", "... get iscsilun stats "));
    DEBUGMSGOID(("header_Iscsilun", name, *length));
    DEBUGMSG(("header_Iscsilun", "\n"));

    return LowIndex;
}
void Init_Iscsilunsetting(void)
{
    //init parameter
    //iscsilunnum = 0;
    curret_iscsilunid = -1;
    curret_iscsilunindex = 0;
 
    if(iscsilun_id_aryP != NULL)
    {
        free(iscsilun_id_aryP);
        iscsilun_id_aryP = NULL;
    }
 
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    iscsilunnum = portalP->lun_count;


    if(iscsilunnum > 0)
    {
        iscsilun_id_aryP = calloc(iscsilunnum+1, sizeof(int));
        //memset(iscsilun_id_aryP, 0, (iscsilunnum+1) * sizeof(int));    
        int i = 0;
        for(i = 0; i < portalP->lun_count; i++)
        {
            iscsilun_id_aryP[i] = (int)portalP->lun_ary[i];
        }
    }
    free(portalP);
}


int Get_Next_Iscsilun(void)
{     
    if (iscsilunnum > 0 && curret_iscsilunindex < iscsilunnum)
    {
        curret_iscsilunid = iscsilun_id_aryP[curret_iscsilunindex];
        curret_iscsilunindex++;
        return curret_iscsilunindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}

