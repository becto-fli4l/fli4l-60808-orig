#!/bin/sh

. /srv/www/include/cgi-helper

show_html_header "DHCP"

LEASES_FILE="/var/run/iscdhcpd.leases"

# sends Signal HUP to dhcpd (writes the content of leases to lease-file)
killall -HUP dhcpd

sm=S
TMPDIR=/tmp/$$
rm -rf $TMPDIR
mkdir -p $TMPDIR/active
mkdir -p $TMPDIR/other

# Lese die ISC DHCP Leasedatei aus und benutze das Dateisystem
# als assoziatives Array. Da in der Leasedatei mehrer Eintraege
# fuer die gleiche Lease vorhanden sein koennen und nur der letzte
# gueltig ist schreiben wir einfach alle Leases in ein Verzeichnis
# und ueberschreiben so alte Eintraege automatisch. Zum Parsen
# der Leasedatei nutzen wir zwei verschachtelte case Strukturen.
# Das ist definitv nicht 100% bullet-prof, aber besser als nichts.
# Die aus der Leasedatei ermittelten Daten schreiben wir so in die
# temporaeren Dateien, das wir sie spaeter einfach per Shell source
# wieder einlesen koennen.
sed -e 's/`/!/g' $LEASES_FILE | while read line
do

  case $sm in
      L)
          case $line in
              starts\ *)
                  starts=`echo $line|sed -e 's/starts [0-9] \([^;]\+\).*/\1/'`
                  ;;
              ends\ *)
                  ends=`echo $line|sed -e 's/ends [0-9] \([^;]\+\).*/\1/'`
                  ;;
              cltt\ *)
                  cltt=`echo $line|sed -e 's/cltt [0-9] \([^;]\+\).*/\1/'`
                  ;;
              binding\ state\ *)
                  state=`echo $line|sed -e 's/binding\ state\ \([^;]\+\).*/\1/'`
                  ;;
              next\ binding\ state\ *)
                  nextstate=`echo $line|sed -e 's/next binding\ state\ \([^;]\+\).*/\1/'`
                  ;;
              hardware\ ethernet\ *)
                  mac=`echo $line|sed -e 's/hardware ethernet \([^;]\+\).*/\1/'`
                  ;;
              uid\ *)
                  uid=`echo $line|sed -e 's/uid \"\([^\"]\+\)\".*/\1/'`
                  ;;
              client-hostname\ *)
                  hostname=`echo $line|sed -e 's/client-hostname \"\([^\"]\+\)\".*/\1/'`
                  ;;
              })
	          [ $state = "active" ] && subdir="active" || subdir="other"
                  sm=S
                  datei=`echo $lease|tr . _`
                  {
                      echo "ip=\"$lease\""
                      echo "mac=\"$mac\""
                      echo "hostname=\"$hostname\""
                      echo "starts=\"$starts\""
                      echo "ends=\"$ends\""
                      echo "cltt=\"$cltt\""
                      echo "state=\"$state"\"
                      echo "nextstate=\"$nextstate"\"
                      echo "uid=\"$uid\""
                  }>$TMPDIR/$subdir/$datei
                  ;;
              *)
                  ;;
          esac
          ;;
      S)
          case $line in
              ^#.*)
                  ;;

              lease\ *)
                  lease=`echo $line|sed -e 's/lease \([^ ]\+\).*/\1/'`
                  sm=L
                  ;;

              *)
                  ;;
          esac
          ;;
      *)
          ;;
  esac

done

show_tab_header "aktive DHCP-Leases: $LEASES_FILE" "no"
echo "<table class=\"normtable\">"
echo "<tr><th>Status</th><th>Hostname</th><th>IP-Adresse</th><th>Mac-Adresse</th><th>Zugeteilt am<br />[Datum Uhrzeit]</th><th>Letzter Kontakt am<br />[Datum Uhrzeit]</th><th>L&auml;uft ab<br />[Datum Uhrzeit]</th><th>UID</th></tr>"

