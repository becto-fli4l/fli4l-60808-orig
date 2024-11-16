#!/bin/sh
# this script starts 3 qemu instances of fli4l. one as a pppoe-server, one as
# the actual router and one as a testing client.
#
# their connections are like this (numbers are ethX interfaces):
# +--------+    +--------+    +--------+
# | client 0----0 router 1----0 server 1----QEmu---Internet
# +--------+    +--------+    +--------+

update_version=

extract_value ()
{
    sed -n -e "s/^$1='\([^']*\)'.*/\1/p" $2/build/rc.cfg
}

usage ()
{
    [ "$1" ] && echo "Error: $1"
    cat <<EOF
usage:
    $0 [-u] <config dir>
        -u : set fli4l revision if installed from svn repository
    possible config dirs:
EOF
    for i in */base.txt; do
	if grep -q "HOSTNAME=" $i 2> /dev/null; then
	    echo "        `dirname $i`"
	fi
    done
    exit
}

configure_mtools ()
{
    case $1 in
	"$tmpdir/server-config") img="server.img";;
	"$tmpdir/client-config") img="client.img";;
	"$config") img="router.img";;
	*) echo "ERROR: $1"; exit;;
    esac
    img="$tmpdir/$img"
    cat > "$tmpdir/mtools.conf" <<-END
	drive a: file="$img" 1.44m mformat_only
	drive b: file="$img" 1.44m mformat_only
	END
    dd if=/dev/zero of="$img" bs=1024 count=1440 2>/dev/null
    export MTOOLSRC="$tmpdir/mtools.conf"
    mformat -t 80 -h 2 -s 18 a:
}

while [ "$1" ]; do
    case $1 in
	-u)
		update_version=-u
		;;
	*)      
		if [ -z "$config" ]; then
		    config=`echo $1 | sed -e 's#/*$##'`
		else
		    usage
		fi
		;;
    esac
    shift
done

if [ ! -f $config/base.txt ]; then
    usage
fi

: ${TMP:=/tmp}
if [ ! -w $TMP ]
then
	echo "Temporary directory '$TMP' not writable!"
	exit 1
fi

umask 077
i=0
until [ $? -eq 0 ] && [ -e "$tmpdir" ]
do
    tmpdir="$TMP/fli4l-qemu-net.$$.$i"
    i=$(($i + 1))
    mkdir "$tmpdir" 2>/dev/null
done
trap "rm -rf $tmpdir" EXIT

cp -r config $tmpdir/server-config
cp -r config $tmpdir/client-config
cat > $tmpdir/server-config/_fli4l.txt <<END
HOSTNAME='testserver'
SER_CONSOLE='yes'
IP_NET_N='1'
IP_NET_1='10.0.2.10/24'
IP_NET_1_DEV='eth1'
IP_ROUTE_N='1'
IP_ROUTE_1='0.0.0.0/0 10.0.2.2'
PF_INPUT_N='0'
PF_FORWARD_2='172.16.20.0/24 ACCEPT'
PF_POSTROUTING_1='172.16.20.0/24 MASQUERADE'
DOMAIN_NAME='test.bogus'
DNS_FORWARDERS='10.0.2.3'
OPT_DNS='no'
HOSTS_N='0'
OPT_PPPOE_SERVER='yes'
PPPOE_SERVER_ETH='eth0'
PPPOE_SERVER_LOCAL_IP='172.16.20.1'
PPPOE_SERVER_REMOTE_IP='172.16.20.5'
PPPOE_SERVER_DNS1='10.0.2.3'
PPPOE_SERVER_DNS2='10.0.2.3'
END
cat > $tmpdir/client-config/_fli4l.txt <<END
HOSTNAME='testclient'
BOOT_TYPE='cd'
MOUNT_BOOT='ro'
SER_CONSOLE='yes'
IP_NET_N='1'
IP_NET_1='dhcp'
IP_NET_1_DEV='eth0'
IGNORE_IP_NET_WARNING='yes'
DOMAIN_NAME='test.bogus'
PF_INPUT_N='0'
PF_FORWARD_N='0'
PF_POSTROUTING_N='0'
OPT_DNS='no'
HOSTS_N='0'
OPT_DHCP_CLIENT='yes'
DHCP_CLIENT_TYPE='dhclient'
DHCP_CLIENT_INTERFACES='IP_NET_1_DEV'
DHCP_CLIENT_USEPEERDNS='yes'
OPT_HDDRV='yes'
HDDRV_N='1'
HDDRV_1='ide-cd'
OPT_ARP='yes'
OPT_E3='yes'
OPT_FTP='yes'
OPT_IFTOP='yes'
OPT_IMONC='yes'
OPT_NETCAT='yes'
OPT_NETSTAT='yes'
OPT_NTTCP='yes'
OPT_SERIAL='yes'
OPT_STRACE='yes'
OPT_TAIL='yes'
OPT_TCPDUMP='yes'
OPT_TOP='yes'
OPT_TRACEROUTE='yes'
OPT_TRACEROUTE6='yes'
OPT_WGET='yes'
WGET_SSL='yes'
END

