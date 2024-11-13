#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## traffic control with cron
## Creation:    2009 fs
## Last Update: $Id$
## mandatory: all files in /tmp/c3surf /var/run/c3surf
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
. /var/run/c3surf.conf
# testing -----------------------
#DEBUG="+"
#
# : ${OPT_C3SURF_TRAFFIC:=yes}
# End: for testing
# -------------------------------
_tmp=/tmp
_max_bytes=1000000 # limit of bytes
_blocktime=60      # block for 60 minutes
#
# ---------------------------------------------------
# - check the download limit
# ---------------------------------------------------
checkIptablesTraffic()
{
  # save iptabels output
  # upload ist this
  # iptables -vxnL c3surf_control > $_tmp/c3surf_traffic.$$
  # download
  # create traffic file
  /sbin/iptables -vxnL c3surf_traffic | /bin/grep RETURN > $_tmp/c3surf_traffic.$$
  if [ -s $_tmp/c3surf_traffic.$$ ]
  then
    # reset iptables counters
    # iptables -Z c3surf_control
    iptables -Z c3surf_traffic
    {
      # only need this if we do not use grep above
      # read t_headline
      # read t_headline
      # while read t_pkts t_bytes t_target t_prot t_opt t_in t_out t_source t_dest
      while read t_pkts t_bytes t_target t_prot t_opt t_in t_out t_source t_ip
      do
[ $DEBUG ] && echo "$t_ip traffic was $t_bytes bytes"
        if [ $t_bytes -gt $_max_bytes ]
        then
          echo "$t_ip" >> $_tmp/c3surf_limit.new
          if [ -f $_tmp/c3surf_limit.last ]
          then
            t_limit=`/bin/grep $t_ip < $_tmp/c3surf_limit.last`
            if [ -n "$t_limit" ]
            then
              # logout the user
[ $DEBUG ] && echo "logout $t_ip and block for $_blocktime minutes"
              /usr/local/bin/c3surf_worker.sh "doTrafficLogout" "$t_ip" "$_blocktime"
            fi
          fi
        fi
      done
    } < $_tmp/c3surf_traffic.$$
    # do the work with the temp files
    if [ -f $_tmp/c3surf_limit.new ]
    then
      mv -f $_tmp/c3surf_limit.new $_tmp/c3surf_limit.last
    else
      rm -f $_tmp/c3surf_limit.last
    fi
  else
    [ -f $_tmp/c3surf_limit.last ] && rm -f $_tmp/c3surf_limit.last
  fi # test -s $_tmp/c3surf_traffic.$$
  # remove the traffic file
  rm $_tmp/c3surf_traffic.$$
}

# is the OPT_C3SURF_TRAFFIC active ?
if [ "$OPT_C3SURF_TRAFFIC" = "yes" ]
then
  # parm check
  if [ -n "$1" ]
  then
    _max_bytes=$1
  fi
  if [ -n "$2" ]
  then
    _blocktime=$2
  fi

  checkIptablesTraffic
fi