for datei in `ls $TMPDIR/active`
do
    .  $TMPDIR/active/$datei

    echo "<tr><td><center>$state</center></td><td><center>$hostname</center></td><td><center>$ip</center></td><td><center>$mac</center></td><td><center>$starts</center></td><td><center>$cltt</center></td><td><center>$ends</center></td><td><center>$uid</center></td></tr>"

done

echo "</table>"
show_tab_footer

echo "<br />"

show_tab_header "andere DHCP-Leases: $LEASES_FILE" "no"
echo "<table class=\"normtable\">"
echo "<tr><th>Status</th><th>Hostname</th><th>IP-Adresse</th><th>Mac-Adresse</th><th>Zugeteilt am<br />[Datum Uhrzeit]</th><th>Letzter Kontakt am<br />[Datum Uhrzeit]</th><th>L&auml;uft ab<br />[Datum Uhrzeit]</th><th>UID</th></tr>"

for datei in `ls $TMPDIR/other`
do
    .  $TMPDIR/other/$datei

    echo "<tr><td><center>$state</center></td><td><center>$hostname</center></td><td><center>$ip</center></td><td><center>$mac</center></td><td><center>$starts</center></td><td><center>$cltt</center></td><td><center>$ends</center></td><td><center>$uid</center></td></tr>"

done

rm -rf $TMPDIR

echo "</table>"
show_tab_footer

echo "<br />"
show_tab_header "DHCP-Ranges" "no"
echo "<table class=\"normtable\">"
echo "<tr>"
echo "<th>Network</th>"
echo "<th>Start</th>"
echo "<th>End</th>"
echo "<th>Leasetime default/max[sec]</th>"
echo "<th>Subnetmask</th>"
echo "<th>Broadcast</th>"
echo "<th>Defaultrouter</th>"
echo "<th>DNS-Domain</th>"
echo "<th>DNS-Server</th>"
echo "<th>NTP-Server</th>"
echo "</tr>"

for datei in `ls /etc/dhcpd.d/dhcp_range_*.conf`
do
    netid=`echo $datei|sed -e 's/.*dhcp_range_NET\([[:digit:]]\+\).conf$/\1/'`
    sm=S
    while read line
    do
	case $sm in
	    L)
		case $line in
		    default-lease-time\ *)
			default_lease_time=`echo $line|sed -e 's/default-lease-time \([^;]\+\).*/\1/'`
			;;
		    max-lease-time\ *)
			max_lease_time=`echo $line|sed -e 's/max-lease-time \([^;]\+\).*/\1/'`
			;;
		    range\ *)
			range_start=`echo $line|sed -e 's/range \([^ ]\+\).*/\1/'`
			range_end=`echo $line|sed -e 's/range [^ ]\+ \([^;]\+\).*/\1/'`
			;;
		    option\ routers\ *)
			def_router=`echo $line|sed -e 's/option routers \([^;]\+\).*/\1/'`
			;;
		    option\ domain-name\ *)
			dns_domain=`echo $line|sed -e 's/option domain-name \"\([^\"]\+\)\".*/\1/'`
			;;
		    option\ domain-name-servers\ *)
			dns_server=`echo $line|sed -e 's/option domain-name-servers \([^;]\+\).*/\1/'`
			;;
		    option\ broadcast-address\ *)
			broadcast=`echo $line|sed -e 's/option broadcast-address \([^;]\+\).*/\1/'`
			;;
		    option\ subnet-mask\ *)
			subnet_mask=`echo $line|sed -e 's/option subnet-mask \([^;]\+\).*/\1/'`
			;;
		    option\ ntp-servers\ *)
			ntp_server=`echo $line|sed -e 's/option ntp-servers \([^;]\+\).*/\1/'`
			;;
		    })
                    sm=S
		    echo "<tr>"
		    echo "<td><center>&nbsp;&nbsp;&nbsp;IP_NET_$netid&nbsp;&nbsp;&nbsp;</center></td>"
		    echo "<td><center>$range_start</center></td>"
		    echo "<td><center>$range_end</center></td>"
		    echo "<td><center>$default_lease_time / $max_lease_time</center></td>"
		    echo "<td><center>$subnet_mask</center></td>"
		    echo "<td><center>$broadcast</center></td>"
		    echo "<td><center>$def_router</center></td>"
		    echo "<td><center>$dns_domain</center></td>"
		    echo "<td><center>$dns_server</center></td>"
		    echo "<td><center>$ntp_server</center></td>"
		    echo "</tr>"
                    ;;
		    *)
			;;
		esac
		;;
	    S)
		case $line in
		    subnet\ *)
			subnet=`echo $line|sed -e 's/subnet \([^ ]\+\).*/\1/'`
                        #subnet_mask=`echo $line|sed -e 's/subnet [^ ]\+[ ]\+netmask \([^ ]\+\).*/\1/'`
			default_lease_time=
			max_lease_time=
			range_start=
			range_end=
			subnet_mask=
			broadcast=
			def_router=
			dns_domain=
			dns_server=
			ntp_server=
			sm=L
			;;
		    
		    *)
			;;
		esac
		;;
	    *)
		;;
	esac
    done<$datei
