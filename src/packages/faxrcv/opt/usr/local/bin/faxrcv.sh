#!/bin/sh
#-----------------------------------------------------------------------------
# /usr/local/bin/faxrcv.sh - start or stop capifaxrcvd            __FLI4LVER__
#
# Creation:     15.04.2004  felix
# Last Update:  $Id$
#-----------------------------------------------------------------------------

CONFIGFILE="/var/run/faxrcv.conf"
THISSCRIPT=`basename "$0"`

## usage [error]*
# display usage message and terminates program
usage () {
  case "$#" in
  0) ;;
  *) echo "$THISSCRIPT: ERROR $@" ;;
  esac

  echo "Usage: $THISSCRIPT start|stop [faxid]"
  echo
  echo "Starts or stops a capifaxrcvd daemon. If faxid is omitted, all instances"
  echo "of capifaxrcvd under control of OPT_FAXRCV will be started/stopped."
  exit 1
}

check_faxid () {
    faxid="$1"

    if [ "0$faxid" -gt "$FAXRCV_N" -o "0$faxid" -lt 1 ]
    then
      usage "unknown faxid ("$faxid")"
      return 1
    fi
}

## start_daemon faxid
# starts capifaxrcvd for corresponding faxid
start_daemon () {
    faxid="$1"

    check_faxid "$faxid" || return

    eval number='$FAXRCV_'$faxid'_NUMBER'
    eval csid='$FAXRCV_'$faxid'_CSID'
    eval directory='$FAXRCV_'$faxid'_DIRECTORY'
    eval ntfycmd='$FAXRCV_'$faxid'_NTFYCMD'
    eval newfax="/usr/local/bin/newfax"$faxid".sh"

    colecho "starting capifaxrcvd (ID: $faxid, MSN: $number) ... " gn

    # tbd: use test syntax, do we need it anyway?
    case "$directory" in
      "") colecho "++ faxrcv FATAL ERROR: no faxdir supplied! ++" br x br; return ;;
    esac

    if [ ! -d "$directory" ]
    then
      echo "creating faxdir ($directory)..."
      mkdir -p $directory
    fi

    case "$csid" in
        "") ;;
        *) entrycsid="-i '$csid' " ;;
    esac

    case "$number" in
        "") ;;
        *) entrynumber="-l $number " ;;
    esac

    # tbd: can we do that a bit better?
    if ps | grep capifaxrcvd | grep -v grep | grep -qe "$entrynumber"
    then
      colecho "capifaxrcvd already running, won't start it twice..." br x br
    else
      capifaxrcvd ${entrycsid}${entrynumber}-n $newfax $directory
    fi
  }

## stop_daemon faxid
# stops capifaxrcvd for corresponding faxid
stop_daemon () {
      faxid="$1"
      faxnumber=`eval echo '$FAXRCV_'$faxid'_NUMBER'`

      check_faxid "$faxid" || return

      # unfortunately busybox cannot do that :(
      # ps --no-heading -C capifaxrcvd -o pid -o args | while read pid args
      ps | grep capifaxrcvd | grep -v grep | while read pid uid vmsize stat command args
      do
        if [ -n "$faxnumber" ]
        then
          # only kill specified capifaxrcvd
          if echo "$args" | grep -qv -- "-l $faxnumber"
          then
            continue
          fi
        fi

        colecho "killing capifaxrcvd (PID: $pid) ..." gn
        kill $pid
        sleep 2
        if ps | grep -qe "^\W*$pid"
        then
          colecho "could not kill capifaxrcvd (you may want to retry by calling this script again)" br x br
        fi
      done
}


if [ "$#" -gt 2 ]
then
  usage "wrong number of arguments"
fi

. $CONFIGFILE

action="$1"
faxid="$2"

case "$action" in
  start)
    if [ -n "$faxid" ]
    then
      start_daemon "$faxid"
    else
      # start all configured daemons
      for idx in `seq 1 $FAXRCV_N`
      do
        start_daemon "$idx"
      done
    fi
  ;;
  stop)
    if [ -n "$faxid" ]
    then
      stop_daemon "$faxid"
    else
      # kill all running instances of capifaxrcvd
      for idx in `seq 1 $FAXRCV_N`
      do
        stop_daemon "$idx"
      done
    fi
  ;;
  help|--help|-h)
    usage
  ;;
  *)
    usage "unknown parameter"
  ;;
esac
