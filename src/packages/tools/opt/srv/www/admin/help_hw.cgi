#!/bin/sh
#------------------------------------------------------------------------------
# hw.cgi - shows hardware info
#
# Creation:     21.10.2008  jw5
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

get_values ()
{
    id=$1
    dev_type=$2
    man=$3
    device=$4
    revision=''
    case $5 in
      -r*)
        revision=`echo $5 | sed 's/^-r//'`
        case $6 in
	  -p*) 
	   subman=$7
	   subdevice=$8
	   ;;
	  *)
	   subman=$6
	   subdevice=$7
	   ;;
	esac
	;;
      *)
 	 subman=$5
	 subdevice=$6
         ;;
    esac
}
get_driver ()
{
    # we expect $alias with the path to a 'modalias' file in the sysfs
    # the (maybe) loaded driver is returned in $driveractive
    symlink=`echo $alias | sed 's/modalias/driver/'`
    driveractive=`ls -l $symlink | sed 's#.*../##'`
    : ${driveractive:="none"}
}
get_drivers ()
{
    # we expect $alias with the path to a 'modalias' file in the sysfs
    # the maybe matching drivers will be returned in $driverlist in html-format
    templist=':'
    idefound=''
    driverlist=''
    if [ -f $alias ] 
    then
      if [ -n "`grep pci $alias`" ]
      then
        bustype=pci
      else
        if [ -n "`grep usb $alias`" ]
        then
          bustype=usb
        else
          bustype=other
        fi
      fi
      driverlist_=`sed -n -f /usr/share/hw_detect.sed.${bustype} $alias | sed 's/[^ ]\+ //;$!d;s/\(ide_pci\|ata\)_generic//g;s/pata_acpi//'`
      case X$driverlist_
      in
        X) ;;
	*)
	  for a in $driverlist_
	  do
	    b=`echo $a | sed 's/[-_]/[-_]/g'`
	  done
	  for a in $driverlist_
	  do
	    case $templist in
	      *:$a:*) ;;
	      *)
	        templist="$templist:$a:"
	        b=`echo $a | sed 's/[-_]/[-_]/g'`
	        driverinfo=`sed -n "s/^$b //p" /usr/share/hw_driverinfo.txt`
	        isloaded=`grep -i "^$b " /proc/modules`
	        case X$isloaded
	        in
	          X) ;;
	          *) a="<i><b>$a</b></i>" ;;
	        esac
	        driverlist="$driverlist<br>$a"
	        if [ -n "$driverinfo" ]
	        then
	          driverlist="$driverlist&nbsp;($driverinfo)"
	        fi
	      ;;
	    esac
	  done
	  driverlist=`echo $driverlist | sed 's/^<br>//;s/ /\&nbsp;/g'`
	  ;;
      esac
    fi
}


