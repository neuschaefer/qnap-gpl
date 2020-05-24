#ifndef SNMP_CACHEACCELERATION_H
#define SNMP_CACHEACCELERATION_H

int handle_Cacheaccelerationservice(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
int handle_Cacheaccelerationavailablepercent(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
int handle_Cacheaccelerationreadhitrate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
int handle_Cacheaccelerationwritehitrate(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
int handle_Cacheaccelerationstatus(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo,netsnmp_request_info *requests);
#endif
