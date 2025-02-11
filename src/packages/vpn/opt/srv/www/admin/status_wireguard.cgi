#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/admin/status_wireguard.cgi
# Creation:     10.01.2020 Christoph Fritsch <fli4l@dechristo.net>
# Last Update:  12.09.2020 Christoph Fritsch
#------------------------------------------------------------------------------

. /srv/www/include/cgi-helper
. /etc/boot.d/netalias.inc

PEER_FILE='/etc/wireguard/wireguard.peers'
SERVER_FILE='/etc/wireguard/wireguard.names'
QR_PATH='img/wireguard'

#########################################################################

# download WireGuard Peer config file
download_peer_config ()
{
    local interface=$1
    local peerNo=$2
    local peerName=$(get_peer_name $interface $peerNo)
    local configFileName=${peerName}".conf"
    local configFile="/srv/www/admin/img/wireguard/"${interface}/${configFileName}

    if [ -f ${configFile} ]
    then
        echo "Content-Type: text/plain"
        echo "Content-Disposition: attachment; filename="${configFileName}
        echo
        cat ${configFile}
    else
        show_html_header "WireGuard"
        show_error "${_WG_invalidFileErrorTitle}" "${_WG_invalidFileErrorMessage}"
        show_html_footer
    fi
}

# download WireGuard server config (e.g. to get server private key if autogenerated)
download_server_config ()
{
    local interface=$1

    local serverName=`grep "^$interface" $SERVER_FILE | cut -d":" -f3`
    local serverPort=`grep "^$interface" $SERVER_FILE | cut -d":" -f6`
    local serverPrivateKey=`grep "^$interface" $SERVER_FILE | cut -d":" -f4`
    local config_idx=`grep "^$interface" $SERVER_FILE | cut -d":" -f2`

    eval local wg_srv_ip4_var='WIREGUARD_'$config_idx'_LOCAL_IP4'
    eval local wg_srv_ip6_var='WIREGUARD_'$config_idx'_LOCAL_IP6'

    local wg_client_ip4=$(grep $wg_srv_ip4_var /etc/rc.cfg | cut -d"=" -f2 | cut -d"'" -f2)
    if [ x$wg_client_ip4 != "x" ]; then
        ipv4_config_string='LOCAL_IP4='${wg_client_ip4}
    else
        ipv4_config_string='# LOCAL_IP4='
    fi

    local wg_client_ip6=$(grep $wg_srv_ip6_var /etc/rc.cfg | cut -d"=" -f2 | cut -d"'" -f2)
    if [ x$wg_client_ip6 != "x" ]; then
        ipv6_config_string='LOCAL_IP6='${wg_client_ip6}
    else
        ipv6_config_string='# LOCAL_IP6='
    fi

    if [ x$serverName != "x" ]
    then
        echo "Content-Type: text/plain"
        echo "Content-Disposition: attachment; filename=vpn.txt"
        echo
        cat <<EOF
WIREGUARD[] {
    NAME='${serverName}'
    ${ipv4_config_string}
    ${ipv6_config_string}
    PRIVATE_KEY='${serverPrivateKey}'
    LISTEN_PORT='${serverPort}'
    # KEEP_ALIVE='25'
    # DEFAULT_ALLOWED_IPS[]='tbd'
    # PUSH_DNS[]='tbd'

    PEER[] {
        # tbd
    }
}

EOF
    else
        show_html_header "WireGuard"
        show_error "${_WG_invalidFileErrorTitle}" "${_WG_invalidFileErrorMessage}"
        show_html_footer
    fi
}

# resolve interface and peer number into peer name
get_peer_name()
{
    local interface=$1
    local peerNo=$2
    local peerName=`grep "^$interface:[0-9]*:$peerNo" $PEER_FILE | cut -d":" -f6`
    echo $peerName
}

# resolve interface and peer public key into peer name
get_peer_name_byKey()
{
    local interface=$1
    local peerKey=$2
    # match wireguard.peers even if no private key given for some client
    local peerName=`grep -E "^($interface):([0-9]*):([0-9]*):([a-zA-z0-9\/+]{43}[=]{1}|[/s]{0}):" $PEER_FILE | grep $peerKey | cut -d":" -f6`
    echo $peerName
}

