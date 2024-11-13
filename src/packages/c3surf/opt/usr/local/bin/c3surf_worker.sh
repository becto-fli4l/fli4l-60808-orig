#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## central worker
## Creation:    2008 fs
## Last Update: $Id$
##------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------

# #############################################################
# Zentrales c3Surf Modul.....
# #############################################################
# habe dies Design gewählt, weil jedes Modul wieder
# die c3Surf Variablen initialisieren musste.
# Es wird viel Global benötigt. -- nix für Softwareengineers
# #############################################################
# params:
# $1 : Command
# $2 : IP-Address (REMOTE_ADDR)
# $3 : ID: UserID (LoginUsr), ProcessID (3Surf)
# $4 : forename (3Surf)
# $5 : surename (3Surf)
# $6 : email (3Surf)
# $7 : mac address (optional)  <--- OBSOLETE 08.2008
# #############################################################
# FUNCTIONS
# #############################################################
# Testing Vars
# write some output to /tmp ("+"), to be silent comment it out
#DEBUG_INFO="+"
#TEST_VERSION="+"
#[ $TEST_VERSION ] && OPT_C3SURF_VOUCHER="yes"

# +++++++++++++ NEW STUFF


# removes the usr:pwd string from the .htpasswd
# used by doUserDelete and removeVoucher
removeHtPasswd()
{
  # take user away from .htpasswd (password dosn't matter)
  # sed deletes sometimes the hole file - do not know why
  # /bin/sed "/$fsid:$fspw/d" /srv/www/c3surf/login/.htpasswd > /srv/www/c3surf/login/.htpasswd.$$
  # grep -v show all line exept the seach string
  /bin/grep -v "${fsid}:" /srv/www/c3surf/login/.htpasswd > /srv/www/c3surf/login/.htpasswd.$$
  # /bin/rm -f /srv/www/c3surf/login/.htpasswd
  /bin/mv -f /srv/www/c3surf/login/.htpasswd.$$ /srv/www/c3surf/login/.htpasswd
}


# #################################################
# voucher
# #################################################
# ---- Voucher addon (begin) -----
# removes uid string from the voucher-printfile
# used by activateVoucher
removePrintVoucher()
{
  if [ "$C3SURF_VOUCHER_PRTUPDATE" = "yes" -a -s "$C3SURF_VOUCHER_PRINTFILE" ]
  then
    # grep -v show all line exept the seach string
    /bin/grep -v "$c3surf_id," "$C3SURF_VOUCHER_PRINTFILE" > /tmp/c3surf_tmp_voucherprint.$$
    # /bin/rm -f "$C3SURF_VOUCHER_PRINTFILE"
    /bin/mv -f /tmp/c3surf_tmp_voucherprint.$$ "$C3SURF_VOUCHER_PRINTFILE"
  fi
}

# activateVoucher
# used by initUserVar
activateVoucher()
{
  if [ "$OPT_C3SURF_VOUCHER" = "yes" ]
  then
    # READ_PATH is for C3SURF_WORKON_TMP
    voucher_un="$C3SURF_READ_PATH/$c3surf_id.data-voucher"
    # remember the persistent name, if needed
    if [ "$C3SURF_WORKON_TMP" = "yes" ]
    then
      voucher_persistent_un="$C3SURF_PERSISTENT_PATH/$c3surf_id.data-voucher"
    fi

    if [ ! -f "$c3surf_un" ]
    then
      if [ -f "$voucher_un" ] 
      then
        /bin/mv -f "$voucher_un" "$c3surf_un"
        if [ "$C3SURF_WORKON_TMP" = "yes" ]
        then
          [ -f "$voucher_persistent_un" ] && /bin/mv -f "$voucher_persistent_un" "$c3surf_persistent_un"
        fi
        # remove voucher from printlist
        removePrintVoucher
      fi
    fi
  fi
}

# deactivateVoucher
# used by dologin (on error) and dologout (on c3surf_quota_block="none")
deactivateVoucher()
{
  if [ "$fsmodule" = "voucher" ]
  then
    [ -f "$c3surf_un" ] && /bin/mv -f "$c3surf_un" "$voucher_un"
    if [ "$C3SURF_WORKON_TMP" = "yes" ]
    then
      [ -f "$c3surf_persistent_un" ] && /bin/mv -f "$c3surf_persistent_un" "$voucher_persistent_un"
    fi
  fi
}

# checks if this is a voucher login / logout
# voucher is converted to a c3surf LOGINUSR
# used by doLogin, doLogout
# and enables QUOTA if the livetime is not "0"
checkVoucher()
{
  if [ "$fsmodule" = "voucher" ]
  then
    log_msg="${log_msg}:voucher"
    # check the livetime
    if [ $fslives -eq 0 ]
    then
      C3SURF_QUOTA="voucher"
    else
      C3SURF_QUOTA="yes"
    fi
    # check the voucher valid date
    if [ $c3surf_nstatus -eq 0 ]
    then
      if [ $fsvalid -gt 0 -a $c3surf_reference_ts -gt $fsvalid ]
      then
        # voucher is outdated
        c3surf_sstatus="$c3surf_login_msg_voucheroff"
        c3surf_hstatus="_c3surf_login_msg_voucheroff"
        c3surf_nstatus=7
        c3surf_cstatus="voucher_outdated:"
      fi
    fi
    # calc a new voucher valid date
    if [ $c3surf_nstatus -eq 0 ]
    then
      if [ $fslives -gt 0 ]
      then
        # voucher is used the first time, calc a new date
        fslives_secs=$(/usr/bin/expr $fslives \* 3600)
        fsvalid=$(/usr/bin/expr $c3surf_reference_ts + $fslives_secs)
        fslives="-1"
        # change the data now
        /bin/grep -v "fsvalid=" "$c3surf_un" > /tmp/voucher.$$
        /bin/grep -v "fslives=" /tmp/voucher.$$ > "$c3surf_un"
        /bin/rm -f /tmp/voucher.$$
        echo "fsvalid=\"$fsvalid\"" >> "$c3surf_un"
        echo "fslives=\"$fslives\"" >> "$c3surf_un"
        if [ "$C3SURF_WORKON_TMP" = "yes" ]
        then
          /bin/cp -f "$c3surf_un" "$c3surf_persistent_un"
        fi
      fi
    fi
  fi
}

