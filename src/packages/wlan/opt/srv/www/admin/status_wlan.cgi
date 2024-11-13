#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/srv/www/admin/status_wlan.cgi                          __FLI4LVER__
#
# Creation: Karsten Horsmann 11.12.2005
# Last Update:  $Id$
#------------------------------------------------------------------------------

# set_debug='yes'
# get main helper functions
. /srv/www/include/cgi-helper


ls /var/run/wlan_wlan*_idx.conf | cut -d "_" -f2 > /var/run/wlancgi.tmp
tab=$(sed -ne '1p' /var/run/wlancgi.tmp)
rm /var/run/wlancgi.tmp
: ${FORM_action:="$tab"}
: ${FORM_option:="GENERAL"}
: ${FORM_refresh:="false"}

# Security
check_rights "wlan" "change"

#------------------------------------------------------------------------------
# Funktionen
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# immer wieder genutze Formularfelder
#------------------------------------------------------------------------------
WLAN_FORM(){
  echo "<input type=\"hidden\" name=\"action\" value=\"$FORM_action\">"
  echo "<input type=\"hidden\" name=\"interface\" value=\"$FORM_interface\">"
  echo "<input type=\"hidden\" name=\"option\" value=\"$1\">"
}

#------------------------------------------------------------------------------
# Code um die Daten eines Clients oder AP's heraus zu finden
#------------------------------------------------------------------------------
WLAN_STATION(){
  unset hostname
  unset hostip
  if [ -f /var/run/dnsmasq.pid ]
  then
    if [ -f /etc/dnsmasq.d/dhcp.conf ]
    then
      leasefile=$(sed -n 's/dhcp-leasefile=//p' /etc/dnsmasq.d/dhcp.conf)
      hostname=$(grep -i $1 $leasefile | cut -d " " -f4)
      hostip=$(grep -i $1 $leasefile | cut -d " " -f3)
    fi
    if [ -z $hostname ]
    then
      for i in $(ls /etc/dnsmasq.d/dhcp_hosts_*)
      do
        y=$i
        hostname=$(sed -n "s/.*$1.*,\([^,]\+\),[^,]\+,[^,]\+$/\1/pI" $y)
        hostip=$(sed -n "s/.*$1.*,\([^,]\+\),[^,]\+,[^,]\+,[^,]\+$/\1/pI" $y)
      done
    fi
  else
    hostname=$(arp | grep $1 | cut -d " " -f1)
    hostip=$(arp -n | grep $1 | cut -d " " -f1)
  fi
}

#------------------------------------------------------------------------------
# Überprüfung von eingegebenen MAC-Adressen auf Korrektheit
#------------------------------------------------------------------------------
WLAN_CHECK_MAC(){
  #check mac for correct format 12:45:78:01:34:67
  macok=1
  if ! echo $1 | grep -q -i '[0-9A-F]\{2\}\(:[0-9A-F]\{2\}\)\{5\}'
  then
    macok=0
  fi
  if [ $macok = 0 ]
  then
    show_error "Error: Format of MAC Address" "The format of the MAC Address isn't in proper style!"
    echo "<br />"
    adderror=1
  fi
}

#------------------------------------------------------------------------------
# Zurückschreiben der Datei und löschen von temp Dateien
# WLAN Einstellungen neu laden und WLAN neu starten
#------------------------------------------------------------------------------
WLAN_RELOAD(){
    if [ -f /var/run/wlan_${FORM_interface}.conf.$$ ]
    then
      cat /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf
      rm /var/run/wlan_${FORM_interface}.conf.$$
    fi
    if [ -f /var/run/wlan_${FORM_interface}.conf.$$.tmp ]
    then
      rm /var/run/wlan_${FORM_interface}.conf.$$.tmp
    fi
    if [ "$FORM_vap" = "1" ]
    then
      interface=$(echo $1 | cut -d "v" -f1)
    else
      interface=$1
    fi
    wlanconfig.sh $interface stop
    sleep 2
    wlanconfig.sh $interface
}

