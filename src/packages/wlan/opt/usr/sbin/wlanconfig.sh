#!/bin/sh
#------------------------------------------------------------------------------
# /usr/sbin/wlanconfig.sh - configure wlan options to device   __FLI4LVER__   
#
# Creation:	    05.08.2002 rresch
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /etc/boot.d/base-helper
case "$1" in 
    -x)
        set -x
        wlanconfig_debug=-x
        shift
        ;;
esac


device=$1
if [ -z "$device" -o -z "`ip link show $device 2>/dev/null`" ]; then
    log_error "Usage: $0 <WLAN-Ethernet-Device> [down]"
    exit 17
fi
if [ -f /var/run/wlan_led.conf ]; then
    . /var/run/wlan_led.conf
fi


if [ -n "$2" ]
then
  case $2 in
    down|stop)
      if type -p brctl >/dev/null
      then
        bridge=''
        brctl show | sed '1d' |\
          while read line
          do
            set $line
            if [ -n "$4" ] # We found a new bridge
            then
              bridge=$1
              interface=$4
            else
              interface=$1
            fi
            if [ "$interface" == "$device" -a -n "$bridge" ]
            then
              brctl delif $bridge $device
            fi
          done
      fi
      if [ -f /var/run/hostapd_$device.pid ]
      then
        kill `cat /var/run/hostapd_$device.pid`;
        rm /var/run/hostapd_$device.pid
      fi
      if [ -f /var/run/wpa_supplicant_$device.pid ]
      then
        kill `cat /var/run/wpa_supplicant_$device.pid`
        rm /var/run/wpa_supplicant_$device.pid
      fi
      pid=`ps | grep _$device.pid | grep -v grep | sed 's/^ *\([0-9]\+\) *.*/\1/'`
      if [ -n "$pid" ]
      then
        for pi in $pid
        do
          kill $pi
        done
      fi
      if type -p iw >/dev/null && [ -n "`ip link show mon.$device 2>/dev/null`" ]
      then
        iw dev mon.$device del
      fi
      ip link set dev $device down
      if [ -f /usr/bin/wlan_setleds ]
      then 
        /usr/bin/wlan_setleds
      fi
      exit 0
      ;;
    restart)
      $0 $1 down
      $0 $1
      ;;
    reload)
      WLAN_RELOAD='yes'
      ;;
  esac
fi

if [ ! -f /var/run/wlan.conf ]; then
    log_error "No wlan config found (/var/run/wlan.conf is missing)"
    exit 18
fi

if [ -n "`iwconfig \"$device\" 2>/dev/null`" ]; then
    drivername='other' # Has to be set to drivername if detected 
    candomaster='no'   # Set it to 'yes' when card is capaple of doing AP-Mode
    candowpa='no'      # Set it to 'yes' if card can do WPA 
    cando_n='no'       # Set it to 'yes' if card can do 802.11n 
    ismac80211='no'    # Set it to 'yes' if driver is mac80211 
    # Detect WLAN-Card
    # First try new method to detect
    i=`sed -n "s/^$device=\(.*\)$/\1/p" /var/run/netdrivers.conf`
    if [ -n "$i" ]; then
	case $i in
	    hostap_*)
	        i=hostap
		;;
	    orinoco_*)
	        i=orinoco
		;;
	esac   
	drivername=$i
	if [ -n "`ls /usr/local/wlan/detect_*_$i 2>/dev/null`" ]
	then
          for i in /usr/local/wlan/detect_*_$i; do
	    . $i
	  done
	else
	  if [ -f /usr/local/wlan/detect_900_others ]
	  then
	    . /usr/local/wlan/detect_900_others
	  fi
	fi
    else
	for i in /usr/local/wlan/detect_*; do
	    case "$drivername" in
                other)
          	    if [ -f "$i" ]; then
			. $i
		    fi
		    ;;
	    esac
	done
    fi
    . /var/run/wlan.conf
else
    log_error "'$device' does not support the Wireless Extensions"
    exit 19
fi

