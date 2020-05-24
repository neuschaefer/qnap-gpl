/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright (C) 2007 Apple, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

	config_require(qnapNAS/SystemInfo)
	config_require(qnapNAS/storage)
	config_require(qnapNAS/SystemInfoEx)
	config_require(qnapNAS/storageEx)
	config_require(qnapNAS/common)
#ifdef STORAGE_V2
    config_require(qnapNAS/Volume) 
    config_require(qnapNAS/Pool)   
    config_require(qnapNAS/Raid)
    config_require(qnapNAS/Cpu)
    config_require(qnapNAS/Systemfan)   
    config_require(qnapNAS/Systempower)       
    config_require(qnapNAS/Enclosure)
    config_require(qnapNAS/Disk)
    config_require(qnapNAS/Msatadisk)	
    config_require(qnapNAS/Diskperformance)
    config_require(qnapNAS/Iscsistorage)   
    config_require(qnapNAS/Iscsitarget) 
    config_require(qnapNAS/Iscsilun)     
    config_require(qnapNAS/Cacheacceleration)     
#endif
/* add the host resources mib to the default mibs to load */
config_add_mib(NAS-MIB)

