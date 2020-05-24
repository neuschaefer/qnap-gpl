#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Iscsitarget.h"
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


            
#define	SNMP_ISCSITARGETINDEX		    1
#define	SNMP_ISCSITARGETID		        2
#define	SNMP_ISCSITARGETNAME    		3
#define	SNMP_ISCSITARGETIQN     		4
#define	SNMP_ISCSITARGETSTATUS		    5

#define MAX_ISCSITARGET_NUMBER 128



int *iscsitarget_id_aryP = NULL;
int iscsitargetnum = 0;
int curret_iscsitargetid = -1;
int curret_iscsitargetindex = 0;
static struct counter64 counter64_return;
static char string[256]={};

struct variable4 iscsitarget_variables[] = {
    {SNMP_ISCSITARGETINDEX, ASN_INTEGER,RONLY,
     var_Iscsitarget, 2, {1,1}},
    {SNMP_ISCSITARGETID, ASN_INTEGER, RONLY,
     var_Iscsitarget, 2, {1,2}},
    {SNMP_ISCSITARGETNAME, ASN_OCTET_STR,RONLY,
     var_Iscsitarget, 2, {1,3}},
    {SNMP_ISCSITARGETIQN, ASN_OCTET_STR, RONLY,
     var_Iscsitarget, 2, {1,4}},
    {SNMP_ISCSITARGETSTATUS, ASN_INTEGER, RONLY,
     var_Iscsitarget, 2, {1,5}}
};

oid IscsitargetNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,2,1,11,1 };
oid iscsitarget_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,2,1,11,2 };

int handle_IscsitargetNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    int value = portalP->target_count;
    free(portalP);
  
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_IscsitargetNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Iscsitarget(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("IscsitargetNumber", handle_IscsitargetNumber, IscsitargetNumber_oid, OID_LENGTH(IscsitargetNumber_oid), HANDLER_CAN_RONLY));
    iscsitargetnum = 0;
    curret_iscsitargetid = -1;
    curret_iscsitargetindex = 0;
    REGISTER_MIB("qnapNAS/Iscsitarget", iscsitarget_variables, variable4, iscsitarget_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Iscsitarget:init", "initializing table IscsitargetTable\n"));
}

u_char* var_Iscsitarget(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int iscsitarget_idx;
    DEBUGMSGTL(("qnapNAS/Iscsitarget:var_Iscsitarget", "var_Iscsitarget start\n"));
    
    iscsitarget_idx = header_Iscsitarget(vp, name, length, exact, var_len, write_method);
    if (iscsitarget_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_ISCSITARGETINDEX:
            long_return = iscsitarget_idx;
            return (u_char *) & long_return;
        case SNMP_ISCSITARGETID:
        {
            //long_return = iscsitarget_id_aryP[iscsitarget_idx-1];
            long_return = curret_iscsitargetid;
            return (u_char *) & long_return;
        }
        break;
        case SNMP_ISCSITARGETNAME:
        {   
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            CFG_Get_Target_Name_By_Index(iscsitarget_id_aryP[iscsitarget_idx-1], string ,sizeof(string));
            *var_len = strlen(string);
            return (unsigned char *) string;            
        }
        break;
        case SNMP_ISCSITARGETIQN:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            CFG_Get_Target_Iqn_By_Index(iscsitarget_id_aryP[iscsitarget_idx-1], string ,sizeof(string));
            *var_len = strlen(string);
            return (unsigned char *) string;   
        }
        break;        
        case SNMP_ISCSITARGETSTATUS:
        {
            //target_info *targetP = calloc(1, sizeof(target_info)); 
            connect_info *initiatorsP = calloc(MAX_INIT_PER_TARGET, sizeof(connect_info));            
            //CFG_Get_Target_Info_By_Index(iscsitarget_id_aryP[iscsitarget_idx-1], targetP); 
            char targetiqn[256] = "";
            CFG_Get_Target_Iqn_By_Index(iscsitarget_id_aryP[iscsitarget_idx-1], targetiqn ,sizeof(targetiqn));            
            int connect_count = Cmd_Get_Target_Connected_Initiators(targetiqn, initiatorsP, MAX_INIT_PER_TARGET);
            if (connect_count > 0)
            {
                long_return = TARGET_CONNECTED;
            }
            else if (connect_count < 0)
            {
                long_return = TARGET_OFFLINE;
            }    
            else
            {
                char target_section[MAX_NAME_SIZE] = "";
                char tmp_buf[MAX_NAME_SIZE] = "";
                snprintf(target_section, sizeof(target_section), CFG_TARGET_INFOI, iscsitarget_id_aryP[iscsitarget_idx-1]);
                if (Ini_Conf_Get_Field(ISCSI_CFG_PATH, target_section,
                           CFG_TARGET_STATUS, tmp_buf, sizeof(tmp_buf)) >= 0)
                {           
                    long_return = atoi(tmp_buf);
                }
            }
            free(initiatorsP);
            //free(targetP);
            return (u_char *) & long_return; 
        }         
        default:
            DEBUGMSGTL(("var_Iscsitarget", "unknown sub-id %d in var_Iscsitarget\n",
                        vp->magic));
    }
    return NULL;
}

int header_Iscsitarget(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define ISCSITARGET_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             iscsitarget_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_Iscsitarget", "start header_Iscsitarget: "));
    DEBUGMSGOID(("header_Iscsitarget", name, *length));
    DEBUGMSG(("header_Iscsitarget", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Iscsitargetsetting();
    for (;;) 
    {
        iscsitarget_idx = Get_Next_Iscsitarget();
        DEBUGMSGTL(("header_Iscsitarget", "... index %d\n", iscsitarget_idx));
        if (iscsitarget_idx == -1)
            break;
        newname[ISCSITARGET_ENTRY_NAME_LENGTH] = iscsitarget_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = iscsitarget_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || iscsitarget_idx < LowIndex)*/) 
        {
            LowIndex = iscsitarget_idx;
            break;
        }
    }
    // if(iscsitarget_id_aryP != NULL)
    // {
        // free(iscsitarget_id_aryP);
        // iscsitarget_id_aryP = NULL;
    // }
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_Iscsitarget", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[ISCSITARGET_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_Iscsitarget", "... get iscsitarget stats "));
    DEBUGMSGOID(("header_Iscsitarget", name, *length));
    DEBUGMSG(("header_Iscsitarget", "\n"));

    return LowIndex;
}
void Init_Iscsitargetsetting(void)
{
    //init parameter
    //iscsitargetnum = 0;
    curret_iscsitargetid = -1;
    curret_iscsitargetindex = 0;
 
    if(iscsitarget_id_aryP != NULL)
    {
        free(iscsitarget_id_aryP);
        iscsitarget_id_aryP = NULL;
    }
 
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    iscsitargetnum = portalP->target_count;


    if(iscsitargetnum > 0)
    {
        iscsitarget_id_aryP = calloc(iscsitargetnum+1, sizeof(int));
        //memset(iscsitarget_id_aryP, 0, (iscsitargetnum+1) * sizeof(int));    
        int i = 0;
        for(i = 0; i < portalP->target_count; i++)
        {
            iscsitarget_id_aryP[i] = (int)portalP->target_ary[i];
        }
    }
    free(portalP);
}


int Get_Next_Iscsitarget(void)
{     
    if (iscsitargetnum > 0 && curret_iscsitargetindex < iscsitargetnum)
    {
        curret_iscsitargetid = iscsitarget_id_aryP[curret_iscsitargetindex];
        curret_iscsitargetindex++;
        return curret_iscsitargetindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}

