#!/bin/sh
#
# genpowerfail.sh	This script is run when the UPS tells the system
#			the power has gone. Tell everybody, ans start the 
#			shutdown based on the failure type.                
#			This script is also being run when the power comes
#			up again (if it does in time!)
#
# Version:	/etc/genpowerfail 1.0.1
#		/etc/init.d/genpowerfail.sh 1.0.2
#
# Author:	Tom Webster <webster@kaiwan.com>
#
#
# Modification:
#		2002/03/28 by Tiger Fu <tigerfu@iei.com.tw

# Set the path.
PATH=/sbin:/etc:/bin:/usr/bin

# Set location of upsstatus file
statpath="/etc/upsstatus"

# Set location of file containing PID of running shutdowns
spidpath="/etc/shutdownpid"

restart_time=0

POWER_MODE=`/sbin/getcfg UPS "Power Loss Mode" -d Standby`
# See what happened.
case "$1" in
    start)
# Called with a powerfail event, check to see if a shutdown is running, and stop it if it is.
        if [ -f $spidpath ]
        then
# Shutdown is running, kill it to process the new event
#			shutdown -c "Halting running shutdown to process power event...." &
#			kill `pidof halt`
            if [ $POWER_MODE = Shutdown ]
            then
                /bin/kill `/bin/pidof poweroff` 2>/dev/null
            else
                if [ -e "/tmp/ups_stop_service" ]; then
# check if the HAL subsystem exist
                    if [ -x /sbin/hal_app ]; then
                        /sbin/setcfg UPS "AC Power" OK
                    else
                        /sbin/config_util 1
                        /sbin/storage_boot_init 1
                        /sbin/setcfg UPS "AC Power" OK
                        /bin/umount /mnt/HDA_ROOT
                    fi
                    /sbin/reboot
                else
                    /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
                fi
            fi
        fi

        if [ $POWER_MODE = Shutdown ]
        then
            restart_time=`/sbin/getcfg UPS "Shutdown System In" -d 5`
        else
            restart_time=`/sbin/getcfg UPS "Standby System In" -d 5`
        fi

        echo $restart_time
# Get power problem and act on it
        if [ -r $statpath ]
        then
            stats=`head $statpath`
            echo $stats
            case "$stats" in
                FAIL)  # Power is down
                    if [ -f /var/run/shutdown.pid ]
                    then
#                  		kill `pidof shutdown`
#						kill `pidof halt`
                        if [ $POWER_MODE = Shutdown ]
                        then
                            /bin/kill `/bin/pidof poweroff` 2>/dev/null
                        else
                            if [ -e "/tmp/ups_stop_service" ]; then
# check if the HAL subsystem exist
                                if [ -x /sbin/hal_app ]; then
                                    /sbin/setcfg UPS "AC Power" OK
                                else
                                    /sbin/config_util 1
                                    /sbin/storage_boot_init 1
                                    /sbin/setcfg UPS "AC Power" OK
                                    /bin/umount /mnt/HDA_ROOT
                                fi
                                /sbin/reboot
                            else
                                /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
                            fi
                        fi
                    fi

#                   shutdown -h +$restart_time &
                    time=$restart_time
                    time=`expr ${time} \* 60`
#					/sbin/halt -d +${time} &
                    if [ $POWER_MODE = Shutdown ]
                    then
                        /sbin/poweroff -d +${time} &
                    else
                        /usr/sbin/ups_stop_service ${time} &
                    fi
#"THE POWER IS DOWN! SHUTTING DOWN SYSTEM! PLEASE LOG OFF NOW!" < /dev/console &
                    ;;
                SCRAM) # Battery is low
                    if [ -f /var/run/shutdown.pid ]
                    then
#                       kill `pidof shutdown`
#						kill `pidof halt`
                        if [ $POWER_MODE = Shutdown ]
                        then
                            /bin/kill `/bin/pidof poweroff` 2>/dev/null
                        else
                            if [ -e "/tmp/ups_stop_service" ]; then
# check if the HAL subsystem exist
                                if [ -x /sbin/hal_app ]; then
                                    /sbin/setcfg UPS "AC Power" OK
                                else
                                    /sbin/config_util 1
                                    /sbin/storage_boot_init 1
                                    /sbin/setcfg UPS "AC Power" OK
                                    /bin/umount /mnt/HDA_ROOT
                                fi
                                /sbin/reboot
                            else
                                /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
                            fi
                        fi
                    fi
