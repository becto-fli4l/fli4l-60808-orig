#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services                                      __FLI4LVER__
##
## Creation:    07.01.2008 Frank Saurbier - c3Surf@arcor.de
## Last Update: $Id$
##
## Copyright (c) 2008-2010 - Frank Saurbier <c3surf@arcor.de>
## Copyright (c) 2010-2016 - Frank Saurbier, fli4l-Team <team@fli4l.de>
##
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------

. /srv/www/include/cgi-helper
. /var/run/c3surf.conf

# language
# Thx to Wieland Groﬂe for helping
if [ -f $C3SURF_READ_PATH/c3surf-lang.overwrite ]
then
  . $C3SURF_READ_PATH/c3surf-lang.overwrite
  if [ -f /srv/www/lang/c3surf.$c3surf_admin_lang ]
  then
    . /srv/www/lang/c3surf.$c3surf_admin_lang
  fi
fi

# fix for fli4l v.3.3.0 and higher versions
if [ -f /var/run/dhcp_leases_dir ]
then
  . /var/run/dhcp_leases_dir
fi

# (fs) 20080317 dnsdhcp leases file
c3surf_leases_file="$DHCP_LEASES_DIR/dnsmasq.leases"

c3surf_tab_virgin=true

###################################
simple_output ()
{
  htmlspecialchars | while read line
  do
    case $FORM_subaction in
    $log_file1) #login
      fsstamp=`echo $line | cut -d "|" -f 1`
      fsip=`echo $line | cut -d "|" -f 2`
      fsid=`echo $line | cut -d "|" -f 3`
      fsvorname=`echo $line | cut -d "|" -f 4`
      fsname=`echo $line | cut -d "|" -f 5`
      fsmail=`echo $line | cut -d "|" -f 6`
      fsmac=`echo $line | cut -d "|" -f 7`
      fsaction=`echo $line | cut -d "|" -f 8`
      echo "<tr>\
<td><nobr>$fsstamp</nobr></td>\
<td><nobr>$fsip</nobr></td>\
<td><nobr>$fsid</nobr></td>\
<td><nobr>$fsvorname</nobr></td>\
<td><nobr>$fsname</nobr></td>\
<td><nobr>$fsmail</nobr></td>\
<td><nobr>$fsmac</nobr></td>\
<td><nobr>$fsaction</nobr></td>\
</tr>"
    ;;
    $log_file2) #invalid
      fsstamp=`echo $line | cut -d "|" -f 1`
      fsmac=`echo $line | cut -d "|" -f 2`
      fserror=`echo $line | cut -d "|" -f 3`
      fsip=`echo $line | cut -d "|" -f 4`
      fsparms=`echo $line | cut -d "|" -f 5`
      echo "<tr>\
<td><nobr>$fsstamp</nobr></td>\
<td><nobr>$fsmac</nobr></td>\
<td><nobr>$fserror</nobr></td>\
<td><nobr>$fsip</nobr></td>\
<td><nobr>$fsparms</nobr></td>\
</tr>"
    ;;
    $log_file3) #page
      fsstamp=`echo $line | cut -d "|" -f 1`
      fsip=`echo $line | cut -d "|" -f 2`
      fsbrowser=`echo $line | cut -d "|" -f 3`
      fsany=`echo $line | cut -d "|" -f 4`
      echo "<tr>\
<td><nobr>$fsstamp</nobr></td>\
<td><nobr>$fsip</nobr></td>\
<td><nobr>$fsbrowser</nobr></td>\
<td><nobr>$fsany</nobr></td>\
</tr>"
    ;;
    *) # generischer Teil f¸r alles andere
      echo "<tr><td><nobr>$line</nobr></td></tr>"
    esac
  done
}


# ###########################################
# Rechte checken (im Moment nur c3surf:admin)
# ###########################################
check_rights "c3surf" "admin"

# TAB: show lockme if system is locked
if [ "$FORM_action" = "" ]
then
  if test -f $C3SURF_SYSLOCK_FILE
  then
    FORM_action="lockme"
  fi
fi

# hack for proper tab-selection, inspired by pf.cgi
: ${FORM_action:=online}
: ${FORM_work:=}


# ####################################################
# beginne mit der Arbeit, dann klappt auch der refresh
# ####################################################
case $FORM_action in
  online)
    case $FORM_work in
      addtime)
        /usr/local/bin/c3surf_worker.sh "addIPTime" "$FORM_host" "$FORM_adtime"
        myrefresh="yes"
      ;;
      block)
        /usr/local/bin/c3surf_worker.sh "addTimeBlock" "$FORM_host" "240"
        myrefresh="yes"
      ;;
      logout)
        /usr/local/bin/c3surf_worker.sh "doAdminNCLogout" "$FORM_host"
        myrefresh="yes"
      ;;
      blacklist)
        /usr/local/bin/c3surf_blacklist.sh "$FORM_mac" "add" "$FORM_host"
        /usr/local/bin/c3surf_worker.sh "doAdminLogout" "$FORM_host"
        myrefresh="yes"
      ;;
    esac
  ;;

  userdata)
    case $FORM_work in
      usr_quotareset)
        # remove quota file
        rm -f "$C3SURF_TMP_PATH/$FORM_uid.quota"
        # better use c3surf_worker.sh "removeTimeBlock" "" "$FORM_uid"
        # remove block file
        [ -f $C3SURF_TMP_PATH/$FORM_uid.block ] && rm -f "$C3SURF_TMP_PATH/$FORM_uid.block"
        myrefresh="yes"
      ;;
      usr_add)
        # call the c3surf_worker()
        # it does all checks an error logs
        /usr/local/bin/c3surf_worker.sh "doAdminUser" \
                                        "$FORM_pw" "$FORM_id" \
                                        "$FORM_vorname" "$FORM_name" "$FORM_mail" \
                                        "$FORM_time" "$FORM_blocktime" "$FORM_counter" \
                                        "$FORM_curfew"
        if [ -n "$FORM_id" ]
        then
          myrefresh="yes"
        else
          myrefresh="no"
        fi
#        myrefresh="yes"
      ;;
      usr_save)
        # call the c3surf_worker()
        # it does all checks an error logs
        /usr/local/bin/c3surf_worker.sh "doAdminUser" \
                                        "$FORM_pw" "$FORM_id" \
                                        "$FORM_vorname" "$FORM_name" "$FORM_mail" \
                                        "$FORM_time" "$FORM_blocktime" "$FORM_counter" \
                                        "$FORM_curfew"
        if [ -n "$FORM_id" ]
        then
          myrefresh="yes"
        else
          myrefresh="no"
        fi