# get IPv4 address of the given local WireGuard interface
get_interface_ip4()
{
    local interface=$1
    local ipv4=$(ip a s $interface | grep -e "^\s*inet\s" | awk '/inet/ {print $2}')
    echo $ipv4
}

# get IPv6 address of the given local WireGuard interface
get_interface_ip6()
{
    local interface=$1
    local ipv6=$(ip a s $interface | grep -e "^\s*inet6\s" | awk '/inet/ {print $2}')
    echo $ipv6
}

# get IP addresss string in form "IPv4 | IPv6", e.g. 10.0.0.1/24 | fd00:aaaa:bbbb:cccc:dddd::1/64
# if no IPv4 configured: "- | fd00:aaaa:bbbb:cccc:dddd::1/64"
# if no IPv6 configured: "10.0.0.1/24 | -"
get_interface_ipString()
{
    local interface=$1
    local ipv4=$(get_interface_ip4 $interface)
    local ipv6=$(get_interface_ip6 $interface)

    if [ -z ${ipv4} ]; then
        ipv4='-'
    fi

    if [ -z ${ipv6} ]; then
        ipv6='-'
    fi

    local ipString=${ipv4}' | '${ipv6}

    echo $ipString
}


# convert bytes into readable units (Bytes, KB, MB, GB, TB)
convertBytes ()
{
   local bytes=$1
   local restBytes=$1
   local i=0

    # byte string is longer than 3 chars (avoid out of range MAXINT)
    # limit to TB (i=4) - bigger byte numbers converted to TB
    while [ `expr length ${restBytes}` -gt 3 -a $i -lt 4 ]
    do
        #restBytes=`echo "${restBytes}/1024" | bc`
        restBytes=$((${restBytes}/1024))
        i=$((i+1))
    done

    case ${i} in
      0)   unit="Bytes"  ;;
      1)   unit="KB"  ;;
      2)   unit="MB"    ;;
      3)   unit="GB" ;;
      4)   unit="TB"  ;;
      *)   unit="N/A"  ;; #some error occured - should never happen
    esac

    local divider=$((1024**i))

    local convertedBytes=$((bytes/divider))
    local decimalPlaces=$((bytes%divider*1000/divider))

    echo ${convertedBytes}","${decimalPlaces} ${unit}
}

# print data transfer rx/tx bar
printDataTransferBar ()
{
    local rx=$1
    local tx=$2

    local peerTransferTotal=$((rx+tx))
    if [ x${peerTransferTotal} != "x0" ];
    then
        local peerTransferRXpercentage=$((rx*100/peerTransferTotal))
        local peerTransferTXpercentage=$((100-peerTransferRXpercentage))
        echo "<div class=\"w3-red\" style=\"min-width:35ch\">"
        echo "<span class=\"w3-green\" style=\"display:inline-block;width:"${peerTransferRXpercentage}"%\">"${peerTransferRXpercentage}"%</span>"
        echo "<span style=\"display:inline-block;text-align:center\">"${peerTransferTXpercentage}"%</span>"
        echo "</div>"
    else
        echo "<div class=\"w3-light-grey\" style=\"min-width:35ch\">"${_WG_DataTransferNoDataText}"</div>"
    fi
}

# print connection status indicator
# as it is UDP there is no actual connection - we thus assume that if 
# last handshake was > 3min (180sec) ago, the connection is still active
printConnectionStatusBar ()
{
    local timestampNow=$1
    local lastHandshake=$2
    local secSinceHandshake=$((timestampNow-lastHandshake))
    
    local connectionStatus="<span class=\"reddot\"></span>offline"
    # assuming that handshake is renewed every 3min latest
    if [ 180 -gt ${secSinceHandshake} ];
    then
        echo "<div class=\"w3-container w3-green\" style=\"text-align: center; width:100%\">online</div>"
    else
        echo "<div class=\"w3-container w3-red\" style=\"text-align: center; width:100%\">offline</div>"
    fi
}