done

echo "</table>"
show_tab_footer


if [ ! -f /etc/dhcpd.d/dhcp_hosts_NET*.conf ]
then
    show_info "Hinweis" "Es wurden keine DHCP-Hosts konfiguriert."
else
    echo "<br />"
    show_tab_header "DHCP-Hosts" "no"
    echo "<table class=\"normtable\">"
    echo "<tr><th>Network</th><th>Hostname</th><th>IP-Adresse</th><th>MAC-Adresse</th><th>Leasetime default/max[sec]</th></tr>"
    for datei in `ls /etc/dhcpd.d/dhcp_hosts_*.conf`
    do
	netid=`echo $datei|sed -e 's/.*dhcp_hosts_NET\([[:digit:]]\+\).conf$/\1/'`
	mkdir -p $TMPDIR
	sm=I
        anzahl=0
	while read line
	do
	    case $sm in
		H)
		    case $line in
			hardware\ ethernet\ *)
			    mac=`echo $line|sed -e 's/hardware ethernet \([^;]\+\).*/\1/'`
			    ;;
			fixed-address\ *)
			    ipaddr=`echo $line|sed -e 's/fixed-address \([^;]\+\).*/\1/'`
			    ;;
			})
			anzahl=`expr $anzahl "+" 1`
			sm=G
			datei=`echo $ipaddr|tr . _`
			{
			    echo "ipaddr=\"$ipaddr\""
			    echo "mac=\"$mac\""
			    echo "hostname=\"$hostname\""
			    echo "lease=\"$lease\""
			    }>$TMPDIR/$datei
			;;
			*)
			    ;;
		    esac
		    ;;
		G)
		    case $line in
			host\ *)
			    hostname=`echo $line|sed -e 's/host \([^ ]\+\).*/\1/'`
			    ipaddr=
			    mac=
			    lease_time=
			    sm=H
			    ;;
			default-lease-time\ *)
			    default_lease_time=`echo $line|sed -e 's/default-lease-time \([^;]\+\).*/\1/'`
			    ;;
			max-lease-time\ *)
			    max_lease_time=`echo $line|sed -e 's/max-lease-time \([^;]\+\).*/\1/'`
			    ;;
			*)
			    ;;
		    esac
		    ;;
		I)
		    case $line in
			group)
			    sm=G
			    ;;
			*)
			    ;;
		    esac
		    ;;
		*)
		    ;;
	    esac
	done<$datei

	for datei in `ls $TMPDIR`
	do
	    .  $TMPDIR/$datei

            if [ $anzahl -ne 0 ]
            then
                echo "<td rowspan=$anzahl>&nbsp;&nbsp;&nbsp;IP_NET_$netid&nbsp;&nbsp;&nbsp;</td>"
                anzahl=0
            fi
            echo "<td><center>$hostname</center></td><td><center>$ipaddr</center></td><td><center>$mac</center></td><td><center>$default_lease_time / $max_lease_time</center></td></tr>"

	done
	rm -rf $TMPDIR
    done

    echo "</table>"
    show_tab_footer
fi

show_html_footer
