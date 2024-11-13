#!/bin/sh
#----------------------------------------------------------------------------
# /usr/local/bin/qos - configuration of OPT_QOS                  __FLI4LVER__
#
# Creation:     02.05.2001  he
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /etc/boot.d/base-helper
script=qos.sh
fw_tmp=/tmp/qos.$$

run_command ()
{
    cmd=$1
    shift
    if ! $cmd "$@" > $fw_tmp 2>&1
    then
        log_error "Error executing $cmd $@"
        log_error < $fw_tmp
        rm -f $fw_tmp
        return 1
    fi
    rm -f $fw_tmp
    return 0
}

add_prefix ()
{
    eval val="\$$1"
    case x$val in
	x) ;;
	*) eval $1="'$2 $val'" ;;
    esac

}

add_port ()
{
    # add_port port type client_option server_option
    case $1 in
	none)
	    ;;
	*)
	    case $proto in
		*icmp*)
		    log_error "ignoring port option for icmp protocol"
		    return
		    ;;
		*igmp*)
		    log_error "ignoring port option for igmp protocol"
		    return
		    ;;
		*tcp* | *udp*)
		    ;;
		*)
		    proto='tcp udp'
		    ;;
	    esac
	    case "$2" in 
		client)
		    du32_param="$du32_param $3 $1"
		    ;;
		server)
		    du32_param="$du32_param $4 $1"
		    ;;
	    esac
	    ;;
    esac
}

translate_var ()
{
    eval var='$'$1
    case $var in
        *Kibit/s)
            var=`echo $var|sed s/'Kibit\/s'/kbit/`
            ;;
        *Mibit/s)
            var=`echo $var|sed s/'Mibit\/s'/mbit/`
            ;;
    esac
    eval $1='$var'
}

setup_classes ()
{
    do_modprobe sch_htb
    do_modprobe sch_sfq
    case $QOS_PRIO in
        yes) do_modprobe sch_prio ;;
    esac
    do_modprobe cls_fw
    do_modprobe imq numdevs=2

    ## --- global settings --- ##

    burst='' # autocalculated by tc
    mtu='' # using MTU of device by default
    perturb='perturb 10'

    ## --- inbound traffic --- ##
    root_inet=`expr $QOS_CLASS_N + 1`

    case "$QOS_INTERNET_DEFAULT_DOWN" in
	0)
	    QOS_INTERNET_DEFAULT_DOWN=$root_inet
	    ;;
    esac

    ## aktiviere imq0 und imq1
    ip link set imq0 up
    ip link set imq1 up
    ## HTB für Inbound-Device aktivieren:
    $TC qdisc add dev imq0 root handle 10: htb \
        default $QOS_INTERNET_DEFAULT_DOWN r2q 1 || \
	log_error 'ERROR creating main qos configuration'

    ## Klasse für die den beschränkten Internet Traffic einrichten.
    translate_var QOS_INTERNET_BAND_DOWN
    
    $TC class add dev imq0 parent 10:        \
        classid 10:$root_inet htb rate $QOS_INTERNET_BAND_DOWN $burst || \
	log_error 'ERROR creating main qos configuration'

    ## prio oder sfq (normal)?
    case $QOS_PRIO in
        yes)
            $TC qdisc add dev imq0 parent 10:$root_inet handle 11: prio bands $QOS_PRIO_CLASSN
            [ 0$QOS_PRIO_CLASSN -eq 0 ] || for idx in `seq 1 $QOS_PRIO_CLASSN`
            do
                $TC qdisc add dev imq0 parent 11:$idx sfq $mtu $perturb
            done
        ;;
        *)
            $TC qdisc add dev imq0 parent 10:$root_inet sfq $mtu $perturb
        ;;
    esac

    ## --- outbound traffic --- ##
    case "$QOS_INTERNET_DEFAULT_UP" in
	0)
	    QOS_INTERNET_DEFAULT_UP=$root_inet
	    ;;
    esac

    ## HTB für Outbound-Device aktivieren:
    $TC qdisc add dev imq1 root handle 20: htb \
        default $QOS_INTERNET_DEFAULT_UP r2q 1 || \
	log_error 'ERROR creating main qos configuration'

    ## Standardklasse einrichten:
    translate_var QOS_INTERNET_BAND_UP
    
    $TC class add dev imq1 parent 20:     \
        classid 20:$root_inet htb rate $QOS_INTERNET_BAND_UP $burst || \
        log_error 'ERROR creating main qos configuration'

    ## prio oder sfq (normal)?
    case $QOS_PRIO in
        yes)
            $TC qdisc add dev imq1 parent 20:$root_inet handle 21: prio bands $QOS_PRIO_CLASSN
            [ 0$QOS_PRIO_CLASSN -eq 0 ] || for idx in `seq 1 $QOS_PRIO_CLASSN`
            do
                $TC qdisc add dev imq1 parent 21:$idx sfq $mtu $perturb
            done
        ;;
        *)
            $TC qdisc add dev imq1 parent 20:$root_inet sfq $mtu $perturb
        ;;
    esac

    ## --- Klassen einrichten --- ##

    [ 0$QOS_CLASS_N -eq 0 ] || for idx in `seq 1 $QOS_CLASS_N`
    do
	eval parent='$QOS_CLASS_'$idx'_PARENT'
	eval maxband='$QOS_CLASS_'$idx'_MAXBANDWIDTH'
        translate_var maxband
	eval minband='$QOS_CLASS_'$idx'_MINBANDWIDTH'
        translate_var minband
	eval direction='$QOS_CLASS_'$idx'_DIRECTION'
	eval prio='$QOS_CLASS_'$idx'_PRIO'

	add_prefix prio prio
	add_prefix maxband ceil

	case $direction in
	    down)
		dev=imq0
		prefix=10
		;;
	    up)
		dev=imq1
		prefix=20
		;;
	esac
	class="$prefix:$idx"
	case $parent in
	    0)
		cparent=$prefix:$root_inet
		;;
	    *)
		cparent="$prefix:$parent"
		;;
	esac
	$TC class add dev $dev parent $cparent    \
	    classid $class htb rate $minband $maxband $burst $prio || \
	    log_error "Error creating class no $idx"
	$TC qdisc add dev $dev parent $class sfq $mtu $perturb
    done
}