# prepare overview table across all WireGuard configurations
show_overview()
{

    for interface in `wg show interfaces`; do
        local switch="odd"
        local numPeers=`wg show ${interface} peers | wc -l`
        local serverName=`grep "^$interface" $SERVER_FILE | cut -d":" -f3`
        local serverPort=`grep "^$interface" $SERVER_FILE | cut -d":" -f6`
        local timestampNow=`date +%s`
        local linkToInterfaceDetails=${myname}"?section="${interface}
        local server_cfg_link=${myname}"?action=srvcfg&section="${interface}
        local ipString=$(get_interface_ipString $interface)

        echo "<a href=\""${linkToInterfaceDetails}"\"><h2>"${serverName}"@"${interface}" ("${ipString}") on UDP port "${serverPort}" with "${numPeers}" peers configured</a>  |  <a href=\""${server_cfg_link}"\">config download</a></h2>"
        echo "<table class=\"normtable connection\"><tr><th>"${_WG_PeerStatusTextTH}"</th><th>"${_WG_PeerNameTextTH}"</th><th colspan=\"2\">"${_WG_DataTransferTextTH}"</th><th>"${_WG_PeerEndpointTextTH}"</th></tr>"

        local peerNo=1
        for peer in `wg show ${interface} peers`; do
            wg show ${interface} dump | grep ${peer} | while read pubKey presharedKey endpoint allowedIPs lHandshake rx tx keepalive
            do
                local linkToPeerDetails=${myname}"?section="${interface}"&peer="${peerNo}

                echo "<tr class=\""${switch}"\"><td>"$(printConnectionStatusBar ${timestampNow} ${lHandshake})"</td><td><a href=\""${linkToPeerDetails}"\">"$(get_peer_name_byKey ${interface} ${peer})"</a></td><td>"$(convertBytes ${rx})"/"$(convertBytes ${tx})"</td><td>"$(printDataTransferBar ${rx} ${tx})"</td><td>"${endpoint}"</td></tr>"
            done
            peerNo=$((peerNo+1))
            # change row-color
            if [ ${switch} = "odd" ]
            then
                local switch="even"
            else
                local switch="odd"
            fi
        done
        echo '</table>'
    done
}


# show peer info overview for individual WireGuard Server
show_server_peers()
{
    local interface=$1
    local serverName=`grep "^$interface" $SERVER_FILE | cut -d":" -f3`
    local serverPort=`grep "^$interface" $SERVER_FILE | cut -d":" -f6`
    local numPeers=`wg show $interface peers | wc -l`
    local peers=`wg show $interface peers`
    local server_cfg_link=${myname}"?action=srvcfg&section="${interface}
    local ipString=$(get_interface_ipString $interface)

    echo "<h2>"${serverName}"@"${interface}" ("${ipString}") on UDP port "${serverPort}" with "${numPeers}" peers configured - <a href=\""${server_cfg_link}"\">config download</a></h2>"

    echo "<table class=\"normtable connection\"><tr><th>"${_WG_PeerStatusTextTH}"</th><th>"${_WG_PeerNameTextTH}"</th><th colspan=\"2\">"${_WG_DataTransferTextTH}"</th><th>"${_WG_PeerEndpointTextTH}"</th></tr>"

    local switch="odd"
    local peerNo=1
    
    for peerKey in ${peers}; do
        local peerEndpoint=`wg show ${interface} endpoints | grep ${peerKey} | cut -d$'\t' -f2`
        #get_dns_name ${peerEndpoint}
        #local peerEndpointDNS=$res
        
        local linkToPeerDetails=${myname}"?section="${interface}"&peer="${peerNo}
        local peerLatestHandshakeTimestamp=`wg show ${interface} latest-handshakes | grep ${peerKey} | cut -d$'\t' -f2`
        local timestampNow=`date +%s`
        
        wg show ${interface} transfer | grep ${peerKey} | while read pubKey rx tx
        do
            echo "<tr class=\""${switch}"\"><td>"$(printConnectionStatusBar ${timestampNow} ${peerLatestHandshakeTimestamp})"</td><td><a href=\""${linkToPeerDetails}"\">"$(get_peer_name_byKey ${interface} ${peerKey})"</a></td><td>"$(convertBytes ${rx})"/"$(convertBytes ${tx})"</td><td>"$(printDataTransferBar ${rx} ${tx})"</td><td>"${peerEndpoint}"</td></tr>"
        done
        peerNo=$((peerNo+1))
        # change row-color
        if [ ${switch} = "odd" ]
        then
            switch="even"
        else
            switch="odd"
        fi
    done 
    echo '</table>'
}