#------------------------------------------------------------------------------
# ACL Liste
#------------------------------------------------------------------------------
WLAN_ACL_list() {
(
  . /var/run/wlan_${FORM_interface}.conf
  echo "<table class=\"normtable\">"
  #----------------
  # WLAN_ACL_POLICY
  #----------------
  if [ "$WLAN_ACL_POLICY" = "allow" ]
  then
    acl=$(echo "<option selected value=\"allow\">${_WLAN_WHITELIST}</option><option \
        value=\"deny\">${_WLAN_BLACKLIST}</option><option value=\"open\">${_WLAN_OPEN}</option>");
  elif [ "$WLAN_ACL_POLICY" = "deny" ]
  then
    acl=$(echo "<option value=\"allow\">${_WLAN_WHITELIST}</option><option selected \
        value=\"deny\">${_WLAN_BLACKLIST}</option><option value=\"open\">${_WLAN_OPEN}</option>");
  elif [ "$WLAN_ACL_POLICY" = "open" ]
  then
    acl=$(echo "<option value=\"allow\">${_WLAN_WHITELIST}</option><option value=\"deny\"> \
        ${_WLAN_BLACKLIST}</option><option selected value=\"open\">${_WLAN_OPEN}</option>");
  fi
  echo "<form name=\"acl_list\" action=${myname} method=\"post\">"
  echo "<tr><th>${_WLAN_list}: "
  echo "<select name=\"acl_list\" size=\"1\">"
  echo $acl
  echo "</select>"
  WLAN_FORM ACL
  echo "</th><th><input type=\"submit\" name=\"todo\" value=${_WLAN_change} class=\"actions\">&nbsp;&nbsp;</th></tr>"
  echo "</form>"
    
  #----------------------------------------------------
  # ACL Liste erstellen mit den Radiobutton zum löschen
  #----------------------------------------------------
  n=1
  while [ "$n" -le "$WLAN_ACL_MAC_N" ]
  do
    unset hostname
    unset hostip
    echo "<form name=\"delete_host\" action=${myname} method=\"post\">"
    line=$(sed -n "s/^WLAN_ACL_MAC_${n}='\(.*\)'$/\1/p" < /var/run/wlan_${FORM_interface}.conf)
    WLAN_STATION $line
    echo "<tr><td style=\"text-align:center\">${hostname}<br/>${line}<br/>${hostip}</td><td> \
          <input type=\"hidden\" name=\"mac\" value=\"${line}\"> \
          <input type=\"submit\" name=\"todo\" value=\"${_WLAN_del}\" class=\"actions\"></td></tr>";
    WLAN_FORM ACL
    echo "</form>"
    n=$((n+1))        
  done
    
  #------------------------
  # Formular zum hinzufügen
  #------------------------
  echo "<tr><td colspan=6></br></td></tr>"
  echo "<tr style=\"background-color: #DFDFDF;\">"
  echo "<form name=\"create_host\" action=${myname} method=\"post\">"
  echo "    <td style=\"text-align:center\"><input type=\"text\" name=\"mac\" size=\"15\" value=\"\"></td>"
  WLAN_FORM ACL
  echo "    <td><input type=\"submit\" name=\"todo\" value=\"${_WLAN_add}\" class=\"actions\">&nbsp;&nbsp;</td>"
  echo "</form>"
  echo "</tr>"
  echo "<tr><td colspan=6></td></tr>"
  echo "</table>"
  echo "<br/>"
)
}

#------------------------------------------------------------------------------
# ACL List change (white, black, open)
#------------------------------------------------------------------------------
WLAN_ACL_change() {
  check_rights "status" "change"
  # option muß ausgewertet werden?
  # lösche existierendes ersetze mit neuem
  cp /var/run/wlan_${FORM_interface}.conf /var/run/wlan_${FORM_interface}.conf.$$
  while read -r line
  do
    x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f3)
    if [ "$x" = "POLICY" ]
    then
      line=WLAN_ACL_${x}=\'${FORM_acl_list}\'
    fi
    echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
  done < /var/run/wlan_${FORM_interface}.conf.$$
  mv /var/run/wlan_${FORM_interface}.conf.$$.tmp /var/run/wlan_${FORM_interface}.conf.$$
  WLAN_RELOAD $FORM_action
}

#------------------------------------------------------------------------------
# ACL ADD
#------------------------------------------------------------------------------
WLAN_ACL_add() {
(
  . /var/run/wlan_${FORM_interface}.conf
  check_rights "status" "change"
  if [ "x$FORM_mac" = "x" ]
  then
    show_error "Error: no MAC Address" "no MAC Address was specified"
    echo "<br />"
    adderror=1
  elif grep -i -q $FORM_mac /var/run/wlan_${FORM_interface}.conf  #check for same hostname in any config-file
  then
    show_error "Error: duplicate MAC Address" "The MAC Address $FORM_mac already exists!"
    echo "<br />"
    adderror=1
  else
    WLAN_CHECK_MAC $FORM_mac
  fi
  if [ -n "$adderror" ]
  then
    show_error "Error in data" "No host was added to the ACL configuration. Please try again!"
    echo "<br />"
  else
    mac=$(echo $FORM_mac | tr '[:lower:]' '[:upper:]')
    m=$WLAN_ACL_MAC_N
    n=$WLAN_ACL_MAC_N
    m=$((m+1))
    echo "WLAN_ACL_MAC_${m}='${mac}'" >> /var/run/wlan_${FORM_interface}.conf
    sed -e "s/WLAN_ACL_MAC_N='$n'/WLAN_ACL_MAC_N='$m'/g" /var/run/wlan_${FORM_interface}.conf > /var/run/wlan_${FORM_interface}.conf.$$
    cat /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf
    WLAN_RELOAD $FORM_action
  fi
)    
}

