#!/bin/sh
#
### BEGIN INIT INFO
# Provides: dhcp6s
# Default-Start:
# Default-Stop:
# Should-Start:
# Required-Start: $network
# Required-Stop:
# Short-Description: Start and stop the DHCPv6 server agent
# Description: dhcp6s provides IPv6 addresses and prefix assignment
#              administrative policy and configuration information for
#              DHCPv6 clients.  dhcp6s also manages those addresses and
#              prefixes, such as IPv6 addresses, prefixes, DNS server
#              addresses, or ntp server addresses.
### END INIT INFO
#
# The fields below are left around for legacy tools (will remove later).
#
# chkconfig: - 66 36
# description: dhcp6s provides IPv6 addresses and prefix assignment \
#              administrative policy and configuration information for \
#              DHCPv6 clients.  dhcp6s also manages those addresses and \
#              prefixes, such as IPv6 addresses, prefixes, DNS server \
#              addresses, or ntp server addresses.
# processname: dhcp6s
# config: /etc/dhcp6s.conf
# config: /etc/sysconfig/dhcp6s

. /etc/init.d/functions

RETVAL=0

prog=dhcp6s
dhcp6s=/usr/sbin/dhcp6s
lockfile=/var/lock/subsys/dhcp6s

# Check that networking is up.
# networking is not up, return 1 for generic error
. /etc/sysconfig/network
[ $NETWORKING = "no" ] && exit 1

start() {
    # return 5 if program is not installed
    [ -x $dhcp6s ] || exit 5

    # return 6 if program is not configured
    [ -f /etc/dhcp6s.conf ] || exit 6
    [ -f /etc/sysconfig/dhcp6s ] || exit 6
    . /etc/sysconfig/dhcp6s

    if [ -z "$DHCP6SIF" ]; then
        logger -s -t "$prog" -p "daemon.info" "Warning: $prog listening on ALL interfaces"
    fi

    echo -n $"Starting $prog: "
    daemon $dhcp6s -c /etc/dhcp6s.conf $DHCP6SARGS $DHCP6SIF
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && touch $lockfile
    return $RETVAL
}

stop() {
    echo -n $"Shutting down $prog: "
    killproc $prog -TERM
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && success || failure
    echo
    rm -f $lockfile
    return $RETVAL
}

# See how we were called.
case "$1" in
    start)
        start
        RETVAL=$?
        ;;
    stop)
        stop
        RETVAL=$?
        ;;
    restart|force-reload)
        [ -f $lockfile ] && stop
        start
        RETVAL=$?
        ;;
    try-restart|reload)
        RETVAL=3
        ;;
    condrestart)
        if [ -f $lockfile ]; then
            stop && start
        fi
        ;;
    status)
        status $prog
        RETVAL=$?
        ;;
    *)
        echo $"Usage: $0 {start|stop|restart|try-restart|reload|force-reload|status}"
        RETVAL=3
        ;;
esac

exit $RETVAL
