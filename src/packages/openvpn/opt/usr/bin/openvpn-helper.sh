#!/bin/sh
#$Id$

. /etc/boot.d/base-helper
. /etc/rc.d/fwrules-helper

SCRIPT=`basename $0`

# $config is exported from openvpn and contains the current configuration
ovpn_cfg=`basename $config .conf`

# if there is a ipv6-tunnel-config we also do ipv6-stuff here
if [ $ovpn_ipv6 = yes ]
then
  . /etc/rc.d/fwrules-helper.ipv6
fi

# script_type should contain up/down but if we're running the down
# event with the down-root plugin script_type is undefined. So we
# determinate which script to run via the script name.
#logger -t openvpn-$ovpn_cfg "running script_type = $script_type"

# Extend path to /usr/local/bin
PATH=$PATH:/usr/local/bin

case $0 in
  *-route-up)
    ovpn_action=route-up
  ;;
  *-route-down)
    ovpn_action=route-down
  ;;
  *-up)
    ovpn_action=up
  ;;
  *-down)
    ovpn_action=down
  ;;
  *)
    logger -t openvpn-$ovpn_cfg "ups, something goes wrong in $0."
    exit 1
  ;;
esac

# if a user pre script exist run it
ext=/etc/openvpn/$ovpn_cfg-$ovpn_action-pre
if [ -f "$ext" ]
then
  logger -t openvpn-$ovpn_cfg "running $ext script"
  . $ext
fi
if [ $ovpn_ipv6 = yes ]
then
  ext=/etc/openvpn/$ovpn_cfg-$ovpn_action-pre-ipv6
  if [ -f "$ext" ]
  then
    logger -t openvpn-$ovpn_cfg "running $ext script"
    . $ext
  fi
fi