#------------------------------------------------------------------------------
# ACL DEL
#------------------------------------------------------------------------------
WLAN_ACL_del() {
(
  . /var/run/wlan_${FORM_interface}.conf
  check_rights "status" "change"
  if [ "x$FORM_mac" = "x" ]
  then
    show_error "Error: no MAC Address" "no MAC Address was chosen"
    echo "<br />"
    show_error "Error in data" "No host was deleted from the configuration. Please try again!"
    echo "<br />"
  else
    mac=$(echo $FORM_mac | tr '[:lower:]' '[:upper:]')
    #----------------------------
    #Nummer der zu löschenden MAC
    #----------------------------
    l=$(grep $mac /var/run/wlan_${FORM_interface}.conf | cut -d "=" -f1 | cut -d "_" -f4)
    #--------------------------------
    #Anzahl aller MAC vor dem Löschen
    #--------------------------------
    n=$WLAN_ACL_MAC_N
    #---------------------------------
    #Anzahl aller MAC nach dem Löschen
    #---------------------------------
    m=$((WLAN_ACL_MAC_N-1))
    #---------------
    #Löschen der MAC
    #---------------
    sed -e "s/WLAN_ACL_MAC_N='${n}'/WLAN_ACL_MAC_N='${m}'/g" /var/run/wlan_${FORM_interface}.conf > /var/run/wlan_${FORM_interface}.conf.$$
    grep -v WLAN_ACL_MAC_${l} /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf.$$.tmp
    cat /var/run/wlan_${FORM_interface}.conf.$$.tmp > /var/run/wlan_${FORM_interface}.conf.$$
    #--------------------------------------
    #Neu sortieren der MAC nach dem Löschen
    #--------------------------------------
    if [ "$n" -ne "$l" ]
    then
      p=$((l+1))
      while [ "$l" -lt "$n" ]
      do
        sed -e "s/WLAN_ACL_MAC_$p/WLAN_ACL_MAC_$l/g" /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf.$$.tmp
        cat /var/run/wlan_${FORM_interface}.conf.$$.tmp > /var/run/wlan_${FORM_interface}.conf.$$
        l=$((l+1))
        p=$((p+1))
      done
    fi
    #-------------------------------------------------------
    # Zurückschreiben der Datei und löschen von temp Dateien
    #-------------------------------------------------------
    WLAN_RELOAD $FORM_action
  fi
)
}


#------------------------------------------------------------------------------
# Verbundene Devices
#------------------------------------------------------------------------------
WLAN_SCAN() {
  unset ISAP
  ISAP=$(iw dev $FORM_action info | sed -n '/AP/p')  # Ist es ein AP, dann enthält die Statusmeldung "AP"
  if [ "$ISAP" = "" ]
  then
    iw dev $FORM_action scan > /var/run/devices.tmp          # andere APs zeigen (bin client oder adhoc)
  else
    iw dev $FORM_action station dump > /var/run/devices.tmp  # Geräte auflisten, die mit dem FLI verbunden sind
  fi
  echo "<table class=\"normtable\">"
  while read -r line
  do
    case $line in
    Station*)
      echo "<tr><td> </td></tr>"
      echo "<tr><th>"$line
      unset mac
      mac=$(echo $line | cut -d " " -f2)
      WLAN_STATION $mac
      if [ -n $hostname ]
      then
        echo " "$hostname"</th></tr>"
      else
        echo "</th></tr>"
      fi
      echo "<tr><td> </td></tr>"
    ;;
    tx*)
      case $line in
      *MBit*) ;;
      *) line=$(echo $line | sed -f /usr/local/wlan/mcstable.sed);;
      esac
      echo "<tr><td>"$line"</td></tr>"
    ;;
    BSS*)
      echo "<tr><td> </td></tr>"
      echo "<tr><th>"$line"</th></tr>"
    ;;
    freq*)
      echo "<tr><td>"$line" Mhz</td></tr>"
    ;;
    SSID*)
      echo "<tr><td>"$line"</td></tr>"
    ;;
    DS*)
      line=`echo $line | cut -d " " -f4,5`
      echo "<tr><td>"$line"</td></tr>"
    ;;
    signal*)
      echo "<tr><td>"$line"</td></tr>"
    ;;
    esac
  done < /var/run/devices.tmp
  echo "<tr><td> </td></tr>"
  echo "<tr><th>Updated: $(date -R)</th></tr>"
  echo "</table>"
  echo "<br/>"
  rm /var/run/devices.tmp
}

