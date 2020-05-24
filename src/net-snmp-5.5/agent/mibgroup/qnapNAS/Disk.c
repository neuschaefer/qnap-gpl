#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Disk.h"
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



#define	SNMP_DISKINDEX		    1
#define	SNMP_DISKID		        2
#define	SNMP_DISKENCLOSUREID	3
#define	SNMP_DISKSUMMARY         4
#define	SNMP_DISKSMARTINFO     	5
#define	SNMP_DISKTEMP     	    6
#define	SNMP_DISKGLOBALSPARE    7
#define	SNMP_DISKMODEL       	8
#define	SNMP_DISKCAPACITY 	    9

int disknum = 0;
int curret_diskid = -1;
int curret_diskindex = 0;
static char string[256]={};
static struct counter64 counter64_return;

int pd_id_ary[MAX_PD_NUM];
int enc_id_ary[MAX_SE_NUM];
typedef struct {
	int diskid;	
	int diskencid;	
} SNMPDISK_INFO;




SNMPDISK_INFO * sysdisk_listP = NULL;


struct variable4 disk_variables[] = {
    {SNMP_DISKINDEX, ASN_INTEGER,RONLY,
     var_Disk, 2, {1,1}},
    {SNMP_DISKID, ASN_INTEGER, RONLY,
     var_Disk, 2, {1,2}},
    {SNMP_DISKENCLOSUREID, ASN_INTEGER,RONLY,
     var_Disk, 2, {1,3}},
    {SNMP_DISKSUMMARY, ASN_OCTET_STR,RONLY,
     var_Disk, 2, {1,4}},
    {SNMP_DISKSMARTINFO, ASN_INTEGER,RONLY,
     var_Disk, 2, {1,5}},
    {SNMP_DISKTEMP, ASN_INTEGER,RONLY,
     var_Disk, 2, {1,6}},
    {SNMP_DISKGLOBALSPARE, ASN_INTEGER,RONLY,
     var_Disk, 2, {1,7}},
    {SNMP_DISKMODEL, ASN_OCTET_STR,RONLY,
     var_Disk, 2, {1,8}},
    {SNMP_DISKCAPACITY, ASN_COUNTER64,RONLY,
     var_Disk, 2, {1,9}}       
};

oid DiskNumber_oid[] =     { 1,3,6,1,4,1,24681,1,4,1,1,1,1,5,1 };
oid disk_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,5,2 };

int handle_DiskNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{

    memset(enc_id_ary, 0, sizeof(enc_id_ary));

    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, enclosure_list_callback, NULL);

	disknum = 0;
	int i = 0;
    for(i = 0; i < se_count; i++)
    {
        disknum += PD_Enumerate(enc_id_ary[i], NULL, 0, NULL, NULL);
    }        
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &disknum, sizeof(disknum));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}



void init_Disk(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("DiskNumber", handle_DiskNumber, DiskNumber_oid, OID_LENGTH(DiskNumber_oid), HANDLER_CAN_RONLY));


    disknum = 0;
    curret_diskid = -1;
    curret_diskindex = 0;
    REGISTER_MIB("qnapNAS/Disk", disk_variables, variable4, disk_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Disk:init", "initializing table DiskTable\n"));
}