# show all relevant peer infos, QRCode and link to config file
show_peer_info()
{
    local interface=$1
    local peerNo=$2

    local peerName=$(get_peer_name $interface $peerNo)
    local peerPrivateKey=`grep "^$interface:[0-9]*:$peerNo" $PEER_FILE | cut -d":" -f4`
    local peerPublicKey=`grep "^$interface:[0-9]*:$peerNo" $PEER_FILE | cut -d":" -f5`

    local peerInfo=`wg show $interface dump | grep $peerPublicKey`

    # dump output is separated by tab (-d$'\t') but gets lost in variable $peerInfo
    local peerPresharedKey=`echo -e $peerInfo | cut -d' ' -f2`
    local peerEndpoint=`echo $peerInfo | cut -d' ' -f3`
    local peerAllowedIPs=`echo $peerInfo | cut -d' ' -f4`
    local peerLatestHandshakeTimestamp=`echo $peerInfo | cut -d' ' -f5`
    local timestampNow=`date +%s`
    local peerConfigFileName=${peerName}".conf"
    local peerConfigFile="/srv/www/admin/img/wireguard/"${interface}/${peerConfigFileName}
    local peerIPString=$(grep ^Address $peerConfigFile | sed 's/Address = //g' | sed 's/, / | /g')

    # show "(never)" if we have not seen a handshake yet
    if [ $peerLatestHandshakeTimestamp -eq 0 ]; 
    then
        local peerLatestHandshake="(never)"
    # otherwise show x sec ago + readable time of last handshake
    else
        local secSinceHandshake=$((timestampNow-peerLatestHandshakeTimestamp))
        local readableHandshakeTime=`date -d @${peerLatestHandshakeTimestamp} +'%d.%m.%Y %H:%M:%S'` 
        local peerLatestHandshake=${secSinceHandshake}" sec ago ("${readableHandshakeTime}")"
    fi

    local peerTransferRX=`echo $peerInfo | cut -d' ' -f6`
    local peerTransferTX=`echo $peerInfo | cut -d' ' -f7`
    local peerTransferTotal=$((peerTransferTX+peerTransferRX))

    local peerPersistentKeepalive=`echo $peerInfo | cut -d' ' -f8`
    local qrCodeFile=${QR_PATH}"/"${interface}"/"${peerName}".png"

    local peer_link=${myname}"?action=download&section="${interface}"&peer="${peerNo}
    local download="<a href=\""${peer_link}"\">"${_WG_DownloadCfgTextTH}"</a>"

    echo "<table class=\"normtable connection\"><tr><th>Key</th><th>Value</th><th>QRCode</th></tr>"
    echo "<tr class=\"odd\"><td>"${_WG_PeerNameTextTH}"</td><td>"${peerName}"</td><td rowspan=\"11\"><img src=\""${qrCodeFile}"\" alt=\"QRCode\" width=\"231\" height=\"231\"></td></tr>"
    echo "<tr class=\"even\"><td>"${_WG_PeerEndpointTextTH}"</td><td>"${peerEndpoint}"</td></tr>"
    echo "<tr class=\"odd\"><td>Peer IPv4 | IPv6</td><td>"${peerIPString}"</td></tr>"
    
    echo "<tr class=\"even\"><td>"${_WG_PeerLatestHandshakeTextTH}"</td><td>"
    # assuming that handshake is renewed every 3min latest
    if [ 180 -gt ${secSinceHandshake} ];
    then
        echo "<div class=\"w3-container w3-green\" style=\"text-align: center; width:100%\">"${peerLatestHandshake}"</div>"
    else
        echo "<div class=\"w3-container w3-red\" style=\"text-align: center; width:100%\">"${peerLatestHandshake}"</div>"
    fi
    echo "</td></tr>"

    echo "<tr class=\"odd\"><td>"${_WG_PeerPrivateKeyTextTH}"</td><td>"${peerPrivateKey}"</td></tr>"
    echo "<tr class=\"even\"><td>"${_WG_PeerPublicKeyTextTH}"</td><td>"${peerPublicKey}"</td></tr>"
    echo "<tr class=\"odd\"><td>"${_WG_PeerPresharedKeyTextTH}"</td><td>"${peerPresharedKey}"</td></tr>"
    echo "<tr class=\"even\"><td>"${_WG_AllowedIPsTextTH}"</td><td>"${peerAllowedIPs}"</td></tr>"
    echo "<tr class=\"odd\"><td>"${_WG_KeepAliveIntervalTextTH}"</td><td>"${peerPersistentKeepalive}"</td></tr>"
    echo "<tr class=\"even\"><td rowspan=\"2\">"${_WG_DataTransferTextTH}"</td><td>"$(convertBytes ${peerTransferRX})" / "$(convertBytes ${peerTransferTX})"</td></tr>"
    echo "<tr class=\"even\"><td>"$(printDataTransferBar ${peerTransferRX} ${peerTransferTX})"</td></tr>"

    echo "<tr class=\"odd\"><td colspan=\"3\" style=\"text-align: center; vertical-align: middle;\">"${download}"</td></tr>"
    echo '</table>'
}