#------------------------------------------------------------------------------
#  General
#------------------------------------------------------------------------------
WLAN_GENERAL() {
(
  . /var/run/wlan_${FORM_interface}.conf
  echo "<table class=\"normtable\">"
  echo "<form name=\"edit\" action=\"$myname\" method=\"post\">"
  echo "<tr><th>ESSID:</th><th colspan=\"5\"><input maxLength=\"32\" name=\"essid\" size=\"32\" value=\"$WLAN_ESSID\"></th></tr>"
  case ${FORM_vap} in
  1)
    dev=$(echo $FORM_action | cut -d "v" -f1)
    WLAN_CHANNEL=$(grep WLAN_CHANNEL /var/run/wlan_${dev}.conf | cut -d "'" -f2)
  ;;
  esac
  echo "<tr><td>${_WLAN_Channel}:</td><td colspan=\"5\"><input maxLength=\"5\" name=\"chan\" size=\"5\" value=\"$WLAN_CHANNEL\"></td></tr>"
  echo "<tr><td>${_WLAN_MAC}:</td><td colspan=\"5\"><input maxLength=\"19\" name=\"mac\" size=\"19\" value=\"$FORM_interface\" readonly></td></tr>"
  echo "<tr><td>${_WLAN_Mode}:</td><td colspan=\"5\"><input maxLength=\"7\" name=\"mode\" size=\"6\" value=\"$WLAN_MODE\"></td></tr>"
  echo "<tr><td>${_WLAN_Rate}:</td><td colspan=\"5\"><input maxLength=\"5\" name=\"rate\" size=\"5\" value=\"$WLAN_RATE\" readonly></td></tr>"
  if [ -n "$WLAN_DIVERSITY" ]
  then
    echo "<tr><td>${_WLAN_Diversity}:</td><td><input maxLength=\"3\" name=\"diversity\" size=\"3\" value=\"$WLAN_DIVERSITY\"></td>"
    case $WLAN_DIVERSITY in
    no)
      echo "<td>RX:</td><td><input maxLength=\"1\" name=\"rx\" size=\"1\" value=\"\"></td>"
      echo "<td>TX:</td><td><input maxLength=\"1\" name=\"tx\" size=\"1\" value=\"\"></td></tr>"
    ;;
    *)
      echo "<td>RX:</td><td><input maxLength=\"1\" name=\"rx\" size=\"1\" value=\"$WLAN_DIVERSITY_RX\"></td>"
      echo "<td>TX:</td><td><input maxLength=\"1\" name=\"tx\" size=\"1\" value=\"$WLAN_DIVERSITY_TX\"></td></tr>"
    ;;
    esac
  fi
  echo "<tr><td></td><td colspan=\"5\"><input type=\"submit\" value=\"${_WLAN_change}\" name=\"todo\" class=\"actions\"></td></tr>"
  WLAN_FORM GENERAL
  echo "</form>"
  vap=$(echo ${FORM_action} | grep "v")
  if [ -z "${vap}" ]
  then
    echo "<form action=\"$myname\" method=\"post\">"
    WLAN_FORM GENERAL
    echo "<tr><td colspan=\"6\"></td></tr>"
    if [ -f /var/run/hostapd_${FORM_action}.pid ] || [ -f /var/run/wpa_supplicant_${FORM_action}.pid ]
    then
      echo "<tr><td>Status: <img src=\"../img/online.png\" alt=\"online\"></td><td colspan=\"5\">"
      echo "<input type=\"submit\" value=\"${_WLAN_off}\" name=\"todo\" class=\"actions\"></td></tr>"
    else
      echo "<tr><td>Status: <img src=\"../img/offline.png\" alt=\"offline\"></td><td colspan=\"5\">"
      echo "<input type=\"submit\" value=\"${_WLAN_on}\" name=\"todo\" class=\"actions\"></td></tr>"
    fi
    echo "</p>"
    echo "</form>"
  fi
  echo "</table>"
  echo "<br/>"
)
}

#------------------------------------------------------------------------------
#  General change
#------------------------------------------------------------------------------

WLAN_GENERAL_CHANGE() {
#lösche existierendes ersetze mit neuem
  cp /var/run/wlan_${FORM_interface}.conf /var/run/wlan_${FORM_interface}.conf.$$
  for i in FORM_essid FORM_chan FORM_mode FORM_diversity FORM_rx FORM_tx
  do
    case $i in
    FORM_essid)
      while read -r line
      do
        x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f2)
        if [ "$x" = "ESSID" ]
        then
          line="WLAN_${x}='${FORM_essid}'"
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;  
    FORM_chan)
      while read -r line
      do
        x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f2)
        if [ "$x" = "CHANNEL" ]
        then
          line="WLAN_${x}='${FORM_chan}'"
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;
    FORM_mode)
      while read -r line
      do
        x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f2)
        if [ "$x" = "MODE" ]
        then
          if [ "${FORM_mode}" = "master" || "${FORM_mode}" = "managed" || "${FORM_mode}" = "ad-hoc" ]
          then
            line="WLAN_${x}='${FORM_mode}'"
          fi
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;
    FORM_diversity)
      while read -r line
      do
        x=$(echo $line | cut -d "'" -f1 | cut -d "_" -f2)
        if [ "$x" = "DIVERSITY=" ]
        then
          line="WLAN_${x}'${FORM_diversity}'"
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;
    FORM_rx)
      while read -r line
      do
        x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f2,3)
        if [ "$x" = "DIVERSITY_RX" ]
        then
          line="WLAN_${x}='${FORM_rx}'"
          echo "</br>"
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;
    FORM_tx)
      while read -r line
      do
        x=$(echo $line | cut -d "=" -f1 | cut -d "_" -f2,3)
        if [ "$x" = "DIVERSITY_TX" ]
        then
          line="WLAN_${x}='${FORM_tx}'"
        fi
        echo $line >> /var/run/wlan_${FORM_interface}.conf.$$.tmp
      done < /var/run/wlan_${FORM_interface}.conf.$$
    ;;
    esac
    mv /var/run/wlan_${FORM_interface}.conf.$$.tmp /var/run/wlan_${FORM_interface}.conf.$$
  done
  #-------------------------------------------------------
  # Zurückschreiben der Datei und löschen von temp Dateien
  # Config neu laden
  #-------------------------------------------------------
  cat /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf
  unset ISAP
  ISAP=$(iw dev $device info | sed -n '/AP/p')  # Ist es ein AP, dann enthält die Statusmeldung "AP"
  if [ "$ISAP" = "" ]
  then
    if type -p dhcpcd >/dev/null
    then
      pid=$(cat /var/run/dhcpcd-$device.pid)
      kill $pid
      WLAN_RELOAD $FORM_action
      dhcpcd $device
    elif type -p dhclient >/dev/null
    then
      kill $(cat /var/run/dhclient.pid)
      addr=$(ip addr show dev $device | grep inet | cut -d " " -f 6 | cut -d "/" -f 1)
      ip addr del $addr1 dev $device
      WLAN_RELOAD $FORM_action
      dhclient -sf /etc/dhclient-script $device
    fi
  else
    WLAN_RELOAD $FORM_action
  fi
}