setup_filters ()
{
    ## --- initialize vars --- ##

    idx_filter=1

    ## --- Filterregeln einrichten --- ###
    ## See http://www.faqs.org/docs/iptables/traversingoftables.html 
    ## for a description of how packets traverse the chains. You will see
    ## that:
    ##   - prerouting mangle comes before prerouting dnat -- therefore
    ##     we will see the IP-Address used for masquerading and not 
    ##     the real destination -> use FORWARD to classify incoming traffic
    ##     and to send it to the imq chain
    ##   - there is no postrouting hook after INPUT, so we may classify 
    ##     packets in INPUT, but they are not send to the imq device
    ##   - postrouting mangle comes before postrouting snat, so we still
    ##     have the original ip adresses and can use postrouting mangle
    ##     to classify packets
    ##
    ## http://iptables-tutorial.frozentux.net/scripts/rc.test-iptables.txt 
    ## demonstrates this
    ##
    ## IMQ hooks only into the PRE- and POSTROUTING hooks and send packets
    ## marked by target IMQ to the imq device only at these two points.
    ## Packet delivery to the imq device in PREROUTING happens directly 
    ## after the mangle table has been passed (not in the table itself!).
    ##
    ## Prerouting hook -> mangle (NF_IP_PRI_MANGLE)
    ##			  imq    (NF_IP_PRI_MANGLE+1)
    ## 			  nat	 (NF_IP_PRI_NAT_DST)
    ##
    ## In POSTROUTING packets reach the device after ALL tables have been 
    ## passed. 
    ## 
    ## Postrouting hook ->mangle (NF_IP_PRI_MANGLE)
    ## 			  nat	 (NF_IP_PRI_NAT_SRC)
    ##			  imq    (NF_IP_PRI_LAST)
    ##
    ## Since we classify in FORWARD it would be too early to send
    ## packets to the imq device in the prerouting hook. Therefore we delay 
    ## this to postrouting. We do this by marking packets in FORWARD 
    ## and the imq device picks these packets up at the end of 
    ## postrouting. By doing this it may happen that we don't account for
    ## incoming packets if they are dropped by filter rules.
    ##
    ## See http://wiki.nix.hu/cgi-bin/twiki/view/IMQ/ImqFaq#When_does_IMQ_and_filters_attach 
    ## if you ever think about tc filters instead of iptables mark to 
    ## classify packets.
    ##
    ## An alternative approach would be to use connmark to classify connections
    ## and use these marks in prerouting to classify packets. We have to use 
    ## this to classify and shape incoming traffic to the router. We can mark 
    ## connections in OUTPUT and use these marks to send the packets to the 
    ## imq chain in PREROUTING. 
    ## 
    $IT -N qosout -t mangle
    $IT -N qosin -t mangle

    for idx in `seq $QOS_INTERNET_DEV_N`
    do
        eval i='$QOS_INTERNET_DEV_'$idx
        $IT -A FORWARD     -t mangle -i $i -j qosin
        $IT -A FORWARD     -t mangle -i $i -j IMQ --todev 0
        ## There is no postrouting hook after input so it doesn't 
	## help to classify packets here
	# $IT -A INPUT       -t mangle -i $i -j qosin
        # $IT -A INPUT       -t mangle -i $i -j IMQ --todev 0
        $IT -A POSTROUTING -t mangle -o $i -j qosout
        $IT -A POSTROUTING -t mangle -o $i -j IMQ --todev 1
    done

    [ 0$QOS_FILTER_N -eq 0 ] || for idx in `seq $QOS_FILTER_N -1 1`
    do
	eval class='$QOS_FILTER_'$idx'_CLASS'
	eval intip='$QOS_FILTER_'$idx'_IP_INTERN'
	eval extip='$QOS_FILTER_'$idx'_IP_EXTERN'
	eval port='$QOS_FILTER_'$idx'_PORT'
	eval type='$QOS_FILTER_'$idx'_PORT_TYPE'
	eval option='$QOS_FILTER_'$idx'_OPTION'
        comment="QOS_FILTER_$idx"

	: ${intip:=0.0.0.0/0}
	: ${extip:=0.0.0.0/0}
	: ${port:=none}
	: ${type:=none}

	for fclass in $class
	do

            case $QOS_PRIO in
                yes)
                    class_direction=`echo $fclass|sed s/[0-9]//g`
                    fclass=`echo $fclass|sed s/[a-z]//g`
                    ;;
                *)
                    eval class_direction='$QOS_CLASS_'$fclass'_DIRECTION'
                    ;;
            esac

	    for fintip in $intip
	    do
		for fextip in $extip
		do
		    for fport in $port
		    do
			case "$fport" in
			    *-*)
				fport=`echo $fport | sed 's/-/:/'`
				;;
			esac
			for ftype in $type
			do
			    du32_param=
			    proto=

			    case $option in
				ICMP)
				    proto='icmp'
				    ;;
				IGMP)
				    proto='igmp'
				    ;;
				TCP)
				    proto='tcp'
				    ;;
				UDP)
				    proto='udp'
				    ;;
				ACK)
				    proto='tcp'
				    du32_param="-m length --length :80"
				    ;;
				TCPSMALL)
				    proto='tcp'
				    du32_param="-m length --length :800"
				    ;;
				TOSMD)
				    proto='tcp udp'
				    du32_param="-m tos --tos 16"
				    ;;
				TOSMT)
				    proto='tcp udp'
				    du32_param="-m tos --tos 8"
				    ;;
				TOSMR)
				    proto='tcp udp'
				    du32_param="-m tos --tos 4"
				    ;;
				TOSMC)
				    proto='tcp udp'
				    du32_param="-m tos --tos 2"
				    ;;
				DSCP*)
				    dscp=`echo $option|sed s/DSCP//g`^
				    du32_param="-m dscp --dscp-class $dscp"
				    ;;
			    esac

			    case $class_direction in
				down)
				    chain='qosin'
                                    pprefix='10'
                                    case $QOS_PRIO in
                                        yes)
                                            prefix='11'
                                            ;;
                                        *)
                                            prefix='10'
                                            ;;
                                    esac
				    dev="imq0"

				    du32_param="--dst $fintip --src $fextip $du32_param"
				    add_port $fport "$ftype" --sport --dport 
				    ;;
				up)
				    chain='qosout'
                                    pprefix='20'
                                    case $QOS_PRIO in
                                        yes)
        				           prefix='21'
                                        ;;
                                        *)
                                            prefix='20'
                                        ;;
                                    esac
				    dev="imq1"

				    du32_param="--src $fintip --dst $fextip $du32_param"
				    add_port $fport "$ftype" --dport --sport 
				    ;;
			    esac

			    case x$proto in
				x)
				    $IT -A $chain -t mangle $du32_param -m comment --comment $comment -j MARK --set-mark $idx_filter || \
				        log_error "Error creating filter no. $idx"
				    ;;
				*)
				    for i in $proto
				    do
					$IT -A $chain -t mangle -p $i $du32_param -m comment --comment $comment -j MARK --set-mark $idx_filter || \
					    log_error "Error creating filter no. $idx"
				    done
				    ;;
			    esac

			    $TC filter add dev $dev parent $pprefix:0 \
			        protocol ip handle $idx_filter fw \
				flowid $prefix:$fclass || \
				log_error "Error creating filter no. $idx"

			    idx_filter=`expr $idx_filter + 1`
			done
		    done
		done
	    done
        done
    done
}


