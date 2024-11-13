#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## delete voucher (all or expired)
## optional param: $1="force" : deletes all vouchers
## without param:             : deletes all expired vouchers (cron)
## Creation:    2008 fs
## Last Update: $Id$
## mandatory: all files in /tmp/c3surf /var/run/c3surf
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
# testing only
#DEBUG_INFO="+"

. /var/run/c3surf.conf

v_path="$C3SURF_READ_PATH"

# today
now_ts=$(/bin/date +%s)

[ "$DEBUG_INFO" ] && echo "c3surf_voucher_del starts ..."

# check and delete expired vouchers
for f in "$v_path"/*.data-voucher
  do
  # schließe den Abfrage-String aus
  if [ "$f" != "$v_path/*.data-voucher" ]
  then
    # delete all old values
    unset fsid fspw fsname fsvorname fsmail fssecs fsblocksecs fscounter fstime fsblocktime fsmodule fsvalid
    #unset fsid fspw fsvalid
    # initialize the user vars
    . "$f"
    if [ "$1" = "force" -o -n "$fsvalid" -a $fsvalid != 0 -a $fsvalid -lt $now_ts ]
    then
      # let c3surfWorker.sh do the work
      /usr/local/bin/c3surf_worker.sh "doUserDelete" "" "$fsid" "$fsvorname" "$fsname" "$fsmail"
      [ "$DEBUG_INFO" ] && echo "doUserDelete: - $fsid $fsvorname $fsname $fsmail"
    else
      [ "$DEBUG_INFO" ] && echo "nothing to do"
    fi
  fi
done

[ "$DEBUG_INFO" ] && echo "...done"
