#!/bin/sh
#--------------------------------------------------------------------
# /usr/local/bin/oac.sh - control oac from commandline
#
# Creation:     26.01.2010 rresch
# Last Update:  $Id$
#--------------------------------------------------------------------

usage ()
{
    echo "Usage: $this list"
    echo "       $this status [<name>|<number>]"
    echo "       $this show-filter"
    echo "       $this block [limit=<minutes>] <name>|<number>"
    echo "       $this unblock [limit=<minutes>] <name>|<number>"
    echo "       $this blockgroup [limit=<minutes>] <name>|<number>"
    echo "       $this unblockgroup [limit=<minutes>] <name>|<number>"
}

abort ()
{
    rm -f $tmp.*
    echo "$1"
    exit 255
}

show_hook ()
{
    local chain=$1
    local hook=$2
    iptables -nvL $chain --line-numbers | grep -e ^Chain -e '^.*pkts' -e $hook
    echo
}
show_chain ()
{
    local chain=$1
    iptables -nvL $chain --line-numbers
    echo
}

reverseaction ()
{
    case $1 in
	block) echo unblock;;
	unblock) echo block;;
	blockgroup) echo unblockgroup;;
	unblockgroup) echo blockgroup;;
    esac
}

get_target ()
{
    set `sed -n "/^${target} \| ${target} /{p;q;}" $configfile` >/tmp/$$tmp_oac_get_target
    if [ ! -s /tmp/$$tmp_oac_get_target ]
    then
      rm /tmp/$$tmp_oac_get_target
      num=$1
      name=$2
      ip4=$3
      mac=$4
      mac2=$5
      [ "$num" ] || abort "Unknown target '$target'"
      [ "$ip4" ] || abort "Invalid target '$target'"
    else
      rm /tmp/$$tmp_oac_get_target
      abort "Unknown target '$target'"
    fi
}

status ()
{
    fwstat="open"
    case $blocked in
        *" $name "*)  fwstat="blocked" ;;
    esac

    status=offline
    exitcode=1
    case $online in
        *" $ip4 "*)
            status=online
	    exitcode=0
            ;;
    esac

    case $name in
        *-*) n=`echo $name | sed -e 's/-/_/g'` ;;
        *)   n=$name ;;
    esac
    limit=none
    eval timestamp=\$_timestamp_$n
    if [ "$timestamp" ]; then
        remains=0
        if [ "$timestamp" -gt $nowuts ]; then
	    remains=`expr $timestamp - $nowuts`
        fi
        limit=`date -u -d @$remains +%H:%M`
    fi

    echo "num=$num name=$name IP4=$ip4 state=$status fwstat=$fwstat limit=$limit"
}

#
# get a list of online hosts
#
get_online()
{
    online=
    while read ip type flags other; do
        case "$flags" in
            0x2) online="$online $ip " ;;
        esac
    done < /proc/net/arp
}

#
# get a list of blocked clients
#
get_blocked()
{
    blocked=
    for i in `iptables -nvL oac-fw | sed -ne '/.* client-[0-9]\+ name .*/{s/.* client-[0-9]\+ name //;s/ .*//p}'`; do
        blocked="$blocked $i "
    done
}

#
# get a list of hosts with limits
#
get_timestamps()
{
    for f in /var/spool/atd/*; do
        [ -f $f ] || continue
        n=`sed -ne '/^#oac client-[0-9]\+ name /{s/^#oac client-[0-9]\+ name //;s/-/_/g;p}' $f`
        l=${f##*/}
        l=${l%%-*}
        eval _timestamp_$n=$l
    done
}

# initialise Variables
this="`basename $0`"
tmp="/tmp/$this.$$"
configfile="/var/run/oac.cfg"
configfile_groups=/var/run/oac-groups.cfg
exitcode=0
action="$1"
case $2 in
    limit=*)
    limit=`echo $2 | sed 's/limit=//'`
    target="$3"
    ;;
    *)
        limit=0
        target=$2
        ;;
esac

case "$action" in
    block* | unblock*)
        if [ ! "$target" ]; then
            usage
            exit 1
        fi
        ;;
esac

