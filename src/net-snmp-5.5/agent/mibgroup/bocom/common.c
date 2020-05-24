#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "common.h"

int GetOneStringFromCommand(char *cmd, char *buff, int buf_size)
{
	int ret = -1;
	FILE *fp = popen(cmd, "r");
	if (fp) {
		if (fgets(buff, buf_size, fp)) {
			ret = 0;
		}
		fclose(fp);
	}
	return ret;
}

int GetNasDiskNumber()
{
	int num = 0;
	char buff[128];
	if (GetOneStringFromCommand("/sbin/getsysinfo hdnum", buff, sizeof(buff)) == 0) {
		num = atoi(buff);
	}
	return num;
}
int GetNasNetworkNumber()
{
	int num = 0;
	char buff[128];
	if (GetOneStringFromCommand("/sbin/getcfg Network \"Interface Number\" -d 0", buff, sizeof(buff)) == 0) {
		num = atoi(buff);
	}
	return num;
}

int GetSystemFanNumber()
{
	int num = 0;
	char buff[128];
	if (GetOneStringFromCommand("/sbin/getsysinfo sysfannum", buff, sizeof(buff)) == 0) {
		num = atoi(buff);
	}
	return num;
}

int GetSystemVolumeNumber()
{
	int num = 0;
	char buff[128];
	if (GetOneStringFromCommand("/sbin/getsysinfo sysvolnum", buff, sizeof(buff)) == 0) {
		num = atoi(buff);
	}
	return num;
}

