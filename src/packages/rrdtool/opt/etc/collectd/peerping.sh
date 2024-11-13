#!/bin/sh

. /etc/boot.d/base-helper

echo $$ > /var/run/peerping.pid
mkdir -p /var/run/peerping

logger -t "rrdtool peerping.sh [$$]" "peerping.sh was started"

HOSTNAME="${COLLECTD_HOSTNAME:-`hostname`}"
INTERVAL=${COLLECTD_INTERVAL%.*}

tmp_peer=/tmp/vpnping.val.peer.$$
tmp_ping=/tmp/vpnping.val.ping.$$

remove_runfiles ()
{
    rm -f $tmp_peer
    rm -f $tmp_ping
    rm -f /var/run/peerping.pid
}

sighandler_sigint ()
{
    remove_runfiles
    logger -t "rrdtool peerping.sh [$$]" "exiting because signal SIGINT"
    exit 1
}

sighandler_sigterm ()
{
    remove_runfiles
    logger -t "rrdtool peerping.sh [$$]" "exiting because signal SIGTERM"
    exit 1
}

trap 'sighandler_sigint' 2
trap 'sighandler_sigterm' 15

while [ true ]
do
    start=`date +%s`

    . /var/run/rrdtool_peerping.conf
    . /var/run/netaliases.conf

    if [ -f /var/run/peerping.pid ]
    then
        oldpid=`cat /var/run/peerping.pid`
        if [ "$$" != "$oldpid" ]
        then
            #exit because new prozess was started by collectd
            logger -t "rrdtool peerping.sh [$$]" "exiting because a other peerping.sh prozess with pid [$oldpid] was started"
            remove_runfiles
            exit 1
        fi
    else
        logger -t "rrdtool peerping.sh [$$]" "warning: missing pid-file - recreating the pid-file"
        echo $$ > /var/run/peerping.pid
    fi
    peers=""
    > $tmp_peer
    for idx in `seq 1 $RRDTOOL_PEERPING_N`
    do
        eval peer='$RRDTOOL_PEERPING_'$idx
        peerdev=""
        peerip=""
        net_alias_lookup_dev $peer peerdev
        if [ "x$peerdev" != "x" ]
        then
            if [ -f /var/run/peerping/$peer.nodev ]
            then
                rm /var/run/peerping/$peer.nodev
            fi
            peerip=`ip addr show $peerdev 2>/dev/null | sed -n 's#.*peer \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\)/32.*#\1#p'`
            if [ "x$peerip" != "x" ]
            then
                if [ -f /var/run/peerping/$peer.nopeerip ]
                then
                    rm /var/run/peerping/$peer.nopeerip
                fi
                echo "$peerdev: $peerip" >> $tmp_peer
                peers="$peers $peerip"
            else
                if [ ! -f /var/run/peerping/$peer.nopeerip ]
                then
                    > /var/run/peerping/$peer.nopeerip
                    logger -t "rrdtool peerping.sh [$$]" "warning: no peer-ip found for vpnpeer $peer on device $peerdev"
                fi
            fi
        else
            if [ ! -f /var/run/peerping/$peer.nodev ]
            then
                > /var/run/peerping/$peer.nodev
                logger -t "rrdtool peerping.sh [$$]" "warning: function net_alias_lookup_dev does not find a device for vpnpeer $peer"
            fi
        fi
    done
    
    if [ "x$peers" != "x" ]
    then
        fping -u -c 5 -p 500 $peers 2>&1 | tr -s ' ' ' ' | cut -d ' ' -f1,8 > $tmp_ping
    else
        > $tmp_ping
    fi
    
    for idx in `seq 1 $RRDTOOL_PEERPING_N`
    do
        peerdev=""
        peerip=""
        min="U"
        avg="U"
        max="U"
        eval peer='$RRDTOOL_PEERPING_'$idx
        eval peername='$RRDTOOL_PEERPING_'$idx
        net_alias_lookup_dev $peer peerdev
        if [ "x$peerdev" != "x" ]
        then
            peerip=`grep "$peerdev: " $tmp_peer | cut -d " " -f2`
            if [ "x$peerip" != "x" ]
            then
                set -- `grep "^$peerip " $tmp_ping | sed 's#/# #g'`
                if [ "$4" ]
                then
                    min="$2"
                    avg="$3"
                    max="$4"
                fi
            fi
        fi
        
        ## Daten an collectd übergeben
        echo "PUTVAL $HOSTNAME/peerping/ping-avg-$peername interval=$INTERVAL N:$avg"
        echo "PUTVAL $HOSTNAME/peerping/ping-min-$peername interval=$INTERVAL N:$min"
        echo "PUTVAL $HOSTNAME/peerping/ping-max-$peername interval=$INTERVAL N:$max"
    done
    rm -f $tmp_peer
    rm -f $tmp_ping
    end=`date +%s`
    diff=`expr $end - $start`
    wait=`expr $INTERVAL - $diff`
    if [ $wait -ge 1 ]
    then
        if [ $wait -le $INTERVAL ]
        then
            sleep $wait
        else
            logger -t "rrdtool peerping.sh [$$]" "warning: loop waittime [$wait] is greater as the INTERVAL [$INTERVAL]"
        fi
    else
        logger -t "rrdtool peerping.sh [$$]" "warning: loop waittime [$wait] is less then 1sec"
        sleep $INTERVAL
    fi
done