#        myrefresh="yes"
      ;;
      usr_esc)
        myrefresh="yes"
      ;;
      usr_del)
        if [ -n "$FORM_id" ]
        then
          /usr/local/bin/c3surf_worker.sh "doUserDelete" "-" "$FORM_id" "-" "-" "-"
        fi
        myrefresh="yes"
      ;;
    esac
  ;;

  voucher)
    case $FORM_work in
      vch_del)
        if [ -n "$FORM_id" ]
        then
          /usr/local/bin/c3surf_worker.sh "doUserDelete" "-" "$FORM_id" "-" "voucher" "-"
        fi
        myrefresh="yes"
      ;;
      vch_downprint)
        # download the print-file
        http_header download "ctype=text/plain;filename=print_voucher.txt"
        /bin/cat "$C3SURF_VOUCHER_PRINTFILE"
        exit 0
      ;;
      vch_delprint)
        # delete the print-file
        /bin/rm -f "$C3SURF_VOUCHER_PRINTFILE" 2>/dev/null
        myrefresh="yes"
      ;;
      vch_print) # ---- is obsolete ----
        # print the print-file
        myrefresh="yes"
      ;;
      button)
        case "$FORM_cmd" in
          "$_vch_cmd_refresh")                                                                          
            if [ -n "$FORM_filter" ]
            then
              FORM_action="$FORM_action&filter=$FORM_filter"
            fi           
            myrefresh="yes"                                                                               
          ;;                                                                                            
          "$_vch_cmd_gen")
            # generate vouchers, as much as defined in config
            /usr/local/bin/c3surf_voucher_gen.sh
            myrefresh="yes"
          ;;
          "$_vch_cmd_clean")
            # remove expired vouchers
            /usr/local/bin/c3surf_voucher_del.sh
            myrefresh="yes"
          ;;
          "$_vch_cmd_force")
            # ask to remove all vouchers first
            myrefresh="no"
          ;;
          "$_vch_cmd_force_yes")
            # remove all vouchers
            /usr/local/bin/c3surf_voucher_del.sh "force"
            myrefresh="yes"
          ;;
          *)
            myrefresh="yes"
          ;;
        esac
      ;;
      vch_esc)
        myrefresh="yes"
      ;;
    esac
  ;;

  blocklist)
    case $FORM_work in
      unblock)
        /usr/local/bin/c3surf_worker.sh "removeTimeBlock" "$FORM_host" "$FORM_id"
        myrefresh="yes"
      ;;
      blacklist)
        /usr/local/bin/c3surf_blacklist.sh "$FORM_mac" "add" "$FORM_host"
        /usr/local/bin/c3surf_worker.sh "removeTimeBlock" "$FORM_host" "$FORM_id"
        myrefresh="yes"
      ;;
    esac
  ;;

  blacklist)
    if [ "$FORM_work" = "remove" ]
    then
      /usr/local/bin/c3surf_blacklist.sh "$FORM_mac" "remove"
      myrefresh="yes"
    fi
  ;;

  arplist)
    case $FORM_work in
      adm_login)
        /usr/local/bin/c3surf_worker.sh "doAdminLogin" "$FORM_host"
        myrefresh="yes"
      ;;
      blacklist)
        /usr/local/bin/c3surf_blacklist.sh "$FORM_mac" "add" "$FORM_host"
        myrefresh="yes"
      ;;
    esac
  ;;

  leaseslist)
    case $FORM_work in
      adm_login)
        /usr/local/bin/c3surf_worker.sh "doAdminLogin" "$FORM_host"
        myrefresh="yes"
      ;;
      blacklist)
        /usr/local/bin/c3surf_blacklist.sh "$FORM_mac" "add" "$FORM_host"
        myrefresh="yes"
      ;;
    esac
  ;;

  lockme)
    case $FORM_work in
      langselect)
        # select language
        if [ -n $FORM_language ]
        then
          if [ "$FORM_language" = "httpgui"
          then
            rm -f $C3SURF_PERSISTENT_PATH/c3surf-lang.overwrite 2>/dev/null
            if [ "$C3SURF_WORKON_TMP" = "yes" ]
            then
              rm -f $C3SURF_TMP_PATH/c3surf-lang.overwrite 2>/dev/null
            fi
          else
            echo "c3surf_admin_lang=\"$FORM_language\"" > $C3SURF_PERSISTENT_PATH/c3surf-lang.overwrite
            # copy from persistent path to work path
            if [ "$C3SURF_WORKON_TMP" = "yes" ]
            then
              cp -f $C3SURF_PERSISTENT_PATH/c3surf-lang.overwrite $C3SURF_TMP_PATH/c3surf-lang.overwrite
            fi
          fi
        fi
        myrefresh="yes"
      ;;
      lock_logout)
        # lock the system first
        echo "$_lck_msg_system_to_user" > $C3SURF_PERSISTENT_PATH/c3surf.lock
        # copy from persistent path to work path
        if [ "$C3SURF_WORKON_TMP" = "yes" ]
        then
          cp -f $C3SURF_PERSISTENT_PATH/c3surf.lock $C3SURF_SYSLOCK_FILE
        fi
        # logout all users
        for f in $C3SURF_TMP_PATH/*.time
        do
          # schlieﬂe den Abfrage-String aus
          if [ ! "$f" = "$C3SURF_TMP_PATH/*.time" ]
          then
            /usr/local/bin/c3surf_worker.sh "doSyslockLogout" `basename $f .time`
          fi
        done
        myrefresh="yes"
      ;;
      lock_only)
        echo "$_lck_msg_system_to_user" > $C3SURF_PERSISTENT_PATH/c3surf.lock
        if [ "$C3SURF_WORKON_TMP" = "yes" ]
        then
          cp -f $C3SURF_PERSISTENT_PATH/c3surf.lock $C3SURF_SYSLOCK_FILE
        fi
        myrefresh="yes"
      ;;
      unlock)
        rm -f $C3SURF_PERSISTENT_PATH/c3surf.lock
        if [ "$C3SURF_WORKON_TMP" = "yes" ]
        then
          rm -f $C3SURF_SYSLOCK_FILE
        fi
        myrefresh="yes"
      ;;
    esac
  ;;

  logview)
    if [ -n "$FORM_clearlog_yes" -a -n "$FORM_subaction" ]
    then
      # -------------------------------------- not good for httpd.log
      # rm -f $C3SURF_LOG_PATH/$FORM_subaction
      # --------------------------------------
      > $C3SURF_LOG_PATH/$FORM_subaction
      # --------------------------------------
      # unset FORM_subaction
      # --------------------------------------
      FORM_subaction=""
      myrefresh="yes"
    fi

    if [ -n "$FORM_download" -a -n "$FORM_subaction" ]
    then
      http_header download "ctype=text/plain;filename=$FORM_subaction"
      /bin/cat "$C3SURF_LOG_PATH"/"$FORM_subaction"
      exit 0
    fi
  ;;
esac

# ####################################################
# mache eine Refresh, damit die get-Parameter aus dem Browser verschwinden
# ####################################################
if [ "$myrefresh" = "yes" ]
then
  echo "Location: $myname?action=$FORM_action"
  echo
  exit
fi

# nur die "online" Seite aktualisieren
if [ "$FORM_action" = "online" ]
then
  show_html_header "c3Surf $C3SURF_VERSION" "refresh=60;url=$myname?action=$FORM_action"
else
  show_html_header "c3Surf $C3SURF_VERSION"
fi

echo "<h2>c3Surf&nbsp;Admin&nbsp;&nbsp;&nbsp;&rarr;&nbsp;<a href=\"http://$C3SURF_HTTPD_HOST_NAME/\" target=\"_blank\">$_login_page_link</a>&nbsp;&larr;</h2>"
echo "<br>"

# Jetzt in lang/c3surf.de
# _online="Wer ist online"
# _blocklist="IP Blocklist"
# _blacklist="MAC Blacklist"
# _arplist="ARP Tabelle"
# _lockme="Deny all Logins"
# _logview="Logdatei"

#
#################################
# Tabs ausgeben
#################################

# show the leases tab if we have a valid leases file
if [ -f "$c3surf_leases_file" ]
then
  have_lease=yes
else
  have_lease=no
fi

# for use with fli4l < 2.3.0, show voucher tab
# [ -z "$OPT_C3SURF_VOUCHER" ] && OPT_C3SURF_VOUCHER="no"
: ${OPT_C3SURF_VOUCHER:=no}

case $have_lease$OPT_LOGINUSR$OPT_C3SURF_VOUCHER in
  yesyesyes)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_userdata" "$myname?action=userdata" \
                    "$_voucher" "$myname?action=voucher" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_leaseslist" "$myname?action=leaseslist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
  yesyesno)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_userdata" "$myname?action=userdata" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_leaseslist" "$myname?action=leaseslist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
  yesnono)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_leaseslist" "$myname?action=leaseslist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
  noyesyes)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_userdata" "$myname?action=userdata" \
                    "$_voucher" "$myname?action=voucher" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
  noyesno)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_userdata" "$myname?action=userdata" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
  *)
    show_tab_header "$_online" "$myname?action=online" \
                    "$_blocklist" "$myname?action=blocklist" \
                    "$_blacklist" "$myname?action=blacklist" \
                    "$_arplist" "$myname?action=arplist" \
                    "$_lockme" "$myname?action=lockme" \
                    "$_logview" "$myname?action=logview"
  ;;
esac

case $FORM_action in
#################################
###    TAB: Wer ist online    ###
#################################
  online)

    echo "<table class=\"normtable\">"
    # Angemeldete suchen
    for f in $C3SURF_TMP_PATH/*.time
    do
      # schlieﬂe den Abfrage-String aus
      if [ "$f" = "$C3SURF_TMP_PATH/*.time" ]
      then
        show_info "$_gen_info_head" "$_onl_info_nobody_txt"
      else
        {
          read c3surf_time c3surf_count c3surf_id c3surf_ip c3surf_mac c3surf_vorname c3surf_name c3surf_mail c3surf_curfew c3surf_hostname
        } < $f

        # Restzeit berechnen
#        if [ "$c3surf_id" = "-" ]
        if [ $c3surf_time -eq 0 ]
        then
          show_time="&infin;"
        else
          c3surf_now=`date +%s`
          c3surf_time=`/usr/bin/expr $c3surf_time - $c3surf_now`
          c3surf_time=`/usr/bin/expr $c3surf_time / 60`
          if [ $c3surf_time -le 1 ]
          then
            show_time=" &le;1"
          else
            show_time=$c3surf_time
          fi
        fi

        # Ausgabe der Header (einmalig)
        if [ $c3surf_tab_virgin = true ]
        then
          cat <<-EOF
            <tr>
            <td><center><b>$_gen_h_ip</b></center></td>
            <td><center><b>$_gen_h_mac</b></center></td>
            <td><center><b>$_gen_h_hostname</b></center></td>
            <td><center><b>$_gen_h_id</b></center></td>
            <td><center><b>$_gen_h_forename<br>$_gen_h_surname</b></center></td>
            <td><center><b>$_gen_h_mail</b></center></td>
            <td><center><b>$_gen_h_time</b><br><nobr>($_gen_h_minutes)</nobr></center></td>
            <td><center><b>$_gen_h_prolong</b><br><nobr>($_gen_h_minutes)</nobr></center></td>
            <td><center><b>$_gen_h_reduce</b><br><nobr>($_gen_h_minutes)</nobr></center></td>
            <td><center><b>$_gen_h_command</b></center></td>
            </tr>
EOF
          c3surf_tab_virgin=false
        fi

        if [ -z "$c3surf_mail" -o "$c3surf_mail" = "none" -o "$c3surf_mail" = "-" ]
        then
          mymail=""
        else
          mymail="<a href=\"mailto:$c3surf_mail\">$c3surf_mail</a>"
        fi

		c3surf_hostname=`arp $c3surf_ip | cut  -d " " -f 1 | cut -d "." -f 1`

        # Ausgabe der Inhalte
        cat <<-EOF
          <tr>
          <td><center>$c3surf_ip</center></td>
          <td><center>$c3surf_mac</center></td>
          <td><center>$c3surf_hostname</center></td>
          <td><center>$c3surf_id</center></td>
          <td><center>$c3surf_vorname<br>$c3surf_name</center></td>
          <td><center>$mymail</center></td>
          <td><center>$show_time</center></td>
          <td><center>
EOF
#        if [ ! "$c3surf_id" = "-" ]
        if [ ! $c3surf_time -eq 0 ]
        then
          cat <<-EOF
            &nbsp;<a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=15">+15</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=30">+30</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=60">+60</a><br>
            &nbsp;<a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=120">+120</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=240">+240</a>&nbsp;
            </center></td><td><center>
            &nbsp;<a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=-15">-15</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=-30">-30</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=-60">-60</a><br>
            &nbsp;<a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=-120">-120</a>&nbsp;
            <a href="$myname?action=online&work=addtime&host=$c3surf_ip&adtime=-240">-240</a>&nbsp;
EOF
        else
          echo "<td><center></center></td>"
        fi

        echo "</center></td>\
              <td><center>&nbsp;<a href=\"$myname?action=online&work=logout&host=$c3surf_ip\">$_onl_cmd_logout</a>&nbsp;"

        if [ ! "$c3surf_id" = "-" ]
#        if [ ! $c3surf_time -eq 0 ]
        then
          echo "<br>\
               &nbsp;<a href=\"$myname?action=online&work=block&host=$c3surf_ip\">$_onl_cmd_blockip 240 min</a>&nbsp;<br>\
               &nbsp;<a href=\"$myname?action=online&work=blacklist&host=$c3surf_ip&mac=$c3surf_mac\">$_onl_cmd_macblack</a>&nbsp;"
        fi

        echo "</center></td></tr>"

        # Ende else
      fi
    done
    # Tabelle schlieﬂen
    echo "</table>"
  #################################
  ### ende   TAB: Wer ist online    ###
  #################################
  ;;
  #################################
  ###      TAB: Accounts        ###
  #################################
  userdata)

# ---------------------------
# Arbeitsbereich Benutzer neu anlegen, editiern und lˆschen
# ---------------------------
# init vars
  my_cmd="$_usr_cmd_add"
  my_work="usr_add"
  my_esc=""

  if [ "$FORM_work" = "usr_edit" ]
  then
    if [ -f "$C3SURF_READ_PATH/$FORM_uid.data" ]
    then
      . "$C3SURF_READ_PATH/$FORM_uid.data"
      my_cmd="$_usr_cmd_save"
      my_work="usr_save"
      my_esc="<a href=\"$myname?action=userdata&work=usr_esc\">$_usr_cmd_esc</a>"
    fi
  fi

  if [ "$FORM_work" = "usr_delete" ]
  then
    if [ -f "$C3SURF_READ_PATH/$FORM_uid.data" ]
    then
      . "$C3SURF_READ_PATH/$FORM_uid.data"
      my_cmd="$_usr_cmd_del"
      my_work="usr_del"
      my_esc="<a href=\"$myname?action=userdata&work=usr_esc\">$_usr_cmd_esc</a>"
    fi
  fi

# Form
    cat <<-EOF
      <table class="normtable">
      <tr>
      <form action="$myname" method="get">
      <table class="normtable">
      <tr>
      <td colspan="9"><center>$_gen_h_workingarea</center></td>
      </tr>
      <tr>
      <td><center><b>$_gen_h_id</b></center></td>
      <td><center><b>$_gen_h_forename</b></center></td>
      <td><center><b>$_gen_h_surname</b></center></td>
      <td><center><b>$_gen_h_mail</b></center></td>
      <td><center><b>$_gen_h_password</b></center></td>
      <td><center><b>$_gen_h_time<br>$_gen_h_minutes</b></center></td>
      <td><center><b>$_gen_h_blocktime<br>$_gen_h_minutes</b></center></td>
      <td><center><b>$_gen_h_counter</b></center></td>
      <td><center><b>$_gen_h_command</b></center></td>
      </tr>
      <tr>
      <td><center><input type="text" name="id" size="12" value="$fsid"></center></td>
      <td><center><input type="text" name="vorname" size="15" value="$fsvorname"></center></td>
      <td><center><input type="text" name="name" size="20" value="$fsname"></center></td>
      <td><center><input type="text" name="mail" size="25" value="$fsmail"></center></td>
      <td><center><input type="password" name="pw" size="12"></center></td>
      <td><center><input type="text" name="time" size="5" value="$fstime"></center></td>
      <td><center><input type="text" name="blocktime" size="5" value="$fsblocktime"></center></td>
      <td><center><input type="text" name="counter" size="5" value="$fscounter"></center></td>
      <td><center>
      <input type="hidden" name="action" value="userdata">
      <input type="hidden" name="work" value="$my_work">
      <input type="submit" value="$my_cmd" class="actions">
      </td>
      </tr>
      <tr>
      <td colspan="5" align="right">$_usr_h_defaultvals</td>
      <td><center>$C3SURF_TIME</center></td>
      <td><center>$C3SURF_BLOCKTIME</center></td>
      <td><center>$C3SURF_COUNTER</center></td>
      <td><center>$my_esc</center></td>
      </tr>
      <tr>
      <td colspan="2" align="right">$_usr_h_curfew</td>
      <td colspan="6" align="left"><input type="text" name="curfew" size="72" value="$fscurfew"></td>
      <td><center></center></td>
      </tr>
      </table>
      </tr>
      </table>
      <br>
EOF


# ---------------------------
# Anzeigebereich Benutzer 
# ---------------------------
if [ -z "$FORM_work"]
then
    echo "<table class=\"normtable\">"
    # Userdaten suchen
    for f in "$C3SURF_READ_PATH"/*.data
    do
      # schlieﬂe den Abfrage-String aus
      if [ "$f" = "$C3SURF_READ_PATH/*.data" ]
      then
        show_info "$_gen_info_head" "$_usr_info_noaccount_txt"
      else
        . "$f"
#
# Userdaten ggf. auf Default setzen
#
        # do work if user has no module signature
        # ---- Voucher addon (signature) -----
        # need this after activateVoucher in c3surf_worker
        if [ -z "$fsmodule"]
        then
          # C3SURF_TIME
          if [ -z "$fssecs" ]
          then
            fstime=$C3SURF_TIME
            fssecs=$C3SURF_SECS
          else
            # compatibility with older c3surf version
            if [ -z "$fstime" ]
            then
              fstime=$(expr "$fssecs" \/ "60")
            fi
          fi
          # C3SURF_BLOCKTIME
          if [ -z "$fsblocksecs" ]
          then
            fsblocktime=$C3SURF_BLOCKTIME
            fsblocksecs=$C3SURF_BLOCKSECS
          else
            # compatibility with older c3surf version
            if [ -z "$fsblocktime" ]
            then
              if [ $fsblocksecs -gt 0 ]
              then
                fsblocktime=$(expr "$fsblocksecs" \/ "60")
              else
                fsblocktime=$fsblocksecs
              fi
            fi
          fi
          # C3SURF_COUNTER
          if [ -z "$fscounter" ]
          then
            fscounter=$C3SURF_COUNTER
          fi
#
# Lese die Quota Daten
#
          if [ -f "$C3SURF_TMP_PATH/$fsid.quota" ]
          then
            {
              read c3surf_quota_ts c3surf_quota_time c3surf_quota_maxtime c3surf_quota_count c3surf_quota_maxcount
            } < "$C3SURF_TMP_PATH/$fsid.quota"
            # noch f¸r Anzeige berechnen
#
# --- hier noch berechnen
#
#  ggf. die aktuellen Zahlen ermitteln - Rechenzeit?
#
# --- hier noch berechnen
#
          else
            c3surf_quota_ts="-"
            c3surf_quota_time="-"
            c3surf_quota_maxtime="-"
            c3surf_quota_count="-"
            c3surf_quota_maxcount="$fscounter"
          fi

          # special display for negative c3surf_quota_maxcount
          case $c3surf_quota_maxcount in
            -2)
              c3surf_quota_maxcount="*"
            ;;
            -1)
              c3surf_quota_maxcount="-"
            ;;
          esac
          if [ $c3surf_quota_maxtime -eq 0 ] ; then c3surf_quota_time="&infin;" ; fi

          # Ausgabe der Header (einmalig)
          if [ $c3surf_tab_virgin = true ]
          then
            cat <<-EOF
              <tr>
              <td rowspan="2"><center><b>$_gen_h_id</b></center></td>
              <td rowspan="2"><center><b>$_gen_h_forename<br>$_gen_h_surname</b></center></td>
              <td rowspan="2"><center><b>$_gen_h_mail<br>$_gen_h_curfew</b></center></td>
              <td rowspan="2"><center><b>$_gen_h_time<br>$_gen_h_minutes</b><br><nobr>($_gen_h_seconds)</nobr></center></td>
              <td rowspan="2"><center><b>$_gen_h_blocktime<br>$_gen_h_minutes</b><br><nobr>($_gen_h_seconds)</nobr></center></td>
              <td rowspan="2"><center><b>$_gen_h_counter</b></center></td>
              <td rowspan="2"><center></center></td>
              <td colspan="2"><center><b>$_gen_h_quota</b></center></td>
              <td rowspan="2"><center><b>$_gen_h_command</b></center></td>
              </tr>
              <tr>
              <td><center><b>$_gen_h_time<br></b>$_gen_h_seconds</center></td>
              <td><center><b>$_gen_h_counter / $_gen_h_max</b></center></td>
              </tr>
EOF
            c3surf_tab_virgin=false
          fi

          if [ -z "$fsmail" -o "$fsmail" = "none" -o "$fsmail" = "-" ]
          then
            mymail=""
          else
            mymail="<a href=\"mailto:$fsmail\">$fsmail</a>"
          fi

          my_bgcolor=""
          if [ -f $C3SURF_TMP_PATH/$fsid.online ]
          then
            my_bgcolor=" bgcolor=\"lightgreen\""
          fi
          if [ -f $C3SURF_TMP_PATH/$fsid.block ]
          then
            my_bgcolor=" bgcolor=\"lightcoral\""
          fi

          # ###################
          #if [ -n "$fscurfew" ]
          #then
          #  mymail="$mymail<br><nobr>$fscurfew</nobr>"
          #fi
          # ###################

          # Ausgabe der Inhalte
          cat <<-EOF
            <tr>
            <td$my_bgcolor><center>$fsid</center></td>
            <td><center>$fsvorname<br>$fsname</center></td>
            <td><center>$mymail<br><nobr>$fscurfew</nobr></center></td>
            <td><center>$fstime<br>($fssecs)</center></td>
            <td><center>$fsblocktime<br>($fsblocksecs)</center></td>
            <td><center>$fscounter</center></td>
            <td><center></center></td>
            <td><center>$c3surf_quota_time</center></td>
            <td><center>$c3surf_quota_count / $c3surf_quota_maxcount</center></td>
            <td><center>
            <nobr>$_usr_cmd_userprefix <a href="$myname?action=userdata&work=usr_edit&uid=$fsid">$_usr_cmd_edit</a></nobr>
            <nobr><a href="$myname?action=userdata&work=usr_delete&uid=$fsid">$_usr_cmd_del</a></nobr>
            <nobr>$_usr_cmd_quotaprefix <a href="$myname?action=userdata&work=usr_quotareset&uid=$fsid">$_usr_cmd_quotareset</a></nobr>
            </center></td>
            </tr>
EOF
        else
          # unset the voucher data
          unset fsmodule fsvalid
        fi  # end if -z fsmodule
        # unset the module-tag
        unset fsid fspw fsname fsvorname fsmail fssecs fsblocksecs fscounter fstime fsblocktime fscurfew
      # End else read user.data file
      fi
    done
    # Tabelle schlieﬂen
    echo "</table>"

fi # check FORM_work


  #################################
  ### ende   TAB: Accounts      ###
  #################################
  ;;


  #################################
  ###      TAB: Voucher        ###
  #################################
  voucher)
  # init vars
  # for expire date calculation
  v_now=$(/bin/date +%s)

  # : ${FORM_filter:=*}
  
  if [ -n "$FORM_filter" ]                                                                      
  then                                                                                          
    filter_option="<option value=\"$FORM_filter\">$FORM_filter</option>"
  fi                                                                                            
  filter_option="$filter_option <option value=\"\">all</option>"                                
  for idx in `seq 1 $C3SURF_VOUCHER_N`
  do
    filter_option="$filter_option <option value=\"$idx-\">$idx</option>"
  done

# ---------------------------
# Arbeitsbereich Voucher
# ---------------------------
# init vars
#  my_cmd="$_usr_cmd_add"
#  my_work="usr_add"
#  my_esc=""


if [ -s "$C3SURF_VOUCHER_PRINTFILE" ]
then

  select other languages
  myLangSelect=""
  for f in /srv/www/lang/c3surf.*
  do
    # schlieﬂe den Abfrage-String und die selektierte Sprache aus
    if [ "$f" != "/srv/www/lang/c3surf.*" ]
    then
      otherLang=${f#*.}
      myLangSelect="$myLangSelect&nbsp;&lsaquo;<a href=\"c3surf_voucher_print.cgi?language=$otherLang\" target=\"_blank\">$otherLang</a>&rsaquo;"
    fi
  done
  if [ -n "$myLangSelect" ]
  then
    myLangSelect="$_vch_cmd_print$myLangSelect&nbsp;&nbsp;"
  else
    myLangSelect="<a href=\"c3surf_voucher_print.cgi\" target=\"_blank\">$_vch_cmd_print</a>"
  fi

cat <<-EOF
<br>
<center>
$_gen_h_printfile
<br>
<nobr>$_gen_h_workfile
&nbsp;
<!-- <a href=\"$myname?action=voucher&work=vch_print\">$_vch_cmd_print</a> -->
<!-- <a href=\"c3surf_voucher_print.cgi\" target=\"_blank\">$_vch_cmd_print</a> -->
$myLangSelect 
&nbsp;
<a href="$myname?action=voucher&work=vch_downprint">$_vch_cmd_downprint</a>
&nbsp;
<a href="$myname?action=voucher&work=vch_delprint">$_vch_cmd_delprint</a>
</center>
EOF
fi


#if [ -n "$FORM_clearlog" ]
if [ "$FORM_cmd" = "$_vch_cmd_force" ]
then
  vch_ask="&rarr;<input class=\"sbutton\" type=\"submit\" name=\"cmd\" value=\"$_vch_cmd_force_yes\">"
fi


cat <<-EOF
<br>
<form action="$myname" method="GET">
<input type="hidden" name="action" value="voucher">
<input type="hidden" name="work" value="button">
&nbsp;<select name="filter">
$filter_option
</select>
&nbsp;
<input class="sbutton" type="submit" name="cmd" value="$_vch_cmd_refresh">
&nbsp;&nbsp;
&nbsp;&nbsp;
<input class="sbutton" type="submit" name="cmd" value="$_vch_cmd_gen">
<input class="sbutton" type="submit" name="cmd" value="$_vch_cmd_clean">
&nbsp;
&nbsp;
<input class="sbutton" type="submit" name="cmd" value="$_vch_cmd_force">$vch_ask
</form>
EOF

if [ "$FORM_cmd" = "$_vch_cmd_force" ]
then
  show_info "$_vch_force_titel" "$_vch_force_question"
fi

# ---------------------------
# Anzeigebereich Voucher
# ---------------------------
if [ -z "$FORM_work"]
then
    echo "<table class=\"normtable\">"
    # Userdaten suchen
    for f in "$C3SURF_READ_PATH"/$FORM_filter*.data-voucher
    do
      # schlieﬂe den Abfrage-String aus
      if [ "$f" = "$C3SURF_READ_PATH/$FORM_filter*.data-voucher" ]
      then
        show_info "$_gen_info_head" "$_vch_info_noaccount_txt"
      else
        . "$f"
        # valid and valid thru
        if [ $fsvalid -eq 0 ]
        then
          v_valid="ultimo"
          v_status="valid"
        else
          # need datediff in loop summer/winter difference
          v_datediff=`/bin/date -d "1970.1.1-0:0:$fsvalid" +%z`
          v_datediff=`/usr/bin/expr $v_datediff "*" 36`
          v_valid=`/usr/bin/expr $fsvalid "+" $v_datediff`
          v_valid=`/bin/date -d "1970.1.1-0:0:$v_valid" +%d.%m.%Y-%H:%M:%S`
          if [ $fsvalid -eq 0 -o $fsvalid -gt $v_now ]
          then
            v_status="good"
          else
            v_status="expired"
          fi
        fi
#
# Checke Quota Daten
#
        if [ -f "$C3SURF_TMP_PATH/$fsid.quota" ]
        then
          {
            read c3surf_quota_ts c3surf_quota_time c3surf_quota_maxtime c3surf_quota_count c3surf_quota_maxcount
          } < "$C3SURF_TMP_PATH/$fsid.quota"

          if [ $c3surf_quota_maxtime -eq 0 ]
          then
            # unlimited gibt es nocht nicht
            v_status="$v_status ($c3surf_quota_count)"
          else
            # das hier gibt es
            my_left=`/usr/bin/expr $c3surf_quota_time "/" 60`
            if [ $my_left -lt 1 ]
            then
              my_left="&lt;1"
            fi
            v_status="$v_status ($c3surf_quota_count)"
            fstime="$my_left&frasl;$fstime"
          fi
        fi

        case $fslives in
          -1)
            v_lives=$_lives_date
          ;;
          0)
            v_lives=$_lives_once
          ;;
          *)
            v_lives="$fslives $_lives_hour"
          ;;
        esac

        my_bgcolor=""
        if [ -f $C3SURF_TMP_PATH/$fsid.online ]
        then
          my_bgcolor=" bgcolor=\"lightgreen\""
        fi
        if [ -f $C3SURF_TMP_PATH/$fsid.block ]
        then
          my_bgcolor=" bgcolor=\"lightcoral\""
        fi

        # Ausgabe der Header (einmalig)
        if [ $c3surf_tab_virgin = true ]
        then
          cat <<-EOF
            <tr>
            <td><center><b>$_gen_h_id</b></center></td>
            <td><center><b>$_gen_h_time<br>$_gen_h_minutes</b></center></td>
            <td><center><b>$_gen_h_expiredate</b></center></td>
            <td><center><b>$_gen_h_lives</b></center></td>
            <td><center><b>$_gen_h_status</b></center></td>
            <td><center><b>$_gen_h_created</b></center></td>
            <td><center></center></td>
            <td><center><b>$_gen_h_command</b></center></td>
            </tr>
EOF
          c3surf_tab_virgin=false
        fi

        # Ausgabe der Inhalte
        cat <<-EOF
          <tr>
          <td$my_bgcolor><center>$fsid</center></td>
          <td><center>$fstime</center></td>
          <td><center>$v_valid</center></td>
          <td><center>$v_lives</center></td>
          <td><center><nobr>$v_status</nobr></center></td>
          <td><center>$fscreated</center></td>
          <td><center></center></td>
          <td><center>
          <nobr><a href="$myname?action=voucher&work=vch_del&id=$fsid">$_vch_cmd_del</a></nobr>
          </center></td>
          </tr>
EOF
        unset fsid fspw fsname fsvorname fsmail fssecs fsblocksecs fscounter fstime fsblocktime fscurfew fscreated fsmodule fsvalid fslives
        unset c3surf_quota_ts c3surf_quota_time c3surf_quota_maxtime c3surf_quota_count c3surf_quota_maxcount

      fi # End else read .data-voucher files
    done
    # Tabelle schlieﬂen
    echo "</table>"

fi # check FORM_work


  #################################
  ### ende   TAB: Voucher       ###
  #################################
;;



  #################################
  ### anfang   TAB: IP/UID-Blocklist   ###
  #################################
  blocklist)

  echo "<table class=\"normtable\">"
  # Angemeldete suchen
  for f in $C3SURF_TMP_PATH/*.block
  do
    # schlieﬂe den Abfrage-String aus
    if [ "$f" = "$C3SURF_TMP_PATH/*.block" ]
    then
      show_info "$_gen_info_head" "$_blo_info_noblock_txt"
    else
      {
        read c3surf_time c3surf_mac c3surf_id c3surf_ip
      } < $f

      # Restzeit berechnen
      c3surf_now=`date +%s`
      c3surf_time=`/usr/bin/expr $c3surf_time - $c3surf_now`
      c3surf_time=`/usr/bin/expr $c3surf_time / 60`
      if [ $c3surf_time -le 1 ]
      then
        c3surf_time=" &le;1"
      fi

      # Ausgabe der Header (einmalig)
      if [ $c3surf_tab_virgin = true ]
      then
        echo "<tr>\
             <td><center><b>$_gen_h_id</b></center></td>\
             <td><center><b>$_gen_h_ip</b></center></td>\
             <td><center><b>$_gen_h_mac</b></center></td>\
             <td><center><b>$_gen_h_timelock<br>(Minuten)</b></center></td>\
             <td><center><b>$_gen_h_command</b></center></td>\
             </tr>"
        c3surf_tab_virgin=false
      fi

      # Ausgabe der Inhalte
      echo "<tr>\
            <td><center>$c3surf_id</center></td>\
            <td><center>$c3surf_ip</center></td>\
            <td><center>$c3surf_mac</center></td>\
            <td><center>$c3surf_time</center></td>\
            <td><center>&nbsp;<a href=\"$myname?action=blocklist&work=unblock&host=$c3surf_ip&id=$c3surf_id\">$_blo_cmd_unlock</a>&nbsp;"
      if [ ! "$c3surf_mac" = "" ]
      then
        echo "<br>&nbsp;<a href=\"$myname?action=blocklist&work=blacklist&host=$c3surf_ip&mac=$c3surf_mac\">$_onl_cmd_macblack</a>&nbsp;"
      fi
      echo "</center></td></tr>"

      # Ende else
    fi
  done
  # Tabelle schlieﬂen
  echo "</table>"
  #################################
  ### ende   TAB: IP-Blocklist    ###
  #################################
  ;;
  #################################
  ### anfang   TAB: MAC-Blacklist   ###
  #################################
  blacklist)

    echo "<table class=\"normtable\">"
    # Blacklist pr¸fen
    if [ -s $C3SURF_BLACKLIST_FILE ]
    then
      # schreibe den header
      echo "<tr>\
           <td><center><b>$_gen_h_mac</b></center></td>\
           <td><center><b>$_gen_h_forename</b></center></td>\
           <td><center><b>$_gen_h_surname</b></center></td>\
           <td><center><b>$_gen_h_mail</b></center></td>\
           <td><center><b>$_gen_h_command</b></center></td>\
           </tr>"
      # hole die Daten des Surfers
      {
        while read c3surf_mac c3surf_vorname c3surf_name c3surf_mail
        do
          # schreibe raus
          echo "<tr>\
                <td><center>&nbsp;$c3surf_mac&nbsp;</center></td>\
                <td><center>&nbsp;$c3surf_vorname&nbsp;</center></td>\
                <td><center>&nbsp;$c3surf_name&nbsp;</center></td>\
                <td><center>&nbsp;<a href=\"mailto:$c3surf_mail\">$c3surf_mail</a>&nbsp;</center></td>\
                <td><center>&nbsp;<a href=\"$myname?action=blacklist&work=remove&mac=$c3surf_mac\">$_blk_cmd_remove</a>&nbsp;</center></td>\
                </tr>"
        done
      } < $C3SURF_BLACKLIST_FILE
    else
      # Keine Blacklist vorhanden
      show_info "$_gen_info_head" "$_blk_info_noblock_txt"
    fi
    # Tabelle schlieﬂen
    echo "</table>"
  # Ende TAB:blaclist [action=blacklist]
  #################################
  ### ende   TAB: Blacklist   ###
  #################################
  ;;
  #################################
  ### anfang   TAB: ARP-list   ###
  #################################
  arplist)

    echo "<table class=\"normtable\">"

    # ARP-List lesen
    {
      read c3surf_header
      while read c3surf_ip c3surf_hwtype c3surf_flags c3surf_mac c3surf_mask c3surf_device
      do

        # Ausgabe der Header (einmalig)
        if [ $c3surf_tab_virgin = true ]
        then
          echo "<tr>\
               <td><center><b>$_gen_h_ip</b></center></td>\
               <td><center><b>$_gen_h_mac</b></center></td>\
               <td><center><b>$_gen_h_device</b></center></td>\
               <td><center><b>$_gen_h_command</b></center></td>\
               </tr>"
          c3surf_tab_virgin=false
        fi

        # Ausgabe der Inhalte
        echo "<tr>\
              <td><center>$c3surf_ip</center></td>\
              <td><center>$c3surf_mac</center></td>\
              <td><center>$c3surf_device</center></td>\
              <td><center>&nbsp;<a href=\"$myname?action=arplist&work=adm_login&host=$c3surf_ip\">$_arp_cmd_allow_forever</a>&nbsp;"
        if [ -n "$c3surf_mac" ]
        then
          echo "&nbsp;<a href=\"$myname?action=arplist&work=blacklist&host=$c3surf_ip&mac=$c3surf_mac\">$_onl_cmd_macblack</a>&nbsp;"
        fi
        echo "</center></td></tr>"

      done
    } < /proc/net/arp

    if [ $c3surf_tab_virgin = true ]
    then
      # Keine ARP Eintr‰ge vorhanden
      show_info "$_gen_info_head" "$_arp_info_noarp_txt"
    fi
    # Tabelle schlieﬂen
    echo "</table>"
  # Ende TAB:arplist [action=arplist]
  #################################
  ### ende   TAB: ARP-list   ###
  #################################
  ;;
  ###################################
  ### anfang   TAB: LEASES-list   ###
  ###################################
  leaseslist)

    echo "<table class=\"normtable\">"

    # LEASES lesen
    if [ -s $c3surf_leases_file ]
    then

      # LEASE Ende Berechnung vorbereiten (Konstante aus der Schleife ziehen)
      c3surf_datediff=`date +%z`
      c3surf_datediff=`expr $c3surf_datediff "*" 36`

      {
        while read c3surf_leaseend c3surf_mac c3surf_ip c3surf_hostname c3surf_fix
        do

          # LEASE Ende berechnen
          if [ $c3surf_leaseend -eq 0 ]
          then
            c3surf_leaseend=$_lease_neverends
          else
            c3surf_leaseend=`expr $c3surf_leaseend "+" $c3surf_datediff`
            c3surf_leaseend=`date -d "1970.1.1-0:0:$c3surf_leaseend" +%d.%m.%Y'&nbsp;'%H:%M:%S`
          fi

          # Ausgabe der Header (einmalig)
          if [ $c3surf_tab_virgin = true ]
          then
            echo "<tr>\
                 <td><center><b>$_gen_h_ip</b></center></td>\
                 <td><center><b>$_gen_h_mac</b></center></td>\
                 <td><center><b>$_gen_h_hostname</b></center></td>\
                 <td><center><b>$_gen_h_leaseend</b></center></td>\
                 <td><center><b>$_gen_h_command</b></center></td>\
                 </tr>"
            c3surf_tab_virgin=false
          fi

          # Ausgabe der Inhalte
          echo "<tr>\
                <td><center>$c3surf_ip</center></td>\
                <td><center>$c3surf_mac</center></td>\
                <td><center>$c3surf_hostname</center></td>\
                <td><center>$c3surf_leaseend</center></td>\
                <td><center>&nbsp;<a href=\"$myname?action=leaseslist&work=adm_login&host=$c3surf_ip\">$_lease_cmd_allow_forever</a>&nbsp;"
          if [ -n "$c3surf_mac" ]
          then
            echo "&nbsp;<a href=\"$myname?action=leaseslist&work=blacklist&host=$c3surf_ip&mac=$c3surf_mac\">$_onl_cmd_macblack</a>&nbsp;"
          fi
          echo "</center></td></tr>"

        done
      } < $c3surf_leases_file
    fi

    if [ $c3surf_tab_virgin = true ]
    then
      # Keine LEASE Eintr‰ge vorhanden
      show_info "$_gen_info_head" "$_lease_info_nolease_txt"
    fi
    # Tabelle schlieﬂen
    echo "</table>"
  # Ende TAB:leaseslist [action=leaseslist]
  #################################
  ### ende   TAB: LEASES-list   ###
  #################################
  ;;

  #################################
  ### anfang   TAB: Lock c3Surf  ###
  #################################
  lockme)

    # select other languages
    myLangSelect=""
    for f in /srv/www/lang/c3surf.*
    do
      # schlieﬂe den Abfrage-String und die selektierte Sprache aus
      # if [ "$f" != "/srv/www/lang/c3surf.*" -a "$f" != "/srv/www/lang/c3surf.$c3surf_admin_lang" ]
      if [ "$f" != "/srv/www/lang/c3surf.*" ]
      then
        otherLang=${f#*.}
        if [ "$otherLang" = "$c3surf_admin_lang" ]
        then
          myLangSelect="$myLangSelect&nbsp;&lsaquo;<a href=\"$myname?action=lockme&work=langselect&language=$otherLang\"><b>$otherLang</b></a>&rsaquo;"
        else
          myLangSelect="$myLangSelect&nbsp;&lsaquo;<a href=\"$myname?action=lockme&work=langselect&language=$otherLang\">$otherLang</a>&rsaquo;"
        fi
      fi
    done
    if [ -n "$myLangSelect" ]
    then
      if [ "$c3surf_admin_lang" = "httpgui" -o -z "$c3surf_admin_lang" ]
      then
        myLangSelect="&nbsp;<a href=\"$myname?action=lockme&work=langselect&language=httpgui\"><b>$_select_default</b></a>$myLangSelect"
      else
        myLangSelect="&nbsp;<a href=\"$myname?action=lockme&work=langselect&language=httpgui\">$_select_default</a>$myLangSelect"
      fi
      cat <<-EOF
      &nbsp;$_select_language: $myLangSelect
      <br>
EOF
    fi

    # check System Status
    if test -f $C3SURF_SYSLOCK_FILE
    then
      show_error "$_lck_msg_header" "$_lck_msg_system_is_locked"
      echo "<table class=\"normtable\">"
      echo "<br>"
      echo "<tr>\
            <td><center> &nbsp; <a href=\"$myname?action=lockme&work=unlock\"><b>$_lck_action_unlock</b></a> &nbsp; </center></td>\
            </tr>"
    else
      if [ "$OPT_LOGINUSR" = "yes" ]
      then
        show_info "$_lck_msg_header" "$_lck_msg_system_is_unlocked_for_login"
      else
        show_info "$_lck_msg_header" "$_lck_msg_system_is_unlocked_for_free"
      fi

      echo "<table class=\"normtable\">"
      echo "<br>"
      echo "<tr>\
            <td><center>&nbsp;<a href=\"$myname?action=lockme&work=lock_only\"><b>$_lck_action_lock_only</b></a>&nbsp;</center></td>\
            <td><center>&nbsp;<a href=\"$myname?action=lockme&work=lock_logout\"><b>$_lck_action_lock_logout</b></a>&nbsp;</center></td>\
            </tr>"
    fi
    # Tabelle schlieﬂen
    echo "</table>"
    echo "<br>"
  # Ende TAB:online [main=lockme]
  #################################
  ### ende   TAB: Lock 3surf   ###
  #################################
  ;;
  #################################
  ### anfang   TAB: Logview     ###
  #################################
  logview)

    # some constants (basename of log) = sort order of logfiles
    log_file1="c3surf_login.log"
    log_file2="c3surf_invalid.log"
    log_file3="c3surf_page.log"
    log_file4="c3surf_httpd.log"
    log_file5="c3surf_worker.log"
    _log_sort_normal_idx="1"
    _log_sort_reverse_idx="2"

    # some defaults
    : ${FORM_log_lines:=30}
    : ${FORM_log_sort:=$_log_sort_reverse_idx}
    : ${FORM_showall:=}

    #################################
    # Tabs generieren
    #################################

    fs_tabs=""
    for f in "$log_file1" "$log_file2" "$log_file3" "$log_file4" "$log_file5"
    do
      # teste ob Datei da ist
      # if test -f $C3SURF_LOG_PATH/$f
      # teste ob Datei mit Inhalt da ist
      if test -s $C3SURF_LOG_PATH/$f
      then
        : ${FORM_subaction:=$f}
        if [ $FORM_subaction = $f ]
        then
          fs_tabs=$(echo "$fs_tabs" "$f" "no")
        else
          fs_tabs=$(echo "$fs_tabs" "$f" "$myname?action=logview&subaction=$f&log_sort=$FORM_log_sort&log_lines=$FORM_log_lines")
        fi
      fi
    done

    #################################
    # Tabs ausgeben
    #################################
    if [ -n "$fs_tabs" ]
    then

      # prepare select field for sort
      log_sorta_idx=$FORM_log_sort
      if [ $log_sorta_idx -eq $_log_sort_reverse_idx ]
      then
        log_sortb_idx=$_log_sort_normal_idx
        log_sortb_txt=$_log_sort_normal_txt
        log_sorta_txt=$_log_sort_reverse_txt
      else
        log_sortb_idx=$_log_sort_reverse_idx
        log_sortb_txt=$_log_sort_reverse_txt
        log_sorta_txt=$_log_sort_normal_txt
      fi

      # formular Auswahl
      cat <<-EOF
<br>
<form action="$myname" method="GET">
&nbsp;<select name="log_sort">
<option value="$log_sorta_idx">$log_sorta_txt</option>
<option value="$log_sortb_idx">$log_sortb_txt</option>
</select>
<input type="hidden" name="action" value="logview">
<input type="hidden" name="subaction" value="$FORM_subaction">
<input class="itext" type="text" size="3" maxlength="4" name="log_lines" value="$FORM_log_lines">&nbsp;
<input class="sbutton" type="submit" value="$_log_refresh">&nbsp;
<input class="sbutton" type="submit" name="showall" value="$_log_show_all">
<input class="sbutton" type="submit" name="download" value="$_log_download">
&nbsp;&nbsp;<input class="sbutton" type="submit" name="clearlog" value="$_log_clear">
EOF

      if [ -n "$FORM_clearlog" ]
      then
        cat <<-EOF
&rarr;<input class="sbutton" type="submit" name="clearlog_yes" value="$_log_clear_yes">
EOF
      fi

      cat <<-EOF
</form>
EOF

      if [ -n "$FORM_clearlog" ]
      then
        show_info "$FORM_subaction" "$_log_clear_question"
      else
        show_tab_header $fs_tabs

        # table header
        echo "<table class=\"normtable\">"

          if [ $c3surf_tab_virgin = true ]
          then
            case $FORM_subaction in
              $log_file1)
                echo "<tr>\
                 <th><center>$_gen_h_timestamp</center></th>\
                 <th><center>$_gen_h_ip</center></th>\
                 <th><center>$_gen_h_id</center></th>\
                 <th><center>$_gen_h_forename</center></th>\
                 <th><center>$_gen_h_surname</center></th>\
                 <th><center>$_gen_h_mail</center></th>\
                 <th><center>$_gen_h_mac</center></th>\
                 <th><center>$_gen_h_action</center></th>\
                 </tr>"
              ;;
              $log_file2)
                echo "<tr>\
                 <th><center>$_gen_h_timestamp</center></th>\
                 <th><center>$_gen_h_mac</center></th>\
                 <th><center>$_gen_h_error</center></th>\
                 <th><center>$_gen_h_ip</center></th>\
                 <th><center>$_gen_h_parms</center></th>\
                 </tr>"
              ;;
              $log_file3)
                 echo "<tr>\
                 <th><center>$_gen_h_timestamp</center></th>\
                 <th><center>$_gen_h_ip</center></th>\
                 <th><center>$_gen_h_browser</center></th>\
                 <th><center>$_gen_h_any</center></th>\
                 </tr>"
              ;;
              *)
                echo "<tr>\
                 <th><left><nobr>$_gen_h_generic</nobr></left></th>\
                 </tr>"
            esac
            c3surf_tab_virgin=false
          fi

        # generischer Teil
        if [ -n "$FORM_showall" ]
        then
          if [ $FORM_log_sort -eq $_log_sort_reverse_idx ]
          then
            cat $C3SURF_LOG_PATH/$FORM_subaction | sed '1!G;h;$!d' | simple_output
          else
            cat $C3SURF_LOG_PATH/$FORM_subaction | simple_output
          fi
        else
          if [ $FORM_log_sort -eq $_log_sort_reverse_idx ]
          then
            cat $C3SURF_LOG_PATH/$FORM_subaction | do_tail $FORM_log_lines | sed '1!G;h;$!d' | simple_output
          else
            cat $C3SURF_LOG_PATH/$FORM_subaction | do_tail $FORM_log_lines | simple_output
          fi
        fi

        # Tabelle schlieﬂen
        echo "</table>"

        show_tab_footer

      fi # Lˆschen Best‰tigen

    else
      show_info "$_gen_info_head" "$_log_info_txt"
    fi
  # Ende TAB:logview [main=logview]
  #################################
  ### ende   TAB: Logview   ###
  #################################
  ;;

  *)
    show_error "$_tab_gen_err_title" "$_tab_gen_err_unknowntab_text"

esac

#
show_tab_footer

#
show_html_footer
