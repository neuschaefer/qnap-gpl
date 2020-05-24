#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Enclosure.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 


            
#define	SNMP_ENCLOSUREINDEX		        1
#define	SNMP_ENCLOSUREID		        2
#define	SNMP_ENCLOSUREMODEL		        3
#define	SNMP_ENCLOSURESERIALNUM		    4
#define	SNMP_ENCLOSURESLOT		        5
#define	SNMP_ENCLOSURENAME		        6
#define	SNMP_ENCLOSURESYSTEMTEMP	    7


extern int enclosure_list_callback(int enc_id, void *contextP)
{
    int ret = 0;
    int is_internal = SE_Is_Internal(enc_id);
    is_internal = is_internal && (enc_id != SE_Get_Cache_Enc_Id());//20131108 luciferchen+, KS-redmine #6153.
    
    if (0 == is_internal)
    {
        ret = -2;
        return ret;
    }
    // else if (1 == SE_Is_Removing(enc_id))    
    // {
        // ret = -2;
    // }
//20140618, WebbLiang, task#8582, if the enclosure was safely removed, don't show it.
#ifdef JBOD_ROAMING
    char enc_file[32] = {0}, section[MAX_SECTION_LEN] = {0}, safe_remove_state[8] = {0};

    snprintf(section, sizeof(section), ENC_HOTSWAP_SECTION_ENCID, enc_id);
    Ini_Conf_Get_Field(ENC_JBOD_ROAMING_STATUS_FILE, section, ENC_JBOD_ROAMING_KEY_SAFE_REMOVE_STATE, safe_remove_state, sizeof(safe_remove_state));
    snprintf(enc_file, sizeof(enc_file), "/etc/enclosure_%d.conf", enc_id);
    if (access(enc_file, 0) != 0 && !strcmp(safe_remove_state, "1"))
    {
        ret = -2
        return ret;
    }    
#endif    

    //*==> 20140318 luciferchen+, KS-redmine #7618.
    int bmp_pcie_enc_id = 0;
    int result = SE_Get_Pci_Cache_Enc_Id(&bmp_pcie_enc_id);
    if(bmp_pcie_enc_id & (1 << enc_id))
    {
        is_internal = FALSE;
    }
    //<== end of 20140318 luciferchen+.*/
    
    if (FALSE == is_internal)
    {
        ret = -2;
        return ret;
    }
    else 
    {
        // Benjamin 20120711 patch for BUG 25929: Do not reply disk info if enclosure is removing.    
        if (TRUE == Volume_Is_Enclosure_Hot_Removed(enc_id)) 
        {        
            ret = -2;
            return ret;
        }

    }        
    return ret;    
}



int enc_id_ary[MAX_SE_NUM];
int enclosurenum = 0;
int curret_enclosureid = -1;
int curret_enclosureindex = 0;
static char string[256]={};

struct variable4 enclosure_variables[] = {
    {SNMP_ENCLOSUREINDEX, ASN_INTEGER,RONLY,
     var_enclosure, 2, {1,1}},
    {SNMP_ENCLOSUREID, ASN_INTEGER, RONLY,
     var_enclosure, 2, {1,2}},
    {SNMP_ENCLOSUREMODEL, ASN_OCTET_STR,RONLY,
     var_enclosure, 2, {1,3}},
    {SNMP_ENCLOSURESERIALNUM, ASN_OCTET_STR, RONLY,
     var_enclosure, 2, {1,4}},
    {SNMP_ENCLOSURESLOT, ASN_INTEGER, RONLY,
     var_enclosure, 2, {1,5}},
    {SNMP_ENCLOSURENAME, ASN_OCTET_STR, RONLY,
     var_enclosure, 2, {1,6}},
    {SNMP_ENCLOSURESYSTEMTEMP, ASN_INTEGER, RONLY,
     var_enclosure, 2, {1,7}}
};

oid EnclosureNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,1,1 };
oid enclosure_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,1,1,2 };

int handle_EnclosureNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    memset(enc_id_ary, 0, sizeof(enc_id_ary));

    int value = SE_Enumerate(enc_id_ary, MAX_SE_NUM, 
                            enclosure_list_callback, NULL);
    
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_EnclosureNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Enclosure(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("EnclosureNumber", handle_EnclosureNumber, EnclosureNumber_oid, OID_LENGTH(EnclosureNumber_oid), HANDLER_CAN_RONLY));
    enclosurenum = 0;
    curret_enclosureid = -1;
    curret_enclosureindex = 0;
    REGISTER_MIB("qnapNAS/Enclosure", enclosure_variables, variable4, enclosure_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Enclosure:init", "initializing table EnclosureTable\n"));
}

