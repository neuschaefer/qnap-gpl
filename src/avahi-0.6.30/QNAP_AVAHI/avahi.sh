#!/bin/sh
GETCFG=/sbin/getcfg
AVAHI_SRC=/mnt/ext/opt/avahi0630
DBUS_DAEMON=/usr/sbin/dbus-daemon
AVAHI_DAEMON=/usr/sbin/avahi-daemon
AVAHI_DIR=/etc/avahi
AVAHI_CONF_DIR=/etc/avahi/services
AVAHI_TM_CONF_FILE=${AVAHI_CONF_DIR}/timemachine.service
MYCLOUDNAS_CONF="/etc/config/qnapddns.conf"
Project_Name=`/sbin/getcfg Project Name -f /var/default`
/sbin/test -f ${AVAHI_DAEMON} || exit 0

DBUS_PID_FILE="/var/run/dbus/pid"
AVAHI_PID_FILE="/var/run/avahi-daemon/pid"
AVAHI_LOCK_FILE="/tmp/avahi0630.lock"
AVAHI_SET_LOCK_FILE="/var/lock/avahi_set_conf.lock"

IS_DEF_VOL="no"

function wait_dbus_ready
{
    local wait_sec=$1
    local i=0
    while [ $i -lt ${wait_sec} ]; do
        local dbus_status=`/usr/sbin/check_dbus`
        if [ "${dbus_status}" == "active" ]; then
            echo "active"
            return 0
        fi
        (( i++ ))
        sleep 1
    done
    echo "inactive"
    return 0
}

is_def_vol_exist()
{
	share_list=1
	[ -x /sbin/user_cmd ] && share_list=`/sbin/user_cmd -s`
	if [ -z "$share_list" ]; then
		IS_DEF_VOL="no"
	else
		IS_DEF_VOL="yes"
	fi
}

_renew_timemachine_share_folder()
{
	dk_index=1
	/sbin/user_cmd -s | while read _share_name
	do
		#echo "_share_name is ${_share_name}"
		tm=`getcfg "${_share_name}" timemachine -d "no" -f /etc/config/smb.conf`
		#echo "${_share_name}: timemachine=$tm"
		if [ "${tm}" == "yes" ]; then
			uuid=`grep \"${_share_name}\" /var/netatalk/afp_voluuid.conf 2>/dev/null | cut -f 2 -d$'\t'`
/bin/cat >> $2 <<__EOF__
<txt-record>dk$dk_index=adVN=${_share_name},adVF=$1,adVU=${uuid}</txt-record>
__EOF__
			dk_index=$(($dk_index+1))
		fi
	done
}