#------------------------------------------------------------------------------
#  Security
#------------------------------------------------------------------------------
WLAN_SEC_WPS(){

  if [ "$mode" = "master" -a "$wps" = "yes" ]
  then
    echo "<form name=\"wps\" action=\"$myname\" method=\"post\">"
    echo "<tr><td></td><td colspan="2"></td></tr>"
    echo "<tr><th>${_WLAN_WPS}:</th><th colspan=\"2\">${_WLAN_Pin_and_PBC}</th></tr>"
    echo "<tr><td>${_WLAN_Pin}:</td><td colspan=\"2\"><input maxLength=\"8\" name=\"wps_pin\" size=\"8\" \
      value=\"\"><input type=\"submit\" name=\"todo\" value=\"${_WLAN_set}\" class=\"actions\">&nbsp;&nbsp;</td></tr>"
    echo "<tr><td>${_WLAN_Push_Button_Config}</td><td colspan=\"2\"><input type=\"submit\" value=\"${_WLAN_pbc}\" name=\"todo\" class=\"actions\"></td></tr>"
    WLAN_FORM SECURITY
    echo "</form>"
    #------------------------------------------------------------------------------
    # WPS-Key Liste erstellen mit Schaltflächen zum löschen
    #------------------------------------------------------------------------------
    path=/var/lib/persistent
    wlan=$(echo $WLAN_ESSID)
    file=$path/wlan/hostapd.$FORM_action.psk.$wlan
    echo "<tr><td></td><td colspan=\"2\"></td></tr>"
    echo "<tr><th>WPS Keys:</th><th colspan=\"2\"></th></tr>"
    if [ -s $file ]
    then
      while read -r line
      do
        unset mac
        mac=${line%% *}
        WLAN_STATION $mac
        echo "<tr>"
        echo "<form name=\"del\" action=\"$myname\" method=\"post\">"
        echo "<input type=\"hidden\" name=\"file\" value=\"$file\"><td style=\"text-align:center\">"
        if [ -n ${hostname} -a -n ${hostip} ]
        then
            echo ${hostname}"<br/>"${mac}"<br/>"${hostip}"<br/>"
        else
            echo ${mac}
        fi
        echo "</td><td style=\"text-align:center\">${line#* }</td><td><input type=\"hidden\" \
          name=\"mackey\" value=\"$line\"><input type=\"submit\" name=\"todo\" \
          value=\"${_WLAN_del}\" class=\"actions\"></td>";
        WLAN_FORM SECURITY
        echo "<input type=\"hidden\" name=\"file\" value=\"$file\">"
        echo "</form></tr>";
      done < $file
      echo "<tr><td colspan=\"3\"></br></td></tr>"
    fi
    #------------------------------------------------------------------------------
    # WPS-Key Add
    #------------------------------------------------------------------------------  
    echo "<tr><td style=\"text-align:center\">${_WLAN_MAC}</td><td style=\"text-align:center\">${_WLAN_Key}</td><td></td></tr>"
    echo "<tr>"
    echo "<form name=\"add\" action=\"$myname\" method=\"post\">"
    echo "<td style=\"text-align:center\"><input type=\"text\" name=\"mackey1\" \
      size=\"15\" value=\"\"></td><td><input type=\"text\" name=\"keymac\" size=\"64\" value=\"\">"
    WLAN_FORM SECURITY
    echo "<input type=\"hidden\" name=\"file\" value=\"$file\">"
    echo "<td><input type=\"submit\" name=\"todo\" value=\"${_WLAN_add}\" class=\"actions\">&nbsp;&nbsp;</td>"
    echo "</form></tr>"
  fi
}

WLAN_SEC_KEY_HIDDEN(){
  echo "<input maxLength=\"64\" name=\"key\" size=\"64\" value=\"*************\">"
  WLAN_FORM SECURITY
  echo "<input type=\"submit\" value=\"${_WLAN_show_plain}\" name=\"todo\" class=\"actions\">"
  echo "<tr><td></td><td colspan=\"5\"><input type=\"submit\" value=\"${_WLAN_change}\" name=\"todo\" class=\"actions\"></td></tr>"
}