#########################################################################

# Define important variables

# sanitize $FORM_action so it can be safely used
FORM_action=$(echo "$FORM_action" | sed 's/[^a-zA-Z0-9_]//g')

# set default section if not given otherwise
if [ "$FORM_section" = "" ]; then
    FORM_section="overview"
fi
# set default peer if not given otherwise
if [ "$FORM_peer" = "" ]; then
    FORM_peer="all"
fi

#########################################################################


wgInterfaces=`wg |grep interface | cut -d":" -f2 | sed 's/^[ ]*//' | sort`

interface_tabs=""
for availableInterface in overview $wgInterfaces; do
    if [ $FORM_section == $availableInterface ]; then
        section_link="no"
    else
        section_link="${myname}?section=$availableInterface"
    fi
    
    alias_name=""
    net_alias_lookup_name $availableInterface alias_name
    if [ x"$alias_name" != "x" ]
    then
        interface=`echo "${alias_name}&nbsp;[$availableInterface]"`
    else
        interface=$availableInterface
    fi
    interface_tabs="${interface_tabs} ${interface} ${section_link}"
done



# if one of the wg-interfaces is chosen get its peers
peer_tabs=''
if [ `echo "${wgInterfaces}" | grep "$FORM_section"` ]
then
    peers=`grep ^$FORM_section: $PEER_FILE | cut -d":" -f3`

    for peer in all $peers; do
        if [ $FORM_peer == $peer ]; then
            peer_link="no"
        else
            peer_link="${myname}?section=$FORM_section&peer=$peer"
        fi
        #match interface:configNo:peerNo)
        peerName=$(get_peer_name $FORM_section $peer)
        if [ x"$peerName" != "x" ]
        then
            peer=`echo "${peerName}&nbsp;[peer$peer]"`
        elif [ $peer != all ]
        then
            peer="[peer$peer]"
        fi
        peer_tabs="${peer_tabs} ${peer} ${peer_link}"
    done
fi


# Security
case $FORM_action in
    download)  download_peer_config $FORM_section $FORM_peer
               exit 0;;
    srvcfg)    download_server_config $FORM_section
               exit 0;;
    wg_status) sec_action="view" ;;
    wg_keys)   sec_action="admin" ;;
    *)         sec_action="view"  ;;
esac

check_rights "wireguard" "$sec_action"



show_html_header "WireGuard"
# bindet automatisch Skripte ein wenn richtig benannt /srv/www/admin/OpenVPN.cgi, /srv/www/css/OpenVPN.css, /srv/www/lang/OpenVPN.de


show_tab_header ${interface_tabs}
show_tab_header ${peer_tabs}


case $FORM_section in
    overview)
        show_overview
        ;;
    wg[0-9]*)
        case $FORM_peer in
            all)
                show_server_peers ${FORM_section}
                ;;
            [0-9]*)
                show_peer_info $FORM_section $FORM_peer
                ;;
        *)
            show_error "Error: invalid peer" "The given link is not valid!"
            ;;
        esac
        ;;
    *)
        show_error "Error: invalid section" "The given link is not valid!"
        ;;
esac

show_tab_footer

show_tab_footer

show_html_footer