u_char* var_Disk(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int disk_idx;
    DEBUGMSGTL(("qnapNAS/Disk:var_Disk", "var_Disk start\n"));
    
    disk_idx = header_Disk(vp, name, length, exact, var_len, write_method);
    if (disk_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_DISKINDEX:
            long_return = disk_idx;
            return (u_char *) & long_return;
        case SNMP_DISKID:
            long_return = sysdisk_listP[disk_idx-1].diskid;
            return (u_char *) & long_return;
        case SNMP_DISKENCLOSUREID:
        {           
            long_return = sysdisk_listP[disk_idx-1].diskencid;
            return (u_char *) & long_return;
        }
        case SNMP_DISKSUMMARY:
        { 
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            //get hd_status
            PD_DEV_ID dev_id; 
            dev_id = PD_MAKE_DEV_ID(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid);
            STORAGE_STATUS disk_status;  
            NAS_Disk_Get_Status(dev_id, &disk_status); 
 
            
            //get smart status
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid, &summary);            
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
        case SNMP_DISKSMARTINFO:
        {   
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid, &summary);
            long_return = !result ? summary.health : -1;      
            return (u_char *) & long_return;
        }
        case SNMP_DISKTEMP:
        {   
            struct smart_summary summary; 
            memset(&summary, 0, sizeof(struct smart_summary));
            int result = 0;
            result = PD_SMART_Get_Summary(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid, &summary);
            long_return = !result ? summary.temperature : -1; 			
			if(!result && summary.temperature == 0)
			{
				long_return =-1;				
			}
				
            return (u_char *) & long_return;
        }  
        case SNMP_DISKGLOBALSPARE:
        {           
            long_return = RAID_Is_Global_Spare(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid);            ; 
            return (u_char *) & long_return;
        }  
        case SNMP_DISKMODEL:
        {      
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            /* int is_msata_enc = FALSE;
            int is_pcie_enc = FALSE; */
            //if (0  == sysdisk_listP[disk_idx-1].diskencid)
            //{
                // int cache_enc_id = SE_Get_Cache_Enc_Id();
                // if(-1 != cache_enc_id)
                // {
                    // is_msata_enc = TRUE;
                    // is_pcie_enc = FALSE;
                // }

                ////*==> Start of 20140318 luciferchen+, KS-redmine #7618.
                ////Output PCIe drives when query information of NAS host.
                // int bitmap_pcie_enc_id = 0;
                // int result = SE_Get_Pci_Cache_Enc_Id(&bitmap_pcie_enc_id);
                // if(!result && bitmap_pcie_enc_id)
                // {
                    // int pcie_enc_id = 0;
                    // while(bitmap_pcie_enc_id)
                    // {
                        // if(bitmap_pcie_enc_id & 0x1)
                        // {
                            // is_msata_enc = FALSE;
                            // is_pcie_enc = TRUE;
                        // }
                        // ++pcie_enc_id;
                        // bitmap_pcie_enc_id >>= 1;
                    // }
                // }
                //<== End of 20140318 luciferchen+.*/       
            //} 
            //else
            //{
                /* is_msata_enc = FALSE;
                is_pcie_enc = FALSE; */
            //}
            
            PD_INFO pd_info;
            PD_Get_Info(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid, &pd_info);

            // if(is_msata_enc == TRUE)
            // {
                // snprintf(string, sizeof(string), "%s %s (mSATA)",pd_info.vendor, pd_info.model);
            // }
            // else if(is_pcie_enc == TRUE)
            // {
                // snprintf(string, sizeof(string), "%s %s (PCIe)",pd_info.vendor, pd_info.model);
            // }
            //else
            //{
                //typeP = PD_SATA == pd_infoP->type ? "SATA" : "SAS";
                char typeP[16] = {0};
                if (pd_info.type == PD_SATA)
                    snprintf(typeP, sizeof(typeP), "SATA");
                else
                {
                    if(pd_info.rotation_speed < 10000 && !(pd_info.capabilities & PD_CAP_SSD))
                    {
                        snprintf(typeP, sizeof(typeP), "NL-SAS");
                    }
                    else
                    {
                        snprintf(typeP, sizeof(typeP), "SAS");
                    }
                }
                snprintf(string, sizeof(string), "%s %s (%s)",pd_info.vendor, pd_info.model, typeP);
            //}
            
            *var_len = strlen(string);
            return (unsigned char *) string; 
        }  
        case SNMP_DISKCAPACITY:
        { 
            PD_INFO pd_info;
            PD_Get_Info(sysdisk_listP[disk_idx-1].diskencid, sysdisk_listP[disk_idx-1].diskid, &pd_info);       
            unsigned long long uCapacity = pd_info.capacity * pd_info.sector_size;        

            counter64_return.low = uCapacity & 0xffffffff;
            counter64_return.high = uCapacity >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }          
        default:
            DEBUGMSGTL(("var_Disk", "unknown sub-id %d in var_Disk\n",
                        vp->magic));
    }
    return NULL;
}