WLAN_SEC_KEY_SHOW(){
  echo "<input maxLength=\"64\" name=\"key\" size=\"64\" value=\"$key\">"
  WLAN_FORM SECURITY
  echo "<input type=\"submit\" value=\"${_WLAN_show_star}\" name=\"todo\" class=\"actions\">"
  echo "<tr><td></td><td colspan=\"5\"><input type=\"submit\" value=\"${_WLAN_change}\" name=\"todo\" class=\"actions\"></td></tr>"
  if [ "$mode" = "master" ]
  then
    if [ "$type" = "WPA-PSK" ]
    then
      wlan_qrcode="WIFI:S:$WLAN_ESSID;T:WPA;P:$key;;"
    elif [ "$type" = "WEP" ]
    then
      wlan_qrcode="WIFI:S:$WLAN_ESSID;T:WEP;P:$key;;"
    fi
    echo "<tr><td>${_WLAN_qrcode}:</td><td colspan=\"2\" align=\"center\">"
    echo "<img alt=\"${_WLAN_qralt}\" title=\"${_WLAN_qralt}\" style=\"width: 250px; height: auto;\" src=\"data:image/svg+xml;base64,"
    qrencode -t SVG --foreground=27613a -o - "$wlan_qrcode" | base64
    echo "\">"
    echo "</td></tr>"
  fi
}

WLAN_SECURITY() {
(
  . /var/run/wlan_${FORM_interface}.conf
  if [ -f /var/run/dhcp_leases_dir ]
  then
    . /var/run/dhcp_leases_dir
  fi
  #Sicherheit
  wpa=$WLAN_WPA_ENCRYPTION
  wep=$WLAN_ENC_N
  wps=$WLAN_WPS
  mode=$WLAN_MODE
  if [ "$wpa" ]
  then
    key=$WLAN_WPA_PSK
    type=$WLAN_WPA_KEY_MGMT
    enc=$WLAN_WPA_ENCRYPTION
    wpa=$WLAN_WPA_TYPE
    n=$WLAN_ENC_N
  elif [ "$wep" -gt 0 ]
  then
    type=WEP
    n=$WLAN_ENC_N
    i=0
    j=1
    while [ j -ge n ]
    do
      key=WLAN_ENC_$j
      i=$((i+1))
      j=$((j+1))
    done
    enc=echo "TKIP"        
  else
    type=off
    enc=test
  fi
  echo "<table class=\"normtable\">"
  echo "<form name=\"edit\" action=\"$myname\" method=\"post\">"
  echo "<tr><th>${_WLAN_SECURITY}:</th><th colspan=\"5\">$type</th></tr>"
  echo "<tr><td>${_WLAN_Encryption}:</td><td  colspan=\"5\">$enc</td></tr>"
  if [ "$wpa" = "1" -o "$wpa" = "2" ]
  then
    echo "<tr><td>${_WLAN_WPA_Type}:</td><td colspan=\"5\">${wpa}</td></tr>"
  elif [ "$wpa" = "3" ]
  then
    echo "<tr><td>${_WLAN_WPA_Type}:</td><td colspan=\"5\">1 & 2</td></tr>"
  fi
  i=0
  j=1
  echo "<tr><td>${_WLAN_Key}:</td><td colspan=\"5\">"
  case x$1 in
  xshow_plain)
    WLAN_SEC_KEY_SHOW
  ;;
  xchange)
    while read -r line
    do
      line1=$(echo $line | cut -d "'" -f1)
      case $line1 in
      WLAN_WPA_PSK=)
        if [ "x$FORM_key" = "x" ]
        then
          show_error "Error: no key" "no key was specified!"
          echo "<br/>"
          adderror=1
        elif ! echo "$FORM_key" | grep -q "^[][a-zA-Z0-9.,;:!?|/+*_^@\\#\$%&()\`<=>{}~-]\{16,63\}$"
        then
          show_error "Error: key doesn't fit" "The key contains unsupported characters!"
          echo "<br/>"
          adderror=1
        fi  
        if [ -n "$adderror" ]
        then
          show_error "Error in data" "Key wasn't changed. Please try again!"
          echo "<br/>"
        else
          line=WLAN_WPA_PSK=\'$FORM_key\'
        fi 
      ;;
      esac
      echo $line >> /var/run/wlan_${FORM_interface}.conf.$$
    done < /var/run/wlan_${FORM_interface}.conf
    if [ -z "$adderror" ]
    then
      cat /var/run/wlan_${FORM_interface}.conf.$$ > /var/run/wlan_${FORM_interface}.conf
      WLAN_RELOAD $FORM_action
      WLAN_SEC_KEY_HIDDEN
    fi
    rm /var/run/wlan_${FORM_interface}.conf.$$
    unset adderror
  ;;
  xwps_set)
    bdev=$(echo ${FORM_action} | cut -d "v" -f1)
    hostapd_cli -p /var/run/hostapd.$bdev -i $FORM_action wps_pin any $FORM_wps_pin 120 >/dev/null 2>&1
    WLAN_SEC_KEY_HIDDEN
  ;;
  xwps_pbc)
    bdev=$(echo ${FORM_action} | cut -d "v" -f1)
    hostapd_cli -p /var/run/hostapd.$bdev -i $FORM_action wps_pbc >/dev/null 2>&1
    WLAN_SEC_KEY_HIDDEN
  ;;
  xwps_del)
    sed -ie "/^${FORM_mackey}$/d" $FORM_file
    id=$(ps | grep hostapd | grep $FORM_action | cut -d " " -f2)
    kill -s 1 $id 
    WLAN_SEC_KEY_HIDDEN
  ;;
  xwps_add)
    if [ "x$FORM_mackey1" = "x" ]
    then
      show_error "Error: no MAC Address" "no MAC Address was specified"
      echo "<br/>"
      adderror=1
    elif [ -n "$(grep -i $FORM_mackey1 $FORM_file)" ]  #check for same mac-address in any config-file
    then
      show_error "Error: duplicate mac address" "The mac address $FORM_mackey1 already exists!"
      echo "<br/>"
      adderror=1
    else
      #check mac for correct format 12:45:78:01:34:67
      WLAN_CHECK_MAC $FORM_mackey1
    fi
    if [ "x$FORM_keymac" = "x" ]
    then
      show_error "Error: no key" "no key was specified!"
      echo "<br/>"
      adderror=1
    elif ! echo "$FORM_keymac" | grep -q "^[][a-zA-Z0-9.,;:!?|/+*_^@\\#\$%&()\`<=>{}~-]\{16,63\}$"
    then
      show_error "Error: key doesn't fit" "The key contains unsupported characters!"
      echo "<br/>"
      adderror=1
    fi  
    if [ -n "$adderror" ]
    then
      show_error "Error in data" "No host was added. Please try again!"
      echo "<br/>"
    else
      mac=$(echo $FORM_mackey1 | tr '[:upper:]' '[:lower:]')
      echo $mac" "$FORM_keymac >> $FORM_file
      id=$(ps | grep hostapd | grep $FORM_action | cut -d " " -f2)
      kill -s 1 $id 
    fi
    WLAN_SEC_KEY_HIDDEN
  ;;
  x)
    WLAN_SEC_KEY_HIDDEN
  ;;    
  esac
  WLAN_FORM SECURITY
  echo "</form>"
  WLAN_SEC_WPS
  echo "</table>"
  echo "<br/>"
)
}