# write the .data file to .data-voucher
# the persistent path is used for writing!
# used by writeUserVar
writeVoucher()
{
  if [ "$fsmodule" = "voucher" ]
  then
    mv -f "$c3surf_un" "$voucher_un"
    # put the data to TMP if necessary
    if [ "$C3SURF_WORKON_TMP" = "yes" ]
    then
      mv -f "$c3surf_persistent_un" "$voucher_persistent_un"
    fi
  fi
}

# removes the voucher on logout or quota limit
# used by Logout (voucher is a LOGINUSR here)
removeVoucher()
{
  if [ "$fsmodule" = "voucher" ]
  then
    # remove user .data
    rm -f "$c3surf_un"
    if [ "$C3SURF_WORKON_TMP" = "yes" ]
    then
      [ -f "$c3surf_persistent_un" ] && /bin/rm -f "$c3surf_persistent_un"
    fi
    # take user away from .htpasswd
    removeHtPasswd
  fi
}

# ---- Voucher addon (end) -----

# ------------ NEW STUFF


# #################################################
# log
# #################################################

# log valid Logins
logValid()
{
  if [ "$C3SURF_DOLOG_LOGIN" = "yes" ]
  then
    if [ -n "$1" ]
    then
      echo "$c3surf_log_ts|$c3surf_ip|$c3surf_id|$c3surf_vorname|$c3surf_name|$c3surf_mail|$c3surf_mac|$1" >> "$C3SURF_LOG_PATH/c3surf_login.log"
    else
      echo "$c3surf_log_ts|$c3surf_ip|$c3surf_id|$c3surf_vorname|$c3surf_name|$c3surf_mail|$c3surf_mac|unknown" >> "$C3SURF_LOG_PATH/c3surf_login.log"
    fi
  fi
}

# log invalid logins
logInvalid()
{
  if [ "$C3SURF_DOLOG_INVALID" = "yes" ]
  then
    echo "$c3surf_log_ts|$c3surf_mac|$c3surf_cstatus $c3surf_sstatus|$c3surf_ip|$c3surf_id $c3surf_vorname $c3surf_name $c3surf_mail" >> "$C3SURF_LOG_PATH/c3surf_invalid.log"
  fi
}

# log worker events
logWorker()
{
  echo "$c3surf_log_ts|$c3surf_cmd|$c3surf_ip|$c3surf_id|$c3surf_vorname|$c3surf_name|$c3surf_mail|$c3surf_mac|$c3surf_error" >> "$C3SURF_LOG_PATH/c3surf_worker.log"
}

# #################################################
# initVars
# #################################################

# name: user data file
initUserVar()
{
  # READ_PATH is for C3SURF_WORKON_TMP
  c3surf_un="$C3SURF_READ_PATH/$c3surf_id.data"
  # remember the persistent name, if needed
  if [ "$C3SURF_WORKON_TMP" = "yes" ]
  then
    c3surf_persistent_un="$C3SURF_PERSISTENT_PATH/$c3surf_id.data"
  fi

  # allow only one login
  c3surf_on="$C3SURF_TMP_PATH/$c3surf_id.online"
  # ---- Voucher addon (begin) -----
  # if this is not an admin unlimited login, then activate an voucher
  if [ "$c3surf_id" != "-" ]
  then
    activateVoucher
  fi
  # ---- Voucher addon (end) -----
}

# read the user.data file
readUserVar()
{
  if [ -f "$c3surf_un" ]
  then
    . "$c3surf_un"
    # only doAdminUser may change this values
    # ---------- test this a while ----------
    if [ "$c3surf_cmd" != "doAdminUser" ]
    then
      [ -n "$c3surf_id" ] || c3surf_id="$fsid"
      [ -n "$c3surf_vorname" ] || c3surf_vorname="$fsvorname"
      [ -n "$c3surf_name" ] || c3surf_name="$fsname"
      [ -n "$c3surf_mail" ] || c3surf_mail="$fsmail"
      [ -n "$c3surf_curfew" ] || c3surf_curfew="$fscurfew"
    fi
  fi
}

# write the user.data file
# reihenfolge wichtig: 1. calcUserVar() -> cryptUserPwd() -> writeUserVar()
# needs a correct cpypted password from setUserPwd()
writeUserVar()
{
  if [ -n "$c3surf_id" ]
  then
    echo "fsid=\"$fsid\""  > "$c3surf_un"
    echo "fspw=\"$fspw\"" >> "$c3surf_un"
    echo "fsname=\"$fsname\"" >> "$c3surf_un"
    echo "fsvorname=\"$fsvorname\"" >> "$c3surf_un"
    echo "fsmail=\"$fsmail\"" >> "$c3surf_un"
    if [ -n "$fstime" ]
    then
      echo "fstime=\"$fstime\"" >> "$c3surf_un"
      echo "fssecs=\"$fssecs\"" >> "$c3surf_un"
    fi
    if [ -n "$fsblocktime" ]
    then
      echo "fsblocktime=\"$fsblocktime\"" >> "$c3surf_un"
      echo "fsblocksecs=\"$fsblocksecs\"" >> "$c3surf_un"
    fi
    if [ -n "$fscounter" ]
    then
      echo "fscounter=\"$fscounter\"" >> "$c3surf_un"
    fi
    if [ -n "$fscurfew" ]
    then
      echo "fscurfew=\"$fscurfew\"" >> "$c3surf_un"
    fi
    echo "fscreated=\"$c3surf_log_ts\"" >> "$c3surf_un"
    # ---- Voucher addon (begin) -----
    # if [ "OPT_C3SURF_VOUCHER" = "yes" ]
    if [ -n "$fsmodule" ]
    then
      if [ "$fsmodule" = "voucher" ]
      then
        echo "fsmodule=\"$fsmodule\"" >> "$c3surf_un"
        if [ -n "$fsvalid" ]
        then
          echo "fsvalid=\"$fsvalid\"" >> "$c3surf_un"
        else
          # only valid thru 0:00 next day
          echo "fsvalid=\"$c3surf_tomorrow_ts\"" >> "$c3surf_un"
        fi
        echo "fslives=\"$fslives\"" >> "$c3surf_un"
      fi
    fi
    # ---- Voucher addon (end) -----

    # put the user.data to TMP
    if [ "$C3SURF_WORKON_TMP" = "yes" ]
    then
      cp -f "$c3surf_un" "$c3surf_persistent_un"
    fi

    # ---- Voucher addon (begin) -----
    # check and finish voucher writing
    writeVoucher
    # ---- Voucher addon (end) -----

  else
    c3surf_error="writeUserVar: cannot write user.data file, c3surf_id is empty"
    logWorker
  fi
}

