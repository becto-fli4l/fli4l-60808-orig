#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## conterdown with cron
## Creation:    2007 fs
## Last Update: $Id$
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
# do the arp check "c3surf_retry_check"-times before logout
# quick and dirty for testing -- should be an optional parm in config -- fs 20110220
c3surf_retry_check=5
# ------------------
. /var/run/c3surf.conf
c3surf_reference_curfew=$(/bin/date +%H)
#
# ---------------------------------------------------
# - calc the timer 
# ---------------------------------------------------
calcTime()
{
  fsnow=`date +%s`
  fstime=`/usr/bin/expr $fstime - $fsnow`
}
#
# ---------------------------------------------------
# - check the ARP table and logout if IP is gone
# ---------------------------------------------------
checkArp()
{
  if [ "$C3SURF_CHECK_ARP" = "yes" ]
  then
    if [ -f /proc/net/arp ]
    then
      arp_ip_found="no"
      {
        read arp_header
        # read all fields from arp-table, need arp_flags for online state
        while read arp_ip arp_hwtype arp_flags arp_mac arp_mask arp_device
        do
          if [ "$arp_ip" = "$fsip" ]
          then
            # check arp_flags online state (11/2010 mail fli4l-dev "probleme mit arp")
            if [ "$arp_flags" = "0x2" ]
            then
              arp_ip_found="yes"
              if [ -f "$C3SURF_TMP_PATH/${fsip}.arp" ]
              then
                rm -f "$C3SURF_TMP_PATH/${fsip}.arp"
              fi
            fi
            break
          fi
        done
      } < /proc/net/arp
      # do the logout without counter Quota
      if [ "$arp_ip_found" = "no" ]
      then
        if [ -f "$C3SURF_TMP_PATH/${fsip}.arp" ]
        then
          {
            read c3surf_check_counter
          } < $C3SURF_TMP_PATH/${fsip}.arp
          if [ ${c3surf_check_counter} -lt ${c3surf_retry_check} ]
          then
            c3surf_check_counter=`expr ${c3surf_check_counter} + 1`
            echo "${c3surf_check_counter}" > $C3SURF_TMP_PATH/${fsip}.arp
          else
            /usr/local/bin/c3surf_worker.sh "doArpLogout" "$fsip" "$fsid"
            rm -f "$C3SURF_TMP_PATH/${fsip}.arp"
          fi
        else
          echo "1" > $C3SURF_TMP_PATH/${fsip}.arp
        fi
      fi # no IP found
    fi # check for /proc/net/arp
  fi # ARP-Check
}
# ---------------------------------------------------
# - check curfew and logout if it hits
# ---------------------------------------------------
checkCurfew()
{
  if [ "$C3SURF_CHECK_CURFEW" = "yes" ]
  then
    if [ -n "$fscurfew" ]
    then
      curfew_found="no"
      for i in $fscurfew
      do
        if [ $i -eq $c3surf_reference_curfew ]
        then
          curfew_found="yes"
          break
        fi
      done
      # do the logout without counter Quota
      if [ "$curfew_found" = "yes" ]
      then
        /usr/local/bin/c3surf_worker.sh "doCurfewLogout" "$fsip" "$fsid"
      fi # curfew found
    fi # check for fscurfew
  fi # CURFEW-Check
}
#
# ---------------------------------------------------
# - check the blocked UID/IP and unblock - parallel?
# ---------------------------------------------------
for f in $C3SURF_TMP_PATH/*.block
do
  # schlieﬂe den Abfrage-String aus
  if [ "$f" != "$C3SURF_TMP_PATH/*.block" ]
  then
    {
      read fstime c3surf_rest
    } < $f
    fsnow=`date +%s`
    fstime=`/usr/bin/expr $fstime - $fsnow`
    if [ $fstime -le 0 ]
    then
      rm -f "$f"
    fi
  fi
done
#
# ---------------------------------------------------
# - countdown the time for UID/IP
# ---------------------------------------------------
for f in $C3SURF_TMP_PATH/*.time
do
  # schlieﬂe den Abfrage-String aus
  if [ "$f" != "$C3SURF_TMP_PATH/*.time" ]
  then
    {
      read fstime fscount fsid fsip fsmac fsvorname fsname fsmail fscurfew
    } < $f
    # check (Admin) unlimited login
    if [ $fstime -eq 0 ]
    then
      if [ "$fsid" != "-" ]
      then
        # this is not an admin login, it is an unlimited user login: do arpcheck
        checkArp
      fi
    else
      # check the online time left
      calcTime
      if [ $fstime -le 0 ]
      then
        # this is a real quota limit logout
        /usr/local/bin/c3surf_worker.sh "doLogout" "$fsip" "$fsid"
      else
        # lets check the ARP if configured (soft logout)
        checkArp
      fi # Logout
    fi # unlimited (Admin) login
    # check all for curfew if configured (soft logout)
    checkCurfew
  fi # ignore *.time
done
#