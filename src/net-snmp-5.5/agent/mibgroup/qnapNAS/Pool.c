#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Pool.h"
#include "common.h"

#ifdef QNAP_HAL_SUPPORT
#include "err_trace.h"
#include "utils.h"
#include "ini_config.h"
#include "hal.h"
#include "raid.h"
#include "storage_man.h"
#endif 


            
#define	SNMP_POOLINDEX		    1
#define	SNMP_POOLID		        2
#define	SNMP_POOLCAPACITY		3
#define	SNMP_POOLFREESIZE		4
#define	SNMP_POOLSTATUS		    5


int *pool_id_aryP = NULL;
int poolnum = 0;
int curret_poolid = -1;
int curret_poolindex = 0;
static struct counter64 counter64_return;

struct variable4 pool_variables[] = {
    {SNMP_POOLINDEX, ASN_INTEGER,RONLY,
     var_pool, 2, {1,1}},
    {SNMP_POOLID, ASN_INTEGER, RONLY,
     var_pool, 2, {1,2}},
    {SNMP_POOLCAPACITY, ASN_COUNTER64,RONLY,
     var_pool, 2, {1,3}},
    {SNMP_POOLFREESIZE, ASN_COUNTER64, RONLY,
     var_pool, 2, {1,4}},
    {SNMP_POOLSTATUS, ASN_INTEGER, RONLY,
     var_pool, 2, {1,5}}
};

oid PoolNumber_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,2,1 };
oid pool_variables_oid[] = { 1,3,6,1,4,1,24681,1,4,1,1,1,2,2,2 };

int handle_PoolNumber(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    POOL_REF *pool_list = NULL;
    POOL_REF *pool_next = NULL;
    int value = Pool_Enumerate(NULL, 0, dm_pool_callback, &pool_list);

    while (pool_list) 
    {
        pool_next = pool_list->nextP;
        free(pool_list);
        pool_list = pool_next;
    }
    
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_INTEGER, (u_char *) &value, sizeof(value));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_PoolNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}


void init_Pool(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("PoolNumber", handle_PoolNumber, PoolNumber_oid, OID_LENGTH(PoolNumber_oid), HANDLER_CAN_RONLY));
    poolnum = 0;
    curret_poolid = -1;
    curret_poolindex = 0;
    REGISTER_MIB("qnapNAS/Pool", pool_variables, variable4, pool_variables_oid);
                 
    DEBUGMSGTL(("qnapNAS/Pool:init", "initializing table PoolTable\n"));
}

u_char* var_pool(struct variable * vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
    int pool_idx;
    DEBUGMSGTL(("qnapNAS/Pool:var_pool", "var_pool start\n"));
    
    pool_idx = header_pool(vp, name, length, exact, var_len, write_method);
    if (pool_idx == MATCH_FAILED)
        return NULL;
    

    switch (vp->magic) 
    {
        case SNMP_POOLINDEX:
            long_return = pool_idx;
            return (u_char *) & long_return;
        case SNMP_POOLID:
            long_return = curret_poolid;
            return (u_char *) & long_return;
        case SNMP_POOLCAPACITY:
        {
            unsigned long long ulPoolSize = 0;
            unsigned long long ulPoolFree = 0;
            Pool_Get_Size( curret_poolid, &ulPoolSize, &ulPoolFree);        
            ulPoolSize = ulPoolSize* 512;
            counter64_return.low = ulPoolSize & 0xffffffff;
            counter64_return.high = ulPoolSize >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_POOLFREESIZE:
        {
            unsigned long long ulPoolSize = 0;
            unsigned long long ulPoolFree = 0;
            Pool_Get_Size( curret_poolid, &ulPoolSize, &ulPoolFree);
            ulPoolFree = ulPoolFree* 512;
            counter64_return.low = ulPoolFree & 0xffffffff;
            counter64_return.high = ulPoolFree >> 32;
            *var_len = sizeof(counter64_return);
            return (u_char *) &counter64_return;
        }
        case SNMP_POOLSTATUS:
        {
            POOL_STATUS pool_status = PS_NONE_STATUS;
            Pool_Get_Status(curret_poolid, &pool_status);
            long_return = pool_status;
            return (u_char *) & long_return;
        }
        default:
            DEBUGMSGTL(("var_pool", "unknown sub-id %d in var_pool\n",
                        vp->magic));
    }
    return NULL;
}

int header_pool(struct variable *vp, oid * name, size_t * length, int exact, size_t * var_len, WriteMethod ** write_method)
{
#define POOL_ENTRY_NAME_LENGTH	17
    oid             newname[MAX_OID_LEN];
    int             pool_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("header_pool", "start header_pool: "));
    DEBUGMSGOID(("header_pool", name, *length));
    DEBUGMSG(("header_pool", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, (int) vp->namelen * sizeof(oid));
  
    Init_Poolsetting();
    for (;;) 
    {
        pool_idx = Get_Next_Pool();
        DEBUGMSGTL(("header_pool", "... index %d\n", pool_idx));
        if (pool_idx == -1)
            break;
        newname[POOL_ENTRY_NAME_LENGTH] = pool_idx;
        result = snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
        if (exact && (result == 0))
        {
            LowIndex = pool_idx;
            break;
        }
        if ((!exact && (result < 0)) /*&& (LowIndex == -1 || pool_idx < LowIndex)*/) 
        {
            LowIndex = pool_idx;
            break;
        }
    }
    
    if(pool_id_aryP != NULL)
    {
        free(pool_id_aryP);
        pool_id_aryP = NULL;
    }
    
    if (LowIndex == -1) 
    {
        DEBUGMSGTL(("header_pool", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    newname[POOL_ENTRY_NAME_LENGTH] = LowIndex;
    memcpy((char *) name, (char *) newname, ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = (WriteMethod*)0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("header_pool", "... get pool stats "));
    DEBUGMSGOID(("header_pool", name, *length));
    DEBUGMSG(("header_pool", "\n"));

    return LowIndex;
}
void Init_Poolsetting(void)
{
    //init parameter
    poolnum = 0;
    curret_poolid = -1;
    curret_poolindex = 0;
    
    //init pool array and pool num
    if(pool_id_aryP != NULL)
    {
        free(pool_id_aryP);
        pool_id_aryP = NULL;
    }
    pool_id_aryP = calloc(MAX_POOL_NUMBER, sizeof(int));
    memset(pool_id_aryP, 0, MAX_POOL_NUMBER * sizeof(int));
    //poolnum = Pool_Enumerate(pool_id_aryP, MAX_POOL_NUMBER, NULL, NULL); 

    POOL_REF *pool_list = NULL;
    POOL_REF *pool_next = NULL;
    POOL_CONFIG *pool_infoP = NULL;    
    poolnum = Pool_Enumerate(NULL, 0, dm_pool_callback, &pool_list);

    int idx = 0;
    while (pool_list) 
    {
        pool_infoP = &pool_list->pool_info;
        pool_id_aryP[idx++] = pool_infoP->pool_id;
        pool_next = pool_list->nextP;
        free(pool_list);
        pool_list = pool_next;
    }    
}


int Get_Next_Pool(void)
{     
    if (poolnum > 0 && curret_poolindex < poolnum)
    {
        curret_poolid = pool_id_aryP[curret_poolindex];
        curret_poolindex++;
        return curret_poolindex;
    }
    else
    {
        return -1;  //if out of index, return -1
    }
}