#------------------------------------------------------------------------------
#  Card Info
#------------------------------------------------------------------------------
WLAN_card() {
  phy=$(grep ${FORM_interface} /var/run/wlanphy.conf | cut -d "=" -f1)
  echo "<br/>"
  #--------
  #Frequenz
  #--------
  echo "<table class=\"normtable\">"
  echo "<tr><th>${_WLAN_MAC}:</th><th colspan=\"2\">${FORM_interface}</th></tr>"
  echo "<tr><td colspan=\"3\"><br/></td></tr>"
  echo "<tr><th>${_WLAN_Channel}:</th><th>${_WLAN_Frequency}:</th><th>Info:</th></tr>"
  iw phy ${phy} info | sed -n '/MHz/s/.*\* \([0-9]\{4\}\).*\[\([0-9]\{1,3\}\)\] *(\(.*\))$/\1 \2 (\3)/p' |\
  while read -r line
  do
    set $line
    case $3$4$5$6$7$8$9$10 in
    *disabled*) ;;
    *radar*) ;;
    *)
      echo "<tr><td>$2</td><td>$1 MHz</td><td>$3 $4</td></tr>"
    ;;
    esac
  done
  #--------
  #Bitrates
  #--------
  echo "<tr><td colspan=\"3\"><br/></td></tr>"
  ht=$(iw phy ${phy} info | sed -e 's/^ *[^ ]* //;/non-HT/ !d')
  ht=$(echo $ht | cut -d " " -f1)
  echo "<tr><th colspan=\"2\">${_WLAN_Bitrates}:</th><th>$ht</th></tr>"
  iw phy ${phy} info | sed -e 's/^ *[^ ]* //;/Mbps/ !d' | sort -k 1n -u |\
  while read -r line
  do
    set $line
    echo "<tr><td>$1</td><td>$2</td><td>$3 $4 $5</td></tr>"
  done
  echo "</table>"
  echo "<br/>"
}

wlan_statistics () {
#------------------------------------------------------------------------------
#  rrdtool
#------------------------------------------------------------------------------
  . /srv/www/include/rrd-common.inc
  . /srv/www/include/rrd-wlan.inc
  dev=$FORM_action
  FORM_action="$dev&option=STATISTICS"
  : ${FORM_rrd_graphtime_wlan:=$rrd_default_graphtime}
  rrd_open_tab_list wlan
  rrd_render_graph wlan
  rrd_close_tab_list
  FORM_action=$dev
}

#------------------------------------------------------------------------------
#  Hauptteil
#
# Ein wenig Doku für die Übersicht:
#
# Var               Inhalt
# FORM_interface    Mac der WLAN-Karte
# FORM_action       Interface der Karte (wlanX)
# FORM_option       ausgewählter Tab
# FORM_todo         was soll gemacht werden
#
#------------------------------------------------------------------------------
show_html_header "WLAN"

local maintabs=""
local source=""
local label=""
local label_lng_file=""

#------------------------------------------------------------------------------
# Überprüfung der Mac-Adressen ob diese vorhanden und richtig sind
#------------------------------------------------------------------------------
for i in $(ls /var/run/wlan_*_idx.conf | cut -d " " -f1)
do
  j=$(cat "$i" | cut -d "_" -f2 | cut -d "." -f1)
  mac=$(echo $i | cut -d "_" -f2)
  case $j in
  default)
    show_error "Error: Typo in MAC Address!" "The given MAC Address is not \
      correct! </br> Please change it to: </br> WLAN_X_MAC='$mac'"
    show_html_footer
    break
  ;;
  esac
done