# write the userid and password to .htpasswd file
#  --- uses only the internal fsid and fspw !!!
writeUserPwd()
{
  if [ -n "$fsid" -a -n "$fspw" ]
  then
    removeHtPasswd
    echo "${fsid}:${fspw}" >> /srv/www/c3surf/login/.htpasswd
  fi
}

# crypt password 
# use before writeUserVar() and writeUserPwd()
# do nothing if c3surf_pw is empty - c3surf_pw comes uncrypted
#  trick: do not change pw if an existing user is edited without pw-change
# returns 0 = true if password was changed = crypted
cryptUserPwd()
{
  if [ -n "$c3surf_pw" ]
  then
    # fspw=$(htpasswd -nbd "$c3surf_id" "$c3surf_pw" | /usr/bin/cut -d ":" -f 2)
    echo "$c3surf_pw" | htpasswd -c /tmp/c3ht.$$ "$c3surf_id"
    fspw=$(/usr/bin/cut -d ":" -f 2 /tmp/c3ht.$$)
    rm -f /tmp/c3ht.$$
    return 0
  else
    return 1
  fi
}

# calc the user vars (not the pw!)
# use before writeUserVar()
# dont forget cryptUserPwd() it sets the user pwd
calcUserVar()
{

  # trick - use this for add and/or update
  # if there was no user.data file set the fsid for a new user
  [ -n "$fsid" ] || fsid="$c3surf_id"

  # now start checking the parms
  if [ -n "$c3surf_name" ]
  then
    fsname=$(echo "$c3surf_name" | tr -d '\n' | tr -c 'äüöÄÜÖß\-[:alnum:]' '_')
  else
    fsname="none"
  fi

  if [ -n "$c3surf_vorname" ]
  then
    fsvorname=$(echo "$c3surf_vorname" | tr -d '\n' | tr -c 'äüöÄÜÖß\-[:alnum:]' '_')
  else
    fsvorname="none"
  fi

  if [ -n "$c3surf_mail" ]
  then
    fsmail=$(echo "$c3surf_mail" | tr -d '\n' | tr -c '@äüöÄÜÖß\-.[:alnum:]' '_')
  else
    fsmail="none"
  fi
  # optional time
  if [ -n "$c3surf_time" ]
  then
    fstime=$(echo "$c3surf_time" | tr -d '\n' | tr -c '[:digit:]' 'X' | tr -d 'X')
    if [ -n "$fstime" ]
    then
      fssecs=`expr $fstime \* 60`
    fi
  else
    fstime=""
  fi
  # optional blocktime
  if [ -n "$c3surf_blocktime" ]
  then
    fsblocktime=$(echo "$c3surf_blocktime" | tr -d '\n' | tr -c '\-[:digit:]' 'X' | tr -d 'X')
    if [ -n "$fsblocktime" ]
    then
      if [ $fsblocktime -le 0 ]
      then
        fsblocksecs=$fsblocktime
      else
        fsblocksecs=`expr $fsblocktime \* 60`
      fi
    fi
  else
    fsblocktime=""
  fi
  # optional counter
  if [ -n "$c3surf_counter" ]
  then
    fscounter=$(echo "$c3surf_counter" | tr -d '\n' | tr -c '\-[:digit:]' 'X' | tr -d 'X')
    if [ -n "$fscounter" ]
    then
      if [ $fscounter -lt -1 ]
      then
        fscounter=-2
      fi
    fi
  else
    fscounter=""
  fi
  # optional curfew
  if [ -n "$c3surf_curfew" ]
  then
    fscurfew=$(echo "$c3surf_curfew" | tr -d '\n' | tr -c '\ [:digit:]' 'X' | tr -d 'X')
  else
    fscurfew=""
  fi
  # ---- Voucher addon (begin) -----
  # optional module
  if [ -n "$c3surf_module" ]
  then
    fsmodule="$c3surf_module"
    fsvalid="$c3surf_valid"
    fslives="$c3surf_lives"
  else
    fsmodule=""
    fsvalid=""
    fslives=""
  fi
  # ---- Voucher addon (end) -----
}

# set the user defaults to global
setUserDefaults()
{
  if [ "$OPT_LOGINUSR" = "yes" ]
  then
    # C3SURF_TIME
    if [ -n "$fssecs" ]
    then
      C3SURF_SECS=$fssecs
    fi
    # C3SURF_BLOCKSECS
    if [ -n "$fsblocksecs" ]
    then
      C3SURF_BLOCKSECS=$fsblocksecs
    fi
    # C3SURF_COUNTER
    if [ -n "$fscounter" ]
    then
      C3SURF_COUNTER=$fscounter
    fi
  fi
}

# #################################################
# html
# #################################################
# feed the status-frame of the html page
# allways use the IP-address here
printHtmlStatus()
{
  echo "$c3surf_hstatus" > "$C3SURF_TMP_PATH/$c3surf_ip.status"
# [ -n "$DEBUG_INFO" ] && echo "DEBUG $c3surf_id : $c3surf_hstatus" > "$C3SURF_TMP_PATH/$c3surf_ip.status"
}

# #################################################
# mac
# #################################################