u_char* var_enclosure(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int enclosure_idx;
    DEBUGMSGTL(("qnapNAS/Enclosure:var_enclosure", "var_enclosure start\n"));
    
    enclosure_idx = header_enclosure(vp, name, length, exact, var_len, write_method);
    if (enclosure_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_ENCLOSUREINDEX:
            long_return = enclosure_idx;
            return (u_char *) & long_return;
        case SNMP_ENCLOSUREID:
            long_return = curret_enclosureid;
            return (u_char *) & long_return;
        case SNMP_ENCLOSUREMODEL:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            ENCLOSURE_INFO enc_info;
            SE_Get_Info(curret_enclosureid, &enc_info);
#ifndef IS_G
            strncpy(string, enc_info.model, sizeof(enc_info.model));
#else
            if (0 == curret_enclosureid) 
            {
                char model[32];
                Get_Display_Model_Name(model, sizeof(model));
                strncpy(string, model, sizeof(model));
            }
            else
            {
                strncpy(string, enc_info.model, sizeof(enc_info.model));
            }
#endif                   
            *var_len = strlen(string);
            return (unsigned char *) string;   
        }
        case SNMP_ENCLOSURESERIALNUM:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            SE_Get_Serial_No(curret_enclosureid, string, sizeof(string));
            if(strlen(string) == 0)
	    {
		strcpy(string, "--");
	    }
	    *var_len = strlen(string);
            return (unsigned char *) string;
        }
        case SNMP_ENCLOSURESLOT:
        {
            long_return = SE_Get_PD_Port_Num(curret_enclosureid);
            return (u_char *) & long_return;
        }
        case SNMP_ENCLOSURENAME:
        {
			int str_len=sizeof(string)-1;
			string[str_len] = '\0';
            if (0 == curret_enclosureid) 
            {
                Get_Server_Name(string, sizeof(string));
            }
            else
            {
                SE_Get_Friendly_Name(curret_enclosureid, string, sizeof(string));
            }
            *var_len = strlen(string);
            return (unsigned char *) string;
        }
        case SNMP_ENCLOSURESYSTEMTEMP:
        {
            ENCLOSURE_SYS_STATUS enc_sys_status;
            SE_Get_System_Status(curret_enclosureid, &enc_sys_status);
            long_return = enc_sys_status.sys_temp;
            return (u_char *) & long_return;
        }       
        default:
            DEBUGMSGTL(("var_enclosure", "unknown sub-id %d in var_enclosure\n",
                        vp->magic));
    }
    return NULL;
}

int header_enclosure(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define ENCLOSURE_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             enclosure_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_enclosure", "start header_enclosure: "));
    DEBUGMSGOID(("header_enclosure", name, *length));
    DEBUGMSG(("header_enclosure", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Enclosuresetting();
    for (;;) 
    {
        enclosure_idx = Get_Next_Enclosure();
        DEBUGMSGTL(("header_enclosure", "... index %d\n", enclosure_idx));
        if (enclosure_idx == -1)
            break;
        newname[ENCLOSURE_ENTRY_NAME_LENGTH] = enclosure_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = enclosure_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || enclosure_idx < LowIndex)*/) 
        {
            LowIndex = enclosure_idx;
            break;
        }
    }
    
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_enclosure", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[ENCLOSURE_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_enclosure", "... get enclosure stats "));
    DEBUGMSGOID(("header_enclosure", name, *length));
    DEBUGMSG(("header_enclosure", "\n"));

    return LowIndex;
}
void Init_Enclosuresetting(void)
{
    //init parameter
    enclosurenum = 0;
    curret_enclosureid = -1;
    curret_enclosureindex = 0;
    
    //init enclosure array and enclosure num
    memset(enc_id_ary, 0, sizeof(enc_id_ary));

    enclosurenum = SE_Enumerate(enc_id_ary, MAX_SE_NUM, enclosure_list_callback, NULL);

   
}


int Get_Next_Enclosure(void)
{     
    if (enclosurenum > 0 && curret_enclosureindex < enclosurenum)
    {
        curret_enclosureid = enc_id_ary[curret_enclosureindex];
        curret_enclosureindex++;
        return curret_enclosureindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