QOS_CONFIG_DIR='/var/run'
. "$QOS_CONFIG_DIR"/qos.conf

begin_script QOS "Changing configuration of QOS services ..."

if [ -f /var/run/ip_net.conf ]
then
    . /var/run/ip_net.conf
fi

TC="run_command /sbin/tc"
IT="run_command /sbin/iptables"
SCRIPT='QOS'

if tc -s -d qdisc show 2>/dev/null | grep -q sfq
then
    status='1'
fi

res=0
case "$1" in
    start)
	if [ "$status" != '1' ]
	then

	    begin_script QOS 'Starting Quality of Service ...'

	    > /var/run/rrdtool.qos.up
	    > /var/run/rrdtool.qos.down

	    setup_classes
	    setup_filters
	    end_script
	else
	    log_error 'Can not start Qos - QoS is already running.'
	    res=1
	fi
    ;;
  stop)
      if [ "$status" = '1' ]; then
	  begin_script QOS 'Stopping Quality of Service ...' 
	  $TC qdisc del dev imq0 root
	  $TC qdisc del dev imq1 root

	  for idx in `seq $QOS_INTERNET_DEV_N`; do
	      eval i='$QOS_INTERNET_DEV_'$idx
	      $IT -D FORWARD  -t mangle -i $i -j qosin
	      $IT -D FORWARD  -t mangle -i $i -j IMQ --todev 0
	      $IT -D POSTROUTING -t mangle -o $i -j qosout
	      $IT -D POSTROUTING -t mangle -o $i -j IMQ --todev 1
	  done

	  $IT -F qosout -t mangle
	  $IT -X qosout -t mangle
	  $IT -F qosin -t mangle
	  $IT -X qosin -t mangle

	  ip link set imq1 down
	  ip link set imq0 down

	  end_script
      else
	  log_error 'Can not stop Qos services - QoS services are not not running'
	  res=1
      fi

      ;;
  restart)
      begin_script QOS 'Restarting Quality of Service ...' gn
      if [ "$status" = "1" ]; then
	  $0 stop
      else
	  log_info 'Qos is not running, skipping stop function ...'
      fi
      $0 start
      end_script
      ;;

  status)

      if [ "$status" = "1" ]; then
	  log_info 'QoS is running'
      else
	  log_info 'QoS is not running'
      fi
      ;;

  *)
      echo "Usage: $0 {start|stop|restart|status}"
      ;;
esac

end_script
exit $res
