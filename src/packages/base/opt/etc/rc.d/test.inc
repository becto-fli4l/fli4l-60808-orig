#!/bin/sh
test_cleanup ()
{
	rm -f $net_alias_conf $fwh_state
}
netcalc ()
{
    echo $2
}
dump ()
{
    echo > /dev/null
}

show ()
{
    res=
    echo -n "${indent}iptables "
    for i in "$@"
    do
        case "$i" in
            *' '*) echo -n "'$i' " ;;
            *) echo -n "$i " ;;
        esac
    done | sed -e 's#-[sd] 0.0.0.0/0 ##g'
    echo
    iptables_rules_added=`expr $iptables_rules_added + 1`
}

trap test_cleanup EXIT 
IPTABLES="dump"
fwh_state=/tmp/fwrules-helper.state
tmpl_dir=../fwrules.tmpl
ext_dir=.

IP_NET_N=1
IP_NET_1_DEV=eth0
IP_NET_1=1.2.3.4/24
IP_ROUTE_1="9.9.9.9/9 eth0"
IP_ROUTE_2="0.0.0.0/0 eth0"

net_alias=../boot.d/network_aliases
net_alias_conf=/tmp/netaliases.conf
> $net_alias_conf
. ../boot.d/base-helper

SCRIPT=$script_name

log_error () {
    case $# in
        0)
            while read line
            do
                echo "$SCRIPT: $line"  1>&2
            done
            ;;
        *)
            echo "$SCRIPT: $*"  1>&2
            ;;
    esac
}

. fwrules-helper
: ${LOG:=yes}
setup_logging $LOG INPUT   fw-input   1/minute:1 2/minute:2 3/minute:3 5
setup_logging $LOG FORWARD fw-forward 1/minute:1 2/minute:2 3/minute:3 5

IPTABLES=show
case $1 in
    -d | -x)
        shift
        set -x
        FWRULES_DO_DEBUG='yes'
        ;;
esac