# get mac for the IP from ARP
# sets: c3surf_mac
getMacFromArp()
{
  {
    read arp_header
    while read arp_ip arp_hwtype arp_flags arp_mac arp_mask arp_device
    do
      if [ -n "$tmp_arp_mac" ]; then break; fi
      if [ "$arp_ip" = "$c3surf_ip" ]
      then
        tmp_arp_mac="$arp_mac"
      fi
    done
  } < /proc/net/arp
  # no capital letters
  c3surf_mac="$(echo "$tmp_arp_mac" | sed -e 'y/ABCDEF/abcdef/')"
  # etwas aufräumen
  unset arp_header arp_ip arp_hwtype arp_flags arp_mac arp_mask arp_device tmp_arp_mac
} # getMacFromArp


# #################################################
# TimeBlock (Quota, Admin)
# block IP or ID for C3SURF_BLOCKTIME minutes
# #################################################
# fileformat: c3surf_block_time c3surf_mac c3surf_id $c3surf_ip
# optional parameter: block_secs
initTimeBlockVar()
{
  if [ $# -gt 0 ]
  then
    c3surf_time_block=$1
  else
    c3surf_time_block=$C3SURF_BLOCKSECS
  fi
  if [ "$OPT_LOGINUSR" = "yes" ]
  then
    c3surf_bn="$C3SURF_TMP_PATH/$c3surf_id.block"
  else
    c3surf_bn="$C3SURF_TMP_PATH/$c3surf_ip.block"
  fi
}

addTimeBlock()
{
  if [ $c3surf_time_block -ge 0 ]
  then
    if [ $c3surf_time_block -eq 0 ]
    then
      # block until tomorrow "00:00"
      # this blocks until next day on logout, better take the quota timestamp.
      # it is the timestamp of the first login.
      # c3surf_block_time=$c3surf_tomorrow_ts
      if [ -n "$c3surf_quota_ts" ]
      then
        c3surf_block_time=$c3surf_quota_ts
      else
        c3surf_block_time=$c3surf_tomorrow_ts
      fi
    else
      # C3SURF_COUNTER -eq -2 : use the quota timestamp -- this is for long time quota
      if [ $C3SURF_COUNTER -gt -2 ]
      then
        # c3surf_block_time=$(/usr/bin/expr $c3surf_reference_ts + $C3SURF_BLOCKSECS)
        c3surf_block_time=$(/usr/bin/expr $c3surf_reference_ts + $c3surf_time_block)
      else
        c3surf_block_time=$c3surf_quota_ts
      fi
    fi
    # !! write TimeBlock
    if [ $c3surf_block_time -gt $c3surf_reference_ts ]
    then
      echo "$c3surf_block_time" "$c3surf_mac" "$c3surf_id" "$c3surf_ip" > "$c3surf_bn"
    fi
  fi
} # addTimeBlock

removeTimeBlock()
{
  rm -f "$c3surf_bn"
} # removeTimeBlock


# #################################################
# setIPFilter
# #################################################
openIPFilter()
{
  /sbin/iptables -t nat -I c3surf_nat_control 1 -s $c3surf_ip -j RETURN
  /sbin/iptables -I c3surf_control 1 -s $c3surf_ip -j RETURN
  if [ $OPT_C3SURF_TRAFFIC = 'yes' ]
  then
    /sbin/iptables -I c3surf_traffic 1 -d $c3surf_ip -j RETURN
  fi
}

closeIPFilter()
{
  if [ $OPT_C3SURF_TRAFFIC = 'yes' ]
  then
    /sbin/iptables -D c3surf_traffic -d $c3surf_ip -j RETURN
  fi
  /sbin/iptables -D c3surf_control -s $c3surf_ip -j RETURN
  /sbin/iptables -t nat -D c3surf_nat_control -s $c3surf_ip -j RETURN
}

# #################################################
# read/write-User is online
# #################################################

writeUserOnline()
{
  echo "$c3surf_id" "$c3surf_ip" "$c3surf_mac" > "$c3surf_on"
}

readUserOnline()
{
  {
    read "c3surf_online_id" c3surf_online_ip c3surf_online_mac
  } < "$c3surf_on"
}

removeUserOnline()
{
  rm -f "$c3surf_on"
}

# #################################################
# read/write-Timer
# setzt den Timer für die IP oder ID
# #################################################
#
# fileformat: c3surf_time c3surf_count c3surf_id c3surf_ip c3surf_mac c3surf_vorname c3surf_name c3surf_mail c3surf_curfew

# set filename for timer: always subst user to IP
initTimerVar()
{
  c3surf_tn="$C3SURF_TMP_PATH/$c3surf_ip.time"
}

writeTimer()
{
  echo "$c3surf_time" "$c3surf_count" "$c3surf_id" "$c3surf_ip" "$c3surf_mac" "$c3surf_vorname" "$c3surf_name" "$c3surf_mail" "$c3surf_curfew" > "$c3surf_tn"
}

readTimer()
{
  {
    read c3surf_time c3surf_count c3surf_id c3surf_ip c3surf_mac c3surf_vorname c3surf_name c3surf_mail c3surf_curfew
  } < "$c3surf_tn"
}

removeTimer()
{
  rm -f "$c3surf_tn"
}

# used by login
setTimer()
{
if [ -f $c3surf_tn ]
then
  readTimer
  c3surf_count=$(/usr/bin/expr $c3surf_count + 1)
  # nur Zähler erhöhen, nicht Zeit verlängern
  # fstime=`date +%s`
  # c3surf_time=`expr $fstime + $C3SURF_SECS`
  # /usr/local/bin/c3surf_log_login.sh "$c3surf_ip" "$c3surf_vorname" "$c3surf_name" "$c3surf_mail" "$c3surf_mac" "prolong:$c3surf_count"
else
  # es ist die erste Anmeldung
  c3surf_count="0"
  if [ "$c3surf_id" = "-" ]
  then
    # the c3surf_tn is initialised with IP-address
    c3surf_time="0"
    if [ -z "$c3surf_mac" ]
    then
      c3surf_mac="-"
    fi
  else
    if [ $C3SURF_SECS -eq 0 ]
    then
      # unlimited login for C3SURF_SECS=0
      c3surf_time=0
    else
      # limit login to calculated time
      c3surf_time=$(/usr/bin/expr $c3surf_reference_ts + $C3SURF_SECS)
    fi
  fi
  openIPFilter
fi
writeTimer
} # setTimer

# =====
# QUOTA
# =====
# set quota PATH
initQuotaVar()
{
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    # check for unlimited login by admin
    if [ "$c3surf_id" = "-" ]
    then
      C3SURF_QUOTA="admin"
      c3surf_qn=""
    else
      if [ "$OPT_LOGINUSR" = "yes" ]
      then
        # ID: LoginUsr
        c3surf_qn="$C3SURF_TMP_PATH/$c3surf_id.quota"
      else
        # IP: FreeSurf
        c3surf_qn="$C3SURF_TMP_PATH/$c3surf_ip.quota"
      fi
    fi
  fi
  c3surf_quota_block="none"
}

calcQuota()
{
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    # tmpnow=$(/bin/date +%s) <-- better use $c3surf_reference_ts
    if [ ! $c3surf_quota_maxtime -eq 0 ]
    then
      c3surf_quota_time=$(/usr/bin/expr $c3surf_time - $c3surf_reference_ts)
    fi
    # special: counter<0 do not recognize the counter 
# calc counter for maxcount < 0
#    if [ $c3surf_quota_maxcount -lt 0 ]
#    then
#      c3surf_quota_count=$c3surf_quota_maxcount
#    else
      # for doAutoLogout do not increase count
      if [ "$c3surf_update_counter"="yes" ]
      then
        c3surf_quota_count=$(/usr/bin/expr $c3surf_quota_count + 1)
      fi
#    fi
  fi
}

writeQuota()
{
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    echo "$c3surf_quota_ts $c3surf_quota_time $c3surf_quota_maxtime $c3surf_quota_count $c3surf_quota_maxcount" > "$c3surf_qn"
  fi
}

# used by doLogout
removeQuota()
{
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    rm -f "$c3surf_qn"
  fi
}

# calc a quota timestamp
calcQuotaTimestamp()
{
  if [ $C3SURF_BLOCKSECS -ge 0 ]
  then
    if [ $C3SURF_BLOCKSECS -eq 0 ]
    then
      # until tomorrow "00:00"
      c3surf_quota_ts=$c3surf_tomorrow_ts
    else
      # can use long term quota
      if [ $C3SURF_COUNTER -gt -2 ]
      then
        # until block and surf time
        # c3surf_quota_ts=$(/usr/bin/expr $c3surf_reference_ts + $C3SURF_BLOCKSECS + $C3SURF_SECS)
        c3surf_quota_ts=0
      else
        # until block
        c3surf_quota_ts=$(/usr/bin/expr $c3surf_reference_ts + $C3SURF_BLOCKSECS)
      fi
    fi
  else
    # c3surf_quota_ts=$c3surf_reference_ts
    c3surf_quota_ts=0
  fi
}

# read the quotas and set defaults
# and check the quota timestamp
readQuota()
{
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    if [ -f "$c3surf_qn" ]
    then
      {
        read c3surf_quota_ts c3surf_quota_time c3surf_quota_maxtime c3surf_quota_count c3surf_quota_maxcount
      } < "$c3surf_qn"
      # check for valid timestamp
      if [ $c3surf_quota_ts -gt 0 -a $c3surf_reference_ts -gt $c3surf_quota_ts ]
      then
        # quota expired
        calcQuotaTimestamp
        c3surf_quota_maxtime=$C3SURF_SECS
        c3surf_quota_maxcount=$C3SURF_COUNTER
        c3surf_quota_count=0
        c3surf_quota_time=$c3surf_quota_maxtime
        # update expired data
        writeQuota
      else
        # adjust the global login time to quota
        # do not allow 0 secs here - it is unlimited login
        if [ $c3surf_quota_maxtime -ne 0 -a $c3surf_quota_time -eq 0 ]
        then 
          C3SURF_SECS=1
        else
          C3SURF_SECS=$c3surf_quota_time
        fi
      fi
    else
      calcQuotaTimestamp
      c3surf_quota_maxtime=$C3SURF_SECS
      c3surf_quota_maxcount=$C3SURF_COUNTER
      c3surf_quota_count=0
      c3surf_quota_time=$c3surf_quota_maxtime
      # save the quota (preserve timestamp)
      writeQuota
    fi
  fi
}

# checks Quota and makes a block
checkQuota()
{
  initQuotaVar
  if [ "$C3SURF_QUOTA" = "yes" ]
  then
    # read the user.data
    readUserVar
# [ -n "$DEBUG_INFO" ] && set > /tmp/readuser.var
    # set the user specific values
    setUserDefaults
# [ -n "$DEBUG_INFO" ] && set > /tmp/userdefaults.var
    readQuota
    calcQuota
    writeQuota
# [ -n "$DEBUG_INFO" ] && set > /tmp/userquota.var
    if [ $c3surf_quota_maxtime -gt 0 -a $c3surf_quota_time -le 0 ]
    then
      # quota block
      c3surf_quota_block="time"
    else
      if [ $c3surf_quota_maxcount -ge 0 -a $c3surf_quota_count -gt $c3surf_quota_maxcount ]
      then
        c3surf_quota_block="count"
      else
        c3surf_quota_block="none"
      fi
    fi
  fi
}

# ======
# LOGOUT
# ======
###################################################
# 3surf: logout this IP
# parameter: $1 = IP-address, $2 optional = action (counter, logout)
#
# Frank Saurbier, 2007, 3surf@arcor.de
#
# mandatory: all files in /tmp/c3surf /var/run/c3surf
###################################################
# logout the user
# optional Parameter: $1 log message default: "logout"
doLogout()
{
  if [ -f "$c3surf_tn" ]
  then
    if [ $# -gt 0 ]
    then
      log_msg="$1"
    else
      log_msg="logout"
    fi

    # read the Timer Data
    readTimer

    if [ "$OPT_LOGINUSR" = "yes" ]
    then
      # set the Variables for user-files
      initUserVar
      # remove online flag
      removeUserOnline
    fi

    # ---- Voucher addon (begin) -----
    checkVoucher
    # ---- Voucher addon (end) -----

    checkQuota
    if [ "$c3surf_quota_block" = "none" ]
    then
      # log the logout
      logValid "$log_msg"
      # ---- Voucher addon (begin) -----
      if [ -n "$fslives" ]
      then
        if [ $fslives -eq 0 ]
        then
          removeVoucher
        else
          deactivateVoucher
        fi
      fi
      # ---- Voucher addon (end) -----
    else
      initTimeBlockVar
      addTimeBlock
      logValid "$log_msg:quota:$c3surf_quota_block"
      removeQuota
      # ---- Voucher addon (begin) -----
      removeVoucher
      # ---- Voucher addon (end) -----
    fi

    removeTimer
  fi

  # always close the iptable rule
  closeIPFilter
}

# logout and caltulate time, but do not increase the quota counter
doAutoLogout()
{
  c3surf_update_counter="no"
  doLogout "$1"
}

# logout without Quota-Check or Calculation
doAdminLogout()
{
  C3SURF_QUOTA="admin"
  doLogout "$1"
}


# =====
# LOGIN
# =====
# ##################################################
# 3surf: check the post-method and login if everything is ok
# parameter: 1.IP-address, 2.id, 3.vorname, 4.name, 5.mail
# /usr/local/bin/c3surf_login.sh "$REMOTE_ADDR" "$fsid" "$fsvorname" "$fsname" "$fsmail"
# Frank Saurbier, 2008, 3surf@arcor.de
#
# mandatory: all files in /tmp/c3surf /var/run/c3surf
# nicht sourcen: endet mit exit code
# ##################################################
# c3surf_nstatus > 0 Status for html
# c3surf_nstatus = 0 Login erlaubt
# c3surf_nstatus < 0 no Login and no Status for html
checkLogin()
{
  if [ "$OPT_LOGINUSR" = "yes" ]
  then
    # check Admin login
    initUserVar
    if [ -f "$c3surf_un" ]
    then
      if [ -f "$c3surf_on" ]
      then
        c3surf_sstatus="$c3surf_login_msg_multilogin"
        c3surf_hstatus="_c3surf_login_msg_multilogin"
        c3surf_nstatus=7
        c3surf_cstatus="multi_login:"
      else
      readUserVar
      fi
    else
      c3surf_sstatus="$c3surf_login_msg_nouser"
      c3surf_hstatus="_c3surf_login_msg_nouser"
      c3surf_nstatus=7
      c3surf_cstatus="unknown_user:"
    fi
  else
    # check all params
    if [ -z "$c3surf_vorname" -o -z "$c3surf_name" -o -z "$c3surf_mail" ]
    then
      # missing params = no login
      c3surf_sstatus="$c3surf_login_msg_noparm"
      c3surf_hstatus="_c3surf_login_msg_noparm"
      c3surf_nstatus=7
      c3surf_cstatus="param_error:"
    else
      # test for double post
      if [ -f "$c3surf_tn" ]
      then
        # check the id to identify a double POST from html
        {
          read fs_time fs_count fs_id fs_rest
        } < "$c3surf_tn"
        if [ "$fs_id" = "$c3surf_id" ]
        then
          # this was a double POST -- silent Error
          # silent errors are less than 0
          c3surf_nstatus=-1
          c3surf_cstatus="double_post:"
        fi # id is the same = same process calls us
      fi # file for IP exists
    fi # check login and double post
  fi # LoginUsr

  # go on with some general tests

  # system locked
  if [ $c3surf_nstatus -eq 0 ]
  then
    # System is locked
    if [ -f $C3SURF_SYSLOCK_FILE ]
    then
      # system lock: no login
      c3surf_sstatus="$c3surf_login_msg_syslock"
      c3surf_hstatus="_c3surf_login_msg_syslock"
      c3surf_nstatus=7
      c3surf_cstatus="system_lock:"
    fi
  fi

  # mac blacklist
  if [ $c3surf_nstatus -eq 0 ]
  then
    # check the c3surf MAC-blocklist
    if [ -z "$c3surf_mac" ]
    then
      if [ ! "$C3SURF_SLOPPY_MAC" = "yes" ]
      then
        # fatal error, we have no mac-address
        c3surf_sstatus="$c3surf_login_msg_nomac"
        c3surf_hstatus="_c3surf_login_msg_nomac"
        c3surf_nstatus=7
        c3surf_cstatus="no_mac-address:"
      fi
    else
      if [ -s "$C3SURF_BLACKLIST_FILE" ]
      then
        # blacklist lesen und mac vergleichen
        mac_result=$(/bin/grep "$c3surf_mac" "$C3SURF_BLACKLIST_FILE")
        if [ -n "$mac_result" ]
        then
          # login leider nicht erlaubt
          c3surf_sstatus="$c3surf_login_msg_macblack"
          c3surf_hstatus="_c3surf_login_msg_macblack"
          c3surf_nstatus=7
          c3surf_cstatus="mac-blacklist:"
        fi
# ####################################
#        {
#          while read fs_mac fs_rest
#          do
#            if [ "$fs_mac" = "$c3surf_mac" ]
#            then
#              # login leider nicht erlaubt
#              c3surf_sstatus="$c3surf_login_msg_macblack"
#              c3surf_hstatus="_c3surf_login_msg_macblack"
#              c3surf_nstatus=7
#              c3surf_cstatus="mac-blacklist:"
#              break
#            fi
#          done
#        } < "$C3SURF_BLACKLIST_FILE"
# #####################################
      fi # end blacklist
    fi # no mac address
  fi

  # Quotas: IP or ID block (check initTimeBlockVar -> c3surf_bn)
  if [ $c3surf_nstatus -eq 0 ]
  then
    initTimeBlockVar
    if [ -s "$c3surf_bn" ]
    then
      # login not permitted: quota block
      c3surf_sstatus="$c3surf_login_msg_quotablock"
      c3surf_hstatus="_c3surf_login_msg_quotablock"
      c3surf_nstatus=7
      c3surf_cstatus="quota-block:"
    fi
    # Ende ip-Block
  fi

  # check curfew-list and this hour
  if [ $c3surf_nstatus -eq 0 ]
  then
    if [ -n "$fscurfew" ]
    then
      for i in $fscurfew
      do
        if [ $i -eq $c3surf_reference_curfew ]
        then
          # login not permitted: curfew lock
          c3surf_sstatus="$c3surf_login_msg_curfew"
          c3surf_hstatus="_c3surf_login_msg_curfew"
          c3surf_nstatus=7
          c3surf_cstatus="curfew:"
          break
        fi
      done
    fi
  fi

  if [ $c3surf_nstatus -eq 0 ]
  then
    # login is allowed
    c3surf_sstatus="$c3surf_login_msg_ok"
    c3surf_hstatus="_c3surf_login_msg_ok"
  fi
}


# worker for command "doLogin"
doLogin()
{
  log_msg="login"
  checkLogin
  # html status feedback
  printHtmlStatus
  # Login
  # ---- Voucher addon (begin) -----
  checkVoucher
  # ---- Voucher addon (end) -----
  if [ $c3surf_nstatus -eq 0 ]
  then
    setUserDefaults
    initQuotaVar
    if [ "$C3SURF_QUOTA" = "yes" ]
    then
      readQuota
    fi
    # do the login (set the timer)
    setTimer
    if [ "$OPT_LOGINUSR" = "yes" ]
    then
      writeUserOnline
    fi
    # log it
    logValid "$log_msg"
  else
    logInvalid
    c3surf_error="nothing done"
    # ---- Voucher addon (begin) -----
    deactivateVoucher
    # ---- Voucher addon (end) -----
  fi
}


# worker for command "doAdminLogin"
# no checks, admin is the master
# just login a HOST!!!
doAdminLogin()
{
  # only check if host is in use
  if [ ! -f "$c3surf_tn" ]
  then
    c3surf_id="-"
    [ $c3surf_vorname ] || c3surf_vorname="hostlogin"
    [ $c3surf_name ] || c3surf_name="unlimited"
    [ $c3surf_mail ] || c3surf_mail="-"
    # do the login (set the timer)
    setTimer
    # log it
    logValid "admin:login"
  else
    c3surf_error="nothing done"
    c3surf_sstatus="multiple login ignored"
    c3surf_cstatus="login_by_admin:"
    logInvalid
  fi
}

# ###################
# Delete User account
# ###################
# delete the user (do a logout first!)
doUserDelete()
{
  # read user vars
  readUserVar
  # remove user .data
  rm -f "$c3surf_un"
  # rm -f "$C3SURF_TMP_PATH/$fsid.quota"
  removeQuota
  # delete PERSISTENT too, if TMP is on
  if [ "$C3SURF_WORKON_TMP" = "yes" ]
  then
    # delete if file exists
    [ -f "$c3surf_persistent_un" ] && rm -f "$c3surf_persistent_un"
  fi
  removeHtPasswd
}



#########################################
# worker doAdminUser
#########################################
# called from admin/c3surf.cgi
# /usr/local/binc3surf_worker.sh "doAdminUser" \
#                                   "$FORM_pw" "$FORM_id" \
#                                   "$FORM_vorname" "$FORM_name" "$FORM_mail" \
#                                   "$FORM_time" "$FORM_blocktime" "$FORM_counter"
#            ++++ new voucher vars:
#                                   "$FORM_module" "$FORM_valid" "$FORM_lives"
doAdminUser()
{
  # check the inputs (we have to do it!)
  if [ -n "$c3surf_id" ]
  then
    initUserVar
    # get original user data
    readUserVar
    # check all user vars and calc valid values (exept password)
    calcUserVar
    # new password -- crypt it!!!! and set internal var
    # returns 0 if there was some work to do
    cryptUserPwd
    if [ $? -eq 0 ]
    then
      # if pw was changed or is new user update .htpasswd file
      writeUserPwd
    fi
    # write the user.data file
    writeUserVar
  else
    c3surf_cmd="userAdmin"
    c3surf_error="webAdmin cannot add user whithout UID"
    logWorker
  fi
}


# #############################################################
# MAIN
# #############################################################

# #############################################################
# INIT
# #############################################################

# init packet globals
. /var/run/c3surf.conf

# init var to enable/disable the quota counter to default,
# update it later in doAutoLogout
c3surf_update_counter="$C3SURF_QUOTA"

# get the Parameters
c3surf_cmd="$1"
# --- parms are variable ---
case "$c3surf_cmd" in
  "doAdminUser")
    c3surf_pw="$2"
    c3surf_id="$3"
    c3surf_vorname="$4"
    c3surf_name="$5"
    c3surf_mail="$6"
    c3surf_time="$7"
    c3surf_blocktime="$8"
    c3surf_counter="$9"
    c3surf_curfew="$10"
    # ---- Voucher addon (begin) -----
    c3surf_module="$11"
    c3surf_valid="$12"
    c3surf_lives="$13"
    # ---- Voucher addon (end) -----
  ;;
  # alle anderen (old 2.2.2 stuff)
  *)
    c3surf_ip="$2"
    c3surf_id="$3" # or time: addIPTime, addTimeBlock
    c3surf_vorname="$4"
    c3surf_name="$5"
    c3surf_mail="$6"
  ;;