#                   sutdown -h now &
                    time=$restart_time
                    time=`expr ${time} \* 60`
#                   /sbin/halt -d +${time} &
                    if [ $POWER_MODE = Shutdown ]
                    then
                        /sbin/poweroff -d +${time} &
                    else
                        /usr/sbin/ups_stop_service ${time} &
                    fi
#"THE POWER IS DOWN! BATTERY POWER IS LOW!  EMERGENCY SHUTDOWN!" < /dev/console &
                    ;;
                CABLE) # Possible bad cable
#					shutdown -c "Connectation Lose"
#					kill `pidof halt`
                    if [ $POWER_MODE = Shutdown ]
                    then
                        /bin/kill `/bin/pidof poweroff` 2>/dev/null
                    else
                        if [ -e "/tmp/ups_stop_service" ]; then
# check if the HAL subsystem exist
                            if [ -x /sbin/hal_app ]; then
                                /sbin/setcfg UPS "AC Power" OK
                            else
                                /sbin/config_util 1
                                /sbin/storage_boot_init 1
                                /sbin/setcfg UPS "AC Power" OK
                                /bin/umount /mnt/HDA_ROOT
                            fi
                            /sbin/reboot
                        else
                            /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
                        fi
                    fi
                    ;;
                *)     # Unknown message, assume power is down
                    if [ -f /var/run/shutdown.pid ]
                    then
#                       kill `pidof shutdown`
#						kill `pidof halt`
                        if [ $POWER_MODE = Shutdown ]
                        then
                            /bin/kill `/bin/pidof poweroff` 2>/dev/null
                        else	
                            if [ -e "/tmp/ups_stop_service" ]; then
# check if the HAL subsystem exist
                                if [ -x /sbin/hal_app ]; then
                                    /sbin/setcfg UPS "AC Power" OK
                                else
                                    /sbin/config_util 1
                                    /sbin/storage_boot_init 1
                                    /sbin/setcfg UPS "AC Power" OK
                                    /bin/umount /mnt/HDA_ROOT
                                fi
                                /sbin/reboot
                            else
                                /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
                            fi
                        fi
                    fi                                        

#                   shutdown -h +$restart_time &
                    time=$restart_time
                    time=`expr ${time} \* 60`
#                   /sbin/halt -d +${time} &
                    if [ $POWER_MODE = Shutdown ]
                    then
                        /sbin/poweroff -d +${time} &
                    else
                        /usr/sbin/ups_stop_service ${time} &
                    fi
#"THE POWER IS DOWN! SHUTTING DOWN SYSTEM! PLEASE LOG OFF NOW!" < /dev/console &
                    ;;
            esac
        else
# genowerfail called, and upsstatus dosen't exist.
# Assume user is using powerd, and shutdown.
#			shutdown -h +$restart_time &
            time=$restart_time
            time=`expr ${time} \* 60`
#           sbin/halt -d +${time} &
            if [ $POWER_MODE = Shutdown ]
            then
                /sbin/poweroff -d +${time} &
            else
                /usr/sbin/ups_stop_service ${time} &
            fi
#"THE POWER IS DOWN! SHUTTING DOWN SYSTEM! PLEASE LOG OFF NOW!" < /dev/console &
        fi
        ;;
    stop)
# Ok, power is good again. Say so on the console.
#echo "THE POWER IS BACK, GOING MULTI USER"

#		shutdown -c "THE POWER IS BACK"
#		kill `pidof halt`
        if [ $POWER_MODE = Shutdown ]
        then
            /bin/kill `/bin/pidof poweroff` 2>/dev/null
        else
            if [ -e "/tmp/ups_stop_service" ]; then
                if [ -x /sbin/hal_app ]; then
                    /sbin/setcfg UPS "AC Power" OK
                else
                    /sbin/config_util 1
                    /sbin/storage_boot_init 1
                    /sbin/setcfg UPS "AC Power" OK
                    /bin/umount /mnt/HDA_ROOT
                fi
                    /sbin/reboot
            else
                /bin/kill `/bin/pidof ups_stop_service` 2>/dev/null
            fi
        fi
        ;;
    *)
        echo "Usage: /etc/init.d/genpowerfail.sh {start|stop}"
        exit 1
        ;;
esac

exit 0
