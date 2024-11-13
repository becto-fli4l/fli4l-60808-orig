#!/bin/sh
# Super-simple AT-Daemon

SPOOLDIR='/var/spool/atd'

usage()
{
  echo "Usage:"
  echo "`basename $0` <add|add-daily> <when> <what> [<comment>]"
  echo "<when> is a Unix-Timestamp or a date recognized by 'date'"
  echo "       needs to be in the future."
  echo "       you may also use +<seconds> to define a job in <seconds> in the future"
  echo "       notice that this atd only executes jobs once in an minute!"
  echo "<what> is a shell-commandline"
  echo "<comment> to be able to find a job later - optional"
  echo "with add-daily the job resubmits itselves every day it is executed"
  echo "`basename $0` list"
  echo "    lists jobs in shell-readable way"
  echo "`basename $0` remove <filename>"
  echo "    removes job identified by <filename>"
  exit 1
}

if [ -z "$1" ]
then
  ownname=`basename $0`
  mypid=$$
  if [ -f /var/run/$ownname.pid ]
  then
    otherpid=`cat /var/run/$ownname.pid`
    if [ -z "`ps | sed -n \"/^ $otherpid root/p\"`" ]
    then
      rm /var/run/$ownname.pid
    fi
  fi
  if [ ! -f /var/run/$ownname.pid ]
  then
    echo $mypid >/var/run/$ownname.pid
    while true
    do
      if [ -d $SPOOLDIR ]
      then
        if [ -n "`ls $SPOOLDIR`" ]
	then
	  uts=`date +%s`
	  for file in $SPOOLDIR/*
	  do
	    if [ "`basename $file | sed 's/-.*//'`" -le $uts ]
	    then
	      /bin/sh $file
	      rm -f $file
	    fi
	  done
	fi
	sec=`date +%S`
        # Sleep until next Minute, 5 Seconds
	sleep `expr 65 - $sec`
      else
	mkdir -p $SPOOLDIR
	# Sleep a second to avoid 100% CPU Load if dir-creation doesn't work
	sleep 1
      fi
    done
    rm /var/run/$ownname.pid
  else
    echo `basename $0` is already active.
    usage
  fi
else
  case $1 in
    add|add-daily)
      if [ -z "$3" -o -n "$5" ]
      then
        usage
        exit
      fi
      timestamp=0
      now=`date +%s`
      case $2 in
        +*)
	  timestamp=`echo $2 | sed 's/^+//'`
	  timestamp=`expr $now + $timestamp`
	  ;;
	*)
          test=`expr $2 + 0 2>/dev/null`
	  if [ "$test" != "$2" ]
          then
	    timestamp=`date +%s -d "$2" 2>/dev/null`
	    if [ $timestamp -lt $now ]
	    then
	      timestamp=`expr $timestamp + 86400`
	    fi
          else
	    timestamp=$2
          fi
	  ;;
      esac
      if [ "0$timestamp" -gt $now ]
      then
	lastnum=`ls $SPOOLDIR/${timestamp}-* 2>/dev/null | sed '$!d'`
	if [ -n "$lastnum" ]
	then
	  lastnum=`basename $lastnum | sed '$!d' | sed "s/${timestamp}-//"`
	fi
	newnum=`expr 0$lastnum + 1`
	filename=$SPOOLDIR/${timestamp}-$newnum
	echo "#!/bin/sh" >$filename
	echo "#$4" >>$filename
	echo "#Created: `date`" >>$filename
	echo "#When: $2" >>$filename
	echo "#$1" >>$filename
        echo "$3" >>$filename
	if [ "$1" = "add-daily" ]
	then
	  timestamp=`expr $timestamp + 86400`
	  echo "$0 add-daily $timestamp \"$3\" \"$4\"" >>$filename 
	fi
      else
        if [ "0$timestamp" -gt 0 ]
	then
	  echo Timestamp in the past
	else
          echo Invalid Timestamp
	fi
        usage
      fi
      ;;
    list)
      if [ -n "`ls $SPOOLDIR`" ]
      then
	uts=`date +%s`
	for file in $SPOOLDIR/*
	do
	  filename=`basename $file`
	  timestamp=`echo $filename | sed 's/-.*//'`
	  if [ $timestamp -lt $uts ]
	  then
	    timestamp=$uts
	  fi
	  secondsremain=`expr $timestamp - $uts`
	  comment=`sed -n '2p' <$file | sed 's/^#//'`
	  command=`sed -n '6p' <$file`
	  remainhours=`expr $secondsremain / 3600`
	  remainminutes=`expr $secondsremain % 3600 / 60`
	  remainminutes=`echo 00$remainminutes | sed 's/.*\(..\)$/\1/'`
	  remainseconds=`expr $secondsremain % 60`
	  remainseconds=`echo 00$remainseconds | sed 's/.*\(..\)$/\1/'`
	  echo "timestamp=\"$timestamp\";remainhms=\"$remainhours:$remainminutes:$remainseconds\";remains=\"$secondsremain\";command=\"$command\";comment=\"$comment\";filename=\"$filename\""
	done
      fi
      ;;
    remove)
      if [ -f $SPOOLDIR/$2 ]
      then
        rm -f $SPOOLDIR/$2
      fi
      ;;
    *)
      echo Unknown Command: \""$1"\"
      usage
      ;;
  esac
fi