esac


if [ -n "$c3surf_ip" ]
then
  # get mac
  getMacFromArp
  # always need the timer 
  # only doUserDelete has to do it because we have the c3surf_id only!
  initTimerVar
fi

# my reference time_stamps (now and tomorrow 00:00)
c3surf_reference_ts=$(/bin/date +%s)
# this hour
c3surf_reference_curfew=$(/bin/date +%H)
# ############
# give c3surf_log_ts same time as c3surf_reference_ts
#c3surf_datediff=`date +%z`
#c3surf_datediff=`expr $c3surf_datediff "*" 36`
#c3surf_log_ts=`expr $c3surf_reference_ts "+" $c3surf_datediff`
#c3surf_log_ts=`date -d "1970.1.1-0:0:$c3surf_log_ts" +%d.%m.%Y-%H:%M:%S`
# ######################################################################
# this is much easier - but not the same timestamp
c3surf_log_ts=$(/bin/date +%d.%m.%Y-%H:%M:%S)
c3surf_tomorrow_ts=$(/bin/date -d +24:0 +%s)

# no parallel processing please
while [ -f /tmp/c3surf_worker.lck ]
do
  c3surf_wait_ts=$(/bin/date +%s)
  if [ $(/usr/bin/expr $c3surf_wait_ts - $c3surf_reference_ts) -gt 2 ]
  then
    c3surf_error="worker waits to long - semaphore deleted"
    logWorker
    rm -f /tmp/c3surf_worker.lck
  fi