case "$action" in
    list)
        while read num name ip4 mac mac2
        do
          if [ -n "$mac2" ]
          then
            echo "num=$num name=$name IPv4=$ip4 mac=$mac mac2=$mac2"
          else
            echo "num=$num name=$name IPv4=$ip4 mac=$mac"
          fi
        done <$configfile
        ;;

    status)
        nowuts=`date +%s`
        get_timestamps
        get_blocked
        get_online
        while read num name ip4 mac mac2
        do
            case $target in
                '' | "$num" | "$name") ;;
                *) continue ;;
            esac
            status
        done < $configfile
        ;;

    show-filter)
	show_hook INPUT oac-inp
	show_chain oac-inp
	show_hook FORWARD oac-fw-pre
	show_chain oac-fw-pre
	show_chain oac-fw
	if show_chain oac-fw-pre | grep -q oac-fw-knw || show_chain oac-inp | grep -q oac-fw-knw ; then
	    show_chain oac-fw-knw
	fi
	;;

    block)
        get_target $target
	filename=`grep -ls "oac client-$num name $name" /var/spool/atd/*`
	if [ -n "$filename" ]
	then
	    rm $filename
	fi
	if [ $limit -gt 0 ]
	then
	    /usr/local/bin/atd.sh add +`expr $limit \* 60` "$0 unblock $num" "oac client-$num name $name"
	fi
	if ! iptables -nvL oac-fw | grep -q "client-$num name $name "
	then
	    . /etc/boot.d/base-helper
	    . /etc/rc.d/fwrules-helper
            [ -f /var/run/oac-reject ] && act=REJECT || act=DROP
	    fw_append_rule filter oac-fw "$ip4 $act BIDIRECTIONAL" "client-$num name $name"
	    if [ -n "$mac" ]
	    then
	        fw_append_rule filter oac-fw "mac:$mac $act" "client-$num name $name"
	        if [ -n "$mac2" ]
	        then
		    fw_append_rule filter oac-fw "mac:$mac2 $act" "client-$num name $name"
	        fi
	    fi
	else
	    if [ $limit -eq 0 ]
	    then
	        echo "unable to block: already blocked"
	        exitcode=1
	    fi
	fi
        ;;

    unblock)
        get_target $target
	filename=`grep -ls "oac client-$num name $name" /var/spool/atd/*`
	if [ -n "$filename" ]
	then
	    rm $filename
	fi
	if [ $limit -gt 0 ]
	then
	    /usr/local/bin/atd.sh add +`expr $limit \* 60` "$0 block $num" "oac client-$num name $name"
	fi
	if iptables -nvL oac-fw | grep -q "client-$num name $name "
	then
	    . /etc/boot.d/base-helper
	    . /etc/rc.d/fwrules-helper
	    [ -f /var/run/oac-reject ] && act=REJECT || act=DROP
            if [ -n "$mac" ]
	    then
	        fw_delete_rule filter oac-fw "mac:$mac $act" "client-$num name $name"
	        if [ -n "$mac2" ]
	        then
		    fw_delete_rule filter oac-fw "mac:$mac2 $act" "client-$num name $name"
	        fi
	    fi
	    fw_delete_rule filter oac-fw "$ip4 $act BIDIRECTIONAL" "client-$num name $name"
	else
	    if [ $limit -eq 0 ]
	    then
	        echo "unable to unblock: already free"
	        exitcode=1
	    fi
	fi
        ;;

    blockgroup|unblockgroup)
	OAC_GROUP_N=0
	if [ -f $configfile_groups ]
	then
	    . $configfile_groups
	fi
        group=${target}
	type=number
        expr ${target} + 0 >/dev/null 2>&1 || type="name"
	if [ "$type" = "name" ]
	then
	    parameter=$group
	    group=0
	    for i in `seq 1 $OAC_GROUP_N`
	    do
	        eval name=\$OAC_GROUP_${i}_NAME
	        if [ "$name" = "$parameter" ]
	        then
	            group=$i
	        fi
	    done
	fi
	if [ $group -gt 0 -a $group -le $OAC_GROUP_N ]
	then
	    act=`echo $action | sed 's/group//'`
	    eval numclients=\$OAC_GROUP_${group}_CLIENT_N
	    for i in `seq 1 $numclients`
	    do
	        eval client=\$OAC_GROUP_${group}_CLIENT_$i
	        if [ $limit -gt 0 ]
	        then
	            $0 $act limit=$limit $client >/dev/null 2>&1
	        else
	            $0 $act $client >/dev/null 2>&1
	        fi
	    done
	fi
        ;;
    *) usage ;;
esac

rm -f $tmp.*
exit $exitcode