case "$OPT_WLAN" in
  yes)
    # change the mac address
    [ "$WLAN_MAC_OVERRIDE" ] && ip link set dev $device addr $WLAN_MAC_OVERRIDE

    # define bridge devname
    eval bridge_devname='$BRIDGE_DEVNAME_'$device 
    WLAN_NICK=`hostname`
    # remove channel nr.
    WLAN_N=`echo $WLAN_CHANNEL | sed -n 's/[0-9]//gp' | sed -n 's/.*\([nN]\)$/\1/p'`
    WLAN_STANDARD=`echo $WLAN_CHANNEL | sed -n 's/[0-9nN]//gp'`
    # fix wlan encription mode
    if [ -z "$WLAN_ENC_MODE" ]; then
        case "$WLAN_ENC_N" in
          1|2|3|4)
            WLAN_ENC_MODE='restricted'
            ;;
          *)
            WLAN_ENC_MODE='off'
        esac
    fi

    # If Card cannot do master then default to ad-hoc
    if [ "$candomaster" != yes ]; then
        case "$WLAN_MODE" in
	    master)
	        log_error "Device $device does not support 'master'-mode, using ad-hoc instead"
                WLAN_MODE="ad-hoc"
		;;
        esac
    fi
  
    # Running Pre-Base-Config Scripts
    for i in /usr/local/wlan/pre_*_${drivername} /usr/local/wlan/pre_*_${drivername}_*; do
	if [ -f "$i" ]; then
	    . $i
	fi
    done
    for i in /usr/local/wlan/pre_*_generic /usr/local/wlan/pre_*_generic_*; do
	if [ -f "$i" ]; then
	    . $i
	fi
    done
   
    # Set WLAN-Mode - only if non mac80211 NIC
    if [ "$ismac80211" != "yes" ]
    then
        [ "$WLAN_MODE" ] && iwconfig $device mode $WLAN_MODE
    fi
    
    # Set WLAN-Nick-Name - only if non mac80211 NIC
    if [ "$ismac80211" != "yes" ]
    then
        [ "$WLAN_NICK" ] && iwconfig $device nick $WLAN_NICK
    fi
    
    # Set WLAN-Transfer-Rate
    if [ "$ismac80211" != "yes" ]
    then
      case "x$WLAN_RATE" in
	x) ;;
	xauto) iwconfig $device rate auto ;;
	*)     iwconfig $device rate ${WLAN_RATE}M ;;
      esac
    fi
    # Set WLAN-ESSID
    [ "$WLAN_ESSID" -a "$ismac80211" != "yes" ] && iwconfig $device essid "$WLAN_ESSID"

    # Set WLAN-WEP-Encryption
    case "$WLAN_ENC_N" in
	0) ;;
	1|2|3|4)
	    case "$WLAN_ENC_MODE" in
		on) WLAN_ENC_MODE='restricted' ;; 
	    esac
	    if [ $WLAN_ENC_ACTIVE -gt $WLAN_ENC_N ]; then
		WLAN_ENC_ACTIVE=1
	    fi
	    if [ $WLAN_ENC_N -lt 4 ]; then
		idx=`expr $WLAN_ENC_N + 1`
		while [ $idx -lt 5 ]; do
		    eval WLAN_ENC_$idx=\$WLAN_ENC_${WLAN_ENC_ACTIVE}
		    idx=`expr $idx + 1`   
		done
	    fi
	    iwconfig $device enc $WLAN_ENC_1
	    iwconfig $device enc [1] $WLAN_ENC_1
	    iwconfig $device enc [2] $WLAN_ENC_2
	    iwconfig $device enc [3] $WLAN_ENC_3
	    iwconfig $device enc [4] $WLAN_ENC_4
	    iwconfig $device enc [$WLAN_ENC_ACTIVE]
	    iwconfig $device enc $WLAN_ENC_MODE
	    ;;
    esac

    # Set WLAN-Channel
    if [ "$WLAN_CHANNEL" ]; then
	WLAN_CHANNEL=`echo $WLAN_CHANNEL | sed 's/[a-zA-Z]*//g'`
	case "$WLAN_CHANNEL" in
	    0) ;;
	    *) case $ismac80211 in
	         no) iwconfig $device channel $WLAN_CHANNEL ;;
		 yes)
		   HTCAP=''
		   if [ "$cando_n" = "yes" -a -n "$WLAN_N" ]
		   then
		     if [ "$WLAN_N" = "N" ]
		     then
		       HTCAP='HT20'
		       if [ $WLAN_CHANNEL -ge 1 -a $WLAN_CHANNEL -le 9 ]
		       then
		         HTCAP='HT40+'
		       fi
		       if [ $WLAN_CHANNEL -ge 5 -a $WLAN_CHANNEL -le 13 ]
		       then
		         HTCAP='HT40-'
		       fi
		       if [ $WLAN_CHANNEL -ge 40 -a $WLAN_CHANNEL -le 52 ]
		       then
		         HTCAP='HT40+'
		       fi
		       if [ $WLAN_CHANNEL -ge 48 -a $WLAN_CHANNEL -le 60 ]
		       then
		         HTCAP='HT40-'
		       fi
		     else
		       HTCAP='HT20'
		     fi
		   fi
		   iw dev $device set channel $WLAN_CHANNEL $HTCAP >/dev/null 2>&1
		   ;;
	       esac
	       ;;
	esac
    fi

    # Set Diversity
    if [ "$WLAN_DIVERSITY" = "yes" -a -f /var/run/wlanphy.conf ]; then
	case $ismac80211 in
	    yes)
		: ${WLAN_DIVERSITY_RX:=1}
		: ${WLAN_DIVERSITY_TX:=1}
		phy=`sed -n "s/=$MAC_ADD//p" /var/run/wlanphy.conf`
		if [ -n "$phy" ]
		then
		    iw phy $phy set antenna $WLAN_DIVERSITY_TX $WLAN_DIVERSITY_RX
		fi
		;;
	esac
    fi

    # Set RTS-Feature
    [ "$WLAN_RTS" = yes ] && iwconfig $device rts 500
  
    # Running Post-Base-Config-Scripts
    for i in /usr/local/wlan/post_*_${drivername} /usr/local/wlan/post_*_${drivername}_*; do
	if [ -f "$i" ]; then
	    . $i
	fi
    done
    for i in /usr/local/wlan/post_*_generic /usr/local/wlan/post_*_generic_*; do
	if [ -f "$i" ]; then
	    . $i
	fi
    done

    ip link set $device up

    if [ "$ismac80211" = "yes" ]
    then
      case "x$WLAN_N" in
        x)
          case "x$WLAN_RATE" in
	        x) ;;
            xauto) iwconfig $device rate auto ;;
	        *)     iwconfig $device rate ${WLAN_RATE}M ;;
          esac
        ;;
      esac
    fi

    if [ -f /usr/bin/wlan_setleds ]
    then 
        /usr/bin/wlan_setleds
    fi

    # second try to change mac
    [ "$WLAN_MAC_OVERRIDE" ] && ip link set dev $device addr $WLAN_MAC_OVERRIDE

    ;;
esac
exit 0