set_avahi_conf()
{
	/bin/rm -rf ${AVAHI_CONF_DIR}/web.service \
				${AVAHI_CONF_DIR}/samba.service \
				${AVAHI_CONF_DIR}/afp.service \
				${AVAHI_CONF_DIR}/ssh.service \
				${AVAHI_CONF_DIR}/sftp-ssh.service \
				${AVAHI_CONF_DIR}/https.service \
				${AVAHI_CONF_DIR}/upnp.service \
				${AVAHI_CONF_DIR}/itunes.service \
				${AVAHI_CONF_DIR}/printer*.service \
				${AVAHI_CONF_DIR}/qmobile.service \
				${AVAHI_CONF_DIR}/qdiscover.service \
				${AVAHI_CONF_DIR}/csco.service \
				${AVAHI_CONF_DIR}/ftp.service \
				${AVAHI_CONF_DIR}/timemachine.service \
				${AVAHI_CONF_DIR}/qbox_cluster.service >&/dev/null

	is_def_vol_exist

	MAC_ADDR=`/sbin/getcfg "MAC" "eth0" -f "/var/MacAddress" | /bin/tr '[a-z]' '[A-Z]'`
	WEB_PORT=`/sbin/getcfg System "Web Access Port" -d 8080`
	HTTPS_PORT=`/sbin/getcfg "Stunnel" "Port" -d "443"`
	HOSTNAME=`/sbin/getcfg System "Server Name"`
	MYCLOUDNAS_NAME=`/sbin/getcfg "QNAP DDNS Service" "Host Name" -f "${MYCLOUDNAS_CONF}"`
	MYCLOUDNAS_DN=`/sbin/getcfg "QNAP DDNS Service" "Domain Name Server" -f "${MYCLOUDNAS_CONF}"`
	FW_VER=`/sbin/getcfg "System" "Version" -d --`
	FW_BN=`/sbin/getcfg System "Build Number" -d --`
	SN=`/sbin/get_hwsn`
	DISPLAY_MODEL=`/sbin/get_display_name`
	MODEL=`/sbin/getcfg "System" "Model" -d NAS`
	
	if [ "x$HOSTNAME" = "x" ]; then
		/sbin/gen_hostname
		/bin/sync
		HOSTNAME=`/sbin/getcfg System "Server Name"`
	fi

	#qcs: fix  When the host reboot, node list can not find this host.
	[ -d /usr/local/qcs_tool ] && /usr/local/qcs_tool/qcs-avahi.sh

	if [ `/sbin/getcfg "Bonjour Service" Web -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "Web Service Name" -d "$HOSTNAME"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/web.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_http._tcp</type>
<port>${WEB_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" SAMBA -u -d 1` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "SAMBA Service Name" -d "$HOSTNAME"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/samba.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_smb._tcp</type>
<port>445</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" AFP -u -d 0` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(AFP)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "AFP Service Name" -d "$HOSTNAME(AFP)"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/afp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_afpovertcp._tcp</type>
<port>548</port>
</service>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_device-info._tcp</type>
<port>0</port>
<txt-record>model=Xserve</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" SSH -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(SSH)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "SSH Service Name" -d "$HOSTNAME(SSH)"`
		fi
		SSH_PORT=`/sbin/getcfg LOGIN "SSH Port"`
/bin/cat > ${AVAHI_CONF_DIR}/ssh.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_ssh._tcp</type>
<port>${SSH_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" FTP -u -d 0` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		BROWSENAME=`/sbin/getcfg "Bonjour Service" "FTP Service Name"`
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(FTP)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "FTP Service Name" -d "$HOSTNAME(FTP)"`
		fi
		FTP_PORT=`/sbin/getcfg FTP Port`
/bin/cat > ${AVAHI_CONF_DIR}/ftp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_ftp._tcp</type>
<port>${FTP_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" HTTPS -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(HTTPS)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "HTTPS Service Name" -d "$HOSTNAME(HTTPS)"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/https.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_https._tcp</type>
<port>${HTTPS_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" UPNP -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(DLNA)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "UPNP Service Name" -d "$HOSTNAME(DLNA)"`
		fi
		UPNP_PORT=`/sbin/getcfg "TwonkyMedia" "Port" -d 9000`
/bin/cat > ${AVAHI_CONF_DIR}/upnp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_upnp._tcp</type>
<port>${UPNP_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" "QMobile" -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(Apps)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "QMobile Service Name" -d "$HOSTNAME(Apps)"`
		fi
		QMOBILE_PORT=`/sbin/getcfg "QMobile" "Port" -d 3986`
/bin/cat > ${AVAHI_CONF_DIR}/qmobile.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_qmobile._tcp</type>
<port>${QMOBILE_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" QNAP -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" QNAP -d "$HOSTNAME"`
		fi
		atype="http";
		aport=${WEB_PORT};
		if [ `/sbin/getcfg "Stunnel" "Enable" -u -d 1` = 1  -a `/sbin/getcfg "System" "Force SSL" -u -d 0` = 1 ]; then
			atype="https"
			aport=${HTTPS_PORT}
		fi
		upnp_pf=`/sbin/getcfg UPnP_Global Enable -d FALSE -f /etc/config/upnpc.conf`
		webadm_ext_port=`/sbin/getcfg System ExtPort -d 0`
		web_ext_port=`/sbin/getcfg QWEB ExtPort -d 0`
		web_ssl_ext_port=`/sbin/getcfg QWEB ExtSSLPort -d 0`
		webadm_ssl_ext_port=`/sbin/getcfg Stunnel ExtSSLPort -d 0`

		qTXT="accessType=${atype},accessPort=${aport},model=${MODEL},displayModel=${DISPLAY_MODEL},fwVer=${FW_VER},fwBuildNum=${FW_BN}"
		if [ x != x"$SN" ]; then
			qTXT="${qTXT},serialNum=${SN}"
		fi
		if [ x != x"$MYCLOUDNAS_NAME" -a x != x"$MYCLOUDNAS_DN" ]; then
			qTXT="${qTXT},myCloudNASName=${MYCLOUDNAS_NAME},myCloudNASDomain=${MYCLOUDNAS_DN}"
		fi
		if [ x$upnp_pf = xTRUE ]; then
			qTXT="${qTXT},webAdmPort=${webadm_ext_port},webAdmSslPort=${webadm_ssl_ext_port},webPort=${web_ext_port},webSslPort=${web_ssl_ext_port}"
		fi
		
/bin/cat > ${AVAHI_CONF_DIR}/qdiscover.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_qdiscover._tcp</type>
<port>${WEB_PORT}</port>
<txt-record>$qTXT</txt-record>
</service>
</service-group>
__EOF__
	fi
	
	if [ `/sbin/getcfg iTune Enable -u -d FALSE` = TRUE ]; then
		_iTunes_ID=`/sbin/get_hwsn`
		if [ $? != 0 ]; then
			_iTunes_ID=`/sbin/uuidgen | /bin/cut -c 1-8`
		else
			len=${#_iTunes_ID}
			if [ ${len} -gt 16 ]; then
				sublen=$(( ${len}-16+1  ))
				_iTunes_ID=`echo ${_iTunes_ID} | cut -c ${sublen}-${len}`
			fi
		fi
		if [ "x`/sbin/getcfg iTune "Password Enabled" -u -d 0`" = "x1" ]; then
			export _Passowrd=true
		else
			export _Passowrd=false
		fi
/bin/cat > ${AVAHI_CONF_DIR}/itunes.service <<__EOF__
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$HOSTNAME(iTunes)</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_daap._tcp</type>
<port>3689</port>
<txt-record>txtvers=1</txt-record>
<txt-record>Machine Name=$HOSTNAME(iTunes)</txt-record>
<txt-record>Database ID=${_iTunes_ID}</txt-record>
<txt-record>iTSh Version=131073</txt-record>
<txt-record>Version=196610</txt-record>
<txt-record>Passowrd=${_Passowrd}</txt-record>
</service>
</service-group>

__EOF__
	fi

for i in 1 2 3 4 5 6 7 8 9 10; do
[ $i -eq 1 ] && i=""
	if [ `/sbin/getcfg "Bonjour Service" "PRINTER$i" -u -d 0` = 1 ]; then
		local phy_exist=`/sbin/getcfg "Printers" "Phy exist$i"`
		[ "x$phy_exist" = "x" ] || [ $phy_exist -eq 0 ] && continue
		local status=`/sbin/getcfg "Printers" "Status$i"`
		[ $status -eq 4 ] && continue
		local service_name=`/sbin/getcfg "Bonjour Service" "PRINTER Service Name$i"`
		local printer_port=`/bin/cat /etc/config/cups/cupsd.conf 2>>/dev/null | /bin/grep "Listen" | /bin/cut -d':' -f 2 | /usr/bin/head -n 1`
		local printer_name=`/sbin/getcfg "Printers" "Name$i"`
		local manufacture=`/sbin/getcfg "Printers" "Manufacture$i"`
		local model=`/sbin/getcfg "Printers" "Model$i"`
/bin/cat > ${AVAHI_CONF_DIR}/printer$i.service <<__EOF__
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$service_name(Airprint)</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_ipp._tcp</type>
<subtype>_universal._sub._ipp._tcp</subtype>
<port>$printer_port</port>
<txt-record>txtvers=1</txt-record>
<txt-record>qtotal=1</txt-record>
<txt-record>priority=0</txt-record>
<txt-record>rp=printers/$printer_name</txt-record>
<txt-record>ty=$manufacture $model</txt-record>
<txt-record>note=$manufacture $model</txt-record>
<txt-record>product=(GPL Ghostscript)</txt-record>
<txt-record>pdl=application/pdf,application/postscript,application/vnd.cups-raster,application/octet-stream,image/png</txt-record>
<txt-record>URF=none</txt-record>
</service>
</service-group>

__EOF__
	fi
done

if [ `/sbin/getcfg "Appletalk" Enable -u -d FALSE` = "TRUE" ]; then
	ps | grep "/usr/local/sbin/afpd" | grep -v grep > /dev/null 2>&1
	afp_exist=$?
	if [ "x${afp_exist}" = "x0" ]; then
		if [ `/sbin/getcfg "TimeMachine" Enabled -u -d FALSE` = "TRUE" ]; then
			STOR_TYPE=`/sbin/getcfg "TimeMachine" "Storage Type" -d 1`
			if [ "x$STOR_TYPE" = "x2" ]; then
				TM_VN=home
				TM_SN=homes
			else
				TM_VN=TMBackup
				TM_SN=TMBackup
			fi
			AD_CHECK=`/sbin/getcfg global "server role" -d "x" -f /etc/config/smb.conf`
			if [ "$AD_CHECK" == "active directory domain controller" ]; then
				TM_ADVF=0xa1
			else
				LDAP_ENABLE=`/sbin/getcfg "LDAP" "Enable" -d "FALSE"`
				SAMBA_AUTH=`/sbin/getcfg global "passdb backend" -d "x" -f /etc/config/smb.conf`
				if [ "x$LDAP_ENABLE" == "xTRUE" ] && [ "$SAMBA_AUTH" != "smbpasswd" ]; then
					TM_ADVF=0xa1
				else
					TM_ADVF=0xa3
				fi
			fi
			uuid=`grep \"${TM_SN}\" /var/netatalk/afp_voluuid.conf | cut -f 2 -d$'\t'`
			if [ "x${uuid}" != "x" ]; then
/bin/cat > ${AVAHI_CONF_DIR}/timemachine.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">${HOSTNAME}(TimeMachine)</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_adisk._tcp</type>
<port>9</port>
<txt-record>dk0=adVN=${TM_VN},adVF=${TM_ADVF},adVU=${uuid}</txt-record>
__EOF__

_renew_timemachine_share_folder ${TM_ADVF} "${AVAHI_CONF_DIR}/timemachine.service"

/bin/cat >> ${AVAHI_CONF_DIR}/timemachine.service <<__EOF__
</service>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_smb._tcp</type>
<port>445</port>
</service>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_afpovertcp._tcp</type>
<port>548</port>
</service>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_device-info._tcp</type>
<port>0</port>
<txt-record>model=Xserve</txt-record>
</service>
</service-group>

__EOF__
			fi
		fi
	fi
fi
if [ x"`/sbin/getcfg Qsync Enable -u -d FALSE`" = x"TRUE" ] && [ x"`/sbin/getcfg Qsync cluster_slave_enable -u -d FALSE`" = x"FALSE" ]; then
	QBOXCLUSTER_PORT=`/sbin/getcfg -f "/etc/config/qbox_cluster/qbox_cluster_s.conf" "global" "Port"`
	if [ x"`/sbin/getcfg Stunnel Enable -d 0`" != x"1" ]; then
		HTTPS_PORT=-1
	fi
	if [ x"`/sbin/getcfg Qsync cluster_master_enable -d FALSE`" = x"TRUE" ]; then
		MASTER=1
	else
		MASTER=0
	fi
/bin/cat > ${AVAHI_CONF_DIR}/qbox_cluster.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">${HOSTNAME}(QBOX_CLUSTER)</name>
<service>
<host-name>${HOSTNAME}.local</host-name>
<type>_qboxcluster._tcp</type>
<port>${QBOXCLUSTER_PORT}</port>
<txt-record>webport=${WEB_PORT},sslwebport=${HTTPS_PORT},master=${MASTER}</txt-record>
</service>
</service-group>
__EOF__
fi
}
clean_avahi_thunderbolt_conf()
{
	/bin/rm -rf ${AVAHI_CONF_DIR}/tbt.web.service \
			${AVAHI_CONF_DIR}/tbt.samba.service \
			${AVAHI_CONF_DIR}/tbt.afp.service \
			${AVAHI_CONF_DIR}/tbt.ssh.service \
			${AVAHI_CONF_DIR}/tbt.sftp-ssh.service \
			${AVAHI_CONF_DIR}/tbt.https.service \
			${AVAHI_CONF_DIR}/tbt.upnp.service \
			${AVAHI_CONF_DIR}/tbt.itunes.service \
			${AVAHI_CONF_DIR}/tbt.printer*.service \
			${AVAHI_CONF_DIR}/tbt.qmobile.service \
			${AVAHI_CONF_DIR}/tbt.qdiscover.service \
			${AVAHI_CONF_DIR}/tbt.csco.service \
			${AVAHI_CONF_DIR}/tbt.ftp.service \
			${AVAHI_CONF_DIR}/tbt.timemachine.service >&/dev/null
}

set_avahi_thunderbolt_conf()
{
	is_def_vol_exist

	WEB_PORT=`/sbin/getcfg System "Web Access Port" -d 8080`
	HTTPS_PORT=`/sbin/getcfg "Stunnel" "Port" -d "443"`
	HOSTNAME=`/sbin/getcfg System "Server Name"`
	MYCLOUDNAS_NAME=`/sbin/getcfg "QNAP DDNS Service" "Host Name" -f "${MYCLOUDNAS_CONF}"`
	MYCLOUDNAS_DN=`/sbin/getcfg "QNAP DDNS Service" "Domain Name Server" -f "${MYCLOUDNAS_CONF}"`
	FW_VER=`/sbin/getcfg "System" "Version" -d --`
	FW_BN=`/sbin/getcfg System "Build Number" -d --`
	SN=`/sbin/get_hwsn`
	DISPLAY_MODEL=`/sbin/get_display_name`
	MODEL=`/sbin/getcfg "System" "Model" -d NAS`
	
	if [ "x$HOSTNAME" = "x" ]; then
		/sbin/gen_hostname
		/bin/sync
		HOSTNAME=`/sbin/getcfg System "Server Name"`
	fi
	TBTHOSTNAME="${HOSTNAME}tbt.local"

	if [ `/sbin/getcfg "Bonjour Service" Web -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "Web Service Name" -d "$HOSTNAME"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/tbt.web.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_http._tcp</type>
<port>${WEB_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" SAMBA -u -d 1` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "SAMBA Service Name" -d "$HOSTNAME"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/tbt.samba.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_smb._tcp</type>
<port>445</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" AFP -u -d 0` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(AFP)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "AFP Service Name" -d "$HOSTNAME(AFP)"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/tbt.afp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_afpovertcp._tcp</type>
<port>548</port>
</service>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_device-info._tcp</type>
<port>0</port>
<txt-record>model=Xserve</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" SSH -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(SSH)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "SSH Service Name" -d "$HOSTNAME(SSH)"`
		fi
		SSH_PORT=`/sbin/getcfg LOGIN "SSH Port"`
/bin/cat > ${AVAHI_CONF_DIR}/tbt.ssh.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_ssh._tcp</type>
<port>${SSH_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" FTP -u -d 0` = 1 -a "x${IS_DEF_VOL}" = "xyes" ]; then
		BROWSENAME=`/sbin/getcfg "Bonjour Service" "FTP Service Name"`
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(FTP)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "FTP Service Name" -d "$HOSTNAME(FTP)"`
		fi
		FTP_PORT=`/sbin/getcfg FTP Port`
/bin/cat > ${AVAHI_CONF_DIR}/tbt.ftp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_ftp._tcp</type>
<port>${FTP_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" HTTPS -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(HTTPS)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "HTTPS Service Name" -d "$HOSTNAME(HTTPS)"`
		fi
/bin/cat > ${AVAHI_CONF_DIR}/tbt.https.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_https._tcp</type>
<port>${HTTPS_PORT}</port>
<txt-record>path=/</txt-record>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" UPNP -u -d 0` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(DLNA)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "UPNP Service Name" -d "$HOSTNAME(DLNA)"`
		fi
		UPNP_PORT=`/sbin/getcfg "TwonkyMedia" "Port" -d 9000`
/bin/cat > ${AVAHI_CONF_DIR}/tbt.upnp.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_upnp._tcp</type>
<port>${UPNP_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" "QMobile" -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME(Apps)"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" "QMobile Service Name" -d "$HOSTNAME(Apps)"`
		fi
		QMOBILE_PORT=`/sbin/getcfg "QMobile" "Port" -d 3986`
/bin/cat > ${AVAHI_CONF_DIR}/tbt.qmobile.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_qmobile._tcp</type>
<port>${QMOBILE_PORT}</port>
</service>
</service-group>
__EOF__
	fi

	if [ `/sbin/getcfg "Bonjour Service" QNAP -u -d 1` = 1 ]; then
		if [ -f /var/qfunc/.QTS ]; then
			BROWSENAME="$HOSTNAME"
		else
			BROWSENAME=`/sbin/getcfg "Bonjour Service" QNAP -d "$HOSTNAME"`
		fi
		atype="http";
		aport=${WEB_PORT};
		if [ `/sbin/getcfg "Stunnel" "Enable" -u -d 1` = 1  -a `/sbin/getcfg "System" "Force SSL" -u -d 0` = 1 ]; then
			atype="https"
			aport=${HTTPS_PORT}
		fi
		upnp_pf=`/sbin/getcfg UPnP_Global Enable -d FALSE -f /etc/config/upnpc.conf`
		webadm_ext_port=`/sbin/getcfg System ExtPort -d 0`
		web_ext_port=`/sbin/getcfg QWEB ExtPort -d 0`
		web_ssl_ext_port=`/sbin/getcfg QWEB ExtSSLPort -d 0`
		webadm_ssl_ext_port=`/sbin/getcfg Stunnel ExtSSLPort -d 0`

		qTXT="accessType=${atype},accessPort=${aport},model=${MODEL},displayModel=${DISPLAY_MODEL},fwVer=${FW_VER},fwBuildNum=${FW_BN}"
		if [ x != x"$SN" ]; then
			qTXT="${qTXT},serialNum=${SN}"
		fi
		if [ x != x"$MYCLOUDNAS_NAME" -a x != x"$MYCLOUDNAS_DN" ]; then
			qTXT="${qTXT},myCloudNASName=${MYCLOUDNAS_NAME},myCloudNASDomain=${MYCLOUDNAS_DN}"
		fi
		if [ x$upnp_pf = xTRUE ]; then
			qTXT="${qTXT},webAdmPort=${webadm_ext_port},webAdmSslPort=${webadm_ssl_ext_port},webPort=${web_ext_port},webSslPort=${web_ssl_ext_port}"
		fi
		
/bin/cat > ${AVAHI_CONF_DIR}/tbt.qdiscover.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$BROWSENAME(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_qdiscover._tcp</type>
<port>${WEB_PORT}</port>
<txt-record>$qTXT</txt-record>
</service>
</service-group>
__EOF__
	fi
	
	if [ `/sbin/getcfg iTune Enable -u -d FALSE` = TRUE ]; then
		_iTunes_ID=`/sbin/get_hwsn`
		if [ $? != 0 ]; then
			_iTunes_ID=`/sbin/uuidgen | /bin/cut -c 1-8`
		else
			len=${#_iTunes_ID}
			if [ ${len} -gt 16 ]; then
				sublen=$(( ${len}-16+1  ))
				_iTunes_ID=`echo ${_iTunes_ID} | cut -c ${sublen}-${len}`
			fi
		fi
		if [ "x`/sbin/getcfg iTune "Password Enabled" -u -d 0`" = "x1" ]; then
			export _Passowrd=true
		else
			export _Passowrd=false
		fi
/bin/cat > ${AVAHI_CONF_DIR}/tbt.itunes.service <<__EOF__
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$HOSTNAME(iTunes)(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_daap._tcp</type>
<port>3689</port>
<txt-record>txtvers=1</txt-record>
<txt-record>Machine Name=$HOSTNAME(iTunes)</txt-record>
<txt-record>Database ID=${_iTunes_ID}</txt-record>
<txt-record>iTSh Version=131073</txt-record>
<txt-record>Version=196610</txt-record>
<txt-record>Passowrd=${_Passowrd}</txt-record>
</service>
</service-group>

__EOF__
	fi

for i in 1 2 3 4 5 6 7 8 9 10; do
[ $i -eq 1 ] && i=""
	if [ `/sbin/getcfg "Bonjour Service" "PRINTER$i" -u -d 0` = 1 ]; then
		local phy_exist=`/sbin/getcfg "Printers" "Phy exist$i"`
		[ "x$phy_exist" = "x" ] || [ $phy_exist -eq 0 ] && continue
		local status=`/sbin/getcfg "Printers" "Status$i"`
		[ $status -eq 4 ] && continue
		local service_name=`/sbin/getcfg "Bonjour Service" "PRINTER Service Name$i"`
		local printer_port=`/bin/cat /etc/config/cups/cupsd.conf 2>>/dev/null | /bin/grep "Listen" | /bin/cut -d':' -f 2 | /usr/bin/head -n 1`
		local printer_name=`/sbin/getcfg "Printers" "Name$i"`
		local manufacture=`/sbin/getcfg "Printers" "Manufacture$i"`
		local model=`/sbin/getcfg "Printers" "Model$i"`
/bin/cat > ${AVAHI_CONF_DIR}/tbt.printer$i.service <<__EOF__
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">$service_name(Airprint)(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_ipp._tcp</type>
<subtype>_universal._sub._ipp._tcp</subtype>
<port>$printer_port</port>
<txt-record>txtvers=1</txt-record>
<txt-record>qtotal=1</txt-record>
<txt-record>priority=0</txt-record>
<txt-record>rp=printers/$printer_name</txt-record>
<txt-record>ty=$manufacture $model</txt-record>
<txt-record>note=$manufacture $model</txt-record>
<txt-record>product=(GPL Ghostscript)</txt-record>
<txt-record>pdl=application/pdf,application/postscript,application/vnd.cups-raster,application/octet-stream,image/png</txt-record>
<txt-record>URF=none</txt-record>
</service>
</service-group>

__EOF__
	fi
done

if [ `/sbin/getcfg "Appletalk" Enable -u -d FALSE` = "TRUE" ]; then
	ps | grep "/usr/local/sbin/afpd" | grep -v grep > /dev/null 2>&1
	afp_exist=$?
	if [ "x${afp_exist}" = "x0" ]; then
		if [ `/sbin/getcfg "TimeMachine" Enabled -u -d FALSE` = "TRUE" ]; then
			STOR_TYPE=`/sbin/getcfg "TimeMachine" "Storage Type" -d 1`
			if [ "x$STOR_TYPE" = "x1" ]; then
				TM_VN=TMBackup
			else
				TM_VN=home
			fi
			AD_CHECK=`/sbin/getcfg global "server role" -d "x" -f /etc/config/smb.conf`
			if [ "$AD_CHECK" == "active directory domain controller" ]; then
				TM_ADVF=0xa1
			else
				LDAP_ENABLE=`/sbin/getcfg "LDAP" "Enable" -d "FALSE"`
				SAMBA_AUTH=`/sbin/getcfg global "passdb backend" -d "x" -f /etc/config/smb.conf`
				if [ "x$LDAP_ENABLE" == "xTRUE" ] && [ "$SAMBA_AUTH" != "smbpasswd" ]; then
					TM_ADVF=0xa1
				else
					TM_ADVF=0xa3
				fi
			fi
			uuid=`grep \"${TM_VN}\" /var/netatalk/afp_voluuid.conf | cut -f 2 -d$'\t'`
			if [ "x${uuid}" != "x" ]; then
/bin/cat > ${AVAHI_CONF_DIR}/tbt.timemachine.service <<__EOF__
<?xml version="1.0" standalone='no'?><!--*-nxml-*-->
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
<name replace-wildcards="yes">${HOSTNAME}(TimeMachine)(Thunderbolt)</name>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_adisk._tcp</type>
<port>9</port>
<txt-record>dk0=adVN=${TM_VN},adVF=${TM_ADVF},adVU=${uuid}</txt-record>
__EOF__

_renew_timemachine_share_folder ${TM_ADVF} "${AVAHI_CONF_DIR}/tbt.timemachine.service"

/bin/cat >> ${AVAHI_CONF_DIR}/tbt.timemachine.service <<__EOF__
</service>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_smb._tcp</type>
<port>445</port>
</service>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_afpovertcp._tcp</type>
<port>548</port>
</service>
<service>
<host-name>${TBTHOSTNAME}</host-name>
<type>_device-info._tcp</type>
<port>0</port>
<txt-record>model=Xserve</txt-record>
</service>
</service-group>

__EOF__
			fi
		fi
	fi
fi
}

set_avahi_thunderbolt_host()
{
	ip=`/sbin/ifconfig $1 | grep "inet addr" | cut -f 2 -d ':' | cut -f 1 -d ' '`
	tbtpostfix="tbt"
	tbthost="${2}${tbtpostfix}.local"
	
	if [ "x${ip}" != "x" ]; then
		if [ "x${1#*tbtbr}" != "x${1}" ]; then
			echo "${ip} ${tbthost}" >> ${AVAHI_DIR}/hosts
		fi
	fi
}

set_avahi_thunderbolt_hosts()
{
	hostname=`/sbin/getcfg System "Server Name"`
	
	cd /sys/class/net/

	for i in *
	do
		if [ "x${i#*tbtbr}" != "x${i}" ]; then
			set_avahi_thunderbolt_host ${i} ${hostname}
		fi
	done
}

clean_avahi_thunderbolt_hosts()
{
/bin/cat > ${AVAHI_DIR}/hosts <<__EOF__
# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

# This file contains static ip address <-> host name mappings.  These
# can be useful to publish services on behalf of a non-avahi enabled
# device. Please bear in mind that host names are expected to be
# fully qualified domain names, i.e. ending in .local!

# See avahi.hosts(5) for more information on this configuration file!

# Examples:
# 192.168.0.1 router.local
# 2001::81:1 test.local
__EOF__
}

dbus_reload()
{
	DBUS_PID=`/bin/cat ${DBUS_PID_FILE} 2>>/dev/null`
	kill -s HUP $DBUS_PID 2>/dev/null
}

dbus_start()
{
    local dbus_status=`/usr/sbin/check_dbus`
    if [ "${dbus_status}" == "active" ]; then
		echo "Dbus is already running"
		return 0
	fi
	
	if [ -L /var/run/dbus ] || [ -d ${AVAHI_SRC}/var/run/dbus ]; then
		/bin/rm -rf /var/run/dbus
		/bin/rm -rf ${AVAHI_SRC}/var/run/dbus
	fi

	/bin/mkdir -p ${AVAHI_SRC}/var/run/dbus
	/bin/ln -sf ${AVAHI_SRC}/var/run/dbus /var/run/dbus

	/bin/echo -n "Dbus "
	${DBUS_DAEMON} --system
	/bin/sleep 1
	/bin/kill -HUP `/bin/cat ${DBUS_PID_FILE} 2>>/dev/null` 2>/dev/null 1>/dev/null
	/bin/sleep 1
}

dbus_stop()
{
	DBUS_PID=`/bin/cat ${DBUS_PID_FILE} 2>>/dev/null`
	if [ ! -z "$DBUS_PID" ]; then
		DBUS_PID=`/bin/cat ${DBUS_PID_FILE} 2>>/dev/null`
		/bin/echo -n "Dbus "
		/bin/kill -9 $DBUS_PID 2>/dev/null 1>/dev/null
		/bin/sleep 1
	fi
	
	/bin/rm -rf /var/run/dbus
	/bin/rm -rf $AVAHI_SRC/var/run/dbus
}

avahi_hosts_reload()
{
if [ -x /sbin/hal_app ]; then
	if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ];then
		clean_avahi_thunderbolt_hosts
		set_avahi_thunderbolt_hosts
	fi
fi
}

avahi_service_reload()
{
	if [ x`/sbin/getcfg "Bonjour Service" Enable -d TRUE` != "xFALSE" ]; then
	if [ -x /sbin/hal_app ]; then
		if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
			clean_avahi_thunderbolt_conf
			if [ x`/bin/ls /sys/class/net/ | grep tbtbr` != "x" ]; then
				set_avahi_thunderbolt_conf
			fi
		fi
	fi
		set_avahi_conf
	fi
}

avahi_start()
{
	if [ -f ${AVAHI_LOCK_FILE} ]; then
		/bin/kill -INT `/bin/cat ${AVAHI_PID_FILE} 2>/dev/null` 2>/dev/null 1>/dev/null
		/bin/rm -f ${AVAHI_LOCK_FILE}
	fi
	if [ -f /etc/config/timemachine.conf ]; then
		cp /etc/config/timemachine.conf /etc/config/timemachine.conf-pre_avahi 2>/dev/null
	fi
	[ -f ${AVAHI_DAEMON} ] || exit 1

	/bin/echo -n "Starting services: "

	/bin/kill -INT `/bin/cat ${AVAHI_PID_FILE} 2>/dev/null` 2>/dev/null 1>/dev/null
	/bin/rm -rf /var/run/avahi-daemon
	/bin/rm -rf ${AVAHI_SRC}/var/run/avahi-daemon
	mkdir -p ${AVAHI_SRC}/var/run/avahi-daemon
	ln -sf ${AVAHI_SRC}/var/run/avahi-daemon /var/run/avahi-daemon

	/bin/sleep 2

	# create the lock file 
	touch ${AVAHI_LOCK_FILE}
	/sbin/ldconfig

	if [ x`${GETCFG} "Bonjour Service" Enable -d TRUE` != "xFALSE" ]; then
		/bin/echo -n "Avahi "
		if [ -x /sbin/hal_app ]; then
			if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
				clean_avahi_thunderbolt_conf
				clean_avahi_thunderbolt_hosts
				if [ x`/bin/ls /sys/class/net/ | grep tbtbr` != "x" ]; then
					set_avahi_thunderbolt_conf
					set_avahi_thunderbolt_hosts
				fi
			fi
		fi
		set_avahi_conf
		/bin/sleep 2
        if [ "$(wait_dbus_ready 5)" == "inactive" ]; then
            echo "Restart dbus"
            dbus_stop
            dbus_start
        fi
		# work around for Bug 213791
		/bin/sed -i 's/#allow-interfaces=eth0/allow-interfaces=eth0/g' /etc/avahi/avahi-daemon.conf
		${AVAHI_DAEMON} -D
		/bin/sleep 3
		CURRENT_AVAHI_PID=`/bin/cat ${AVAHI_PID_FILE} 2>/dev/null`
		/bin/kill ${CURRENT_AVAHI_PID} 2>/dev/null 1>/dev/null
		/bin/sed -i 's/allow-interfaces=eth0/#allow-interfaces=eth0/g' /etc/avahi/avahi-daemon.conf
		# end of work around for Bug 213791		

		${AVAHI_DAEMON} -D
		/bin/sleep 3
		CURRENT_AVAHI_PID=`/bin/cat ${AVAHI_PID_FILE} 2>/dev/null`
		if [ -z "$CURRENT_AVAHI_PID" ] ; then
			#/sbin/write_log "Error: Avahi failed to start." 1
			${AVAHI_DAEMON} -D
		fi
	fi
	if [ "x"`${GETCFG} TimeMachine Enabled -d FALSE` = "xTRUE" ]; then
		/etc/init.d/atalk.sh reload
	fi
	/bin/echo " OK"
}

avahi_stop()
{
	if [ ! -f ${AVAHI_LOCK_FILE} ]; then
		#Avahi hasn't been enabled or started
		return 0
	fi
	/bin/echo -n "Shutting down services: "
	/bin/echo -n "Avahi "
	/bin/kill -INT `/bin/cat ${AVAHI_PID_FILE} 2>/dev/null` 2>/dev/null 1>/dev/null
	/bin/rm -rf ${AVAHI_PID_FILE}
	/bin/rm -rf /var/run/avahi-daemon/socket
	/bin/rm -rf /etc/config/timemachine.conf-pre_avahi
	/bin/rm -f ${AVAHI_LOCK_FILE}
}

avahi_reload()
{
	CURRENT_AVAHI_PID=`/bin/cat ${AVAHI_PID_FILE} 2>/dev/null`
	if [ ! -z "$CURRENT_AVAHI_PID" ]; then
		if [ -f ${AVAHI_SET_LOCK_FILE} ]; then
			exit 0;
		fi
		touch ${AVAHI_SET_LOCK_FILE}
		avahi_service_reload
		avahi_hosts_reload
		/bin/kill -HUP $CURRENT_AVAHI_PID 2>/dev/null 1>/dev/null
		rm -f ${AVAHI_SET_LOCK_FILE}
	fi
}

case "$1" in
	start)
		avahi_start
		;;
	stop)
		avahi_stop
		;;
	restart)
		avahi_stop
		avahi_start
		;;
	reload)
		avahi_reload
		;;
	dbus_start)
		/bin/echo -n "Starting services: "
		dbus_start
		/bin/echo " OK"
		avahi_start
		if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
			if [ -f /etc/init.d/thunderbolt.sh ]; then
				/etc/init.d/thunderbolt.sh daemon_restart
			fi
		fi
		;;
	dbus_stop)
		if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
			if [ -f /etc/init.d/thunderbolt.sh ]; then
				/etc/init.d/thunderbolt.sh daemon_stop
			fi
		fi
		avahi_stop
		dbus_stop
		;;
	dbus_restart)
		if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
			if [ -f /etc/init.d/thunderbolt.sh ]; then
				/etc/init.d/thunderbolt.sh daemon_stop
			fi
		fi
		CURRENT_AVAHI_PID=`/bin/cat ${AVAHI_PID_FILE} 2>>/dev/null`
		if [ ! -z "$CURRENT_AVAHI_PID" ] ; then
			avahi_stop
		fi
		dbus_stop
		/bin/echo -n "Starting services: "
		dbus_start
		/bin/echo " OK"
		if [ ! -z "$CURRENT_AVAHI_PID" ] ; then
			avahi_start
		fi
		if [ "x$(/sbin/hal_app --check_tbt_support)" = "xyes" ]; then
			if [ -f /etc/init.d/thunderbolt.sh ]; then
				/etc/init.d/thunderbolt.sh daemon_restart
			fi
		fi
		;;
	dbus_reload)
		dbus_reload
		;;
	hosts_reload)
		avahi_hosts_reload
		;;
	*)
		/bin/echo "Usage: $0 {start|stop|restart|dbus_start|dbus_stop|dbus_restart|dbus_reload|hosts_reload}"
		exit 1
esac

exit 0