done
> /tmp/c3surf_worker.lck

# my logging
c3surf_error="starting"
[ -n "$DEBUG_INFO" ] && logWorker

# #############################################################
# COMMAND HANDLER
# #############################################################

c3surf_error="processing"

case "$c3surf_cmd" in
  "doLogin")
    # set vars for msg and error handling
    c3surf_nstatus="0"
    c3surf_sstatus=""
    c3surf_hstatus=""
    c3surf_cstatus=""
    . /usr/lib/c3surf/c3surf_worker.msg
    doLogin
  ;;

  "doLogout")
    doLogout "logout"
  ;;

  # minimal checks
  # login a host without limits
  # used by admin/c3surf.cgi
  "doAdminLogin")
    doAdminLogin
  ;;

  # logout without any quota-check
  # used my blocklist / blacklist
  # used by admin/c3surf.cgi
  "doAdminLogout")
    doAdminLogout "admin:logout"
  ;;

  # logout without quota-count-check
  # used by admin/c3surf.cgi
  "doAdminNCLogout")
    doAutoLogout "admin:nocount:logout"
  ;;

  # logout without quota-count-check
  # used by login, if next user logs in on same terminal
  "doAutoLogout")
    doAutoLogout "auto:logout"
  ;;

  # logout without quota-count-check
  # used by c3surf_countdown if ARP-CHECK fails
  "doArpLogout")
    doAutoLogout "arpchk:logout"
  ;;

  # logout with quota-count-check / curfew is a real logout event
  # used by c3surf_countdown if checkCurfew hits
  "doCurfewLogout")
    doLogout "curfew:logout"
    # this is without quota check
    # doAutoLogout "curfew:logout"
  ;;

  # logout without quota-count-check
  # used by admin/c3surf.cgi (SystemLock with user logout)
  "doSyslockLogout")
    doAutoLogout "syslock:logout"
  ;;

  # special: no IP for logout, only ID -- no init above done
  # delete a user
  # call: c3surf_worker.sh "doUserDelete" "-" "$id"
  # used by Admin cgi
  "doUserDelete")
    if [ -n "$c3surf_id" ]
    then
      # init user
      initUserVar
      if [ -f "$c3surf_on" ]
      then
        # user is online - we have to init and do a logout
        readUserOnline
        c3surf_ip="$c3surf_online_ip"
        c3surf_mac="$c3surf_online_mac"
        # now we have all for init
        initTimerVar
        #  logout the user
        doAutoLogout "deluser:logout"
      else
        # we need this file in doUserDelete
        initQuotaVar
      fi # online
      doUserDelete
    else
      c3surf_error="worker needs UID for doUserDelete"
      logWorker
    fi
  ;;

  # User Administration
  # called by admin/c3surf.cgi
  "doAdminUser")
    doAdminUser
  ;;

  # used by Admin cgi
  "addIPTime")
    # is a global option
    # initTimerVar
    if [ -f "$c3surf_tn" ]
    then
      readTimer
      if [ $# -ge 3 -a -n "$3" ]
      then
        fstime="$3"
      else
        fstime=$C3SURF_TIME
      fi
      fstimesecs=$(/usr/bin/expr $fstime \* 60)
      c3surf_time=$(/usr/bin/expr $c3surf_time + $fstimesecs)
      c3surf_count=$(/usr/bin/expr $c3surf_count + 1)
      writeTimer
      logValid "addtime:$fstime"
    fi
  ;;

  # ----- OPT_C3SURF_TRAFFIC (begin) -----
  # logout without quota-count-check
  # used by c3surf_traffic.sh
  "doTrafficLogout")
    # we have to check for admin unlimited hostlogin first
    if [ -f "$c3surf_tn" ]
    then
      readTimer
      # do not touch the admin logins for hosts
      if [ "$c3surf_id" != "-" ]
      then
        # this does not check any quota
        # timer starts with full time after the block
        # doAdminLogout "logout:traffic"
        doAutoLogout "logout:traffic"
        if [ -n "$3" ]
        then
          my_blocktime=$(/usr/bin/expr "$3" \* 60)
          initTimeBlockVar $my_blocktime
        else
          initTimeBlockVar
        fi
        addTimeBlock
      fi
    fi
  ;;
  # ----- OPT_C3SURF_TRAFFIC (end) -----

  # .cgi User online
  "addTimeBlock")
    doAdminLogout "admin:block"
    if [ -n "$3" ]
    then
      my_blocktime=$(/usr/bin/expr "$3" \* 60)
      initTimeBlockVar $my_blocktime
    else
      initTimeBlockVar
    fi
    addTimeBlock
  ;;

  # .cgi Blocklist
  "removeTimeBlock")
    # needs $2=ip and $3=id
    initTimeBlockVar
    removeTimeBlock
  ;;

  *)
    c3surf_error="illegal command"
    logWorker
  ;;
esac

[ -n "$DEBUG_INFO" ] && logWorker

# delete semaphore variable
rm -f /tmp/c3surf_worker.lck