case $ovpn_action in
  up)
    net_alias_add $ovpn_cfg $dev
    # add INPUT chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "add chain in-ovpn-$ovpn_cfg to INPUT"
    fw_add_chain filter in-ovpn-$ovpn_cfg
    fw_prepend_rule filter in-ovpn "if:$dev:any in-ovpn-$ovpn_cfg"
    # add FORWARD chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "add chain fw-ovpn-$ovpn_cfg to FORWARD"
    fw_add_chain filter fw-ovpn-$ovpn_cfg
    fw_prepend_rule filter fw-ovpn "if:$dev:any fw-ovpn-$ovpn_cfg"
    fw_prepend_rule filter fw-ovpn "if:any:$dev fw-ovpn-$ovpn_cfg"
    # add PREROUTING chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "add chain pi-ovpn-$ovpn_cfg to PREROUTING"
    fw_add_chain nat pi-ovpn-$ovpn_cfg
    fw_prepend_rule nat pi-ovpn "if:$dev:any pi-ovpn-$ovpn_cfg"
    # add POSTROUTING chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "add chain po-ovpn-$ovpn_cfg to POSTROUTING"
    fw_add_chain nat po-ovpn-$ovpn_cfg
    fw_prepend_rule nat po-ovpn "if:any:any po-ovpn-$ovpn_cfg"

    sed -e "s#VPNDEV#$dev#" /var/run/openvpn/$ovpn_cfg/fwrules | while read ovpn_table ovpn_chain ovpn_fwrule
    do
      logger -t openvpn-$ovpn_cfg "adding rule \"$ovpn_fwrule\" to $ovpn_table $ovpn_chain"
      fw_append_rule $ovpn_table $ovpn_chain "$ovpn_fwrule"
    done

    # code if ipv6 tunnel
    if [ $ovpn_ipv6 = yes ]
    then
        # add INPUT chain hook to OpenVPN Chain
        logger -t openvpn-$ovpn_cfg "add chain in-ovpn-$ovpn_cfg to INPUT6"
        fw_add_chain6 filter in-ovpn-$ovpn_cfg
        fw_prepend_rule6 filter in-ovpn "if:$dev:any in-ovpn-$ovpn_cfg"
        # add FORWARD chain hook to OpenVPN Chain
        logger -t openvpn-$ovpn_cfg "add chain fw-ovpn-$ovpn_cfg to FORWARD6"
        fw_add_chain6 filter fw-ovpn-$ovpn_cfg
        fw_prepend_rule6 filter fw-ovpn "if:$dev:any fw-ovpn-$ovpn_cfg"
        fw_prepend_rule6 filter fw-ovpn "if:any:$dev fw-ovpn-$ovpn_cfg"
        sed -e "s#VPNDEV#$dev#" /var/run/openvpn/$ovpn_cfg/fwrules.ipv6 | while read ovpn_table ovpn_chain ovpn_fwrule
        do
          logger -t openvpn-$ovpn_cfg "adding rule \"$ovpn_fwrule\" to $ovpn_table ${ovpn_chain}6"
          fw_append_rule6 $ovpn_table $ovpn_chain "$ovpn_fwrule"
        done
    fi
  ;;
  down)
    net_alias_del $ovpn_cfg

    sed -e "s#VPNDEV#$dev#" /var/run/openvpn/$ovpn_cfg/fwrules | while read ovpn_table ovpn_chain ovpn_fwrule
    do
      logger -t openvpn-$ovpn_cfg "removing rule \"$ovpn_fwrule\" from $ovpn_table $ovpn_chain"
      fw_delete_rule $ovpn_table $ovpn_chain "$ovpn_fwrule"
    done
    # remove INPUT chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "remove chain in-ovpn-$ovpn_cfg from INPUT"
    fw_delete_rule filter in-ovpn "if:$dev:any in-ovpn-$ovpn_cfg"
    fw_delete_chain filter in-ovpn-$ovpn_cfg
    # remove FORWARD chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "remove chain fw-ovpn-$ovpn_cfg from FORWARD"
    fw_delete_rule filter fw-ovpn "if:$dev:any fw-ovpn-$ovpn_cfg"
    fw_delete_rule filter fw-ovpn "if:any:$dev fw-ovpn-$ovpn_cfg"
    fw_delete_chain filter fw-ovpn-$ovpn_cfg
    # remove PREROUTING chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "remove chain pi-ovpn-$ovpn_cfg from PREROUTING"
    fw_delete_rule nat pi-ovpn "if:$dev:any pi-ovpn-$ovpn_cfg"
    fw_delete_chain nat pi-ovpn-$ovpn_cfg
    # remove POSTROUTING chain hook to OpenVPN Chain
    logger -t openvpn-$ovpn_cfg "remove chain po-ovpn-$ovpn_cfg from POSTROUTING"
    fw_delete_rule nat po-ovpn "if:any:any po-ovpn-$ovpn_cfg"
    fw_delete_chain nat po-ovpn-$ovpn_cfg
    if [ $ovpn_ipv6 = yes ]
    then
      sed -e "s#VPNDEV#$dev#" /var/run/openvpn/$ovpn_cfg/fwrules.ipv6 | while read ovpn_table ovpn_chain ovpn_fwrule
      do
        logger -t openvpn-$ovpn_cfg "removing rule \"$ovpn_fwrule\" from $ovpn_table ${ovpn_chain}6"
        fw_delete_rule6 $ovpn_table $ovpn_chain "$ovpn_fwrule"
      done
      # remove INPUT chain hook to OpenVPN Chain
      logger -t openvpn-$ovpn_cfg "remove chain in-ovpn-$ovpn_cfg from INPUT6"
      fw_delete_rule6 filter in-ovpn "if:$dev:any in-ovpn-$ovpn_cfg"
      fw_delete_chain6 filter in-ovpn-$ovpn_cfg
      # remove FORWARD chain hook to OpenVPN Chain
      logger -t openvpn-$ovpn_cfg "remove chain fw-ovpn-$ovpn_cfg from FORWARD6"
      fw_delete_rule6 filter fw-ovpn "if:$dev:any fw-ovpn-$ovpn_cfg"
      fw_delete_rule6 filter fw-ovpn "if:any:$dev fw-ovpn-$ovpn_cfg"
      fw_delete_chain6 filter fw-ovpn-$ovpn_cfg
    fi
  ;;
esac

ext=/etc/openvpn/$ovpn_cfg-$ovpn_action-post
if [ -f "$ext" ]
then
  logger -t openvpn-$ovpn_cfg "running $ext script"
  . $ext
fi
if [ $ovpn_ipv6 = yes ]
then
  ext=/etc/openvpn/$ovpn_cfg-$ovpn_action-post-ipv6
  if [ -f "$ext" ]
  then
    logger -t openvpn-$ovpn_cfg "running $ext script"
    . $ext
  fi
fi
