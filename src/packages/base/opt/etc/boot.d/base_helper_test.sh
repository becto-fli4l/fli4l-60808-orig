#!/bin/sh
base_log_file=/tmp/bootmsg.txt
base_colecho=echo
net_alias=./network_aliases
net_alias_conf=/tmp/net_alias.conf
rm -f $net_alias_conf
pwd
. ./base-helper

IP_NET_1=1.2.3.4/24
IP_ROUTE_1='1.2.3.4/24 eth0'
host=`hostname`
host_fqdn=`hostname -f`
host_ip=`host -t A $host | sed -e 's/.*[[:space:]]\([0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+\)/\1/'`

t ()
{
    val="$1"
    exp="$2"
    fail="$3"
    err_msg=/tmp/err.$$
    echo -n "$val -> $exp : "
    if translate_ip_net $val test_var 2> $err_msg; then
        if [ "$res" = "$exp" ]; then
            echo "OK"
        else
            echo "FAILED, got $res"
        fi

    else
        echo "FAILED"
    fi
    if [ -s $err_msg ]; then
        echo "    translate_ip_net output:" 
        sed -e 's/^/        /' $err_msg
    fi
    rm -f $err_msg
}

n ()
{
    post=$1
    val="$2"
    exp="$3"
    fail="$4"
    err_msg=/tmp/err.$$
    echo -n "'$val' -> '$exp' : "
    if net_alias_lookup_$post "$val" res 2> $err_msg; then
        if [ "$res" = "$exp" ]; then
            echo "OK"
        else
            echo "FAILED, got $res"
        fi

    else
        echo "FAILED"
    fi
    if [ -s $err_msg ]; then
        echo "    net_alias_lookup_$post output:" 
        sed -e 's/^/        /' $err_msg
    fi
    rm -f $err_msg
}

data()
{
    cat <<EOF
1.2.3.4 1.2.3.4
none none
default default
pppoe pppoe
dynamic dynamic
any 0.0.0.0/0
IP_NET_1 $IP_NET_1
IP_NET_1_IPADDR 1.2.3.4
IP_ROUTE_1 1.2.3.4/24
@$host      $host_ip
@$host_fqdn $host_ip
EOF
}

cat <<EOF
#
# tranlate_ip_net
#
EOF
data > /tmp/data.$$
while read val res; do
    t $val $res
done < /tmp/data.$$
rm -f /tmp/data.$$


cat <<EOF
#
# net_alias
#
EOF
net_alias_add "foo" eth1.2
n dev "foo" eth1.2
n name eth1.2 "foo"
net_alias_add "foo-bar   blah.moo" eth1.1
n dev "foo-bar   blah.moo" eth1.1
n name  eth1.1 "foo-bar   blah.moo"