create_hw_info ()
{
    show_html_header "Hardware-Info"
    show_tab_header "PCI Hardware" no
    cat <<EOF
<table class="normtable">
<tr>
  <th>Bus</th>
  <th>Device Type</th>
  <th>Vendor</th>
  <th>Device</th>
  <th>Revision</th>
  <th>SubVendor</th>	
  <th>SubDevice</th>	
  <th>active driver</th>
  <th>possible Drivers</th>	
</tr>
EOF
    lspci -D -m | while read line; do
	eval get_values $line
	alias=/sys/bus/pci/devices/$id/modalias
	get_driver
	get_drivers
	: ${driver:=none}
	cat <<EOF
<tr>
  <td>$id</td>
  <td>$dev_type</td>
  <td>$man</td>
  <td>$device</td>
  <td>$revision</td>
  <td>$subman</td>
  <td>$subdevice</td>
  <td>$driveractive</td>
  <td>$driverlist</td>
</tr>
EOF
    done
    echo '</table>'
    show_tab_footer

    #USB
    if [ -f /usr/share/usb.ids.vendor.sed ]
    then
      found="no"
      for i in /sys/bus/usb/devices/*
      do
        if [ `basename $i` != "*" ]
	then
          found="yes"
	fi
      done
      if [ "$found" = "yes" ]
      then

        echo '<br /><br />'
        show_tab_header "USB Hardware" no
        cat <<EOF
<table class="normtable">
<tr>
  <th>Bus</th>
  <th>Device Class</th>
  <th>Subclass</th>
  <th>Protocol</th>
  <th>Vendor</th>
  <th>Device</th>	
  <th>Speed</th>	
  <th>active driver</th>
  <th>possible Drivers</th>	
</tr>
EOF
        for alias in /sys/bus/usb/devices/*-*/modalias
        do
          bus=`echo $alias | sed 's#/sys/bus/usb/devices/##;s#/modalias##'`
	  vendor=`sed -nf /usr/share/usb.ids.vendor.sed $alias`
	  device=`sed -nf /usr/share/usb.ids.device.sed $alias`
	  class=`sed -nf /usr/share/usb.ids.class.sed $alias`
	  subclass=`sed -nf /usr/share/usb.ids.subclass.sed $alias`
	  protocol=`sed -nf /usr/share/usb.ids.protocol.sed $alias`
	  get_driver
	  get_drivers
	  bus1=`echo $bus | sed 's/:.*//'`
          if [ -f /sys/bus/usb/devices/$bus1/speed ]
	  then 
  	    speed=`cat /sys/bus/usb/devices/$bus1/speed`
	  else
	    usb=`echo /sys/bus/usb/devices/usb*/$bus | sed -n 's/.*\(usb[0-9]\{1,\}\)\/.*/\1\/speed/;1p'`
	    speed=`cat /sys/bus/usb/devices/$usb`
	  fi
	  : ${driver:=none}
	  : ${vendor:=unknown}
	  : ${device:=unknown}
	  : ${class:=unknown}
	  : ${subclass:=unknown}
	  : ${protocol:=unknown}
	  : ${speed:=unknown}
	  cat <<EOF
<tr>
  <td>$bus</td>
  <td>$class</td>
  <td>$subclass</td>
  <td>$protocol</td>
  <td>$vendor</td>
  <td>$device</td>
  <td>$speed</td>
  <td>$driveractive</td>
  <td>$driverlist</td>
</tr>
EOF
        done
        echo '</table>'
        show_tab_footer
      fi
    fi
    
    #PCMCIA

    if [ -n "`ls /sys/bus/pcmcia/devices`" ]
    then
      found="no"
      for i in /sys/bus/pcmcia/devices/*
      do
        if [ `basename $i` != "*" ]
	then
          found="yes"
	fi
      done
      if [ "$found" = "yes" ]
      then
        echo '<br /><br />'
        show_tab_header "PCMCIA Hardware" no
        cat <<EOF
<table class="normtable">
<tr>
  <th>Bus</th>
  <th>ID1</th>
  <th>ID2</th>
  <th>ID3</th>
  <th>ID4</th>
  <th>active driver</th>
  <th>possible Drivers</th>	
</tr>
EOF
        for i in /sys/bus/pcmcia/devices/*
        do
          bus=`basename $i`
	  id1=`cat $i/prod_id1`
	  id2=`cat $i/prod_id2`
	  id3=`cat $i/prod_id3`
	  id4=`cat $i/prod_id4`
	  alias=$i/modalias
	  get_driver
	  get_drivers
	  cat <<EOF
<tr>
  <td>$bus</td>
  <td>$id1</td>
  <td>$id2</td>
  <td>$id3</td>
  <td>$id4</td>
  <td>$driveractive</td>
  <td>$driverlist</td>
</tr>
EOF
        done
        echo '</table>'
        show_tab_footer
      fi
    fi
    
 

    if [ -n "`cat /sys/bus/*/devices/*/modalias | sed '/^\(pci\|pcmcia\|usb\)/d'`" ]
    then

        echo '<br /><br />'
        show_tab_header "Other Hardware" no
        cat <<EOF
<table class="normtable">
<tr>
  <th>Bustype</th>
  <th>modalias</th>
  <th>active driver</th>
  <th>possible Drivers</th>	
</tr>
EOF
        for alias in `find /sys/devices -name modalias | sort`
        do
	  case $alias in
	    /sys/devices/pci*|/sys/devices/usb*|*:01/modalias|*:02/modalias|*:03/modalias|*:04/modalias) ;;
	    *)
	      modalias=`cat $alias | sed 's/-.*//'`
	      case $modalias in
	        pcmcia:*);;
		*)
		    get_driver
		    bus=`echo $alias | sed 's/^\/sys\/devices\///;s/[\/:].*//'`
		    case $bus in
		      LNXSYSTM)
			bus1=`echo $alias | sed 's/^\/sys\/devices\/LNXSYSTM:[0-9]\{1,2\}\///;s/[\/:].*//'`
			case $bus1 in
			  LNXSYBUS)
			    bus2=`echo $alias | sed 's/:[0-9][0-9]\/modalias$//;s/.*\///'`
			    bus="$bus/$bus2"
			    ;;
			  *)
			    bus="$bus/$bus1"
			    ;;
			esac
			;;
		    esac
		    if [ "$modalias" != "acpi:device:" -a "$bus" != "LNXSYSTM/modalias" ]
		    then
		    get_drivers	      
		    cat <<EOF
<tr>
  <td>$bus</td>
  <td>$modalias</td>
  <td>$driveractive</td>
  <td>$driverlist</td>
</tr>
EOF
		    fi
	      ;;
	      esac
	  esac
        done
        echo '</table>'
        show_tab_footer
    fi

    show_html_footer
}
# get main helper functions
. /srv/www/include/cgi-helper

# Security
check_rights 'status' 'view'

hw_info=/var/run/hw_info.html
#rm $hw_info >/dev/null 2>&1
[ ! -f $hw_info ] && create_hw_info > $hw_info
exec cat $hw_info

