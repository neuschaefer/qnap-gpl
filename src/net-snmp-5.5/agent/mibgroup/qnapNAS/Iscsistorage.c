#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/net-snmp-config.h>
#include "Iscsistorage.h"
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


oid Iscsiservice_oid[] =        { 1,3,6,1,4,1,24681,1,4,1,1,2,1,1 };
oid Iscsiserviceport_oid[] =    { 1,3,6,1,4,1,24681,1,4,1,1,2,1,2 };
oid Isnsservice_oid[] =         { 1,3,6,1,4,1,24681,1,4,1,1,2,1,3 };
oid Isnsip_oid[] =              { 1,3,6,1,4,1,24681,1,4,1,1,2,1,4 };

int handle_Iscsiservice(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    int value = portalP->bService;
    free(portalP);
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

int handle_Iscsiserviceport(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    int value = atoi(portalP->service_port);
    free(portalP);
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


int handle_Isnsservice(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    int value = portalP->biSNS;
    free(portalP);
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

int handle_Isnsip(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
    portal_info *portalP = calloc(1, sizeof(portal_info));
    CFG_Get_Portal_Info(portalP);
    struct	in_addr  tempip;
    inet_aton(portalP->iSNS_server_ip,&tempip);
    free(portalP);
    switch(reqinfo->mode) 
    {
        case MODE_GET:
            snmp_set_var_typed_value(requests->requestvb, ASN_IPADDRESS, (u_char *) &tempip, sizeof(tempip));
            break;


        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_DiskperformanceNumber\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

void init_Iscsistorage(void)
{
    netsnmp_register_scalar(netsnmp_create_handler_registration("iSSCIService", handle_Iscsiservice, Iscsiservice_oid, OID_LENGTH(Iscsiservice_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("iSNSServicePort", handle_Iscsiserviceport, Iscsiserviceport_oid, OID_LENGTH(Iscsiserviceport_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("iSNSIP", handle_Isnsip, Isnsip_oid, OID_LENGTH(Isnsip_oid), HANDLER_CAN_RONLY));
    netsnmp_register_scalar(netsnmp_create_handler_registration("iSNSService", handle_Isnsservice, Isnsservice_oid, OID_LENGTH(Isnsservice_oid), HANDLER_CAN_RONLY));

                 
    DEBUGMSGTL(("qnapNAS/Volume:init", "initializing table Iscsi Storage\n"));
}