#------------------------------------------------------------------------------
# Anlegen der Tabs für die einzelnen WLANs
#------------------------------------------------------------------------------
for idx in $(ls /var/run/wlan_wlan*_idx.conf | cut -d "_" -f2 | cut -d "." -f1)
do
  label=$(sed -n "s/^WLAN_ESSID='\(.*\)'$/\1/p" < /var/run/wlan_"$idx".conf)
  label="$label (${idx})"
  FORM_interface=$(sed -n "s/^WLAN_MAC='\(.*\)'$/\1/p" < /var/run/wlan_"$idx".conf)
  label=$(echo ${label} | htmlspecialchars)

  # concatinate string for show_tab_header function
  # and choose whether tab is active or inactive
  [ "$idx" != "$FORM_action" ] \
    && maintabs="${maintabs}\"${label}\" \"${myname}?action=${idx}\" " \
    || maintabs="${maintabs}\"${label}\" \"no\" "
done

eval "show_tab_header $maintabs"

FORM_interface=$(sed -n "s/^WLAN_MAC='\(.*\)'$/\1/p" < /var/run/wlan_"$FORM_action".conf)
local subtabs=""
local option="GENERAL"
local option_var_name=""

#------------------------------------------------------------------------------
# Anlegen der Tabs für die einzelnen Einstellungen je nach WLAN-Mode
#------------------------------------------------------------------------------
mode=$(sed -n "s/^WLAN_MODE='\(.*\)'$/\1/p" < /var/run/wlan_${FORM_interface}.conf)

unset vap
vap=$(echo ${FORM_action} | grep "v")
if [ -n "${vap}" ]
then
  z="GENERAL SECURITY SCAN ACL"
  FORM_vap=1
elif [ -f /var/run/collectdmon.pid -a \( "$mode" = "managed" -o "$mode" = "ad-hoc" \) ]
then
  z="GENERAL SECURITY SCAN CARD STATISTICS"
  FORM_vap=0
elif [ ! -f /var/run/collectdmon.pid -a \( "$mode" = "managed" -o "$mode" = "ad-hoc" \) ]
then
  z="GENERAL SECURITY SCAN CARD"
  FORM_vap=0
else
  z="GENERAL SECURITY SCAN ACL CARD"
  FORM_vap=0
fi

for idy in $z
do
  # get label
  case $idy in
  GENERAL)
    eval label=${_WLAN_GENERAL}
  ;;
  SECURITY)
    eval label=${_WLAN_SECURITY}
  ;;
  SCAN)
    idz=$(sed -n "s/^WLAN_MODE='\(.*\)'$/\1/p" < /var/run/wlan_${FORM_interface}.conf)
    case $idz in
    master)
      label=${_WLAN_CONNECTED_DEVS}
    ;;
    managed)
      label=${_WLAN_SCANNED_APS}
    ;;
    ad-hoc)
      label=${_WLAN_CONNECTED_DEVS}
    ;;
    esac
  ;;
  ACL)
    label=${_WLAN_ACL_LIST}
  ;;
  CARD)
    label=${_WLAN_CARD}
  ;;
  STATISTICS)
    label=${_WLAN_STATISTICS}
  ;;
  esac
  # concatinate string for show_tab_header function
  # and choose whether tab is active or inactive
  [ "$idy" != "$FORM_option" ] \
    && subtabs="${subtabs}\"${label}\" \"${myname}?action=${FORM_action}&amp;option=${idy}\" " \
    || subtabs="${subtabs}\"${label}\" \"no\" "
done

eval "show_tab_header $subtabs"

case $FORM_option
in
GENERAL)
  case $FORM_todo
  in
  ${_WLAN_change})
    WLAN_GENERAL_CHANGE
    WLAN_GENERAL
  ;;
  ${_WLAN_on})
    wlanconfig.sh $FORM_action
    WLAN_GENERAL
  ;;
  ${_WLAN_off})
    wlanconfig.sh $FORM_action down
    WLAN_GENERAL
  ;;
  *)
    WLAN_GENERAL
  ;;
  esac
;;

SECURITY)
  case $FORM_todo
  in
  ${_WLAN_show_plain})
    WLAN_SECURITY show_plain
  ;;
  ${_WLAN_show_star})
    WLAN_SECURITY
  ;;
  ${_WLAN_change})
    WLAN_SECURITY change
  ;;
  ${_WLAN_set})
    WLAN_SECURITY wps_set
  ;;
  ${_WLAN_pbc})
    WLAN_SECURITY wps_pbc
  ;;
  ${_WLAN_del})
    WLAN_SECURITY wps_del
  ;;
  ${_WLAN_add})
    WLAN_SECURITY wps_add
  ;;
  *)
    WLAN_SECURITY
  ;;
  esac
;;

SCAN)
  WLAN_SCAN
  ;;

ACL)
  case $FORM_todo
  in
  ${_WLAN_del})
    WLAN_ACL_del
  ;;
  ${_WLAN_add})
    WLAN_ACL_add
  ;;
  ${_WLAN_change})
    WLAN_ACL_change
  ;;
  esac
  WLAN_ACL_list
;;
CARD)
  WLAN_card
;;
STATISTICS)
  wlan_statistics
;;
esac

show_tab_footer
show_tab_footer
show_html_footer
