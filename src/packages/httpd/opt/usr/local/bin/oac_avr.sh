#!/bin/sh
#--------------------------------------------------------------------
# /usr/local/bin/oac_avr.sh - oac-remote-controll from
#				pollin avr-net-io board
#
# Creation:     04.02.2010 rresch
# Last Update:  $Id$
#--------------------------------------------------------------------

# initialize Variables
this=`basename $0`
tmp="/tmp/$this.$$"
configfile="/var/run/oac.cfg"
configfile_groups=/var/run/oac-groups.cfg
groupnum=0
run=0

setport ()
{
  if [ -n "$1" -a -n "$2" ]
  then
    case $firmware in
      pollin) netecho -h $hostname -p 50290 -t "setport $1.$2" >/dev/null;;
      ethersex)
        if [ -n "$userpass" ]
	then
	  netecho -h $hostname -p 2701 -t "!$userpass pin set OUT$1 $2" >/dev/null
	else
	  netecho -h $hostname -p 2701 -t "!pin set OUT$1 $2" >/dev/null
	fi
	;;
    esac
    sleep 1
  fi
}

getport ()
{
  if [ -n "$1" ]
  then
    switch=2
    case $firmware in
      pollin)
	if [ $1 -lt 5 ]
	then
	  switch1=`netecho -h $hostname -p 50290 -t "getport $1"`
	  if [ -n "$switch1" ]
	  then
	    switch=$switch1
	  fi
	else
	  j=`expr $1 - 4`
	  switch1=`netecho -h $hostname -p 50290 -t "getadc $j"`
	  if [ -n "$switch1" ]
	  then 
	    if [ "0$switch1" -gt 511 ]
	    then
	      switch=1
	    fi
	    if [ "0$switch1" -le 511 ]
	    then
	      switch=0
	    fi
	  fi
	fi
	;;
      ethersex)
	if [ -n "$userpass" ]
	then
	  switch1=`netecho -h $hostname -p 2701 -t "!$userpass pin get IN$1"`
	else
	  switch1=`netecho -h $hostname -p 2701 -t "!pin get IN$1"`
	fi
	if [ "$switch1" = "on" ]
	then
	  switch=1
	fi
	if [ "$switch1" = "off" ]
	then
	  switch=0
	fi 
	;;
    esac
    echo $switch
    sleep 1
  fi
}

if [ -f $configfile_groups -a -n "$1" ]
then
  . $configfile_groups
  hostname=$1
  if [ -n "$2" ]
  then
    firmware=$2
    if [ -n "$3" -a -n "$4" ]
    then
      userpass="auth $3 $4"
    else
      userpass=""
    fi
  else
    firmware="pollin"
  fi
  for i in `seq 1 $OAC_GROUP_N`
  do
    eval name=\$OAC_GROUP_${i}_NAME
    if [ "$name" = "avr-net-io" ]
    then
      groupnum=$i
      break
    fi
  done
  if [ $groupnum -eq 0 ]
  then
    sleep 3600
    exit 255
  fi
  eval clients=\$OAC_GROUP_${groupnum}_CLIENT_N
  if [ $clients -gt 8 ]
  then
    clients=8
  fi
  for i in `seq 1 $clients`
  do
    eval client_${i}=\$OAC_GROUP_${groupnum}_CLIENT_${i}
    eval client_${i}_status=-1
    eval client_${i}_fwstat=-1
  done
  while true
  do
    /usr/local/bin/oac.sh status >/tmp/$this$$.status
    for i in `seq 1 $clients`
    do
      run=`expr $run + 1`
      if [ $run -gt 12 ]
      then
        run=0
      fi
      eval client=\$client_${i}
      eval `grep "name=$client IP4" /tmp/$this$$.status`
      # Reset Output-pins every minute or more
      if [ $run -eq $i ]
      then
        laststatus=-1
	lastfwstat=-1
      else
        eval laststatus=\$client_${i}_status
        eval lastfwstat=\$client_${i}_fwstat
      fi
      if [ "$status" != "$laststatus" ]
      then
        eval client_${i}_status=$status
        case "$status" in
          online) setport $i 1;;
          *) setport $i 0;;
        esac
      fi
      case "$fwstat" in
        blocked) fwstat=1 ;;
        *) fwstat=0 ;;
      esac
      if [ "$lastfwstat" != "$fwstat" ]
      then
        eval client_${i}_fwstat=$fwstat
	case $clients in
          1|2|3|4)
	    port=`expr $i + 4`
	    setport $port $fwstat
	    ;;
	esac
      fi
      switch=`getport $i`
      if [ -n "$switch" ]
      then
	if [ "0$fwstat" -ne "0$switch" ]
	then
	  case "$switch" in
	    1*) /usr/local/bin/oac.sh block $client >/dev/null;;
	    0*) /usr/local/bin/oac.sh unblock $client >/dev/null;;
	  esac
	fi
      fi
      sleep 1
    done
    sleep 5
  done
fi