int header_Disk(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define DISK_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             disk_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Disk", "start header_Disk: "));
    DEBUGMSGOID(("header_Disk", name, *length));
    DEBUGMSG(("header_Disk", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Disksetting();
    for (;;) 
    {
        disk_idx = Get_Next_Disk();
        DEBUGMSGTL(("header_Disk", "... index %d\n", disk_idx));
        if (disk_idx == -1)
            break;
        newname[DISK_ENTRY_NAME_LENGTH] = disk_idx;
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
        DEBUGMSGTL(("header_Disk", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[DISK_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Disk", "... get disk stats "));
    DEBUGMSGOID(("header_Disk", name, *length));
    DEBUGMSG(("header_Disk", "\n"));

    return LowIndex;
}
void Init_Disksetting(void)
{
    //init parameter
    disknum = 0;
    curret_diskid = -1;
    curret_diskindex = 0;

    if(sysdisk_listP != NULL)
    {
        free(sysdisk_listP);
        sysdisk_listP = NULL;
    }
    
    memset(enc_id_ary, 0, sizeof(enc_id_ary));
    int se_count = SE_Enumerate(enc_id_ary, MAX_SE_NUM, enclosure_list_callback, NULL);

	disknum = 0;
    int i = 0;
    for(i = 0; i < se_count; i++)
    {
        disknum += PD_Enumerate(enc_id_ary[i], NULL, 0, NULL, NULL);  
    }
    //get sys power total count, allocate all sys power structure
    sysdisk_listP = calloc(disknum, sizeof(SNMPDISK_INFO));
 
    int diskidx = 0;
    // struct pd_ref *pd_list = NULL; 
    // struct pd_ref *pd_next = NULL;
    //PD_DEV_ID dev_id; 
    //PD_INFO *pd_infoP = NULL;

    //STORAGE_STATUS disk_status;    
    for(i = 0; i < se_count; i++)
    {
        memset(pd_id_ary, 0, sizeof(enc_id_ary));
        //PD_Enumerate(enc_id_ary[i], NULL, 0, dm_pd_scan_callback, &pd_list);
        int count = PD_Enumerate(enc_id_ary[i], pd_id_ary, MAX_PD_NUM, NULL, NULL);
        //while (pd_list)
        int j = 0;
        for(j = 0; j < count; j++)
        {            
            //pd_infoP = &pd_list->pd_info;
            //fill in value
            sysdisk_listP[diskidx].diskid = pd_id_ary[j];
            sysdisk_listP[diskidx].diskencid = enc_id_ary[i];           
            //sysdisk_listP[diskidx].diskcapacity = pd_infoP->capacity * pd_infoP->sector_size;            
   
            //strncpy(sysdisk_listP[diskidx].diskmodel, pd_infoP->model, sizeof(pd_infoP->model));
            //strncpy(sysdisk_listP[diskidx].diskdescr, pd_infoP->vendor, sizeof(pd_infoP->vendor));             
 
            // dev_id = PD_MAKE_DEV_ID(enc_id_ary[i], pd_infoP->port_id);

            // NAS_Disk_Get_Status(dev_id, &disk_status);       
            // sysdisk_listP[diskidx].diskstatus = disk_status;
 
            // struct smart_summary summary; 
            // memset(&summary, 0, sizeof(struct smart_summary));
            // int result = 0;
            // result = PD_SMART_Get_Summary(enc_id_ary[i], pd_infoP->port_id, &summary);

            // sysdisk_listP[diskidx].disksmartinfo = !result ? summary.health : -1;
            // sysdisk_listP[diskidx].disktemp = summary.temperature;
            
            diskidx++;
            
            
            // pd_next = pd_list->nextP;
            // free(pd_list);
            // pd_list = pd_next;   
        }
    } 
}


int Get_Next_Disk(void)
{  
    if (disknum > 0 && curret_diskindex < disknum)
    {
        curret_diskindex++;
        curret_diskid = sysdisk_listP[curret_diskindex - 1].diskid;
              
        return curret_diskindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
