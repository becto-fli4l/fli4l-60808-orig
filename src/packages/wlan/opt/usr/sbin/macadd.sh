#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/usr/sbin/macadd.sh                          __FLI4LVER__
#
# Creation: sklein 18.10.2011
# Last Update:  $Id$
#------------------------------------------------------------------------------
fall=0
case $1 in
	add|del|restart|on|off) 
		fall=$1;;
	*) 
		echo "Please add \"add, del, on, off or restart\"!";
		exit 1;;
esac
if [ -n "$2" ]
then
	if [ "$fall" == "add" -o "$fall" == "del" ]
	then
		if [ -n "`echo $3 | sed -n '/^[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}$/p'`" -a "$fall" == "add" ]
		then
			mac=`echo $3 | tr '[:lower:]' '[:upper:]'`
			echo $mac >> /etc/hostapd."$2".acl;
		elif [ -n "`echo $3 | sed -n '/^[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}:[0-9a-fA-F]\{2\}$/p'`" -a "$fall" == "del" ]
		then
			mac=`echo $3 | tr '[:lower:]' '[:upper:]'`
			grep -v $mac /etc/hostapd."$2".acl > /etc/hostapd."$2".acl.tmp;
			mv /etc/hostapd."$2".acl.tmp /etc/hostapd."$2".acl;
		else
			echo "Please enter a valid mac-adress!";
			exit 1;
		fi
	fi
	
	if [ "$fall" == "add" -o "$fall" == "del" -o "$fall" == "restart" ]
	then
		id=`ps | grep hostapd | grep -v grep | grep "$2" | cut -d' ' -f2`;
		echo $id;
		#kill -HUP $id;
		#/usr/sbin/hostapd -P /var/run/hostapd_"$3".pid /etc/hostapd."$3" & > /dev/null
	fi
	
	if [ "$fall" == "on" ]
	then
		echo "Switching on WLAN "$2"!";
		#wlanconfig.sh "$2";
	fi
	
	
	if [ "$fall" == "off" ]
	then
		echo "Switching off WLAN "$2"!";
		if [ -n "$2" ]
		then
			interface=$2
		else
			echo missing interface
			exit 1
		fi
		bridge=''
		brctl show | sed '1d' |\
		while read line
		do
			set $line
			if [ -n "$4" ] # We found a new bridge
			then
    			bridge=$1
    			if=$4
  			else
    			if=$1
  			fi
  			if [ "$if" == "$interface" -a -n "$bridge" ]
  			then
    			brctl delif $bridge $if
    		fi
		done
		ip link set "$interface" down;
		kill `cat /var/run/hostapd_$interface.pid`;
	fi
else
	echo "Please add wlan-interface!";
#while read line
#do
#	if [ "$line" == "macaddr_acl=1" ]
#	then
#		alc=`echo "whitelist"`;
#	elif [ "$line" == "macaddr_acl=2" ]
#	then
#		alc=`echo "blacklist"`;
#	fi
#done < /etc/hostapd.wlan0
#echo "Liste: "$alc;
fi