for conf in $tmpdir/server-config $tmpdir/client-config $config
do
    configure_mtools $conf
    sh ./mkfli4l.sh $update_version $conf || exit
done

if [ "`extract_value SER_CONSOLE $config`" != 'yes' ]
then
    echo "ERROR: Please add SER_CONSOLE='yes' to your configuration"
    exit
fi

type=`extract_value BOOT_TYPE $config`
case $type in
    cd)
	boot="-cdrom $config/build/fli4l.iso"
    ;;
    fd)
	boot="-fda $tmpdir/router.img"
    ;;
    *)
    echo "Don't know how to boot BOOT_TYPE '$type'!"
    exit 1
    ;;
esac

cat > "$tmpdir/startup.sh" <<END
echo "Please wait..."
screen -t server qemu -fda "$tmpdir/server.img" -m 16 -nographic \
    -net nic,vlan=1,macaddr=52:54:00:12:34:12 -net socket,vlan=1,listen=127.0.0.1:30001 \
    -net nic,vlan=2,macaddr=52:54:00:12:34:21 -net user,vlan=2,hostname=outside
echo "Started pppoe-server, waiting 15 sec until starting the router..."
sleep 15
screen -t router qemu $boot -m 24 -nographic \
    -net nic,vlan=0,macaddr=52:54:00:12:34:02 -net socket,vlan=0,listen=127.0.0.1:30002 \
    -net nic,vlan=1,macaddr=52:54:00:12:34:11 -net socket,vlan=1,connect=127.0.0.1:30001
echo "Started router, waiting 15 sec until starting the client..."
sleep 15
screen -t client qemu -cdrom "$tmpdir/client-config/build/fli4l.iso" -m 16 -nographic \
    -net nic,vlan=0,macaddr=52:54:00:12:34:01 -net socket,vlan=0,connect=127.0.0.1:30002
END

cat > "$tmpdir/screenrc" <<END
startup_message off
defscrollback 10000
caption always "%{= kc}%?%-Lw%?%{= kY}%n*%f %t%?(%u)%?%{= kc}%?%+Lw%?"
hardstatus alwayslastline "fli4l qemu screen. Use F11 and F12 to cycle between machines. Use F10 to kill everything."
# Unbind some keybindings that may disrupt our scripted stuff.
bind .
bind ^\\
bind \\\\
bind h
bind ^h
bind H
bind c
bind ^C
bind D
bind d
bind ^D
bind :
bind K
bind q quit
bind x
bind ^X
bind L
bindkey -k F1 prev
bindkey -k F2 next
bindkey -k k; quit
screen -t startup sh "$tmpdir/startup.sh"
END
screen -m -c "$tmpdir/screenrc"

# vim: set sts=4 sw=4:
