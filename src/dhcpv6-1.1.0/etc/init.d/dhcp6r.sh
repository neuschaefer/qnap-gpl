#!/bin/sh
#
### BEGIN INIT INFO
# Provides: dhcp6r
# Default-Start:
# Default-Stop:
# Should-Start:
# Required-Start: $network
# Required-Stop:
# Short-Description: Start and stop the DHCPv6 relay agent
# Description: dhcp6r acts as a DHCPv6 relay agent forwarding DHCPv6 messages
#              from clients to servers and vice versa.
### END INIT INFO
#
# The fields below are left around for legacy tools (will remove later).
#
# chkconfig: - 66 36
# description: dhcp6r acts as a DHCPv6 relay agent forwarding DHCPv6 messages \
#              from clients to servers and vice versa.
# processname: dhcp6r
# config: /etc/sysconfig/dhcp6r

. /etc/init.d/functions

RETVAL=0

prog=dhcp6r
dhcp6r=/usr/sbin/dhcp6r
lockfile=/var/lock/subsys/dhcp6r

# Check that networking is up.
# networking is not up, return 1 for generic error
. /etc/sysconfig/network
[ $NETWORKING = "no" ] && exit 1

start() {
    # return 5 if program is not installed
    [ -x $dhcp6r ] || exit 5

    # return 6 if program is not configured
    [ -f /etc/sysconfig/dhcp6r ] || exit 6
    . /etc/sysconfig/dhcp6r

    echo -n $"Starting $prog: "
    daemon $dhcp6r $DHCP6RARGS
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
        status dhcp6r
        RETVAL=$?
        ;;
    *)
        echo $"Usage: $0 {start|stop|restart|try-restart|reload|force-reload|status}"
        RETVAL=3
        ;;
esac

exit $RETVAL
