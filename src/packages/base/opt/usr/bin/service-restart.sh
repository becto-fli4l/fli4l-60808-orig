#!/bin/sh
#--------------------------------------------------------------------
# /usr/local/bin/service-restart.sh - Run service in a loop
#
# Creation:    08.02.2003 tobig
# Last Update: $Id$
#--------------------------------------------------------------------

SEM=/var/run/service-restart.exit

case $1 in
    start) rm -f $SEM ; exit ;;
    stop)  > $SEM ; exit ;;
esac

if [ "$#" -lt 2 ]
then
  echo "Syntax: $0 <sleeptime>(!<num-attempts>) <programm> [arg1] ..."
  exit 1
fi

if echo "$1" | grep -q "!"
then
  sleeptime="${1%!*}"
  num_attempts="${1#*!}"
  until_success=true
else
  sleeptime="$1"
  until_success=
fi
shift

if echo "$sleeptime" | grep -qv "^[0-9]\+$"
then
  echo "Error: Sleep time is not a number!"
  exit 1
fi

if [ -n "$until_success" ] && echo "$num_attempts" | grep -qv "^[0-9]\+$"
then
  echo "Error: Maximum number of attempts is not a number!"
  exit 1
fi

if [ ! -x "$1" ]
then
  echo "Error: service not found or not executable!"
  exit 1
fi

trap "" 1

remaining_attempts=$num_attempts
while true
do
  if [ -n "$until_success" ]
  then
    if [ $remaining_attempts -eq 0 ]
    then
      logger -t "$0[$$]" "$1 did not start successfully even after $num_attempts, giving up"
      exit
    else
      remaining_attempts=$((remaining_attempts - 1))
    fi
  fi

  "$@"
  rc=$?
  if [ -f $SEM ]; then
    logger -t "$0[$$]" "$1 terminated with $rc, not restarting due to system shutdown ..."
    exit
  fi
  if [ -n "$until_success" -a $rc = 0 ]
  then
    exit
  fi
  # logger -t "$0[$$]" "$1 terminated with $rc, restarting in $sleeptime seconds"
  sleep "$sleeptime"
done
